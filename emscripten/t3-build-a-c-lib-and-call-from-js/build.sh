#!/bin/sh

emcc -s EXPORTED_FUNCTIONS="['_cfunc1', '_cfunc2']" -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' lib.cpp