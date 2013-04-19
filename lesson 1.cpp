#include <windows.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include <gl\glaux.h>

HGLRC hRC = NULL;	// permanent rendering context
HDC hDC = NULL;		// private GDI device context
HWND hWnd = NULL;
HINSTANCE hInstance;

BOOL keys[256];
BOOL active = TRUE;
BOOL isfullscreen = FALSE;

// declaration for 
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

GLvoid resizeGL(GLsizei width, GLsizei height) {
		if (height == 0)	// prevent a divide by zero
			height = 1;

		glViewport(0, 0, width, height);	//reset the current viewport

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//
		gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
}

// all setup for OpenGL goes here
int initGL(GLvoid) {
	glShadeModel(GL_SMOOTH);	//enables smooth shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	//black background
	// depth buffer
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

int drawGL(GLvoid) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//clear the screen and the deep buffer
	glLoadIdentity();
	return TRUE;
}

GLvoid killGL(GLvoid) {
	if (isfullscreen) {
		ChangeDisplaySettings(NULL, 0);		//to original desktop
		ShowCursor(TRUE);
	}
	//....
	if (hRC) {
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "release of DC & RC failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		if (!wglDeleteContext(hRC))
			MessageBox(NULL, "release rendering context failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hRC = NULL;
	}
	//
	if (hDC && !ReleaseDC(hWnd, hDC)) {
		MessageBox(NULL, "release device context failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	//
	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL, "could not release hWnd", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}
	//
	if (!UnregisterClass("londontown", hInstance)) {
		MessageBox(NULL, "could not unregister class", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

int createGL(char * title, int width, int height, int bits, BOOL fullscreenflag) {
	GLuint PixelFormat;

	WNDCLASS wnd;

	DWORD dwExStyle;
	DWORD dwStyle;

	RECT windowRect;
	windowRect.left = (long)0;
	windowRect.right = (long)width;
	windowRect.top = (long)0;
	windowRect.bottom = (long)height;

	hInstance = GetModuleHandle(NULL);
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd.lpfnWndProc = (WNDPROC) WndProc;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInstance;
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = NULL;
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = "londontown";

	if (!RegisterClass(&wnd)) {
		MessageBox(NULL, "failed to register the window class", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	isfullscreen = fullscreenflag;
	
	if (isfullscreen) {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			if (MessageBox(NULL, "the requested fullscreen mode is not supported by\n your video card.\
use windowed mode instead?", "londontown", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				isfullscreen = FALSE;
			} else {
				MessageBox(NULL, "program will now close", "londontown", MB_OK | MB_ICONSTOP);
				return FALSE;
			}
		}
	}

	if (isfullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);		// hide mouse pointer
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(dwExStyle,
								"londontown",
								title,
								WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
								0, 0,
								windowRect.right - windowRect.left,
								windowRect.bottom - windowRect.top,
								NULL,
								NULL,
								hInstance,
								NULL))) {
		killGL();
		MessageBox(NULL, "window creation error.", "error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(hDC = GetDC(hWnd))) {
		killGL();
		MessageBox(NULL, "cannot create a GL device context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		killGL();
		MessageBox(NULL, "cannot find a suitable PixelFormat.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		killGL();
		MessageBox(NULL, "cannot set the PixelFormat.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!(hRC = wglCreateContext(hDC))) {
		killGL();
		MessageBox(NULL, "cannot create a GL rendering context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!wglMakeCurrent(hDC, hRC)) {
		killGL();
		MessageBox(NULL, "cannot activate the GL rendering context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	resizeGL(width, height);

	if (!initGL()) {
		killGL();
		MessageBox(NULL, "Initialization failed.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_ACTIVATE : {
			if (!HIWORD(wParam))
				active = TRUE;
			else
				active = FALSE;
			return 0;
		}
		case WM_SYSCOMMAND : {
			switch (wParam) {
				case SC_SCREENSAVE :
				case SC_MONITORPOWER :
					return 0;
			}
			break;
		}
		case WM_CLOSE : {
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN : {
			keys[wParam] = TRUE;
			return 0;
		}
		case WM_KEYUP : {
			keys[wParam] = FALSE;
			return 0;
		}
		case WM_SIZE : {
			resizeGL(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow) {
	MSG msg;
	int done = FALSE;

	if (MessageBox (NULL, "Would you like to run in fullscreen mode?", "Start fullscreen",
			MB_YESNO | MB_ICONQUESTION) == IDNO) {
		isfullscreen = FALSE;	// windowed mode
	}

	if (!createGL("Lua's OpenGL Framework", 640, 480, 16, isfullscreen))
		return 0;

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				done = TRUE;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			if (active) {
				if (keys[VK_ESCAPE])
					done = TRUE;
				else {
					drawGL();
					SwapBuffers(hDC);
				}
			}

			if (keys[VK_F1]) {
				keys[VK_F1] = FALSE;
				killGL();
				isfullscreen = !isfullscreen;

				if (!createGL("Lua's OpenGL Framework", 640, 480, 16, isfullscreen))
					return 0;
			}
		}
	}

	killGL();
	return msg.wParam;
}