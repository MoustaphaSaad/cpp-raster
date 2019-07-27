#pragma once

#include "raster/Pixel.h"
#include "raster/Swapchain.h"
#include "raster/Quadtree.h"

#include <fabric/Fabric.h>
#include <fabric/Waitgroup.h>

#include <mn/Memory.h>

namespace raster
{
	typedef struct IEngine* Engine;
	struct IEngine
	{
		int width;
		int height;
		Swapchain chain;
		fabric::Waitgroup wg;
		Quadtree tree;
		mn::memory::Arena* shape_arena;
	};

	Engine
	engine_new(int width, int height);

	void
	engine_free(Engine self);
	
	inline static void
	destruct(Engine self)
	{
		engine_free(self);
	}

	Image
	engine_swap(Engine self);

	void
	engine_resize(Engine self, int width, int height);

	void
	engine_circle(Engine self, int x, int y, int r, Pixel color);
}
