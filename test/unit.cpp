/**
 * @file unit.hpp
 * @author Daniel Starke
 * @copyright Copyright 2022 Daniel Starke
 * @date 2022-07-07
 * @version 2022-07-21
 */
#include "../src/HidDescriptor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>


using namespace ::hid::error;


#ifndef NSANITY
/** Source code for sanity check. */
static constexpr const char sanityCheckSrc[] = R"(
UsagePage(Button)
Usage(Button20)
Collection(Application)
Unit(SiLin(Length Mass^2))
Input(3, Rel, {arg2})
0x13
{arg1}
EndCollection
)";


/** Compile time compiled descriptor for sanity check. */
DEF_HID_DESCRIPTOR_AS(
	static sanityCheckDesc,
	(sanityCheckSrc)
	("arg1", 1)
	("arg2", 2)
	("arg3", 3)
);


/** Descriptor data for sanity check. */
static const uint8_t sanityCheckData[] = {
	0x05, 0x09, 0x09, 0x14, 0xA1, 0x01, 0x66, 0x11, 0x02, 0x81, 0x07, 0x13, 0x01, 0xC0
};
#endif /* not NSANITY */


/** Source code management helper class. */
struct Source {
	const char * const source;
	const size_t length;
	
	/** Constructor. */
	Source(const char * const s, const size_t l):
		source{s},
		length{l}
	{}
		
	/** Return pointer to the source code. */
	const char * data() const {
		return this->source;
	}
	
	/** Return the source code size. */
	size_t size() const {
		return this->length;
	}
	
	/** Return some argument values. */
	::hid::detail::ParamMatch find(const ::hid::detail::Token & token) const noexcept {
		if ( ::hid::detail::equals(token, "arg1") ) {
			return ::hid::detail::ParamMatch{1, true};
		} else if ( ::hid::detail::equals(token, "arg2") ) {
			return ::hid::detail::ParamMatch{256, true};
		} else if ( ::hid::detail::equals(token, "arg3") ) {
			return ::hid::detail::ParamMatch{-1, true};
		} else if ( ::hid::detail::equals(token, "arg4") ) {
			return ::hid::detail::ParamMatch{4294967295L, true};
		} else if ( ::hid::detail::equals(token, " arg5 ") ) {
			return ::hid::detail::ParamMatch{4294967296LL, true};
		}
		return ::hid::detail::ParamMatch{0, false};
	}
};


/** Single test vector. */
struct Test {
	const char * const source;
	uint8_t * data;
	const size_t size;
	const EMessage result;
	const size_t errorPos;

	/** Constructor. */
	Test(const char * const s, const EMessage r, const size_t p = 0, std::initializer_list<uint8_t> d = {}):
		source{s},
		data{NULL},
		size{d.size()},
		result{r},
		errorPos{p}
	{
		this->initData(d);
	}
	Test(const char * const s, const EMessage r, std::initializer_list<uint8_t> d):
		source{s},
		data{NULL},
		size{d.size()},
		result{r},
		errorPos{0}
	{
		this->initData(d);
	}
	/** Destructor. */
	~Test() {
		if (this->data != NULL) {
			free(this->data);
		}
	}
private:
	/**
	 * Initializes the data from the given initialization list.
	 * 
	 * @param[in] d - data initialization list
	 */
	void initData(std::initializer_list<uint8_t> d) {
		if (d.size() > 0) {
			this->data = reinterpret_cast<uint8_t *>(malloc(sizeof(uint8_t) * d.size()));
			if (this->data == NULL) {
				fprintf(stderr, "Failed to allocate memory.\n");
				exit(EXIT_FAILURE);
			}
			uint8_t * dst = this->data;
			for (const uint8_t val : d) {
				*dst++ = val;
			}
		}
	}
};


/**
 * Returns the smaller value.
 * 
 * @param[in] a - one value
 * @param[in] b - another value
 * @return smallest value
 */
template <typename T>
constexpr static T min(const T a, const T b) {
	return (a < b) ? a : b;
}


/**
 * Quotes the source code to avoid line-feeds and carrier-returns.
 * The output is written to standard output.
 * 
 * @param[in] src - null-terminated source code string
 */
static void quoteCode(const char * src) {
	printf("\"");
	while (*src != 0) {
		switch (*src) {
		case '\r':
			printf("\\r");
			break;
		case '\n':
			printf("\\n");
			break;
		default:
			printf("%c", *src);
			break;
		}
		src++;
	}
	printf("\"\n");
}


