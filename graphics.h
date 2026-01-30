#pragma once

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "window.h"
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wincodec.h>

class GraphicsEngine {
protected:
	ID3D11Device* pD3Device;
	ID3D11DeviceContext* pD3DContext;
	IDXGISwapChain1* pSwap;
	IDXGIAdapter* pAdapter;
	IDXGIFactory2* pDxgiFactory;
	ID3D11Texture2D* pBackBuffer;
	IDXGISurface* pSurface;
	ID2D1Bitmap1* pBitmap;
	//--//
	IDWriteFactory* pWriteFactory;
	IWICImagingFactory* pIWICFactory;
	//--//
	ID2D1Factory4* pFactory;
	IDXGIDevice1* pDxgi;
	ID2D1Device3* pDevice;
	ID2D1DeviceContext3* pContext;
	//--//
	RECT rc;
public:
	GraphicsEngine();
	~GraphicsEngine();
	HRESULT InitDevice(HWND hWnd);
	HRESULT Render(void* parms, int num_parms, float parameter);
};

class Text : public GraphicsEngine{
protected:
	const WCHAR* text_content;
	UINT32 text_length;
	IDWriteTextFormat* text_format;
	ID2D1SolidColorBrush* text_colour;
public:
	Text();
	~Text();
	void InitText(PCWSTR uri, IDWriteTextFormat* pTextFormat, ID2D1SolidColorBrush* pTextColour);
	void RenderText(ID2D1DeviceContext3* pContext, float parameter);
};

#endif // !GRAPHICS_H