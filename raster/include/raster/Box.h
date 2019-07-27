#pragma once

#include "raster/Vec2i.h"

namespace raster
{
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
			(a.min.x >= b.min.x && a.min.x < (b.min.x + box_width(b))) ||
			(b.min.x >= a.min.x && b.min.x < (a.min.x + box_width(a)))
		);

		bool y_overlap = (
			(a.min.y >= b.min.y && a.min.y < (b.min.y + box_height(b))) ||
			(b.min.y >= a.min.y && b.min.y < (a.min.y + box_height(a)))
		);

		return x_overlap && y_overlap;
	}
}
