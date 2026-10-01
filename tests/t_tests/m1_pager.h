#ifndef PAGER_H
#define PAGER_H

#include "../../include/m1.h"

typedef struct pager {
    FILE *f;
    uint32_t n_pages;
} pager;
// pager *pg acts as "self"
int pager_open(pager *pg, const char *path);
int pager_read(pager *pg, uint32_t n, page *out);
int pager_write(pager *pg, uint32_t n, const page *in);
long pager_alloc(pager *pg); // return new page number, else -1 
int pager_sync(pager *pg);
void pager_close(pager *pg);

#endif