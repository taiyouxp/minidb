#ifndef HASHTAB_H
#define HASHTAB_H

struct nlist {
    struct nlist *next;
    char *name;
    char *defn;
};
// NULL if not found 
struct nlist *lookup(const char *s);

// NULL on out of memory 
struct nlist *install(const char *name, const char *defn); 

#endif