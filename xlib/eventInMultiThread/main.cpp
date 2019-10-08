#include <algorithm>
#include <vector>
#include <iostream>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <pthread.h>
#include <X11/keysym.h>

Display *openDisplay() {
	auto displayName = getenv("DISPLAY");
	assert(displayName);
	auto display = XOpenDisplay(displayName);
	assert(display);
//    device.defaultScreen = DefaultScreen(device.display);
//    int nelements, att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DOUBLEBUFFER, True, GLX_DEPTH_SIZE, 16, None};
//    device.fbConfig = glXChooseFBConfig(device.display, device.defaultScreen, att, &nelements); assert(device.fbConfig);
//    device.visualInfo = glXGetVisualFromFBConfig(device.display, *device.fbConfig); assert(device.visualInfo);
//    device.windowRoot = RootWindow(device.display, device.defaultScreen);
//    device.colormap = XCreateColormap(device.display, device.windowRoot, device.visualInfo->visual, AllocNone);
//
	return display;
}

Window createWindow(Display *display) {
	auto window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 100, 100, 640, 480, 0, 0, 0xffff0000);
	// StructureNotifyMask 可以 监听到 窗口销毁
	// https://tronche.com/gui/x/xlib/events/processing-overview.html#StructureNotifyMask
	XSelectInput(display, window, KeyPressMask);
	XMapWindow(display, window);
	XFlush(display);
	return window;
}

void destroyWindow(Display * display, Window window){
	XDestroyWindow(display, window);
}

void closeDisplay(Display * display){
	XCloseDisplay(display);
}


int main() {

	auto display = openDisplay();
	auto window = createWindow(display);

	std::vector<int> eventTypes;

	for (;;) {
		XEvent event;
		XNextEvent(display, &event);
		if (std::find(eventTypes.begin(), eventTypes.end(), event.type) == eventTypes.end()) {
			eventTypes.push_back(event.type);
			std::cout << event.type << std::endl;
		}
		switch (event.type) {
			case KeyPress: {
				auto keySys = XkbKeycodeToKeysym(display, event.xkey.keycode, 0,
												 (event.xkey.state & ShiftMask) ? 1 : 0);
				if (keySys == XK_Escape) goto L_EXIT;
			}break;
		}
	}

	L_EXIT:

	destroyWindow(display, window);
	closeDisplay(display);


	return 0;
}