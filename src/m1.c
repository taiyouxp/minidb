#include "m1.h"

long offset(page pg, int n_pg, int n_slot)
{
    return (n_pg * PG_SIZE) + sizeof(pg.hdr) + (n_slot * SLOT_SIZE);
}

int write_pg(int n_pg, const page *p, FILE *f)
{
    if (!f || !p) {
        errno = EINVAL;
        perror("write_pg");
        return -1;
    }

    if (fseek(f, (long)PG_SIZE * n_pg, SEEK_SET) != 0) {
        perror("write_pg: fseek");
        return -1;
    }

    if (fwrite(p, PG_SIZE, 1, f) != 1) {
        perror("write_pg: fwrite");
        return -1;
    }

    return 0;
}

int read_pg(int n_pg, page *p, FILE *f)
{
    if (!f || !p) {
        errno = EINVAL;
        perror("read_pg");
        return -1;
    }

    if (fseek(f, (long)PG_SIZE * n_pg, SEEK_SET) != 0) {
        perror("read_pg: fseek");
        return -1;
    }

    if (fread(p, PG_SIZE, 1, f) != 1) {
        if (feof(f))
            fprintf(stderr, "read_pg: fread hit EOF (file too short?)\n");
        else
            perror("read_pg: fread");
        return -1;
    }
}

int next_free_page(FILE *f)
{
	page p0;
	
	if((read_pg(0, &p0, f))== -1)
		return -1;
	
	return p0.hdr.n_page + 1;
}