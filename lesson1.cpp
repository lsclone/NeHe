#include <windows.h>

#include <glew.h>
#include <glut.h>

bool CreateGLWindow(char *title, int width, int height,
		int bits) {
	WNDCLASS Wnd;

	Wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	Wnd.lpfnWndProc = (WNDPROC) WndProc;
	Wnd.cbClsExtra = 0;
	Wnd.cbWndExtra = 0;
	Wnd.hInstance = GetModuleHandle(NULL);
	Wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	Wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	Wnd.hbrBackground = NULL;
	Wnd.lpszMenuName = NULL;
	Wnd.lpszClassName = "londontown";

	if (!RegisterClass(&Wnd)) {
		return false;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SYSCOMMAND: {
			switch (wParam) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
			break;
		}
		case WM_KEYDOWN: {
			keys[wParam] = true;
			return 0;
		}
		case WM_KEYUP: {
			keys[wParam] = false;
			return 0;
		}
		case WM_CLOSE: {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance,
		LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	//bool     

	if (!CreateGLWindow("NeHe's first OpenGL framwork", 640, 480, 16, 0)) {
		return 0;
	}

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				done = true;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			DrawGLScene();
			SwapBuffers(hDC);	//double buffers
		}
	}

	KillGLWindow();
	return msg.wParam;
}