PREFIX = 
CXX = $(PREFIX)g++
CWFLAGS = -Wall -Wextra -Wformat -pedantic -Wshadow -Wconversion -Wparentheses -Wunused -Wno-missing-field-initializers
CXXFLAGS = -Og -g3 -ggdb -gdwarf-3 -std=c++14 -static
COVCFLAGS = -fprofile-arcs -ftest-coverage -fno-inline -DNSANITY
GCOV = gcov
GCOVFLAGS = -b -c -m -f
KCXX = $(PREFIX)clang++-11
KCFLAGS = -emit-llvm -g -O0 -Xclang -disable-O0-optnone -mstackrealign
KLEE = klee
KFLAGS = --libc=uclibc --posix-runtime --optimize -silent-klee-assume --only-output-states-covering-new

.PHONY: cov
cov: unit.cpp ../src/HidDescriptor.hpp
	@$(CXX) $(COVCFLAGS) $(CWFLAGS) $(CXXFLAGS) -o cov unit.cpp
	@./cov >/dev/null 2>&1
	@$(GCOV) $(GCOVFLAGS) unit.cpp | grep -A 1 "File '../src/HidDescriptor.hpp'"
	@gawk '/^function .*::(findEncoding|compile<Source).*blocks executed/ {gsub(/\(anonymous namespace\)::/, ""); sub(/^function /, ""); sub(/ called/, "\n -> called"); print $0;}' HidDescriptor.hpp.gcov
	@rm -f exception.h.gcov initializer_list.gcov new.gcov stdio.h.gcov unit.cpp.gcov unit.gcda unit.gcno 2>/dev/null

.PHONY: unit
unit: unit.cpp ../src/HidDescriptor.hpp
	$(CXX) $(CWFLAGS) $(CXXFLAGS) -o unit unit.cpp
	./unit

.PHONY: fuzzy
fuzzy: fuzzy.cpp ../src/HidDescriptor.hpp
	$(CXX) $(CWFLAGS) $(CXXFLAGS) -o fuzzy fuzzy.cpp
	./fuzzy

.PHONY: klee
klee: klee.cpp ../src/HidDescriptor.hpp
	$(KCXX) $(KCFLAGS) -c -o klee.bc klee.cpp
	$(KLEE) $(KFLAGS) klee.bc

.PHONY: clean
clean:
	@rm -f *.exe 2>/dev/null || true
	@rm -f *.gcda *.gcno *.gcov 2>/dev/null || true
	@rm -f cov unit fuzzy klee 2>/dev/null || true

.PHONY: help
help: 
	@echo 'Targets:'
	@echo ' cov   - Perform code coverage tests.'
	@echo ' unit  - Perform unit tests.'
	@echo ' fuzzy - Perform fuzzy tests.'
	@echo ' klee  - Perform LLVM/Klee tests. Requires LLVM/Clang and Klee.'
	@echo '         See https://klee.github.io/'
