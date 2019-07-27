#include "raster/Window.h"
#include "raster/Image.h"
#include "raster/Engine.h"

#include <mn/IO.h>

#define NOMINMAX
#include <Windows.h>

#include <assert.h>

namespace raster
{
	struct IWindow
	{
		HWND handle;
		bool running;
		Engine engine;
		Render render;
	};


	inline static void
	window_blit(Window self, Image img)
	{
		HDC dc = GetDC(self->handle);

		BITMAPINFOHEADER bmiHeader{};
		bmiHeader.biSize = sizeof(bmiHeader);
		bmiHeader.biWidth = img.width;
		bmiHeader.biHeight = -img.height;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 32;
		bmiHeader.biCompression = BI_RGB;

		BITMAPINFO info{};
		info.bmiHeader = bmiHeader;

		int res = StretchDIBits(
			dc,
			0,
			0,
			img.width,
			img.height,
			0,
			0,
			img.width,
			img.height,
			img.pixels.ptr,
			&info,
			DIB_RGB_COLORS,
			SRCCOPY
		);
		assert(res != 0 && "StretchDIBits failed");

		ReleaseDC(self->handle, dc);
	}

	LRESULT CALLBACK
	window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Window self = Window(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch(msg)
		{
		case WM_CLOSE:
			self->running = false;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			if(self)
				engine_resize(self->engine, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_PAINT: {
			auto start = std::chrono::high_resolution_clock::now();
			self->render(self->engine);
			Image frame = engine_swap(self->engine);
			auto end = std::chrono::high_resolution_clock::now();
			mn::printfmt("render: {:10}us, ", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

			start = std::chrono::high_resolution_clock::now();
			window_blit(self, frame);
			end = std::chrono::high_resolution_clock::now();
			mn::printfmt("blit: {:10}us\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}


	//API
	Window
	window_new(int width, int height, Render render_func)
	{
		assert(width > 0 && height > 0);

		Window self = mn::alloc<IWindow>();

		self->running = true;
		self->engine = engine_new(width, height);
		self->render = render_func;

		HMODULE instance = GetModuleHandle(NULL);
		HCURSOR cursor = LoadCursor(instance, IDC_ARROW);

		WNDCLASSEX window_class{};
		window_class.cbSize = sizeof(WNDCLASSEX);
		window_class.style = 0;
		window_class.lpfnWndProc = window_proc;
		window_class.hInstance = instance;
		window_class.hIcon = LoadIcon(instance, IDI_APPLICATION);
		window_class.hCursor = cursor;
		window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		window_class.lpszClassName = "raster.window";
		window_class.hIconSm = window_class.hIcon;
	
		ATOM res = RegisterClassEx(&window_class);
		assert(res != 0 && "RegisterClassEx failed");

		self->handle = CreateWindow(
			"raster.window",
			"cpp-raster",
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			NULL,
			NULL,
			instance,
			NULL
		);
		assert(self->handle != NULL && "CreateWindow failed");
		SetWindowLongPtr(self->handle, GWLP_USERDATA, LONG_PTR(self));

		ShowWindow(self->handle, SW_SHOW);
		return self;
	}

	void
	window_free(Window self)
	{
		engine_free(self->engine);
		mn::free(self);
	}

	bool
	window_closed(Window self)
	{
		return self->running == false;
	}

	void
	window_poll(Window self)
	{
		MSG msg{};
		BOOL gotMsg = PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE);
		if (gotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
