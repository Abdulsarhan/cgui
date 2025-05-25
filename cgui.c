#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <GL/gl.h>
#include <GL/glx.h>
// the idea is that layers contain a layout (or multiple layouts) and the layer specifies
// how much space each layout takes up. and the layout contains the UI elements that the
// user will see and/or will be interacting with (buttons, text, textboxes,etc...)
// I will need to hook up opengl to this in order for me to see the actual ui.

#define true 1
#define false 0

typedef enum button_flags {
    is_hovering = 1,
    is_blank = 2,
}button_flags;

typedef struct vec2 {
    float x;
    float y;
}vec2;

typedef struct button {
    vec2 top_right;
    vec2 bottom_right;
    vec2 bottom_left;
    vec2 top_left;
    button_flags flags;
}button;

typedef struct textbox {
    vec2 top_right;
    vec2 bottom_right;
    vec2 bottom_left;
    vec2 top_left;
    button_flags flags;
}textbox;

typedef struct text {
    vec2 top_right;
    vec2 bottom_right;
    vec2 bottom_left;
    vec2 top_left;
    button_flags flags;
}text;

typedef struct layout {
    button* buttons;
    textbox* textboxes;
    text* text;
}layout;

typedef struct layer {
    layout* layouts;
}layer;

void fatal(const char* string) {
    printf("%s", string);
}

typedef struct pal_window {
    Display *display;
    xcb_connection_t *xcb_conn;
    xcb_window_t window;
    GLXContext ctx;
    uint8_t window_should_close;
}pal_window;

pal_window platform_create_window() {
    pal_window window = {0};
    // 1. Connect to X11 and get the XCB connection
    window.display = XOpenDisplay(NULL);
    if (!window.display) fatal("Failed to open X display");

    window.xcb_conn = XGetXCBConnection(window.display);
    int default_screen = DefaultScreen(window.display);

    // 2. Get XCB screen
    const xcb_setup_t *setup = xcb_get_setup(window.xcb_conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for (int i = 0; i < default_screen; ++i) xcb_screen_next(&iter);
    xcb_screen_t *screen = iter.data;
    // 3. Create XCB window
    window.window = xcb_generate_id(window.xcb_conn);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = {
        screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
    };

    xcb_create_window(
        window.xcb_conn,
        XCB_COPY_FROM_PARENT,
        window.window,
        screen->root,
        0, 0, 800, 600,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask, values
    );

    xcb_map_window(window.xcb_conn, window.window);
    xcb_flush(window.xcb_conn);
    XVisualInfo vinfo_template;
    int nitems;
    vinfo_template.screen = default_screen;
    XVisualInfo* vinfo = XGetVisualInfo(window.display, mask, &vinfo_template, &nitems);
    // 4. Create OpenGL context using GLX
    window.ctx = glXCreateContext(
        window.display,
        vinfo,
        NULL,
        GL_TRUE
    );
    if (!window.ctx) fatal("Failed to create GLX context");

    // 5. Create X11 window from XCB window
    return window;
}

void platform_makecurrent(pal_window window) {
    glXMakeCurrent(window.display, window.window, window.ctx);
}

void platform_swapbuffers(pal_window window) {
    glXSwapBuffers(window.display, window.window);
}

void make_context_current(pal_window window) {
     platform_makecurrent(window);
}

void swap_buffers(pal_window window) {
    platform_swapbuffers(window);
}

uint8_t pal_window_should_close(pal_window window) {
    return window.window_should_close;
}

void platform_poll_events(pal_window window) {
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(window.xcb_conn))) {
        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
                free(event);
                window.window_should_close = true;
        }
    }
}

void pal_poll_events(pal_window window) {
    platform_poll_events(window);
}

int main() {
    pal_window window = platform_create_window();

    make_context_current(window);
    
    while (!pal_window_should_close(window)) {
        pal_poll_events(window);


        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        swap_buffers(window);
    }

    /* None of this shit really matters, the OS can deal with this. */
    /*
    glXMakeCurrent(window.display, None, NULL);
    glXDestroyContext(window.display, window.ctx);
    xcb_destroy_window(window.xcb_conn, window.window);
    xcb_disconnect(window.xcb_conn);
    XCloseDisplay(window.display);
    */
    return 0;
}
