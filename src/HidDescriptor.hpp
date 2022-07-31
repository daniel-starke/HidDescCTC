/**
 * @file HidDescriptor.hpp
 * @author Daniel Starke
 * @copyright Copyright 2022 Daniel Starke
 * @date 2022-04-20
 * @version 2022-07-31
 * 
 * Helper functions to build a USB HID descriptor. Use `DEF_HID_DESCRIPTOR_AS()`.
 * 
 * @todo check against `UsageType` (may need state table handling)
 * @todo check if HidDescriptor().compile() can be implemented to remove macros
 * 
 * @see https://www.usb.org/sites/default/files/hid1_11.pdf
 * @see - https://www.usb.org/sites/default/files/hut1_2.pdf
 * @see   - https://www.usb.org/sites/default/files/pid1_01.pdf
 * @see   - https://www.usb.org/sites/default/files/usbmon10.pdf
 * @see   - https://www.usb.org/sites/default/files/pdcv10.pdf
 * @see   - https://www.usb.org/sites/default/files/pos1_02.pdf
 * @see   - https://www.usb.org/sites/default/files/oaaddataformatsv6.pdf
 * @see ::hid::detail::compile()
 * @remarks Define `HID_DESCRIPTOR_DEBUG` for runtime debugging (not compile time).
 * @note The usage names are derived from the standard by applying the following rules:
 * - replace leading `+` by `Plus`
 * - replace `/second/second` by `PerSecondSquared`
 * - remove all non-alphanumeric characters like space and underscore characters
 * - capitalize words/abbreviations, whereas dimensions count as one word (e.g. Usb3dControl)
 * - move words with a leading digit behind the first word
 * - remove second key meaning for the keyboard/keypad usage table entries
 */
#ifndef __HIDDESCRIPTOR_HPP__
#define __HIDDESCRIPTOR_HPP__

extern "C" {
#include <stdint.h>
#include <stddef.h>
#ifdef HID_DESCRIPTOR_DEBUG
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#endif
} /* extern "C" */


#if __cpp_constexpr < 201304
#error A compiler with constexpr support of C++14 or newer is required.
#endif


/**
 * @def DEF_HID_DESCRIPTOR_AS
 * Compiles the HID descriptor from the given source code instance.
 * This can be used in global, namespace and function scope. Not in class/struct scope.
 * 
 * @param name - HID descriptor variable name (may contain additional qualifiers like 'static')
 * @param desc - HID descriptor source code
 * @note Make sure to assign the result to a variable with `constexpr` attribute.
 * @see ::hid::detail::Descriptor
 * @remarks Define `HID_DESCRIPTOR_NO_ERROR_REPORT` to suppress error reporting.
 */
#ifdef HID_DESCRIPTOR_NO_ERROR_REPORT
#define DEF_HID_DESCRIPTOR_AS(name, desc) \
	constexpr const auto name = ::hid::Descriptor<::hid::compiledSize(::hid::fromSource desc)>(::hid::fromSource desc)
#else /* not HID_DESCRIPTOR_NO_ERROR_REPORT */
#define DEF_HID_DESCRIPTOR_AS(name, desc) \
	constexpr static const ::hid::Error _hid_error_##__LINE__ = ::hid::compileError(::hid::fromSource desc); \
	constexpr static const size_t _hid_error_##__LINE__##_num = ::hid::reporter<_hid_error_##__LINE__.line, _hid_error_##__LINE__.column, _hid_error_##__LINE__.message>(); \
	constexpr const auto name = ::hid::Descriptor<::hid::compiledSize(::hid::fromSource desc)>(::hid::fromSource desc)
#endif /* not HID_DESCRIPTOR_NO_ERROR_REPORT */


namespace hid {
namespace error {


/**
 * Possible compile error messages.
 */
enum EMessage {
	E_NO_ERROR,
	E_Internal_error,
	E_Unexpected_token,
	E_Number_overflow,
	E_Parameter_value_out_of_range,
	E_Unexpected_end_of_source,
	E_Expected_valid_parameter_name_here,
	E_Invalid_item_name,
	E_Missing_argument,
	E_Missing_named_UsagePage,
	E_Missing_UsagePage,
	E_Missing_Usage_for_Collection,
	E_This_item_has_no_arguments,
	E_Unexpected_item_name_character,
	E_Invalid_argument_name,
	E_Argument_value_out_of_range,
	E_Argument_index_out_of_range,
	E_Unexpected_argument_name_character,
	E_Unexpected_unit_name_character,
	E_Invalid_unit_system_name,
	E_Invalid_unit_name,
	E_Invalid_unit_exponent,
	E_Unexpected_EndCollection,
	E_Unexpected_DelimiterClose,
	E_Unexpected_Delimiter_value,
	E_Missing_EndCollection,
	E_Missing_DelimiterClose,
	E_Missing_ReportSize,
	E_Missing_ReportCount,
	E_Invalid_hex_value,
	E_Invalid_numeric_value,
	E_Negative_numbers_are_not_allowed_in_this_context
};


/**
 * `EMessage` to string mapping.
 */
static constexpr const char * EMessageStr[] __attribute__((unused)) = {
	"No error.",
	"Internal error.",
	"Unexpected token.",
	"Number overflow.",
	"Parameter value out of range.",
	"Unexpected end of source.",
	"Expected valid parameter name here.",
	"Invalid item name.",
	"Missing argument.",
	"Missing named UsagePage.",
	"Missing UsagePage.",
	"Missing Usage for Collection.",
	"This item has no arguments.",
	"Unexpected item name character.",
	"Invalid argument name.",
	"Argument value out of range.",
	"Argument index out of range.",
	"Unexpected argument name character.",
	"Unexpected unit name character.",
	"Invalid unit system name.",
	"Invalid unit name.",
	"Invalid unit exponent.",
	"Unexpected EndCollection.",
	"Unexpected Delimiter(Close).",
	"Unexpected Delimiter value.",
	"Missing EndCollection.",
	"Missing Delimiter(Close).",
	"Missing ReportSize.",
	"Missing ReportCount.",
	"Invalid hex value.",
	"Invalid numeric value.",
	"Negative numbers are not allowed in this context."
};


/**
 * Error output helper.
 * 
 * @see ::hid::detail::errorCheck()
 */
struct Info {
	size_t character;
	size_t line;
	size_t column;
	EMessage message;
	/** Default constructor. */
	constexpr inline Info() noexcept:
		character{0},
		line{0},
		column{0},
		message{E_NO_ERROR}
	{}
};


/**
 * Error output helper.
 * 
 * @see ::hid::detail::errorCheck()
 */
template <size_t Line, size_t Column, EMessage Message>
constexpr inline size_t reporter() noexcept {
	const size_t error[1] = {0};
	return error[Message];
}


/**
 * Sets the error from the given position and message.
 * 
 * @param[out] error - sets this error variable
 * @param[in] pos - input byte position
 * @param[in] msg - error message
 * @return false
 */

class ErrorWriter {
private:
	const char * source;
	Info & error;
public:
	/**
	 * Constructor.
	 * 
	 * @param[in] s - source code base pointer
	 * @param[out] e - error output variable
	 */
	constexpr inline explicit ErrorWriter(const char * s, Info & e) noexcept:
		source{s},
		error{e}
	{}
	
