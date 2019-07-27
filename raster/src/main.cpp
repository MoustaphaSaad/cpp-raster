#include <mn/IO.h>
#include <mn/Memory.h>
#include <mn/Buf.h>
#include <mn/Pool.h>

#define NOMINMAX
#include <Windows.h>

#include <stdint.h>
#include <assert.h>
#include <chrono>

// clamp
inline static int
min(int a, int b)
{
	return b < a ? b : a;
}

inline static int
max(int a, int b)
{
	return a > b ? a : b;
}

inline static int
clamp(int x, int minimum, int maximum)
{
	return max(minimum, min(x, maximum));
}

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



// Vec2i
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

inline static int
lensqr(Vec2i a)
{
	return a.x * a.x + a.y * a.y;
}

inline static int
len(Vec2i a)
{
	sqrt(lensqr(a));
}

inline static Vec2i
min(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y
	};
}

inline static Vec2i
max(Vec2i a, Vec2i b)
{
	return Vec2i{
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y
	};
}


// AABB Box
struct Box
{
	Vec2i min, max;
};

inline static int
box_width(const Box& self)
{
	return self.max.x - self.min.x;
}

inline static int
box_height(const Box& self)
{
	return self.max.y - self.min.y;
}

inline static bool
box_smaller(const Box &a, const Box& b)
{
	return lensqr(a.max - a.min) < lensqr(b.max - b.min);
}

inline static bool
box_intersect(const Box& a, const Box& b)
{
	bool x_overlap = (
		(a.min.x >= b.min.x && a.min.x < b.min.x + box_width(b)) ||
		(b.min.x >= a.min.x && b.min.x < a.min.x + box_width(a))
	);

	bool y_overlap = (
		(a.min.y >= b.min.y && a.min.y < b.min.y + box_height(b)) ||
		(b.min.y >= a.min.y && b.min.y < a.min.y + box_height(a))
	);

	return x_overlap && y_overlap;
}


//Shapes
struct Shape
{
	virtual Box bbox() = 0;
	virtual int distance(Vec2i pixel) = 0;
	virtual Pixel sample(Vec2i pixel) = 0;
};

struct Circle final: Shape
{
	Box box;
	Pixel color;
	Vec2i center;
	int r;

	Box bbox() override {
		return box;
	}

	int distance(Vec2i p) override {
		return lensqr(p -center) - r * r;
	}

	Pixel sample(Vec2i p) {
		if (distance(p) < 0)
			return color;
		return Pixel{};
	}
};


// Quadtree
struct Quadnode
{
	Box box;
	Quadnode* top_left;
	Quadnode* top_right;
	Quadnode* bottom_left;
	Quadnode* bottom_right;
};

struct Quadtree
{
	mn::Pool node_pool;
	Quadnode* root;
	uint32_t limit;
};

inline static Quadnode*
quadtree_node_new(Quadtree& self)
{
	Quadnode* res = (Quadnode*)mn::pool_get(self.node_pool);
	*res = Quadnode{};
	return res;
}

inline static void
quadtree_split(Quadtree& self, Quadnode* node)
{
	if(box_width(node->box) < self.limit && box_height(node->box) < self.limit) {
		return;
	}
	int w2 = box_width(node->box) / 2;
	int h2 = box_height(node->box) / 2;

	node->top_left = quadtree_node_new(self);
	node->top_left->box.min = node->box.min;
	node->top_left->box.max = node->top_left->box.min + Vec2i{ w2, h2 };

	node->top_right = quadtree_node_new(self);
	node->top_right->box.min = node->box.min + Vec2i{ w2, 0 };
	node->top_right->box.max = node->top_right->box.min + Vec2i{ w2, h2 };

	node->bottom_left = quadtree_node_new(self);
	node->bottom_left->box.min = node->box.min + Vec2i{ 0, h2 };
	node->bottom_left->box.max = node->bottom_left->box.min + Vec2i{ w2, h2 };

	node->bottom_right = quadtree_node_new(self);
	node->bottom_right->box.min = node->box.min + Vec2i{ w2, h2 };
	node->bottom_right->box.max = node->bottom_right->box.min + Vec2i{ w2, h2 };

	quadtree_split(self, node->top_left);
	quadtree_split(self, node->top_right);
	quadtree_split(self, node->bottom_left);
	quadtree_split(self, node->bottom_right);
}

