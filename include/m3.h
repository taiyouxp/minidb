// m3 aka 'b+ tree'
#ifndef M3_H
#define M3_H

#include <stdint.h>
#include <stdbool.h>

#define M3_MAX_KEYS 3

typedef struct m3_rec {
	uint32_t page;
	uint32_t slot;
} m3_rec;

typedef struct m3_node {
	bool is_leaf;
	int n_keys;
	uint32_t keys[M3_MAX_KEYS + 1];
	struct m3_node * children[M3_MAX_KEYS + 2];
	m3_rec recs[M3_MAX_KEYS + 1];
	struct m3_node * next;
} m3_node;

typedef struct m3_tree {
	m3_node * root;
} m3_tree;

void m3_init(m3_tree * t);
m3_rec * m3_search(m3_tree * t, uint32_t key);
int m3_insert(m3_tree * t, uint32_t key, m3_rec rec);
void m3_range(
	m3_tree * t, 
	uint32_t lo, 
	uint32_t hi,
    void (*fn)(uint32_t key, m3_rec rec, void * ctx), 
	void * ctx
);
int m3_height(const m3_tree * t);
void m3_print(const m3_tree * t);
void m3_destroy(m3_tree * t);

#endif