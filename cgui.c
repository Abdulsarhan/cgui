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

int main() {
    // 1. Connect to X11 and get the XCB connection
    Display *display = XOpenDisplay(NULL);
    if (!display) fatal("Failed to open X display");

    xcb_connection_t *xcb_conn = XGetXCBConnection(display);
    int default_screen = DefaultScreen(display);

    // 2. Get XCB screen
    const xcb_setup_t *setup = xcb_get_setup(xcb_conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for (int i = 0; i < default_screen; ++i) xcb_screen_next(&iter);
    xcb_screen_t *screen = iter.data;
    // 3. Create XCB window
    xcb_window_t window = xcb_generate_id(xcb_conn);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = {
        screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
    };

    xcb_create_window(
        xcb_conn,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0, 800, 600,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask, values
    );

    xcb_map_window(xcb_conn, window);
    xcb_flush(xcb_conn);
    XVisualInfo vinfo_template;
    int nitems;
    vinfo_template.screen = default_screen;
    XVisualInfo* vinfo = XGetVisualInfo(display, mask, &vinfo_template, &nitems);
    // 4. Create OpenGL context using GLX
    GLXContext ctx = glXCreateContext(
        display,
        vinfo,
        NULL,
        GL_TRUE
    );
    if (!ctx) fatal("Failed to create GLX context");

    // 5. Create X11 window from XCB window
    glXMakeCurrent(display, window, ctx);

    // 6. Render loop
    for (;;) {
        xcb_generic_event_t *event;
        while ((event = xcb_poll_for_event(xcb_conn))) {
            switch (event->response_type & ~0x80) {
                case XCB_KEY_PRESS:
                    free(event);
                    goto exit_loop;
            }
            free(event);
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glXSwapBuffers(display, window);
    }

exit_loop:
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, ctx);
    xcb_destroy_window(xcb_conn, window);
    xcb_disconnect(xcb_conn);
    XCloseDisplay(display);
    return 0;
}
