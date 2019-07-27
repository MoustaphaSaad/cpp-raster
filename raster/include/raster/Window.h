#pragma once

namespace raster
{
	typedef struct IEngine* Engine;
	typedef void(*Render)(Engine);
	typedef struct IWindow* Window;

	Window
	window_new(int width, int height, Render render_func);

	void
	window_free(Window self);

	inline static void
	destruct(Window self)
	{
		window_free(self);
	}

	bool
	window_closed(Window self);

	void
	window_poll(Window self);
}
