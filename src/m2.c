#include "../include/m2.h"

void cache_init(cache * c, FILE * f)
{
	c->file = f;
	c->misses = 0;
	c->hits = 0;
	c->front = 0;
	c->rear = 0;
	
	for (int i = 0; i < CACHE_MAX_SIZE; ++i)
	{
		c->queue[i].Page.hdr.n_page = (uint32_t)-1;
		c->queue[i].is_dirty = false;
	}
}