	/**
	 * Sets the error output variable to the given position and error message.
	 * 
	 * @param[in] pos - source code position
	 * @param[in] msg - error message to output
	 * @return false
	 */
	constexpr inline bool at(const size_t pos, const EMessage msg) noexcept {
		this->error.character = 0;
		this->error.line = 1;
		this->error.column = 1;
		this->error.message = msg;
		for (size_t n = 0; n < pos; n++) {
			const int c = int(this->source[n]);
			if ((c & 0xC0) != 0x80) {
				this->error.character++;
			}
			if (c == '\n') {
				this->error.line++;
				this->error.column = 1;
			} else if (c != '\r') {
				/* we only count the first byte of a UTF-8 character */
				if ((c & 0xC0) != 0x80) {
					this->error.column++;
				}
			}
		}
		return false;
	}
};


} /* namespace error */
namespace detail {
namespace {


/**
 * Parsing token.
 */
struct Token {
	const char * start;
	size_t length;
};


/**
 * Converts the given character to its lower case variant.
 * 
 * @param[in] val - input character
 * @return lower case equivalent
 */
constexpr inline int toLower(const int val) noexcept {
	if (val >= 'A' && val <= 'Z') {
		return val - 'A' + 'a';
	}
	return val;
}


/**
 * Converts the given character to its upper case variant.
 * 
 * @param[in] val - input character
 * @return upper case equivalent
 */
constexpr inline int toUpper(const int val) noexcept {
	if (val >= 'a' && val <= 'z') {
		return val - 'a' + 'A';
	}
	return val;
}


/**
 * Returns the null-terminated string length excluding the
 * null-terminator.
 * 
 * @param[in] str - input string
 * @return number of characters without null-terminator
 */
constexpr inline size_t strLen(const char * str) noexcept {
	size_t len = 0;
	while (*str != 0) {
		str++;
		len++;
	}
	return len;
}


/**
 * Searches for the given character in the the null-terminated string.
 * The character position is returned on success, or -1 if not found.
 * 
 * @param[in] str - input string
 * @param[in] c - character to search for
 * @return position of first character occurrence or -1 if not found
 */
constexpr inline int strFindChr(const char * str, const int c) noexcept {
	int res = 0;
	while (*str != 0) {
		if (*str == c) {
			return res;
		}
		str++;
		res++;
	}
	return -1;
}


/**
 * Checks whether both null-terminated string are the same.
 * 
 * @param[in] str1 - first string
 * @param[in] str2 - second string
 * @return true on match, else false
 * @remarks The function is case sensitive.
 */
constexpr inline bool equals(const char * str1, const char * str2) noexcept {
	while (*str1 != 0 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *str1 == 0 && *str2 == 0;
}


/**
 * Checks whether the given null-terminated string starts with the
 * passed null-terminated prefix.
 * 
 * @param[in] prefix - prefix string
 * @param[in] str - string
 * @return match length on match, else 0
 * @remarks The function is case sensitive.
 */
constexpr inline size_t startWidth(const char * prefix, const char * str) noexcept {
	size_t res = 0;
	while (*prefix && (*prefix == *str)) {
		prefix++;
		str++;
		res++;
	}
	return (*prefix == 0) ? res : 0;
}


/**
 * Checks whether the given null-terminated string starts with the
 * passed null-terminated prefix.
 * 
 * @param[in] prefix - prefix string
 * @param[in] len - prefix string length
 * @param[in] str - string
 * @return match length on match, else 0
 * @remarks The function is case sensitive.
 */
constexpr inline bool startWidthN(const char * prefix, const size_t len, const char * str) noexcept {
	size_t n = 0;
	size_t res = 0;
	while (*prefix && (*prefix == *str) && n < len) {
		prefix++;
		str++;
		n++;
		res++;
	}
	return (*prefix == 0 || n >= len) ? res : 0;
}


/**
 * Checks whether the given null-terminated string starts with the
 * passed null-terminated prefix by comparing both case in-sensitive.
 * 
 * @param[in] prefix - prefix string
 * @param[in] len - prefix string length
 * @param[in] str - string
 * @return match length on match, else 0
 * @remarks The function is case sensitive.
 */
constexpr inline bool startWidthIN(const char * prefix, const size_t len, const char * str) noexcept {
	size_t n = 0;
	size_t res = 0;
	while (*prefix && (toUpper(*prefix) == toUpper(*str)) && n < len) {
		prefix++;
		str++;
		n++;
		res++;
	}
	return (*prefix == 0 || n >= len) ? res : 0;
}


/**
 * Compares the given token with a passed string. Both are compared case sensitive.
 * The token needs to match the passed string exactly and completely to return true.
 * 
 * @param[in] token - token to compare
 * @param[in] str - compare with this string
 * @return true on match, else false
 */
constexpr inline bool equals(const Token & token, const char * str) noexcept {
	const char * left = token.start;
	const char * right = str;
	size_t length = token.length;
	for (;length > 0 && *right != 0 && *left == *right; length--, left++, right++);
	if (length > 0 && *right == 0) {
		return false;
	} else if (length == 0 && *right != 0) {
		return false;
	} else if (length == 0 && *right == 0) {
		return true;
	}
	return *left == *right;
}


/**
 * Compares the given token with a passed string. Both are compared case in-sensitive.
 * The token needs to match the passed string exactly and completely to return true.
 * 
 * @param[in] token - token to compare
 * @param[in] str - compare with this string
 * @return true on match, else false
 */
constexpr inline bool equalsI(const Token & token, const char * str) noexcept {
	const char * left = token.start;
	const char * right = str;
	size_t length = token.length;
	for (;length > 0 && *right != 0 && toUpper(*left) == toUpper(*right); length--, left++, right++);
	if (length > 0 && *right == 0) {
		return false;
	} else if (length == 0 && *right != 0) {
		return false;
	} else if (length == 0 && *right == 0) {
		return true;
	}
	return toUpper(*left) == toUpper(*right);
}


/**
 * Checks whether the given character is a start of comment character.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isComment(const int val) noexcept {
	switch (val) {
	case '#':
	case ';':
		return true;
	default:
		return false;
	}
}


/**
 * Checks whether the given character is a whitespace character.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isWhitespace(const int val) noexcept {
	switch (val) {
	case ' ':
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
		return true;
	default:
		return false;
	}
}


/**
 * Checks whether the given character is a valid alphabet character.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isAlpha(const int val) noexcept {
	return (val >= 'a' && val <= 'z') || (val >= 'A' && val <= 'Z');
}


/**
 * Checks whether the given character is a valid digit.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isDigit(const int val) noexcept {
	return val >= '0' && val <= '9';
}


/**
 * Checks whether the given character is a valid hex digit.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isHexDigit(const int val) noexcept {
	return isDigit(val) || (val >= 'a' && val <= 'f') || (val >= 'A' && val <= 'F');
}


/**
 * Checks whether the given character is a valid item name character.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isItemChar(const int val) noexcept {
	return val == '_' || isAlpha(val);
}


/**
 * Checks whether the given character is a valid argument value character.
 * 
 * @param[in] val - input character
 * @return true on match, else false
 */
constexpr inline bool isArgChar(const int val) noexcept {
	return val == '_' || isAlpha(val) || isDigit(val);
}


/**
 * Single HID descriptor input source parameter.
 */
struct Param {
	const char * name; /**< parameter name */
	int64_t value; /**< parameter value */
};


/**
 * Helper structure to encapsulate a parameter match.
 */
struct ParamMatch {
	int64_t value; /**< parameter value */
	bool valid; /** true if the value is value, i.e. a parameter with the passed name was found */
};


/**
 * HID descriptor input source with parameter set.
 * 
 * @tparam S - source size in characters
 * @tparam P - parameter count
 */
template <size_t S, size_t P = 0>
struct Source {
    char code[S]; /**< Source code. */
    enum { Size = S }; /**< Source size. */
	Param params[P + 1]; /**< Parameter set. */
    enum { Count = P }; /**< Parameter count. */
	
	/** Default constructor. */
    constexpr inline Source() noexcept: code{0}, params{0} {}
	
	/**
	 * Returns a pointer to the source code.
	 * 
	 * @return source code pointer
	 */
	constexpr inline const char * data() const noexcept {
		return this->code;
	}
	
	/**
	 * Returns the source code size in bytes.
	 * 
	 * @return source code size in characters
	 */
	constexpr inline size_t size() const noexcept {
		return S;
	}
	
	/**
	 * Returns the parameter count.
	 * 
	 * @return parameter count
	 */
	constexpr inline size_t count() const noexcept {
		return P;
	}
	
	/**
	 * Adds a new parameter by name and value.
	 * 
	 * @param[in] name - parameter name
	 * @param[in] value - parameter value
	 * @return Source code with appended parameter
	 */
	constexpr inline Source<S, P + 1> operator() (const char * name, const int64_t value) const noexcept {
		Source<S, P + 1> tmp;
		/* copy previous source code */
		for (size_t s = 0; s < S; s++) {
			tmp.code[s] = this->code[s];
		}
		/* copy previous parameter set */
		for (size_t p = 0; p < P; p++) {
			tmp.params[p] = this->params[p];
		}
		/* append new parameter */
		tmp.params[P].name = name;
		tmp.params[P].value = value;
		return tmp;
	}
	
	/**
	 * Finds a parameter with the given name in the internal parameter set.
	 * The value of the last parameter with this name will be returned.
	 * 
	 * @param[in] token - parameter name token
	 * @return associated value
	 */
	constexpr inline ParamMatch find(const Token & token) const noexcept {
		for (int p = P - 1; p >= 0; p--) {
			if ( equals(token, this->params[p].name) ) {
				return ParamMatch{this->params[p].value, true};
			}
		}
		return ParamMatch{0, false};
	}
};


/** Does nothing. */
class NullWriter {
public:
	/**
	 * Constructor.
	 */
	constexpr inline explicit NullWriter() noexcept {}

	/**
	 * Returns the current write position.
	 * 
	 * @return 0
	 */
	constexpr inline size_t getPosition() const noexcept {
		return 0;
	}

	/**
	 * Writes the given byte to the buffer.
	 * 
	 * @param[in] val - byte value to write
	 * @return true
	 */
	constexpr inline bool write(const uint8_t) const noexcept {
		return true;
	}
};


/** Calculates the needed output size. */
class SizeEstimator {
private:
	size_t pos; /**< position */
public:
	/**
	 * Constructor.
	 */
	constexpr inline explicit SizeEstimator() noexcept:
		pos(0)
	{}

	/**
	 * Returns the current write position.
	 * 
	 * @return write position
	 */
	constexpr inline size_t getPosition() const noexcept {
		return this->pos;
	}

	/**
	 * Writes the given byte to the buffer.
	 * 
	 * @param[in] val - byte value to write
	 * @return true
	 */
	constexpr inline bool write(const uint8_t) noexcept {
		this->pos++;
		return true;
	}
};


/** Writes bytes to a given buffer. */
class BufferWriter {
private:
	uint8_t * const data;
	size_t size;
	size_t pos; /**< position */
public:
	/**
	 * Constructor.
	 * 
	 * @param[in] d - data pointer
	 * @param[in] s - data length
	 */
	constexpr inline explicit BufferWriter(uint8_t * d, const size_t s) noexcept:
		data(d),
		size(s),
		pos(0)
	{}

	/**
	 * Returns the current write position.
	 * 
	 * @return write position
	 */
	constexpr inline size_t getPosition() const noexcept {
		return this->pos;
	}

	/**
	 * Writes the given byte to the buffer.
	 * 
	 * @param[in] val - byte value to write
	 * @return true on success, else false
	 */
	constexpr inline bool write(const uint8_t val) noexcept {
		if (this->pos >= this->size) return false;
		this->data[this->pos++] = val;
		return true;
	}
};


/**
 * Returns the number of bytes needed at least to encode the given unsigned integer.
 * 
 * @param[in] val - value to encode
 * @return minimum encoded number of bytes
 */
constexpr inline size_t encodedSize(const uint32_t val) noexcept {
	if (val > uint32_t(0xFFFF)) return 4;
	if (val > uint32_t(0xFF)) return 2;
	return 1;
}


/**
 * Returns the number of bytes needed at least to encode the given signed integer.
 * 
 * @param[in] val - value to encode
 * @return minimum encoded number of bytes
 */
constexpr inline size_t encodedSize(const int32_t val) noexcept {
	const int32_t absVal = (val < 0) ? -val - 1 : val;
	if (absVal > 0x7FFF) return 4;
	if (absVal > 0x7F) return 2;
	return 1;
}


/**
 * Returns the encoded size mapping value from the passed encoded byte size.
 * 
 * @param[in] val - encoded size in bytes
 * @return mapped value
 * @see HID 1.11 ch. 6.2.2.2
 */
constexpr inline uint32_t encodedSizeValue(const size_t val) noexcept {
	switch (val) {
	case 4: return 3;
	case 2: return 2;
	case 1: return 1;
	default: return 0;
	}
}


/**
 * Encodes the given value with the given length in little-endian format.
 * 
 * @param[in,out] out - write encoded value using this object
 * @param[in] val - value to encode
 * @param[in] len - value length in bytes
 * @return encoded number of bytes
 * @tparam Write - shell implement `write(uint8_t)`
 * @see HID 1.11 ch. 5.8
 */
template <typename Writer>
constexpr inline size_t encodeValue(Writer & out, const uint32_t val, const size_t len) noexcept {
	out.write(uint8_t(val & 0xFF));
	if (len > 1) {
		out.write(uint8_t((val >> 8) & 0xFF));
		if (len > 2) {
			out.write(uint8_t((val >> 16) & 0xFF));
			out.write(uint8_t((val >> 24) & 0xFF));
		}
	}
	return len;
}


/**
 * Encodes the given unsigned integer value with variable length.
 * 
 * @param[in,out] out - write encoded value using this object
 * @param[in] val - value to encode
 * @return encoded number of bytes
 * @see ::hid::detail::encodedSize()
 * @tparam Write - shell implement `write(uint8_t)`
 * @see HID 1.11 ch. 5.8
 */
template <typename Writer>
constexpr inline size_t encodeUnsigned(Writer & out, const uint32_t val) noexcept {
	return encodeValue(out, val, encodedSize(val));
}


/**
 * Encodes the given signed integer value with variable length.
 * 
 * @param[in,out] out - write encoded value using this object
 * @param[in] val - value to encode
 * @return encoded number of bytes
 * @see ::hid::detail::encodedSize()
 * @tparam Write - shell implement `write(uint8_t)`
 * @see HID 1.11 ch. 5.8
 */
template <typename Writer>
constexpr inline size_t encodeSigned(Writer & out, const int32_t val) noexcept {
	const int32_t absVal = (val < 0) ? -val - 1 : val;
	if (absVal > 0x7FFF) return encodeValue(out, uint32_t(val), 4);
	if (absVal > 0x7F) return encodeValue(out, uint32_t(int16_t(val)), 2);
	return encodeValue(out, uint32_t(int8_t(val)), 1);
}


/**
 * Usage Types.
 * 
 * @see HID 1.11 ch. 3.4
 **/
enum UsageType {
	UT_NONE = 0,
	/* Control, ch. 3.4.1 */
	UT_LC   = 1 << 0,  /**< Linear Control */
	UT_OOC  = 1 << 1,  /**< On/Off Control */
	UT_MC   = 1 << 2,  /**< Momentary Control */
	UT_OSC  = 1 << 3,  /**< One Shot Control */
	UT_RTC  = 1 << 4,  /**< Re-trigger Control */
	/* Data, ch. 3.4.2 */
	UT_SEL  = 1 << 5,  /**< Selector */
	UT_SV   = 1 << 6,  /**< Static Value */
	UT_SF   = 1 << 7,  /**< Static Flag */
	UT_DV   = 1 << 8,  /**< Dynamic Value */
	UT_DF   = 1 << 9,  /**< Dynamic Flag */
	/* Collection, ch. 3.4.3 */
	UT_NARY = 1 << 10, /**< Named Array */
	UT_CA   = 1 << 11, /**< Application Collection */
	UT_CL   = 1 << 12, /**< Logical Collection */
	UT_CP   = 1 << 13, /**< Physical Collection */
	UT_US   = 1 << 14, /**< Usage Switch */
	UT_UM   = 1 << 15, /**< Usage Modifier */
	/* others */
	UT_BB   = 1 << 16  /**< Buffered Bytes */
};


/**
 * Single HID descriptor element encoding.
 */
struct Encoding {
	const char * name; /**< token name */
	uint32_t value; /**< encoded value */
	uint32_t type; /**< usage type (in case of a usage type element) */
	const Encoding * const arg; /**< argument encoding map or NULL */
	/** Default constructor. */
	constexpr inline Encoding() noexcept:
		name{NULL},
		value{0},
		type{UT_NONE},
		arg{NULL}
	{}
	constexpr inline Encoding(const char * const n, const uint32_t v, const Encoding * const a = NULL) noexcept:
		name{n},
		value{v},
		type{UT_NONE},
		arg{a}
	{}
	constexpr inline Encoding(const char * const n, const uint32_t v, const uint32_t t) noexcept:
		name{n},
		value{v},
		type{t},
		arg{NULL}
	{}
};


/** Used to simplify end of map definition and check. */
constexpr const Encoding endOfMap;

/** Used to simplify unsigned numeric argument checks. */
constexpr const Encoding numArg[] = {endOfMap};

/** Used to simplify signed numeric argument checks. */
constexpr const Encoding signedNumArg[] = {endOfMap};

/** Used to simplify set/clear argument checks for input/output/feature items. */
constexpr const Encoding clearArg[] = {endOfMap};

/** Used to simplify usage argument item checks. */
constexpr const Encoding usageArg[] = {endOfMap};

/** Used to simplify end collection item checks. */
constexpr const Encoding endCol[] = {endOfMap};


/**
 * HID descriptor collection item argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.6
 */
constexpr const Encoding colArgMap[] = {
	{"Physical"     , 0x00},
	{"Application"  , 0x01},
	{"Logical"      , 0x02},
	{"Report"       , 0x03},
	{"NamedArray"   , 0x04},
	{"UsageSwitch"  , 0x05},
	{"UsageModifier", 0x06},
	endOfMap
};


/**
 * HID descriptor input item argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.5
 */
constexpr const Encoding inputArgMap[] = {
	{"Data" , 0x001, clearArg},
	{"Cnst" , 0x001},
	{"Ary"  , 0x002, clearArg},
	{"Var"  , 0x002},
	{"Abs"  , 0x004, clearArg},
	{"Rel"  , 0x004},
	{"NWarp", 0x008, clearArg},
	{"Warp" , 0x008},
	{"Lin"  , 0x010, clearArg},
	{"NLin" , 0x010},
	{"Prf"  , 0x020, clearArg},
	{"NPrf" , 0x020},
	{"NNull", 0x040, clearArg},
	{"Null" , 0x040},
	{"Bit"  , 0x100, clearArg},
	{"Buf"  , 0x100},
	endOfMap
};


/**
 * HID descriptor output/feature item argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.5
 */
constexpr const Encoding outputFeatureArgMap[] = {
	{"Data" , 0x001, clearArg},
	{"Cnst" , 0x001},
	{"Ary"  , 0x002, clearArg},
	{"Var"  , 0x002},
	{"Abs"  , 0x004, clearArg},
	{"Rel"  , 0x004},
	{"NWarp", 0x008, clearArg},
	{"Warp" , 0x008},
	{"Lin"  , 0x010, clearArg},
	{"NLin" , 0x010},
	{"Prf"  , 0x020, clearArg},
	{"NPrf" , 0x020},
	{"NNull", 0x040, clearArg},
	{"Null" , 0x040},
	{"NVol" , 0x080, clearArg},
	{"Vol"  , 0x080},
	{"Bit"  , 0x100, clearArg},
	{"Buf"  , 0x100},
	endOfMap
};


/**
 * HID descriptor unit exponent item argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.7
 */
constexpr const Encoding unitExpMap[] = {
	{"0",  0x0},
	{"1",  0x1},
	{"2",  0x2},
	{"3",  0x3},
	{"4",  0x4},
	{"5",  0x5},
	{"6",  0x6},
	{"7",  0x7},
	{"-8", 0x8},
	{"-7", 0x9},
	{"-6", 0xA},
	{"-5", 0xB},
	{"-4", 0xC},
	{"-3", 0xD},
	{"-2", 0xE},
	{"-1", 0xF},
	endOfMap
};


/**
 * HID descriptor unit item argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.7
 */
constexpr const Encoding unitMap[] = {
	{"Length",   1, unitExpMap},
	{"Mass",     2, unitExpMap},
	{"Time",     3, unitExpMap},
	{"Temp",     4, unitExpMap},
	{"Current",  5, unitExpMap},
	{"Luminous", 6, unitExpMap},
	endOfMap
};


/**
 * HID descriptor unit item system argument token encoding map.
 * The unit description is generalized by using the non-system names.
 * For their actual units please see the comments to the table below.
 * 
 * @see HID 1.11 ch. 6.2.2.7
 */
constexpr const Encoding unitSystemMap[] = {
	{"None",   0x00, unitMap}, /* Length,     Mass, Time,    Temp,       Current, Luminous*/
	{"SiLin",  0x01, unitMap}, /* Centimeter, Gram, Seconds, Kelvin,     Ampere,  Candela */
	{"SiRot",  0x02, unitMap}, /* Radians,    Gram, Seconds, Kelvin,     Ampere,  Candela */
	{"EngLin", 0x03, unitMap}, /* Inch,       Slug, Seconds, Fahrenheit, Ampere,  Candela */
	{"EngRot", 0x04, unitMap}, /* Degrees,    Slug, Seconds, Fahrenheit, Ampere,  Candela */
	endOfMap
};


/**
 * HID descriptor delimiter argument token encoding map.
 * 
 * @see HID 1.11 ch. 6.2.2.8
 */
constexpr const Encoding delimMap[] = {
	{"Close", 0x00},
	{"Open",  0x01},
	endOfMap
};


/**
 * HID descriptor usage generic desktop argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 4
 */
constexpr const Encoding genDeskMap[] = {
	{"Pointer"                              , 0x01, UT_CP},
	{"Mouse"                                , 0x02, UT_CA},
	{"Joystick"                             , 0x04, UT_CA},
	{"Gamepad"                              , 0x05, UT_CA},
	{"Keyboard"                             , 0x06, UT_CA},
	{"Keypad"                               , 0x07, UT_CA},
	{"MultiAxisController"                  , 0x08, UT_CA},
	{"TabletPcSystemControls"               , 0x09, UT_CA},
	{"WaterCoolingDevice"                   , 0x0A, UT_CA},
	{"ComputerChassisDevice"                , 0x0B, UT_CA},
	{"WirelessRadioControls"                , 0x0C, UT_CA},
	{"PortableDeviceControl"                , 0x0D, UT_CA},
	{"SystemMultiAxisController"            , 0x0E, UT_CA},
	{"SpatialController"                    , 0x0F, UT_CA},
	{"AssistiveControl"                     , 0x10, UT_CA},
	{"DeviceDock"                           , 0x11, UT_CA},
	{"DockableDevice"                       , 0x12, UT_CA},
	{"X"                                    , 0x30, UT_DV},
	{"Y"                                    , 0x31, UT_DV},
	{"Z"                                    , 0x32, UT_DV},
	{"Rx"                                   , 0x33, UT_DV},
	{"Ry"                                   , 0x34, UT_DV},
	{"Rz"                                   , 0x35, UT_DV},
	{"Slider"                               , 0x36, UT_DV},
	{"Dial"                                 , 0x37, UT_DV},
	{"Wheel"                                , 0x38, UT_DV},
	{"HatSwitch"                            , 0x39, UT_DV},
	{"CountedBuffer"                        , 0x3A, UT_CL},
	{"ByteCount"                            , 0x3B, UT_DV},
	{"MotionWakeup"                         , 0x3C, UT_OSC|UT_DF},
	{"Start"                                , 0x3D, UT_OOC},
	{"Select"                               , 0x3E, UT_OOC},
	{"Vx"                                   , 0x40, UT_DV},
	{"Vy"                                   , 0x41, UT_DV},
	{"Vz"                                   , 0x42, UT_DV},
	{"Vbrx"                                 , 0x43, UT_DV},
	{"Vbry"                                 , 0x44, UT_DV},
	{"Vbrz"                                 , 0x45, UT_DV},
	{"Vno"                                  , 0x46, UT_DV},
	{"FeatureNotification"                  , 0x47, UT_DV|UT_DF},
	{"ResolutionMultiplier"                 , 0x48, UT_DV},
	{"Qx"                                   , 0x49, UT_DV},
	{"Qy"                                   , 0x4A, UT_DV},
	{"Qz"                                   , 0x4B, UT_DV},
	{"Qw"                                   , 0x4C, UT_DV},
	{"SystemControl"                        , 0x80, UT_CA},
	{"SystemPowerDown"                      , 0x81, UT_OSC},
	{"SystemSleep"                          , 0x82, UT_OSC},
	{"SystemWakeUp"                         , 0x83, UT_OSC},
	{"SystemContextMenu"                    , 0x84, UT_OSC},
	{"SystemMainMenu"                       , 0x85, UT_OSC},
	{"SystemAppMenu"                        , 0x86, UT_OSC},
	{"SystemMenuHelp"                       , 0x87, UT_OSC},
	{"SystemMenuExit"                       , 0x88, UT_OSC},
	{"SystemMenuSelect"                     , 0x89, UT_OSC},
	{"SystemMenuRight"                      , 0x8A, UT_RTC},
	{"SystemMenuLeft"                       , 0x8B, UT_RTC},
	{"SystemMenuUp"                         , 0x8C, UT_RTC},
	{"SystemMenuDown"                       , 0x8D, UT_RTC},
	{"SystemColdRestart"                    , 0x8E, UT_OSC},
	{"SystemWarmRestart"                    , 0x8F, UT_OSC},
	{"DpadUp"                               , 0x90, UT_OOC},
	{"DpadDown"                             , 0x91, UT_OOC},
	{"DpadRight"                            , 0x92, UT_OOC},
	{"DpadLeft"                             , 0x93, UT_OOC},
	{"IndexTrigger"                         , 0x94, UT_MC|UT_DV},
	{"PalmTrigger"                          , 0x95, UT_MC|UT_DV},
	{"Thumbstick"                           , 0x96, UT_CP},
	{"SystemFunctionShift"                  , 0x97, UT_MC},
	{"SystemFunctionShiftLock"              , 0x98, UT_OOC},
	{"SystemFunctionShiftLockIndicator"     , 0x99, UT_DV},
	{"SystemDismissNotification"            , 0x9A, UT_OSC},
	{"SystemDoNotDisturb"                   , 0x9B, UT_OOC},
	{"SystemDock"                           , 0xA0, UT_OSC},
	{"SystemUndock"                         , 0xA1, UT_OSC},
	{"SystemSetup"                          , 0xA2, UT_OSC},
	{"SystemBreak"                          , 0xA3, UT_OSC},
	{"SystemDebuggerBreak"                  , 0xA4, UT_OSC},
	{"ApplicationBreak"                     , 0xA5, UT_OSC},
	{"ApplicationDebuggerBreak"             , 0xA6, UT_OSC},
	{"SystemSpeakerMute"                    , 0xA7, UT_OSC},
	{"SystemHibernate"                      , 0xA8, UT_OSC},
	{"SystemDisplayInvert"                  , 0xB0, UT_OSC},
	{"SystemDisplayInternal"                , 0xB1, UT_OSC},
	{"SystemDisplayExternal"                , 0xB2, UT_OSC},
	{"SystemDisplayBoth"                    , 0xB3, UT_OSC},
	{"SystemDisplayDual"                    , 0xB4, UT_OSC},
	{"SystemDisplayToggleIntExtMode"        , 0xB5, UT_OSC},
	{"SystemDisplaySwapPrimarySecondary"    , 0xB6, UT_OSC},
	{"SystemDisplayToggleLcdAutoscale"      , 0xB7, UT_OSC},
	{"SensorZone"                           , 0xC0, UT_CL},
	{"Rpm"                                  , 0xC1, UT_DV},
	{"CoolantLevel"                         , 0xC2, UT_DV},
	{"CoolantCriticalLevel"                 , 0xC3, UT_SV},
	{"CoolantPump"                          , 0xC4, UT_US},
	{"ChassisEnclosure"                     , 0xC5, UT_CL},
	{"WirelessRadioButton"                  , 0xC6, UT_OOC},
	{"WirelessRadioLed"                     , 0xC7, UT_OOC},
	{"WirelessRadioSliderSwitch"            , 0xC8, UT_OOC},
	{"SystemDisplayRotationLockButton"      , 0xC9, UT_OOC},
	{"SystemDisplayRotationLockSliderSwitch", 0xCA, UT_OOC},
	{"ControlEnable"                        , 0xCB, UT_DF},
	{"DockableDeviceUniqueId"               , 0xD0, UT_DV},
	{"DockableDeviceVendorId"               , 0xD1, UT_DV},
	{"DockableDevicePrimaryUsagePage"       , 0xD2, UT_DV},
	{"DockableDevicePrimaryUsageId"         , 0xD3, UT_DV},
	{"DockableDeviceDockingState"           , 0xD4, UT_DF},
	{"DockableDeviceDisplayOcclusion"       , 0xD5, UT_CL},
	{"DockableDeviceObjectType"             , 0xD6, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage simulation controls argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 5
 */
constexpr const Encoding simCtrlMap[] = {
	{"FlighSimulationDevice"      , 0x01, UT_CA},
	{"AutomobileSimulationDevice" , 0x02, UT_CA},
	{"TankSimulationDevice"       , 0x03, UT_CA},
	{"SpaceshipSimulationDevice"  , 0x04, UT_CA},
	{"SubmarineSimulationDevice"  , 0x05, UT_CA},
	{"SailingSimulationDevice"    , 0x06, UT_CA},
	{"MotorcycleSimiulationDevice", 0x07, UT_CA},
	{"SportsSimulationDevice"     , 0x08, UT_CA},
	{"AirplaneSimulationDevice"   , 0x09, UT_CA},
	{"HelicopterSimulationDevice" , 0x0A, UT_CA},
	{"MagicCarpetSimulationDevice", 0x0B, UT_CA},
	{"BicycleSimulationDevice"    , 0x0C, UT_CA},
	{"FlightControlStick"         , 0x20, UT_CA},
	{"FlightStick"                , 0x21, UT_CA},
	{"CyclicControl"              , 0x22, UT_CP},
	{"CyclicTrim"                 , 0x23, UT_CP},
	{"FlightYoke"                 , 0x24, UT_CA},
	{"TrackControl"               , 0x25, UT_CP},
	{"Aileron"                    , 0xB0, UT_DV},
	{"AileronTrim"                , 0xB1, UT_DV},
	{"AntiTorqueControl"          , 0xB2, UT_DV},
	{"AutopilotEnable"            , 0xB3, UT_OOC},
	{"ChaffRelease"               , 0xB4, UT_OSC},
	{"CollectiveControl"          , 0xB5, UT_DV},
	{"DiveBrake"                  , 0xB6, UT_DV},
	{"ElectronicCountermeasures"  , 0xB7, UT_OOC},
	{"Elevator"                   , 0xB8, UT_DV},
	{"ElevatorTrim"               , 0xB9, UT_DV},
	{"Rudder"                     , 0xBA, UT_DV},
	{"Throttle"                   , 0xBB, UT_DV},
	{"FlightCommunications"       , 0xBC, UT_OOC},
	{"FlareRelease"               , 0xBD, UT_OSC},
	{"LandingGear"                , 0xBE, UT_OOC},
	{"ToeBrake"                   , 0xBF, UT_DV},
	{"Trigger"                    , 0xC0, UT_MC},
	{"WeaponsArm"                 , 0xC1, UT_OOC},
	{"WeaponsSelect"              , 0xC2, UT_OSC},
	{"WingFlaps"                  , 0xC3, UT_DV},
	{"Accelerator"                , 0xC4, UT_DV},
	{"Brake"                      , 0xC5, UT_DV},
	{"Clutch"                     , 0xC6, UT_DV},
	{"Shifter"                    , 0xC7, UT_DV},
	{"Steering"                   , 0xC8, UT_DV},
	{"TurretDirection"            , 0xC9, UT_DV},
	{"BarrelElevation"            , 0xCA, UT_DV},
	{"DivePlane"                  , 0xCB, UT_DV},
	{"Ballast"                    , 0xCC, UT_DV},
	{"BicycleCrank"               , 0xCD, UT_DV},
	{"HandleBars"                 , 0xCE, UT_DV},
	{"FrontBrake"                 , 0xCF, UT_DV},
	{"RearBrake"                  , 0xD0, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage VR controls argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 6
 */
constexpr const Encoding vrCtrlMap[] = {
	{"Belt"              , 0x01, UT_CA},
	{"BodySuit"          , 0x02, UT_CA},
	{"Flexor"            , 0x03, UT_CP},
	{"Grove"             , 0x04, UT_CA},
	{"HeadTracker"       , 0x05, UT_CP},
	{"HeadMountedDisplay", 0x06, UT_CA},
	{"HandTracker"       , 0x07, UT_CA},
	{"Oculometer"        , 0x08, UT_CA},
	{"Vest"              , 0x09, UT_CA},
	{"AnimatronicDevice" , 0x0A, UT_CA},
	{"StereoEnable"      , 0x20, UT_OOC},
	{"DisplayEnable"     , 0x21, UT_OOC},
	endOfMap
};


/**
 * HID descriptor usage sport controls argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 7
 */
constexpr const Encoding sportCtrlMap[] = {
	{"BaseballBat"       , 0x01, UT_CA},
	{"GolfBat"           , 0x02, UT_CA},
	{"RowingMachine"     , 0x03, UT_CA},
	{"Treadmill"         , 0x04, UT_CA},
	{"Oar"               , 0x30, UT_DV},
	{"Slope"             , 0x31, UT_DV},
	{"Rate"              , 0x32, UT_DV},
	{"StickSpeed"        , 0x33, UT_DV},
	{"StickFaceAngle"    , 0x34, UT_DV},
	{"StickHeelToe"      , 0x35, UT_DV},
	{"StickFollowThrough", 0x36, UT_DV},
	{"StickTempo"        , 0x37, UT_DV},
	{"StickType"         , 0x38, UT_NARY},
	{"StickHeight"       , 0x39, UT_DV},
	{"Putter"            , 0x50, UT_SEL},
	{"Iron1"             , 0x51, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron2"             , 0x52, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron3"             , 0x53, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron4"             , 0x54, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron5"             , 0x55, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron6"             , 0x56, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron7"             , 0x57, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron8"             , 0x58, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron9"             , 0x59, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron10"            , 0x5A, UT_SEL}, /* changed name to avoid leading digit */
	{"Iron11"            , 0x5B, UT_SEL}, /* changed name to avoid leading digit */
	{"SandWedge"         , 0x5C, UT_SEL},
	{"LoftWedge"         , 0x5D, UT_SEL},
	{"PowerWedge"        , 0x5E, UT_SEL},
	{"Wood1"             , 0x5F, UT_SEL}, /* changed name to avoid leading digit */
	{"Wood3"             , 0x60, UT_SEL}, /* changed name to avoid leading digit */
	{"Wood5"             , 0x61, UT_SEL}, /* changed name to avoid leading digit */
	{"Wood7"             , 0x62, UT_SEL}, /* changed name to avoid leading digit */
	{"Wood9"             , 0x63, UT_SEL}, /* changed name to avoid leading digit */
	endOfMap
};


/**
 * HID descriptor usage game controls argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 8
 */
constexpr const Encoding gameCtrlMap[] = {
	{"3dGameController"    , 0x01, UT_CA},
	{"PinballDevice"       , 0x02, UT_CA},
	{"GunDevice"           , 0x03, UT_CA},
	{"PointOfView"         , 0x20, UT_CP},
	{"TurnRightLeft"       , 0x21, UT_DV},
	{"PitchForwardBackward", 0x22, UT_DV},
	{"RollRightLeft"       , 0x23, UT_DV},
	{"MoveRightLeft"       , 0x24, UT_DV},
	{"MoveForwardBackward" , 0x25, UT_DV},
	{"MoveUpDown"          , 0x26, UT_DV},
	{"LeanRightLeft"       , 0x27, UT_DV},
	{"LeanForwardBackward" , 0x28, UT_DV},
	{"HeightOfPov"         , 0x29, UT_DV},
	{"Flipper"             , 0x2A, UT_MC},
	{"SecondaryFlipper"    , 0x2B, UT_MC},
	{"Bump"                , 0x2C, UT_MC},
	{"NewGame"             , 0x2D, UT_OSC},
	{"ShootBall"           , 0x2E, UT_OSC},
	{"Player"              , 0x2F, UT_OSC},
	{"GunBolt"             , 0x30, UT_OOC},
	{"GunClip"             , 0x31, UT_OOC},
	{"GunSelector"         , 0x32, UT_NARY},
	{"GunSingleShot"       , 0x33, UT_SEL},
	{"GunBurst"            , 0x34, UT_SEL},
	{"GunAutomatic"        , 0x35, UT_SEL},
	{"GunSafety"           , 0x36, UT_OOC},
	{"GamepadFireJump"     , 0x37, UT_CL},
	{"GamepadTrigger"      , 0x39, UT_CL},
	{"FormFittingGamepad"  , 0x3A, UT_SF},
	endOfMap
};


/**
 * HID descriptor usage generic device controls argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 9
 */
constexpr const Encoding genDevCtrlMap[] = {
	{"BackgroundNonuserControls"   , 0x06, UT_CA},
	{"BatteryStrength"             , 0x20, UT_DV},
	{"WirelessChannel"             , 0x21, UT_DV},
	{"WirelessId"                  , 0x22, UT_DV},
	{"DiscoverWirelessControl"     , 0x23, UT_OSC},
	{"SecurityCodeCharacterEntered", 0x24, UT_OSC},
	{"SecurityCodeCharacterErased" , 0x25, UT_OSC},
	{"SecurityCodeCleared"         , 0x26, UT_OSC},
	{"SequenceId"                  , 0x27, UT_DV},
	{"SequenceIdReset"             , 0x28, UT_DF},
	{"RfSignalStrength"            , 0x29, UT_DV},
	{"SofwareVersion"              , 0x2A, UT_CL},
	{"ProtocolVersion"             , 0x2B, UT_CL},
	{"HardwareVersion"             , 0x2C, UT_CL},
	{"Major"                       , 0x2D, UT_SV},
	{"Minor"                       , 0x2E, UT_SV},
	{"Revision"                    , 0x2F, UT_SV},
	{"Handedness"                  , 0x30, UT_NARY},
	{"EitherHand"                  , 0x31, UT_SEL},
	{"LeftHand"                    , 0x32, UT_SEL},
	{"RightHand"                   , 0x33, UT_SEL},
	{"BothHands"                   , 0x34, UT_SEL},
	{"GripPoseOffset"              , 0x40, UT_CP},
	{"PointerPoseOffset"           , 0x41, UT_CP},
	endOfMap
};


/**
 * HID descriptor usage keyboard/keypad argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 10
 */
constexpr const Encoding keyboardMap[] = {
	{"NoEventIndicated"           , 0x00, UT_SEL},
	{"KeyboardErrorRollOver"      , 0x01, UT_SEL},
	{"KeyboardPostFail"           , 0x02, UT_SEL},
	{"KeyboardErrorUndefined"     , 0x03, UT_SEL},
	{"KeyboardA"                  , 0x04, UT_SEL},
	{"KeyboardB"                  , 0x05, UT_SEL},
	{"KeyboardC"                  , 0x06, UT_SEL},
	{"KeyboardD"                  , 0x07, UT_SEL},
	{"KeyboardE"                  , 0x08, UT_SEL},
	{"KeyboardF"                  , 0x09, UT_SEL},
	{"KeyboardG"                  , 0x0A, UT_SEL},
	{"KeyboardH"                  , 0x0B, UT_SEL},
	{"KeyboardI"                  , 0x0C, UT_SEL},
	{"KeyboardJ"                  , 0x0D, UT_SEL},
	{"KeyboardK"                  , 0x0E, UT_SEL},
	{"KeyboardL"                  , 0x0F, UT_SEL},
	{"KeyboardM"                  , 0x10, UT_SEL},
	{"KeyboardN"                  , 0x11, UT_SEL},
	{"KeyboardO"                  , 0x12, UT_SEL},
	{"KeyboardP"                  , 0x13, UT_SEL},
	{"KeyboardQ"                  , 0x14, UT_SEL},
	{"KeyboardR"                  , 0x15, UT_SEL},
	{"KeyboardS"                  , 0x16, UT_SEL},
	{"KeyboardT"                  , 0x17, UT_SEL},
	{"KeyboardU"                  , 0x18, UT_SEL},
	{"KeyboardV"                  , 0x19, UT_SEL},
	{"KeyboardW"                  , 0x1A, UT_SEL},
	{"KeyboardX"                  , 0x1B, UT_SEL},
	{"KeyboardY"                  , 0x1C, UT_SEL},
	{"KeyboardZ"                  , 0x1D, UT_SEL},
	{"Keyboard1"                  , 0x1E, UT_SEL},
	{"Keyboard2"                  , 0x1F, UT_SEL},
	{"Keyboard3"                  , 0x20, UT_SEL},
	{"Keyboard4"                  , 0x21, UT_SEL},
	{"Keyboard5"                  , 0x22, UT_SEL},
	{"Keyboard6"                  , 0x23, UT_SEL},
	{"Keyboard7"                  , 0x24, UT_SEL},
	{"Keyboard8"                  , 0x25, UT_SEL},
	{"Keyboard9"                  , 0x26, UT_SEL},
	{"Keyboard0"                  , 0x27, UT_SEL},
	{"KeyboardEnter"              , 0x28, UT_SEL},
	{"KeyboardEscape"             , 0x29, UT_SEL},
	{"KeyboardDelete"             , 0x2A, UT_SEL},
	{"KeyboardTab"                , 0x2B, UT_SEL},
	{"KeyboardSpacebar"           , 0x2C, UT_SEL},
	{"KeyboardMinus"              , 0x2D, UT_SEL},
	{"KeyboardEqual"              , 0x2E, UT_SEL},
	{"KeyboardCurlyBracketOpen"   , 0x2F, UT_SEL},
	{"KeyboardCurlyBracketClose"  , 0x30, UT_SEL},
	{"KeyboardBackslash"          , 0x31, UT_SEL},
	{"KeyboardNonUsHash"          , 0x32, UT_SEL},
	{"KeyboardColon"              , 0x33, UT_SEL},
	{"KeyboardApostrophe"         , 0x34, UT_SEL},
	{"KeyboardGraveAccentAndTilde", 0x35, UT_SEL},
	{"KeyboardComma"              , 0x36, UT_SEL},
	{"KeyboardPoint"              , 0x37, UT_SEL},
	{"KeyboardSlash"              , 0x38, UT_SEL},
	{"KeyboardCapsLock"           , 0x39, UT_SEL},
	{"KeyboardF1"                 , 0x3A, UT_SEL},
	{"KeyboardF2"                 , 0x3B, UT_SEL},
	{"KeyboardF3"                 , 0x3C, UT_SEL},
	{"KeyboardF4"                 , 0x3D, UT_SEL},
	{"KeyboardF5"                 , 0x3E, UT_SEL},
	{"KeyboardF6"                 , 0x3F, UT_SEL},
	{"KeyboardF7"                 , 0x40, UT_SEL},
	{"KeyboardF8"                 , 0x41, UT_SEL},
	{"KeyboardF9"                 , 0x42, UT_SEL},
	{"KeyboardF10"                , 0x43, UT_SEL},
	{"KeyboardF11"                , 0x44, UT_SEL},
	{"KeyboardF12"                , 0x45, UT_SEL},
	{"KeyboardPrintScreen"        , 0x46, UT_SEL},
	{"KeyboardScrollLock"         , 0x47, UT_SEL},
	{"KeyboardPause"              , 0x48, UT_SEL},
	{"KeyboardInsert"             , 0x49, UT_SEL},
	{"KeyboardHome"               , 0x4A, UT_SEL},
	{"KeyboardPageUp"             , 0x4B, UT_SEL},
	{"KeyboardDeleteForward"      , 0x4C, UT_SEL},
	{"KeyboardEnd"                , 0x4D, UT_SEL},
	{"KeyboardPageDown"           , 0x4E, UT_SEL},
	{"KeyboardRightArrow"         , 0x4F, UT_SEL},
	{"KeyboardLeftArrow"          , 0x50, UT_SEL},
	{"KeyboardDownArrow"          , 0x51, UT_SEL},
	{"KeyboardUpArrow"            , 0x52, UT_SEL},
	{"KeypadNumLockAndClear"      , 0x53, UT_SEL},
	{"KeypadDivide"               , 0x54, UT_SEL},
	{"KeypadMultiply"             , 0x55, UT_SEL},
	{"KeypadMinus"                , 0x56, UT_SEL},
	{"KeypadPlus"                 , 0x57, UT_SEL},
	{"KeypadEnter"                , 0x58, UT_SEL},
	{"Keypad1"                    , 0x59, UT_SEL},
	{"Keypad2"                    , 0x5A, UT_SEL},
	{"Keypad3"                    , 0x5B, UT_SEL},
	{"Keypad4"                    , 0x5C, UT_SEL},
	{"Keypad5"                    , 0x5D, UT_SEL},
	{"Keypad6"                    , 0x5E, UT_SEL},
	{"Keypad7"                    , 0x5F, UT_SEL},
	{"Keypad8"                    , 0x60, UT_SEL},
	{"Keypad9"                    , 0x61, UT_SEL},
	{"Keypad0"                    , 0x62, UT_SEL},
	{"KeypadPoint"                , 0x63, UT_SEL},
	{"KeyboardNonUsBackslash"     , 0x64, UT_SEL},
	{"KeyboardApplication"        , 0x65, UT_SEL},
	{"KeyboardPower"              , 0x66, UT_SEL},
	{"KeyboardEqual"              , 0x67, UT_SEL},
	{"KeyboardF13"                , 0x68, UT_SEL},
	{"KeyboardF14"                , 0x69, UT_SEL},
	{"KeyboardF15"                , 0x6A, UT_SEL},
	{"KeyboardF16"                , 0x6B, UT_SEL},
	{"KeyboardF17"                , 0x6C, UT_SEL},
	{"KeyboardF18"                , 0x6D, UT_SEL},
	{"KeyboardF19"                , 0x6E, UT_SEL},
	{"KeyboardF20"                , 0x6F, UT_SEL},
	{"KeyboardF21"                , 0x70, UT_SEL},
	{"KeyboardF22"                , 0x71, UT_SEL},
	{"KeyboardF23"                , 0x72, UT_SEL},
	{"KeyboardF24"                , 0x73, UT_SEL},
	{"KeyboardExecute"            , 0x74, UT_SEL},
	{"KeyboardHelp"               , 0x75, UT_SEL},
	{"KeyboardMenu"               , 0x76, UT_SEL},
	{"KeyboardSelect"             , 0x77, UT_SEL},
	{"KeyboardStop"               , 0x78, UT_SEL},
	{"KeyboardAgain"              , 0x79, UT_SEL},
	{"KeyboardUndo"               , 0x7A, UT_SEL},
	{"KeyboardCut"                , 0x7B, UT_SEL},
	{"KeyboardCopy"               , 0x7C, UT_SEL},
	{"KeyboardPaste"              , 0x7D, UT_SEL},
	{"KeyboardFind"               , 0x7E, UT_SEL},
	{"KeyboardMute"               , 0x7F, UT_SEL},
	{"KeyboardVolumeUp"           , 0x80, UT_SEL},
	{"KeyboardVolumeDown"         , 0x81, UT_SEL},
	{"KeyboardLockingCapsLock"    , 0x82, UT_SEL},
	{"KeyboardLockingNumLock"     , 0x83, UT_SEL},
	{"KeyboardLockingScrollLock"  , 0x84, UT_SEL},
	{"KeypadComma"                , 0x85, UT_SEL},
	{"KeypadEqual"                , 0x86, UT_SEL},
	{"KeyboardInternational1"     , 0x87, UT_SEL},
	{"KeyboardInternational2"     , 0x88, UT_SEL},
	{"KeyboardInternational3"     , 0x89, UT_SEL},
	{"KeyboardInternational4"     , 0x8A, UT_SEL},
	{"KeyboardInternational5"     , 0x8B, UT_SEL},
	{"KeyboardInternational6"     , 0x8C, UT_SEL},
	{"KeyboardInternational7"     , 0x8D, UT_SEL},
	{"KeyboardInternational8"     , 0x8E, UT_SEL},
	{"KeyboardInternational9"     , 0x8F, UT_SEL},
	{"KeyboardLang1"              , 0x90, UT_SEL},
	{"KeyboardLang2"              , 0x91, UT_SEL},
	{"KeyboardLang3"              , 0x92, UT_SEL},
	{"KeyboardLang4"              , 0x93, UT_SEL},
	{"KeyboardLang5"              , 0x94, UT_SEL},
	{"KeyboardLang6"              , 0x95, UT_SEL},
	{"KeyboardLang7"              , 0x96, UT_SEL},
	{"KeyboardLang8"              , 0x97, UT_SEL},
	{"KeyboardLang9"              , 0x98, UT_SEL},
	{"KeyboardAlternateErase"     , 0x99, UT_SEL},
	{"KeyboardSysReqAttention"    , 0x9A, UT_SEL},
	{"KeyboardCancel"             , 0x9B, UT_SEL},
	{"KeyboardClear"              , 0x9C, UT_SEL},
	{"KeyboardPrior"              , 0x9D, UT_SEL},
	{"KeyboardReturn"             , 0x9E, UT_SEL},
	{"KeyboardSeparator"          , 0x9F, UT_SEL},
	{"KeyboardOut"                , 0xA0, UT_SEL},
	{"KeyboardOper"               , 0xA1, UT_SEL},
	{"KeyboardClearAgain"         , 0xA2, UT_SEL},
	{"KeyboardCrSelProps"         , 0xA3, UT_SEL},
	{"KeyboardExSel"              , 0xA4, UT_SEL},
	{"Keypad00"                   , 0xB0, UT_SEL},
	{"Keypad000"                  , 0xB1, UT_SEL},
	{"ThausendsSeparator"         , 0xB2, UT_SEL},
	{"DecimalSeparator"           , 0xB3, UT_SEL},
	{"CurrencyUnit"               , 0xB4, UT_SEL},
	{"CurrencySubUnit"            , 0xB5, UT_SEL},
	{"KeypadBracketOpen"          , 0xB6, UT_SEL},
	{"KeypadBracketClose"         , 0xB7, UT_SEL},
	{"KeypadCurlyBracketOpen"     , 0xB8, UT_SEL},
	{"KeypadCurlyBracketClose"    , 0xB9, UT_SEL},
	{"KeypadTab"                  , 0xBA, UT_SEL},
	{"KeypadBackspace"            , 0xBB, UT_SEL},
	{"KeypadA"                    , 0xBC, UT_SEL},
	{"KeypadB"                    , 0xBD, UT_SEL},
	{"KeypadC"                    , 0xBE, UT_SEL},
	{"KeypadD"                    , 0xBF, UT_SEL},
	{"KeypadE"                    , 0xC0, UT_SEL},
	{"KeypadF"                    , 0xC1, UT_SEL},
	{"KeypadXor"                  , 0xC2, UT_SEL},
	{"KeypadCircumflex"           , 0xC3, UT_SEL},
	{"KeypadPercent"              , 0xC4, UT_SEL},
	{"KeypadLessThan"             , 0xC5, UT_SEL},
	{"KeypadGreaterThan"          , 0xC6, UT_SEL},
	{"KeypadAmpersand"            , 0xC7, UT_SEL},
	{"KeypadDoubleAmpersand"      , 0xC8, UT_SEL},
	{"KeypadVerticalBar"          , 0xC9, UT_SEL},
	{"KeypadDoubleVerticalBar"    , 0xCA, UT_SEL},
	{"KeypadColon"                , 0xCB, UT_SEL},
	{"KeypadHash"                 , 0xCC, UT_SEL},
	{"KeypadSpace"                , 0xCD, UT_SEL},
	{"KeypadAtSign"               , 0xCE, UT_SEL},
	{"KeypadExclamationMark"      , 0xCF, UT_SEL},
	{"KeypadMemoryStore"          , 0xD0, UT_SEL},
	{"KeypadMemoryRecall"         , 0xD1, UT_SEL},
	{"KeypadMemoryClear"          , 0xD2, UT_SEL},
	{"KeypadMemoryAdd"            , 0xD3, UT_SEL},
	{"KeypadMemorySubtract"       , 0xD4, UT_SEL},
	{"KeypadMemoryMultiply"       , 0xD5, UT_SEL},
	{"KeypadMemoryDivide"         , 0xD6, UT_SEL},
	{"KeypadPlusMinus"            , 0xD7, UT_SEL},
	{"KeypadClear"                , 0xD8, UT_SEL},
	{"KeypadClearEntry"           , 0xD9, UT_SEL},
	{"KeypadBinary"               , 0xDA, UT_SEL},
	{"KeypadOctal"                , 0xDB, UT_SEL},
	{"KeypadDecimal"              , 0xDC, UT_SEL},
	{"KeypadHexadecimal"          , 0xDD, UT_SEL},
	{"KeyboardLeftControl"        , 0xE0, UT_DV},
	{"KeyboardLeftShift"          , 0xE1, UT_DV},
	{"KeyboardLeftAlt"            , 0xE2, UT_DV},
	{"KeyboardLeftGui"            , 0xE3, UT_DV},
	{"KeyboardRightControl"       , 0xE4, UT_DV},
	{"KeyboardRightShift"         , 0xE5, UT_DV},
	{"KeyboardRightAlt"           , 0xE6, UT_DV},
	{"KeyboardRightGui"           , 0xE7, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage LED argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 11
 */
constexpr const Encoding ledMap[] = {
	{"NumLock"                , 0x01, UT_OOC},
	{"CapsLock"               , 0x02, UT_OOC},
	{"ScrollLock"             , 0x03, UT_OOC},
	{"Compose"                , 0x04, UT_OOC},
	{"Kana"                   , 0x05, UT_OOC},
	{"Power"                  , 0x06, UT_OOC},
	{"Shift"                  , 0x07, UT_OOC},
	{"DoNotDisturb"           , 0x08, UT_OOC},
	{"Mute"                   , 0x09, UT_OOC},
	{"ToneEnable"             , 0x0A, UT_OOC},
	{"HighCutFilter"          , 0x0B, UT_OOC},
	{"LowCutFitler"           , 0x0C, UT_OOC},
	{"EqualizerEnable"        , 0x0D, UT_OOC},
	{"SoundFieldOn"           , 0x0E, UT_OOC},
	{"SurroundOn"             , 0x0F, UT_OOC},
	{"Repeat"                 , 0x10, UT_OOC},
	{"Stereo"                 , 0x11, UT_OOC},
	{"SamplingRateDetect"     , 0x12, UT_OOC},
	{"Spinning"               , 0x13, UT_OOC},
	{"Cav"                    , 0x14, UT_OOC},
	{"Clv"                    , 0x15, UT_OOC},
	{"RecordingFormatDetect"  , 0x16, UT_OOC},
	{"OffHook"                , 0x17, UT_OOC},
	{"Ring"                   , 0x18, UT_OOC},
	{"MessageWaiting"         , 0x19, UT_OOC},
	{"DataMode"               , 0x1A, UT_OOC},
	{"BatteryOperation"       , 0x1B, UT_OOC},
	{"BatteryOk"              , 0x1C, UT_OOC},
	{"BatteryLow"             , 0x1D, UT_OOC},
	{"Speaker"                , 0x1E, UT_OOC},
	{"HeadSet"                , 0x1F, UT_OOC},
	{"Hold"                   , 0x20, UT_OOC},
	{"Microphone"             , 0x21, UT_OOC},
	{"Coverage"               , 0x22, UT_OOC},
	{"NightMode"              , 0x23, UT_OOC},
	{"SendCalls"              , 0x24, UT_OOC},
	{"CallPickup"             , 0x25, UT_OOC},
	{"Conference"             , 0x26, UT_OOC},
	{"Standby"                , 0x27, UT_OOC},
	{"CameraOn"               , 0x28, UT_OOC},
	{"CameraOff"              , 0x29, UT_OOC},
	{"OnLine"                 , 0x2A, UT_OOC},
	{"OffLine"                , 0x2B, UT_OOC},
	{"Busy"                   , 0x2C, UT_OOC},
	{"Ready"                  , 0x2D, UT_OOC},
	{"PaperOut"               , 0x2E, UT_OOC},
	{"PaperJam"               , 0x2F, UT_OOC},
	{"Remote"                 , 0x30, UT_OOC},
	{"Forward"                , 0x31, UT_OOC},
	{"Reverse"                , 0x32, UT_OOC},
	{"Stop"                   , 0x33, UT_OOC},
	{"Rewind"                 , 0x34, UT_OOC},
	{"FastForward"            , 0x35, UT_OOC},
	{"Play"                   , 0x36, UT_OOC},
	{"Pause"                  , 0x37, UT_OOC},
	{"Record"                 , 0x38, UT_OOC},
	{"Error"                  , 0x39, UT_OOC},
	{"UsageSelectedIndicator" , 0x3A, UT_US},
	{"UsageInUseIndicator"    , 0x3B, UT_US},
	{"UsageMultiModeIndicator", 0x3C, UT_UM},
	{"IndicatorOn"            , 0x3D, UT_SEL},
	{"IndicatorFlash"         , 0x3E, UT_SEL},
	{"IndicatorSlowBlink"     , 0x3F, UT_SEL},
	{"IndicatorFastBlink"     , 0x40, UT_SEL},
	{"IndicatorOff"           , 0x41, UT_SEL},
	{"FlashOnTime"            , 0x42, UT_DV},
	{"SlowBlinkOnTime"        , 0x43, UT_DV},
	{"SlowBlinkOffTime"       , 0x44, UT_DV},
	{"FastBlinkOnTime"        , 0x45, UT_DV},
	{"FastBlinkOffTime"       , 0x46, UT_DV},
	{"UsageIndicatorColor"    , 0x47, UT_UM},
	{"IndicatorRed"           , 0x48, UT_SEL},
	{"IndicatorGreen"         , 0x49, UT_SEL},
	{"IndicatorAmber"         , 0x4A, UT_SEL},
	{"GenericIndicator"       , 0x4B, UT_OOC},
	{"SystemSyspend"          , 0x4C, UT_OOC},
	{"ExternalPowerConnected" , 0x4D, UT_OOC},
	{"IndicatorBlue"          , 0x4E, UT_SEL},
	{"IndicatorOrange"        , 0x4F, UT_SEL},
	{"GoodStatus"             , 0x50, UT_OOC},
	{"WarningStatus"          , 0x51, UT_OOC},
	{"RgbLed"                 , 0x52, UT_CL},
	{"RedLedChannel"          , 0x53, UT_DV},
	{"BlueLedChannel"         , 0x54, UT_DV},
	{"GreenLedChannel"        , 0x55, UT_DV},
	{"LedIntensity"           , 0x56, UT_DV},
	{"PlayerIndicator"        , 0x60, UT_NARY},
	{"Player1"                , 0x61, UT_SEL},
	{"Player2"                , 0x62, UT_SEL},
	{"Player3"                , 0x63, UT_SEL},
	{"Player4"                , 0x64, UT_SEL},
	{"Player5"                , 0x65, UT_SEL},
	{"Player6"                , 0x66, UT_SEL},
	{"Player7"                , 0x67, UT_SEL},
	{"Player8"                , 0x68, UT_SEL},
	endOfMap
};


/**
 * HID descriptor usage button argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 12
 */
constexpr const Encoding buttonMap[] = {
	{"NoButtonPressed", 0x00, UT_SEL|UT_OOC|UT_MC|UT_OSC},
	{"Button#"        , 0x01, UT_SEL|UT_OOC|UT_MC|UT_OSC}, /* range start */
	{"Button#"        , 0xFFFF, UT_SEL|UT_OOC|UT_MC|UT_OSC}, /* range end */
	endOfMap
};


/**
 * HID descriptor usage ordinal argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 13
 */
constexpr const Encoding ordinalMap[] = {
	{"Instance#", 0x01, UT_UM}, /* range start */
	{"Instance#", 0xFFFF, UT_UM}, /* range end */
	endOfMap
};


/**
 * HID descriptor usage telephony device argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 14
 */
constexpr const Encoding telDevMap[] = {
	{"Phone"                   , 0x01, UT_CA},
	{"AnsweringMachine"        , 0x02, UT_CA},
	{"MessageControls"         , 0x03, UT_CL},
	{"Handset"                 , 0x04, UT_CL},
	{"Headset"                 , 0x05, UT_CL},
	{"TelephonyKeyPad"         , 0x06, UT_NARY},
	{"ProgrammableButton"      , 0x07, UT_NARY},
	{"HookSwitch"              , 0x20, UT_OOC},
	{"Flash"                   , 0x21, UT_MC},
	{"Feature"                 , 0x22, UT_OSC},
	{"Hold"                    , 0x23, UT_OOC},
	{"Radial"                  , 0x24, UT_OSC},
	{"Transfer"                , 0x25, UT_OSC},
	{"Drop"                    , 0x26, UT_OSC},
	{"Park"                    , 0x27, UT_OOC},
	{"ForwardCalls"            , 0x28, UT_OOC},
	{"AlternateFunction"       , 0x29, UT_MC},
	{"Line"                    , 0x2A, UT_OSC|UT_NARY},
	{"SpeakerPhone"            , 0x2B, UT_OOC},
	{"Conference"              , 0x2C, UT_OOC},
	{"RingEnable"              , 0x2D, UT_OOC},
	{"RingSelect"              , 0x2E, UT_OSC},
	{"PhoneMute"               , 0x2F, UT_OOC},
	{"CallerId"                , 0x30, UT_MC},
	{"Send"                    , 0x31, UT_OOC},
	{"SpeedDial"               , 0x50, UT_OSC},
	{"StoreNumber"             , 0x51, UT_OSC},
	{"RecallNumber"            , 0x52, UT_OSC},
	{"PhoneDirectory"          , 0x53, UT_OOC},
	{"VoiceMail"               , 0x70, UT_OOC},
	{"ScreenCalls"             , 0x71, UT_OOC},
	{"DoNotDisturb"            , 0x72, UT_OOC},
	{"Message"                 , 0x73, UT_OSC},
	{"AnswerOnOff"             , 0x74, UT_OOC},
	{"InsideDialTone"          , 0x90, UT_MC},
	{"OutsideDialTone"         , 0x91, UT_MC},
	{"InsideRingTone"          , 0x92, UT_MC},
	{"OutsideRingTone"         , 0x93, UT_MC},
	{"PriorityRingTone"        , 0x94, UT_MC},
	{"InsideRingback"          , 0x95, UT_MC},
	{"PriorityRingback"        , 0x96, UT_MC},
	{"LineBusyTone"            , 0x97, UT_MC},
	{"ReorderTone"             , 0x98, UT_MC},
	{"CallWaitingTone"         , 0x99, UT_MC},
	{"ConfirmationTone1"       , 0x9A, UT_MC},
	{"ConfirmationTone2"       , 0x9B, UT_MC},
	{"TonesOff"                , 0x9C, UT_OOC},
	{"OutsideRingback"         , 0x9D, UT_MC},
	{"Ringer"                  , 0x9E, UT_OOC},
	{"PhoneKey0"               , 0xB0, UT_SEL},
	{"PhoneKey1"               , 0xB1, UT_SEL},
	{"PhoneKey2"               , 0xB2, UT_SEL},
	{"PhoneKey3"               , 0xB3, UT_SEL},
	{"PhoneKey4"               , 0xB4, UT_SEL},
	{"PhoneKey5"               , 0xB5, UT_SEL},
	{"PhoneKey6"               , 0xB6, UT_SEL},
	{"PhoneKey7"               , 0xB7, UT_SEL},
	{"PhoneKey8"               , 0xB8, UT_SEL},
	{"PhoneKey9"               , 0xB9, UT_SEL},
	{"PhoneKeyStar"            , 0xBA, UT_SEL},
	{"PhoneKeyPound"           , 0xBB, UT_SEL},
	{"PhoneKeyA"               , 0xBC, UT_SEL},
	{"PhoneKeyB"               , 0xBD, UT_SEL},
	{"PhoneKeyC"               , 0xBE, UT_SEL},
	{"PhoneKeyD"               , 0xBF, UT_SEL},
	{"PhoneCallHistoryKey"     , 0xC0, UT_SEL},
	{"PhoneCallerIdKey"        , 0xC1, UT_SEL},
	{"PhoneSettingsKey"        , 0xC2, UT_SEL},
	{"HostControl"             , 0xF0, UT_OOC},
	{"HostAvailable"           , 0xF1, UT_OOC},
	{"HostCallActive"          , 0xF2, UT_OOC},
	{"ActivateHandsetAudio"    , 0xF3, UT_OOC},
	{"RingType"                , 0xF4, UT_NARY},
	{"RediablePhoneNumber"     , 0xF5, UT_OOC},
	{"StopRingTone"            , 0xF8, UT_SEL},
	{"PstnRingTone"            , 0xF9, UT_SEL},
	{"HostRingTone"            , 0xFA, UT_SEL},
	{"AlertSoundError"         , 0xFB, UT_SEL},
	{"AlertSoundConfirm"       , 0xFC, UT_SEL},
	{"AlertSoundNotification"  , 0xFD, UT_SEL},
	{"SilentRing"              , 0xFE, UT_SEL},
	{"EmailMessageWaiting"     , 0x108, UT_OOC},
	{"VoicemailMessageWaiting" , 0x109, UT_OOC},
	{"HostHold"                , 0x10A, UT_OOC},
	{"IncomingCallHistoryCount", 0x110, UT_DV},
	{"OutgoingCallHistoryCount", 0x111, UT_DV},
	{"IncomingCallHistory"     , 0x112, UT_CL},
	{"OutgoingCallHistory"     , 0x113, UT_CL},
	{"PhoneLocale"             , 0x114, UT_DV},
	{"PhoneTimeSecond"         , 0x140, UT_DV},
	{"PhoneTimeMinute"         , 0x141, UT_DV},
	{"PhoneTimeHour"           , 0x142, UT_DV},
	{"PhoneTimeDay"            , 0x143, UT_DV},
	{"PhoneTimeMonth"          , 0x144, UT_DV},
	{"PhoneTimeYear"           , 0x145, UT_DV},
	{"HandsetNickname"         , 0x146, UT_DV},
	{"AddressBookId"           , 0x147, UT_DV},
	{"CallDuration"            , 0x14A, UT_DV},
	{"DualModePhone"           , 0x14B, UT_CA},
	endOfMap
};


/**
 * HID descriptor usage consumer argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 15
 */
constexpr const Encoding consumerMap[] = {
	{"ConsumerControl"                       , 0x01, UT_CA},
	{"NumericKeyPad"                         , 0x02, UT_NARY},
	{"ProgrammableButtons"                   , 0x03, UT_NARY},
	{"Microphone"                            , 0x04, UT_CA},
	{"Headphone"                             , 0x05, UT_CA},
	{"GraphicEqualizer"                      , 0x06, UT_CA},
	{"Plus10"                                , 0x20, UT_OSC}, /* changed name to avoid leading digit */
	{"Plus100"                               , 0x21, UT_OSC}, /* changed name to avoid leading digit */
	{"AmPm"                                  , 0x22, UT_OSC},
	{"Power"                                 , 0x30, UT_OOC},
	{"Reset"                                 , 0x31, UT_OSC},
	{"Sleep"                                 , 0x32, UT_OSC},
	{"SleepAfter"                            , 0x33, UT_OSC},
	{"SleepMode"                             , 0x34, UT_RTC},
	{"Illumination"                          , 0x35, UT_OOC},
	{"FunctionButtons"                       , 0x36, UT_NARY},
	{"Menu"                                  , 0x40, UT_OOC},
	{"MenuPick"                              , 0x41, UT_OSC},
	{"MenuUp"                                , 0x42, UT_OSC},
	{"MenuDown"                              , 0x43, UT_OSC},
	{"MenuLeft"                              , 0x44, UT_OSC},
	{"MenuRight"                             , 0x45, UT_OSC},
	{"MenuEscape"                            , 0x46, UT_OSC},
	{"MenuValueIncrease"                     , 0x47, UT_OSC},
	{"MenuValueDecrease"                     , 0x48, UT_OSC},
	{"DataOnScreen"                          , 0x60, UT_OOC},
	{"ClosedCaption"                         , 0x61, UT_OOC},
	{"ClosedCaptionSelect"                   , 0x62, UT_OSC},
	{"VcrTv"                                 , 0x63, UT_OOC},
	{"BroadcastMode"                         , 0x64, UT_OSC},
	{"Snapshot"                              , 0x65, UT_OSC},
	{"Still"                                 , 0x66, UT_OSC},
	{"PictureInPictureToggle"                , 0x67, UT_OSC},
	{"PictureInPictureSwap"                  , 0x68, UT_OSC},
	{"RedMenuButton"                         , 0x69, UT_MC},
	{"GreenMenuButton"                       , 0x6A, UT_MC},
	{"BlueMenuButton"                        , 0x6B, UT_MC},
	{"YellowMenuButton"                      , 0x6C, UT_MC},
	{"Aspect"                                , 0x6D, UT_OSC},
	{"Mode3dSelect"                          , 0x6E, UT_OSC}, /* changed name to avoid leading digit */
	{"DisplayBrightnessIncrement"            , 0x6F, UT_RTC},
	{"DisplayBrightnessDecrement"            , 0x70, UT_RTC},
	{"DisplayBrightness"                     , 0x71, UT_LC},
	{"DisplayBacklightToggle"                , 0x72, UT_OOC},
	{"DisplaySetBrightnessToMinimum"         , 0x73, UT_OSC},
	{"DisplaySetBrightnessToMaximum"         , 0x74, UT_OSC},
	{"DisplaySetAutoBrightness"              , 0x75, UT_OOC},
	{"CameraAccessEnabled"                   , 0x76, UT_OOC},
	{"CameraAccessDisabled"                  , 0x77, UT_OOC},
	{"CameraAccessToggle"                    , 0x78, UT_OOC},
	{"KeyboardBrightnessIncrement"           , 0x79, UT_OSC},
	{"KeyboardBrightnessDecrement"           , 0x7A, UT_OSC},
	{"KeyboardBacklightSetLevel"             , 0x7B, UT_LC},
	{"KeyboardBacklightOoc"                  , 0x7C, UT_OOC},
	{"KeyboardBacklightSetMinimum"           , 0x7D, UT_OSC},
	{"KeyboardBacklightSetMaximum"           , 0x7E, UT_OSC},
	{"KeyboardBacklightAuto"                 , 0x7F, UT_OOC},
	{"Selection"                             , 0x80, UT_NARY},
	{"AssignSelection"                       , 0x81, UT_OSC},
	{"ModeStep"                              , 0x82, UT_OSC},
	{"RecallLast"                            , 0x83, UT_OSC},
	{"EnterChannel"                          , 0x84, UT_OSC},
	{"OrderMovie"                            , 0x85, UT_OSC},
	{"Channel"                               , 0x86, UT_LC},
	{"MediaSelection"                        , 0x87, UT_NARY},
	{"MediaSelectComputer"                   , 0x88, UT_SEL},
	{"MediaSelectTv"                         , 0x89, UT_SEL},
	{"MediaSelectWww"                        , 0x8A, UT_SEL},
	{"MediaSelectDvd"                        , 0x8B, UT_SEL},
	{"MediaSelectTelephone"                  , 0x8C, UT_SEL},
	{"MediaSelectProgramGuide"               , 0x8D, UT_SEL},
	{"MediaSelectVideoPhone"                 , 0x8E, UT_SEL},
	{"MediaSelectGames"                      , 0x8F, UT_SEL},
	{"MediaSelectMessages"                   , 0x90, UT_SEL},
	{"MediaSelectCd"                         , 0x91, UT_SEL},
	{"MediaSelectVcr"                        , 0x92, UT_SEL},
	{"MediaSelectTuner"                      , 0x93, UT_SEL},
	{"Quit"                                  , 0x94, UT_OSC},
	{"Help"                                  , 0x95, UT_OOC},
	{"MediaSelectTape"                       , 0x96, UT_SEL},
	{"MediaSelectCable"                      , 0x97, UT_SEL},
	{"MediaSelectSatellite"                  , 0x98, UT_SEL},
	{"MediaSelectSecurity"                   , 0x99, UT_SEL},
	{"MediaSelectHome"                       , 0x9A, UT_SEL},
	{"MediaSelectCall"                       , 0x9B, UT_SEL},
	{"ChannelIncrement"                      , 0x9C, UT_OSC},
	{"ChannelDecrement"                      , 0x9D, UT_OSC},
	{"MediaSelectSap"                        , 0x9E, UT_SEL},
	{"VcrPlus"                               , 0xA0, UT_OSC},
	{"Once"                                  , 0xA1, UT_OSC},
	{"Daily"                                 , 0xA2, UT_OSC},
	{"Weekly"                                , 0xA3, UT_OSC},
	{"Monthly"                               , 0xA4, UT_OSC},
	{"Play"                                  , 0xB0, UT_OOC},
	{"Pause"                                 , 0xB1, UT_OOC},
	{"Record"                                , 0xB2, UT_OOC},
	{"FastForward"                           , 0xB3, UT_OOC},
	{"Rewind"                                , 0xB4, UT_OOC},
	{"ScanNextTrack"                         , 0xB5, UT_OSC},
	{"ScanPreviousTrack"                     , 0xB6, UT_OSC},
	{"Stop"                                  , 0xB7, UT_OSC},
	{"Eject"                                 , 0xB8, UT_OSC},
	{"RandomPlay"                            , 0xB9, UT_OOC},
	{"SelectDisc"                            , 0xBA, UT_NARY},
	{"EnterDisc"                             , 0xBB, UT_MC},
	{"Repeat"                                , 0xBC, UT_OSC},
	{"Tracking"                              , 0xBD, UT_LC},
	{"TrackNormal"                           , 0xBE, UT_OSC},
	{"SlowTracking"                          , 0xBF, UT_LC},
	{"FrameForward"                          , 0xC0, UT_RTC},
	{"FrameBack"                             , 0xC1, UT_RTC},
	{"Mark"                                  , 0xC2, UT_OSC},
	{"ClearMark"                             , 0xC3, UT_OSC},
	{"RepeatFromMark"                        , 0xC4, UT_OOC},
	{"ReturnToMark"                          , 0xC5, UT_OSC},
	{"SearchMarkForward"                     , 0xC6, UT_OSC},
	{"SearchMarkBackwards"                   , 0xC7, UT_OSC},
	{"CounterReset"                          , 0xC8, UT_OSC},
	{"ShowCounter"                           , 0xC9, UT_OSC},
	{"TrackingIncrement"                     , 0xCA, UT_RTC},
	{"TrackingDecrement"                     , 0xCB, UT_RTC},
	{"StopEject"                             , 0xCC, UT_OSC},
	{"PlayPause"                             , 0xCD, UT_OSC},
	{"PlaySkip"                              , 0xCE, UT_OSC},
	{"VoiceCommand"                          , 0xCF, UT_OSC},
	{"InvokeCaptureInterface"                , 0xD0, UT_SEL},
	{"StartOrStopGameRecording"              , 0xD1, UT_SEL},
	{"HistoricalGameCapture"                 , 0xD2, UT_SEL},
	{"CaptureGameScreenshot"                 , 0xD3, UT_SEL},
	{"ShowOrHideRecordingIndicator"          , 0xD4, UT_SEL},
	{"StartOrStopMicrophoneCapture"          , 0xD5, UT_SEL},
	{"StartOrStopCameraCapture"              , 0xD6, UT_SEL},
	{"StartOrStopGameBroadcast"              , 0xD7, UT_SEL},
	{"Volume"                                , 0xE0, UT_LC},
	{"Balance"                               , 0xE1, UT_LC},
	{"Mute"                                  , 0xE2, UT_OOC},
	{"Bass"                                  , 0xE3, UT_LC},
	{"Treble"                                , 0xE4, UT_LC},
	{"BassBoost"                             , 0xE5, UT_OOC},
	{"SurroundMode"                          , 0xE6, UT_OSC},
	{"Loudness"                              , 0xE7, UT_OOC},
	{"Mpx"                                   , 0xE8, UT_OOC},
	{"VolumeIncrement"                       , 0xE9, UT_RTC},
	{"VolumeDecrement"                       , 0xEA, UT_RTC},
	{"SpeedSelect"                           , 0xF0, UT_OSC},
	{"PlaybackSpeed"                         , 0xF1, UT_NARY},
	{"StandardPlay"                          , 0xF2, UT_SEL},
	{"LongPlay"                              , 0xF3, UT_SEL},
	{"ExtendedPlay"                          , 0xF4, UT_SEL},
	{"Slow"                                  , 0xF5, UT_OSC},
	{"FanEnable"                             , 0x100, UT_OOC},
	{"FanSpeed"                              , 0x101, UT_LC},
	{"LightEnable"                           , 0x102, UT_OOC},
	{"LightIlluminationLevel"                , 0x103, UT_LC},
	{"ClimateControlEnable"                  , 0x104, UT_OOC},
	{"RoomTemperature"                       , 0x105, UT_LC},
	{"SecurityEnalbe"                        , 0x106, UT_OOC},
	{"FireAlarm"                             , 0x107, UT_OSC},
	{"PoliceAlarm"                           , 0x108, UT_OSC},
	{"Proximity"                             , 0x109, UT_LC},
	{"Motion"                                , 0x10A, UT_OSC},
	{"DuressAlarm"                           , 0x10B, UT_OSC},
	{"HoldupAlarm"                           , 0x10C, UT_OSC},
	{"MedicalAlarm"                          , 0x10D, UT_OSC},
	{"BalanceRight"                          , 0x150, UT_RTC},
	{"BalanceLeft"                           , 0x151, UT_RTC},
	{"BassIncrement"                         , 0x152, UT_RTC},
	{"BassDecrement"                         , 0x153, UT_RTC},
	{"TrebleIncrement"                       , 0x154, UT_RTC},
	{"TrebleDecrement"                       , 0x155, UT_RTC},
	{"SpeakerSystem"                         , 0x160, UT_CL},
	{"ChannelLeft"                           , 0x161, UT_CL},
	{"ChannelRight"                          , 0x162, UT_CL},
	{"ChannelCenter"                         , 0x163, UT_CL},
	{"ChannelFront"                          , 0x164, UT_CL},
	{"ChannelCenterFront"                    , 0x165, UT_CL},
	{"ChannelSide"                           , 0x166, UT_CL},
	{"ChannelSurround"                       , 0x167, UT_CL},
	{"ChannelLowFrequencyEnhancement"        , 0x168, UT_CL},
	{"ChannelTop"                            , 0x169, UT_CL},
	{"ChannelUnknown"                        , 0x16A, UT_CL},
	{"SubChannel"                            , 0x170, UT_LC},
	{"SubChannelIncrement"                   , 0x171, UT_OSC},
	{"SubChannelDecrement"                   , 0x172, UT_OSC},
	{"AlternateAudioIncrement"               , 0x173, UT_OSC},
	{"AlternateAudioDecrement"               , 0x174, UT_OSC},
	{"ApplicationLaunchButtons"              , 0x180, UT_NARY},
	{"AlLaunchButtonConfigurationTool"       , 0x181, UT_SEL},
	{"AlProgrammableButtonConfiguration"     , 0x182, UT_SEL},
	{"AlConsumerControlConfiguration"        , 0x183, UT_SEL},
	{"AlWordProcessor"                       , 0x184, UT_SEL},
	{"AlTextEditor"                          , 0x185, UT_SEL},
	{"AlSpreadsheet"                         , 0x186, UT_SEL},
	{"AlGraphicsEditor"                      , 0x187, UT_SEL},
	{"AlPresentationApp"                     , 0x188, UT_SEL},
	{"AlDatabaseApp"                         , 0x189, UT_SEL},
	{"AlEmailReader"                         , 0x18A, UT_SEL},
	{"AlNewsreader"                          , 0x18B, UT_SEL},
	{"AlVoicemail"                           , 0x18C, UT_SEL},
	{"AlContactsAddressBook"                 , 0x18D, UT_SEL},
	{"AlCalenderSchedule"                    , 0x18E, UT_SEL},
	{"AlTaskProjectManager"                  , 0x18F, UT_SEL},
	{"AlLogJournalTimecard"                  , 0x190, UT_SEL},
	{"AlCheckbookFinance"                    , 0x191, UT_SEL},
	{"AlCalculator"                          , 0x192, UT_SEL},
	{"AlAvCapturePlayback"                   , 0x193, UT_SEL},
	{"AlLocalMachineBrowser"                 , 0x194, UT_SEL},
	{"AlLanWanBrowser"                       , 0x195, UT_SEL},
	{"AlInternetBrowser"                     , 0x196, UT_SEL},
	{"AlRemoteNetworkingIspConnect"          , 0x197, UT_SEL},
	{"AlNetworkConference"                   , 0x198, UT_SEL},
	{"AlNetworkChat"                         , 0x199, UT_SEL},
	{"AlTelephonyDialer"                     , 0x19A, UT_SEL},
	{"AlLogon"                               , 0x19B, UT_SEL},
	{"AlLogoff"                              , 0x19C, UT_SEL},
	{"AlLogonLogoff"                         , 0x19D, UT_SEL},
	{"AlTerminalLockScreensaver"             , 0x19E, UT_SEL},
	{"AlControlPanel"                        , 0x19F, UT_SEL},
	{"AlCommandLineProcessorRun"             , 0x1A0, UT_SEL},
	{"AlProcessTaskManager"                  , 0x1A1, UT_SEL},
	{"AlSelectTaskApplication"               , 0x1A2, UT_SEL},
	{"AlNextTaskApplication"                 , 0x1A3, UT_SEL},
	{"AlPreviousTaskApplication"             , 0x1A4, UT_SEL},
	{"AlPreemptiveHaltTaskApplication"       , 0x1A5, UT_SEL},
	{"AlIntegratedHelpCenter"                , 0x1A6, UT_SEL},
	{"AlDocuments"                           , 0x1A7, UT_SEL},
	{"AlThesaurus"                           , 0x1A8, UT_SEL},
	{"AlDictionary"                          , 0x1A9, UT_SEL},
	{"AlDesktop"                             , 0x1AA, UT_SEL},
	{"AlSpellCheck"                          , 0x1AB, UT_SEL},
	{"AlGrammarCheck"                        , 0x1AC, UT_SEL},
	{"AlWirelessStatus"                      , 0x1AD, UT_SEL},
	{"AlKeyboardLayout"                      , 0x1AE, UT_SEL},
	{"AlVirusProtection"                     , 0x1AF, UT_SEL},
	{"AlEncryption"                          , 0x1B0, UT_SEL},
	{"AlScreenSaver"                         , 0x1B1, UT_SEL},
	{"AlAlarms"                              , 0x1B2, UT_SEL},
	{"AlClock"                               , 0x1B3, UT_SEL},
	{"AlFileBrowser"                         , 0x1B4, UT_SEL},
	{"AlPowerStatus"                         , 0x1B5, UT_SEL},
	{"AlImageBrowser"                        , 0x1B6, UT_SEL},
	{"AlAudioBrowser"                        , 0x1B7, UT_SEL},
	{"AlMovieBrowser"                        , 0x1B8, UT_SEL},
	{"AlDigitalRightsManager"                , 0x1B9, UT_SEL},
	{"AlDigitalWallet"                       , 0x1BA, UT_SEL},
	{"AlInstantMessaging"                    , 0x1BC, UT_SEL},
	{"AlOemFeatureTipsTutorialBrowser"       , 0x1BD, UT_SEL},
	{"AlOemHelp"                             , 0x1BE, UT_SEL},
	{"AlOnlineCommunity"                     , 0x1BF, UT_SEL},
	{"AlEntertainmentContentBrowser"         , 0x1C0, UT_SEL},
	{"AlOnlineShoppingBrowser"               , 0x1C1, UT_SEL},
	{"AlSmartCardInformationHelp"            , 0x1C2, UT_SEL},
	{"AlMarketMonitorFinanceBrowser"         , 0x1C3, UT_SEL},
	{"AlCustomizedCorporateNewsBrowser"      , 0x1C4, UT_SEL},
	{"AlOnlineActivityBrowser"               , 0x1C5, UT_SEL},
	{"AlResearchSearchBrowser"               , 0x1C6, UT_SEL},
	{"AlAudioPlayer"                         , 0x1C7, UT_SEL},
	{"AlMessageStatus"                       , 0x1C8, UT_SEL},
	{"AlContactSync"                         , 0x1C9, UT_SEL},
	{"AlNavigation"                          , 0x1CA, UT_SEL},
	{"AlContextAwareDesktopAssistant"        , 0x1CB, UT_SEL},
	{"GenericGuiApplicationControls"         , 0x200, UT_NARY},
	{"AcNew"                                 , 0x201, UT_SEL},
	{"AcOpen"                                , 0x202, UT_SEL},
	{"AcClose"                               , 0x203, UT_SEL},
	{"AcExit"                                , 0x204, UT_SEL},
	{"AcMaximize"                            , 0x205, UT_SEL},
	{"AcMinimize"                            , 0x206, UT_SEL},
	{"AcSave"                                , 0x207, UT_SEL},
	{"AcPrint"                               , 0x208, UT_SEL},
	{"AcProperties"                          , 0x209, UT_SEL},
	{"AcUndo"                                , 0x21A, UT_SEL},
	{"AcCopy"                                , 0x21B, UT_SEL},
	{"AcCut"                                 , 0x21C, UT_SEL},
	{"AcPaste"                               , 0x21D, UT_SEL},
	{"AcSelectAll"                           , 0x21E, UT_SEL},
	{"AcFind"                                , 0x21F, UT_SEL},
	{"AcFindAndReplace"                      , 0x220, UT_SEL},
	{"AcSearch"                              , 0x221, UT_SEL},
	{"AcGoTo"                                , 0x222, UT_SEL},
	{"AcHome"                                , 0x223, UT_SEL},
	{"AcBack"                                , 0x224, UT_SEL},
	{"AcForward"                             , 0x225, UT_SEL},
	{"AcStop"                                , 0x226, UT_SEL},
	{"AcRefresh"                             , 0x227, UT_SEL},
	{"AcPreviousLink"                        , 0x228, UT_SEL},
	{"AcNextLink"                            , 0x229, UT_SEL},
	{"AcBookmarks"                           , 0x22A, UT_SEL},
	{"AcHistory"                             , 0x22B, UT_SEL},
	{"AcSubscriptions"                       , 0x22C, UT_SEL},
	{"AcZoomIn"                              , 0x22D, UT_SEL},
	{"AcZoomOut"                             , 0x22E, UT_SEL},
	{"AcZoom"                                , 0x22F, UT_LC},
	{"AcFullScreenView"                      , 0x230, UT_SEL},
	{"AcNormalView"                          , 0x231, UT_SEL},
	{"AcViewToggle"                          , 0x232, UT_SEL},
	{"AcScrollUp"                            , 0x233, UT_SEL},
	{"AcScrollDown"                          , 0x234, UT_SEL},
	{"AcScroll"                              , 0x235, UT_LC},
	{"AcPanLeft"                             , 0x236, UT_SEL},
	{"AcPanRight"                            , 0x237, UT_SEL},
	{"AcPan"                                 , 0x238, UT_LC},
	{"AcNewWindow"                           , 0x239, UT_SEL},
	{"AcTileHorizontally"                    , 0x23A, UT_SEL},
	{"AcTileVertically"                      , 0x23B, UT_SEL},
	{"AcFormat"                              , 0x23C, UT_SEL},
	{"AcEdit"                                , 0x23D, UT_SEL},
	{"AcBold"                                , 0x23E, UT_SEL},
	{"AcItalics"                             , 0x23F, UT_SEL},
	{"AcUnderline"                           , 0x240, UT_SEL},
	{"AcStrikethrough"                       , 0x241, UT_SEL},
	{"AcSubscript"                           , 0x242, UT_SEL},
	{"AcSuperscript"                         , 0x243, UT_SEL},
	{"AcAllCaps"                             , 0x244, UT_SEL},
	{"AcRemote"                              , 0x245, UT_SEL},
	{"AcResize"                              , 0x246, UT_SEL},
	{"AcFlipHorizontal"                      , 0x247, UT_SEL},
	{"AcFlipVertical"                        , 0x248, UT_SEL},
	{"AcMirrorHorizontal"                    , 0x249, UT_SEL},
	{"AcMirrorVertical"                      , 0x24A, UT_SEL},
	{"AcFontSelect"                          , 0x24B, UT_SEL},
	{"AcFontColor"                           , 0x24C, UT_SEL},
	{"AcFontSize"                            , 0x24D, UT_SEL},
	{"AcJustifyLeft"                         , 0x24E, UT_SEL},
	{"AcJustifyCenterH"                      , 0x24F, UT_SEL},
	{"AcJustifyRight"                        , 0x250, UT_SEL},
	{"AcJustifyBlockH"                       , 0x251, UT_SEL},
	{"AcJustifyTop"                          , 0x252, UT_SEL},
	{"AcJustifyCenterV"                      , 0x253, UT_SEL},
	{"AcJustifyBottom"                       , 0x254, UT_SEL},
	{"AcJustifyBlockV"                       , 0x255, UT_SEL},
	{"AcIndentDecrease"                      , 0x256, UT_SEL},
	{"AcIndentIncrease"                      , 0x257, UT_SEL},
	{"AcNumberedList"                        , 0x258, UT_SEL},
	{"AcRestartNumbering"                    , 0x259, UT_SEL},
	{"AcBulletedList"                        , 0x25A, UT_SEL},
	{"AcPromote"                             , 0x25B, UT_SEL},
	{"AcDemote"                              , 0x25C, UT_SEL},
	{"AcYes"                                 , 0x25D, UT_SEL},
	{"AcNo"                                  , 0x25E, UT_SEL},
	{"AcCancel"                              , 0x25F, UT_SEL},
	{"AcCatalog"                             , 0x260, UT_SEL},
	{"AcBuyCheckout"                         , 0x261, UT_SEL},
	{"AcAddToChart"                          , 0x262, UT_SEL},
	{"AcExpand"                              , 0x263, UT_SEL},
	{"AcExpandAll"                           , 0x264, UT_SEL},
	{"AcCollapse"                            , 0x265, UT_SEL},
	{"AcCollapseAll"                         , 0x266, UT_SEL},
	{"AcPrintPreview"                        , 0x267, UT_SEL},
	{"AcPasteSpecial"                        , 0x268, UT_SEL},
	{"AcInsertMode"                          , 0x269, UT_SEL},
	{"AcDelete"                              , 0x26A, UT_SEL},
	{"AcLock"                                , 0x26B, UT_SEL},
	{"AcUnlock"                              , 0x26C, UT_SEL},
	{"AcProtect"                             , 0x26D, UT_SEL},
	{"AcUnprotect"                           , 0x26E, UT_SEL},
	{"AcAttachComment"                       , 0x26F, UT_SEL},
	{"AcDeleteComment"                       , 0x270, UT_SEL},
	{"AcViewComment"                         , 0x271, UT_SEL},
	{"AcSelectWord"                          , 0x272, UT_SEL},
	{"AcSelectSentence"                      , 0x273, UT_SEL},
	{"AcSelectPragraph"                      , 0x274, UT_SEL},
	{"AcSelectColumn"                        , 0x275, UT_SEL},
	{"AcSelectRow"                           , 0x276, UT_SEL},
	{"AcSelectTable"                         , 0x277, UT_SEL},
	{"AcSelectObject"                        , 0x278, UT_SEL},
	{"AcRedoRepeat"                          , 0x279, UT_SEL},
	{"AcSort"                                , 0x27A, UT_SEL},
	{"AcSortAscending"                       , 0x27B, UT_SEL},
	{"AcSortDescending"                      , 0x27C, UT_SEL},
	{"AcFilter"                              , 0x27D, UT_SEL},
	{"AcSetClock"                            , 0x27E, UT_SEL},
	{"AcViewClock"                           , 0x27F, UT_SEL},
	{"AcSelectTimeZone"                      , 0x280, UT_SEL},
	{"AcEditTimeZones"                       , 0x281, UT_SEL},
	{"AcSetAlarm"                            , 0x282, UT_SEL},
	{"AcClearAlarm"                          , 0x283, UT_SEL},
	{"AcSnoozeAlarm"                         , 0x284, UT_SEL},
	{"AcResetAlarm"                          , 0x285, UT_SEL},
	{"AcSynchronize"                         , 0x286, UT_SEL},
	{"AcSendReceive"                         , 0x287, UT_SEL},
	{"AcSendTo"                              , 0x288, UT_SEL},
	{"AcReply"                               , 0x289, UT_SEL},
	{"AcReplyAll"                            , 0x28A, UT_SEL},
	{"AcForwardMsg"                          , 0x28B, UT_SEL},
	{"AcSend"                                , 0x28C, UT_SEL},
	{"AcAttachFile"                          , 0x28D, UT_SEL},
	{"AcUpload"                              , 0x28E, UT_SEL},
	{"AcDownload"                            , 0x28F, UT_SEL},
	{"AcSetBoarders"                         , 0x290, UT_SEL},
	{"AcInsertRow"                           , 0x291, UT_SEL},
	{"AcInsertColumn"                        , 0x292, UT_SEL},
	{"AcInsertFile"                          , 0x293, UT_SEL},
	{"AcInsertPicture"                       , 0x294, UT_SEL},
	{"AcInsertObject"                        , 0x295, UT_SEL},
	{"AcInsertSymbol"                        , 0x296, UT_SEL},
	{"AcSaveAndClose"                        , 0x297, UT_SEL},
	{"AcRename"                              , 0x298, UT_SEL},
	{"AcMerge"                               , 0x299, UT_SEL},
	{"AcSplit"                               , 0x29A, UT_SEL},
	{"AcDistributeHorizontally"              , 0x29B, UT_SEL},
	{"AcDistributeVertically"                , 0x29C, UT_SEL},
	{"AcNextKeyboardLayoutSelect"            , 0x29D, UT_SEL},
	{"AcNavigateGuidance"                    , 0x29E, UT_SEL},
	{"AcDesktopShowAllWindows"               , 0x29F, UT_SEL},
	{"AcSoftKeyLeft"                         , 0x2A0, UT_SEL},
	{"AcSoftKeyRight"                        , 0x2A1, UT_SEL},
	{"AcDesktopShowAllApplications"          , 0x2A2, UT_SEL},
	{"AcIdleKeepAlive"                       , 0x2B0, UT_SEL},
	{"ExtendedKeyboardAttributesCollection"  , 0x2C0, UT_CL},
	{"KeyboardFormFactor"                    , 0x2C1, UT_SV},
	{"KeyboardKeyType"                       , 0x2C2, UT_SV},
	{"KeyboardPhysicalLayout"                , 0x2C3, UT_SV},
	{"VendorSpecificKeyboardPhysicalLayout"  , 0x2C4, UT_SV},
	{"KeyboardIetfLanguageTagIndex"          , 0x2C5, UT_SV},
	{"ImplementedKeyboardInputAssistControls", 0x2C6, UT_SV},
	{"KeyboardInputAssistPrevious"           , 0x2C7, UT_SEL},
	{"KeyboardInputAssistNext"               , 0x2C8, UT_SEL},
	{"KeyboardInputAssistPreviousGroup"      , 0x2C9, UT_SEL},
	{"KeyboardInputAssistNextGroup"          , 0x2CA, UT_SEL},
	{"KeyboardInputAssistAccept"             , 0x2CB, UT_SEL},
	{"KeyboardInputAssistCancel"             , 0x2CC, UT_SEL},
	{"PrivacyScreenToggle"                   , 0x2D0, UT_OOC},
	{"PrivacyScreenLevelDecrement"           , 0x2D1, UT_RTC},
	{"PrivacyScreenLevelIncrement"           , 0x2D2, UT_RTC},
	{"PrivacyScreenLevelMinimum"             , 0x2D3, UT_OSC},
	{"PrivacyScreenLevelMaximum"             , 0x2D4, UT_OSC},
	{"ContactEdited"                         , 0x500, UT_OOC},
	{"ContactAdded"                          , 0x501, UT_OOC},
	{"ContactRecordedActive"                 , 0x502, UT_OOC},
	{"ContactIndex"                          , 0x503, UT_DV},
	{"ContactNickname"                       , 0x504, UT_DV},
	{"ContactFirstName"                      , 0x505, UT_DV},
	{"ContactLastName"                       , 0x506, UT_DV},
	{"ContactFullName"                       , 0x507, UT_DV},
	{"ContactPhoneNumberPersonal"            , 0x508, UT_DV},
	{"ContactPhoneNumberBusiness"            , 0x509, UT_DV},
	{"ContactPhoneNumberMobile"              , 0x50A, UT_DV},
	{"ContactPhoneNumberPager"               , 0x50B, UT_DV},
	{"ContactPhoneNumberFax"                 , 0x50C, UT_DV},
	{"ContactPhoneNumberOther"               , 0x50D, UT_DV},
	{"ContactEmailPersonal"                  , 0x50E, UT_DV},
	{"ContactEmailBusiness"                  , 0x50F, UT_DV},
	{"ContactEmailOther"                     , 0x510, UT_DV},
	{"ContactEmailMain"                      , 0x511, UT_DV},
	{"ContactSpeedDialNumber"                , 0x512, UT_DV},
	{"ContactStatusFlag"                     , 0x513, UT_DV},
	{"ContactMisc"                           , 0x514, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage digitizers argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 16
 */
constexpr const Encoding digitizersMap[] = {
	{"Digitizer"                                , 0x01, UT_CA},
	{"Pen"                                      , 0x02, UT_CA},
	{"LightPen"                                 , 0x03, UT_CA},
	{"TouchScreen"                              , 0x04, UT_CA},
	{"TouchPad"                                 , 0x05, UT_CA},
	{"Whiteboard"                               , 0x06, UT_CA},
	{"CoordinateMeasuringMachine"               , 0x07, UT_CA},
	{"Digitizer3d"                              , 0x08, UT_CA}, /* changed name to avoid leading digit */
	{"StereoPlotter"                            , 0x09, UT_CA},
	{"ArticulatedArm"                           , 0x0A, UT_CA},
	{"Armature"                                 , 0x0B, UT_CA},
	{"MultiplePointDigitizer"                   , 0x0C, UT_CA},
	{"FreeSpaceWand"                            , 0x0D, UT_CA},
	{"DeviceConfiguration"                      , 0x0E, UT_CA},
	{"CapacitiveHeatMapDigitizer"               , 0x0F, UT_CA},
	{"Stylus"                                   , 0x20, UT_CA|UT_CL},
	{"Puck"                                     , 0x21, UT_CL},
	{"Finger"                                   , 0x22, UT_CL},
	{"DeviceSettings"                           , 0x23, UT_CL},
	{"CharacterGesture"                         , 0x24, UT_CL},
	{"TipPressure"                              , 0x30, UT_DV},
	{"BarrelPressure"                           , 0x31, UT_DV},
	{"InRange"                                  , 0x32, UT_MC},
	{"Touch"                                    , 0x33, UT_MC},
	{"Untouch"                                  , 0x34, UT_OSC},
	{"Tap"                                      , 0x35, UT_OSC},
	{"Quality"                                  , 0x36, UT_DV},
	{"DataValid"                                , 0x37, UT_MC},
	{"TransducerIndex"                          , 0x38, UT_DV},
	{"TabletFunctionKeys"                       , 0x39, UT_CL},
	{"ProgramChangeKeys"                        , 0x3A, UT_CL},
	{"BatteryStrength"                          , 0x3B, UT_DV},
	{"Invert"                                   , 0x3C, UT_MC},
	{"XTilt"                                    , 0x3D, UT_DV},
	{"YTilt"                                    , 0x3E, UT_DV},
	{"Azimuth"                                  , 0x3F, UT_DV},
	{"Altitude"                                 , 0x40, UT_DV},
	{"Twist"                                    , 0x41, UT_DV},
	{"TipSwitch"                                , 0x42, UT_MC},
	{"SecondaryTipSwitch"                       , 0x43, UT_MC},
	{"BarrelSwitch"                             , 0x44, UT_MC},
	{"Eraser"                                   , 0x45, UT_MC},
	{"TabletPick"                               , 0x46, UT_MC},
	{"TouchValid"                               , 0x47, UT_MC},
	{"Width"                                    , 0x48, UT_DV},
	{"Height"                                   , 0x49, UT_DV},
	{"ContactIdentifier"                        , 0x51, UT_DV},
	{"DeviceMode"                               , 0x52, UT_DV},
	{"DeviceIdentifier"                         , 0x53, UT_DV|UT_SV},
	{"ContactCount"                             , 0x54, UT_DV},
	{"ContactCountMaximum"                      , 0x55, UT_SV},
	{"ScanTime"                                 , 0x56, UT_DV},
	{"SurfaceSwitch"                            , 0x57, UT_DF},
	{"ButtonSwitch"                             , 0x58, UT_DF},
	{"PadType"                                  , 0x59, UT_SF},
	{"SecondaryBarrelSwitch"                    , 0x5A, UT_MC},
	{"TransducerSerialNumber"                   , 0x5B, UT_SV},
	{"PreferredColor"                           , 0x5C, UT_DV},
	{"PreferredColorIsLocked"                   , 0x5D, UT_MC},
	{"PreferredLineWidth"                       , 0x5E, UT_DV},
	{"PreferredLineWidthIsLocked"               , 0x5F, UT_MC},
	{"LatencyMode"                              , 0x60, UT_DF},
	{"GestureCharacterQuality"                  , 0x61, UT_DV},
	{"CharacterGestureDataLength"               , 0x62, UT_DV},
	{"CharacterGestureData"                     , 0x63, UT_DV},
	{"GestureCharacterEncoding"                 , 0x64, UT_NARY},
	{"Utf8CharacterGestureEncoding"             , 0x65, UT_SEL},
	{"Utf16LittleEndianCharacterGestureEncoding", 0x66, UT_SEL},
	{"Utf16BigEndianCharacterGestureEncoding"   , 0x67, UT_SEL},
	{"Utf32LittleEndianCharacterGestureEncoding", 0x68, UT_SEL},
	{"Utf32BigEndianCharacterGestureEncoding"   , 0x69, UT_SEL},
	{"CapacitiveHeatMapProtocolVendorId"        , 0x6A, UT_SV},
	{"CapacitiveHeatMapProtocolVersion"         , 0x6B, UT_SV},
	{"CapacitiveHeatMapFrameData"               , 0x6C, UT_DV},
	{"GestureCharacterEnable"                   , 0x6D, UT_DF},
	{"PreferredLineStyle"                       , 0x70, UT_NARY},
	{"PreferredLineStyleIsLocked"               , 0x71, UT_MC},
	{"Ink"                                      , 0x72, UT_SEL},
	{"Pencil"                                   , 0x73, UT_SEL},
	{"Highlighter"                              , 0x74, UT_SEL},
	{"ChiselMarker"                             , 0x75, UT_SEL},
	{"Brush"                                    , 0x76, UT_SEL},
	{"NoPreference"                             , 0x77, UT_SEL},
	{"DigitizerDiagnostic"                      , 0x80, UT_CL},
	{"DigitizerError"                           , 0x81, UT_NARY},
	{"ErrNormalStatus"                          , 0x82, UT_SEL},
	{"ErrTransducersExceeded"                   , 0x83, UT_SEL},
	{"ErrFullTransFeaturesUnavailable"          , 0x84, UT_SEL},
	{"ErrChargeLow"                             , 0x85, UT_SEL},
	{"TransducerSoftwareInfo"                   , 0x90, UT_CL},
	{"TransducerVendorId"                       , 0x91, UT_SV},
	{"TransducerProductId"                      , 0x92, UT_SV},
	{"DeviceSupportedProtocols"                 , 0x93, UT_NARY|UT_CL},
	{"TransducerSupportedProtocols"             , 0x94, UT_NARY|UT_CL},
	{"NoProtocol"                               , 0x95, UT_SEL},
	{"WacomAesProtocol"                         , 0x96, UT_SEL},
	{"UsiProtocol"                              , 0x97, UT_SEL},
	{"MicrosoftPenProtocol"                     , 0x98, UT_SEL},
	{"SupportedReportRates"                     , 0xA0, UT_SV|UT_CL},
	{"ReportRate"                               , 0xA1, UT_DV},
	{"TransducerConnected"                      , 0xA2, UT_SF},
	{"SwitchDisabled"                           , 0xA3, UT_SEL},
	{"SwitchUnimplemented"                      , 0xA4, UT_SEL},
	{"TransducerSwitches"                       , 0xA5, UT_SEL},
	endOfMap
};


/**
 * HID descriptor usage haptics argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 17
 */
constexpr const Encoding hapticsMap[] = {
	{"SimpleHapticController"      , 0x01, UT_CA|UT_CL},
	{"WaveformList"                , 0x10, UT_NARY},
	{"DurationList"                , 0x11, UT_NARY},
	{"AutoTrigger"                 , 0x20, UT_DV},
	{"ManualTrigger"               , 0x21, UT_DV},
	{"AutoTriggerAssociatedControl", 0x22, UT_SV},
	{"Intensity"                   , 0x23, UT_DV},
	{"RepeatCount"                 , 0x24, UT_DV},
	{"RetriggerPeriod"             , 0x25, UT_DV},
	{"WaveformVendorPage"          , 0x26, UT_SV},
	{"WaveformVendorId"            , 0x27, UT_SV},
	{"WaveformCutoffTime"          , 0x28, UT_SV},
	{"WaveformNone"                , 0x1001, UT_SV},
	{"WaveformStop"                , 0x1002, UT_SV},
	{"WaveformClick"               , 0x1003, UT_SV},
	{"WaveformBuzzContinuous"      , 0x1004, UT_SV},
	{"WaveformRumbleContinuous"    , 0x1005, UT_SV},
	{"WaveformPress"               , 0x1006, UT_SV},
	{"WaveformRelease"             , 0x1007, UT_SV},
	endOfMap
};


/**
 * HID descriptor usage PID argument token encoding map.
 * 
 * @see HID PID 1.0 ch. 5
 */
constexpr const Encoding pidMap[] = {
	{"PhysicalInterfaceDevice"     , 0x01, UT_CA},
	{"Normal"                      , 0x20, UT_DV},
	{"SetEffectReport"             , 0x21, UT_CL|UT_LC|UT_SV},
	{"EffectBlockIndex"            , 0x22, UT_DV},
	{"ParameterBlockOffset"        , 0x23, UT_DV},
	{"RomFlag"                     , 0x24, UT_DV},
	{"EffectType"                  , 0x25, UT_NARY},
	{"EtConstantForce"             , 0x26, UT_SEL},
	{"EtRamp"                      , 0x27, UT_SEL},
	{"EtCustomForceData"           , 0x28, UT_SEL},
	{"EtSquare"                    , 0x30, UT_SEL},
	{"EtSine"                      , 0x31, UT_SEL},
	{"EtTriangle"                  , 0x32, UT_SEL},
	{"EtSawtoothUp"                , 0x33, UT_SEL},
	{"EtSawtoothDown"              , 0x34, UT_SEL},
	{"EtSpring"                    , 0x40, UT_SEL},
	{"EtDamper"                    , 0x41, UT_SEL},
	{"EtInertia"                   , 0x42, UT_SEL},
	{"EtFriction"                  , 0x43, UT_SEL},
	{"Duration"                    , 0x50, UT_DV},
	{"SamplePeriod"                , 0x51, UT_DV},
	{"Gain"                        , 0x52, UT_DV},
	{"TriggerButton"               , 0x53, UT_DV},
	{"TriggerRepeatInterval"       , 0x54, UT_DV},
	{"AxesEnable"                  , 0x55, UT_US},
	{"DirectionEnable"             , 0x56, UT_DF},
	{"Direction"                   , 0x57, UT_CL|UT_DV},
	{"TypeSpecificBlockOffset"     , 0x58, UT_CL},
	{"BlockType"                   , 0x59, UT_NARY},
	{"SetEnvelopeReport"           , 0x5A, UT_CL|UT_LC|UT_SV},
	{"AttackLevel"                 , 0x5B, UT_DV},
	{"AttackTime"                  , 0x5C, UT_DV},
	{"FadeLevel"                   , 0x5D, UT_DV},
	{"FadeTime"                    , 0x5E, UT_DV},
	{"SetConditionReport"          , 0x5F, UT_CL|UT_LC|UT_SV},
	{"CpOffset"                    , 0x60, UT_DV},
	{"PositiveCoefficient"         , 0x61, UT_DV},
	{"NegativeCoefficient"         , 0x62, UT_DV},
	{"PositiveSaturation"          , 0x63, UT_DV},
	{"NegativeSaturation"          , 0x64, UT_DV},
	{"DeadBand"                    , 0x65, UT_DV},
	{"DownloadForceSample"         , 0x66, UT_CL},
	{"IsochCustomForceEnable"      , 0x67, UT_DF}, /* no clear usage type found in the standard */
	{"CustomForceDataReport"       , 0x68, UT_CL},
	{"CustomForceData"             , 0x69, UT_DV},
	{"CustomForceVendorDefinedData", 0x6A, UT_DV},
	{"SetCustomForceReport"        , 0x6B, UT_CL|UT_LC|UT_SV},
	{"CustomForceDataOffset"       , 0x6C, UT_DV},
	{"SampleCount"                 , 0x6D, UT_DV},
	{"SetPeriodicReport"           , 0x6E, UT_CL|UT_LC|UT_SV},
	{"Offset"                      , 0x6F, UT_DV},
	{"Magnitude"                   , 0x70, UT_DV},
	{"Phase"                       , 0x71, UT_DV},
	{"Period"                      , 0x72, UT_DV},
	{"SetConstantForceReport"      , 0x73, UT_CL|UT_LC|UT_SV},
	{"SetRampForceReport"          , 0x74, UT_CL|UT_LC|UT_SV},
	{"RampStart"                   , 0x75, UT_DV},
	{"RampEnd"                     , 0x76, UT_DV},
	{"EffectOperationReport"       , 0x77, UT_CL},
	{"EffectOperation"             , 0x78, UT_NARY},
	{"OpEffectStart"               , 0x79, UT_SEL},
	{"OpEffectStartSolo"           , 0x7A, UT_SEL},
	{"OpEffectStop"                , 0x7B, UT_SEL},
	{"LoopCount"                   , 0x7C, UT_DV},
	{"DeviceGainReport"            , 0x7D, UT_CL},
	{"DeviceGain"                  , 0x7E, UT_DV},
	{"PidPoolReport"               , 0x7F, UT_CL},
	{"RamPoolSize"                 , 0x80, UT_DV},
	{"RomPoolSize"                 , 0x81, UT_SV},
	{"RomEffectBlockCount"         , 0x82, UT_SV},
	{"SimultaneousEffectsMax"      , 0x83, UT_SV},
	{"PoolAlignment"               , 0x84, UT_SV},
	{"PidPoolMoveReport"           , 0x85, UT_CL},
	{"MoveSource"                  , 0x86, UT_DV},
	{"MoveDestination"             , 0x87, UT_DV},
	{"MoveLength"                  , 0x88, UT_DV},
	{"PidBlockLoadReport"          , 0x89, UT_CL},
	{"BlockLoadStatus"             , 0x8B, UT_NARY},
	{"BlockLoadSuccess"            , 0x8C, UT_SEL},
	{"BlockLoadFull"               , 0x8D, UT_SEL},
	{"BlockLoadError"              , 0x8E, UT_SEL},
	{"BlockHandle"                 , 0x8F, UT_DV}, /* no clear usage type found in the standard */
	{"PidBlockFreeReport"          , 0x90, UT_CL},
	{"TypeSpecificBlockHandle"     , 0x91, UT_CL},
	{"PidStateReport"              , 0x92, UT_CL},
	{"EffectPlaying"               , 0x94, UT_DF},
	{"PidDeviceControlReport"      , 0x95, UT_CL},
	{"PidDeviceControl"            , 0x96, UT_NARY},
	{"DcEnableActuators"           , 0x97, UT_SEL},
	{"DcDisableActuators"          , 0x98, UT_SEL},
	{"DcStopAllEffects"            , 0x99, UT_SEL},
	{"DcDeviceReset"               , 0x9A, UT_SEL},
	{"DcDevicePause"               , 0x9B, UT_SEL},
	{"DcDeviceContinue"            , 0x9C, UT_SEL},
	{"DevicePaused"                , 0x9F, UT_DF},
	{"ActuatorsEnabled"            , 0xA0, UT_DF},
	{"SafetySwitch"                , 0xA4, UT_DF},
	{"ActuatorOverrideSwitch"      , 0xA5, UT_DF},
	{"ActuatorPower"               , 0xA6, UT_OOC},
	{"StartDelay"                  , 0xA7, UT_DV},
	{"ParameterBlockSize"          , 0xA8, UT_CL},
	{"DeviceManagedPool"           , 0xA9, UT_SF},
	{"SharedParameterBlocks"       , 0xAA, UT_SF},
	{"CreateNewEffectReport"       , 0xAB, UT_CL},
	{"RamPoolAvailable"            , 0xAC, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage Unicode argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 18
 */
constexpr const Encoding unicodeMap[] = {
	{"Ucs#", 0x0000}, /* range start */
	{"Ucs#", 0xFFFF}, /* range end */
	endOfMap
};


/**
 * HID descriptor usage eye and head trackers argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 19
 */
constexpr const Encoding eyeHeadMap[] = {
	{"EyeTracker"              ,  0x01, UT_CA},
	{"HeadTracker"             ,  0x02, UT_CA},
	{"TrackingData"            ,  0x10, UT_CP},
	{"Capabilities"            ,  0x11, UT_CL},
	{"Configuration"           ,  0x12, UT_CL},
	{"Status"                  ,  0x13, UT_CL},
	{"Control"                 ,  0x14, UT_CL},
	{"SensorTimestamp"         ,  0x20, UT_DV},
	{"PositionX"               ,  0x21, UT_DV},
	{"PositionY"               ,  0x22, UT_DV},
	{"PositionZ"               ,  0x23, UT_DV},
	{"GazePoint"               ,  0x24, UT_CP},
	{"LeftEyePosition"         ,  0x25, UT_CP},
	{"RightEyePosition"        ,  0x26, UT_CP},
	{"HeadPosition"            ,  0x27, UT_CP},
	{"HeadDirectionPoint"      ,  0x28, UT_CP},
	{"RotationAboutXAxis"      ,  0x29, UT_DV},
	{"RotationAboutYAxis"      ,  0x2A, UT_DV},
	{"RotationAboutZAxis"      ,  0x2B, UT_DV},
	{"TrackerQuality"          , 0x100, UT_SV},
	{"MinimumTrackingDistance" , 0x101, UT_SV},
	{"OptimumTrackingDistance" , 0x102, UT_SV},
	{"MaximumTrackingDistance" , 0x103, UT_SV},
	{"MaximumScreenPlaneWidth" , 0x104, UT_SV},
	{"MaximumScreenPlaneHeight", 0x105, UT_SV},
	{"DisplayManufacturerId"   , 0x200, UT_SV},
	{"DisplayProductId"        , 0x201, UT_SV},
	{"DisplaySerialNumber"     , 0x202, UT_SV},
	{"DisplayManufacturerDate" , 0x203, UT_SV},
	{"CalibratedScreenWidth"   , 0x204, UT_SV},
	{"CalibratedScreenHeight"  , 0x205, UT_SV},
	{"SamplingFrequency"       , 0x300, UT_DV},
	{"ConfigurationStatus"     , 0x301, UT_DV},
	{"DeviceModeRequest"       , 0x400, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage auxiliary display argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 20
 */
constexpr const Encoding auxDisplayMap[] = {
	{"AlphanumericDisplay"       , 0x01, UT_CA},
	{"AuxiliaryDisplay"          , 0x02, UT_CA},
	{"DisplayAttributesReport"   , 0x20, UT_CL},
	{"AsciiCharacterSet"         , 0x21, UT_SF},
	{"DataReadBack"              , 0x22, UT_SF},
	{"FontReadBack"              , 0x23, UT_SF},
	{"DisplayControlReport"      , 0x24, UT_CL},
	{"ClearDisplay"              , 0x25, UT_DF},
	{"DisplayEnable"             , 0x26, UT_DF},
	{"ScreenSaverDelay"          , 0x27, UT_SV|UT_DV},
	{"ScreenSaverEnable"         , 0x28, UT_DF},
	{"VerticalScroll"            , 0x29, UT_SF|UT_DF},
	{"HorizontalScroll"          , 0x2A, UT_SF|UT_DF},
	{"CharacterReport"           , 0x2B, UT_CL},
	{"DisplayData"               , 0x2C, UT_DV},
	{"DisplayStatus"             , 0x2D, UT_CL},
	{"StatNotReady"              , 0x2E, UT_SEL},
	{"StatReady"                 , 0x2F, UT_SEL},
	{"ErrNotALoadableCharacter"  , 0x30, UT_SEL},
	{"ErrFontDataCannotBeRead"   , 0x31, UT_SEL},
	{"CursorPositionReport"      , 0x32, UT_SEL},
	{"Row"                       , 0x33, UT_DV},
	{"Column"                    , 0x34, UT_DV},
	{"Rows"                      , 0x35, UT_SV},
	{"Columns"                   , 0x36, UT_SV},
	{"CursorPixelPosition"       , 0x37, UT_SF},
	{"CursorMode"                , 0x38, UT_DF},
	{"CursorEnable"              , 0x39, UT_DF},
	{"CursorBlink"               , 0x3A, UT_DF},
	{"FontReport"                , 0x3B, UT_CL},
	{"FontData"                  , 0x3C, UT_BB},
	{"CharacterWidth"            , 0x3D, UT_SV},
	{"CharacterHeight"           , 0x3E, UT_SV},
	{"CharacterSpacingHorizontal", 0x3F, UT_SV},
	{"CharacterSpacingVertical"  , 0x40, UT_SV},
	{"UnicodeCharacterSet"       , 0x41, UT_SF},
	{"Font7Segment"              , 0x42, UT_SF},
	{"DirectMap7Segment"         , 0x43, UT_SF},
	{"Font14Segment"             , 0x44, UT_SF},
	{"DirectMap14Segment"        , 0x45, UT_SF},
	{"DisplayBrightness"         , 0x46, UT_DV},
	{"DisplayContrast"           , 0x47, UT_DV},
	{"CharacterAttribute"        , 0x48, UT_CL},
	{"AtributeReadback"          , 0x49, UT_SF},
	{"AttributeData"             , 0x4A, UT_DV},
	{"CharAttrEnhance"           , 0x4B, UT_OOC},
	{"CharAttrUnderline"         , 0x4C, UT_OOC},
	{"CharAttrBlink"             , 0x4D, UT_OOC},
	{"BitmapSizeX"               , 0x80, UT_SV},
	{"BitmapSizeY"               , 0x81, UT_SV},
	{"MaxBlitSize"               , 0x82, UT_SV},
	{"BitDepthFormat"            , 0x83, UT_SV},
	{"DisplayOrientation"        , 0x84, UT_DV},
	{"PaletteReport"             , 0x85, UT_CL},
	{"PaletteDataSize"           , 0x86, UT_SV},
	{"PaletteDataOffset"         , 0x87, UT_SV},
	{"PaletteData"               , 0x88, UT_BB},
	{"BlitReport"                , 0x8A, UT_CL},
	{"BlitRectangleX1"           , 0x8B, UT_SV},
	{"BlitRectangleY1"           , 0x8C, UT_SV},
	{"BlitRectangleX2"           , 0x8D, UT_SV},
	{"BlitRectangleY2"           , 0x8E, UT_SV},
	{"BlitData"                  , 0x8F, UT_BB},
	{"SoftButton"                , 0x90, UT_CL},
	{"SoftButtonId"              , 0x91, UT_SV},
	{"SoftButtonSide"            , 0x92, UT_SV},
	{"SoftButtonOffset1"         , 0x93, UT_SV},
	{"SoftButtonOffset2"         , 0x94, UT_SV},
	{"SoftButtonReport"          , 0x95, UT_SV},
	{"SoftKeys"                  , 0xC2, UT_SV},
	{"DisplayDataExtensions"     , 0xCC, UT_SF},
	{"CharacterMapping"          , 0xCF, UT_SV},
	{"UnicodeEquivalent"         , 0xDD, UT_SV},
	{"CharacterPageMapping"      , 0xDF, UT_SV},
	{"RequestReport"             , 0xFF, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage sensor argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 21
 */
constexpr const Encoding sensorMap[] = {
	{"Sensor"                                        , 0x01, UT_CA|UT_CP},
	{"Biometric"                                     , 0x10, UT_CA|UT_CP},
	{"BiometricHumanPresence"                        , 0x11, UT_CA|UT_CP},
	{"BiometricHumanProximity"                       , 0x12, UT_CA|UT_CP},
	{"BiometricHumanTouch"                           , 0x13, UT_CA|UT_CP},
	{"BiometricBloodPressure"                        , 0x14, UT_CA|UT_CP},
	{"BiometricBodyTemperature"                      , 0x15, UT_CA|UT_CP},
	{"BiometricHeartRate"                            , 0x16, UT_CA|UT_CP},
	{"BiometricHeartRateVariability"                 , 0x17, UT_CA|UT_CP},
	{"BiometricPeripheralOxygenSaturation"           , 0x18, UT_CA|UT_CP},
	{"BiometricRespiratoryRate"                      , 0x19, UT_CA|UT_CP},
	{"Electrical"                                    , 0x20, UT_CA|UT_CP},
	{"ElectricalCapacitance"                         , 0x21, UT_CA|UT_CP},
	{"ElectricalCurrent"                             , 0x22, UT_CA|UT_CP},
	{"ElectricalPower"                               , 0x23, UT_CA|UT_CP},
	{"ElectricalInductance"                          , 0x24, UT_CA|UT_CP},
	{"ElectricalResistance"                          , 0x25, UT_CA|UT_CP},
	{"ElectricalVoltage"                             , 0x26, UT_CA|UT_CP},
	{"ElectricalPotentiometer"                       , 0x27, UT_CA|UT_CP},
	{"ElectricalFrequency"                           , 0x28, UT_CA|UT_CP},
	{"ElectricalPeriod"                              , 0x29, UT_CA|UT_CP},
	{"Environmental"                                 , 0x30, UT_CA|UT_CP},
	{"EnvironmentalAtmosphericPressure"              , 0x31, UT_CA|UT_CP},
	{"EnvironmentalHumidity"                         , 0x32, UT_CA|UT_CP},
	{"EnvironmentalTemperature"                      , 0x33, UT_CA|UT_CP},
	{"EnvironmentalWindDirection"                    , 0x34, UT_CA|UT_CP},
	{"EnvironmentalWindSpeed"                        , 0x35, UT_CA|UT_CP},
	{"EnvironmentalAirQuality"                       , 0x36, UT_CA|UT_CP},
	{"EnvironmentalHeatIndex"                        , 0x37, UT_CA|UT_CP},
	{"EnvironmentalSurfaceTemperature"               , 0x38, UT_CA|UT_CP},
	{"EnvironmentalVolatileOrganicCompounds"         , 0x39, UT_CA|UT_CP},
	{"EnvironmentalObjectPresence"                   , 0x3A, UT_CA|UT_CP},
	{"EnvironmentalObjectProximity"                  , 0x3B, UT_CA|UT_CP},
	{"Light"                                         , 0x40, UT_CA|UT_CP},
	{"LightAmbientLight"                             , 0x41, UT_CA|UT_CP},
	{"LightConsumerInfrared"                         , 0x42, UT_CA|UT_CP},
	{"LightInfraredLight"                            , 0x43, UT_CA|UT_CP},
	{"LightVisibleLight"                             , 0x44, UT_CA|UT_CP},
	{"LightUltravioletLight"                         , 0x45, UT_CA|UT_CP},
	{"Location"                                      , 0x50, UT_CA|UT_CP},
	{"LocationBroadcast"                             , 0x51, UT_CA|UT_CP},
	{"LocationDeadReckoning"                         , 0x52, UT_CA|UT_CP},
	{"LocationGps"                                   , 0x53, UT_CA|UT_CP},
	{"LocationLookup"                                , 0x54, UT_CA|UT_CP},
	{"LocationOther"                                 , 0x55, UT_CA|UT_CP},
	{"LocationStatic"                                , 0x56, UT_CA|UT_CP},
	{"LocationTriangulation"                         , 0x57, UT_CA|UT_CP},
	{"Mechanical"                                    , 0x60, UT_CA|UT_CP},
	{"MechanicalBooleanSwitch"                       , 0x61, UT_CA|UT_CP},
	{"MechanicalBooleanSwitchArray"                  , 0x62, UT_CA|UT_CP},
	{"MechanicalMultivalueSwitch"                    , 0x63, UT_CA|UT_CP},
	{"MechanicalForce"                               , 0x64, UT_CA|UT_CP},
	{"MechanicalPressure"                            , 0x65, UT_CA|UT_CP},
	{"MechanicalStrain"                              , 0x66, UT_CA|UT_CP},
	{"MechanicalWeight"                              , 0x67, UT_CA|UT_CP},
	{"MechanicalHapticVibrator"                      , 0x68, UT_CA|UT_CP},
	{"MechanicalHallEffectSwitch"                    , 0x69, UT_CA|UT_CP},
	{"Motion"                                        , 0x70, UT_CA|UT_CP},
	{"MotionAccelerometer1d"                         , 0x71, UT_CA|UT_CP},
	{"MotionAccelerometer2d"                         , 0x72, UT_CA|UT_CP},
	{"MotionAccelerometer3d"                         , 0x73, UT_CA|UT_CP},
	{"MotionGyrometer1d"                             , 0x74, UT_CA|UT_CP},
	{"MotionGyrometer2d"                             , 0x75, UT_CA|UT_CP},
	{"MotionGyrometer3d"                             , 0x76, UT_CA|UT_CP},
	{"MotionMotionDetector"                          , 0x77, UT_CA|UT_CP},
	{"MotionSpeedometer"                             , 0x78, UT_CA|UT_CP},
	{"MotionAccelerometer"                           , 0x79, UT_CA|UT_CP},
	{"MotionGyrometer"                               , 0x7A, UT_CA|UT_CP},
	{"MotionGraviyVector"                            , 0x7B, UT_CA|UT_CP},
	{"MotionLinearAccelerometer"                     , 0x7C, UT_CA|UT_CP},
	{"Orientation"                                   , 0x80, UT_CA|UT_CP},
	{"OrientationCompass1d"                          , 0x81, UT_CA|UT_CP},
	{"OrientationCompass2d"                          , 0x82, UT_CA|UT_CP},
	{"OrientationCompass3d"                          , 0x83, UT_CA|UT_CP},
	{"OrientationInclinometer1d"                     , 0x84, UT_CA|UT_CP},
	{"OrientationInclinometer2d"                     , 0x85, UT_CA|UT_CP},
	{"OrientationInclinometer3d"                     , 0x86, UT_CA|UT_CP},
	{"OrientationDistance1d"                         , 0x87, UT_CA|UT_CP},
	{"OrientationDistance2d"                         , 0x88, UT_CA|UT_CP},
	{"OrientationDistance3d"                         , 0x89, UT_CA|UT_CP},
	{"OrientationDeviceOrientation"                  , 0x8A, UT_CA|UT_CP},
	{"OrientationCompass"                            , 0x8B, UT_CA|UT_CP},
	{"OrientationInclinometer"                       , 0x8C, UT_CA|UT_CP},
	{"OrientationDistance"                           , 0x8D, UT_CA|UT_CP},
	{"OrientationRelativeOrientation"                , 0x8E, UT_CA|UT_CP},
	{"OrientationSimpleOrientation"                  , 0x8F, UT_CA|UT_CP},
	{"Scanner"                                       , 0x90, UT_CA|UT_CP},
	{"ScannerBarcode"                                , 0x91, UT_CA|UT_CP},
	{"ScannerRfid"                                   , 0x92, UT_CA|UT_CP},
	{"ScannerNfc"                                    , 0x93, UT_CA|UT_CP},
	{"Time"                                          , 0xA0, UT_CA|UT_CP},
	{"TimeAlarmTimer"                                , 0xA1, UT_CA|UT_CP},
	{"TimeRealTimeClock"                             , 0xA2, UT_CA|UT_CP},
	{"PersonalActivity"                              , 0xB0, UT_CA|UT_CP},
	{"PersonalActivityActivityDetection"             , 0xB1, UT_CA|UT_CP},
	{"PersonalActivityDevicePosition"                , 0xB2, UT_CA|UT_CP},
	{"PersonalActivityPedometer"                     , 0xB3, UT_CA|UT_CP},
	{"PersonalActivityStepDetection"                 , 0xB4, UT_CA|UT_CP},
	{"OrientationExtended"                           , 0xC0, UT_CA|UT_CP},
	{"OrientationExtendedGeomagneticOrientation"     , 0xC1, UT_CA|UT_CP},
	{"OrientationExtendedMagnetometer"               , 0xC2, UT_CA|UT_CP},
	{"Gesture"                                       , 0xD0, UT_CA|UT_CP},
	{"GestureChassisFlipGesture"                     , 0xD1, UT_CA|UT_CP},
	{"GestureHingeFoldGesture"                       , 0xD2, UT_CA|UT_CP},
	{"Other"                                         , 0xE0, UT_CA|UT_CP},
	{"OtherCustom"                                   , 0xE1, UT_CA|UT_CP},
	{"OtherGeneric"                                  , 0xE2, UT_CA|UT_CP},
	{"OtherGenericEnumerator"                        , 0xE3, UT_CA|UT_CP},
	{"OtherHingeAngle"                               , 0xE4, UT_CA|UT_CP},
	{"VendorReserved1"                               , 0xF0, UT_CA|UT_CP},
	{"VendorReserved2"                               , 0xF1, UT_CA|UT_CP},
	{"VendorReserved3"                               , 0xF2, UT_CA|UT_CP},
	{"VendorReserved4"                               , 0xF3, UT_CA|UT_CP},
	{"VendorReserved5"                               , 0xF4, UT_CA|UT_CP},
	{"VendorReserved6"                               , 0xF5, UT_CA|UT_CP},
	{"VendorReserved7"                               , 0xF6, UT_CA|UT_CP},
	{"VendorReserved8"                               , 0xF7, UT_CA|UT_CP},
	{"VendorReserved9"                               , 0xF8, UT_CA|UT_CP},
	{"VendorReserved10"                              , 0xF9, UT_CA|UT_CP},
	{"VendorReserved11"                              , 0xFA, UT_CA|UT_CP},
	{"VendorReserved12"                              , 0xFB, UT_CA|UT_CP},
	{"VendorReserved13"                              , 0xFC, UT_CA|UT_CP},
	{"VendorReserved14"                              , 0xFD, UT_CA|UT_CP},
	{"VendorReserved15"                              , 0xFE, UT_CA|UT_CP},
	{"VendorReserved16"                              , 0xFF, UT_CA|UT_CP},
	{"Event"                                         , 0x200, UT_DV},
	{"EventSensorState"                              , 0x201, UT_NARY},
	{"EventSensorEvent"                              , 0x202, UT_NARY},
	{"Property"                                      , 0x300, UT_DV},
	{"PropertyFriendlyName"                          , 0x301, UT_SV},
	{"PropertyPersistentUniqueId"                    , 0x302, UT_DV},
	{"PropertySensorStatus"                          , 0x303, UT_DV},
	{"PropertyMinimumReportInterval"                 , 0x304, UT_SV},
	{"PropertySensorManufacturer"                    , 0x305, UT_SV},
	{"PropertySensorModel"                           , 0x306, UT_SV},
	{"PropertySensorSerialNumber"                    , 0x307, UT_SV},
	{"PropertySensorDescription"                     , 0x308, UT_SV},
	{"PropertySensorConnectionType"                  , 0x309, UT_NARY},
	{"PropertySensorDevicePath"                      , 0x30A, UT_DV},
	{"PropertyHardwareRevision"                      , 0x30B, UT_SV},
	{"PropertyFirmwareVersion"                       , 0x30C, UT_SV},
	{"PropertyReleaseDate"                           , 0x30D, UT_SV},
	{"PropertyReportInterval"                        , 0x30E, UT_DV},
	{"PropertyChangeSensitivityAbsolute"             , 0x30F, UT_DV},
	{"PropertyChangeSensitivityPercentOfRange"       , 0x310, UT_DV},
	{"PropertyChangeSensitivityPercentRelative"      , 0x311, UT_DV},
	{"PropertyAccuracy"                              , 0x312, UT_DV},
	{"PropertyResolution"                            , 0x313, UT_DV},
	{"PropertyMaximum"                               , 0x314, UT_DV},
	{"PropertyMinimum"                               , 0x315, UT_DV},
	{"PropertyReportingState"                        , 0x316, UT_NARY},
	{"PropertySamplingRate"                          , 0x317, UT_DV},
	{"PropertyResponseCurve"                         , 0x318, UT_DV},
	{"PropertyPowerState"                            , 0x319, UT_NARY},
	{"PropertyMaximumFifoEvents"                     , 0x31A, UT_SV},
	{"PropertyReportLatency"                         , 0x31B, UT_DV},
	{"PropertyFlushFifoEvents"                       , 0x31C, UT_DF},
	{"PropertyMaximumPowerConsumption"               , 0x31D, UT_DV},
	{"PropertyIsPrimary"                             , 0x31E, UT_DF},
	{"DataFieldLocation"                             , 0x400, UT_DV},
	{"DataFieldAltitudeAntennaSeaLevel"              , 0x402, UT_SV},
	{"DataFieldDifferentialReferenceStationId"       , 0x403, UT_SV},
	{"DataFieldAltitudeEllipsoidError"               , 0x404, UT_SV},
	{"DataFieldAltitudeEllipsoid"                    , 0x405, UT_SV},
	{"DataFieldAltitudeSeaLevelError"                , 0x406, UT_SV},
	{"DataFieldAltitudeSeaLevel"                     , 0x407, UT_SV},
	{"DataFieldDifferentialGpsDataAge"               , 0x408, UT_SV},
	{"DataFieldErrorRadius"                          , 0x409, UT_SV},
	{"DataFieldFixQuality"                           , 0x40A, UT_NARY},
	{"DataFieldFixType"                              , 0x40B, UT_NARY},
	{"DataFieldGeoidalSeparation"                    , 0x40C, UT_SV},
	{"DataFieldGpsOperationMode"                     , 0x40D, UT_NARY},
	{"DataFieldGpsSelectionMode"                     , 0x40E, UT_NARY},
	{"DataFieldGpsStatus"                            , 0x40F, UT_NARY},
	{"DataFieldPositionDilutionOfPrecision"          , 0x410, UT_SV},
	{"DataFieldHorizontalDilutionOfPrecision"        , 0x411, UT_SV},
	{"DataFieldVerticalDilutionOfPrecision"          , 0x412, UT_SV},
	{"DataFieldLatitude"                             , 0x413, UT_SV},
	{"DataFieldLongitude"                            , 0x414, UT_SV},
	{"DataFieldTrueHeading"                          , 0x415, UT_SV},
	{"DataFieldMagneticHeading"                      , 0x416, UT_SV},
	{"DataFieldMagneticVariation"                    , 0x417, UT_SV},
	{"DataFieldSpeed"                                , 0x418, UT_SV},
	{"DataFieldSatellitesInView"                     , 0x419, UT_SV},
	{"DataFieldSatellitesInViewAzimuth"              , 0x41A, UT_SV},
	{"DataFieldSatellitesInViewElevation"            , 0x41B, UT_SV},
	{"DataFieldSatellitesInViewIds"                  , 0x41C, UT_SV},
	{"DataFieldSatellitesInViewPrns"                 , 0x41D, UT_SV},
	{"DataFieldSatellitesInViewSnRatio"              , 0x41E, UT_SV},
	{"DataFieldSatellitesUsedCount"                  , 0x41F, UT_SV},
	{"DataFieldSatellitesUsedPrns"                   , 0x420, UT_SV},
	{"DataFieldNmeaSentence"                         , 0x421, UT_SV},
	{"DataFieldAddressLine1"                         , 0x422, UT_SV},
	{"DataFieldAddressLine2"                         , 0x423, UT_SV},
	{"DataFieldCity"                                 , 0x424, UT_SV},
	{"DataFieldStateOrProvince"                      , 0x425, UT_SV},
	{"DataFieldCountryOrRegion"                      , 0x426, UT_SV},
	{"DataFieldPostalCode"                           , 0x427, UT_SV},
	{"PropertyLocation"                              , 0x42A, UT_DV},
	{"PropertyLocationDesiredAccuracy"               , 0x42B, UT_NARY},
	{"DataFieldEnvironmental"                        , 0x430, UT_SV},
	{"DataFieldAtmosphericPressure"                  , 0x431, UT_SV},
	{"DataFieldRelativeHumidity"                     , 0x433, UT_SV},
	{"DataFieldTemperature"                          , 0x434, UT_SV},
	{"DataFieldWindDirection"                        , 0x435, UT_SV},
	{"DataFieldWindSpeed"                            , 0x436, UT_SV},
	{"DataFieldAirQualityIndex"                      , 0x437, UT_SV},
	{"DataFieldEquivalentCo2"                        , 0x438, UT_SV},
	{"DataFieldVolatileOrganicCompoundConcentration" , 0x439, UT_SV},
	{"DataFieldObjectPresence"                       , 0x43A, UT_SF},
	{"DataFieldObjectProximityRange"                 , 0x43B, UT_SV},
	{"DataFieldObjectProximityOutOfRange"            , 0x43C, UT_SF},
	{"PropertyEnvironmental"                         , 0x440, UT_SV},
	{"PropertyReferencePressure"                     , 0x441, UT_SV},
	{"DataFieldMotion"                               , 0x450, UT_DV},
	{"DataFieldMotionState"                          , 0x451, UT_SF},
	{"DataFieldAcceleration"                         , 0x452, UT_SV},
	{"DataFieldAccelerationAxisX"                    , 0x453, UT_SV},
	{"DataFieldAccelerationAxisY"                    , 0x454, UT_SV},
	{"DataFieldAccelerationAxisZ"                    , 0x455, UT_SV},
	{"DataFieldAngularVelocity"                      , 0x456, UT_SV},
	{"DataFieldAngularVelocityAboutXAxis"            , 0x457, UT_SV},
	{"DataFieldAngularVelocityAboutYAxis"            , 0x458, UT_SV},
	{"DataFieldAngularVelocityAboutZAxis"            , 0x459, UT_SV},
	{"DataFieldAngularPosition"                      , 0x45A, UT_SV},
	{"DataFieldAngularPositionAboutXAxis"            , 0x45B, UT_SV},
	{"DataFieldAngularPositionAboutYAxis"            , 0x45C, UT_SV},
	{"DataFieldAngularPositionAboutZAxis"            , 0x45D, UT_SV},
	{"DataFieldMotionSpeed"                          , 0x45E, UT_SV},
	{"DataFieldMotionIntensity"                      , 0x45F, UT_SV},
	{"DataFieldOrientation"                          , 0x470, UT_DV},
	{"DataFieldHeading"                              , 0x471, UT_SV},
	{"DataFieldHeadingXAxis"                         , 0x472, UT_SV},
	{"DataFieldHeadingYAxis"                         , 0x473, UT_SV},
	{"DataFieldHeadingZAxis"                         , 0x474, UT_SV},
	{"DataFieldHeadingCompensatedMagneticNorth"      , 0x475, UT_SV},
	{"DataFieldHeadingCompensatedTrueNorth"          , 0x476, UT_SV},
	{"DataFieldHeadingMagneticNorth"                 , 0x477, UT_SV},
	{"DataFieldHeadingTrueNorth"                     , 0x478, UT_SV},
	{"DataFieldDistance"                             , 0x479, UT_SV},
	{"DataFieldDistanceXAxis"                        , 0x47A, UT_SV},
	{"DataFieldDistanceYAxis"                        , 0x47B, UT_SV},
	{"DataFieldDistanceZAxis"                        , 0x47C, UT_SV},
	{"DataFieldDistanceOutOfRange"                   , 0x47D, UT_SF},
	{"DataFieldTilt"                                 , 0x47E, UT_SV},
	{"DataFieldTiltXAxis"                            , 0x47F, UT_SV},
	{"DataFieldTiltYAxis"                            , 0x480, UT_SV},
	{"DataFieldTiltZAxis"                            , 0x481, UT_SV},
	{"DataFieldRotationMatrix"                       , 0x482, UT_SV},
	{"DataFieldQuaternion"                           , 0x483, UT_SV},
	{"DataFieldMagneticFlux"                         , 0x484, UT_SV},
	{"DataFieldMagneticFluxXAxis"                    , 0x485, UT_SV},
	{"DataFieldMagneticFluxYAxis"                    , 0x486, UT_SV},
	{"DataFieldMagneticFluxZAxis"                    , 0x487, UT_SV},
	{"DataFieldMagnetometerAccuracy"                 , 0x488, UT_NARY},
	{"DataFieldSimpleOrientationDirection"           , 0x489, UT_NARY},
	{"DataFieldMechanical"                           , 0x490, UT_DV},
	{"DataFieldBooleanSwitchState"                   , 0x491, UT_SF},
	{"DataFieldBooleanSwitchArrayStates"             , 0x492, UT_SV},
	{"DataFieldMultivalueSwitchValue"                , 0x493, UT_SV},
	{"DataFieldField"                                , 0x494, UT_SV},
	{"DataFieldAbsolutePressure"                     , 0x495, UT_SV},
	{"DataFieldGaugePressure"                        , 0x496, UT_SV},
	{"DataFieldStrain"                               , 0x497, UT_SV},
	{"DataFieldWeight"                               , 0x498, UT_SV},
	{"PropertyMechanical"                            , 0x4A0, UT_DV},
	{"PropertyVibrationState"                        , 0x4A1, UT_DF},
	{"PropertyForwardVibrationSpeed"                 , 0x4A2, UT_DV},
	{"PropertyBackwardVibrationSpeed"                , 0x4A3, UT_DV},
	{"DataFieldBiometric"                            , 0x4B0, UT_DV},
	{"DataFieldHumanPresence"                        , 0x4B1, UT_SF},
	{"DataFieldHumanProximityRange"                  , 0x4B2, UT_SV},
	{"DataFieldHumanProximityOutOfRange"             , 0x4B3, UT_SF},
	{"DataFieldHumanTouchState"                      , 0x4B4, UT_SF},
	{"DataFieldBloodPressure"                        , 0x4B5, UT_SV},
	{"DataFieldBloodPressureDiastolic"               , 0x4B6, UT_SV},
	{"DataFieldBloodPressureSystolic"                , 0x4B7, UT_SV},
	{"DataFieldHeartRate"                            , 0x4B8, UT_SV},
	{"DataFieldRestingHeartRate"                     , 0x4B9, UT_SV},
	{"DataFieldHeartbeatInterval"                    , 0x4BA, UT_SV},
	{"DataFieldRespiratoryRate"                      , 0x4BB, UT_SV},
	{"DataFieldSpo2"                                 , 0x4BC, UT_SV},
	{"DataFieldLight"                                , 0x4D0, UT_DV},
	{"DataFieldIlluminance"                          , 0x4D1, UT_SV},
	{"DataFieldColorTemperature"                     , 0x4D2, UT_SV},
	{"DataFieldChromaticity"                         , 0x4D3, UT_SV},
	{"DataFieldChromaticityX"                        , 0x4D4, UT_SV},
	{"DataFieldChromaticityY"                        , 0x4D5, UT_SV},
	{"DataFieldConsumerIrSentenceReceive"            , 0x4D6, UT_SV},
	{"DataFieldInfraredLight"                        , 0x4D7, UT_SV},
	{"DataFieldRedLight"                             , 0x4D8, UT_SV},
	{"DataFieldGreenLight"                           , 0x4D9, UT_SV},
	{"DataFieldBlueLight"                            , 0x4DA, UT_SV},
	{"DataFieldUltravioletALight"                    , 0x4DB, UT_SV},
	{"DataFieldUltravioletBLight"                    , 0x4DC, UT_SV},
	{"DataFieldUltravioletIndex"                     , 0x4DD, UT_SV},
	{"DataFieldNearInfraredLight"                    , 0x4DE, UT_SV},
	{"PropertyLight"                                 , 0x4DF, UT_DV},
	{"PropertyConsumerIrSentenceSend"                , 0x4E0, UT_DV},
	{"PropertyAutoBrightnessPreferred"               , 0x4E2, UT_DF},
	{"PropertyAutoColorPreferred"                    , 0x4E3, UT_DF},
	{"DataFieldScanner"                              , 0x4F0, UT_DV},
	{"DataFieldRfidTag40Bit"                         , 0x4F1, UT_SV},
	{"DataFieldNfcSentenceReceive"                   , 0x4F2, UT_SV},
	{"PropertyScanner"                               , 0x4F8, UT_DV},
	{"PropertyNfcSentenceSend"                       , 0x4F9, UT_SV},
	{"DataFieldElectrical"                           , 0x500, UT_SV},
	{"DataFieldCapacitance"                          , 0x501, UT_SV},
	{"DataFieldCurrent"                              , 0x502, UT_SV},
	{"DataFieldElectricalPower"                      , 0x503, UT_SV},
	{"DataFieldInductance"                           , 0x504, UT_SV},
	{"DataFieldResistance"                           , 0x505, UT_SV},
	{"DataFieldVoltage"                              , 0x506, UT_SV},
	{"DataFieldFrequency"                            , 0x507, UT_SV},
	{"DataFieldPeriod"                               , 0x508, UT_SV},
	{"DataFieldPercentOfRange"                       , 0x509, UT_SV},
	{"DataFieldTime"                                 , 0x520, UT_DV},
	{"DataFieldYear"                                 , 0x521, UT_SV},
	{"DataFieldMonth"                                , 0x522, UT_SV},
	{"DataFieldDay"                                  , 0x523, UT_SV},
	{"DataFieldDayOfWeek"                            , 0x524, UT_NARY},
	{"DataFieldHour"                                 , 0x525, UT_SV},
	{"DataFieldMinute"                               , 0x526, UT_SV},
	{"DataFieldSecond"                               , 0x527, UT_SV},
	{"DataFieldMillisecond"                          , 0x528, UT_SV},
	{"DataFieldTimestamp"                            , 0x529, UT_SV},
	{"DataFieldJulianDayOfYear"                      , 0x52A, UT_SV},
	{"DataFieldTimeSinceSystemBoot"                  , 0x52B, UT_SV},
	{"PropertyTime"                                  , 0x530, UT_DV},
	{"PropertyTimeZoneOffsetFromUtc"                 , 0x531, UT_DV},
	{"PropertyTimeZoneName"                          , 0x532, UT_DV},
	{"PropertyDaylightSavingsTimeObserved"           , 0x533, UT_DF},
	{"PropertyTimeTrimAdjustment"                    , 0x534, UT_DV},
	{"PropertyArmAlarm"                              , 0x535, UT_DF},
	{"DataFieldCustom"                               , 0x540, UT_DV},
	{"DataFieldCustomUsage"                          , 0x541, UT_SV},
	{"DataFieldCustomBooleanArray"                   , 0x542, UT_SV},
	{"DataFieldCustomValue"                          , 0x543, UT_SV},
	{"DataFieldCustomValue1"                         , 0x544, UT_SV},
	{"DataFieldCustomValue2"                         , 0x545, UT_SV},
	{"DataFieldCustomValue3"                         , 0x546, UT_SV},
	{"DataFieldCustomValue4"                         , 0x547, UT_SV},
	{"DataFieldCustomValue5"                         , 0x548, UT_SV},
	{"DataFieldCustomValue6"                         , 0x549, UT_SV},
	{"DataFieldCustomValue7"                         , 0x54A, UT_SV},
	{"DataFieldCustomValue8"                         , 0x54B, UT_SV},
	{"DataFieldCustomValue9"                         , 0x54C, UT_SV},
	{"DataFieldCustomValue10"                        , 0x54D, UT_SV},
	{"DataFieldCustomValue11"                        , 0x54E, UT_SV},
	{"DataFieldCustomValue12"                        , 0x54F, UT_SV},
	{"DataFieldCustomValue13"                        , 0x550, UT_SV},
	{"DataFieldCustomValue14"                        , 0x551, UT_SV},
	{"DataFieldCustomValue15"                        , 0x552, UT_SV},
	{"DataFieldCustomValue16"                        , 0x553, UT_SV},
	{"DataFieldCustomValue17"                        , 0x554, UT_SV},
	{"DataFieldCustomValue18"                        , 0x555, UT_SV},
	{"DataFieldCustomValue19"                        , 0x556, UT_SV},
	{"DataFieldCustomValue20"                        , 0x557, UT_SV},
	{"DataFieldCustomValue21"                        , 0x558, UT_SV},
	{"DataFieldCustomValue22"                        , 0x559, UT_SV},
	{"DataFieldCustomValue23"                        , 0x55A, UT_SV},
	{"DataFieldCustomValue24"                        , 0x55B, UT_SV},
	{"DataFieldCustomValue25"                        , 0x55C, UT_SV},
	{"DataFieldCustomValue26"                        , 0x55D, UT_SV},
	{"DataFieldCustomValue27"                        , 0x55E, UT_SV},
	{"DataFieldCustomValue28"                        , 0x55F, UT_SV},
	{"DataFieldGeneric"                              , 0x560, UT_DV},
	{"DataFieldGenericGuidOrPropertykey"             , 0x561, UT_SV},
	{"DataFieldGenericCategoryGuid"                  , 0x562, UT_SV},
	{"DataFieldGenericTypeGuid"                      , 0x563, UT_SV},
	{"DataFieldGenericEventPropertykey"              , 0x564, UT_SV},
	{"DataFieldGenericPropertyPropertykey"           , 0x565, UT_SV},
	{"DataFieldGenericDataFieldPropertykey"          , 0x566, UT_SV},
	{"DataFieldGenericEvent"                         , 0x567, UT_SV},
	{"DataFieldGenericProperty"                      , 0x568, UT_SV},
	{"DataFieldGenericDataField"                     , 0x569, UT_SV},
	{"DataFieldEnumeratorTableRowIndex"              , 0x56A, UT_SV},
	{"DataFieldEnumeratorTableRowCount"              , 0x56B, UT_SV},
	{"DataFieldGenericGuidOrPropertykeyKind"         , 0x56C, UT_NARY},
	{"DataFieldGenericGuid"                          , 0x56D, UT_SV},
	{"DataFieldGenericPropertykey"                   , 0x56E, UT_SV},
	{"DataFieldGenericTopLevelCollectionId"          , 0x56F, UT_SV},
	{"DataFieldGenericReportId"                      , 0x570, UT_SV},
	{"DataFieldGenericReportItemPositionIndex"       , 0x571, UT_SV},
	{"DataFieldGenericFirmwareVartype"               , 0x572, UT_NARY},
	{"DataFieldGenericUnitOfMessure"                 , 0x573, UT_NARY},
	{"DataFieldGenericUnitExponent"                  , 0x574, UT_NARY},
	{"DataFieldGenericReportSize"                    , 0x575, UT_SV},
	{"DataFieldGenericReportCount"                   , 0x576, UT_SV},
	{"PropertyGeneric"                               , 0x580, UT_DV},
	{"PropertyEnumeratorTableRowIndex"               , 0x581, UT_DV},
	{"PropertyEnumeratorTableRowCount"               , 0x582, UT_SV},
	{"DataFieldPersonalActivity"                     , 0x590, UT_DV},
	{"DataFieldActivityType"                         , 0x591, UT_NARY},
	{"DataFieldActivityState"                        , 0x592, UT_NARY},
	{"DataFieldDevicePosition"                       , 0x593, UT_NARY},
	{"DataFieldStepCount"                            , 0x594, UT_SV},
	{"DataFieldStepCountReset"                       , 0x595, UT_DF},
	{"DataFieldStepDuration"                         , 0x596, UT_SV},
	{"DataFieldStepType"                             , 0x597, UT_NARY},
	{"PropertyMinimumActivityDetectionInterval"      , 0x5A0, UT_DV},
	{"PropertySupportedActivityTypes"                , 0x5A1, UT_NARY},
	{"PropertySubscribedActivityTypes"               , 0x5A2, UT_NARY},
	{"PropertySupportedStepTypes"                    , 0x5A3, UT_NARY},
	{"PropertySubscribedStepTypes"                   , 0x5A4, UT_NARY},
	{"PropertyFloorHeight"                           , 0x5A5, UT_DV},
	{"DataFieldCustomTypeId"                         , 0x5B0, UT_SV},
	{"PropertyCustom"                                , 0x5C0, UT_DV},
	{"PropertyCustomValue1"                          , 0x5C1, UT_DV},
	{"PropertyCustomValue2"                          , 0x5C2, UT_DV},
	{"PropertyCustomValue3"                          , 0x5C3, UT_DV},
	{"PropertyCustomValue4"                          , 0x5C4, UT_DV},
	{"PropertyCustomValue5"                          , 0x5C5, UT_DV},
	{"PropertyCustomValue6"                          , 0x5C6, UT_DV},
	{"PropertyCustomValue7"                          , 0x5C7, UT_DV},
	{"PropertyCustomValue8"                          , 0x5C8, UT_DV},
	{"PropertyCustomValue9"                          , 0x5C9, UT_DV},
	{"PropertyCustomValue10"                         , 0x5CA, UT_DV},
	{"PropertyCustomValue11"                         , 0x5CB, UT_DV},
	{"PropertyCustomValue12"                         , 0x5CC, UT_DV},
	{"PropertyCustomValue13"                         , 0x5CD, UT_DV},
	{"PropertyCustomValue14"                         , 0x5CE, UT_DV},
	{"PropertyCustomValue15"                         , 0x5CF, UT_DV},
	{"PropertyCustomValue16"                         , 0x5D0, UT_DV},
	{"DataFieldHinge"                                , 0x5E0, UT_SV|UT_DV},
	{"DataFieldHingeAngle"                           , 0x5E1, UT_SV|UT_DV},
	{"DataFieldGestureSensor"                        , 0x5F0, UT_DV},
	{"DataFieldGestureState"                         , 0x5F1, UT_NARY},
	{"DataFieldHingeFoldInitialAngle"                , 0x5F2, UT_SV},
	{"DataFieldHingeFoldFinalAngle"                  , 0x5F3, UT_SV},
	{"DataFieldHingeFoldContributionPanel"           , 0x5F4, UT_NARY},
	{"DataFieldHingeFoldType"                        , 0x5F5, UT_NARY},
	{"SensorStateUndefined"                          , 0x800, UT_SEL},
	{"SensorStateReady"                              , 0x801, UT_SEL},
	{"SensorStateNotAvailable"                       , 0x802, UT_SEL},
	{"SensorStateNoData"                             , 0x803, UT_SEL},
	{"SensorStateInitializing"                       , 0x804, UT_SEL},
	{"SensorStateAccessDenied"                       , 0x805, UT_SEL},
	{"SensorStateError"                              , 0x806, UT_SEL},
	{"SensorEventUnknown"                            , 0x810, UT_SEL},
	{"SensorEventStateChanged"                       , 0x811, UT_SEL},
	{"SensorEventPropertyChanged"                    , 0x812, UT_SEL},
	{"SensorEventDataUploaded"                       , 0x813, UT_SEL},
	{"SensorEventPollResponse"                       , 0x814, UT_SEL},
	{"SensorEventChangeSensitivity"                  , 0x815, UT_SEL},
	{"SensorEventRangeMaximumReached"                , 0x816, UT_SEL},
	{"SensorEventRangeMinimumReached"                , 0x817, UT_SEL},
	{"SensorEventHighThresholdCrossUpward"           , 0x818, UT_SEL},
	{"SensorEventHighThresholdCrossDownward"         , 0x819, UT_SEL},
	{"SensorEventLowThresholdCrossUpward"            , 0x81A, UT_SEL},
	{"SensorEventLowThresholdCrossDownward"          , 0x81B, UT_SEL},
	{"SensorEventZeroThresholdCrossUpward"           , 0x81C, UT_SEL},
	{"SensorEventZeroThresholdCrossDownward"         , 0x81D, UT_SEL},
	{"SensorEventPeriodExceeded"                     , 0x81E, UT_SEL},
	{"SensorEventFrequencyExceeded"                  , 0x81F, UT_SEL},
	{"SensorEventComplexTrigger"                     , 0x820, UT_SEL},
	{"ConnectionTypePcIntegrated"                    , 0x830, UT_SEL},
	{"ConnectionTypePcAttached"                      , 0x831, UT_SEL},
	{"ConnectionTypePcExternal"                      , 0x832, UT_SEL},
	{"ReportingStateReportNoEvents"                  , 0x840, UT_SEL},
	{"ReportingStateReportAllEvents"                 , 0x841, UT_SEL},
	{"ReportingStateReportThresholdEvents"           , 0x842, UT_SEL},
	{"ReportingStateWakeOnNoEvents"                  , 0x843, UT_SEL},
	{"ReportingStateWakeOnAllEvents"                 , 0x844, UT_SEL},
	{"ReportingStateWakeOnThresholdEvents"           , 0x845, UT_SEL},
	{"PowerStateUndefined"                           , 0x850, UT_SEL},
	{"PowerStateD0FullPower"                         , 0x851, UT_SEL},
	{"PowerStateD1LowPower"                          , 0x852, UT_SEL},
	{"PowerStateD2StandbyPowerWithWakeup"            , 0x853, UT_SEL},
	{"PowerStateD3SleepWithWakeup"                   , 0x854, UT_SEL},
	{"PowerStateD4PowerOff"                          , 0x855, UT_SEL},
	{"FixQualityNoFix"                               , 0x870, UT_SEL},
	{"FixQualityGps"                                 , 0x871, UT_SEL},
	{"FixQualityDgps"                                , 0x872, UT_SEL},
	{"FixTypeNoFix"                                  , 0x880, UT_SEL},
	{"FixTypeGpsSpsModeFixValid"                     , 0x881, UT_SEL},
	{"FixTypeDgpsSpsModeFixValid"                    , 0x882, UT_SEL},
	{"FixTypeGpsPpsModeFixValid"                     , 0x883, UT_SEL},
	{"FixTypeRealTimeKinematic"                      , 0x884, UT_SEL},
	{"FixTypeFloatRtk"                               , 0x885, UT_SEL},
	{"FixTypeEstimatedDeadReckoned"                  , 0x886, UT_SEL},
	{"FixTypeManualInputMode"                        , 0x887, UT_SEL},
	{"FixTypeSimulatorMode"                          , 0x888, UT_SEL},
	{"GpsOperationModeManual"                        , 0x890, UT_SEL},
	{"GpsOperationModeAutomatic"                     , 0x891, UT_SEL},
	{"GpsSelectionModeAutonomous"                    , 0x8A0, UT_SEL},
	{"GpsSelectionModeDgps"                          , 0x8A1, UT_SEL},
	{"GpsSelectionModeEstimatedDeadReckoned"         , 0x8A2, UT_SEL},
	{"GpsSelectionModeManualInput"                   , 0x8A3, UT_SEL},
	{"GpsSelectionModeSimulator"                     , 0x8A4, UT_SEL},
	{"GpsSelectionModeDataNotValid"                  , 0x8A5, UT_SEL},
	{"GpsStatusDataValid"                            , 0x8B0, UT_SEL},
	{"GpsStatusDataNotValid"                         , 0x8B1, UT_SEL},
	{"AccuracyDefault"                               , 0x860, UT_SEL},
	{"AccuracyHigh"                                  , 0x861, UT_SEL},
	{"AccuracyMedium"                                , 0x862, UT_SEL},
	{"AccuracyLow"                                   , 0x863, UT_SEL},
	{"DayOfWeekSunday"                               , 0x8C0, UT_SEL},
	{"DayOfWeekMonday"                               , 0x8C1, UT_SEL},
	{"DayOfWeekTuesday"                              , 0x8C2, UT_SEL},
	{"DayOfWeekWednesday"                            , 0x8C3, UT_SEL},
	{"DayOfWeekThursday"                             , 0x8C4, UT_SEL},
	{"DayOfWeekFriday"                               , 0x8C5, UT_SEL},
	{"DayOfWeekSaturday"                             , 0x8C6, UT_SEL},
	{"KindCategory"                                  , 0x8D0, UT_SEL},
	{"KindType"                                      , 0x8D1, UT_SEL},
	{"KindEvent"                                     , 0x8D2, UT_SEL},
	{"KindProperty"                                  , 0x8D3, UT_SEL},
	{"KindDataField"                                 , 0x8D4, UT_SEL},
	{"MagnetometerAccuracyLow"                       , 0x8E0, UT_SEL},
	{"MagnetometerAccuracyMedium"                    , 0x8E1, UT_SEL},
	{"MagnetometerAccuracyHigh"                      , 0x8E2, UT_SEL},
	{"SimpleOrientationDirectionNotRotated"          , 0x8F0, UT_SEL},
	{"SimpleOrientationDirectionRotated90DegreesCcw" , 0x8F1, UT_SEL},
	{"SimpleOrientationDirectionRotated180DegreesCcw", 0x8F2, UT_SEL},
	{"SimpleOrientationDirectionRotated270DegreesCcw", 0x8F3, UT_SEL},
	{"SimpleOrientationDirectionFaceUp"              , 0x8F4, UT_SEL},
	{"SimpleOrientationDirectionFaceDown"            , 0x8F5, UT_SEL},
	{"VtNull"                                        , 0x900, UT_SEL},
	{"VtBool"                                        , 0x901, UT_SEL},
	{"VtUi1"                                         , 0x902, UT_SEL},
	{"VtI1"                                          , 0x903, UT_SEL},
	{"VtUi2"                                         , 0x904, UT_SEL},
	{"VtI2"                                          , 0x905, UT_SEL},
	{"VtUi4"                                         , 0x906, UT_SEL},
	{"VtI4"                                          , 0x907, UT_SEL},
	{"VtUi8"                                         , 0x908, UT_SEL},
	{"VtI8"                                          , 0x909, UT_SEL},
	{"VtR4"                                          , 0x90A, UT_SEL},
	{"VtR8"                                          , 0x90B, UT_SEL},
	{"VtWstr"                                        , 0x90C, UT_SEL},
	{"VtStr"                                         , 0x90D, UT_SEL},
	{"VtClsid"                                       , 0x90E, UT_SEL},
	{"VtVectorVtUi1"                                 , 0x90F, UT_SEL},
	{"VtF16E0"                                       , 0x910, UT_SEL},
	{"VtF16E1"                                       , 0x911, UT_SEL},
	{"VtF16E2"                                       , 0x912, UT_SEL},
	{"VtF16E3"                                       , 0x913, UT_SEL},
	{"VtF16E4"                                       , 0x914, UT_SEL},
	{"VtF16E5"                                       , 0x915, UT_SEL},
	{"VtF16E6"                                       , 0x916, UT_SEL},
	{"VtF16E7"                                       , 0x917, UT_SEL},
	{"VtF16E8"                                       , 0x918, UT_SEL},
	{"VtF16E9"                                       , 0x919, UT_SEL},
	{"VtF16EA"                                       , 0x91A, UT_SEL},
	{"VtF16EB"                                       , 0x91B, UT_SEL},
	{"VtF16EC"                                       , 0x91C, UT_SEL},
	{"VtF16ED"                                       , 0x91D, UT_SEL},
	{"VtF16EE"                                       , 0x91E, UT_SEL},
	{"VtF16EF"                                       , 0x91F, UT_SEL},
	{"VtF32E0"                                       , 0x920, UT_SEL},
	{"VtF32E1"                                       , 0x921, UT_SEL},
	{"VtF32E2"                                       , 0x922, UT_SEL},
	{"VtF32E3"                                       , 0x923, UT_SEL},
	{"VtF32E4"                                       , 0x924, UT_SEL},
	{"VtF32E5"                                       , 0x925, UT_SEL},
	{"VtF32E6"                                       , 0x926, UT_SEL},
	{"VtF32E7"                                       , 0x927, UT_SEL},
	{"VtF32E8"                                       , 0x928, UT_SEL},
	{"VtF32E9"                                       , 0x929, UT_SEL},
	{"VtF32EA"                                       , 0x92A, UT_SEL},
	{"VtF32EB"                                       , 0x92B, UT_SEL},
	{"VtF32EC"                                       , 0x92C, UT_SEL},
	{"VtF32ED"                                       , 0x92D, UT_SEL},
	{"VtF32EE"                                       , 0x92E, UT_SEL},
	{"VtF32EF"                                       , 0x92F, UT_SEL},
	{"ActivityTypeUnknown"                           , 0x930, UT_SEL},
	{"ActivityTypeStationary"                        , 0x931, UT_SEL},
	{"ActivityTypeFidgeting"                         , 0x932, UT_SEL},
	{"ActivityTypeWalking"                           , 0x933, UT_SEL},
	{"ActivityTypeRunning"                           , 0x934, UT_SEL},
	{"ActivityTypeInVehicle"                         , 0x935, UT_SEL},
	{"ActivityTypeBiking"                            , 0x936, UT_SEL},
	{"ActivityTypeIdle"                              , 0x937, UT_SEL},
	{"UnitNotSpecified"                              , 0x940, UT_SEL},
	{"UnitLux"                                       , 0x941, UT_SEL},
	{"UnitDegreesKelvin"                             , 0x942, UT_SEL},
	{"UnitDegreesCelsius"                            , 0x943, UT_SEL},
	{"UnitPascal"                                    , 0x944, UT_SEL},
	{"UnitNewton"                                    , 0x945, UT_SEL},
	{"UnitMetersPerSecond"                           , 0x946, UT_SEL},
	{"UnitKilogram"                                  , 0x947, UT_SEL},
	{"UnitMeter"                                     , 0x948, UT_SEL},
	{"UnitMetersPerSecondSquared"                    , 0x949, UT_SEL}, /* changed name */
	{"UnitFarad"                                     , 0x94A, UT_SEL},
	{"UnitAmpere"                                    , 0x94B, UT_SEL},
	{"UnitWatt"                                      , 0x94C, UT_SEL},
	{"UnitHenry"                                     , 0x94D, UT_SEL},
	{"UnitOhm"                                       , 0x94E, UT_SEL},
	{"UnitVolt"                                      , 0x94F, UT_SEL},
	{"UnitHerz"                                      , 0x950, UT_SEL},
	{"UnitBar"                                       , 0x951, UT_SEL},
	{"UnitDegreesAntiClockwise"                      , 0x952, UT_SEL},
	{"UnitDegreesClockwise"                          , 0x953, UT_SEL},
	{"UnitDegrees"                                   , 0x954, UT_SEL},
	{"UnitDegreesPerSecond"                          , 0x955, UT_SEL},
	{"UnitDegreesPerSecondSquared"                   , 0x956, UT_SEL}, /* changed name */
	{"UnitKnot"                                      , 0x957, UT_SEL},
	{"UnitPercent"                                   , 0x958, UT_SEL},
	{"UnitSecond"                                    , 0x959, UT_SEL},
	{"UnitMillisecond"                               , 0x95A, UT_SEL},
	{"UnitG"                                         , 0x95B, UT_SEL},
	{"UnitBytes"                                     , 0x95C, UT_SEL},
	{"UnitMilligauss"                                , 0x95D, UT_SEL},
	{"UnitBits"                                      , 0x95E, UT_SEL},
	{"ActivityStateNoStateChange"                    , 0x960, UT_SEL},
	{"ActivityStateStartActivity"                    , 0x961, UT_SEL},
	{"ActivityStateEndActivity"                      , 0x962, UT_SEL},
	{"Exponent0"                                     , 0x970, UT_SEL},
	{"Exponent1"                                     , 0x971, UT_SEL},
	{"Exponent2"                                     , 0x972, UT_SEL},
	{"Exponent3"                                     , 0x973, UT_SEL},
	{"Exponent4"                                     , 0x974, UT_SEL},
	{"Exponent5"                                     , 0x975, UT_SEL},
	{"Exponent6"                                     , 0x976, UT_SEL},
	{"Exponent7"                                     , 0x977, UT_SEL},
	{"Exponent8"                                     , 0x978, UT_SEL},
	{"Exponent9"                                     , 0x979, UT_SEL},
	{"ExponentA"                                     , 0x97A, UT_SEL},
	{"ExponentB"                                     , 0x97B, UT_SEL},
	{"ExponentC"                                     , 0x97C, UT_SEL},
	{"ExponentD"                                     , 0x97D, UT_SEL},
	{"ExponentE"                                     , 0x97E, UT_SEL},
	{"ExponentF"                                     , 0x97F, UT_SEL},
	{"DevicePositionUnknown"                         , 0x980, UT_SEL},
	{"DevicePositionUnchanged"                       , 0x981, UT_SEL},
	{"DevicePositionOnDesk"                          , 0x982, UT_SEL},
	{"DevicePositionInHand"                          , 0x983, UT_SEL},
	{"DevicePositionMovingInBag"                     , 0x984, UT_SEL},
	{"DevicePositionStationaryInBag"                 , 0x985, UT_SEL},
	{"StepTypeUnknown"                               , 0x990, UT_SEL},
	{"StepTypeRunning"                               , 0x991, UT_SEL},
	{"StepTypeWalking"                               , 0x992, UT_SEL},
	{"GestureStateUnknown"                           , 0x9A0, UT_SEL},
	{"GestureStateStarted"                           , 0x9A1, UT_SEL},
	{"GestureStateCompleted"                         , 0x9A2, UT_SEL},
	{"GestureStateCancelled"                         , 0x9A3, UT_SEL},
	{"HingeFoldContributionPanelUnknown"             , 0x9B0, UT_SEL},
	{"HingeFoldContributionPanelPanel1"              , 0x9B1, UT_SEL},
	{"HingeFoldContributionPanelPanel2"              , 0x9B2, UT_SEL},
	{"HingeFoldContributionPanelBoth"                , 0x9B3, UT_SEL},
	{"HingeFoldTypeUnknown"                          , 0x9B4, UT_SEL},
	{"HingeFoldTypeIncreasing"                       , 0x9B5, UT_SEL},
	{"HingeFoldTypeDecreasing"                       , 0x9B6, UT_SEL},
	{"ModifierChangeSensitivityAbsolute"             , 0x1000, UT_US},
	{"ModifierMaximum"                               , 0x2000, UT_US},
	{"ModifierMinimum"                               , 0x3000, UT_US},
	{"ModifierAccuracy"                              , 0x4000, UT_US},
	{"ModifierResolution"                            , 0x5000, UT_US},
	{"ModifierThresholdHigh"                         , 0x6000, UT_US},
	{"ModifierThresholdLow"                          , 0x7000, UT_US},
	{"ModifierCalibrationOffset"                     , 0x8000, UT_US},
	{"ModifierCalibrationMultiplier"                 , 0x9000, UT_US},
	{"ModifierReportInterval"                        , 0xA000, UT_US},
	{"ModifierFrequencyMax"                          , 0xB000, UT_US},
	{"ModifierPeriodMax"                             , 0xC000, UT_US},
	{"ModifierChangeSensitivityPercentOfRange"       , 0xD000, UT_US},
	{"ModifierChangeSensitivityPercentRelative"      , 0xE000, UT_US},
	{"ModifierVendorReserved"                        , 0xF000, UT_US},
	endOfMap
};


/**
 * HID descriptor usage medical instrument argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 22
 */
constexpr const Encoding medInstMap[] = {
	{"MedicalUlrasound"         , 0x01, UT_CA},
	{"VcrAcquisition"           , 0x20, UT_OOC},
	{"FreezeThaw"               , 0x21, UT_OOC},
	{"ClipStore"                , 0x22, UT_OSC},
	{"Update"                   , 0x23, UT_OSC},
	{"Next"                     , 0x24, UT_OSC},
	{"Save"                     , 0x25, UT_OSC},
	{"Print"                    , 0x26, UT_OSC},
	{"MicrophoneEnable"         , 0x27, UT_OSC},
	{"Cine"                     , 0x40, UT_LC},
	{"TransmitPower"            , 0x41, UT_LC},
	{"Volume"                   , 0x42, UT_LC},
	{"Focus"                    , 0x43, UT_LC},
	{"Depth"                    , 0x44, UT_LC},
	{"SoftStepPrimary"          , 0x60, UT_LC},
	{"SoftStepSecondary"        , 0x61, UT_LC},
	{"DepthGainCompensation"    , 0x70, UT_LC},
	{"ZoomSelect"               , 0x80, UT_OSC},
	{"ZoomAdjust"               , 0x81, UT_LC},
	{"SpectralDopplerModeSelect", 0x82, UT_OSC},
	{"SpectralDopplerAdjust"    , 0x83, UT_LC},
	{"ColorDopplerModeSelect"   , 0x84, UT_OSC},
	{"ColorDopplerAdjust"       , 0x85, UT_LC},
	{"MotionModeSelect"         , 0x86, UT_OSC},
	{"MotionModeAdjust"         , 0x87, UT_LC},
	{"Mode2dSelect"             , 0x88, UT_OSC}, /* changed name to avoid leading digit */
	{"Mode2dAdjust"             , 0x89, UT_LC}, /* changed name to avoid leading digit */
	{"SoftControlSelect"        , 0xA0, UT_OSC},
	{"SoftControlAdjust"        , 0xA1, UT_LC},
	endOfMap
};


/**
 * HID descriptor usage braille display argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 23
 */
constexpr const Encoding brailleMap[] = {
	{"BrailleDisplay"           , 0x01, UT_CA},
	{"BrailleRow"               , 0x02, UT_NARY},
	{"Dot8BrailleCell"          , 0x03, UT_DV}, /* changed name to avoid leading digit */
	{"Dot6BrailleCell"          , 0x04, UT_DV}, /* changed name to avoid leading digit */
	{"NumberOfBrailleCells"     , 0x05, UT_DV},
	{"ScreenReaderControl"      , 0x06, UT_NARY},
	{"ScreenReaderIdentifier"   , 0x07, UT_DV},
	{"RouterSet1"               , 0xFA, UT_NARY},
	{"RouterSet2"               , 0xFB, UT_NARY},
	{"RouterSet3"               , 0xFC, UT_NARY},
	{"RouterKey"                , 0x100, UT_SEL},
	{"RowRouterKey"             , 0x101, UT_SEL},
	{"BrailleButtons"           , 0x200, UT_NARY},
	{"BrailleKeyboardDot1"      , 0x201, UT_SEL},
	{"BrailleKeyboardDot2"      , 0x202, UT_SEL},
	{"BrailleKeyboardDot3"      , 0x203, UT_SEL},
	{"BrailleKeyboardDot4"      , 0x204, UT_SEL},
	{"BrailleKeyboardDot5"      , 0x205, UT_SEL},
	{"BrailleKeyboardDot6"      , 0x206, UT_SEL},
	{"BrailleKeyboardDot7"      , 0x207, UT_SEL},
	{"BrailleKeyboardDot8"      , 0x208, UT_SEL},
	{"BrailleKeyboardSpace"     , 0x209, UT_SEL},
	{"BrailleKeyboardLeftSpace" , 0x20A, UT_SEL},
	{"BrailleKeyboardRightSpace", 0x20B, UT_SEL},
	{"BrailleFaceConrols"       , 0x20C, UT_NARY},
	{"BrailleLeftControls"      , 0x20D, UT_NARY},
	{"BrailleRightControls"     , 0x20E, UT_NARY},
	{"BrailleTopControls"       , 0x20F, UT_NARY},
	{"BrailleJoystickCenter"    , 0x210, UT_SEL},
	{"BrailleJoystickUp"        , 0x211, UT_SEL},
	{"BrailleJoystickDown"      , 0x212, UT_SEL},
	{"BrailleJoystickLeft"      , 0x213, UT_SEL},
	{"BrailleJoystickRight"     , 0x214, UT_SEL},
	{"BrailleDPadCenter"        , 0x215, UT_SEL},
	{"BrailleDPadUp"            , 0x216, UT_SEL},
	{"BrailleDPadDown"          , 0x217, UT_SEL},
	{"BrailleDPadLeft"          , 0x218, UT_SEL},
	{"BrailleDPadRight"         , 0x219, UT_SEL},
	{"BraillePanLeft"           , 0x21A, UT_SEL},
	{"BraillePanRight"          , 0x21B, UT_SEL},
	{"BrailleRockerUp"          , 0x21C, UT_SEL},
	{"BrailleRockerDown"        , 0x21D, UT_SEL},
	{"BrailleRockerPress"       , 0x21E, UT_SEL},
	endOfMap
};


/**
 * HID descriptor usage lighting and illumination argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 24
 */
constexpr const Encoding lightMap[] = {
	{"LampArray"                      , 0x01, UT_CA},
	{"LampArrayAttributesReport"      , 0x02, UT_CL},
	{"LampCount"                      , 0x03, UT_SV|UT_DV},
	{"BoundingBoxWidthInMicrometers"  , 0x04, UT_SV},
	{"BoundingBoxHeightInMicrometers" , 0x05, UT_SV},
	{"BoundingBoxDepthInMicrometers"  , 0x06, UT_SV},
	{"LampArrayKind"                  , 0x07, UT_SV},
	{"MinUpdateIntervalInMicroseconds", 0x08, UT_SV},
	{"LampAtributesRequestReport"     , 0x20, UT_CL},
	{"LampId"                         , 0x21, UT_SV|UT_DV},
	{"LampAtributesResponseReport"    , 0x22, UT_CL},
	{"PositionXInMicrometers"         , 0x23, UT_DV},
	{"PositionYInMicrometers"         , 0x24, UT_DV},
	{"PositionZInMicrometers"         , 0x25, UT_DV},
	{"LampPurposes"                   , 0x26, UT_DV},
	{"UpdateLatencyInMicroseconds"    , 0x27, UT_DV},
	{"RedLevelCount"                  , 0x28, UT_DV},
	{"GreenLevelCount"                , 0x29, UT_DV},
	{"BlueLevelCount"                 , 0x2A, UT_DV},
	{"IntensityLevelCount"            , 0x2B, UT_DV},
	{"IsProgrammable"                 , 0x2C, UT_DV},
	{"InputBinding"                   , 0x2D, UT_DV},
	{"LampMultiUpdateReport"          , 0x50, UT_CL},
	{"RedUpdateChannel"               , 0x51, UT_DV},
	{"GreenUpdateChannel"             , 0x52, UT_DV},
	{"BlueUpdateChannel"              , 0x53, UT_DV},
	{"IntensityUpdateChannel"         , 0x54, UT_DV},
	{"LampUpdateFlags"                , 0x55, UT_DV},
	{"LampRangeUpdateReport"          , 0x60, UT_CL},
	{"LampIdStart"                    , 0x61, UT_DV},
	{"LampIdEnd"                      , 0x62, UT_DV},
	{"LampArrayControlReport"         , 0x70, UT_CL},
	{"AutonomousMode"                 , 0x71, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage monitor argument token encoding map.
 * 
 * @see Monitor Control Class Specification 1.0 ch. 6.1.1
 * @remarks No usage types defined in the standard.
 */
constexpr const Encoding monitorMap[] = {
	{"MonitorControl",  0x01},
	{"EdidInformation", 0x02},
	{"VdifInformation", 0x03},
	{"VesaVersion",     0x04},
	endOfMap
};


/**
 * HID descriptor usage monitor enumerated values argument token encoding map.
 * 
 * @see Monitor Control Class Specification 1.0 ch. 6.2
 * @remarks No usage types defined in the standard.
 */
constexpr const Encoding monitorEnumMap[] = {
	{"Enum#", 0x00}, /* range start */
	{"Enum#", 0x3E}, /* range end */
	endOfMap
};


/**
 * HID descriptor usage VESA virtual controls argument token encoding map.
 * 
 * @see Monitor Control Class Specification 1.0 ch. 6.3
 * @remarks No usage types defined in the standard.
 */
constexpr const Encoding vesaCtrlMap[] = {
	/* Contiguous Controls */
	{"Brightness"                       , 0x10},
	{"Contrast"                         , 0x12},
	{"RedVideoGain"                     , 0x16},
	{"GreenVideoGain"                   , 0x18},
	{"BlueVideoGain"                    , 0x1A},
	{"Focus"                            , 0x1C},
	{"HorizontalPosition"               , 0x20},
	{"HorizontalSize"                   , 0x22},
	{"HorizontalPincushion"             , 0x24},
	{"HorizontalPincushionBalance"      , 0x26},
	{"HorizontalMisconvergence"         , 0x28},
	{"HorizontalLinearity"              , 0x2A},
	{"HorizontalLinearityBalance"       , 0x2C},
	{"VerticalPosition"                 , 0x30},
	{"VerticalSize"                     , 0x32},
	{"VerticalPincushion"               , 0x34},
	{"VerticalPincushionBalance"        , 0x36},
	{"VerticalMisconvergence"           , 0x38},
	{"VerticalLinearity"                , 0x3A},
	{"VerticalLinearityBalance"         , 0x3C},
	{"ParallelogramDistortionKeyBalance", 0x40},
	{"TrapezoidalDistortionKey"         , 0x42},
	{"TiltRotation"                     , 0x44},
	{"TopCornerDistortionControl"       , 0x46},
	{"TopCornerDistortionBalance"       , 0x48},
	{"BottomCornerDistortionControl"    , 0x4A},
	{"BottomCornerDistortionBalance"    , 0x4C},
	{"HorizontalMoire"                  , 0x56},
	{"VerticalMoire"                    , 0x58},
	{"RedVideoBlackLevel"               , 0x6C},
	{"GreenVideoBlackLevel"             , 0x6E},
	{"BlueVideoBlackLevel"              , 0x70},
	/* Non-contiguous Controls (Read/Write) */
	{"InputLevelSelect"                 , 0x5E},
	{"InputSourceSelect"                , 0x60},
	{"OnScreenDisplay"                  , 0xCA},
	{"StereoMode"                       , 0xD4},
	/* Non-contiguous Controls (Read-only) */
	{"AutoSizeCenter"                   , 0xA2},
	{"PolarityHorizontalSynchronization", 0xA4},
	{"PolarityVerticalSynchronization"  , 0xA6},
	{"SynchronizationType"              , 0xA8},
	{"ScreenOrientation"                , 0xAA},
	{"HorizontalFrequency"              , 0xAC},
	{"VerticalFrequency"                , 0xAE},
	{"Degauss"                          , 0x01},
	{"Settings"                         , 0xB0},
	endOfMap
};


/**
 * HID descriptor usage camera control argument token encoding map.
 * 
 * @see Usage Tables for HID Power Devices 1.0 ch. 4.1
 */
constexpr const Encoding pwrDevMap[] = {
	{"IName"              , 0x01, UT_SV},
	{"PresentStatus"      , 0x02, UT_CL},
	{"ChangedStatus"      , 0x03, UT_CL},
	{"Ups"                , 0x04, UT_CA},
	{"PowerSupply"        , 0x05, UT_CA},
	{"BatterySystem"      , 0x10, UT_CP},
	{"BatterySystemId"    , 0x11, UT_SV},
	{"Battery"            , 0x12, UT_CP},
	{"BatteryId"          , 0x13, UT_SV},
	{"Charger"            , 0x14, UT_CP},
	{"ChargerId"          , 0x15, UT_SV},
	{"PowerConverer"      , 0x16, UT_CP},
	{"PowerConvererId"    , 0x17, UT_SV},
	{"OutletSystem"       , 0x18, UT_CP},
	{"OutletSystemId"     , 0x19, UT_SV},
	{"Input"              , 0x1A, UT_CP},
	{"InputId"            , 0x1B, UT_SV},
	{"Output"             , 0x1C, UT_CP},
	{"OutputId"           , 0x1D, UT_SV},
	{"Flow"               , 0x1E, UT_CP},
	{"FlowId"             , 0x1F, UT_SV}, /* changed usage type to match similar usage IDs */
	{"Outlet"             , 0x20, UT_CP},
	{"OutletId"           , 0x21, UT_SV},
	{"Gang"               , 0x22, UT_CL|UT_CP},
	{"GangId"             , 0x23, UT_SV},
	{"PowerSummary"       , 0x24, UT_CL|UT_CP},
	{"PowerSummaryId"     , 0x25, UT_SV},
	{"Voltage"            , 0x30, UT_DV},
	{"Current"            , 0x31, UT_DV},
	{"Frequency"          , 0x32, UT_DV},
	{"ApparentPower"      , 0x33, UT_DV},
	{"ActivePower"        , 0x34, UT_DV},
	{"PercentLoad"        , 0x35, UT_DV},
	{"Temperature"        , 0x36, UT_DV},
	{"Humidity"           , 0x37, UT_DV},
	{"BadCount"           , 0x38, UT_DV},
	{"ConfigVoltage"      , 0x40, UT_SV|UT_DV},
	{"ConfigCurrent"      , 0x41, UT_SV|UT_DV},
	{"ConfigFrequency"    , 0x42, UT_SV|UT_DV},
	{"ConfigApparentPower", 0x43, UT_SV|UT_DV},
	{"ConfigActivePower"  , 0x44, UT_SV|UT_DV},
	{"ConfigPercentLoad"  , 0x45, UT_SV|UT_DV},
	{"ConfigTemperature"  , 0x46, UT_SV|UT_DV},
	{"ConfigHumidity"     , 0x47, UT_SV|UT_DV},
	{"SwitchOnControl"    , 0x50, UT_DV},
	{"SwitchOffControl"   , 0x51, UT_DV},
	{"ToggleControl"      , 0x52, UT_DV},
	{"LowVoltageTransfer" , 0x53, UT_DV},
	{"HighVoltageTransfer", 0x54, UT_DV},
	{"DelayBeforeReboot"  , 0x55, UT_DV},
	{"DelayBeforeStartup" , 0x56, UT_DV},
	{"DelayBeforeShutdown", 0x57, UT_DV},
	{"Test"               , 0x58, UT_DV},
	{"ModuleReset"        , 0x59, UT_DV},
	{"AudibleAlarmControl", 0x5A, UT_DV},
	{"Present"            , 0x60, UT_DF},
	{"Good"               , 0x61, UT_DF},
	{"InternalFailure"    , 0x62, UT_DF},
	{"VoltageOutOfRange"  , 0x63, UT_DF},
	{"FrequencyOutOfRange", 0x64, UT_DF},
	{"Overload"           , 0x65, UT_DF},
	{"OverCharged"        , 0x66, UT_DF},
	{"OverTemperature"    , 0x67, UT_DF},
	{"ShutdownRequested"  , 0x68, UT_DF},
	{"ShutdownImminent"   , 0x69, UT_DF},
	{"SwitchOnOff"        , 0x6B, UT_DF},
	{"Switchable"         , 0x6C, UT_DF},
	{"Used"               , 0x6D, UT_DF},
	{"Boost"              , 0x6E, UT_DF},
	{"Buck"               , 0x6F, UT_DF},
	{"Initialized"        , 0x70, UT_DF},
	{"Tested"             , 0x71, UT_DF},
	{"AwaitingPower"      , 0x72, UT_DF},
	{"CommunicationLost"  , 0x73, UT_DF},
	{"IManufacturer"      , 0xFD, UT_SV},
	{"IProduct"           , 0xFE, UT_SV},
	{"ISerialNumber"      , 0xFF, UT_SV},
	endOfMap
};


/**
 * HID descriptor usage camera control argument token encoding map.
 * 
 * @see HID Point of Sale Usage Tables 1.02 ch. 3
 */
constexpr const Encoding barcodeMap[] = {
	{"BarCodeBadgeReader"                        , 0x01, UT_CA},
	{"BarCodeScanner"                            , 0x02, UT_CA},
	{"DumbBarCodeScanner"                        , 0x03, UT_CA},
	{"CordlessScannerBase"                       , 0x04, UT_CA},
	{"BarCodeScannerCradle"                      , 0x05, UT_CA},
	{"AttributeReport"                           , 0x10, UT_CL},
	{"SettingsReport"                            , 0x11, UT_CL},
	{"ScannedDataReport"                         , 0x12, UT_CL},
	{"RawScannedDataReport"                      , 0x13, UT_CL},
	{"TriggerReport"                             , 0x14, UT_CL},
	{"StatusReport"                              , 0x15, UT_CL},
	{"UpsEanControlReport"                       , 0x16, UT_CL},
	{"Ean23LabelControlReport"                   , 0x17, UT_CL},
	{"Code39ControlReport"                       , 0x18, UT_CL},
	{"Interleaved2Of5ControlReport"              , 0x19, UT_CL},
	{"Standard2Of5ConrolReport"                  , 0x1A, UT_CL},
	{"MsiPlesseyControlReport"                   , 0x1B, UT_CL},
	{"CodabarControlReport"                      , 0x1C, UT_CL},
	{"Code128ControlReport"                      , 0x1D, UT_CL},
	{"Misc2dConrolReport"                        , 0x1E, UT_CL},
	{"Control2dReport"                           , 0x1F, UT_CL}, /* changed name to avoid leading digit */
	{"AimingPoinerMode"                          , 0x30, UT_SF},
	{"BarCodePresentSensor"                      , 0x31, UT_SF},
	{"Class1aLaser"                              , 0x32, UT_SF},
	{"Class2Laser"                               , 0x33, UT_SF},
	{"HeaterPresent"                             , 0x34, UT_SF},
	{"ContactScanner"                            , 0x35, UT_SF},
	{"ElectronicArticleSurveillanceNotification" , 0x36, UT_SF},
	{"ConstantElectronicArticleSurveillance"     , 0x37, UT_SF},
	{"ErrorIndication"                           , 0x38, UT_SF},
	{"FixedBeeper"                               , 0x39, UT_SF},
	{"GoodDecoderIndication"                     , 0x3A, UT_SF},
	{"HandsFreeScanning"                         , 0x3B, UT_SF},
	{"IntrinsicallySafe"                         , 0x3C, UT_SF},
	{"KlasseEinsLaser"                           , 0x3D, UT_SF},
	{"LongRangeScanner"                          , 0x3E, UT_SF},
	{"MirrorSpeedControl"                        , 0x3F, UT_SF},
	{"NotOnFileIndication"                       , 0x40, UT_SF},
	{"ProgrammableBeeper"                        , 0x41, UT_SF},
	{"Triggerless"                               , 0x42, UT_SF},
	{"Wand"                                      , 0x43, UT_SF},
	{"WaterResistant"                            , 0x44, UT_SF},
	{"MultiRangeScanner"                         , 0x45, UT_SF},
	{"ProximitySensor"                           , 0x46, UT_SF},
	{"FragmentDecoder"                           , 0x4D, UT_DF},
	{"ScannerReadConfidence"                     , 0x4E, UT_DV},
	{"DataPrefix"                                , 0x4F, UT_NARY},
	{"PrefixAimi"                                , 0x50, UT_SEL},
	{"PrefixNone"                                , 0x51, UT_SEL},
	{"PrefixProprietary"                         , 0x52, UT_SEL},
	{"ActiveTime"                                , 0x55, UT_DV},
	{"AimingLaserPattern"                        , 0x56, UT_DF},
	{"BarCodePresent"                            , 0x57, UT_OOC},
	{"BeeperState"                               , 0x58, UT_OOC},
	{"LaserOnTime"                               , 0x59, UT_DV},
	{"LaserState"                                , 0x5A, UT_OOC},
	{"LockoutTime"                               , 0x5B, UT_DV},
	{"MotorState"                                , 0x5C, UT_OOC},
	{"MotorTimeout"                              , 0x5D, UT_DV},
	{"PowerOnResetScanner"                       , 0x5E, UT_DF},
	{"PreventReadOfBarcodes"                     , 0x5F, UT_DF},
	{"InitiateBarcodeRead"                       , 0x60, UT_DF},
	{"TriggerState"                              , 0x61, UT_OOC},
	{"TriggerMode"                               , 0x62, UT_NARY},
	{"TriggerModeBlinkingLaserOn"                , 0x63, UT_SEL},
	{"TriggerModeContinuousLaserOn"              , 0x64, UT_SEL},
	{"TriggerModeLaserOnWhilePulled"             , 0x65, UT_SEL},
	{"TriggerModeLaserStaysOnAfterTriggerRelease", 0x66, UT_SEL},
	{"CommitParametersToNvm"                     , 0x6D, UT_DF},
	{"ParameterScanning"                         , 0x6E, UT_DF},
	{"ParametersChanged"                         , 0x6F, UT_OOC},
	{"SetParameterDefaultValues"                 , 0x70, UT_DF},
	{"ScannerInCradle"                           , 0x75, UT_OOC},
	{"ScannerInRange"                            , 0x76, UT_OOC},
	{"AimDuration"                               , 0x7A, UT_DV},
	{"GoodReadLampDuration"                      , 0x7B, UT_DV},
	{"GoodReadLampIntensity"                     , 0x7C, UT_DV},
	{"GoodReadLed"                               , 0x7D, UT_DF},
	{"GoodReadToneFrequency"                     , 0x7E, UT_DV},
	{"GoodReadToneLength"                        , 0x7F, UT_DV},
	{"GoodReadToneVolume"                        , 0x80, UT_DV},
	{"NoReadMessage"                             , 0x82, UT_DF},
	{"NotOnFileVolume"                           , 0x83, UT_DV},
	{"PowerupBeep"                               , 0x84, UT_DF},
	{"SoundErrorBeep"                            , 0x85, UT_DF},
	{"SoundGoodReadBeep"                         , 0x86, UT_DF},
	{"SoundNotOnFileBeep"                        , 0x87, UT_DF},
	{"GoodReadWhenToWrite"                       , 0x88, UT_NARY},
	{"GrwtiAfterDecode"                          , 0x89, UT_SEL},
	{"GrwtiBeepLampAferTransmit"                 , 0x8A, UT_SEL},
	{"GrwtiNoBeepLampUseAtAll"                   , 0x8B, UT_SEL},
	{"BooklandEan"                               , 0x91, UT_DF},
	{"ConvertEan8To13Type"                       , 0x92, UT_DF},
	{"ConvertUpcAToEan13"                        , 0x93, UT_DF},
	{"ConvertUpcEToA"                            , 0x94, UT_DF},
	{"Ean13"                                     , 0x95, UT_DF},
	{"Ean8"                                      , 0x96, UT_DF},
	{"Ean99128Mandatory"                         , 0x97, UT_DF},
	{"Ean99P5128Optional"                        , 0x98, UT_DF},
	{"UpcEan"                                    , 0x9A, UT_DF},
	{"UpcEanCouponCode"                          , 0x9B, UT_DF},
	{"UpcEanPeriodicals"                         , 0x9C, UT_DV},
	{"UpcA"                                      , 0x9D, UT_DF},
	{"UpcAWith128Mandatory"                      , 0x9E, UT_DF},
	{"UpcAWith128Optional"                       , 0x9F, UT_DF},
	{"UpcAWithP5Optional"                        , 0xA0, UT_DF},
	{"UpcE"                                      , 0xA1, UT_DF},
	{"UpcE1"                                     , 0xA2, UT_DF},
	{"Periodical"                                , 0xA9, UT_NARY},
	{"PeriodicalAutoDiscriminatePlus2"           , 0xAA, UT_SEL},
	{"PeriodicalOnlyDecodeWidthPlus2"            , 0xAB, UT_SEL},
	{"PeriodicalIgnorePlus2"                     , 0xAC, UT_SEL},
	{"PeriodicalAutoDiscriminatePlus5"           , 0xAD, UT_SEL},
	{"PeriodicalOnlyDecodeWidthPlus5"            , 0xAE, UT_SEL},
	{"PeriodicalIgnorePlus5"                     , 0xAF, UT_SEL},
	{"Check"                                     , 0xB0, UT_NARY},
	{"CheckDisablePrice"                         , 0xB1, UT_SEL},
	{"CheckEnable4DigitPrice"                    , 0xB2, UT_SEL},
	{"CheckEnable5DigitPrice"                    , 0xB3, UT_SEL},
	{"CheckEnableEuropean4DigitPrice"            , 0xB4, UT_SEL},
	{"CheckEnableEuropean5DigitPrice"            , 0xB5, UT_SEL},
	{"EanTwoLabel"                               , 0xB7, UT_DF},
	{"EanThreeLabel"                             , 0xB8, UT_DF},
	{"Ean8FlagDigit1"                            , 0xB9, UT_DV},
	{"Ean8FlagDigit2"                            , 0xBA, UT_DV},
	{"Ean8FlagDigit3"                            , 0xBB, UT_DV},
	{"Ean13FlagDigit1"                           , 0xBC, UT_DV},
	{"Ean13FlagDigit2"                           , 0xBD, UT_DV},
	{"Ean13FlagDigit3"                           , 0xBE, UT_DV},
	{"AddEan23LabelDefinition"                   , 0xBF, UT_DF},
	{"ClearAllEan23LabelDefinitions"             , 0xC0, UT_DF},
	{"Codabar"                                   , 0xC3, UT_DF},
	{"Code128"                                   , 0xC4, UT_DF},
	{"Code39"                                    , 0xC7, UT_DF},
	{"Code93"                                    , 0xC8, UT_DF},
	{"FullAsciiConversion"                       , 0xC9, UT_DF},
	{"Interleaved2Of5"                           , 0xCA, UT_DF},
	{"ItalianPharmacyCode"                       , 0xCB, UT_DF},
	{"MsiPlessey"                                , 0xCC, UT_DF},
	{"Standard2Of5Iata"                          , 0xCD, UT_DF},
	{"Standard2Of5"                              , 0xCE, UT_DF},
	{"TransmitStartStop"                         , 0xD3, UT_DF},
	{"TriOptic"                                  , 0xD4, UT_DF},
	{"UccEan128"                                 , 0xD5, UT_DF},
	{"CheckDigit"                                , 0xD6, UT_NARY},
	{"CheckDigitDisable"                         , 0xD7, UT_SEL},
	{"CheckDigitEnableInerleaved2Of5Opcc"        , 0xD8, UT_SEL},
	{"CheckDigitEnableInterleaved2Of5Uss"        , 0xD9, UT_SEL},
	{"CheckDigitEnableStandard2Of5Opcc"          , 0xDA, UT_SEL},
	{"CheckDigitEnableStandard2Of5Uss"           , 0xDB, UT_SEL},
	{"CheckDigitEnableOneMsiPlessey"             , 0xDC, UT_SEL},
	{"CheckDigitEnableTwoMsiPlessey"             , 0xDD, UT_SEL},
	{"CheckDigitCodabarEnable"                   , 0xDE, UT_SEL},
	{"CheckDigitCode39Enable"                    , 0xDF, UT_SEL},
	{"TransmitCheckDigit"                        , 0xF0, UT_NARY},
	{"DisableCheckDigitTransmit"                 , 0xF1, UT_SEL},
	{"EnableCheckDigitTransmit"                  , 0xF2, UT_SEL},
	{"SymbologyIdentifier1"                      , 0xFB, UT_DV},
	{"SymbologyIdentifier2"                      , 0xFC, UT_DV},
	{"SymbologyIdentifier3"                      , 0xFD, UT_DV},
	{"DecodedData"                               , 0xFE, UT_DV},
	{"DecodedDataContinued"                      , 0xFF, UT_DF},
	{"BarSpaceData"                              , 0x100, UT_DV},
	{"ScannerDataAccuracy"                       , 0x101, UT_DV},
	{"RawDataPolarity"                           , 0x102, UT_NARY},
	{"PolarityInvertedBarCode"                   , 0x103, UT_SEL},
	{"PolarityNormalBarCode"                     , 0x104, UT_SEL},
	{"MinimumLengthToDecode"                     , 0x106, UT_DV},
	{"MaximumLengthToDecode"                     , 0x107, UT_DV},
	{"FirstDiscreteLengthToDecode"               , 0x108, UT_DV},
	{"SecondDiscreteLengthToDecode"              , 0x109, UT_DV},
	{"DataLengthMethod"                          , 0x10A, UT_NARY},
	{"DlMethodReadAny"                           , 0x10B, UT_SEL},
	{"DlMethodCheckInRange"                      , 0x10C, UT_SEL},
	{"DlMethodCheckForDiscrete"                  , 0x10D, UT_SEL},
	{"AztecCode"                                 , 0x110, UT_DF},
	{"Bc412"                                     , 0x111, UT_DF},
	{"ChannelCode"                               , 0x112, UT_DF},
	{"Code16"                                    , 0x113, UT_DF},
	{"Code32"                                    , 0x114, UT_DF},
	{"Code49"                                    , 0x115, UT_DF},
	{"CodeOne"                                   , 0x116, UT_DF},
	{"ColorCode"                                 , 0x117, UT_DF},
	{"DataMatrix"                                , 0x118, UT_DF},
	{"MaxiCode"                                  , 0x119, UT_DF},
	{"MicroPdf"                                  , 0x11A, UT_DF},
	{"Pdf417"                                    , 0x11B, UT_DF},
	{"PosiCode"                                  , 0x11C, UT_DF},
	{"QrCode"                                    , 0x11D, UT_DF},
	{"SuperCode"                                 , 0x11E, UT_DF},
	{"UltraCode"                                 , 0x11F, UT_DF},
	{"Usd5SlugCode"                              , 0x120, UT_DF},
	{"VeriCode"                                  , 0x121, UT_DF},
	endOfMap
};


/**
 * HID descriptor usage weighing devices argument token encoding map.
 * 
 * @see HID Point of Sale Usage Tables 1.02 ch. 4
 */
constexpr const Encoding weightDevMap[] = {
	{"WeighingDevice"                 , 0x01, UT_CA},
	{"ScaleDevice"                    , 0x20, UT_CL},
	{"ScaleClass"                     , 0x21, UT_CL}, /* renamed according to name in ch. 4.2 */
	{"ScaleClassIMetric"              , 0x22, UT_SEL},
	{"ScaleClassIiMetric"             , 0x23, UT_SEL},
	{"ScaleClassIiiMetric"            , 0x24, UT_SEL},
	{"ScaleClassIiilMetric"           , 0x25, UT_SEL},
	{"ScaleClassIvMetric"             , 0x26, UT_SEL},
	{"ScaleClassIiiEnglish"           , 0x27, UT_SEL},
	{"ScaleClassIiilEnglish"          , 0x28, UT_SEL},
	{"ScaleClassIvEnglish"            , 0x29, UT_SEL},
	{"ScaleClassGeneric"              , 0x2A, UT_SEL},
	{"ScaleAttributeReport"           , 0x30, UT_CL},
	{"ScaleControlReport"             , 0x31, UT_CL},
	{"ScaleDataReport"                , 0x32, UT_CL},
	{"ScaleStatusReport"              , 0x33, UT_CL},
	{"ScaleWeightLimitReport"         , 0x34, UT_CL},
	{"ScaleStatisticsReport"          , 0x35, UT_CL},
	{"DataWeight"                     , 0x40, UT_DV},
	{"DataScaling"                    , 0x41, UT_DV}, /* changed usage type to match similar usage IDs */
	{"WeightUnit"                     , 0x50, UT_CL},
	{"WeightUnitMilligram"            , 0x51, UT_SEL},
	{"WeightUnitGram"                 , 0x52, UT_SEL},
	{"WeightUnitKilogram"             , 0x53, UT_SEL},
	{"WeightUnitCarats"               , 0x54, UT_SEL},
	{"WeightUnitTaels"                , 0x55, UT_SEL},
	{"WeightUnitGrains"               , 0x56, UT_SEL},
	{"WeightUnitPennyweights"         , 0x57, UT_SEL},
	{"WeightUnitMetricTon"            , 0x58, UT_SEL},
	{"WeightUnitAvoirTon"             , 0x59, UT_SEL},
	{"WeightUnitTroyOunce"            , 0x5A, UT_SEL},
	{"WeightUnitOunce"                , 0x5B, UT_SEL},
	{"WeightUnitPound"                , 0x5C, UT_SEL},
	{"CalibrationCount"               , 0x60, UT_DV},
	{"ReZeroCount"                    , 0x61, UT_DV},
	{"ScaleStatus"                    , 0x70, UT_CL},
	{"ScaleStatusFault"               , 0x71, UT_SEL},
	{"ScaleStatusStableAtCenterOfZero", 0x72, UT_SEL},
	{"ScaleStatusInMotion"            , 0x73, UT_SEL},
	{"ScaleStatusWeightStable"        , 0x74, UT_SEL},
	{"ScaleStatusUnderZero"           , 0x75, UT_SEL},
	{"ScaleStatusOverWeightLimit"     , 0x76, UT_SEL},
	{"ScaleStatusRequiresCalibration" , 0x77, UT_SEL},
	{"ScaleStatusRequiresRezeroing"   , 0x78, UT_SEL},
	{"ZeroScale"                      , 0x80, UT_OOC},
	{"EnforcedZeroReturn"             , 0x81, UT_OOC},
	endOfMap
};


/**
 * HID descriptor usage magnetic stripe reader (MSR) argument token encoding map.
 * 
 * @see HID Point of Sale Usage Tables 1.02 ch. 5
 */
constexpr const Encoding msrMap[] = {
	{"MsrDeviceReadOnly", 0x01, UT_CA},
	{"Track1Length"     , 0x11, UT_SF|UT_DF|UT_SEL},
	{"Track2Length"     , 0x12, UT_SF|UT_DF|UT_SEL},
	{"Track3Length"     , 0x13, UT_SF|UT_DF|UT_SEL},
	{"TrackJisLength"   , 0x14, UT_SF|UT_DF|UT_SEL},
	{"TrackData"        , 0x20, UT_SF|UT_DF|UT_SEL},
	{"Track1Data"       , 0x21, UT_SF|UT_DF|UT_SEL},
	{"Track2Data"       , 0x22, UT_SF|UT_DF|UT_SEL},
	{"Track3Data"       , 0x23, UT_SF|UT_DF|UT_SEL},
	{"TrackJisData"     , 0x24, UT_SF|UT_DF|UT_SEL},
	endOfMap
};


/**
 * HID descriptor usage camera control argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 25
 */
constexpr const Encoding cameraCtrlMap[] = {
	{"CameraAutoFocus", 0x20, UT_OSC},
	{"CameraShutter",   0x21, UT_OSC},
	endOfMap
};


/**
 * HID descriptor usage arcade argument token encoding map.
 * 
 * @see Open Arcade Architecture Device Data Format Specification 1.100 ch. 2
 */
constexpr const Encoding arcadeMap[] = {
	{"GeneralPurposeIoCard"            , 0x01, UT_CA},
	{"CoinDoor"                        , 0x02, UT_CA},
	{"WatchdogTimer"                   , 0x03, UT_CA},
	{"GeneralPurposeAnalogInputState"  , 0x30, UT_DV},
	{"GeneralPurposeDigitalInputState" , 0x31, UT_DV},
	{"GeneralPurposeOpticalInputState" , 0x32, UT_DV},
	{"GeneralPurposeDigitalOutputState", 0x33, UT_DV},
	{"NumberOfCoinDoors"               , 0x34, UT_DV},
	{"CoinDrawerDropCount"             , 0x35, UT_DV},
	{"CoinDrawerDropStart"             , 0x36, UT_OOC},
	{"CoinDrawerDropService"           , 0x37, UT_OOC},
	{"CoinDrawerDropTilt"              , 0x38, UT_OOC},
	{"CoinDoorTest"                    , 0x39, UT_OOC},
	{"CoinDoorLockout"                 , 0x40, UT_OOC},
	{"WatchdogTimeout"                 , 0x41, UT_DV},
	{"WatchdogAction"                  , 0x42, UT_NARY},
	{"WatchdogReboot"                  , 0x43, UT_SEL},
	{"WatchdogRestart"                 , 0x44, UT_SEL},
	{"AlarmInput"                      , 0x45, UT_DV},
	{"CoinDoorCounter"                 , 0x46, UT_OOC},
	{"IoDirectionMapping"              , 0x47, UT_DV},
	{"SetIoDirection"                  , 0x48, UT_OOC},
	{"ExtendedOpticalInputState"       , 0x49, UT_DV},
	{"PinPadInputState"                , 0x4A, UT_DV},
	{"PinPadStatus"                    , 0x4B, UT_DV},
	{"PinPadOutput"                    , 0x4C, UT_OOC},
	{"PinPadCommand"                   , 0x4D, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage FIDO alliance argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 27
 */
constexpr const Encoding fidoMap[] = {
	{"U2fAuthenticatorDevice", 0x01, UT_CA},
	{"InputReportData",        0x20, UT_DV},
	{"OutputReportData",       0x21, UT_DV},
	endOfMap
};


/**
 * HID descriptor usage page item argument token encoding map.
 * 
 * @see HID Usage Tables 1.2 ch. 3
 */
constexpr const Encoding usagePageMap[] = {
	{"GenericDesktop"             , 0x01, genDeskMap},
	{"SimulationControls"         , 0x02, simCtrlMap},
	{"VrControls"                 , 0x03, vrCtrlMap},
	{"SportControls"              , 0x04, sportCtrlMap},
	{"GameControls"               , 0x05, gameCtrlMap},
	{"GenericDeviceControls"      , 0x06, genDevCtrlMap},
	{"Keyboard"                   , 0x07, keyboardMap},
	{"Led"                        , 0x08, ledMap},
	{"Button"                     , 0x09, buttonMap},
	{"Ordinal"                    , 0x0A, ordinalMap},
	{"TelephonyDevice"            , 0x0B, telDevMap},
	{"Consumer"                   , 0x0C, consumerMap},
	{"Digitizers"                 , 0x0D, digitizersMap},
	{"Haptics"                    , 0x0E, hapticsMap},
	{"Pid"                        , 0x0F, pidMap},
	{"Unicode"                    , 0x10, unicodeMap},
	{"EyeAndHeadTrackers"         , 0x12, eyeHeadMap},
	{"AuxiliaryDisplay"           , 0x14, auxDisplayMap},
	{"Sensors"                    , 0x20, sensorMap},
	{"MediacalInstrument"         , 0x40, medInstMap},
	{"BrailleDisplay"             , 0x41, brailleMap},
	{"LightingAndIllumination"    , 0x59, lightMap},
	{"Monitor"                    , 0x80, monitorMap},
	{"MonitorEnumeratedValues"    , 0x81, monitorEnumMap}, /* Monitor Control Class Specification 1.0 ch. 6 */
	{"VesaVirtualControls"        , 0x82, vesaCtrlMap}, /* Monitor Control Class Specification 1.0 ch. 6 */
	{"Power"                      , 0x84, pwrDevMap},
	{"BarCodeScanner"             , 0x8C, barcodeMap},
	{"WeighingDevices"            , 0x8D, weightDevMap},
	{"MagneticStripeReaderDevices", 0x8E, msrMap},
	{"CameraControl"              , 0x90, cameraCtrlMap},
	{"Arcade"                     , 0x91, arcadeMap},
	{"GamingDevice"               , 0x92},
	{"FidoAlliance"               , 0xF1D0, fidoMap},
	endOfMap
};


/**
 * HID descriptor item token encoding map.
 */
constexpr const Encoding itemMap[] = {
	/* HID 1.11 ch. 6.2.2.4 */
	{"Input"            , 0x80, inputArgMap},
	{"Output"           , 0x90, outputFeatureArgMap},
	{"Feature"          , 0xB0, outputFeatureArgMap},
	{"Collection"       , 0xA0, colArgMap},
	{"EndCollection"    , 0xC0, endCol},
	/* HID 1.11 ch. 6.2.2.7 */
	{"UsagePage"        , 0x04, usagePageMap},
	{"LogicalMinimum"   , 0x14, signedNumArg},
	{"LogicalMaximum"   , 0x24, signedNumArg},
	{"PhysicalMinimum"  , 0x34, signedNumArg},
	{"PhysicalMaximum"  , 0x44, signedNumArg},
	{"UnitExponent"     , 0x54, unitExpMap},
	{"Unit"             , 0x64, unitSystemMap},
	{"ReportSize"       , 0x74, numArg},
	{"ReportId"         , 0x84, numArg},
	{"ReportCount"      , 0x94, numArg},
	{"Push"             , 0xA4},
	{"Pop"              , 0xB4},
	/* HID 1.11 ch. 6.2.2.8 (all local items are unsigned integers) */
	{"Usage"            , 0x08, usageArg},
	{"UsageMinimum"     , 0x18, usageArg},
	{"UsageMaximum"     , 0x28, usageArg},
	{"DesignatorIndex"  , 0x38, numArg},
	{"DesignatorMinimum", 0x48, numArg},
	{"DesignatorMaximum", 0x58, numArg},
	{"StringIndex"      , 0x78, numArg},
	{"StringMinimum"    , 0x88, numArg},
	{"StringMaximum"    , 0x98, numArg},
	{"Delimiter"        , 0xA8, delimMap},
	endOfMap
};


/**
 * Searches for an encoding in the given map which matches the passed
 * token. The token is matched case in-sensitive.
 * 
 * @param[in] token - token to find
 * @param[in] map - search within this map
 * @param[out] res - variable to store dynamic results
 * @param[in,out] error - variable to receive a possible parsing error
 * @return map entry pointer if found or NULL
 */
constexpr static const Encoding * findEncoding(const Token & token, const Encoding * map, Encoding & res, error::EMessage & error) noexcept {
	using namespace ::hid::error;
	if (token.length == 0) {
		return NULL;
	}
	size_t i = 0;
	while (map->name != NULL) {
		if ( equalsI(token, map->name) ) {
			error = E_NO_ERROR;
			return map;
		}
		if (i < 3) {
			const int idx = strFindChr(map->name, '#');
			if (idx >= 0) {
				/* handle argument with index */
				if (map->name[idx + 1] != 0 || ( ! equals(map[0].name, map[1].name) )) {
					/* invalid index map item */
					error = E_Internal_error;
					return NULL;
				}
				if (token.length <= size_t(idx) || ( ! startWidthIN(map->name, size_t(idx), token.start) )) {
					/* name does not match */
					error = E_Invalid_argument_name;
					return NULL;
				}
				uint32_t num = 0;
				for (size_t n = size_t(idx); n < token.length; n++) {
					const char c = token.start[n];
					if ( ! isDigit(c) ) {
						/* not a numeric value */
						error = E_Unexpected_argument_name_character;
						return NULL;
					}
					const uint32_t oldNum = num;
					num = uint32_t((num * 10) + c - '0');
					if (oldNum > num) {
						/* number overflow */
						error = E_Argument_index_out_of_range;
						return NULL;
					}
				}
				if (num < map[0].value || num > map[1].value) {
					/* out of allowed value range */
					error = E_Argument_index_out_of_range;
					return NULL;
				}
				if (num != 0 && token.start[idx] == '0') {
					/* leading zeros are not allowed */
					error = E_Invalid_argument_name;
					return NULL;
				}
				res.name = map->name;
				res.value = num;
				error = E_NO_ERROR;
				return &res;
			}
		}
		map++;
		i++;
	}
	return NULL;
}


/**
 * Compiles the HID description into the given buffer.
 * 
 * @param[in] source - source code description
 * @param[out] out - output writer instance
 * @param[out] error - possible error
 * @return true on success, else false
 * @tparam Source - shall implement `size_t size()`, `const char * data()` and `ParamMatch find(Token)`
 * @tparam Writer - shall implement `write(uint8_t)`
 */
template <typename Source, typename Writer>
constexpr bool compile(const Source & source, Writer & out, ::hid::error::Info & error) noexcept {
	using namespace ::hid::error;
	enum State {
		HID_START                 = 0x000,
		HID_WITHIN_COMMENT        = 0x001,
		HID_WITHIN_ITEM           = 0x002,
		HID_WITHIN_ARG_LIST       = 0x004,
		HID_WITHIN_ARG            = 0x008,
		HID_WITHIN_PARAM          = 0x010,
		HID_WITHIN_HEX_LIT        = 0x020,
		HID_WITHIN_NUM_LIT        = 0x040,
		HID_WITHIN_UNIT_SYS       = 0x080,
		HID_WITHIN_UNIT_DESC      = 0x100,
		HID_WITHIN_UNIT           = 0x200,
		HID_WITHIN_UNIT_EXP       = 0x400
	};
#define _HID_WITHIN(x) ((flags & HID_WITHIN_##x) != 0)
	int colLevel = 0;
	int delimLevel = 0;
	int usageAtLevel = -1;
	size_t reportSizes = 0;
	size_t reportCounts = 0;
	const char * ptr = source.data();
	const size_t len = source.size();
	ErrorWriter errorMsg{ptr, error};
	Token tItem = {ptr, 0};
	Token tArg = {ptr, 0};
	bool hasUsagePage{false};
	bool hasArg{false};
	bool multiArg{false};
	bool negLit{false};
	EMessage subError{E_NO_ERROR};
	Encoding dynMap;
	const Encoding * encMap{NULL}; /* current */
	const Encoding * usagePage{NULL}; /* current; used for all subsequent Usage items, regardless of the hierarchy */
	const Encoding * encUnit{NULL}; /* current */
	uint32_t flags = HID_START;
	uint32_t item{0}, arg{0}, lit{0};
	size_t n{0};
	for (; n < len && *ptr != 0; ) {
#ifdef HID_DESCRIPTOR_DEBUG
		constexpr const char * flagsStr[] = {"COMMENT", "ITEM", "ARG_LIST", "ARG", "PARAM", "HEX_LIT", "NUM_LIT", "UNIT_SYS", "UNIT_DESC", "UNIT", "UNIT_EXP"};
		printf("in: %3u, out: %3u, c:", unsigned(n), unsigned(out.getPosition()));
		if (isprint(*ptr) != 0) {
			printf(" '%c'", *ptr);
		} else {
			printf("    ");
		}
		if (flags == HID_START) {
			printf(", flags = START");
		} else {
			bool first = true;
			for (size_t i = 0; i < (sizeof(flagsStr) / sizeof(*flagsStr)); i++) {
				if (((flags >> i) & 1) != 0) {
					printf("%s%s", first ? ", flags = " : " | ", flagsStr[i]);
					first = false;
				}
			}
			if ( multiArg ) {
				printf(" | MULTI_ARG");
			}
			if ( negLit ) {
				printf(" | NEG_LIT");
			}
			if ( hasArg ) {
				printf(" | HAS_ARG");
			}
		}
		printf("\n");
#endif /* HID_DESCRIPTOR_DEBUG */
		if (flags == HID_START) {
			if ( isItemChar(*ptr) ) {
				/* start of item name */
				flags = HID_WITHIN_ITEM;
				tItem.start = ptr;
				tItem.length = 1;
			} else if (*ptr == '{') {
				/* start of user parameter */
				flags = HID_WITHIN_PARAM;
				tArg.start = ptr + 1;
				tArg.length = 0;
			} else if (ptr[0] == '0' && (n + 1) < len && ptr[1] == 'x') {
				/* start of hex literal */
				flags = HID_WITHIN_HEX_LIT;
				if ((n + 2) >= len) {
					return errorMsg.at(n + 2, E_Unexpected_end_of_source);
				}
				if ( ! isHexDigit(ptr[2]) ) {
					return errorMsg.at(n + 2, E_Invalid_hex_value);
				}
				lit = 0;
				n++;
				ptr++;
			} else if ( isDigit(*ptr) ) {
				/* start of number literal */
				/* note: negative number literals are only allowed as argument */
				flags = HID_WITHIN_NUM_LIT;
				lit = 0;
				continue; /* re-parse as number literal */
			} else if (*ptr == '-') {
				return errorMsg.at(n, E_Negative_numbers_are_not_allowed_in_this_context);
			} else if ( isComment(*ptr) ) {
				flags = HID_WITHIN_COMMENT;
			} else if ( ! isWhitespace(*ptr) ) {
				return errorMsg.at(n, E_Unexpected_token);
			}
		} else if ( _HID_WITHIN(COMMENT) ) {
			if (*ptr == '\r' || *ptr == '\n') {
				flags = HID_START;
			}
		} else if ( _HID_WITHIN(PARAM) ) {
			if (*ptr == '}') {
				/* end of user parameter */
				flags &= ~HID_WITHIN_PARAM;
				const ParamMatch & param = source.find(tArg);
				if ( ! param.valid ) {
					return errorMsg.at(n, E_Expected_valid_parameter_name_here);
				}
				if ( _HID_WITHIN(ARG_LIST) ) {
					/* merge multiple arguments via OR */
					if (encMap->arg == signedNumArg) {
						if (param.value < INT64_C(-0x80000000) || param.value > INT64_C(0x7FFFFFFF)) {
							return errorMsg.at(n, E_Parameter_value_out_of_range);
						}
					} else {
						if (param.value < 0 || param.value > UINT32_C(0xFFFFFFFF)) {
							return errorMsg.at(n, E_Parameter_value_out_of_range);
						}
					}
					arg |= uint32_t(param.value);
					hasArg = true;
				} else {
					/* encode as literal */
					if (param.value < 0) {
						return errorMsg.at(n, E_Negative_numbers_are_not_allowed_in_this_context);
					}
					if (param.value > UINT32_C(0xFFFFFFFF)) {
						return errorMsg.at(n, E_Parameter_value_out_of_range);
					}
					encodeUnsigned(out, uint32_t(param.value));
				}
			} else {
				tArg.length++;
			}
		} else if ( _HID_WITHIN(ITEM) ) {
			if ( isItemChar(*ptr) ) {
				tItem.length++;
			} else if (isWhitespace(*ptr) || *ptr == '(') {
				/* skip whitespaces */
				if ( isWhitespace(*ptr) ) {
					while ((n + 1) < len && isWhitespace(ptr[1])) {
						n++;
						ptr++;
					}
					if ((n + 1) < len && ptr[1] == '(') {
						n++;
						ptr++;
					}
				}
				flags &= ~HID_WITHIN_ITEM;
				subError = E_Invalid_item_name;
				encMap = findEncoding(tItem, itemMap, dynMap, subError);
				if (encMap == NULL) {
					return errorMsg.at(n, subError);
				} else if (encMap->arg == colArgMap) {
					/* Collection */
					if (usageAtLevel != colLevel) {
						return errorMsg.at(n, E_Missing_Usage_for_Collection);
					}
					colLevel++;
				} else if (encMap->arg == endCol) {
					/* EndCollection */
					if (colLevel <= 0) {
						return errorMsg.at(n, E_Unexpected_EndCollection);
					}
					if (reportSizes < reportCounts) {
						return errorMsg.at(n, E_Missing_ReportSize);
					} else if (reportCounts < reportSizes) {
						return errorMsg.at(n, E_Missing_ReportCount);
					}
					colLevel--;
					usageAtLevel--;
				} else if ( equalsI(tItem, "Usage") ) {
					/* needed to check if there is a Usage item for every Collection */
					usageAtLevel = colLevel;
				}
				if (*ptr == '(') {
					/* start of argument list */
					flags |= HID_WITHIN_ARG_LIST;
					if (encMap->arg == NULL) {
						return errorMsg.at(n, E_This_item_has_no_arguments);
					} else if (encMap->arg == unitSystemMap) {
						/* Unit */
						flags |= HID_WITHIN_UNIT_SYS;
					}
					/* standard item */
					item = encMap->value;
					arg = 0;
					hasArg = false;
					multiArg = (encMap->arg == inputArgMap || encMap->arg == outputFeatureArgMap);
				} else {
					/* end of item */
					if (encMap->arg != NULL && (encMap->arg->name != NULL || encMap->arg == usageArg)) {
						return errorMsg.at(n, E_Missing_argument);
					}
					encodeUnsigned(out, encMap->value);
				}
			} else {
				return errorMsg.at(n, E_Unexpected_item_name_character);
			}
		} else if ( _HID_WITHIN(ARG) ) {
			if ( _HID_WITHIN(UNIT_DESC) ) {
				if ( _HID_WITHIN(UNIT) ) {
					if ( isAlpha(*ptr) ) {
						tArg.length++;
					} else if (isWhitespace(*ptr) || *ptr == ')' || *ptr == '^') {
						/* end of unit name */
						flags &= ~HID_WITHIN_UNIT;
						subError = E_Invalid_unit_name;
						encUnit = findEncoding(tArg, encMap->arg, dynMap, subError);
						if (encUnit == NULL) {
							return errorMsg.at(n, subError);
						}
						if (*ptr == '^') {
							/* start of unit exponent */
							flags |= HID_WITHIN_UNIT_EXP;
							tArg.start = ptr + 1;
							tArg.length = 0;
						} else {
							/* end of unit without exponent (treat as exponent == 1) */
							const uint32_t offset = 4 * encUnit->value;
							arg &= ~uint32_t(0xF << offset);
							arg |= uint32_t(1 << offset);
							continue; /* re-parse as unit description */
						}
					} else {
						return errorMsg.at(n, E_Unexpected_unit_name_character);
					}
				} else if ( _HID_WITHIN(UNIT_EXP) ) {
					if (*ptr == '-') {
						/* sign is only allowed at the beginning of the exponent */
						if (tArg.length > 0) {
							return errorMsg.at(n, E_Invalid_unit_exponent);
						}
						tArg.length++;
					} else if ( isDigit(*ptr) ) {
						tArg.length++;
					} else {
						/* end of unit exponent */
						flags &= ~HID_WITHIN_UNIT_EXP;
						subError = E_Invalid_unit_exponent;
						const Encoding * encUnitExp = findEncoding(tArg, encUnit->arg, dynMap, subError);
						if (encUnitExp == NULL) {
							return errorMsg.at(n, subError);
						}
						/* the unit exponent for the current unit is stored at the specific nipple */
						const uint32_t offset = 4 * encUnit->value;
						arg &= ~uint32_t(0xF << offset);
						arg |= uint32_t(encUnitExp->value << offset);
						flags |= HID_WITHIN_UNIT_DESC;
						continue; /* re-parse as unit description */
					}
				} else if ( isAlpha(*ptr) ) {
					/* start of unit name */
					flags |= HID_WITHIN_UNIT;
					tArg.start = ptr;
					tArg.length = 1;
				} else if (*ptr == ')') {
					/* end of unit description */
					flags &= ~(HID_WITHIN_ARG | HID_WITHIN_UNIT_SYS | HID_WITHIN_UNIT_DESC);
				} else if ( ! isWhitespace(*ptr) ) {
					return errorMsg.at(n, E_Unexpected_unit_name_character);
				}
			} else if ( isArgChar(*ptr) ) {
				tArg.length++;
			} else if (_HID_WITHIN(UNIT_SYS)) {
				if ( hasArg ) {
					/* invalid internal state */
					return errorMsg.at(n, E_Internal_error);
				} else if (isWhitespace(*ptr) || *ptr == '(') {
					/* skip whitespaces */
					if ( isWhitespace(*ptr) ) {
						while ((n + 1) < len && isWhitespace(ptr[1])) {
							n++;
							ptr++;
						}
						if ((n + 1) < len && ptr[1] == '(') {
							n++;
							ptr++;
						}
					}
					/* start of unit description for the given unit system */
					subError = E_Invalid_unit_system_name;
					const Encoding * encUnitSys = findEncoding(tArg, encMap->arg, dynMap, subError);
					if (encUnitSys == NULL) {
						return errorMsg.at(n, subError);
					}
					flags |= HID_WITHIN_UNIT_DESC;
					arg = encUnitSys->value;
					encMap = encUnitSys;
					hasArg = true;
				} else if (*ptr == ')') {
					/* end of unit system */
					flags &= ~HID_WITHIN_UNIT_SYS;
					continue; /* re-parse as argument */
				} else {
					return errorMsg.at(n, E_Unexpected_argument_name_character);
				}
			} else if (isWhitespace(*ptr) || *ptr == ')' || (multiArg && *ptr == ',')) {
				/* end of argument */
				flags &= ~HID_WITHIN_ARG;
				/* possible Usage|UsageMinimum|UsageMaximum argument according to current UsagePage */
				if (encMap->arg == usageArg) {
					if (usagePage == NULL || usagePage->arg == NULL) {
						if ( hasUsagePage ) {
							return errorMsg.at(n, E_Missing_named_UsagePage);
						} else {
							return errorMsg.at(n, E_Missing_UsagePage);
						}
					}
					encMap = usagePage;
				}
				subError = E_Invalid_argument_name;
				const Encoding * encItem = findEncoding(tArg, encMap->arg, dynMap, subError);
				if (encItem == NULL) {
					return errorMsg.at(n, subError);
				} else if (encMap->arg == usagePageMap) {
					/* Usage map from UsagePage argument */
					usagePage = encItem;
				}
				if (encItem->arg == clearArg) {
					arg &= ~(encItem->value);
				} else {
					/* merge multiple arguments via OR if unspecified */
					arg |= encItem->value;
				}
				hasArg = ( ! multiArg ) || *ptr != ',';
				if (*ptr == ')') {
					continue; /* re-parse as argument list */
				}
			} else {
				return errorMsg.at(n, E_Unexpected_argument_name_character);
			}
		} else if ( _HID_WITHIN(HEX_LIT) ) {
			if ( isHexDigit(*ptr) ) {
				const uint32_t oldLit = lit;
				lit <<= 4;
				if (lit < oldLit) {
					return errorMsg.at(n, E_Number_overflow);
				}
				if (*ptr < 'A') {
					lit |= uint32_t(*ptr - '0');
				} else if (*ptr < 'a') {
					lit |= uint32_t(*ptr - 'A' + 10);
				} else {
					lit |= uint32_t(*ptr - 'a' + 10);
				}
			} else if ( _HID_WITHIN(ARG_LIST) ) {
				if (isWhitespace(*ptr) || *ptr == ')' || (multiArg && *ptr == ',')) {
					/* end of hex literal */
					flags &= ~HID_WITHIN_HEX_LIT;
					/* merge multiple arguments via OR */
					if (encMap->arg == signedNumArg && lit > 0x7FFFFFFF) {
						return errorMsg.at(n, E_Number_overflow);
					}
					arg |= lit;
					hasArg = ( ! multiArg ) || *ptr != ',';
					if (*ptr == ')') {
						continue; /* re-parse as argument list */
					}
				} else {
					return errorMsg.at(n, E_Invalid_hex_value);
				}
			} else if ( isWhitespace(*ptr) ) {
				/* end of hex literal */
				flags &= ~HID_WITHIN_HEX_LIT;
				encodeUnsigned(out, lit);
			} else {
				return errorMsg.at(n, E_Invalid_hex_value);
			}
		} else if ( _HID_WITHIN(NUM_LIT) ) {
			if ( isDigit(*ptr) ) {
				const uint32_t oldLit = lit;
				lit *= 10;
				if (lit < oldLit) {
					return errorMsg.at(n, E_Number_overflow);
				}
				lit += uint32_t(*ptr - '0');
				if (lit < oldLit) {
					return errorMsg.at(n, E_Number_overflow);
				}
			} else if ( _HID_WITHIN(ARG_LIST) ) {
				if (isWhitespace(*ptr) || *ptr == ')' || (multiArg && *ptr == ',')) {
					/* end of number literal */
					flags &= ~HID_WITHIN_NUM_LIT;
					/* merge multiple arguments via OR */
					if ( negLit ) {
						if (lit > 0x80000000) {
							return errorMsg.at(n, E_Number_overflow);
						}
						arg |= uint32_t(-int32_t(lit));
						negLit = false;
					} else {
						if (encMap->arg == signedNumArg && lit > 0x7FFFFFFF) {
							return errorMsg.at(n, E_Number_overflow);
						}
						arg |= lit;
					}
					hasArg = ( ! multiArg ) || *ptr != ',';
					if (*ptr == ')') {
						continue; /* re-parse as argument list */
					}
				} else {
					return errorMsg.at(n, E_Invalid_numeric_value);
				}
			} else if ( isWhitespace(*ptr) ) {
				/* end of number literal */
				flags &= ~HID_WITHIN_NUM_LIT;
				encodeUnsigned(out, lit);
			} else {
				return errorMsg.at(n, E_Invalid_numeric_value);
			}
		} else if ( _HID_WITHIN(ARG_LIST) ) {
			if ( hasArg ) {
				if (*ptr == ')') {
					/* end of argument list */
					flags &= ~(HID_WITHIN_ARG_LIST | HID_WITHIN_UNIT_SYS);
					if (encMap->arg == signedNumArg) {
						item |= encodedSizeValue(encodedSize(int32_t(arg)));
						encodeUnsigned(out, item);
						encodeSigned(out, int32_t(arg));
					} else if (encMap->arg == unitExpMap) {
						/* UnitExponent */
						const int32_t sArg = int32_t(arg);
						if (sArg > 7 || sArg < -8) {
							return errorMsg.at(n, E_Argument_value_out_of_range);
						}
						encodeUnsigned(out, item | 1); /* encoding one byte data */
						encodeUnsigned(out, uint32_t(sArg & 0xF)); /* see unitExpMap */
					} else {
						if (encMap->arg == delimMap) {
							if (arg == 0) {
								/* Delimiter(Close) */
								if (delimLevel <= 0) {
									return errorMsg.at(n, E_Unexpected_DelimiterClose);
								}
								delimLevel--;
							} else if (arg == 1) {
								/* Delimiter(Open) */
								delimLevel++;
							} else {
								return errorMsg.at(n, E_Unexpected_Delimiter_value);
							}
						} else if (encMap->arg == usagePageMap || encMap->arg == usageArg) {
							/* UsagePage/Usage/UsageMinimum/UsageMaximum */
							if (arg > 0xFFFF) {
								return errorMsg.at(n, E_Argument_value_out_of_range);
							}
							if (encMap->arg == usagePageMap) {
								/* UsagePage */
								hasUsagePage = true;
							}
						} else if (encMap->value == 0x74) {
							/* ReportSize */
							reportSizes++;
						} else if (encMap->value == 0x94) {
							/* ReportCount */
							reportCounts++;
						}
						item |= encodedSizeValue(encodedSize(arg));
						encodeUnsigned(out, item);
						encodeUnsigned(out, arg);
					}
					/* commas are only allowed within argument lists */
					multiArg = false;
				} else if (multiArg && *ptr == ',') {
					hasArg = false;
				} else if ( ! isWhitespace(*ptr) ) {
					return errorMsg.at(n, E_Unexpected_token);
				}
			} else {
				if ( isItemChar(*ptr) ) {
					/* start of argument */
					flags |= HID_WITHIN_ARG;
					tArg.start = ptr;
					tArg.length = 1;
				} else if (ptr[0] == '0' && (n + 1) < len && ptr[1] == 'x') {
					/* start of hex literal */
					flags |= HID_WITHIN_HEX_LIT;
					if ((n + 2) >= len) {
						return errorMsg.at(n + 2, E_Unexpected_end_of_source);
					}
					if ( ! isHexDigit(ptr[2]) ) {
						return errorMsg.at(n + 2, E_Invalid_hex_value);
					}
					lit = 0;
					n++;
					ptr++;
				} else if (*ptr == '-') {
					/* start of negative number literal */
					if (encMap->arg != signedNumArg && encMap->arg != unitExpMap) {
						return errorMsg.at(n, E_Negative_numbers_are_not_allowed_in_this_context);
					}
					flags |= HID_WITHIN_NUM_LIT;
					lit = 0;
					negLit = true;
				} else if ( isDigit(*ptr) ) {
					/* start of number literal */
					flags |= HID_WITHIN_NUM_LIT;
					lit = 0;
					continue; /* re-parse as number literal */
				} else if (*ptr == '{') {
					/* start of user parameter */
					flags |= HID_WITHIN_PARAM;
					tArg.start = ptr + 1;
					tArg.length = 0;
				} else if (*ptr == ')') {
					/* end of argument list */
					return errorMsg.at(n, E_Missing_argument);
				} else if ( ! isWhitespace(*ptr) ) {
					return errorMsg.at(n, E_Unexpected_argument_name_character);
				}
			}
		}
		n++;
		ptr++;
	}
	/* end of source code */
	if (_HID_WITHIN(HEX_LIT) || _HID_WITHIN(NUM_LIT)) {
		/* end of hex/number literal */
		flags &= ~(HID_WITHIN_HEX_LIT | HID_WITHIN_NUM_LIT);
		if (flags == HID_START) {
			encodeUnsigned(out, lit);
		}
	}
	if ( _HID_WITHIN(ITEM) ) {
		flags &= ~HID_WITHIN_ITEM;
		subError = E_Invalid_item_name;
		encMap = findEncoding(tItem, itemMap, dynMap, subError);
		if (encMap == NULL) {
			return errorMsg.at(n, subError);
		} else if (encMap->arg == colArgMap) {
			/* Collection */
			if (usageAtLevel != colLevel) {
				return errorMsg.at(n, E_Missing_Usage_for_Collection);
			}
			colLevel++;
		} else if (encMap->arg == endCol) {
			/* EndCollection */
			if (colLevel <= 0) {
				return errorMsg.at(n, E_Unexpected_EndCollection);
			}
			if (reportSizes < reportCounts) {
				return errorMsg.at(n, E_Missing_ReportSize);
			} else if (reportCounts < reportSizes) {
				return errorMsg.at(n, E_Missing_ReportCount);
			}
			colLevel--;
			usageAtLevel--;
		}
		/* end of item */
		if (encMap->arg != NULL && (encMap->arg->name != NULL || encMap->arg == usageArg)) {
			return errorMsg.at(n, E_Missing_argument);
		}
		if (flags == HID_START) {
			encodeUnsigned(out, encMap->value);
		}
	}
	if (colLevel > 0) {
		return errorMsg.at(n, E_Missing_EndCollection);
	}
	if (delimLevel > 0) {
		return errorMsg.at(n, E_Missing_DelimiterClose);
	}
	if (flags != HID_START && flags != HID_WITHIN_COMMENT) {
		return errorMsg.at(n, E_Unexpected_end_of_source);
	}
	error = ::hid::error::Info();
	return true;
#undef _HID_WITHIN
}


/**
 * Returns the byte size of the compiled HID descriptor.
 * 
 * @param[in] source - source code description
 * @return compiled HID descriptor size
 */
template <size_t S, size_t P>
constexpr inline size_t compiledSize(const ::hid::detail::Source<S, P> & source) noexcept {
	::hid::error::Info error;
	SizeEstimator out;
	compile(source, out, error);
	return out.getPosition();
}


/**
 * Returns the byte size of the compiled HID descriptor.
 * 
 * @param[in] source - source code description
 * @return compiled HID descriptor size
 */
template <size_t S, size_t P>
constexpr inline ::hid::error::Info compileError(const ::hid::detail::Source<S, P> & source) noexcept {
	::hid::error::Info error;
	NullWriter out;
	compile(source, out, error);
	return error;
}


/**
 * Compiled HID descriptor instance.
 * 
 * @tparam N - HID descriptor size
 */
template <size_t N>
struct Descriptor {
    uint8_t data[N]; /**< Compiled HID descriptor data. */
    enum { Size = N }; /**< Data size. */
	
	/**
	 * Constructor.
	 * 
	 * @param[in] source - source code description
	 * @remarks This should be processed at compile time (i.e. used as constexpr).
	 * @see ::hid::detail::compile()
	 */
	template <size_t S, size_t P>
    constexpr inline explicit Descriptor(const ::hid::detail::Source<S, P> & source) noexcept:
		data{0}
	{
		::hid::error::Info error;
		BufferWriter out(this->data, N);
		compile(source, out, error);
	}
	
	/**
	 * Returns the data size.
	 * 
	 * @return data size
	 */
	constexpr inline size_t size() const {
		return N;
	}
};


/**
 * Compiled HID descriptor instance.
 * 
 * @tparam N - HID descriptor size
 */
template <>
struct Descriptor<0> {
    uint8_t * data; /**< Compiled HID descriptor data. */
    enum { Size = 0 }; /**< Data size. */
	
	/**
	 * Constructor.
	 * 
	 * @param[in] source - source code description
	 * @remarks This should be processed at compile time (i.e. used as constexpr).
	 * @see ::hid::detail::compile()
	 */
	template <size_t S, size_t P>
    constexpr inline explicit Descriptor(const ::hid::detail::Source<S, P> & /* source */) noexcept:
		data{NULL}
	{}
	
	/**
	 * Returns the data size.
	 * 
	 * @return data size
	 */
	constexpr inline size_t size() const noexcept {
		return 0;
	}
};


} /* anonymous namespace */
} /* namespace detail */


using Error = ::hid::error::Info;
using ::hid::error::reporter;
using ::hid::detail::compile;
using ::hid::detail::compiledSize;
using ::hid::detail::compileError;
using ::hid::detail::Descriptor;


/**
 * Creates a HidDescriptor source instance from the given source code.
 * Fill out the parameters (if any) and call compile on it to create
 * the byte representation.
 * 
 * @param[in] source - HID descriptor source code
 * @return HID descriptor source code object
 */
template <size_t N>
static constexpr ::hid::detail::Source<N + 1> fromSource(const char (&source)[N]) noexcept {
	::hid::detail::Source<N + 1> tmp;
	for (size_t n = 0; n < N; n++) {
		tmp.code[n] = source[n];
	}
	tmp.code[N] = 0; /* ensure null-termination */
	return tmp;
}


} /* namespace hid */


#endif /* __HIDDESCRIPTOR_HPP__ */
