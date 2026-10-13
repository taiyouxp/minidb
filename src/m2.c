// m2 aka 'page cache'
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  "../include/m2.h"

int cache_init(cache *c, pager *pg, int capacity)
{
    if (!c || !pg || capacity < 1) {
        errno = EINVAL;
        perror("cache_init");
        return -1;
    }

    memset(c, 0, sizeof *c);
    c->frames = calloc((size_t)capacity, sizeof(frame));
    // zeroed: empty, clean, unpinned 
    
    if (!c->frames) { perror("cache_init: calloc"); return -1; }
    c->pg = pg;
    c->capacity = capacity;
    return 0;
}

void cache_destroy(cache *c)
{
    if(!c) return;
    free(c->frames);
    memset(c, 0, sizeof *c);
}

// which frame holds page n? NULL if none 
static frame *find(const cache *c, uint32_t n)
{
    for (int i = 0; i < c->capacity; i++)
        if (c->frames[i].in_use && c->frames[i].page_num == n)
            return &c->frames[i];
    return NULL;
}

bool cache_has(const cache *c, uint32_t n) { return find(c, n) != NULL; }

/* 
* an empty frame, or evicts the least recently used unpinned page
* (writing it first if dirty) 
*/
static frame *free_frame(cache *c)
{
    frame *victim = NULL;

    for (int i = 0; i < c->capacity; i++) 
    {
        frame *f = &c->frames[i];
        if (!f->in_use)
            return f; 
        if (f->pins == 0 && (!victim || f->last_used < victim->last_used))
            victim = f;
    }

    if (!victim) 
    {
        fprintf(stderr, "cache: every page is pinned\n");
        return NULL;
    }

    if (victim->is_dirty) 
    {
        if (pager_write(c->pg, victim->page_num, &victim->pg) != 0)
            return NULL; // keep it dirty but don't lose data 
        victim->is_dirty = false;
    }
    victim->in_use = false;
    return victim;
}

page *cache_fix(cache *c, uint32_t n)
{
    frame *f = find(c, n);

    if (f) c->hits++; // its a cache hit :)  
    else 
    {
        c->misses++; // its a cache miss :(
        f = free_frame(c);
        if (!f) return NULL;
        if (pager_read(c->pg, n, &f->pg) != 0) return NULL;
        f->page_num = n;
        f->in_use = true;
        f->is_dirty = false;
        f->pins = 0;
    }

    f->pins++;
    f->last_used = ++c->tick;
    return &f->pg; // the same memory the cache holds, not a copy 
}

void cache_unfix(cache *c, uint32_t n, bool dirtied)
{
    frame *f = find(c, n);
    if (!f || f->pins == 0) 
    {
        fprintf(stderr, "cache_unfix: page %u is not pinned\n", n);
        return;
    }
    if (dirtied) f->is_dirty = true;
    f->pins--;
}
// page goes to disk as zeros, cache loads it on first fix 
long cache_alloc(cache *c) { return pager_alloc(c->pg); }  

int cache_flush(cache *c)
{
    for (int i = 0; i < c->capacity; i++) 
    {
        frame *f = &c->frames[i];
        if (f->in_use && f->is_dirty) 
        {
            if (pager_write(c->pg, f->page_num, &f->pg) != 0) return -1;
            f->is_dirty = false;
        }
    }
    return pager_sync(c->pg);
}

int cache_insert(cache *c, uint32_t n_pg, const reg *r)
{
    page *p = cache_fix(c, n_pg); // pin 
    if (!p) { perror("cache_insert: cache_fix"); return -1; }

    if (p->hdr.reg_size == 0) // all 'zeros' page: first use 
        page_init(p, n_pg);

    if (p->hdr.n_slots >= R_QNT) 
    {
        fprintf(stderr, "cache_insert: page %u is full\n", n_pg);
        cache_unfix(c, n_pg, false);
        return -1;
    }

    int slot = p->hdr.n_slots;
    serialise(r, (uint8_t *)&p->registers[slot]);
    p->hdr.n_slots++;

    cache_unfix(c, n_pg, true); // unpin + mark as dirty
    return slot;
}

int cache_get(cache *c, uint32_t n_pg, int slot, reg *out)
{
    page *p = cache_fix(c, n_pg);
    if (!p) return -1;

    if (slot < 0 || slot >= p->hdr.n_slots) 
    {
        cache_unfix(c, n_pg, false);
        return -1;
    }
    deserialise((uint8_t *)&p->registers[slot], out);
    cache_unfix(c, n_pg, false); // only read: not dirty
    return 0;
}

// a useful helper 
void cache_dump(const cache *c)
{
    for (int i = 0; i < c->capacity; i++) 
    {
        const frame *f = &c->frames[i];
        if(f->in_use)
            printf("frame %d: page %u  pins=%d dirty=%d last_used=%lu\n",
            i, f->page_num, f->pins, f->is_dirty, f->last_used);
        else
            printf("frame %d: empty\n", i);
    }
}