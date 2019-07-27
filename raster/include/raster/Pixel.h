#pragma once

#include <stdint.h>

namespace raster
{
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
}
