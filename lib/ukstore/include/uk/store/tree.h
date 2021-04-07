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

struct uk_tree_node {
	struct uk_tree_node *prev;
	struct uk_tree_node **next;
	uint16_t next_nodes_nr;
	uint16_t next_nodes_free;
};

static uint16_t __default_nodes_nr = 2;

/**
 * Returns an entry that contains the node.
 *
 * @param ptr pointer to the node
 * @param type the structure that contains the node
 * @param field structure field name
 * @return pointer to the entry
 */
#define uk_tree_entry(ptr, type, field) __containerof(ptr, type, field)

/**
 * Used internally for the index
 */
#define __uk_tree_for_internal()	\
	for (int32_t __uk_tree_idx = -1; unlikely(__uk_tree_idx == -1);)


/**
 * Iterates through all next nodes.
 *
 * Checkpatch error is false positive.
 *
 * @param p pointer iterator
 * @param root the parent where to iterate
 */
#define uk_tree_for_each_child(p, root)					\
	__uk_tree_for_internal()					\
		for (__uk_tree_idx = 0, p = (root)->next[__uk_tree_idx];\
			__uk_tree_idx < (root)->next_nodes_nr;		\
			++__uk_tree_idx,				\
			p = (root)->next[__uk_tree_idx])


/**
 * Iterates through all next nodes and gets the entry
 *
 * Checkpatch error is false positive.
 *
 * @param p pointer iterator
 * @param root the parent where to iterate
 * @param field structure field name
 */
#define uk_tree_for_each_child_entry(p, root, field)			\
	__uk_tree_for_internal()					\
		for (__uk_tree_idx = 0,	(p) = uk_tree_entry(		\
				(&(root)->field)->next[__uk_tree_idx],	\
				typeof(*(root)), field);		\
			__uk_tree_idx < (&(root)->field)->next_nodes_nr;\
			++__uk_tree_idx,				\
			(p) = uk_tree_entry(				\
				(&(root)->field)->next[__uk_tree_idx],	\
				typeof(*(root)), field))


/**
 * Initializes a node / the root.
 *
 * @param name the root
 */
#define UK_TREE_ROOT_INIT(name)	\
	__uk_tree_init_node(name)


/**
 * Modifies the default size of the next array.
 *
 * @param size array size
 */
#define UK_TREE_SET_DEFAULT_SIZE(size)	\
	do { (__default_nodes_nr) = (size) } while (0)

/**
 * Initializes a node with default values.
 *
 * @param node pointer to the node to be initialized
 * @return 0 on success, negative on error
 */
static inline int8_t
__uk_tree_init_node(struct uk_tree_node *node)
{
	if (unlikely(!node))
		return -EINVAL;

	node->prev = NULL;
	node->next_nodes_nr = __default_nodes_nr;
	node->next = (struct uk_tree_node **)
			calloc(node->next_nodes_nr, sizeof(*node->next));

	if (likely(node->next)) {
		UK_WRITE_ONCE(node->next_nodes_free, __default_nodes_nr);
		return 0;
	} else {
		return -ENOMEM;
	}
}

/**
 * Deletes a node and all subsequent children.
 *
 * @param tree_node pointer to the node where to start deletion
 */
static inline void
__uk_tree_del(struct uk_tree_node *tree_node)
{
	if (unlikely(!tree_node))
		return;

	for (uint16_t i = 0; i < tree_node->next_nodes_nr; ++i) {
		if (!tree_node->next[i])
			__uk_tree_del(tree_node->next[i]);
	}

	if (likely(tree_node->next)) {
		free(tree_node->next);
		tree_node->next = NULL;
	}
}


/**
 * Doubles the capacity of the next array.
 *
 * @param place where to double the capacity
 * @return 0 on success, negative on error
 */
static inline int8_t
__uk_tree_double_size(struct uk_tree_node *place)
{
	if (unlikely(!place || !place->next))
		return -EINVAL;

	place->next_nodes_free = place->next_nodes_nr;
	place->next_nodes_nr <<= 1;

	place->next = (struct uk_tree_node **) realloc(place->next,
				sizeof(*place->next) * place->next_nodes_nr);

	memset(place->next + place->next_nodes_free, 0,
		sizeof(place->next) * place->next_nodes_free);

	if (unlikely(!place->next))
		return -ENOMEM;

	return 0;
}


/**
 * Shrinks next array to exactly the needed capacity.
 *
 * @param place where to do the resize
 * @return 0 on success, negative on error
 */
