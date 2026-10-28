// source: https://stackoverflow.com/a/4384446
#include <stdlib.h>
#include <string.h>
#include "hashtab.h"

#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE];

static unsigned hash(const char *s)
{
    unsigned hashval = 0;
    for (; *s != '\0'; s++)
        hashval = (unsigned char)*s + 31 * hashval;
    return hashval % HASHSIZE;
}

static char *str_dup(const char *s)
{
    char *p = malloc(strlen(s) + 1);
    if (p) strcpy(p, s);
    return p;
}

struct nlist *lookup(const char *s)
{
    for (struct nlist *np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np;
    return NULL;
}

struct nlist *install(const char *name, const char *defn)
{
    struct nlist *np = lookup(name);
    if (np == NULL) {
        np = malloc(sizeof *np);
        if (np == NULL || (np->name = str_dup(name)) == NULL) return NULL;
        unsigned h = hash(name);
        np->next = hashtab[h];
        hashtab[h] = np;
    } else
        free(np->defn);
    if ((np->defn = str_dup(defn)) == NULL) return NULL;
    return np;
}