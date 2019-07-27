#include <mn/IO.h>

#include <fabric/Fabric.h>

#include <chrono>

#include "raster/Window.h"
#include "raster/Engine.h"

using namespace raster;


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

	engine_circle(self, -anim_x, 100, anim_r, Pixel{ 255, 255 });
	engine_circle(self, anim_x, 0, anim_r, Pixel{ 255 });
	engine_circle(self, 0, 0, anim_r, Pixel{ 0, 255 });
	engine_circle(self, 100, anim_y, anim_r, Pixel{ 0, 0, 255 });
	engine_circle(self, 200, -anim_y, anim_r, Pixel{ 0, 255, 255 });

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

int main_loop()
{
	Window wnd = window_new(1280, 720, app_render);
	while (window_closed(wnd) == false)
		window_poll(wnd);
	window_free(wnd);
	return 0;
}

int main(int argc, char** argv)
{
	return fabric::fabric_main_exec(fabric::fabric_new(0), main_loop);
}
