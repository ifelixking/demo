// xlib 是可以多线程的
// demo 演示了在主线程中创建window，在另外两个子线程中XNextEvent是可行的
// 但是需要注意：
// 1. 需要在程序入口处调用 XInitThread
// 2. 当调用 Display 为参数的相关方法时，需要使用 XLockDisplay
// 3. 可以创建（Open）多个 Display，就可以不互相影响，
// 但是display为相关资源（Window, GLXContext）等的根对象，要想资源共用貌似只能在一个display下

#include <algorithm>
#include <vector>
#include <iostream>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <thread>
#include <X11/keysym.h>

Display *openDisplay() {
    auto displayName = getenv("DISPLAY");
    assert(displayName);
    auto display = XOpenDisplay(displayName);
    assert(display);
    return display;
}

Window createWindow(Display *display) {
    auto window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 100, 100, 640, 480, 0, 0, 0xffaaaa);
    // StructureNotifyMask 可以 监听到 窗口销毁
    // https://tronche.com/gui/x/xlib/events/processing-overview.html#StructureNotifyMask
    XSelectInput(display, window, KeyPressMask);
    XMapWindow(display, window);
    XFlush(display);
    return window;
}

void destroyWindow(Display *display, Window window) {
    XDestroyWindow(display, window);
}

void closeDisplay(Display *display) {
    XCloseDisplay(display);
}

struct LockDisplay{
    LockDisplay(Display * display):m_display(display){XLockDisplay(display);}
    ~LockDisplay(){XUnlockDisplay(m_display);}
    Display * m_display;
};

void messageLoop(Display *display) {
    std::vector<int> eventTypes;
    auto threadID = std::this_thread::get_id();
    std::cout << "thread: " << threadID << std::endl;
    for (;;) {
        XEvent event;
        LockDisplay lock(display);
        //XLockDisplay(display);
        XNextEvent(display, &event);
        //XUnlockDisplay(display);
//        if (std::find(eventTypes.begin(), eventTypes.end(), event.type) == eventTypes.end()) {
//            eventTypes.push_back(event.type);
//            std::cout << event.type << std::endl;
//        }
        switch (event.type) {
            case KeyPress: {
                //XLockDisplay(display);
                auto keySys = XkbKeycodeToKeysym(display, event.xkey.keycode, 0,
                                                 (event.xkey.state & ShiftMask) ? 1 : 0);
                //XUnlockDisplay(display);
                if (keySys == XK_Escape) return;

            }
                break;
        }
    }
}


int main() {

    XInitThreads();

    auto display = openDisplay();
    auto window = createWindow(display);

    std::thread thread1([&display]() {
        messageLoop(display);
    });
    std::thread thread2([&display]() {
        messageLoop(display);
    });
    thread1.join();
    thread2.join();


    destroyWindow(display, window);
    closeDisplay(display);

    return 0;
}