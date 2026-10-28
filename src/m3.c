#include "m3.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static m3_node * node_create(bool is_leaf)
{
	m3_node * n = calloc(1, sizeof(m3_node));
	if (!n) { perror("m3: calloc"); exit(1); }
	n->is_leaf = is_leaf;
	return n;
}

void m3_init(m3_tree * t)
{
	t->root = node_create(true);
}

static m3_rec * leaf_search(m3_node * n, uint32_t key)
{
	int i = 0;
	while (i < n->n_keys && n->keys[i] < key)
		i++;
	if (i < n->n_keys && n->keys[i] == key)
		return &n->recs[i];
	return NULL;
}
// walking down to the leaf where 'key' belongs
static m3_node * find_leaf(m3_tree * t, uint32_t key)
{
	m3_node * n = t->root;

	while (!n->is_leaf) {
		int i = 0;
		while (i < n->n_keys && key >= n->keys[i])
			i++;
		n = n->children[i];
	}
	return n;
}

m3_rec * m3_search(m3_tree * t, uint32_t key)
{
	return leaf_search(find_leaf(t, key), key);
}

static void split_leaf(m3_node * leaf, uint32_t * promote_key, m3_node ** promote_node)
{
	m3_node * right = node_create(true);
	int total = leaf->n_keys;
	int mid = total / 2;

	right->n_keys = total - mid;
	for (int i = 0; i < right->n_keys; ++i) {
		right->keys[i] = leaf->keys[mid + i];
		right->recs[i] = leaf->recs[mid + i];
	}

	leaf->n_keys = mid;
	right->next = leaf->next;
	leaf->next = right;

	*promote_key = right->keys[0];
	*promote_node = right;
}

static void split_internal(m3_node * node, uint32_t * promote_key, m3_node ** promote_node)
{
	m3_node * right = node_create(false);
	int total = node->n_keys;
	int mid = total / 2;

	*promote_key = node->keys[mid];
	right->n_keys = total - mid - 1;

	for (int i = 0; i < right->n_keys; ++i) {
		right->keys[i] = node->keys[mid + 1 + i];
		right->children[i] = node->children[mid + 1 + i];
	}
	/*
	* fix: a node with 'total' keys	has children 0..total, so the last one is.. [total]...
	* (it was [total + 1], which surpass the end of array)
	*/
	right->children[right->n_keys] = node->children[total];

	node->n_keys = mid;
	*promote_node = right;
}
/*
* returns true if 'n' split. then promote_key / promote_node will 
* be added to the parent.
* this ensures that 'key' is not in the tree yet.
*/
static bool insert_rec(
	m3_node * n,
	uint32_t key, 
	m3_rec rec,
    uint32_t * promote_key, 
	m3_node ** promote_node)
{
	if (n->is_leaf) {
		int i = 0;
		while (i < n->n_keys && n->keys[i] < key)
			i++;
		if (i < n->n_keys && n->keys[i] == key) {
			n->recs[i] = rec;
			return false;
		}

		for (int j = n->n_keys; j > i; --j) {
			n->keys[j] = n->keys[j - 1];
			n->recs[j] = n->recs[j - 1];
		}
		n->keys[i] = key;
		n->recs[i] = rec;
		n->n_keys++;

		if (n->n_keys > M3_MAX_KEYS) {
			split_leaf(n, promote_key, promote_node);
			return true;
		}
		return false;
	}

	int i = 0;
	while (i < n->n_keys && key >= n->keys[i])
		i++;

	uint32_t child_key;
	m3_node * child_new;
	if (!insert_rec(n->children[i], key, rec, &child_key, &child_new))
		return false;
 
	// child split: add the new key at i and the new child at i + 1 
	for (int j = n->n_keys; j > i; --j) {
		n->keys[j] = n->keys[j - 1];
		n->children[j + 1] = n->children[j];
	}
	n->keys[i] = child_key;
	n->children[i + 1] = child_new;
	n->n_keys++;
 
	if (n->n_keys <= M3_MAX_KEYS)
		return false;
	split_internal(n, promote_key, promote_node);
	return true;	
}

int m3_insert(m3_tree * t, uint32_t key, m3_rec rec)
{
	if(m3_search(t, key))
		return -1;

	uint32_t pk;
	m3_node * pn;

	if (insert_rec(t->root, key, rec, &pk, &pn)) {
		// tree split: tree grows one level
		m3_node * new_root = node_create(false);
		new_root->keys[0] = pk;
		new_root->children[0] = t->root;
		new_root->children[1] = pn;
		new_root->n_keys = 1;
		t->root = new_root;
	}
	return 0;
}

void m3_range(m3_tree * t, 
	uint32_t lo, 
	uint32_t hi,
    void (*fn)(uint32_t key, m3_rec rec, void * ctx), 
	void * ctx)
{
	for (m3_node * n = find_leaf(t, lo); n; n = n->next)
		for (int i = 0; i < n->n_keys; ++i) {
			if (n->keys[i] > hi)
				return;
			if (n->keys[i] >= lo)
				fn(n->keys[i], n->recs[i], ctx);
		}
}

int m3_height(const m3_tree * t)
{
	int h = 1;
	for (const m3_node * n = t->root; !n->is_leaf; n = n->children[0])
		h++;
	return h;
}
// helper for m3_print
static void print_node(const m3_node * n, int depth)
{
	printf("%*s%s [", depth * 4, "", n->is_leaf ? "leaf" : "node");
	for (int i = 0; i < n->n_keys; ++i)
		printf("%s%u", i ? " " : "", n->keys[i]);
	puts("]");
	if (!n->is_leaf)
		for (int i = 0; i <= n->n_keys; ++i)
			print_node(n->children[i], depth + 1);
}

void m3_print(const m3_tree * t)
{
	printf("height = %d\n", m3_height(t));
	print_node(t->root, 0);
 
	const m3_node * n = t->root;
	while (!n->is_leaf)
		n = n->children[0];
	printf("leaf chain: ");
	for (; n; n = n->next) {
		printf("[");
		for (int i = 0; i < n->n_keys; ++i)
			printf("%s%u", i ? " " : "", n->keys[i]);
		printf("]%s", n->next ? " -> " : "");
	}
	putchar('\n');
}
// helper for m3_destroy
static void destroy_node(m3_node * n)
{
	if (!n->is_leaf) {
		for (int i = 0; i <= n->n_keys; ++i)
			destroy_node(n->children[i]); // its called recursion.
	}
	free(n);
}

void m3_destroy(m3_tree * t)
{
	destroy_node(t->root);
	t->root = NULL;
}