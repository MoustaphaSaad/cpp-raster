#pragma once

#include "raster/Image.h"

namespace raster
{
	struct Swapchain
	{
		Image imgs[2];
		int ix;
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

	inline static void
	destruct(Swapchain& self)
	{
		swapchain_free(self);
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
}
