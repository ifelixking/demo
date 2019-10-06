
#include <cassert>
#include <string>

// opengl
#include <GL/glew.h>
// x
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>

const auto EVENT_MASK = ExposureMask | KeyPressMask | StructureNotifyMask;

struct Device {
    Display *display;
    Window windowRoot;
    int defaultScreen;
    GLXFBConfig *fbConfig;
    XVisualInfo *visualInfo;
    Colormap colormap;
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
void openDevice(Device & device){
    auto displayName = getenv("DISPLAY"); assert(displayName);
    device.display = XOpenDisplay(displayName); assert(device.display);
    device.defaultScreen = DefaultScreen(device.display);
    int nelements, att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DOUBLEBUFFER, True, GLX_DEPTH_SIZE, 16, None};
    device.fbConfig = glXChooseFBConfig(device.display, device.defaultScreen, att, &nelements); assert(device.fbConfig);
    device.visualInfo = glXGetVisualFromFBConfig(device.display, *device.fbConfig); assert(device.visualInfo);
    device.windowRoot = RootWindow(device.display, device.defaultScreen);
    device.colormap = XCreateColormap(device.display, device.windowRoot, device.visualInfo->visual, AllocNone);
}
void closeDevice(const Device & device){
    XFreeColormap(device.display, device.colormap);
    XFree(device.fbConfig);
    XFree(device.visualInfo);
    XCloseDisplay(device.display);
}

Window createWindow(const Device & device){
    XSetWindowAttributes swa;
    swa.colormap = device.colormap;
    swa.event_mask = EVENT_MASK;
    unsigned long valueMask = CWColormap | CWEventMask;
    auto window = XCreateWindow(device.display, device.windowRoot, 100, 100, 320, 240, 0,
                                device.visualInfo->depth, InputOutput, device.visualInfo->visual,
                                valueMask, &swa);
    XMapWindow(device.display, window);
    XFlush(device.display);
    return window;
}
void destroyWindow(const Device & device, Window window){
    auto iResult = XUnmapWindow(device.display, window); assert(iResult);
    iResult = XDestroyWindow(device.display, window); assert(iResult);
}

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

void initGLEW(const Device & device, GLXContext context){
    MakeCurrent mc(device.display, device.windowRoot, context);
    auto err = glewInit(); assert(GLEW_OK == err);
}
// ===========================================================================================


Device g_device;
GLXContext  g_context;
Window g_window;
bool g_exitFlag = false;

// ===========================================================================================
GLuint buffer;
void initResource(const Device & device, Window window, GLXContext context){
    MakeCurrent mc(device.display, window, context);

    float data[] = {
            0.5f, 0.5f,
            -0.5f, 0.5f,
            -0.5f, -0.5f,
            0.5f, -0.5f,
    };

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
}

void destroyResource(const Device & device, Window window, GLXContext context){
    MakeCurrent mc(device.display, window, context);
    glDeleteBuffers(1, &buffer);
}

void render(){
    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(float)<<1, 0);
    glDrawArrays(GL_QUADS, 0, 4);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void eventLoopWithRender(Display * display, GLXContext context){
    XEvent event;
    for (;;){
        XNextEvent(display, &event);
        switch (event.type){
            case Expose:{
                MakeCurrent mc(display, event.xexpose.window, context);
                render();
                glXSwapBuffers(display, event.xexpose.window);
            } break;
            case ConfigureNotify: break;
            case KeyPress: {
                auto keySys = XkbKeycodeToKeysym(display, event.xkey.keycode, 0,
                                                 (event.xkey.state & ShiftMask) ? 1 : 0);
                if (keySys == XK_Escape) return;
            }break;
        }
    }
}

void *threadRender(void *) {

    auto context = createGLContext(g_device, g_context);

    // initResource(device, window1, context);

    eventLoopWithRender(g_device.display, context);

    // destroyResource(device, window1, context);


    destroyGLContext(g_device, context);
    g_exitFlag = true;
    return nullptr;
}

int main(){

    openDevice(g_device);
    g_context = createGLContext(g_device, nullptr);
    initGLEW(g_device, g_context);
    g_window = createWindow(g_device);

    initResource(g_device, g_device.windowRoot, g_context);

    pthread_t thread;
    auto iResult = pthread_create(&thread, nullptr, threadRender, nullptr); assert(iResult == 0);

    for (;!g_exitFlag;){
        pthread_yield();
    }

    destroyResource(g_device, g_device.windowRoot, g_context);

    destroyGLContext(g_device, g_context);
    closeDevice(g_device);
    return 0;
}