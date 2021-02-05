#include "raster/Quadtree.h"
#include "raster/Image.h"
#include "raster/Swapchain.h"
#include "raster/Shape.h"
#include "raster/Engine.h"

#include <mn/Fabric.h>
#include <mn/Memory.h>
#include <math.h>

namespace raster
{
	inline static Quadnode*
	quadtree_node_new(Quadtree self)
	{
		Quadnode* res = (Quadnode*)mn::pool_get(self->node_pool);
		::memset(res, 0, sizeof(*res));
		res->engine = self->engine;
		return res;
	}

	inline static void
	quadtree_split(Quadtree self, Quadnode* node)
	{
		if(box_width(node->box) < self->limit && box_height(node->box) < self->limit) {
			++self->node_count;
			node->shapes = mn::chan_new<Shape*>(64);
			mn::go(self->engine->f, [node, shapes = mn::chan_new(node->shapes)]{
				for (auto shape: shapes)
					quadnode_raster(node, shape);
				mn::chan_free(shapes);
			});
			return;
		}
		int w2f = int(::floor(box_width(node->box) / 2.0f));
		int h2f = int(::floor(box_height(node->box) / 2.0f));
		int w2c = int(::ceil(box_width(node->box) / 2.0f));
		int h2c = int(::ceil(box_height(node->box) / 2.0f));

		node->top_left = quadtree_node_new(self);
		node->top_left->box.min = node->box.min;
		node->top_left->box.max = node->top_left->box.min + Vec2i{ w2f, h2f };

		node->top_right = quadtree_node_new(self);
		node->top_right->box.min = node->box.min + Vec2i{ w2f, 0 };
		node->top_right->box.max = node->top_right->box.min + Vec2i{ w2c, h2f };

		node->bottom_left = quadtree_node_new(self);
		node->bottom_left->box.min = node->box.min + Vec2i{ 0, h2f };
		node->bottom_left->box.max = node->bottom_left->box.min + Vec2i{ w2f, h2c };

		node->bottom_right = quadtree_node_new(self);
		node->bottom_right->box.min = node->box.min + Vec2i{ w2f, h2f };
		node->bottom_right->box.max = node->bottom_right->box.min + Vec2i{ w2c, h2c };

		quadtree_split(self, node->top_left);
		quadtree_split(self, node->top_right);
		quadtree_split(self, node->bottom_left);
		quadtree_split(self, node->bottom_right);;
	}

	inline static void
	quadnode_chan_close(Quadnode* node)
	{
		if (node->shapes)
		{
			mn::chan_close(node->shapes);
			mn::chan_free(node->shapes);
		}
		if (node->top_left) quadnode_chan_close(node->top_left);
		if (node->top_right) quadnode_chan_close(node->top_right);
		if (node->bottom_left) quadnode_chan_close(node->bottom_left);
		if (node->bottom_right) quadnode_chan_close(node->bottom_right);
	}


	//API
	void
	quadnode_raster(Quadnode* node, Shape* shape)
	{
		Image img = swapchain_back(node->engine->chain);
		for (int j = node->box.min.y; j < node->box.max.y; ++j)
		{
			for (int i = node->box.min.x; i < node->box.max.x; ++i)
			{
				img(i, j) += shape->sample(Vec2i{ i, j }) + Pixel{ 0, 0, 50, 50 };
			}
		}
		mn::waitgroup_done(node->engine->wg);
	}

	Quadtree
	quadtree_new(Engine engine, uint32_t width, uint32_t height, uint32_t limit)
	{
		Quadtree self = mn::alloc<IQuadtree>();
		self->engine = engine;
		self->node_pool = mn::pool_new(sizeof(Quadnode), 1024);
		self->root = quadtree_node_new(self);
		self->node_count = 0;
		self->root->box.max = Vec2i{int(width), int(height)};
		self->limit = limit;

		quadtree_split(self, self->root);

		return self;
	}

	void
	quadtree_free(Quadtree self)
	{
		quadnode_chan_close(self->root);
		mn::pool_free(self->node_pool);
		mn::free(self);
	}
}
