/**
 * @file fuzzy.hpp
 * @author Daniel Starke
 * @copyright Copyright 2022 Daniel Starke
 * @date 2022-07-07
 * @version 2022-07-15
 */
#include "../src/HidDescriptor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>


/** Entry point. */
int main() {
	/* Example based on HID 1.11 appendix D.1 */
	const char base[] = R"(0xFF
254 819 189 481 0x1242 {arg1}
UsagePage(GenericDesktop)
Usage(Joystick)
Collection(Application)
	UsagePage(GenericDesktop)
	Usage(Pointer)
	Collection(Physical)
		LogicalMinimum(-127)
		LogicalMaximum(127)
		ReportSize(8)
		ReportCount(2)
		Push
		Usage(X)
		Usage(Y)
		Input(Data, Var, Abs)
		Usage(HatSwitch)
		LogicalMinimum(0)
		LogicalMaximum(3)
		PhysicalMinimum(0)
		PhysicalMaximum(270)
		Unit(EngRot(Length)) # Degrees
		ReportCount(1)
		ReportSize(4)
		Input(Data, Var, Abs, Null)
		LogicalMinimum(0)
		LogicalMaximum(1)
		ReportCount(2)
		ReportSize(1)
		UsagePage(Button)
		UsageMinimum(Button1)
		UsageMaximum(Button2)
		Unit(None())
		Input(Data, Var, Abs)
	EndCollection
	UsageMinimum(Button3)
	UsageMinimum(Button4)
	Input(Data, Var, Abs)
	# use LogicalMinimum/LogicalMaximum from before Push
	Pop
	UsagePage(SimulationControls)
	Usage(Throttle)
	ReportCount({arg1})
	ReportSize(1)
	Input(Data, Var, Abs)
EndCollection
0xFF
)";
	const uint8_t check[] = {
		0xFF, 0xFE, 0x33, 0x03, 0xBD, 0xE1, 0x01, 0x42, 0x12, 0x01, 0x05, 0x01, 0x09, 0x04, 0xA1, 0x01,
		0x05, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x02, 0xA4, 0x09,
		0x30, 0x09, 0x31, 0x81, 0x02, 0x09, 0x39, 0x15, 0x00, 0x25, 0x03, 0x35, 0x00, 0x46, 0x0E, 0x01,
		0x65, 0x14, 0x95, 0x01, 0x75, 0x04, 0x81, 0x42, 0x15, 0x00, 0x25, 0x01, 0x95, 0x02, 0x75, 0x01,
		0x05, 0x09, 0x19, 0x01, 0x29, 0x02, 0x65, 0x00, 0x81, 0x02, 0xC0, 0x19, 0x03, 0x19, 0x04, 0x81,
		0x02, 0xB4, 0x05, 0x02, 0x09, 0xBB, 0x95, 0x01, 0x75, 0x01, 0x81, 0x02, 0xC0, 0xFF
	};
	char input[sizeof(base)];
	uint8_t buf[65536];
	const char subs[] = " _#;^-,aAx09(){}\0";
	hid::Error error;
	/* sanity check */
	{
			hid::detail::BufferWriter out(buf, sizeof(buf));
			const auto source = hid::fromSource(base)("arg1", 1);
			hid::compile(source, out, error);
			if (error.message != 0) {
				fprintf(stderr, "Error: Unexpected error on valid input.\n");
				return EXIT_FAILURE;
			}
			if (out.getPosition() != sizeof(check)) {
				fprintf(stderr, "Error: Unexpected compiled data length.\n");
				return EXIT_FAILURE;
			}
			if (memcmp(buf, check, out.getPosition()) != 0) {
				fprintf(stderr, "Error: Unexpected compiled data.\n");
				return EXIT_FAILURE;
			}
	}
	/* fuzzing: substitute random bytes */
	srand(0);
	for (size_t i = 0; i < 100000; i++) {
		memcpy(input, base, sizeof(base));
		for (size_t j = 0; j < 10; j++) {
			input[rand() % (sizeof(base) - 1)] = subs[rand() % sizeof(subs)];
			hid::detail::BufferWriter out(buf, sizeof(buf));
			const auto source = hid::fromSource(input)("arg1", 1);
			hid::compile(source, out, error);
		}
	}
	return EXIT_SUCCESS;
}
