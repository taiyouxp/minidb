#include "m3.h"
#include <stdlib.h>
#include <stdio.h>

static m3_node * node_create(bool is_leaf)
{
	m3_node * n = calloc(1, sizeof(m3_node));
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

m3_rec * m3_search(m3_tree * t, uint32_t key)
{
	m3_node * n = t->root;

	while (!n->is_leaf) {
		int i = 0;
		while (i < n->n_keys && key >= n->keys[i])
			i++;
		n = n->children[i];
	}

	return leaf_search(n, key);
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
	right->children[right->n_keys] = node->children[total + 1];

	node->n_keys = mid;
	*promote_node = right;
}

static bool insert_rec(m3_node * n, uint32_t key, m3_rec rec,
                       uint32_t * promote_key, m3_node ** promote_node)
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

	uint32_t pk;
	m3_node * pn;
	bool split = insert_rec(n->children[i], key, rec, &pk, &pn);
	if (!split)
		return false;

	for (int j = n->n_keys; j > i; --j) {
		n->keys[j] = n->keys[j - 1];
		n->children[j + 1] = n->children[j];
	}
	n->keys[i] = pk;
	n->children[i + 1] = pn;
	n->n_keys++;

	if (n->n_keys > M3_MAX_KEYS) {
		split_internal(n, promote_key, promote_node);
		return true;
	}
	return false;
}

int m3_insert(m3_tree * t, uint32_t key, m3_rec rec)
{
	uint32_t promote_key;
	m3_node * promote_node;

	bool split = insert_rec(t->root, key, rec, &promote_key, &promote_node);

	if (split) {
		m3_node * new_root = node_create(false);
		new_root->keys[0] = promote_key;
		new_root->children[0] = t->root;
		new_root->children[1] = promote_node;
		new_root->n_keys = 1;
		t->root = new_root;
	}

	return 0;
}

void m3_print(m3_tree * t)
{
	m3_node * n = t->root;

	while (!n->is_leaf)
		n = n->children[0];

	while (n) {
		for (int i = 0; i < n->n_keys; ++i)
			printf("(%u -> pg%u,sl%u) ", n->keys[i], n->recs[i].page, n->recs[i].slot);
		n = n->next;
	}
	printf("\n");
}

static void node_destroy(m3_node * n)
{
	if (!n->is_leaf) {
		for (int i = 0; i <= n->n_keys; ++i)
			node_destroy(n->children[i]);
	}
	free(n);
}

void m3_destroy(m3_tree * t)
{
	node_destroy(t->root);
	t->root = NULL;
}