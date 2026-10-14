/*
* the intent of this 
* - current integration m1+m2 (doing a first approach to the right workflow for main)
* - doing the exercises of the final slides of m1 and m2 
*/

#include <assert.h>
#include "m2.h"

#define N_PAGES 5
#define PER_PHASE 10

static reg make(uint32_t n_pg, int slot)
{
    reg r = { 
    n_pg * 1000u + (uint32_t)slot,
    {'d','a','t','a'} 
    };
    return r;
}

void hit_or_miss(cache *c, uint32_t n_pg) // i guess they never miss huh
{
    if(find(c, n_pg) != NULL)
        printf("\nhit :)");
    printf("\nmiss :(");
}

int main(void)
{
    remove("mini.db");

    pager pg;
    cache c;
    assert(pager_open(&pg, "mini.db") == 0);
    cache_init(&c, &pg); // 3 frames 
    for (int i = 0; i < N_PAGES; i++) 
        assert(cache_alloc(&c) == i);

    /* 
    1: fill page by page (10 records each).
    Each page is loaded once and then hit 9 times. 
    */
    for (uint n = 0; n < N_PAGES; n++)
        for (int k = 0; k < PER_PHASE; k++) {
            reg r = make(n, k);
            assert(cache_insert(&c, n, &r) == k);
        }

    printf("1. sequential access: hits=%u misses=%u\n", c.hits, c.misses);
    assert(c.misses == 5 && c.hits == 45);
    /* 
    1.2: round robin over 5 pages with only 3 frames.
    LRU always evicts the page we need next,
    so every access misses. 
    */
    c.hits = c.misses = 0;
    for (int round = 0; round < PER_PHASE; round++)
        for (int n = 0; n < N_PAGES; n++) 
        {
            int slot = PER_PHASE + round;
            reg r = make(n, slot);
            assert(cache_insert(&c, n, &r) == slot);
        }
    
    printf("1.2. round robin: hits=%u misses=%u\n", c.hits, c.misses);
    assert(c.misses == 50 && c.hits == 0);

    // flush dirty pages, close everything 
    assert(cache_flush(&c) == 0);
    pager_close(&pg);

    // reopen with a fresh cache and verify every record from disk 
    assert(pager_open(&pg, "mini.db") == 0);
    assert(pg.n_pages == N_PAGES);
    cache_init(&c, &pg);

    for (uint32_t n = 0; n < N_PAGES; n++)
        for (int slot = 0; slot < 2 * PER_PHASE; slot++) 
        {
            reg got;
            assert(cache_get(&c, n, slot, &got) == 0);
            assert(got.id == n * 1000u + (uint32_t)slot);
            assert(memcmp(got.data, "data", 4) == 0);
        }

    reg none;
    assert(cache_get(&c, 0, 20, &none) == -1); // slot 20 was never written 

    cache_flush(&c);
    pager_close(&pg);
    puts("tests 1: ok");

    assert(pager_open(&pg, "mini.db") == 0);
    cache_init(&c, &pg); // 3 frames 
    for (int i = 0; i < N_PAGES; i++) 
        assert(cache_alloc(&c) == i);
 
    /* 
    * 2. the exercise of 29.9. acesses: 1, 2, 3, 1, 4, 2, 5, 1, 2, 3
    * which page is in each frame, in the final of this sequence.
    * 2.1. to mark each acess as hit or miss, we should expect 4 hits, 4 misses
    * so this is like the first sequence: there are 
    * the pages 1, 2 and 3. (here we call it frame) 
    * now we're going to access 
    * -> frame [1], a hit. frame 1 now comes to the place of 3. 
    *  3 stays in the second place. sequence: 2, 3, 1
    *   -> frame [4], but it will be a miss. insert 4 in the place of 2 (LRU).
    *       sequence: 3, 1, 4.
    *       -> frame [2]. a miss since frame 2 was evicted it now comes back in the place of 3
    *           sequence: 1, 4, 2.
    *           -> frame [5] is a miss. it will be inserted in the place of 1.
    *               sequence: 4, 2, 5.
    *               -> frames [1, 2, 3]. so 1 is a miss, it replaces 4. sequence: [2, 5, 1] 
    *               2 is a hit. it its the least used. sequence:  [5, 1, 2].
    *               finally 3 (a miss), which replaces 5. final sequence:  [1, 2, 3].
    * 
    * will put this in practice below:
    */    
    
    reg r; 
    int n = 0;
    while(n != 10) 
    {
        if (n == 4)
        {
            r = make(1, 1);
            assert(cache_insert(&c, n, &r) == 1);
            hit_or_miss(&c, n);
            n++;
            continue;
        } 
        else if (n == 5) 
        {
            r = make(4, 1);
            assert(cache_insert(&c, 4, &r));
            hit_or_miss(&c, n);
            n++;
            continue;
        } 
        else if (n == 6)
        {
            r = make(5, 1);
            assert(cache_insert(&c, 5, &r));
            hit_or_miss(&c, n);
            n++;
            continue;
        } 
        else if (n == 7) 
        {
            for (int i = 0; i < 3; i++) 
            {
                r = make(i, 1);
                assert(cache_insert(&c, i, &r));
                hit_or_miss(&c, i);
            }
            n += 3; // end of this loop
        } 
        r = make(n, 1);
        assert(cache_insert(&c, n, &r) == 1);
        hit_or_miss(&c, n);
    }

    printf("\neach access (hit or miss) after the sequence: hits=%u misses=%u\n", c.hits, c.misses);
 
    /* 
    * 2.2. which pages are in each frame on the final sequence:
    * the expected are [1, 2, 3].
    */
    for(int i=0; i < CACHE_MAX_SIZE; i++) 
        printf("%p\n", (void *)find(&c,  i));

    /*
    * 2.3. page 2 and page 5 were altered.
    * Thus, for page 2, 2 writes in disk. 
    * -> The first on the 2th iteration of insertion.
    * -> The second on the 6th iteration, because it was previouly evicted.
    * -> The third (9th iteration) is not a write, its a cache hit. 
    * While for page 5, only one write on disk (iteration 7) since it was a miss.
    * 
    * 2.4. redoing item 2.1 with 4 frames, and to see how many faults the previously
    * third frame costed.
    */
    cache_flush(&c);
    pager_close(&pg);

    assert(pager_open(&pg, "mini.db") == 0);
    cache_init(&c, &pg); // 3 frames
    for (int i = 0; i < N_PAGES; i++) 
        assert(cache_alloc(&c) == i);


    return 0;
}