// m2 aka 'page cache'
#ifndef M2_H
#define M2_H
#include <stdbool.h>
#include <stdint.h>

#include "m1.h"
#include "pager.h"

// #define CACHE_MAX_SIZE 3 (its a dynamic memory manipulation transition !) 

typedef struct frame // same size as a page 
{
    page pg;
    uint32_t page_num; // which disk page this frame holds
    bool in_use; // false = empty frame 
    bool is_dirty; // changed in memory, not yet on disk 
    int32_t pins; // >0 = someone is using it, can't be evicted (from eviction concept)  
    uint64_t last_used; // higher = more recent 
} frame;

typedef struct cache // buffer pool: frames + rules of 'enter and exit' 
{
    pager *pg;
    frame *frames;
    uint32_t capacity; // allocated in cache init 
    uint64_t tick; // increases on every fix 
    uint32_t hits, misses, writes; // writes = pages written in disk
} cache;

int cache_init(cache *c, pager *pg, int capacity);
page *cache_fix(cache *c, uint32_t n); 
void cache_unfix(cache *c, uint32_t n, bool dirtied);
long cache_alloc(cache *c); 
int cache_flush(cache *c);
int cache_insert(cache *c, uint32_t n_pg, const reg *r); // slot used, or -1 
int cache_get(cache *c, uint32_t n_pg, int slot, reg *out);
void cache_destroy(cache *c);
bool cache_has(const cache *c, uint32_t n); // is page n in memory? 
void cache_dump(const cache *c);

#endif