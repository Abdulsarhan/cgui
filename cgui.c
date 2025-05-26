#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "pal.h"

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

int main() {
    pal_window window = platform_create_window();
    window.window_should_close = false;

    make_context_current(window);

    if (!gladLoadGLLoader((GLADloadproc)gl_get_proc_address)) {
        fprintf(stderr, "ERROR: Failed to intialize glad!\n");
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    float vertices[] = {
	0.15f,  0.15f, 0.0f,  // top right
	0.15f, -0.15f, 0.0f,  // bottom right
       -0.15f, -0.15f, 0.0f,  // bottom left
       -0.15f,  0.15f, 0.0f   // top left
    };
    unsigned int indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3 // second triangle
    };

    glViewport(0, 0, 1920, 1080);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    /* VERTEX SHADER */
    const char* vertexShaderSource = "#version 330 core\n"
	    "layout (location = 0) in vec3 aPos;\n"
	    "void main()\n"
	    "{\n"
	    " gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	    "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    CheckCompilerErrors(vertexShader);

    /* FRAGMENT SHADER */
    const char* fragmentShaderSource = "#version 330 core\n"
	    "out vec4 FragColor;\n"
	    "void main()\n"
	    "{\n"
	    "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	    "}\0";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

/* LINKING VERTEX AND FRAGMENT SHADER */
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /* VBO */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(shaderProgram);

    /* EBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* VAO */
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while (!pal_window_should_close(window)) {
        pal_poll_events(&window);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        get_mouse_position(window);
        swap_buffers(window);

        glClear(GL_COLOR_BUFFER_BIT);
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
