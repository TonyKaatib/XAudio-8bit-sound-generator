#pragma once

#ifndef WINDOW_H
#define WINDOW_H

#define WIN_32_LEAN_AND_MEAN

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

#include <Windows.h>
#include "keyboard.h"

#endif // !WINDOW_H