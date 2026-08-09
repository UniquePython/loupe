#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define DISPLAY_STRING "Example X11 Code"

static Display *display;
static int screen;
static Window win;
static Atom WMDeleteMessage;
static bool running;

static void createWindow(void)
{
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Failed to open display\n");
        exit(1);
    }

    screen = XDefaultScreen(display);
    Window rootwin = XRootWindow(display, screen);
    win = XCreateSimpleWindow(display, rootwin,
                              100, 10,
                              WINDOW_WIDTH, WINDOW_HEIGHT, 5,
                              XBlackPixel(display, screen), XWhitePixel(display, screen));

    XSizeHints sizeHints = {
        .flags = PSize | PMinSize | PMaxSize,
        .min_width = WINDOW_WIDTH,
        .max_width = WINDOW_WIDTH,
        .min_height = WINDOW_HEIGHT,
        .max_height = WINDOW_HEIGHT,
    };

    (void)XSetStandardProperties(display, win, "Simple Window", "window", 0, NULL, 0, &sizeHints);
    (void)XSelectInput(display, win, ButtonPressMask | KeyPressMask | PointerMotionMask | ExposureMask);
    (void)XMapWindow(display, win);

    WMDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    (void)XSetWMProtocols(display, win, &WMDeleteMessage, 1);

    running = true;
}

static void closeWindow(void)
{
    (void)XDestroyWindow(display, win);
    (void)XCloseDisplay(display);
}

static void drawScreen(void)
{
    (void)XDrawString(display, win, DefaultGC(display, screen), 10, 50, DISPLAY_STRING, (int)strlen(DISPLAY_STRING));
}

static void handleEvent(void)
{
    XEvent xev;

    (void)XNextEvent(display, &xev);

    switch (xev.type)
    {
    case Expose:
        drawScreen();
        break;

    case ClientMessage:
        if ((Atom)(xev.xclient.data.l[0]) == WMDeleteMessage)
            running = false;
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
    createWindow();
    while (running)
        handleEvent();
    closeWindow();
    return 0;
}
