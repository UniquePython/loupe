#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>

typedef struct App
{
    bool running;
    Display *display;
    int screen;
    Window win;
    Colormap cmap;
    GLXContext glc;
    Atom wmDeleteMessage;
    const char *message;

} App;

static void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(EXIT_FAILURE);
}

static App createApp(int width, int height, const char *message)
{
    App result = {0};

    result.message = message;
    result.display = XOpenDisplay(NULL);
    if (result.display == NULL)
        die("Failed to open display");

    result.screen = XDefaultScreen(result.display);

    int glxMajor, glxMinor;
    if (!glXQueryVersion(result.display, &glxMajor, &glxMinor))
        die("Failed to query GLX version");
    if (glxMajor < 1 || (glxMajor == 1 && glxMinor < 3))
        die("Invalid GLX version. Expected >= 1.3, found %d.%d", glxMajor, glxMinor);
    printf("GLX version: %d.%d\n", glxMajor, glxMinor);
    printf("GLX extension: %s\n", glXQueryExtensionsString(result.display, result.screen));

    int att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    Window rootwin = XRootWindow(result.display, result.screen);
    XVisualInfo *vi = glXChooseVisual(result.display, result.screen, att);
    if (vi == NULL)
        die("No appropriate visual found.");
    printf("Visual %lu selected\n", vi->visualid);

    result.cmap = XCreateColormap(result.display, rootwin, vi->visual, AllocNone);
    XSetWindowAttributes swa = {
        .colormap = result.cmap,
        .event_mask = ButtonPressMask | KeyPressMask | PointerMotionMask | ExposureMask,
    };

    result.win = XCreateWindow(result.display, rootwin,
                               0, 0, (unsigned int)width, (unsigned int)height, 0,
                               vi->depth, InputOutput, vi->visual,
                               CWColormap | CWEventMask, &swa);

    result.glc = glXCreateContext(result.display, vi, NULL, GL_TRUE);
    XFree(vi);
    if (result.glc == NULL)
        die("Failed to create GLX context");
    if (!glXMakeCurrent(result.display, result.win, result.glc))
        die("Failed to make GLX context current");

    glEnable(GL_DEPTH_TEST);

    XSizeHints sizeHints = {
        .flags = PSize | PMinSize | PMaxSize,
        .min_width = width,
        .max_width = width,
        .min_height = height,
        .max_height = height,
    };

    (void)XSetStandardProperties(result.display, result.win, "Simple Window", "window", 0, NULL, 0, &sizeHints);
    (void)XMapWindow(result.display, result.win);
    (void)XStoreName(result.display, result.win, "Wordpress Application");

    result.wmDeleteMessage = XInternAtom(result.display, "WM_DELETE_WINDOW", False);
    (void)XSetWMProtocols(result.display, result.win, &result.wmDeleteMessage, 1);

    result.running = true;

    return result;
}

static void closeApp(const App *app)
{
    XFreeColormap(app->display, app->cmap);
    (void)glXMakeCurrent(app->display, None, NULL);
    glXDestroyContext(app->display, app->glc);
    (void)XDestroyWindow(app->display, app->win);
    (void)XCloseDisplay(app->display);
}

static void drawApp(const App *app)
{
    XWindowAttributes gwa = {0};
    (void)XGetWindowAttributes(app->display, app->win, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);
    glXSwapBuffers(app->display, app->win);
}

static void handleEvent(App *app)
{
    XEvent xev;

    (void)XNextEvent(app->display, &xev);

    switch (xev.type)
    {
    case Expose:
        drawApp(app);
        break;

    case ClientMessage:
        if ((Atom)(xev.xclient.data.l[0]) == app->wmDeleteMessage)
            app->running = false;
        break;

    case KeyPress:
    {
        KeySym key = XLookupKeysym(&xev.xkey, 0);
        if (key != NoSymbol)
            puts("Keyboard event");
        break;
    }

    case ButtonPress:
    case MotionNotify:
        puts("Mouse event");
        break;

    default:
        break;
    }
}

int main(void)
{
    App app = createApp(900, 600, "App!");
    while (app.running)
        handleEvent(&app);
    closeApp(&app);
    return 0;
}
