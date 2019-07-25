#include <Windows.h>

#include <stdint.h>
#include <assert.h>

#include <mn/Memory.h>


// Windows Windowing Code
typedef struct IWindow* Window;
struct IWindow
{
	HWND handle;
	bool running;
};

LRESULT CALLBACK
window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

inline static Window
window_new(uint32_t width, uint32_t height)
{
	Window self = mn::alloc<IWindow>();

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

	self->running = true;
	ShowWindow(self->handle, SW_SHOW);
	return self;
}

inline static void
window_free(Window self)
{
	free(self);
}

inline static void
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


int main(int argc, char** argv)
{
	Window wnd = window_new(800, 600);
	while (wnd->running)
		window_poll(wnd);
	return 0;
}
