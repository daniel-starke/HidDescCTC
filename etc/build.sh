#!/bin/sh

CXX=clang++

wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-16/wasi-sysroot-16.0.tar.gz || exit 1
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-16/libclang_rt.builtins-wasm32-wasi-16.0.tar.gz || exit 1

tar -xvzf wasi-sysroot-16.0.tar.gz || exit 1
sudo tar -xvzf libclang_rt.builtins-wasm32-wasi-16.0.tar.gz -C $(dirname $(dirname $(dirname $(${CXX} -rtlib=compiler-rt --target=wasm32-wasi --print-libgcc-file-name)))) || exit 1

${CXX} --version
${CXX} --sysroot=wasi-sysroot --target=wasm32-wasi -Dconstexpr= -O3 -nostdlib++ -nostartfiles -fno-exceptions -Wl,--no-entry -Wl,--export-dynamic -Wl,--import-memory -fvisibility=hidden -s -o ../docs/assets/HidWebCompiler.wasm HidWebCompiler.cpp || exit 1
chmod 0777 ../docs/assets/HidWebCompiler.wasm
