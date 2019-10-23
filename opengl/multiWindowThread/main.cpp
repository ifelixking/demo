#include <cassert>
#include <string>
#include <list>
#include <vector>
#include <condition_variable>
#include <chrono>

// opengl
#include <GL/glew.h>
// x
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
#include <thread>

const auto EVENT_MASK = KeyPressMask;

struct Device {
	Display *display;
	Window windowRoot;
	int defaultScreen;
	GLXFBConfig *fbConfig;
	XVisualInfo *visualInfo;
	Colormap colormap;
	GLXContext sharedContext;
};

struct MakeCurrent{
	Display * m_display;
	GLXDrawable m_oldWindow;
	GLXContext m_oldContext;
	MakeCurrent(const Display * display, GLXDrawable win, GLXContext context)
			: m_display((Display *)display)
	{
		m_oldContext = glXGetCurrentContext();
		m_oldWindow = glXGetCurrentDrawable();
		glXMakeCurrent(m_display, win, context);
	}
	~MakeCurrent() {
		glXMakeCurrent(m_display, m_oldWindow, m_oldContext);
	}
};

GLXContext createGLContext(const Device & device, GLXContext sharedContext){
	auto context = glXCreateContext(device.display, device.visualInfo, sharedContext, GL_TRUE); assert(context);
	return context;
}
void destroyGLContext(const Device & device, GLXContext context){
	if (glXGetCurrentContext() == context) {
		auto bResult = glXMakeCurrent(device.display, None, nullptr); assert(bResult);
	}
	glXDestroyContext(device.display, context);
}

void openDevice(Device & device){
	auto displayName = getenv("DISPLAY"); assert(displayName);
	device.display = XOpenDisplay(displayName); assert(device.display);
	device.defaultScreen = DefaultScreen(device.display);
	int nelements, att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DOUBLEBUFFER, True, GLX_DEPTH_SIZE, 16, None};
	device.fbConfig = glXChooseFBConfig(device.display, device.defaultScreen, att, &nelements); assert(device.fbConfig);
	device.visualInfo = glXGetVisualFromFBConfig(device.display, *device.fbConfig); assert(device.visualInfo);
	device.windowRoot = RootWindow(device.display, device.defaultScreen);
	device.colormap = XCreateColormap(device.display, device.windowRoot, device.visualInfo->visual, AllocNone);
	device.sharedContext = createGLContext(device, nullptr);
}
void closeDevice(const Device & device){
	destroyGLContext(device, device.sharedContext);
	XFreeColormap(device.display, device.colormap);
	XFree(device.fbConfig);
	XFree(device.visualInfo);
	XCloseDisplay(device.display);
}

struct RenderWindow{
	Window window;
	std::thread thread;
};

typedef bool (*RenderFunc)();

RenderWindow createRenderWindow(const Device & device, RenderFunc renderFunc){
	XSetWindowAttributes swa;
	swa.colormap = device.colormap;
	swa.event_mask = EVENT_MASK;
	unsigned long valueMask = CWColormap | CWEventMask;
	auto window = XCreateWindow(device.display, device.windowRoot, 100, 100, 640, 480, 0,
								device.visualInfo->depth, InputOutput, device.visualInfo->visual,
								valueMask, &swa);
	XMapWindow(device.display, window);
	XFlush(device.display);

	std::thread thread([&device, window, renderFunc](){
		auto context = createGLContext(device, device.sharedContext);
		MakeCurrent mc(device.display, window, context);
		for (;;) {
			if (!renderFunc()){ break;}
			glXSwapBuffers(device.display, window);
		}
		destroyGLContext(device, context);
	});

	RenderWindow rw = { window, std::move(thread) };
	return rw;
}
void destroyWindow(const Device & device, Window window){
	auto iResult = XUnmapWindow(device.display, window); assert(iResult);
	iResult = XDestroyWindow(device.display, window); assert(iResult);
}

void initGLEW(const Device & device, GLXContext context){
	MakeCurrent mc(device.display, device.windowRoot, context);
	auto err = glewInit(); assert(GLEW_OK == err);
}

bool g_exitFlag = false;

bool render(){

	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	return !g_exitFlag;
}

int main() {

	XInitThreads();

	Device device = {}; openDevice(device);
	initGLEW(device, device.sharedContext);

	auto window1 = createRenderWindow(device, render);
	//auto window2 = createRenderWindow(device, render);

	std::this_thread::sleep_for(std::chrono::seconds(2));


	for (;;){
		XEvent event;
		XNextEvent(device.display, &event);
		switch (event.type){
			case KeyPress: {
				auto keySys = XkbKeycodeToKeysym(device.display, event.xkey.keycode, 0,
												 (event.xkey.state & ShiftMask) ? 1 : 0);
				if (keySys == XK_Escape) goto L_EXIT;
			}
		}
	}

	L_EXIT:
	g_exitFlag = true;
	window1.thread.join();
	//window2.thread.join();
	destroyWindow(device, window1.window);
	//destroyWindow(device, window2.window);

	closeDevice(device);

	return 0;
}