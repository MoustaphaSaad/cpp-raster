#pragma once

#include "raster/Shape.h"

namespace raster
{
	struct Circle final: Shape
	{
		Box box;
		Pixel color;
		Vec2i center;
		int r;

		Box bbox() override
		{
			return box;
		}

		int distance(Vec2i p) override
		{
			return lensqr(p -center) - r * r;
		}

		Pixel sample(Vec2i p) override
		{
			if (distance(p) < 0)
				return color;
			return Pixel{};
		}
	};
}