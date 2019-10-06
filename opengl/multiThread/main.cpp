
#include <cassert>
#include <string>
#include <list>
#include <vector>

// opengl
#include <GL/glew.h>
// x
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
#include <pthread.h>

const auto EVENT_MASK = ExposureMask | KeyPressMask | StructureNotifyMask;

struct Device {
    Display *display;
    Window windowRoot;
    int defaultScreen;
    GLXFBConfig *fbConfig;
    XVisualInfo *visualInfo;
    Colormap colormap;
};

pthread_mutex_t g_makecurrent_lock;

struct MakeCurrent{
    Display * m_display;
    GLXDrawable m_oldWindow;
    GLXContext m_oldContext;
    MakeCurrent(const Display * display, GLXDrawable win, GLXContext context)
            : m_display((Display *)display)
    {
    	pthread_mutex_lock(&g_makecurrent_lock);
        m_oldContext = glXGetCurrentContext();
        m_oldWindow = glXGetCurrentDrawable();
        glXMakeCurrent(m_display, win, context);
    }
    ~MakeCurrent() {
        glXMakeCurrent(m_display, m_oldWindow, m_oldContext);
		pthread_mutex_unlock(&g_makecurrent_lock);
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

const int DIM = 10;
const float SIZE = 1.0f/DIM;

class Renderable{
public:
	Renderable(int position):
		m_position(position){
	}
	// call in opengl resource thread
	void initResource(){
		float x = (float)(m_position % DIM) / DIM;
		float y = (float)(m_position / DIM) / DIM;
		float data[8] = {
				x,y,
				x-SIZE,y,
				x-SIZE,y-SIZE,
				x,y-SIZE,
		};
		MakeCurrent mc(g_device.display, g_device.windowRoot, g_context);
		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	}
	// call in opengl resource thread
	void freeResource(){
		MakeCurrent mc(g_device.display, g_device.windowRoot, g_context);
		glDeleteBuffers(1, &m_vbo);
	}
	// call in opengl render thread
	void render() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(float)<<1, 0);
		glDrawArrays(GL_QUADS, 0, 4);
	}

private:
	int m_position;
	GLuint m_vbo;
};

// ===========================================================================================


pthread_mutex_t g_newRenderables_lock;
std::list<Renderable> g_newRenderables;
pthread_mutex_t g_deleteRenderables_lock;
std::list<Renderable> g_deleteRenderables;

void eventLoopWithRender(Display * display, GLXContext context){
    XEvent event;
    std::vector<Renderable> tmp;
    for (;;){
		if (XCheckMaskEvent(display, EVENT_MASK, &event)){
			switch (event.type){
				case Expose: break;
				case ConfigureNotify: break;
				case KeyPress: {
					auto keySys = XkbKeycodeToKeysym(display, event.xkey.keycode, 0,
													 (event.xkey.state & ShiftMask) ? 1 : 0);
					if (keySys == XK_Escape) return;
				}break;
			}
		}else{
			MakeCurrent mc(display, g_window, context);

			glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			pthread_mutex_lock(&g_newRenderables_lock);
			tmp.assign(g_newRenderables.begin(), g_newRenderables.end());
			g_newRenderables.clear();
			pthread_mutex_unlock(&g_newRenderables_lock);

			for(auto & item : tmp){
				item.render();
			}

			pthread_mutex_lock(&g_deleteRenderables_lock);
			g_deleteRenderables.insert(g_deleteRenderables.end(), tmp.begin(), tmp.end());
			pthread_mutex_unlock(&g_deleteRenderables_lock);

			glXSwapBuffers(display, g_window);
			pthread_yield();
		}
    }
}

void *threadRender(void *) {
    auto context = createGLContext(g_device, g_context);
    eventLoopWithRender(g_device.display, context);
    destroyGLContext(g_device, context);
    g_exitFlag = true;
    return nullptr;
}

int main(){
	XInitThreads();

	auto iResult =  pthread_mutex_init(&g_makecurrent_lock, nullptr); assert(iResult == 0);
	iResult =  pthread_mutex_init(&g_newRenderables_lock, nullptr); assert(iResult == 0);
	iResult =  pthread_mutex_init(&g_deleteRenderables_lock, nullptr); assert(iResult == 0);

    openDevice(g_device);
    g_context = createGLContext(g_device, nullptr);
    initGLEW(g_device, g_context);
    g_window = createWindow(g_device);




//	{
//		Renderable renderable(1);
//		renderable.initResource();
//
//		pthread_mutex_lock(&g_newRenderables_lock);
//		g_newRenderables.push_back(renderable);
//		pthread_mutex_unlock(&g_newRenderables_lock);
//
//		// pthread_mutex_lock(&g_deleteRenderables_lock);
//		// tmp.assign(g_deleteRenderables.begin(), g_deleteRenderables.end());
//		// g_deleteRenderables.clear();
//		// pthread_mutex_unlock(&g_deleteRenderables_lock);
//	}

    pthread_t thread;
    iResult = pthread_create(&thread, nullptr, threadRender, nullptr); assert(iResult == 0);

    std::vector<Renderable> tmp;

    for (int i=0;!g_exitFlag;++i){


    	//if (i < 100) {
			Renderable renderable(i);
			renderable.initResource();

			pthread_mutex_lock(&g_newRenderables_lock);
			g_newRenderables.push_back(renderable);
			pthread_mutex_unlock(&g_newRenderables_lock);

			pthread_mutex_lock(&g_deleteRenderables_lock);
			tmp.assign(g_deleteRenderables.begin(), g_deleteRenderables.end());
			g_deleteRenderables.clear();
			pthread_mutex_unlock(&g_deleteRenderables_lock);
		// }
			for (auto &item : tmp) {
				item.freeResource();
			}


        pthread_yield();
    }

    destroyWindow(g_device, g_window);
    destroyGLContext(g_device, g_context);
    closeDevice(g_device);

	iResult = pthread_mutex_destroy(&g_newRenderables_lock); assert(iResult == 0);
	iResult = pthread_mutex_destroy(&g_deleteRenderables_lock); assert(iResult == 0);

    return 0;
}