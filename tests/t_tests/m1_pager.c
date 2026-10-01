// need to write a class "Pager" to attend the adequate 
// implementation of m1 following serialization of pages
// "an object that stores the open file and the pages number"


#define _FILE_OFFSET_BITS 64 // macro for off_t (type used for file positions and sizes)
#include <stdint.h>
#include <stdio.h>
#include "m1_pager.h"
#include "../../src/m1.c"
// using underlines as if it were methods
static int validate(const pager *pg, uint32_t n)
{
    if(!pg || !pg->f || n >= pg->n_pages) 
    {
        errno = EINVAL;
        perror("pager: invalid page");
        return -1;
    } return 0; }

int pager_open(pager *pg, const char *path)
{
    if (!pg || !path) { errno = EINVAL; return -1; }
    
    pg->f = fopen(path, "r+b");
    if (!pg->f)
    {
        if (errno != ENOENT) { perror("pager_open"); return -1; }
        pg->f = fopen(path, "w+b");
        if (!pg->f) { perror("pager_open: create"); return -1; }
    } 
    if (fseeko(pg->f, 0, SEEK_END) != 0)
    {
        perror("pager_open: fseeko");
        fclose(pg->f); pg->f = NULL;
        return -1;
    } 
    
    off_t size = ftello(pg->f);
    if(size < 0)
    {
        perror("pager_open: ftello");
        fclose(pg->f); pg->f = NULL;
        return -1; 
    }
    pg->n_pages = (uint32_t)(size / PG_SIZE);
    return 0;
}

int pager_read(pager *pg, uint32_t n, page *out)
{
    if(validate(pg, n) != 0 || !out) return -1;
    return read_pg((int)n, out, pg->f);
}

int pager_write(pager *pg, uint32_t n, const page *in)
{
    if(validate(pg, n) != 0 || !in) return -1;
    return write_pg((int)n, in, pg->f);
}

long pager_alloc(pager *pg)
{
    static const page zero; 
    if (!pg || !pg->f) { errno = EINVAL; return -1; }

    uint32_t n = pg->n_pages;
    if (write_pg((int)n, &zero, pg->f) != 0) return -1;

    pg->n_pages++;
    return (long)n;
}

int pager_sync(pager *pg)
{
    if(!pg || !pg->f) { errno = EINVAL; return -1; }
    if(fflush(pg->f) != 0) { perror("pager_sync: fflush"); return -1; }
    if(fsync(fileno(pg->f)) != 0) { perror("pager_sync: fsync"); return -1; }
    
    return 0;
}

void pager_close(pager *pg)
{
    if(pg && pg->f)
    {
        fclose(pg->f);
        pg->f = NULL;
    }
}
