#ifndef PAL_WIN32_H
#define PAL_WIN32_H

// Windows
#include <windows.h>
#include <windowsx.h>

// OpenGL
#include <gl/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>

#include <stdint.h>

typedef struct pal_window {
	HWND hwnd;
	HDC hdc;
	HGLRC hglrc;
}pal_window;

static MSG s_msg = { 0 };
static HDC s_fakeDC = { 0 };
static int s_glVersionMajor = 3;
static int s_glVersionMinor = 3;
static int s_glProfile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
static int s_resizable = WS_OVERLAPPEDWINDOW;
static int s_floating = 0;
static int s_doubleBuffer = PFD_DOUBLEBUFFER;
static int s_window_should_not_close = 1;

void Win32WindowResizeCallback(HWND hwnd, UINT flag, int width, int height)
{
	// Handle resizing
}

LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Mouse Shit
	UINT button = GET_XBUTTON_WPARAM(wParam);
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	// Screen Size Shit.
	int width = LOWORD(lParam);  // Macro to get the low-order word.
	int height = HIWORD(lParam); // Macro to get the high-order word.

	switch (uMsg) {

	case WM_SIZE:

		// Respond to the message:
		Win32WindowResizeCallback(hwnd, (UINT)wParam, width, height);

		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		s_window_should_not_close = 0;
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		DestroyWindow(hwnd);
		s_window_should_not_close = 0;
		break;

	case WM_PAINT:
		break;
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_KEYUP:
		break;
	}


	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

WNDCLASSEXA RegisterWindowClass() {
	WNDCLASSEXA wc = { 0 };

	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.lpfnWndProc = win32_window_proc;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = "Win32 Window Classs";
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);

	RegisterClassExA(&wc);
	return wc;
}

static pal_window platform_create_window(int width, int height) {
	pal_window fakewindow = {0};
	WNDCLASSEXA fakewc = RegisterWindowClass();

	fakewindow.hwnd = CreateWindowExA(
		0,                              // Optional window styles.
		fakewc.lpszClassName,                     // Window class
		"Fake Ass Window.",          // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		fakewc.hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (fakewindow.hwnd == NULL)
	{
		return fakewindow;
	}

	s_fakeDC = GetDC(fakewindow.hwnd);

	PIXELFORMATDESCRIPTOR fakePFD;
	ZeroMemory(&fakePFD, sizeof(fakePFD));
	fakePFD.nSize = sizeof(fakePFD);
	fakePFD.nVersion = 1;
	fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fakePFD.iPixelType = PFD_TYPE_RGBA;
	fakePFD.cColorBits = 32;
	fakePFD.cAlphaBits = 8;
	fakePFD.cDepthBits = 24;

	int fakePFDID = ChoosePixelFormat(s_fakeDC, &fakePFD);

	if (fakePFDID == 0) {
		MessageBoxA(fakewindow.hwnd, "ChoosePixelFormat() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}
	if (SetPixelFormat(s_fakeDC, fakePFDID, &fakePFD) == 0) {
		MessageBoxA(fakewindow.hwnd, "SetPixelFormat() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}

	HGLRC fakeRC = wglCreateContext(s_fakeDC);
	if (fakeRC == 0) {
		MessageBoxA(fakewindow.hwnd, "wglCreateContext() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}
	if (wglMakeCurrent(s_fakeDC, fakeRC) == 0) {
		MessageBoxA(fakewindow.hwnd, "wglMakeCurrent() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)(wglGetProcAddress("wglChoosePixelFormatARB"));
	if (wglChoosePixelFormatARB == NULL) {
		MessageBoxA(fakewindow.hwnd, "wglGetProcAddress() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)(wglGetProcAddress("wglCreateContextAttribsARB"));
	if (wglCreateContextAttribsARB == NULL) {
		MessageBoxA(fakewindow.hwnd, "wglGetProcAddress() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT == NULL) {
		MessageBoxA(fakewindow.hwnd, "wglGetProcAddress() failed.", "Try again later", MB_ICONERROR);
		return fakewindow;
	}

	WNDCLASSEXA wc = RegisterWindowClass();
	pal_window window = {0};
	window.hwnd = CreateWindowExA(
		s_floating,           // Optional window styles.
		wc.lpszClassName,     // Window class
		NULL,          // Window text
		s_resizable,          // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,

		NULL,       // Parent window    
		NULL,       // Menu
		wc.hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (window.hwnd == NULL) {
		return window;
	}

	window.hdc = GetDC(window.hwnd);

	const int pixelAttribs[] = {
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_ALPHA_BITS_ARB, 8,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
	WGL_SAMPLES_ARB, 4, // NOTE: Maybe this is used for multisampling?
	0
	};

	int pixelFormatID; UINT numFormats;
	uint8_t status = wglChoosePixelFormatARB(window.hdc, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
	if (status == 0 || numFormats == 0) {
		MessageBoxA(window.hwnd, "wglChoosePixelFormatARB() failed.", "Try again later", MB_ICONERROR);
		return window;
	}

	PIXELFORMATDESCRIPTOR PFD;
	PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | s_doubleBuffer;
	DescribePixelFormat(window.hdc, pixelFormatID, sizeof(PFD), &PFD);
	SetPixelFormat(window.hdc, pixelFormatID, &PFD);

	int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, s_glVersionMajor,
		WGL_CONTEXT_MINOR_VERSION_ARB, s_glVersionMinor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, s_glProfile,
		0
	};

	window.hglrc = wglCreateContextAttribsARB(window.hdc, 0, contextAttribs);
	if (window.hglrc) {

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(fakeRC);
		ReleaseDC(fakewindow.hwnd, s_fakeDC);
		DestroyWindow(fakewindow.hwnd);

		ShowWindow(window.hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(window.hwnd);
		SetFocus(window.hwnd);
		OutputDebugStringA("INFO: Using modern OpenGL Context.");
		return window;
	}
	else {

		ShowWindow(fakewindow.hwnd, SW_SHOW);
		SetForegroundWindow(fakewindow.hwnd);
		SetFocus(fakewindow.hwnd);
		OutputDebugStringA("INFO: Using old OpenGL Context.");
		return fakewindow;
	}

}

void platform_makecurrent(pal_window window) {
	wglMakeCurrent(window.hdc, window.hglrc);
}

void platform_swapbuffers(pal_window window) {
	SwapBuffers(window.hdc);
}

void make_context_current(pal_window window) {
	platform_makecurrent(window);
}

void swap_buffers(pal_window window) {
	platform_swapbuffers(window);
}

uint8_t pal_window_should_close(pal_window window) {
	return s_window_should_not_close;
}

void platform_poll_events(pal_window* window) {

	while (PeekMessageA(&s_msg, NULL, 0, 0, PM_REMOVE)) {
		if (s_msg.message == WM_QUIT) {
			s_window_should_not_close = 0;
			exit(0);
		}
		TranslateMessage(&s_msg);
		DispatchMessageA(&s_msg);
	}

}

void pal_poll_events(pal_window* window) {
	platform_poll_events(window);
}

void* platform_gl_get_proc_address(const unsigned char* procname) {
	return wglGetProcAddress(procname);
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
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
	}
}

void platform_get_mouse_position(pal_window window) {
	if (!window.hwnd) return;

	POINT p;
	if (GetCursorPos(&p)) {
		if (ScreenToClient(window.hwnd, &p)) {
			printf("Mouse position (client coords): %d, %d\n", p.x, p.y);
		}
	}
}

void get_mouse_position(pal_window window) {
	platform_get_mouse_position(window);
}
#endif // PAL_WIN32_H