static inline int8_t
uk_tree_shrink_to_fit(struct uk_tree_node *place)
{
	if (unlikely(!place || !place->next))
		return -EINVAL;

	place->next_nodes_nr -= place->next_nodes_free;
	place->next_nodes_free = 0;
	place->next = (struct uk_tree_node **) realloc(place->next,
				sizeof(*place->next) * place->next_nodes_nr);

	if (unlikely(!place->next))
		return -ENOMEM;

	return 0;
}


/**
 * Adds a node to the tree.
 *
 * @param place where to add the node as a child
 * @param new_entry node to be added
 * @return 0 on success, negative on error
 */
static inline int8_t
__uk_tree_add(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	int8_t ret = 0;

	if (unlikely(!place->next_nodes_free)) {
		ret = __uk_tree_double_size(place);
		if (unlikely(ret < 0))
			return ret;
	}

	for (uint16_t i = 0; i < place->next_nodes_nr; ++i) {
		if (place->next[i] == NULL) {
			new_entry->prev = place;
			place->next[i] = new_entry;
			break;
		}
	}
	UK_WRITE_ONCE(place->next_nodes_free, place->next_nodes_free - 1);
	return ret;
}

/**
 * Initializes a node and adds it to the tree.
 *
 * @param place where to add the node as a child
 * @param new_entry node to be added
 * @return 0 on success, negative on error
 */
static inline int8_t
uk_tree_add_new(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	__uk_tree_init_node(new_entry);
	return __uk_tree_add(place, new_entry);
}

/**
 * Adds an already initalized node to the tree.
 *
 * @param place where to add the node as a child
 * @param new_entry node to be added
 * @return 0 on success, negative on error
 */
static inline int8_t
uk_tree_add_existing(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	return __uk_tree_add(place, new_entry);
}

/**
 * Deletes a node and all children and removes it as child from its parent.
 *
 * @param tree_node node to be deleted
 * @return 0 on success, negative on error
 */
static inline int8_t
uk_tree_del(struct uk_tree_node *tree_node)
{
	struct uk_tree_node *parent = tree_node->prev;

	if (unlikely(!tree_node))
		return -EINVAL;

	if (likely(parent)) {
		for (uint16_t i = 0; i < parent->next_nodes_nr; ++i) {
			if (unlikely(parent->next[i] == tree_node)) {
				UK_WRITE_ONCE(parent->next[i], NULL);
				UK_WRITE_ONCE(parent->next_nodes_free,
						parent->next_nodes_free + 1);
				break;
			}
		}
	}

	__uk_tree_del(tree_node);

	tree_node->next = NULL;
	return 0;
}

/**
 * Replaces a node with another existing one.
 *
 * @param old_entry the node to be replaced
 * @param new_entry the node to be added
 * @return 0 on success, negative on error
 */
static inline int8_t
uk_tree_replace(struct uk_tree_node *old_entry, struct uk_tree_node *new_entry)
{
	struct uk_tree_node *parent;
	int8_t ret;

	if (unlikely(!old_entry))
		return -EINVAL;

	parent = old_entry->prev;

	ret = uk_tree_del(old_entry);
	if (ret)
		return ret;

	for (uint16_t i = 0; i < parent->next_nodes_nr; ++i) {
		if (!parent->next[i]) {
			parent->next[i] = new_entry;
			break;
		}
	}

	return 0;
}

/**
 * Returns a node based on a given path.
 *
 * @param root where to start search
 * @param path indexes of all next arrays
 * @param depth length of the path array
 * @return 0 on success, negative on error
 */
static inline struct uk_tree_node *
uk_tree_find(struct uk_tree_node *root,
		const uint16_t *path, const uint16_t depth)
{
	if (unlikely(!root || !root->next))
		return NULL;

	for (uint16_t i = 0; i < depth; ++i) {
		if (unlikely(!root->next[path[i]]))
			return NULL;
		root = root->next[path[i]];
	}

	return root;
}

/**
 * Checks if a node is a leaf.
 *
 * @param node the node to check
 * @return 0 if leaf, 1 if not
 */
static inline int8_t
uk_tree_is_leaf(struct uk_tree_node *node)
{
	if (unlikely(!node || !node->next))
		return 1;

	for (uint16_t i = 0; i < node->next_nodes_nr; ++i) {
		if (node->next[i])
			return 0;
	}

	return 1;
}

/**
 * Applies a function on all nodes in postorder.
 *
 * @param node the place where to start applying
 * @param f the function that applies an operation on nodes
 */
static inline void
uk_tree_node_apply(struct uk_tree_node *node, void (*f)(struct uk_tree_node *))
{
	struct uk_tree_node *elem;

	if (!node)
		return;

	if (node->next) {
		uk_tree_for_each_child(elem, node) {
			uk_tree_node_apply(elem, f);
		}
	}

	f(node);
}

#endif /* __STORE_TREE_H__ */
