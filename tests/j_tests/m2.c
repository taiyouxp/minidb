#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define CACHE_MAX_SIZE 50

typedef struct frame
{
	page Page;
	bool is_dirty;
} frame;

typedef struct cache
{
	FILE * file;
	frame queue[CACHE_MAX_SIZE];
	unsigned int misses;
	unsigned int hits;
	int front;
	int rear;
} cache;

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
