#ifndef PAL_H
#define PAL_H

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
    if (!window.display) printf("Failed to open X display");

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
    if (!window.ctx) printf("Failed to create GLX context");

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

void platform_poll_events(pal_window* window) {
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(window->xcb_conn))) {
        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
                free(event);
      //          window->window_should_close = true;
        }
    }
}

void pal_poll_events(pal_window* window) {
    platform_poll_events(window);
}

void* platform_gl_get_proc_address(const unsigned char* procname) {
    return glXGetProcAddress(procname);
}
void* gl_get_proc_address(const unsigned char* procname) {
    return platform_gl_get_proc_address(procname);
}

void CheckCompilerErrors(unsigned int vertexShader) {
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n, %s\n", infoLog);
	}
}

void platform_get_mouse_position(pal_window window) {
    xcb_query_pointer_cookie_t cookie = xcb_query_pointer(window.xcb_conn, window.window);
    xcb_query_pointer_reply_t *reply = xcb_query_pointer_reply(window.xcb_conn, cookie, NULL);

    if (reply) {
        printf("Mouse position in window: (%d, %d)\n", reply->win_x, reply->win_y);
        free(reply);
    } else {
        printf("Failed to query pointer.\n");
    }
}

void get_mouse_position(pal_window window) {
    platform_get_mouse_position(window);
}
#endif // PAL_H
