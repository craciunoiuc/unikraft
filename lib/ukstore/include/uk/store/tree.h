/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Craciunoiu Cezar <cezar.craciunoiu@gmail.com>
 *
 * Copyright (c) 2021, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __STORE_TREE_H__
#define __STORE_TREE_H__

#include <uk/arch/atomic.h>
#include <uk/arch/lcpu.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

// TODO tree iteration (inorder/preorder/postorder)
// TODO shrink to fit function
// TODO for_each_child
// TODO for_each_node (inorder -> node list -> iterate on it -> free with define)
// TODO is_leaf

struct uk_tree_node {
	struct uk_tree_node *prev;
	struct uk_tree_node **next;
	uint16_t next_nodes_nr;
	uint16_t next_nodes_free;
};

#define uk_tree_entry(ptr, type, field) __containerof(ptr, type, field)

#define UK_TREE_ROOT_INIT(name) { &(name), &(name) }

#define UK_TREE_ROOT(name)	\
	struct uk_tree_node name = UK_TREE_ROOT_INIT(name)

static uint16_t __default_nodes_nr = 2;

/* The default size of children a new node will have */
#define UK_TREE_SET_DEFAULT_SIZE(size)	\
	__default_nodes_nr = size

static inline void
UK_INIT_TREE_NODE(struct uk_tree_node *node)
{
	node->prev = NULL;
	node->next_nodes_nr = __default_nodes_nr;
	node->next = (struct uk_tree_node **)
			calloc(node->next_nodes_nr, sizeof(*node->next));
	if (likely(node->next)) {
		UK_WRITE_ONCE(node->next_nodes_free, __default_nodes_nr);
	}
}

/**
 * Frees the node. The next nodes also get deleted (no sense to keep the pointer
 * to a processes' max_memory if it doesn't exist anymore)
 *
 */
static inline void
__uk_tree_del(struct uk_tree_node *tree_node)
{
	// Remove all children tables
	for (uint16_t i = 0; i < tree_node->next_nodes_nr; ++i) {
		if (!tree_node->next[i]) {
			__uk_tree_del(tree_node->next[i]);
		}
	}

	// Remove table
	free(tree_node->next);
}

/*
 * Doubles the space of the next array, if needed
 */
static inline int8_t
__uk_tree_double_size(struct uk_tree_node *place)
{
	place->next_nodes_free = place->next_nodes_nr;
	place->next_nodes_nr <<= 1;
	place->next = (struct uk_tree_node **)
			realloc(place->next, place->next_nodes_nr);

	if (unlikely(!place->next)) {
		return -ENOMEM;
	}
	
	return 0;
}

/*
 * Adds an element as a child to a node
 */
static inline void
__uk_tree_add(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	// Double the space (not safe)
	if (unlikely(!place->next_nodes_free)) {
		if(unlikely(__uk_tree_double_size(place) < 0)) {
			return;
		}
	}

	for (uint16_t i = 0; i < place->next_nodes_nr; ++i) {
		if (!place->next[i]) {
			new_entry->prev = place;
			place->next[i] = new_entry;
			break;
		}
	}

	UK_WRITE_ONCE(place->next_nodes_free, place->next_nodes_free - 1);
}

static inline void
uk_tree_add(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	__uk_tree_add(place, new_entry);
}

/*
 * Delete the entry and replace it with another
 */
static inline void
uk_tree_replace(struct uk_tree_node *old_entry, struct uk_tree_node *new_entry)
{
	struct uk_tree_node *parent = old_entry->prev;

	__uk_tree_del(old_entry);

	for (uint16_t i = 0; i < parent->next_nodes_nr; ++i) {
		if (!parent->next[i]) {
			parent->next[i] = new_entry;
			UK_WRITE_ONCE(parent->next_nodes_free,
						parent->next_nodes_free - 1);
			break;
		}
	}
}

/*
 * Delete a node and all children
 */
static inline void
uk_tree_del(struct uk_tree_node *tree_node)
{
	struct uk_tree_node *parent = tree_node->prev;

	// Remove entry in parent table
	if (likely(parent)) {
		printf("STATUS 2\n");
		for (uint16_t i = 0; i < parent->next_nodes_nr; ++i) {
			printf("STATUS 3\n");
			if (unlikely(parent->next[i] == tree_node)) {
				printf("STATUS 4\n");
				UK_WRITE_ONCE(parent->next[i], NULL);
				UK_WRITE_ONCE(parent->next_nodes_free,
						parent->next_nodes_free + 1);
				break;
			}
		}
	}

	__uk_tree_del(tree_node);

	tree_node->next = NULL;
}

/*
 * Takes a path of ints and returns the node found in that place
 */
static inline struct uk_tree_node *
uk_tree_find(struct uk_tree_node *root,
		const uint16_t *path, const uint16_t depth)
{
	if (unlikely(!root || !root->next)) {
		return NULL;
	}

	for (uint16_t i = 0; i < depth; ++i) {
		if (unlikely(!root->next[path[i]])) {
			return NULL;
		}
		root = root->next[path[i]];
	}

	return root;
}

#endif /* __STORE_TREE_H__ */
