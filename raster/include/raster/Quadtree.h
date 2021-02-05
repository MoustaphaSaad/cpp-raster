#pragma once

#include "raster/Box.h"
#include "raster/Shape.h"

#include <mn/Pool.h>
#include <mn/Fabric.h>

#include <atomic>

namespace raster
{
	typedef struct IEngine* Engine;
	struct Quadnode
	{
		Box box;
		Quadnode* top_left;
		Quadnode* top_right;
		Quadnode* bottom_left;
		Quadnode* bottom_right;

		Engine engine;
		mn::Chan<Shape*> shapes;
	};

	inline static bool
	quadnode_leaf(Quadnode* self)
	{
		return self->top_left == nullptr;
	}

	void
	quadnode_raster(Quadnode* node, Shape* shape);

	typedef struct IQuadtree* Quadtree;
	struct IQuadtree
	{
		Engine engine;
		mn::Pool node_pool;
		Quadnode* root;
		size_t node_count;
		int limit;
	};

	Quadtree
	quadtree_new(Engine engine, uint32_t width, uint32_t height, uint32_t limit);

	void
	quadtree_free(Quadtree self);

	inline static void
	destruct(Quadtree self)
	{
		quadtree_free(self);
	}
}
