#include <mn/IO.h>
#include <mn/Memory.h>
#include <mn/Buf.h>

#include <Windows.h>

#include <stdint.h>
#include <assert.h>


// Pixel
struct Pixel
{
	uint8_t b, g, r, a;
};

inline static uint8_t
sat_addu8(uint8_t a, uint8_t b)
{
	uint8_t res = a + b;
	res |= -(res < a);
	return res;
}

inline static Pixel
operator+(Pixel a, Pixel b)
{
	return Pixel{
		sat_addu8(a.b, b.b),
		sat_addu8(a.g, b.g),
		sat_addu8(a.r, b.r),
		sat_addu8(a.a, b.a)
	};
}

inline static Pixel
operator+=(Pixel& a, Pixel b)
{
	a.b = sat_addu8(a.b, b.b);
	a.g = sat_addu8(a.g, b.g);
	a.r = sat_addu8(a.r, b.r);
	a.a = sat_addu8(a.a, b.a);
	return a;
}


// Image
struct Image
{
	mn::Buf<Pixel> pixels;
	uint32_t width, height;

	Pixel&
	operator()(size_t i, size_t j)
	{
		return pixels[j * width + i];
	}

	const Pixel&
	operator()(size_t i, size_t j) const
	{
		return pixels[j * width + i];
	}
};

inline static Image
image_new(uint32_t width, uint32_t height)
{
	Image self{};
	self.pixels = mn::buf_with_count<Pixel>(size_t(width) * size_t(height));
	self.width = width;
	self.height = height;
	return self;
}

inline static void
image_free(Image& self)
{
	buf_free(self.pixels);
}

inline static void
image_clear(Image& self)
{
	mn::buf_fill(self.pixels, Pixel{});
}



// Math
struct Vec2i
{
	int x, y;
};

inline static Vec2i
operator+(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x + b.x,
		a.y + b.y
	};
}

inline static Vec2i
operator+(Vec2i a)
{
	return Vec2i{ +a.x, +a.y };
}

inline static Vec2i
operator+=(Vec2i& a, Vec2i b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline static Vec2i
operator-(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x - b.x,
		a.y - b.y
	};
}

inline static Vec2i
operator-(Vec2i a)
{
	return Vec2i{ -a.x, -a.y };
}

inline static Vec2i
operator-=(Vec2i& a, Vec2i b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

inline static Vec2i
operator*(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x * b.x,
		a.y * b.y
	};
}

inline static Vec2i
operator*=(Vec2i& a, Vec2i b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

inline static Vec2i
operator/(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x / b.x,
		a.y / b.y
	};
}

inline static Vec2i
operator/=(Vec2i& a, Vec2i b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}



// Swapchain
struct Swapchain
{
	Image imgs[2];
	uint32_t ix;
};

inline static Swapchain
swapchain_new(uint32_t width, uint32_t height)
{
	Swapchain self{};
	self.imgs[0] = image_new(width, height);
	self.imgs[1] = image_new(width, height);
	return self;
}

inline static void
swapchain_free(Swapchain& self)
{
	image_free(self.imgs[0]);
	image_free(self.imgs[1]);
}

inline static Image
swapchain_back(Swapchain& self)
{
	return self.imgs[self.ix];
}

inline static Image
swapchain_front(Swapchain& self)
{
	return self.imgs[(self.ix + 1) % 2];
}

inline static void
swapchain_swap(Swapchain& self)
{
	self.ix = (self.ix + 1) % 2;
}

inline static void
swapchain_resize(Swapchain& self, uint32_t width, uint32_t height)
{
	image_free(self.imgs[0]);
	image_free(self.imgs[1]);
	self.imgs[0] = image_new(width, height);
	self.imgs[1] = image_new(width, height);
}



// Engine
typedef struct IEngine* Engine;
struct IEngine
{
	Swapchain chain;
};

inline static Engine
engine_new(uint32_t width, uint32_t height)
{
	Engine self = mn::alloc<IEngine>();
	self->chain = swapchain_new(width, height);
	return self;
}

inline static void
engine_free(Engine self)
{
	swapchain_free(self->chain);
	mn::free(self);
}

inline static Image
engine_swap(Engine self)
{
	Image res = swapchain_back(self->chain);
	swapchain_swap(self->chain);
	image_clear(swapchain_back(self->chain));
	return res;
}

inline static void
engine_resize(Engine self, uint32_t width, uint32_t height)
{
	swapchain_resize(self->chain, width, height);
}

inline static void
engine_circle(Engine self, uint32_t x, uint32_t y, uint32_t r)
{

}



// Window
typedef void(*Render)(Engine);

typedef struct IWindow* Window;
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
	case WM_PAINT:
		self->render(self->engine);
		window_blit(self, engine_swap(self->engine));
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

inline static Window
window_new(uint32_t width, uint32_t height, Render render_func)
{
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

inline static void
window_free(Window self)
{
	engine_free(self->engine);
	mn::free(self);
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



// Application
inline static void
app_render(Engine self)
{
	static int mov = 0;

	Image img = swapchain_back(self->chain);
	for (size_t j = 0; j < img.height; ++j)
		for (size_t i = 0; i < img.width; ++i)
			img(i, j) += Pixel{ (i + mov) % 255, (j + mov) % 255, 0, 255 };

	++mov;
}

int main(int argc, char** argv)
{
	Window wnd = window_new(800, 600, app_render);
	while (wnd->running)
		window_poll(wnd);
	window_free(wnd);
	return 0;
}
