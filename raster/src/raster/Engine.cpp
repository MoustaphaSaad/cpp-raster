#include "raster/Engine.h"
#include "raster/Circle.h"

#include <assert.h>

namespace raster
{
	inline static void
	engine_send(Engine self, Quadnode* node, Shape* shape)
	{
		if(quadnode_leaf(node))
		{
			mn::waitgroup_add(self->wg, 1);
			mn::chan_send(node->shapes, shape);
			return;
		}

		Box box = shape->bbox();
		if (box_intersect(box, node->top_left->box))
			engine_send(self, node->top_left, shape);
		if (box_intersect(box, node->top_right->box))
			engine_send(self, node->top_right, shape);
		if (box_intersect(box, node->bottom_left->box))
			engine_send(self, node->bottom_left, shape);
		if (box_intersect(box, node->bottom_right->box))
			engine_send(self, node->bottom_right, shape);
	}

	//API
	Engine
	engine_new(int width, int height)
	{
		assert(width > 0 && height > 0);

		Engine self = mn::alloc<IEngine>();
		self->width = int(width);
		self->height = int(height);
		self->chain = swapchain_new(width, height);
		self->wg = 0;
		self->f = mn::fabric_new();
		self->tree = quadtree_new(self, width, height, width/4);
		self->shape_arena = mn::allocator_arena_new();
		return self;
	}

	void
	engine_free(Engine self)
	{
		quadtree_free(self->tree);
		swapchain_free(self->chain);
		mn::allocator_free(self->shape_arena);
		mn::fabric_free(self->f);
		mn::free(self);
	}

	Image
	engine_swap(Engine self)
	{
		mn::waitgroup_wait(self->wg);
		self->shape_arena->free_all();

		Image res = swapchain_back(self->chain);
		swapchain_swap(self->chain);
		image_clear(swapchain_back(self->chain));

		return res;
	}

	void
	engine_resize(Engine self, int width, int height)
	{
		assert(width > 0 && height > 0);

		self->width = width;
		self->height = height;
		swapchain_resize(self->chain, width, height);
		quadtree_free(self->tree);
		self->tree = quadtree_new(self, width, height, width/4);
	}

	void
	engine_circle(Engine self, int x, int y, int r, Pixel color)
	{
		x += self->width / 2;
		y += self->height / 2;

		Circle* shape = mn::alloc_construct_from<Circle>(self->shape_arena);
		shape->box.min = Vec2i{ clamp(x - r, 0, self->width), clamp(y - r, 0, self->height) };
		shape->box.max = Vec2i{ clamp(x + r, 0, self->width), clamp(y + r, 0, self->height) };
		shape->color = color;
		shape->center = Vec2i{ x, y };
		shape->r = r;

		engine_send(self, self->tree->root, shape);
	}
}
