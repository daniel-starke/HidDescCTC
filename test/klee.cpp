/**
 * @file klee.hpp
 * @author Daniel Starke
 * @copyright Copyright 2022 Daniel Starke
 * @date 2022-07-07
 * @version 2022-07-09
 */
#include "../src/HidDescriptor.hpp"
#include <cstdio>
#include <cstdlib>
#include <klee/klee.h>


/** Entry point. */
int main() {
	char input[100];
	uint8_t buf[65536];
	hid::Error error;
	hid::detail::BufferWriter out(buf, sizeof(buf));
  	klee_make_symbolic(input, sizeof(input), "input");
	input[99] = 0;
	const auto source = hid::fromSource(input)("arg1", 1);
	hid::compile(source, out, error);
	return EXIT_SUCCESS;
}
