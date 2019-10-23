#include <stdio.h>
#include <emscripten/html5.h>
#include <emscripten.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

int main()
{

	EmscriptenWebGLContextAttributes attribs;
	emscripten_webgl_init_context_attributes(&attribs);
	attribs.alpha = false;
	attribs.enableExtensionsByDefault = false;

	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas1", &attribs);
	emscripten_webgl_make_context_current(context);

	EM_ASM({
		var render = function(secTime){
			Module._render();
			window.requestAnimationFrame(render);
		};
		window.requestAnimationFrame(render);
	});

	return 0;
}

extern "C"
{
	void EMSCRIPTEN_KEEPALIVE render()
	{
		glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}