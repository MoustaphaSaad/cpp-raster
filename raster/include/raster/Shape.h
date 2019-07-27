#pragma once

#include "raster/Box.h"
#include "raster/Pixel.h"
#include "raster/Vec2i.h"

namespace raster
{
	struct Shape
	{
		virtual Box bbox() = 0;
		virtual int distance(Vec2i pixel) = 0;
		virtual Pixel sample(Vec2i pixel) = 0;
	};
}
