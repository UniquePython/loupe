#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

typedef struct App
{
    bool running;
    Display *display;
    int screen;
    Window win;
    Atom wmDeleteMessage;
    const char *message;

} App;

static App createApp(int width, int height, const char *message)
{
    App result = {0};

    result.message = message;
    result.display = XOpenDisplay(NULL);
    if (result.display == NULL)
    {
        fprintf(stderr, "Failed to open display\n");
        exit(1);
    }

    result.screen = XDefaultScreen(result.display);
    Window rootwin = XRootWindow(result.display, result.screen);
    result.win = XCreateSimpleWindow(result.display, rootwin,
                                     100, 10,
                                     (unsigned int)width, (unsigned int)height, 5,
                                     XBlackPixel(result.display, result.screen), XWhitePixel(result.display, result.screen));

    XSizeHints sizeHints = {
        .flags = PSize | PMinSize | PMaxSize,
        .min_width = width,
        .max_width = width,
        .min_height = height,
        .max_height = height,
    };

    (void)XSetStandardProperties(result.display, result.win, "Simple Window", "window", 0, NULL, 0, &sizeHints);
    (void)XSelectInput(result.display, result.win, ButtonPressMask | KeyPressMask | PointerMotionMask | ExposureMask);
    (void)XMapWindow(result.display, result.win);

    result.wmDeleteMessage = XInternAtom(result.display, "WM_DELETE_WINDOW", False);
    (void)XSetWMProtocols(result.display, result.win, &result.wmDeleteMessage, 1);

    result.running = true;

    return result;
}

static void closeApp(const App *app)
{
    (void)XDestroyWindow(app->display, app->win);
    (void)XCloseDisplay(app->display);
}

static void drawApp(const App *app)
{
    (void)XDrawString(app->display,
                      app->win,
                      DefaultGC(app->display, app->screen),
                      10, 50,
                      app->message, (int)strlen(app->message));
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
