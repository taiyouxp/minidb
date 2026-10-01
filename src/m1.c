#include "../include/m1.h"
#include <sys/types.h>

off_t offset(int n_pg, int n_slot)
{
    return (off_t)n_pg * PG_SIZE + H_SIZE + (off_t)n_slot * SLOT_SIZE;
}

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

    if (fread(p, PG_SIZE, 1, f) != 1) {
        if (feof(f))
            fprintf(stderr, "read_pg: fread hit EOF (file too short?)\n");
        else
            perror("read_pg: fread");
        return -1;
    }

    return 0;
}