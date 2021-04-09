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

#include <uk/arch/lcpu.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#define UK_TREE_LEAF_FLAG	1

struct uk_tree_node {
	struct uk_tree_node **next;
	uint16_t next_nodes_nr;
	uint16_t next_nodes_free;
	uint32_t flags;
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
 * Initializes a node / the root.
 *
 * @param name the root
 */
#define UK_TREE_NODE_INIT(name)	\
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
static inline int
__uk_tree_init_node(struct uk_tree_node *node)
{
	if (unlikely(!node))
		return -EINVAL;

	node->next_nodes_nr = __default_nodes_nr;
	node->flags = 0;
	node->next = (struct uk_tree_node **)
			calloc(node->next_nodes_nr, sizeof(*node->next));

	if (likely(node->next)) {
		node->next_nodes_free = __default_nodes_nr;
		node->flags |= UK_TREE_LEAF_FLAG;
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
static inline int
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
static inline int
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
static inline int
uk_tree_add(struct uk_tree_node *place, struct uk_tree_node *new_entry)
{
	int ret = 0;

	if (unlikely(!place->next_nodes_free)) {
		ret = __uk_tree_double_size(place);
		if (unlikely(ret < 0))
			return ret;
	}

	for (uint16_t i = 0; i < place->next_nodes_nr; ++i) {
		if (place->next[i] == NULL) {
			place->next[i] = new_entry;
			place->flags &= ~UK_TREE_LEAF_FLAG;
			break;
		}
	}
	place->next_nodes_free--;
	return ret;
}

/**
 * Deletes a node and all children and removes it as child from its parent.
 *
 * @param node_to_del node to be deleted
 * @param parent node which has node_to_del as a child
 * @return 0 on success, negative on error
 */
static inline int
uk_tree_del(struct uk_tree_node *node_to_del, struct uk_tree_node *parent)
{
	if (unlikely(!node_to_del))
		return -EINVAL;

	if (likely(parent)) {
		for (uint16_t i = 0; i < parent->next_nodes_nr; ++i) {
			if (unlikely(parent->next[i] == node_to_del)) {
				parent->next[i] = NULL;
				parent->next_nodes_free++;
				break;
			}
		}
		if (parent->next_nodes_free == parent->next_nodes_nr)
			parent->flags |= UK_TREE_LEAF_FLAG;
	}


	__uk_tree_del(node_to_del);

	node_to_del->next = NULL;
	return 0;
}

/**
 * Checks if a node is a leaf.
 *
 * @param node the node to check
 * @return 0 if leaf, 1 if not
 */
static inline int
uk_tree_is_leaf(struct uk_tree_node *node)
{
	if (unlikely(!node || !node->next))
		return 1;

	return node->flags & UK_TREE_LEAF_FLAG;
}

#endif /* __STORE_TREE_H__ */
