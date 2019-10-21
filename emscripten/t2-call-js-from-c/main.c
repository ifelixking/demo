#include <stdio.h>
#include <emscripten.h>

int main(){

	// 嵌入 JavaScript 代码, 就像嵌入汇编一样
	EM_ASM({
		console.log("this is a javascript code")
	});

	// 向 js 中传参
	EM_ASM_ARGS({
		var msg = UTF8ToString($1);		// 获得字符串参数
		for (var i=0; i<$0; ++i){
			console.log(msg + i)
		}
	}, 3, "show me");

	// 调用 js 并获得返回值
	double result = EM_ASM_DOUBLE({
		return $0 * $1
	}, 3.14, 2.0);
	printf("result: %f\n", result);

	// 其他
	// EM_ASM：调用 JS 代码，没有参数，也没有返回值。
	// EM_ASM_ARGS JS 代码，可以有任意个参数，但是没有返回值。
	// EM_ASM_INT：调用 JS 代码，可以有任意个参数，返回一个整数。
	// EM_ASM_DOUBLE：调用 JS 代码，可以有任意个参数，返回一个双精度浮点数。
	// EM_ASM_INT_V：调用 JS 代码，没有参数，返回一个整数。
	// EM_ASM_DOUBLE_V：调用 JS 代码，没有参数，返回一个双精度浮点数。

	return 0;
}