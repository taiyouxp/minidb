#include "m2.h"

int next_free_page(FILE *f);

int insert(cache * c, const char * data, uint16_t id, uint16_t reg_num,
           uint32_t * cur_pg, int * cur_slot)
{
	if (*cur_slot >= R_QNT || *cur_pg == 0) {
		int n = next_free_page(c->file);
		if (n <= 0)
			n = 1;

		page p0 = {0};
		if (read_pg(0, &p0, c->file) == -1)
			clearerr(c->file);

		p0.hdr.n_page = (uint32_t)n;
		write_pg(0, &p0, c->file);

		*cur_pg = (uint32_t)n;
		*cur_slot = 0;
	}

	int idx = cache_get(c, *cur_pg);
	page * p = &c->queue[idx].page;

	p->registers[*cur_slot].id = id;
	p->registers[*cur_slot].reg_num = reg_num;
	memcpy(p->registers[*cur_slot].data, data, 4);
	p->hdr.n_slots = (uint16_t)(*cur_slot + 1);
	p->hdr.reg_size = sizeof(reg);

	c->queue[idx].is_dirty = true;
	(*cur_slot)++;

	return *cur_slot - 1;
}

int main(void)
{
	FILE *f = fopen("tests.db", "r+b");
	if (!f) f = fopen("tests.db", "w+b");

	cache c;
	cache_init(&c, f);

	uint32_t cur_pg = 0;
	int cur_slot = R_QNT;

	insert(&c, "abcd", 1, 1001, &cur_pg, &cur_slot);
	insert(&c, "efgh", 2, 1002, &cur_pg, &cur_slot);

	for (uint16_t i = 3; i <= R_QNT; ++i)
		insert(&c, "zzzz", i, (uint16_t)(1000 + i), &cur_pg, &cur_slot);

	insert(&c, "mnop", 999, 2001, &cur_pg, &cur_slot);

	printf("hits=%u misses=%u\n", c.hits, c.misses);

	cache_get(&c, 10);
	cache_get(&c, 11);

	page chk = {0};
	read_pg(1, &chk, f);
	printf("pg1 slots=%u reg0=%.4s reg509=%.4s\n",
	       chk.hdr.n_slots, chk.registers[0].data, chk.registers[509].data);

	fclose(f);
	return 0;
}