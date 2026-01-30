#include "window.h"
#include "game.h"

HWND ghMainWnd;
Game* game = new Game;

bool LockInstance();
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_HAND);
	wc.hCursor = LoadIcon(0, IDI_HAND);
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"Class name";
	wc.hIconSm = LoadIcon(0, IDI_HAND);

	if (!RegisterClassEx(&wc)) {
		MessageBox(0, L"RegisterClass Failed! Time to debug >:)", 0, 0);
		return 1;
	}

	ghMainWnd = CreateWindowEx(
		0,
		L"Class name",
		L"Hyperborean Physics",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		0,
		0,
		hInstance,
		0);

	if (ghMainWnd == 0) {
		MessageBox(0, L"CreateWindow FAILED! Time to debug >:)", 0, 0);
		return 1;
	}

	ShowWindow(ghMainWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(ghMainWnd);

	MSG msg;

	if (LockInstance())
		return false;

	game->Init(NULL, 0, ghMainWnd);
	while (TRUE) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				return (int)msg.wParam;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} //end if

		//Game Logic will go here
		game->Logic(NULL, 0);
	}//end while
	game->Shutdown(NULL, 0);
	return static_cast<int>(msg.wParam);
}

bool LockInstance() {
	HANDLE mutex;

	mutex = CreateMutex(NULL, true, L"Did you know 98.1% of all gamblers quit playing before hitting the ultiumate jackpot? True Story!");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return true;

	return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	switch (msg) {
	case WM_CREATE:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		int result = MessageBox(hWnd, L"Are you sure you want to quit?", L"Quitting", MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES) {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}