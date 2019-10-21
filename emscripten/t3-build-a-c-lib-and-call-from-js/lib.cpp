#include <stdio.h>
#include <emscripten.h>
#include <string>

extern "C" {

	int cfunc1(int a, int b) { 
		return a + b; 
	}

	std::string result;
	const char * cfunc2(const char * name){
		result = "hello ";
		result += std::string(name);
		return result.c_str();
	}

}