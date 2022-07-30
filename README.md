HidDescCTC
==========

The USB **HID** **Desc**riptor **C**ompile **T**ime **C**ompiler can be
used to define USB HID descriptors without any runtime overhead using a
domain specific language and the `constexpr` feature in C++14.  

Features
========

- USB HID complaint descriptor encoding
- zero runtime overhead
- parameterizable fields and values
- easy to understand text based declarative language
- compile time syntax and sanity checks
- single, dependency free header file (only requires `stdint.h` and `stddef.h`)
- [PlatformIO](https://platformio.org/) compatible
- WASM compatible (see playground below)

Playground
==========

Try out the HidDescCTC with C++ here:  
[https://godbolt.org](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:2,endLineNumber:46,positionColumn:2,positionLineNumber:46,selectionStartColumn:2,selectionStartLineNumber:46,startColumn:2,startLineNumber:46),source:'%23include+%3Chttps://raw.githubusercontent.com/daniel-starke/HidDescCTC/main/src/HidDescriptor.hpp%3E%0A%0Aconst+uint8_t+*+getDescriptor()+%7B%0A%09DEF_HID_DESCRIPTOR_AS(%0A%09%09static+hidDesc,%0A%09%09(R%22(%0A%09%23+Example+based+on+HID+1.11+appendix+B.1%0A%09UsagePage(GenericDesktop)%0A%09Usage(Keyboard)%0A%09Collection(Application)%0A%09%09ReportId(%7Bid%7D)%0A%09%09ReportSize(1)%0A%09%09ReportCount(8)%0A%09%09UsagePage(Keyboard)%0A%09%09UsageMinimum(224)%0A%09%09UsageMaximum(231)%0A%09%09LogicalMinimum(0)%0A%09%09LogicalMaximum(1)%0A%09%09Input(Data,+Var,+Abs)+%23+Modifier+byte%0A%09%09ReportCount(1)%0A%09%09ReportSize(8)%0A%09%09Input(Cnst)+%23+Reserved+byte%0A%09%09ReportCount(5)%0A%09%09ReportSize(1)%0A%09%09UsagePage(LED)%0A%09%09UsageMinimum(1)%0A%09%09UsageMaximum(%7BmaxLedId%7D)%0A%09%09Output(Data,+Var,+Abs)+%23+LED+report%0A%09%09ReportCount(1)%0A%09%09ReportSize(3)%0A%09%09Output(Cnst)+%23+LED+report+padding%0A%09%09ReportCount(6)%0A%09%09ReportSize(8)%0A%09%09LogicalMinimum(0)%0A%09%09LogicalMaximum(255)%0A%09%09UsagePage(Keyboard)%0A%09%09UsageMinimum(0)%0A%09%09UsageMaximum(255)%0A%09%09Input(Data,+Ary)%0A%09EndCollection%0A%09)%22)%0A%09%09(%22id%22,+1)%0A%09%09(%22maxLedId%22,+5)%0A%09)%3B%0A++++return+hidDesc.data%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:48.69655062788111,l:'4',m:100,n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g91,filters:(b:'0',binary:'1',commentOnly:'0',demangle:'0',directives:'0',execute:'1',intel:'1',libraryCode:'1',trim:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-O2',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+9.1+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),header:(),k:51.303449372118905,l:'4',m:100,n:'0',o:'',s:0,t:'0')),l:'2',m:100,n:'0',o:'',t:'0')),version:4)

The implemented compiler and syntax can also be tried out online using WebAssembly:  
https://daniel-starke.github.io/HidDescCTC  
This is available as progressive web application (PWA).  
Note that all user parameters are replaced with the value `0` here.

Syntax
======

The used syntax is very close to the one used in the standard and by the [HID Descriptor Tool](https://www.usb.org/document-library/hid-descriptor-tool).  
Apply the following rules to get the HidDescCTC usage name from the name in the standard:
- replace leading `+` by `Plus`
- replace `/second/second` by `PerSecondSquared`
- remove all non-alphanumeric characters like space and underscore characters
- capitalize words/abbreviations, whereas dimensions count as one word (e.g. Usb3dControl)
- move words with a leading digit behind the first word
- remove second key meaning for the keyboard/keypad usage table entries
- replace non-alphanumeric characters of the keyboard/keypad usage table entries with their spelled out names

Usage
=====

Example:
```.cpp
DEF_HID_DESCRIPTOR_AS(
	static hidDesc,
	(R"(
# Example based on HID 1.11 appendix B.1
UsagePage(GenericDesktop)
Usage(Keyboard)
Collection(Application)
	ReportId({id})
	ReportSize(1)
	ReportCount(8)
	UsagePage(Keyboard)
	UsageMinimum(224)
	UsageMaximum(231)
	LogicalMinimum(0)
	LogicalMaximum(1)
	Input(Data, Var, Abs) # Modifier byte
	ReportCount(1)
	ReportSize(8)
	Input(Cnst) # Reserved byte
	ReportCount(5)
	ReportSize(1)
	UsagePage(LED)
	UsageMinimum(1)
	UsageMaximum({maxLedId})
	Output(Data, Var, Abs) # LED report
	ReportCount(1)
	ReportSize(3)
	Output(Cnst) # LED report padding
	ReportCount(6)
	ReportSize(8)
	LogicalMinimum(0)
	LogicalMaximum(255)
	UsagePage(Keyboard)
	UsageMinimum(0)
	UsageMaximum(255)
	Input(Data, Ary)
EndCollection
)")
	("id", 1)
	("maxLedId", 5)
);
```

This provides the compiled HID descriptor with:
- `hidDesc.data` as `const uint8_t *` pointing to the compiled data
- `hidDesc.size()` as `size_t` with the size of the compiled data

Any number of parameters can be passed and used as seen above.  
These, however, need to be compile time evaluable.  
Usually, report IDs are defined via `enum` and included in the
HID descriptor accordingly for later use in the protocol. These
can be included as parameters in the HID descriptor source.

Define `HID_DESCRIPTOR_NO_ERROR_REPORT` to suppress syntax error outputs.  
`DEF_HID_DESCRIPTOR_AS` can be used in global, namespace and function scope.

If you rather like to avoid using the macro you can compile the HID descriptor
like this:
```.cpp
constexpr static const auto hidSrc = hid::fromSource(R"(
# Example based on HID 1.11 appendix B.1
UsagePage(GenericDesktop)
Usage(Keyboard)
Collection(Application)
	ReportId({id})
	ReportSize(1)
	ReportCount(8)
	UsagePage(Keyboard)
	UsageMinimum(224)
	UsageMaximum(231)
	LogicalMinimum(0)
	LogicalMaximum(1)
	Input(Data, Var, Abs) # Modifier byte
	ReportCount(1)
	ReportSize(8)
	Input(Cnst) # Reserved byte
	ReportCount(5)
	ReportSize(1)
	UsagePage(LED)
	UsageMinimum(1)
	UsageMaximum({maxLedId})
	Output(Data, Var, Abs) # LED report
	ReportCount(1)
	ReportSize(3)
	Output(Cnst) # LED report padding
	ReportCount(6)
	ReportSize(8)
	LogicalMinimum(0)
	LogicalMaximum(255)
	UsagePage(Keyboard)
	UsageMinimum(0)
	UsageMaximum(255)
	Input(Data, Ary)
EndCollection
)")
	("id", 1)
	("maxLedId", 5)
;
constexpr static const auto hidDesc = hid::Descriptor<hid::compiledSize(hidSrc)>(hidSrc);
```

This performs the same steps as the macro above but without error reporting.
Compile time error reporting can be added with the following code:
```.cpp
constexpr static const hid::Error error = hid::compileError(hidSrc);
constexpr static const size_t dummy = hid::reporter<error.line, error.column, error.message>();
```

PlatformIO Integration
======================

The PlatformIO documentation describes [here](https://docs.platformio.org/en/latest/projectconf/section_env_library.html#lib-deps) and [here](https://docs.platformio.org/en/latest/core/userguide/lib/cmd_install.html#version-control) how to add libraries from GitHub directly.  
The latest HidDescCTC version can be included by adding the following line to `platformio.ini` in your environment section:
```.ini
lib_deps = HidDescCTC=https://github.com/daniel-starke/HidDescCTC/archive/refs/heads/main.zip
```

USB Standard References
=======================

- [USB HID v1.11](https://www.usb.org/sites/default/files/hid1_11.pdf)
  - [HID Usage Tables for USB v1.2](https://www.usb.org/sites/default/files/hut1_2.pdf)
    - [Device Class Definition for Physical Interface Devices (PID) v1.0](https://www.usb.org/sites/default/files/pid1_01.pdf)
	- [USB Monitor Control Class Specification v1.0](https://www.usb.org/sites/default/files/usbmon10.pdf)
	- [USB Usage Tables for HID Power Devices v1.0](https://www.usb.org/sites/default/files/pdcv10.pdf)
	- [USB HID Point of Sale Usage Tables v1.02](https://www.usb.org/sites/default/files/pos1_02.pdf)
	- [Open Arcade Architecture Device Data Format Specification v1.100](https://www.usb.org/sites/default/files/oaaddataformatsv6.pdf)

Other References
================

- [HID Descriptor Tool](https://www.usb.org/document-library/hid-descriptor-tool)

Extended Backus–Naur Form
=========================

Syntax description according to ISO/IEC 14977.

```.ebnf
Exponent = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" ;
SignedExponent = "-", ( Exponent | "8" ) ;
Digit = Exponent | "8" | "9" ;

LowerLetter = "a" | "b" | "c" | "d" | "e" | "f" | "g"  "h" | "i" | "j" | "k" | "l" | "m" | "n"  "o" | "p" | q" | "r" | "s" | "t" | "u"  "v" | "w" | "x" | "y" | "z" ;
UpperLetter = "A" | "B" | "C" | "D" | "E" | "F" | "G"  "H" | "I" | "J" | "K" | "L" | "M" | "N"  "O" | "P" | Q" | "R" | "S" | "T" | "U"  "V" | "W" | "X" | "Y" | "Z" ;
EndOfLine = ? line-feed or carrier-return ? ;
Character = ? all visible characters ? ;

Number = Digit, { Digit } ;
SignedNumber = "-", Number ;
HexNumber = "0x", Digit, { Digit } ;

ItemChar = { "_" | LowerLetter | UpperLetter } ;
ArgChar = { ItemChar | Digit } ;

Parameter = "{", { Character - "}" }, "}" ;

UnitName = "Length" | "Mass" | "Time" | "Temp" | "Current" | "Luminous" ;
BaseUnit = UnitName, "^", ( Exponent | SignedExponent ) ;
Unit = BaseUnit, { BaseUnit } ;

ArgumentName = ItemChar, { ArgChar } ;
Argument = ArgumentName | Number | SignedNumber | HexNumber | Parameter ;
ArgumentList = Argument, ( ( "(", Unit, ")" ) | { ",", Argument } ) ;

Item = ItemChar, { ItemChar }, [ "(", ArgumentList , ")" ] ;
Comment = ( ";" | "#" ), { Character - EndOfLine } ;

Grammar = { Item | Number | HexNumber | Parameter | Comment } ;
```

Limitations
===========

- Deduction of the Usage Page map from a numeric value or user parameter is not supported.
- Semantic checks against usage types as defined in HID 1.11 ch. 3.4 are not yet implemented.

License
=======

See [LICENSE](LICENSE).  

Contributions
=============

No content contributions are accepted. Please file a bug report or feature request instead.  
This decision was made in consideration of the used license.
