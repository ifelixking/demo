#!/bin/sh

emcc -s EXPORTED_FUNCTIONS="['_add']" lib.cpp