var _self = (typeof window !== 'undefined')
	? window   // if in browser
	: (
		(typeof WorkerGlobalScope !== 'undefined' && self instanceof WorkerGlobalScope)
			? self // if in worker
			: {}   // if in node js
	);

/** HidWebCompiler instance. */
var HidWebCompiler = (function (_self) {
	if ( ! ('WebAssembly' in window) ) {
		return;
	}

	var globalMem = new WebAssembly.Memory({ initial: 4 });
	const encodeUtf8 = TextEncoder.prototype.encode.bind(new TextEncoder());
	const decodeUtf8 = TextDecoder.prototype.decode.bind(new TextDecoder());
	
	var malloc = null;
	var free = null;
	var compile = null;
	var result = null;
	
	/**
	 *  Calculates the length of the passed null-terminated WASM string.
	 *  
	 *  @param[in] str - WASM memory pointer
	 *  @return string length
	 */
	function strlen(str) {
		const view = new Uint8Array(globalMem.buffer);
		var len = 0;
		while (view[str + len] != 0) len++;
		return len;
	}
	
	/**
	 *  Allocates and sets the given string within the WASM
	 *  instance.
	 *  
	 *  @param[in] str - string to allocate
	 *  @return WASM memory pointer
	 */
	function allocString(str) {
		str += String.fromCharCode(0); /* null-terminated */
		const utf8Str = encodeUtf8(str);
		const ptr = malloc(utf8Str.length);
		new Uint8Array(globalMem.buffer, ptr, utf8Str.length).set(utf8Str);
		return ptr;
	}
	
	/**
	 *  Reads the null-terminated string from the given WASM
	 *  memory pointer.
	 *  
	 *  @param[in] ptr - WASM memory pointer
	 *  @return string from address
	 */
	function readString(ptr) {
		const strView = new Uint8Array(globalMem.buffer, ptr, strlen(ptr));
		return decodeUtf8(strView);
	}
	
	/* WASM context. */
	const imports = {env: {
		memory: globalMem,
		memoryBase: 0,
		table: new WebAssembly.Table({ initial: 0, element: 'anyfunc' }),
		tableBase: 0,
		/**
		 *  Adds the compiled byte to the result string.
		 *  
		 *  @param[in] value - compiled byte value
		 *  @remarks Called from the WASM module.
		 */
		addOutput: function (value) {
			if ( ! result.data ) {
				result.data = "";
			} else if (result.data.length != 0) {
				result.data += " ";
			}
			const hexVal = value.toString(16).toUpperCase();
			result.data += (hexVal.length < 2) ? "0" + hexVal : hexVal;
		},
		/**
		 *  Sets the compilation result.
		 *  
		 *  @param[in] msg - null-terminated error message string
		 *  @param[in] pos - error character (starting at 0)
		 *  @param[in] line - error line (starting at 1)
		 *  @param[in] column - error column (starting at 1)
		 *  @remarks Called from the WASM module.
		 */
		setResult: function (msg, pos, line, column) {
			const msgStr = readString(msg);
			if (msgStr != "No error.") {
				result.error = {
					message: msgStr,
					pos: pos,
					line: line,
					column: column
				};
			}
		}
	}};
	
	var _ = {
		/**
		 *  Load the HidDescCTC WASM.
		 *  
		 *  @param[in] url - path to the WASM file
		 */
		load: function (url) {
			/* Fetch assembly file, compile and instantiate it. */
			WebAssembly.instantiateStreaming(fetch(url), imports)
			.then(function (obj) {
				/* Export global functions. */
				malloc = obj.instance.exports.malloc;
				free = obj.instance.exports.free;
				compile = obj.instance.exports.compile;
			});
		},
		
		/**
		 *  Compiles the passed HID descriptor source code.
		 *  
		 *  @param[in] str - source code
		 *  @return result/error
		 */
		compile: function (str) {
			result = {
				data: "",
				error: null
			};
			const ptr = allocString(str);
			compile(ptr); /* calling the WASM module function */
			free(ptr);
			return result;
		}
	};
	
	_self.HidWebCompiler = _;
	
	return _;
}(_self));
