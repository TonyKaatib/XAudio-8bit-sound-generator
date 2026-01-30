#include "graphics.h"
#include <math.h>
#include <corecrt_wstdio.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Windowscodecs.lib")

Text* text = new Text;
IDWriteTextFormat* defaultTextFormat;
ID2D1SolidColorBrush* defaultTextColour;

GraphicsEngine::GraphicsEngine() {
	pD3Device = 0;
	pD3DContext = 0;
	pSwap = 0;
	pAdapter = 0;
	pDxgiFactory = 0;
	pBackBuffer = 0;
	pSurface = 0;
	pBitmap = 0;
	//--//
	pWriteFactory = 0;
	pIWICFactory = 0;
	//--//
	pFactory = 0;
	pDxgi = 0;
	pDevice = 0;
	pContext = 0;
	ZeroMemory(&rc, sizeof(RECT));
};

HRESULT GraphicsEngine::InitDevice(HWND hWnd) {
	HRESULT hr = S_OK;

	GetClientRect(hWnd, &rc);

	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create D2D1 Factory!", 0, 0);
		return hr;
	}

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE
	};

	UINT totalDriverTypes = ARRAYSIZE(driverTypes);

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		0,
		0,
		D3D11_SDK_VERSION,
		&pD3Device,
		nullptr,
		&pD3DContext
	);

	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create D3D11 Device!", 0, 0);
		return hr;
	}

	hr = pD3Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgi);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to Query Interface DXGI Device!", 0, 0);
		return hr;
	}

	hr = pFactory->CreateDevice(pDxgi, &pDevice);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create D2D1 Device!", 0, 0);
		return hr;
	}

	hr = pDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pContext);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create D2D1 Device Context!", 0, 0);
		return hr;
	}

	DXGI_SWAP_CHAIN_DESC1 sd = { };
	ZeroMemory(&sd, sizeof(sd));
	sd.Width = width;
	sd.Height = height;
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.Stereo = false;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Scaling = DXGI_SCALING_NONE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	sd.Flags = 0;

	hr = pDxgi->GetAdapter(&pAdapter);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create DXGI Adapter!", 0, 0);
		return hr;
	}

	hr = pAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to get parent!", 0, 0);
		return hr;
	}

	hr = pDxgiFactory->CreateSwapChainForHwnd(pD3Device, hWnd, &sd, NULL, NULL, &pSwap);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create Swap Chain!", 0, 0);
		return hr;
	}

	hr = pDxgi->SetMaximumFrameLatency(1);

	hr = pSwap->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to get Back Buffer!", 0, 0);
		return hr;
	}

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET |
		D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96.0f, 96.0f, NULL);

	hr = pSwap->GetBuffer(0, IID_PPV_ARGS(&pSurface));
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to get DXGI Back Buffer!", 0, 0);
		return hr;
	}

	hr = pContext->CreateBitmapFromDxgiSurface(pSurface, NULL, &pBitmap);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create Bitmap!", 0, 0);
		return hr;
	}

	pContext->SetTarget(pBitmap);

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**> (&pIWICFactory));
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create WIC Imaging Factory!", 0, 0);
		return hr;
	}

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory), reinterpret_cast<IUnknown**>(&pWriteFactory));
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create DWrite Factory!", 0, 0);
		return hr;
	}

	if (SUCCEEDED(hr)) {
		pContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &defaultTextColour);
	}
	if (SUCCEEDED(hr)) {
		hr = pWriteFactory->CreateTextFormat(
			L"Tahoma",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			32.0f,
			L"pt-pt",
			&defaultTextFormat
		);
	}
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to create default text format!", 0, 0);
		return hr;
	}
	text->InitText(L"Current frequency: %1f", defaultTextFormat, defaultTextColour);
	return hr;
}

HRESULT GraphicsEngine::Render(void* parms, int num_parms, float parameter) {
	HRESULT hr = S_OK;
	pContext->BeginDraw();
	pContext->SetTransform(D2D1::Matrix3x2F::Identity());
	pContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	// Drawing operations go here
	text->RenderText(pContext, parameter);
	hr = pContext->EndDraw();
	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
		hr = S_OK;
		ReleaseCOM(pBitmap);
		ReleaseCOM(pSurface);
		ReleaseCOM(pBackBuffer);
		hr = pSwap->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		if (FAILED(hr)) {
			MessageBox(0, L"Failure to get Back Buffer!", 0, 0);
			return hr;
		}
		hr = pSwap->GetBuffer(0, IID_PPV_ARGS(&pSurface));
		if (FAILED(hr)) {
			MessageBox(0, L"Failure to get DXGI Back Buffer!", 0, 0);
			return hr;
		}
		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET |
			D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96.0f, 96.0f, NULL);
		hr = pContext->CreateBitmapFromDxgiSurface(pSurface, NULL, &pBitmap);
		if (FAILED(hr)) {
			MessageBox(0, L"Failure to create Bitmap!", 0, 0);
			return hr;
		}
		pContext->SetTarget(pBitmap);
	}
	hr = pSwap->Present(1, 0);
	if (FAILED(hr)) {
		MessageBox(0, L"Failure to Present!", 0, 0);
		return hr;
	}
	return hr;
}

Text::Text() {
	text_content = nullptr;
	text_length = 0;
	text_format = nullptr;
	text_colour = nullptr;
};

void Text::InitText(PCWSTR uri, IDWriteTextFormat* pTextFormat, ID2D1SolidColorBrush* pTextColour) {
	text_content = uri;
	text_length = (UINT32)wcslen(text_content);
	text_format = pTextFormat;
	text_colour = pTextColour;
};

void Text::RenderText(ID2D1DeviceContext3* pContext, float parameter) {
	if (text_content != nullptr && text_format != nullptr && text_colour != nullptr) {
		D2D1_SIZE_F rtSize = pContext->GetSize();
		D2D1_RECT_F layoutRect = D2D1::RectF(
			static_cast<FLOAT>(rc.left + 500),
			static_cast<FLOAT>(rc.top + 500),
			static_cast<FLOAT>(rc.right + 1500),
			static_cast<FLOAT>(rc.bottom + 500)
		);

		static const int bufferLength = 256;
		static wchar_t wsbuffer[bufferLength];

		int length = swprintf_s(wsbuffer, bufferLength, text_content, parameter);

		pContext->DrawText(
			wsbuffer,
			length,
			text_format,
			layoutRect,
			text_colour
		);
	}
};

Text::~Text() {
	text_content = nullptr;
	ReleaseCOM(text_format);
	ReleaseCOM(text_colour);
};	

GraphicsEngine::~GraphicsEngine() {
	ReleaseCOM(pWriteFactory);
	ReleaseCOM(pIWICFactory);
	ReleaseCOM(pDevice);
	ReleaseCOM(pContext);
	ReleaseCOM(pFactory);
	ReleaseCOM(pSwap);
	ReleaseCOM(pD3DContext);
	ReleaseCOM(pD3Device);
}