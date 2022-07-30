/**
 * @file HidWebCompiler.cpp
 * @author Daniel Starke
 * @copyright Copyright 2022 Daniel Starke
 * @date 2022-05-07
 * @version 2022-05-14
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../src/HidDescriptor.hpp"


#define IMPORT_AS(name) __attribute__((import_module("env"), import_name(#name)))
#define EXPORT_AS(name) __attribute__((export_name(#name)))


namespace javaScriptAPI {
/**
 * Processes one output byte from the WASM side.
 * 
 * @param[in] value - byte to output
 */
void addOutput(const uint8_t value) noexcept IMPORT_AS(addOutput);


/**
 * Processes the compile result from the WASM side.
 * 
 * @param[in] msg - result message
 * @param[in] pos - error character (starting at 0)
 * @param[in] line - error line (starting at 1)
 * @param[in] column - error column (starting at 1)
 */
void setResult(const char * msg, const size_t pos, const size_t line, const size_t column) noexcept IMPORT_AS(setResult);


/** Writes bytes to the JavaScript side. */
class Writer {
private:
	size_t pos; /**< position */
public:
	/**
	 * Constructor.
	 */
	constexpr inline Writer() noexcept:
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
	 * Writes the given byte to JavaScript.
	 * 
	 * @param[in] val - byte value to write
	 * @return true on success, else false
	 */
	constexpr inline bool write(const uint8_t val) noexcept {
		addOutput(val);
		this->pos++;
		return true;
	}
};
} /* namespace javaScriptAPI */


/**
 * Exports the malloc() function from WASM.
 * 
 * @param[in] size - requested allocation size
 * @return pointer to the allocated memory region
 */
void * _malloc(const size_t size) noexcept EXPORT_AS(malloc) {
	return malloc(size);
}


/**
 * Exports the free() function from WASM.
 * 
 * @param[in] ptr - previously with malloc() allocated pointer
 */
void _free(void * ptr) noexcept EXPORT_AS(free) {
	if (ptr != 0) {
		free(ptr);
	}
}


/**
 * HID descriptor input source without parameter set.
 */
struct Source {
    const char * code; /**< Source code. */
	size_t len; /**< Source size in bytes. */
	
    /**
     * HID descriptor source code.
     * 
     * @param[in] str - null-terminated source code
     */
    inline Source(const char * str) noexcept:
		code{str},
		len{strlen(str)}
	{}
	
	/**
	 * Returns the source code pointer.
	 * 
	 * @return source code pointer
	 */
	inline const char * data() const noexcept {
		return this->code;
	}
	
	/**
	 * Returns the source code size in bytes.
	 * 
	 * @return source code size in bytes
	 */
	inline size_t size() const noexcept {
		return this->len;
	}
	
	/**
	 * Returns the parameter count in bytes.
	 * 
	 * @return parameter count
	 */
	inline size_t count() const noexcept {
		return 0;
	}
	
	/**
	 * Finds a parameter with the given name in the internal parameter set.
	 * The value of the last parameter with this name will be returned.
	 * 
	 * @param[in] token - parameter name token
	 * @return associated value
	 * @remarks Parameters from JavaScript are not supported. This is a dummy.
	 */
	constexpr inline hid::detail::ParamMatch find(const hid::detail::Token & token) const noexcept {
		return hid::detail::ParamMatch{0, true};
	}
};


/**
 * Exports the compile() function from WASM.
 * This can be called by the JavaScript side to compile a
 * HID descriptor. Make sure to initialize the addOutput()
 * and setResult() target beforehand.
 * The passed source code needs to be null-terminated and
 * within the WASM memory region.
 * 
 * @param[in] source - HID descriptor source code
 * @return compiled HID descriptor bytes
 */
size_t compile(const char * source) noexcept EXPORT_AS(compile) {
	javaScriptAPI::Writer out;
	hid::Error error;
	Source src{source};
	const size_t res = hid::compile(src, out, error);
	javaScriptAPI::setResult(hid::error::EMessageStr[error.message], error.character, error.line, error.column);
	return res;
}
