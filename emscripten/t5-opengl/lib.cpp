#include <stdio.h>
#include <emscripten/html5.h>
#include <emscripten.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

extern "C"{

void EMSCRIPTEN_KEEPALIVE init(const char * canvas){
	printf("%s\n", canvas);

	EmscriptenWebGLContextAttributes attribs;
	emscripten_webgl_init_context_attributes(&attribs);
	attribs.alpha = false;
	attribs.enableExtensionsByDefault = false;

	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(canvas, &attribs);
	emscripten_webgl_make_context_current(context);
}

void EMSCRIPTEN_KEEPALIVE render(){
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

}