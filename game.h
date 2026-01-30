#pragma once

#include "window.h"

class Game {
public:
	static int Init(void* parms, int num_parms, HWND hWnd);
	static int Logic(void* parms, int num_parms);
	static int Shutdown(void* parms, int num_parms);
};