inline static Quadtree
quadtree_new(uint32_t width, uint32_t height, uint32_t limit)
{
	Quadtree self{};
	self.node_pool = mn::pool_new(sizeof(Quadnode), 1024);
	self.root = quadtree_node_new(self);
	self.root->box.max = Vec2i{int(width), int(height)};
	self.limit = limit;
	quadtree_split(self, self.root);
	return self;
}

inline static void
quadtree_free(Quadtree& self)
{
	mn::pool_free(self.node_pool);
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
	int width;
	int height;
	Swapchain chain;
	Quadtree tree;
};

inline static Engine
engine_new(uint32_t width, uint32_t height)
{
	Engine self = mn::alloc<IEngine>();
	self->width = int(width);
	self->height = int(height);
	self->chain = swapchain_new(width, height);
	self->tree = quadtree_new(width, height, width / 10);
	return self;
}

inline static void
engine_free(Engine self)
{
	swapchain_free(self->chain);
	quadtree_free(self->tree);
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
	self->width = width;
	self->height = height;
	swapchain_resize(self->chain, width, height);
	quadtree_free(self->tree);
	self->tree = quadtree_new(width, height, width / 2);
}

inline static void
engine_circle(Engine self, int x, int y, int r, Pixel color)
{
	x += self->width / 2;
	y += self->height / 2;

	Circle shape{};
	shape.box.min = Vec2i{ clamp(x - r, 0, self->width), clamp(y - r, 0, self->height) };
	shape.box.max = Vec2i{ clamp(x + r, 0, self->width), clamp(y + r, 0, self->height) };
	shape.color = color;
	shape.center = Vec2i{ x, y };
	shape.r = r;

	Box b = shape.bbox();

	Image img = swapchain_back(self->chain);

	for (int j = b.min.y; j < b.max.y; ++j)
		for (int i = b.min.x; i < b.max.x; ++i)
			img(i, j) += shape.sample(Vec2i{ i, j });
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
	case WM_PAINT: {
		auto start = std::chrono::high_resolution_clock::now();
		self->render(self->engine);
		auto end = std::chrono::high_resolution_clock::now();
		mn::printfmt("render: {:10}us, ", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

		start = std::chrono::high_resolution_clock::now();
		window_blit(self, engine_swap(self->engine));
		end = std::chrono::high_resolution_clock::now();
		mn::printfmt("blit: {:10}us\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		break;
	}
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
	static int anim_r = 50;
	static int anim_rf = 1;

	static int anim_x = -self->width/2;
	static int anim_xf = 1;

	static int anim_y = -self->height/2;
	static int anim_yf = 1;

	engine_circle(self, -anim_x, 400, anim_r, Pixel{ 255, 255 });
	engine_circle(self, -anim_x, 300, anim_r, Pixel{ 255, 255 });
	engine_circle(self, -anim_x, 200, anim_r, Pixel{ 255, 255 });
	engine_circle(self, anim_x,  100, anim_r, Pixel{ 255 });
	engine_circle(self, 0, 0, anim_r, Pixel{ 0, 255 });
	engine_circle(self, 100, anim_y, anim_r, Pixel{ 0, 0, 255 });
	engine_circle(self, 200, -anim_y, anim_r, Pixel{ 0, 255, 255 });
	engine_circle(self, 300, -anim_y, anim_r, Pixel{ 0, 255, 255 });
	engine_circle(self, 400, -anim_y, anim_r, Pixel{ 0, 255, 255 });

	anim_r += anim_rf;
	anim_x += anim_xf;
	anim_y += anim_yf;

	if (anim_r > (self->width/4) || anim_r < 50)
		anim_rf *= -1;

	if (anim_x > self->width / 2 || anim_x < -self->width / 2)
		anim_xf *= -1;

	if (anim_y > self->height/ 2 || anim_y < -self->height / 2)
		anim_yf *= -1;
}

int main(int argc, char** argv)
{
	Window wnd = window_new(800, 600, app_render);
	while (wnd->running)
		window_poll(wnd);
	window_free(wnd);
	return 0;
}
