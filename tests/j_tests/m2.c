#include "m2.h"

void cache_init(cache * c, FILE * f)
{
	c->file = f;
	c->misses = 0;
	c->hits = 0;
	c->front = 0;
	c->rear = 0;

	for (int i = 0; i < CACHE_MAX_SIZE; ++i) {
		c->queue[i].page.hdr.n_page = (uint32_t)-1;
		c->queue[i].is_dirty = false;
	}
}

static int cache_find(cache * c, uint32_t n_pg)
{
	for (int i = 0; i < CACHE_MAX_SIZE; ++i) {
		if (c->queue[i].page.hdr.n_page == n_pg)
			return i;
	}
	return -1;
}

static void cache_evict(cache * c)
{
	frame * victim = &c->queue[c->front];

	if (victim->is_dirty)
		write_pg((int)victim->page.hdr.n_page, &victim->page, c->file);

	victim->page.hdr.n_page = (uint32_t)-1;
	victim->is_dirty = false;
	c->front = (c->front + 1) % CACHE_MAX_SIZE;
}

static void cache_insert(cache * c, const page * p)
{
	if (c->queue[c->rear].page.hdr.n_page != (uint32_t)-1)
		cache_evict(c);

	c->queue[c->rear].page = *p;
	c->queue[c->rear].is_dirty = false;
	c->rear = (c->rear + 1) % CACHE_MAX_SIZE;
}

int cache_get(cache * c, uint32_t n_pg)
{
	int idx = cache_find(c, n_pg);

	if (idx >= 0) {
		c->hits++;
		return idx;
	}

	c->misses++;

	page tmp = {0};
	if (read_pg((int)n_pg, &tmp, c->file) != 0) {
		clearerr(c->file);
		tmp.hdr.n_page = n_pg;
	}

	cache_insert(c, &tmp);
	return cache_find(c, n_pg);
}