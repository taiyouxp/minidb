/* test_pager.c
some tests:
    1. Round trip. Write(n, b) and then read(n) == b, for n = 0, 1 and 17 
	2. Between processes. Save, close the program, reopen it and read. Without this, you’ve only tested the memory.
	3. Neighbourhood. Writing to page 3 must not alter a single byte on pages 2 or 4 
	4. Size. After any sequence of operations, the file is a multiple of 4096 
	5. Visually. Run `hexdump -C data.db | head`, and the bytes are where the formula said they would
*/
#include <assert.h>
#include <sys/stat.h>
#include "m1_pager.h"
#include "m1_pager.c"

static page mk(int seed)
{
    page p;
    memset(&p, seed, sizeof p);
    return p;
}

int main(void)
{
    remove("t.db"); // comment if you want to know the otherwise 
    pager pg;
    assert(pager_open(&pg, "t.db") == 0);
    for (int i = 0; i < 18; i++) assert(pager_alloc(&pg) == i);

    // 1. round trip: 0, 1, 17
    int nums[] = {0, 1, 17};
    for (int i = 0; i < 3; i++) {
        page w = mk(nums[i] + 1), r;
        assert(pager_write(&pg, nums[i], &w) == 0);
        assert(pager_read(&pg, nums[i], &r) == 0);
        assert(memcmp(&w, &r, sizeof w) == 0);
    }

    // 3. neighbors: write page 3, pages 2 and 4 must stay the same 
    page a = mk(0xAA), b = mk(0xBB), c = mk(0xCC), r;
    pager_write(&pg, 2, &a);
    pager_write(&pg, 4, &c);
    pager_write(&pg, 3, &b);
    pager_read(&pg, 2, &r); assert(memcmp(&a, &r, sizeof a) == 0);
    pager_read(&pg, 4, &r); assert(memcmp(&c, &r, sizeof c) == 0);

    // 4. size is a multiple of 4096 
    pager_sync(&pg);
    struct stat st;
    stat("t.db", &st);
    assert(st.st_size % PG_SIZE == 0);
    assert(st.st_size == 18 * PG_SIZE);

    // 2. across processes: close, reopen, read 
    pager_close(&pg);
    assert(pager_open(&pg, "t.db") == 0);
    assert(pg.n_pages == 18);
    pager_read(&pg, 3, &r);
    assert(memcmp(&b, &r, sizeof b) == 0);
    pager_close(&pg);

    puts("ok :)"); // it means all asserts passed :)

    // hexdump -C t.db | head 
    // hexdump -C -s 12288 -n 32 t.db -> page 3 filled with 0xBB, 
    // 12288 is the byte offset based on the calc
    // not using it rn because it will be needed for the insert(register) method
    // next i should do is the indexing.
    return 0;
}