/**
 * Outputs the given buffer as hex dump to standard output.
 * 
 * @param[in] buf - pointer to the buffer
 * @param[in] len - size of the buffer in bytes
 */
static void hexDump(const uint8_t * buf, const size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (i != 0) {
			printf(", ");
		}
		printf("0x%02X", int(buf[i]));
	}
	if (len == 0 || (len % 16) != 0) {
		printf("\n");
	}
}


/** Entry point. */
int main() {
	enum {
		FAILED_RESULT         = 1,
		FAILED_ERROR_POSITION = 2,
		FAILED_DATA_SIZE      = 4,
		FAILED_DATA_CONTENT   = 8
	};
	uint8_t buf[65536];
	hid::Error error;
	unsigned failMask;
	size_t failed{0}, total{0};
#ifndef NSANITY
	{
		/* sanity check */
		const auto source = hid::fromSource(sanityCheckSrc)("arg1", 1)("arg2", 2)("arg3", 3);
		hid::detail::BufferWriter out(buf, sizeof(buf));
		hid::compile(source, out, error);
		if (sanityCheckDesc.size() != out.getPosition() || sanityCheckDesc.size() != sizeof(sanityCheckData) || memcmp(sanityCheckDesc.data, buf, sanityCheckDesc.size()) != 0) {
			printf("Error: Sanity check failed.\n");
			return EXIT_FAILURE;
		}
	}
#endif /* not NSANITY */
	/* unit tests, see `struct Test` */
	const Test tests[] = {
		/* comment */
		Test("#", E_NO_ERROR),
		Test("#\n", E_NO_ERROR),
		Test("#\r", E_NO_ERROR),
		Test("#\n0", E_NO_ERROR, {0}),
		Test("#\r0", E_NO_ERROR, {0}),
		Test("# text", E_NO_ERROR),
		Test("# text\n", E_NO_ERROR),
		Test("# text\r", E_NO_ERROR),
		Test("# text\n0", E_NO_ERROR, {0}),
		Test("# text\r0", E_NO_ERROR, {0}),
		Test(";", E_NO_ERROR),
		Test(";\n", E_NO_ERROR),
		Test(";\r", E_NO_ERROR),
		Test(";\n0", E_NO_ERROR, {0}),
		Test(";\r0", E_NO_ERROR, {0}),
		Test("; text", E_NO_ERROR),
		Test("; text\n", E_NO_ERROR),
		Test("; text\r", E_NO_ERROR),
		Test("; text\n0", E_NO_ERROR, {0}),
		Test("; text\r0", E_NO_ERROR, {0}),
		/* top level number literal */
		Test("0", E_NO_ERROR, {0}),
		Test("0\n", E_NO_ERROR, {0}),
		Test("0\r", E_NO_ERROR, {0}),
		Test("0 ", E_NO_ERROR, {0}),
		Test("1", E_NO_ERROR, {1}),
		Test("256", E_NO_ERROR, {0, 1}), /* little endian */
		Test("4294967295", E_NO_ERROR, {255, 255, 255, 255}),
		Test("4294967296", E_Number_overflow, 9),
		Test("42949672950", E_Number_overflow, 10),
		Test("-1", E_Negative_numbers_are_not_allowed_in_this_context, 0),
		Test("1a", E_Invalid_numeric_value, 1),
		Test("1#", E_Invalid_numeric_value, 1),
		Test("1;", E_Invalid_numeric_value, 1),
		/* top level hex literal */
		Test("0x0", E_NO_ERROR, {0x00}),
		Test("0x0\n", E_NO_ERROR, {0x00}),
		Test("0x0\r", E_NO_ERROR, {0x00}),
		Test("0x0 ", E_NO_ERROR, {0x00}),
		Test("0x1", E_NO_ERROR, {0x01}),
		Test("0x100", E_NO_ERROR, {0x00, 0x01}), /* little endian */
		Test("0xFFFFFFFF", E_NO_ERROR, {0xFF, 0xFF, 0xFF, 0xFF}),
		Test("0xffffffff", E_NO_ERROR, {0xFF, 0xFF, 0xFF, 0xFF}),
		Test("0x100000000", E_Number_overflow, 10),
		Test("0X0", E_Invalid_numeric_value, 1),
		Test("0x0z", E_Invalid_hex_value, 3),
		Test("0x0#", E_Invalid_hex_value, 3),
		Test("0x0;", E_Invalid_hex_value, 3),
		Test("0x", E_Unexpected_end_of_source, 2),
		Test("0xZ", E_Invalid_hex_value, 2),
		/* top level parameter, see Source::find() */
		Test("{arg1}", E_NO_ERROR, {1}),
		Test("{arg1}\n", E_NO_ERROR, {1}),
		Test("{arg1}\r", E_NO_ERROR, {1}),
		Test("{arg1} ", E_NO_ERROR, {1}),
		Test("{arg1}{arg1}", E_NO_ERROR, {1, 1}),
		Test("{arg2}", E_NO_ERROR, {0, 1}), /* little endian */
		Test("{arg3}", E_Negative_numbers_are_not_allowed_in_this_context, 5),
		Test("{arg4}", E_NO_ERROR, {255, 255, 255, 255}),
		Test("{ arg5 }", E_Parameter_value_out_of_range, 7),
		Test("{arg6}", E_Expected_valid_parameter_name_here, 5),
		Test("{ arg1}", E_Expected_valid_parameter_name_here, 6),
		Test("{arg1 }", E_Expected_valid_parameter_name_here, 6),
		Test("{ arg1 }", E_Expected_valid_parameter_name_here, 7),
		Test("{arg1", E_Unexpected_end_of_source, 5),
		/* items */
		Test("Push", E_NO_ERROR, {0xA4}),
		Test("PUSH", E_NO_ERROR, {0xA4}),
		Test("push", E_NO_ERROR, {0xA4}),
		Test("pushx", E_Invalid_item_name, 5),
		Test("pushx ", E_Invalid_item_name, 5),
		Test("push$", E_Unexpected_item_name_character, 4),
		Test("Push(10)", E_This_item_has_no_arguments, 4),
		Test("Pushx(10)", E_Invalid_item_name, 5),
		Test("UsagePage(GenericDesktop)", E_NO_ERROR, {0x05, 0x01}),
		Test("USAGEPAGE(GENERICDESKTOP)", E_NO_ERROR, {0x05, 0x01}),
		Test("  UsagePage  (  GenericDesktop  )  ", E_NO_ERROR, {0x05, 0x01}),
		Test("\nUsagePage\n(\nGenericDesktop\n)\n", E_NO_ERROR, {0x05, 0x01}),
		Test("\rUsagePage\r(\nGenericDesktop\r)\r", E_NO_ERROR, {0x05, 0x01}),
		Test("\tUsagePage\t(\nGenericDesktop\t)\t", E_NO_ERROR, {0x05, 0x01}),
		/* arguments */
		Test("UsagePage(1)", E_NO_ERROR, {0x05, 0x01}),
		Test("UsagePage(0x1)", E_NO_ERROR, {0x05, 0x01}),
		Test("Delimiter(Open)Delimiter(Close)", E_NO_ERROR, {0xA9, 0x01, 0xA9, 0x00}),
		Test("Delimiter(Open) Delimiter(Close)", E_NO_ERROR, {0xA9, 0x01, 0xA9, 0x00}),
		Test("Delimiter(Open)\nDelimiter(Close)", E_NO_ERROR, {0xA9, 0x01, 0xA9, 0x00}),
		Test("Delimiter(Open)\tDelimiter(Close)", E_NO_ERROR, {0xA9, 0x01, 0xA9, 0x00}),
		Test("Delimiter(Open)\rDelimiter(Close)", E_NO_ERROR, {0xA9, 0x01, 0xA9, 0x00}),
		Test("Delimiter(Open Open)\rDelimiter(Close)", E_Unexpected_token, 15),
		Test("Delimiter(Open)\nDelimiter(Unknown)", E_Invalid_argument_name, 33, {0xA9, 0x01}),
		Test("Delimiter(2)", E_Unexpected_Delimiter_value, 11),
		Test("UsagePage(-1)", E_Negative_numbers_are_not_allowed_in_this_context, 10),
		Test("UsagePage(1", E_Unexpected_end_of_source, 11),
		Test("UsagePage(0x", E_Unexpected_end_of_source, 12),
		Test("UsagePage(0x1", E_Unexpected_end_of_source, 13),
		Test("UsagePage(0xZ)", E_Invalid_hex_value, 12),
		Test("UsagePage(0xAZ)", E_Invalid_hex_value, 13),
		Test("UsagePage(a$)", E_Unexpected_argument_name_character, 11),
		Test("LogicalMaximum(1)", E_NO_ERROR, {0x25, 0x01}),
		Test("LogicalMaximum(-1)", E_NO_ERROR, {0x25, 0xFF}),
		Test("LogicalMaximum(127)", E_NO_ERROR, {0x25, 0x7F}),
		Test("LogicalMaximum(-128)", E_NO_ERROR, {0x25, 0x80}),
		Test("LogicalMaximum(128)", E_NO_ERROR, {0x26, 0x80, 0x00}),
		Test("LogicalMaximum(-129)", E_NO_ERROR, {0x26, 0x7F, 0xFF}),
		Test("LogicalMaximum(32767)", E_NO_ERROR, {0x26, 0xFF, 0x7F}),
		Test("LogicalMaximum(-32768)", E_NO_ERROR, {0x26, 0x00, 0x80}),
		Test("LogicalMaximum(32768)", E_NO_ERROR, {0x27, 0x00, 0x80, 0x00, 0x00}),
		Test("LogicalMaximum(-32769)", E_NO_ERROR, {0x27, 0xFF, 0x7F, 0xFF, 0xFF}),
		Test("LogicalMaximum(2147483647)", E_NO_ERROR, {0x27, 0xFF, 0xFF, 0xFF, 0x7F}),
		Test("LogicalMaximum(0x7FFFFFFF)", E_NO_ERROR, {0x27, 0xFF, 0xFF, 0xFF, 0x7F}),
		Test("LogicalMaximum(0x7fffffff)", E_NO_ERROR, {0x27, 0xFF, 0xFF, 0xFF, 0x7F}),
		Test("LogicalMaximum(-2147483648)", E_NO_ERROR, {0x27, 0x00, 0x00, 0x00, 0x80}),
		Test("LogicalMaximum(2147483648)", E_Number_overflow, 25),
		Test("LogicalMaximum(0x80000000)", E_Number_overflow, 25),
		Test("LogicalMaximum(-2147483649)", E_Number_overflow, 26),
		Test("LogicalMaximum({arg4})", E_Parameter_value_out_of_range, 20),
		Test("StringMaximum(4294967296)", E_Number_overflow, 23),
		Test("StringMaximum(42949672950)", E_Number_overflow, 24),
		Test("StringMaximum(0x100000000)", E_Number_overflow, 24),
		Test("StringMaximum(10z)", E_Invalid_numeric_value, 16),
		Test("ReportId(1)", E_NO_ERROR, {0x85, 0x01}),
		Test("ReportId({arg4})", E_NO_ERROR, {0x87, 0xFF, 0xFF, 0xFF, 0xFF}),
		Test("ReportId({arg4", E_Unexpected_end_of_source, 14),
		Test("ReportId(-1)", E_Negative_numbers_are_not_allowed_in_this_context, 9),
		Test("UsagePage(1)", E_NO_ERROR, {0x05, 0x01}),
		Test("UsagePage(0x10000)", E_Argument_value_out_of_range, 17),
		Test("UsagePage({arg4})", E_Argument_value_out_of_range, 16),
		Test("UsagePage({ arg5 })", E_Parameter_value_out_of_range, 17),
		Test("UsagePage(GenericDesktop)\nUsage(0x10000)", E_Argument_value_out_of_range, 39, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage({arg4})", E_Argument_value_out_of_range, 38, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsageMinimum(0x10000)", E_Argument_value_out_of_range, 46, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsageMinimum({arg4})", E_Argument_value_out_of_range, 45, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsageMaximum(0x10000)", E_Argument_value_out_of_range, 46, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsageMaximum({arg4})", E_Argument_value_out_of_range, 45, {0x05, 0x01}),
		Test("UsagePage(Generic Desktop)", E_Invalid_argument_name, 17),
		Test("UsagePage(Generic\nDesktop)", E_Invalid_argument_name, 17),
		Test("UsagePage(Generic\nDesktop)", E_Invalid_argument_name, 17),
		/* arguments with index */
		Test("UsagePage(Button)\nUsage(NoButtonPressed)", E_NO_ERROR, {0x05, 0x09, 0x09, 0x00}),
		Test("UsagePage(Button)\nUsage(Button1)", E_NO_ERROR, {0x05, 0x09, 0x09, 0x01}),
		Test("UsagePage(Button)\nUsage(Button65535)", E_NO_ERROR, {0x05, 0x09, 0x0A, 0xFF, 0xFF}),
		Test("UsagePage(MonitorEnumeratedValues)\nUsage(Enum0)", E_NO_ERROR, {0x05, 0x81, 0x09, 0x00}),
		Test("UsagePage(Button)\nUsage(Button65536)", E_Argument_index_out_of_range, 35, {0x05, 0x09}),
		Test("UsagePage(Button)\nUsage(Button01)", E_Invalid_argument_name, 32, {0x05, 0x09}),
		Test("UsagePage(Button)\nUsage(Button1x)", E_Unexpected_argument_name_character, 32, {0x05, 0x09}),
		Test("UsagePage(Button)\nUsage(Butto1)", E_Invalid_argument_name, 30, {0x05, 0x09}),
		Test("UsagePage(Button)\nUsage(Button4294967295)", E_Argument_index_out_of_range, 40, {0x05, 0x09}),
		Test("UsagePage(Button)\nUsage(Button4294967296)", E_Argument_index_out_of_range, 40, {0x05, 0x09}),
		/* multi-value arguments */
		Test("Input(0)", E_NO_ERROR, {0x81, 0x00}),
		Test("Input(Cnst)", E_NO_ERROR, {0x81, 0x01}),
		Test("Input(cnst)", E_NO_ERROR, {0x81, 0x01}),
		Test("Input(CNST)", E_NO_ERROR, {0x81, 0x01}),
		Test("Input(Cnst, Data)", E_NO_ERROR, {0x81, 0x00}),
		Test("Input(Data, Cnst)", E_NO_ERROR, {0x81, 0x01}),
		Test("Input(0,1)", E_NO_ERROR, {0x81, 0x01}),
		Test("Input(2, 1, 256)", E_NO_ERROR, {0x82, 0x03, 0x01}),
		Test("Input(2, {arg1}, 0x100, Rel)", E_NO_ERROR, {0x82, 0x07, 0x01}),
		Test("Input(2, {arg1}, 0x100, Data)", E_NO_ERROR, {0x82, 0x02, 0x01}),
		Test("Input(Cnst, Var, Rel, Warp, NLin, NPrf, Null, Buf)", E_NO_ERROR, {0x82, 0x7F, 0x01}),
		Test("Output(Cnst, Var, Rel, Warp, NLin, NPrf, Null, Vol, Buf)", E_NO_ERROR, {0x92, 0xFF, 0x01}),
		Test("Feature(Cnst, Var, Rel, Warp, NLin, NPrf, Null, Vol, Buf)", E_NO_ERROR, {0xB2, 0xFF, 0x01}),
		Test("Input(0 1)", E_Unexpected_token, 8),
		Test("Input(NVol)", E_Invalid_argument_name, 10),
		Test("Input(Null", E_Unexpected_end_of_source, 10),
		/* UnitExponent argument */
		Test("UnitExponent(0)", E_NO_ERROR, {0x55, 0x00}),
		Test("UnitExponent(1)", E_NO_ERROR, {0x55, 0x01}),
		Test("UnitExponent(7)", E_NO_ERROR, {0x55, 0x07}),
		Test("UnitExponent(8)", E_Argument_value_out_of_range, 14),
		Test("UnitExponent(-1)", E_NO_ERROR, {0x55, 0x0F}),
		Test("UnitExponent(-8)", E_NO_ERROR, {0x55, 0x08}),
		Test("UnitExponent(-9)", E_Argument_value_out_of_range, 15),
		Test("UnitExponent(x1)", E_Invalid_argument_name, 15),
		/* Unit argument */
		Test("Unit(1)", E_NO_ERROR, {0x65, 0x01}),
		Test("Unit(0x1)", E_NO_ERROR, {0x65, 0x01}),
		Test("Unit({arg1})", E_NO_ERROR, {0x65, 0x01}),
		Test("Unit(None)", E_NO_ERROR, {0x65, 0x00}),
		Test("Unit(SiLin)", E_NO_ERROR, {0x65, 0x01}),
		Test("Unit(None())", E_NO_ERROR, {0x65, 0x00}),
		Test("Unit(SiLin())", E_NO_ERROR, {0x65, 0x01}),
		Test("Unit(SiRot())", E_NO_ERROR, {0x65, 0x02}),
		Test("Unit(ENGLIN())", E_NO_ERROR, {0x65, 0x03}),
		Test("Unit(engrot())", E_NO_ERROR, {0x65, 0x04}),
		Test("Unit(None(Length))", E_NO_ERROR, {0x65, 0x10}),
		Test("Unit(SiLin(Length))", E_NO_ERROR, {0x65, 0x11}),
		Test("Unit  (  SiLin  (  Length  )  )  ", E_NO_ERROR, {0x65, 0x11}),
		Test("Unit(SiLin(Length Mass))", E_NO_ERROR, {0x66, 0x11, 0x01}),
		Test("Unit(SiLin(Length^1Mass^1))", E_NO_ERROR, {0x66, 0x11, 0x01}),
		Test("Unit(SiLin(Length Mass^1))", E_NO_ERROR, {0x66, 0x11, 0x01}),
		Test("Unit(SiLin(Length^1 Mass))", E_NO_ERROR, {0x66, 0x11, 0x01}),
		Test("Unit(SiLin(Length^0 Mass))", E_NO_ERROR, {0x66, 0x01, 0x01}),
		Test("Unit(SiLin(Length Mass^0))", E_NO_ERROR, {0x65, 0x11}),
		Test("Unit(SiLin(Length^-8Mass^7))", E_NO_ERROR, {0x66, 0x81, 0x07}),
		Test("Unit(SiLin(Length^7Mass^-1))", E_NO_ERROR, {0x66, 0x71, 0x0F}),
		Test("Unit(SiLin(Temp^3))", E_NO_ERROR, {0x67, 0x01, 0x00, 0x03, 0x00}),
		Test("Unit(SiLin(Length^2Mass^3Time^4temp^5CURRENT^6luminouS^7))", E_NO_ERROR, {0x67, 0x21, 0x43, 0x65, 0x07}),
		Test("Unit(SiLin(luminouS^7CURRENT^6temp^5Time^4Mass^3Length^2))", E_NO_ERROR, {0x67, 0x21, 0x43, 0x65, 0x07}),
		Test("Unit(())", E_Unexpected_argument_name_character, 5),
		Test("Unit()", E_Missing_argument, 5),
		Test("Unit(Unknown())", E_Invalid_unit_system_name, 12),
		Test("Unit(None(Length$))", E_Unexpected_unit_name_character, 16),
		Test("Unit(None(LengthX))", E_Invalid_unit_name, 17),
		Test("Unit(None(^1))", E_Unexpected_unit_name_character, 10),
		Test("Unit(None(1))", E_Unexpected_unit_name_character, 10),
		Test("Unit(None(-1))", E_Unexpected_unit_name_character, 10),
		Test("Unit(None(Length^1-))", E_Invalid_unit_exponent, 18),
		Test("Unit(None(Length^x))", E_Invalid_unit_exponent, 17),
		Test("Unit(None(Length^8))", E_Invalid_unit_exponent, 18),
		Test("Unit(None(Length^-9))", E_Invalid_unit_exponent, 19),
		Test("Unit(None(Length^-0))", E_Invalid_unit_exponent, 19),
		Test("Unit(None$())", E_Unexpected_argument_name_character, 9),
		Test("Unit(None None)", E_Invalid_unit_name, 14),
		Test("Unit(None() None)", E_Unexpected_token, 12),
		Test("Unit(", E_Unexpected_end_of_source, 5),
		Test("Unit(None(", E_Unexpected_end_of_source, 10),
		Test("Unit(None()", E_Unexpected_end_of_source, 11),
		/* semantic error tests */
		Test("UsagePage", E_Missing_argument, 9),
		Test("UsagePage ", E_Missing_argument, 9),
		Test("UsagePage(GenericDesktop)\nUsage", E_Missing_argument, 31, {0x05, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage ", E_Missing_argument, 31, {0x05, 0x01}),
		Test("Usage", E_Missing_argument, 5),
		Test("Usage ", E_Missing_argument, 5),
		Test("Usage(Pointer)", E_Missing_UsagePage, 13),
		Test("Collection", E_Missing_Usage_for_Collection, 10),
		Test("Collection(Application)", E_Missing_Usage_for_Collection, 10),
		Test("EndCollection", E_Unexpected_EndCollection, 13),
		Test("EndCollection ", E_Unexpected_EndCollection, 13),
		Test("UsagePage(1)\nUsage(1)", E_NO_ERROR, {0x05, 0x01, 0x09, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage(0x1)\nUsage(0x1)", E_NO_ERROR, {0x05, 0x01, 0x09, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage({arg1})\nUsage({arg1})", E_NO_ERROR, {0x05, 0x01, 0x09, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage(1)\nUsage(Pointer)", E_Missing_named_UsagePage, 26, {0x05, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage(0x1)\nUsage(Pointer)", E_Missing_named_UsagePage, 28, {0x05, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage({arg1})\nUsage(Pointer)", E_Missing_named_UsagePage, 31, {0x05, 0x01}), /* valid, but without named Usage arguments */
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection", E_Missing_argument, 51, {0x05, 0x01, 0x09, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)", E_Missing_EndCollection, 64, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application) ", E_Missing_EndCollection, 65, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportSize(1)\nEndCollection", E_Missing_ReportCount, 92, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x75, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportSize(1)\nEndCollection ", E_Missing_ReportCount, 92, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x75, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportCount(1)\nEndCollection", E_Missing_ReportSize, 93, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x95, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportCount(1)\nEndCollection ", E_Missing_ReportSize, 93, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x95, 0x01}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportSize(1)\nReportCount(1)\nEndCollection", E_NO_ERROR, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x75, 0x01, 0x95, 0x01, 0xC0}),
		Test("UsagePage(GenericDesktop)\nUsage(Pointer)\nCollection(Application)\nReportSize(1)\nReportCount(1)\nEndCollection ", E_NO_ERROR, {0x05, 0x01, 0x09, 0x01, 0xA1, 0x01, 0x75, 0x01, 0x95, 0x01, 0xC0}),
		Test("Delimiter(0)", E_Unexpected_DelimiterClose, 11),
		Test("Delimiter(Close)", E_Unexpected_DelimiterClose, 15),
		Test("Delimiter(Open)", E_Missing_DelimiterClose, 15, {0xA9, 0x01}),
		Test("Delimiter(Open) ", E_Missing_DelimiterClose, 16, {0xA9, 0x01}),
		/* miscellaneous error tests */
		Test("", E_NO_ERROR),
		Test("$", E_Unexpected_token, 0)
	};
	/* preform the unit tests and print details for failed tests */
	for (const Test & test : tests) {
		Source src(test.source, strlen(test.source));
		hid::detail::BufferWriter out(buf, sizeof(buf));
		hid::compile(src, out, error);
		failMask = 0;
		if (error.message != test.result) {
			failMask |= FAILED_RESULT;
		}
		if (test.result != E_NO_ERROR && test.errorPos != error.character) {
			failMask |= FAILED_ERROR_POSITION;
		}
		if (out.getPosition() != test.size) {
			failMask |= FAILED_DATA_SIZE;
		}
		if (test.size > 0 && out.getPosition() > 0) {
			if (memcmp(buf, test.data, min(test.size, out.getPosition())) != 0) {
				failMask |= FAILED_DATA_CONTENT;
			}
		}
		if ( failMask ) {
			if (failed > 0) {
				printf("###############################################################################\n");
			}
			printf("source:   "); quoteCode(test.source);
			printf("data:     "); hexDump(test.data, test.size);
			printf("size:     %u\n", unsigned(test.size));
			printf("result:   %s\n", EMessageStr[test.result]);
			if (test.result != E_NO_ERROR) {
				printf("position: %u\n", unsigned(test.errorPos));
			}
			if (failMask & FAILED_RESULT) {
				printf("mismatching result:         %s\n", EMessageStr[error.message]);
			}
			if (failMask & FAILED_ERROR_POSITION) {
				printf("mismatching error position: %u\n", unsigned(error.character));
			}
			if (failMask & FAILED_DATA_SIZE) {
				printf("mismatching data size:      %u\n", unsigned(out.getPosition()));
			}
			if (failMask & FAILED_DATA_CONTENT) {
				printf("mismatching data:           "); hexDump(buf, out.getPosition());
			}
			failed++;
		}
		total++;
	}
	if (failed > 0) {
		printf("\n");
	}
	printf("FAILED: %u\n", unsigned(failed));
	printf("PASSED: %u\n", unsigned(total - failed));
	printf("TOTAL:  %u\n", unsigned(total));
	return EXIT_SUCCESS;
}
