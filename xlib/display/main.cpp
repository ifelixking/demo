#include <iostream>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <thread>
#include <cassert>


struct Device {
	Display *display;
	Window windowRoot;
	int defaultScreen;
};

void openDevice(Device & device){
	auto displayName = getenv("DISPLAY"); assert(displayName);
	device.display = XOpenDisplay(displayName); assert(device.display);
	device.defaultScreen = DefaultScreen(device.display);
	device.windowRoot = RootWindow(device.display, device.defaultScreen);
}
void closeDevice(const Device & device){
	XCloseDisplay(device.display);
}

int main() {

	Device device1; openDevice(device1);
	Device device2; openDevice(device2);

	auto window = XCreateSimpleWindow(device1.display, device1.windowRoot, 100, 100, 600, 400, 0, 0, 0);
	auto iResult = XMapWindow(device2.display, window);
	iResult = XFlush(device2.display);

	closeDevice(device1);
	closeDevice(device2);

	return 0;
}