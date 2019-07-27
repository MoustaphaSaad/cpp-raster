#pragma once

#include <math.h>

namespace raster
{
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
		return int(sqrt(lensqr(a)));
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
}
