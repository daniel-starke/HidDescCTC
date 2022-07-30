#!/bin/sh

wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-16/wasi-sysroot-16.0.tar.gz || exit 1
tar -xvzf wasi-sysroot-16.0.tar.gz || exit 1
clang++ --sysroot=wasi-sysroot --target=wasm32-wasi -O3 -nostdlib++ -nostartfiles -fno-exceptions -Wl,--no-entry -Wl,--export-dynamic -Wl,--import-memory -fvisibility=hidden -s -o ../docs/assets/HidWebCompiler.wasm HidWebCompiler.cpp || exit 1
