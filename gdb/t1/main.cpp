#include <iostream>

void myFunction(int n){
	for(int i=0; i<n; ++i){
		std::cout << "Hello, World!" << std::endl;
	}
}

int main(int argc, const char ** argv) {
	myFunction(argc);
	return 0;
}