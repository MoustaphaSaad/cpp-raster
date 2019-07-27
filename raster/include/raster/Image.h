#pragma once

#include "raster/Pixel.h"

#include <mn/Buf.h>

#include <assert.h>

namespace raster
{
	struct Image
	{
		mn::Buf<Pixel> pixels;
		int width, height;

		Pixel&
		operator()(int i, int j)
		{
			return pixels[j * width + i];
		}

		const Pixel&
		operator()(int i, int j) const
		{
			return pixels[j * width + i];
		}
	};

	inline static Image
	image_new(int width, int height)
	{
		assert(width > 0 && height > 0);

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
	destruct(Image& self)
	{
		image_free(self);
	}

	inline static void
	image_clear(Image& self)
	{
		mn::buf_fill(self.pixels, Pixel{});
	}
}
