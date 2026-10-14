/* 
* concluding the minimum to submit m3:
* - find a reg without reading the table
* - search and insertion with division
* - without remotion 
*
* new workflow for main: 100 pages x 510 records, indexed by the B+ tree (m3), stored through the cache (m2).
* only the cache touches the file; the tree only maps id -> (page, slot). 
*/
#include <stdio.h>
#include <stdlib.h>
#include "m2.h"
#include "m3.h"

#define N_PAGES 100
#define CACHE_FRAMES 8 // you decide the cache size here 
#define DB_FILE "mini.db"

// this will be on the main file later through all code as it's here.
// it be testing everything we need when called.
#define OK(x) do { if (!(x)) { fprintf(stderr, "FAILED line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

static reg make_rec(uint32_t id)
{
    reg r = { id, {'d','a','t','a'} };
    return r;
}

/*
* splitted the file in parts to be comparing with and without b+ tree approach
* 
*       ---------- the tree with the slide keys (exercises 1 and 2 from m3) ---------- 
*/
static void demo_slides_ex1_2(void)
{
    const uint32_t keys[] = {
        10, 20, 5, 30, 25, 40,
         15, 35, 45, 50, 55};
    
    const int n = (int)(sizeof keys / sizeof keys[0]);
    m3_tree t;
    m3_init(&t);

    for (int i = 0; i < n; i++) {
        m3_rec fake = { 0, (uint32_t)i }; 
        OK(m3_insert(&t, keys[i], fake) == 0);
        if (i == 6 || i == n - 1) {
            printf("\nafter %d keys (last inserted: %u)\n", i + 1, keys[i]);
            m3_print(&t);
        }
    }
    m3_destroy(&t);
}

/*
*       ---------- helpers for the searches ---------- 
* drops everything in memory, so "pages read" counts real disk reads 
*/
static void cold_cache(cache *c)
{
    pager *pg = c->pg;
    int cap = c->capacity;
    OK(cache_flush(c) == 0);
    cache_destroy(c);
    OK(cache_init(c, pg, cap) == 0);
}

// WHERE id BETWEEN lo AND hi, reading every page (no tree) 
static void scan_between(cache *c, uint32_t lo, uint32_t hi)
{
    cold_cache(c);
    int found = 0;
    for (uint32_t n = 0; n < c->pg->n_pages; n++) {
        page *p = cache_fix(c, n);
        OK(p != NULL);
        for (int s = 0; s < p->hdr.n_slots; s++) {
            reg r;
            deserialise((uint8_t *)&p->registers[s], &r);
            if (r.id >= lo && r.id <= hi)
                found++;
        }
        cache_unfix(c, n, false);
    }
    printf("without tree: %d records, pages read from disk: %u\n", found, c->misses);
}

static void query_equals(cache *c, m3_tree *t, uint32_t id)
{
    printf("\nWHERE id = %u\n", id);
    cold_cache(c);

    m3_rec *loc = m3_search(t, id);// the tree is in RAM: 0 page reads 
    if (!loc) {
        printf("with tree: not found, pages read from disk: %u\n", c->misses);
        return;
    }
    reg r;
    OK(cache_get(c, loc->page, (int)loc->slot, &r) == 0);
    OK(r.id == id);
    printf("with tree: found at page %u slot %u, pages read from disk: %u\n",
           loc->page, loc->slot, c->misses);
}

typedef struct { 
    cache *c; 
    int found; 
    uint32_t first, last; 
} range_ctx; // ctx = context

static void handle_match(uint32_t key, m3_rec rec, void *p)
{
    range_ctx *x = p;
    reg r;
    OK(cache_get(x->c, (uint32_t)rec.page, (int)rec.slot, &r) == 0);
    OK(r.id == key);
    if (x->found == 0) x->first = key;
    x->last = key;
    x->found++;
}

static void query_between(cache *c, m3_tree *t, uint32_t lo, uint32_t hi)
{
    printf("\nWHERE id BETWEEN %u AND %u\n", lo, hi);
    cold_cache(c);

    range_ctx x = { c, 0, 0, 0 };
    m3_range(t, lo, hi, handle_match, &x);
    printf("with tree: %d records (ids %u..%u), pages read from disk: %u\n",
           x.found, x.first, x.last, c->misses);

    scan_between(c, lo, hi);
}

int main(void)
{
    puts("=== part 1: the tree alone (slide keys) ===");
    demo_slides_ex1_2();

    puts("\n=== part 2: 100 pages x 510 records, indexed by the tree ===");
    remove(DB_FILE);

    pager pg;
    cache c;
    m3_tree t;
    OK(pager_open(&pg, DB_FILE) == 0);
    OK(cache_init(&c, &pg, CACHE_FRAMES) == 0);
    m3_init(&t);
    for (int i = 0; i < N_PAGES; i++)
        OK(cache_alloc(&c) == i);

    uint32_t id = 1; // ids 1..51000: page n holds ids n*510+1 .. n*510+510 
    for (uint32_t n = 0; n < N_PAGES; n++)
        for (int s = 0; s < R_QNT; s++, id++) {
            reg r = make_rec(id);
            OK(cache_insert(&c, n, &r) == s); // goes into the page, through the cache
            m3_rec where = { n, (uint32_t)s };
            OK(m3_insert(&t, id, where) == 0); // the tree remembers where it went
        }
    OK(cache_flush(&c) == 0);

    printf("records: %u   pages: %u   tree height: %d (M3_MAX_KEYS=%d)\n",
           id - 1, pg.n_pages, m3_height(&t), M3_MAX_KEYS);
    printf("cache (%d frames): hits=%u misses=%u disk writes=%u\n",
           c.capacity, c.hits, c.misses, c.writes);

    puts("\n=== part 3: searches (cache emptied before each one) ===");
    query_equals(&c, &t, 20);
    query_equals(&c, &t, 30000);
    query_equals(&c, &t, 99999);
    query_between(&c, &t, 20, 30);
    query_between(&c, &t, 500, 520);
    query_between(&c, &t, 1000, 2000);

    m3_destroy(&t);
    cache_destroy(&c);
    pager_close(&pg);
    return 0;
}