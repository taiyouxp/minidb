#ifndef M2_H
#define M2_H

#include "m1.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define CACHE_MAX_SIZE 3

typedef struct frame {
	page page;
	bool is_dirty;
} frame;

typedef struct cache {
	FILE * file;
	frame queue[CACHE_MAX_SIZE];
	unsigned int misses;
	unsigned int hits;
	int front;
	int rear;
} cache;

void cache_init(cache * c, FILE * f);

#endif