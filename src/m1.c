#define _FILE_OFFSET_BITS 64 
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/m1.h"
#include "../include/codec.h"

off_t offset_desloc(int n_pg, int n_slot) { return (off_t)n_pg * PG_SIZE + H_SIZE + (off_t)n_slot * SLOT_SIZE; }

int write_pg(int n_pg, const page *p, FILE *f)
{
    if (!f || !p) 
    { 
        errno = EINVAL; 
        perror("write_pg"); 
        return -1; 
    }

    if (fseeko(f, (off_t)PG_SIZE * n_pg, SEEK_SET) != 0) 
    {
        perror("write_pg: fseek");
        return -1;
    }

    if (fwrite(p, PG_SIZE, 1, f) != 1) 
    {
        perror("write_pg: fwrite");
        return -1;
    }

    return 0;
}

int read_pg(int n_pg, page *p, FILE *f)
{
    if (!f || !p) 
    {
        errno = EINVAL;
        perror("read_pg");
        return -1;
    }

    if (fseeko(f, (off_t)PG_SIZE * n_pg, SEEK_SET) != 0) 
    {
        perror("read_pg: fseek");
        return -1;
    }

    if (fread(p, PG_SIZE, 1, f) != 1) 
    {
        if (feof(f))
            fprintf(stderr, "read_pg: fread hit EOF (file too short?)\n");
        else
            perror("read_pg: fread");
        return -1;
    }
    return 0;
}

void page_init(page *p, int n_pg)
{
    memset(p, 0, sizeof *p);
    p->hdr.reg_size = sizeof(reg);
    p->hdr.n_page   = n_pg;
}

void serialise(const reg *r, uint8_t out[SLOT_SIZE])
{
    put32(out, r->id);
    memcpy(out + 4, r->data, 4);
}

void deserialise(const uint8_t in[SLOT_SIZE], reg *r)
{
    r->id = get32(in);
    memcpy(r->data, in + 4, 4);
}

int insert(int n_pg, page *p, FILE *f, const reg *r)
{
    if (read_pg(n_pg, p, f) != 0) 
        return -1;

    if (p->hdr.reg_size == 0)             // all-zero page: first use
        page_init(p, n_pg);

    if (p->hdr.n_slots >= R_QNT) 
    {
        fprintf(stderr, "insert: page %d is full\n", n_pg);
        return -1;
    }

    int slot = p->hdr.n_slots;
    serialise(r, (uint8_t *)&p->registers[slot]);
    p->hdr.n_slots++;

    if (write_pg(n_pg, p, f) != 0) return -1;
    return slot;
}