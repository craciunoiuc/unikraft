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

#ifndef __STORE_INTERNAL_H__
#define __STORE_INTERNAL_H__

#include <uk/store/tree.h>
#include <uk/list.h>

// TODO Use static array cache or inside entries?
// TODO Move refcount to use LinkedList buckets? (what about strdup(string))
// TODO Reduce implementation to bare minimum
// TODO Move this to `inlcude/store.h`?
// TODO Tidy up

/* All types used by the structure */
enum uk_store_entry_type {
	UK_STORE_ENT_NONE,
	UK_STORE_ENT_S8,
	UK_STORE_ENT_U8,
	UK_STORE_ENT_S16,
	UK_STORE_ENT_U16,
	UK_STORE_ENT_S32,
	UK_STORE_ENT_U32,
	UK_STORE_ENT_S64,
	UK_STORE_ENT_U64,
	UK_STORE_ENT_UPTR,
	UK_STORE_ENT_INT,
	UK_STORE_ENT_CHAR,
	UK_STORE_ENT_CHARP,
};

/* Getter definitions */
typedef __s8 (*uk_store_get_s8_func_t)(void);
typedef __u8 (*uk_store_get_u8_func_t)(void);
typedef __s16 (*uk_store_get_s16_func_t)(void);
typedef __u16 (*uk_store_get_u16_func_t)(void);
typedef __s32 (*uk_store_get_s32_func_t)(void);
typedef __u32 (*uk_store_get_u32_func_t)(void);
typedef __s64 (*uk_store_get_s64_func_t)(void);
typedef __u64 (*uk_store_get_u64_func_t)(void);
typedef __uptr (*uk_store_get_uptr_func_t)(void);

/* Setter definitions */
typedef void (*uk_store_set_s8_func_t)(__s8);
typedef void (*uk_store_set_u8_func_t)(__u8);
typedef void (*uk_store_set_s16_func_t)(__s16);
typedef void (*uk_store_set_u16_func_t)(__u16);
typedef void (*uk_store_set_s32_func_t)(__s32);
typedef void (*uk_store_set_u32_func_t)(__u32);
typedef void (*uk_store_set_s64_func_t)(__s64);
typedef void (*uk_store_set_u64_func_t)(__u64);
typedef void (*uk_store_set_uptr_func_t)(__uptr);


/* Stores functions and their types and a node connection to the tree */
struct uk_store_entry {
	enum uk_store_entry_type get_type, set_type;
	union {
		uk_store_get_s8_func_t   s8;
		uk_store_get_u8_func_t   u8;
		uk_store_get_s16_func_t  s16;
		uk_store_get_u16_func_t  u16;
		uk_store_get_s32_func_t  s32;
		uk_store_get_u32_func_t  u32;
		uk_store_get_s64_func_t  s64;
		uk_store_get_u64_func_t  u64;
		uk_store_get_uptr_func_t uptr;
	} get;
	union {
		uk_store_set_s8_func_t   s8;
		uk_store_set_u8_func_t   u8;
		uk_store_set_s16_func_t  s16;
		uk_store_set_u16_func_t  u16;
		uk_store_set_s32_func_t  s32;
		uk_store_set_u32_func_t  u32;
		uk_store_set_s64_func_t  s64;
		uk_store_set_u64_func_t  u64;
		uk_store_set_uptr_func_t uptr;
	} set;
	char *entry_name;
	struct uk_tree_node node;
};

// TODO Change?
#define UK_STORE_REFCOUNT_MAX_SIZE 127

/* Path -> entry associations */
struct uk_store_refcount {
	const char *path;
	struct uk_store_entry *entry;
	struct uk_list_head head;
};

/* Buffer to store associations for faster access (sepparate caches) */
static struct uk_list_head refcount[UK_STORE_REFCOUNT_MAX_SIZE];
static int8_t init_list = 1;

/* Section array start point */
extern struct uk_store_entry *uk_store_libs;

/**
 * Must be used to define all entries registered in the section.
 * This is used to ensure that there are no overlaps in memory.
 *
 * @param name the entry name
 */
#define UK_STORE_DEFINE_ENTRY(name)	\
	static struct uk_store_entry _uk_store_section_head_##name __unused

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section
 */
#define __UK_STORE_ENTRY_REG(entry)					\
	__attribute((__section__(".uk_store_libs_list")))		\
	static struct uk_store_entry *__ptr_##entry __used = &entry

/**
 * Registers an entry in the section and initializes the tree structure.
 *
 * @param offset the offset where to register
 * @param name the name of the entry to save (char *)
 * @param entry the entry to register space for
 */
#define UK_STORE_STATIC_REGISTER_INIT(offset, name, entry)		\
	do {								\
		struct uk_store_entry *to_reg =				\
			(uk_store_libs + (offset));			\
		to_reg->get_type = to_reg->set_type = UK_STORE_ENT_NONE;\
		to_reg->entry_name = strdup(name);			\
		UK_TREE_NODE_INIT(&to_reg->node);			\
		__UK_STORE_ENTRY_REG(_uk_store_section_head_##entry);	\
	} while (0)

/**
 * Registers an entry in the section.
 *
 * @param offset the offset where to register
 * @param entry the entry to register space for
 */
#define UK_STORE_STATIC_REGISTER(offset, entry)				\
	do {								\
		struct uk_store_entry *to_reg =				\
			(uk_store_libs + (offset));			\
		__UK_STORE_ENTRY_REG(_uk_store_section_head_##entry);	\
	} while (0)

/**
 * Initializes an empty entry.
 *
 * @param entry the entry to initialize
 */
static inline void
uk_store_init_entry(struct uk_store_entry *entry)
{
	memset(entry, 0, sizeof(*entry));
	entry->get_type = entry->set_type = UK_STORE_ENT_NONE;
	UK_TREE_NODE_INIT(&entry->node);

	if (unlikely(init_list)) {
		init_list = 0;
		for (int i = 0; i < UK_STORE_REFCOUNT_MAX_SIZE; ++i)
			UK_INIT_LIST_HEAD(&refcount[i]);
	}
}

/**
 * Adds an entry to the `place`'s `next` pointers
 *
 * @param place the place where to add the new entry
 * @param entry the entry to add
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_add_entry(struct uk_store_entry *place, struct uk_store_entry *entry)
{
	if (unlikely(!place || !entry))
		return -EINVAL;

	return uk_tree_add(&place->node, &entry->node);
}

/**
 * Deletes an entry and its children (does not free the name of the children)
 *
 * @param entry the entry to delete
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_del_entry(struct uk_store_entry *entry, struct uk_store_entry *parent)
{
	if (unlikely(!entry))
		return -EINVAL;

	entry->get_type = entry->set_type = UK_STORE_ENT_NONE;
	if (entry->entry_name) {
		free(entry->entry_name);
		entry->entry_name = NULL;
	}

	return uk_tree_del(&entry->node, (parent ? &parent->node : NULL));
}

/* Taken from https://xorshift.di.unimi.it/splitmix64.c */
static inline uint64_t
uk_store_cache_hash(uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return (x ^ (x >> 31)) % UK_STORE_REFCOUNT_MAX_SIZE;
}

/**
 * Searches for an association in the cache
 *
 * @param path the path to check for
 * @return the association or NULL on error
 */
static inline struct uk_store_refcount *
uk_store_cache_find(const char *path)
{
	int bucket = uk_store_cache_hash((uintptr_t)path);
	struct uk_store_refcount *to_ret;

	if (unlikely(!path))
		return NULL;

	uk_list_for_each_entry(to_ret, &refcount[bucket], head) {
		if (to_ret->path == path)
			return to_ret;
	}

	return NULL;
}

/**
 * Adds an entry to the cache
 *
 * @param entry the entry to store
 * @param path the path to check for
 * @return the position occupied or < 0 if error
 */
static inline int
uk_store_cache_entry(struct uk_store_entry *entry, const char *path)
{
	int to_store = uk_store_cache_hash((uintptr_t)path);
	struct uk_store_refcount *assoc;

	if (unlikely(!path))
		return -EINVAL;

	assoc = calloc(1, sizeof(*assoc));
	if (!assoc)
		return -ENOMEM;

	assoc->entry = entry;
	assoc->path = path;

	uk_list_add_tail(&assoc->head, &refcount[to_store]);

	return 0;
}

/**
 * Clears a path-entry association
 *
 * @param path the path to check for
 */
static inline void
uk_store_cache_release_path(const char *path)
{
	struct uk_store_refcount *to_release = uk_store_cache_find(path);

	if (to_release != NULL) {
		uk_list_del(&to_release->head);
		free(to_release);
	}
}

/**
 * Flush the cache 
 *
 */
static inline void
uk_store_cache_release_all()
{
	struct uk_store_refcount *p, *n;

	for (int i = 0; i < UK_STORE_REFCOUNT_MAX_SIZE; ++i) {
		uk_list_for_each_entry_safe(p, n, &refcount[i], head) {
			uk_list_del(&p->head);
			free(p);
		}
	}
}

/**
 * Checks for an entry to the cache with the given path and returns it
 *
 * @param path the path wanted
 * @return the entry or NULL if not found
 */
static inline struct uk_store_entry *
uk_store_get_entry_by_cache(const char *path)
{
	struct uk_store_refcount *to_ret = uk_store_cache_find(path);

	return to_ret ? to_ret->entry : NULL;
}

/**
 * Returns an entry after following the given path
 *
 * @param root the node where to start the search
 * @param path the path to follow
 * @return the entry or NULL if not found
 */
static inline struct uk_store_entry *
uk_store_get_entry_by_path(struct uk_store_entry *root, const char *path)
{
	struct uk_store_entry *entry = NULL;
	const char *slash;

	if (*path == '\0')
		return root;

	slash = strchr(path, '/');
	if (slash == NULL)
		slash = strchr(path, '\0');

	for (uint16_t idx = 0; idx < root->node.next_nodes_nr; ++idx) {
		if (!root->node.next[idx])
			continue;

		entry = uk_tree_entry(root->node.next[idx],
					struct uk_store_entry, node);

		if (!strncmp(entry->entry_name, path, slash - path)) {
			return uk_store_get_entry_by_path(entry,
					slash + !!(*slash));
		}
	}

	return NULL;
}

/**
 * Returns a saved entry
 *
 * @param entry the entry where to start the search
 * @param path the path to follow
 * @return the found entry or NULL
 */
static inline struct uk_store_entry *
uk_store_get_entry(struct uk_store_entry *root, const char *path)
{
	struct uk_store_entry *to_ret = uk_store_get_entry_by_cache(path);

	if (!to_ret) {
		uk_store_cache_entry(root, path);
		return uk_store_get_entry_by_path(root, path);
	}
	return to_ret;
}

/**
 * Checks if an entry is a file (leaf)
 *
 * @param entry the entry to check
 * @return 1 if file, 0 if not, or < 0 on fail
 */
static inline int
uk_store_is_file(struct uk_store_entry *entry)
{
	if (unlikely(!entry))
		return -EINVAL;

	return uk_tree_is_leaf(&entry->node);
}

/**
 * Updates the name of an entry
 *
 * @param entry the entry where to update the name
 * @param name the new name
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_update_name(struct uk_store_entry *entry, const char *name)
{
	if (unlikely(!entry))
		return -EINVAL;

	if (entry->entry_name)
		free(entry->entry_name);

	entry->entry_name = strdup(name);

	if (!entry->entry_name)
		return -ENOMEM;

	return 0;
}

/**
 * Saves a new getter function in the entry
 *
 * @param entry the place where to store the function
 * @param type the new type of the function
 * @param func the function
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_update_getter(struct uk_store_entry *entry,
			enum uk_store_entry_type type, void *func)
{
	if (unlikely(!entry))
		return -EINVAL;

	entry->get_type = type;
	switch (entry->get_type) {
	case UK_STORE_ENT_INT:
	case UK_STORE_ENT_S32:
		entry->get.s32 = func;
		break;

	case UK_STORE_ENT_S16:
		entry->get.s16 = func;
		break;

	case UK_STORE_ENT_CHAR:
	case UK_STORE_ENT_S8:
		entry->get.s8 = func;
		break;

	case UK_STORE_ENT_S64:
		entry->get.s64 = func;
		break;

	case UK_STORE_ENT_U32:
		entry->get.u32 = func;
		break;

	case UK_STORE_ENT_U16:
		entry->get.u16 = func;
		break;

	case UK_STORE_ENT_U8:
		entry->get.u8 = func;
		break;

	case UK_STORE_ENT_U64:
		entry->get.u64 = func;
		break;

	case UK_STORE_ENT_UPTR:
	case UK_STORE_ENT_CHARP:
		entry->get.uptr = func;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * Saves a new setter function in the entry
 *
 * @param entry the place where to save the new function
 * @param type the new type of the function
 * @param func the function
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_update_setter(struct uk_store_entry *entry,
			enum uk_store_entry_type type, void *func)
{
	if (unlikely(!entry))
		return -EINVAL;

	entry->set_type = type;
	switch (entry->set_type) {
	case UK_STORE_ENT_INT:
	case UK_STORE_ENT_S32:
		entry->set.s32 = func;
		break;

	case UK_STORE_ENT_S16:
		entry->set.s16 = func;
		break;

	case UK_STORE_ENT_CHAR:
	case UK_STORE_ENT_S8:
		entry->set.s8 = func;
		break;

	case UK_STORE_ENT_S64:
		entry->set.s64 = func;
		break;

	case UK_STORE_ENT_U32:
		entry->set.u32 = func;
		break;

	case UK_STORE_ENT_U16:
		entry->set.u16 = func;
		break;

	case UK_STORE_ENT_U8:
		entry->set.u8 = func;
		break;

	case UK_STORE_ENT_U64:
		entry->set.u64 = func;
		break;

	case UK_STORE_ENT_UPTR:
	case UK_STORE_ENT_CHARP:
		entry->set.uptr = func;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * Gets the value returned by the saved function and puts it in `out`
 *
 * @param entry the entry to use
 * @param out the place where to store the result
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_get_value(struct uk_store_entry *entry, void *out)
{
	if (unlikely(!entry))
		return -EINVAL;

	switch (entry->get_type) {
	case UK_STORE_ENT_INT:
	case UK_STORE_ENT_S32:
		*((__s32 *) out) = entry->get.s32();
		break;

	case UK_STORE_ENT_S16:
		*((__s16 *) out) = entry->get.s16();
		break;

	case UK_STORE_ENT_CHAR:
	case UK_STORE_ENT_S8:
		*((__s8 *) out) = entry->get.s8();
		break;

	case UK_STORE_ENT_S64:
		*((__s64 *) out) = entry->get.s64();
		break;

	case UK_STORE_ENT_U32:
		*((__u32 *) out) = entry->get.u32();
		break;

	case UK_STORE_ENT_U16:
		*((__u16 *) out) = entry->get.u16();
		break;

	case UK_STORE_ENT_U8:
		*((__u8 *) out) = entry->get.u8();
		break;

	case UK_STORE_ENT_U64:
		*((__u64 *) out) = entry->get.u64();
		break;

	case UK_STORE_ENT_UPTR:
	case UK_STORE_ENT_CHARP:
		*((__uptr *) out) = entry->get.uptr();
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * Sets the value from `in` with the saved function
 *
 * @param entry the entry to use
 * @param in the value to give to the setter
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_set_value(struct uk_store_entry *entry, void *in)
{
	if (unlikely(!entry))
		return -EINVAL;

	switch (entry->set_type) {
	case UK_STORE_ENT_INT:
	case UK_STORE_ENT_S32:
		entry->set.s32(*((__s32 *) in));
		break;

	case UK_STORE_ENT_S16:
		entry->set.s16(*((__s16 *) in));
		break;

	case UK_STORE_ENT_CHAR:
	case UK_STORE_ENT_S8:
		entry->set.s8(*((__s8 *) in));
		break;

	case UK_STORE_ENT_S64:
		entry->set.s64(*((__s64 *) in));
		break;

	case UK_STORE_ENT_U32:
		entry->set.u32(*((__u32 *) in));
		break;

	case UK_STORE_ENT_U16:
		entry->set.u16(*((__u16 *) in));
		break;

	case UK_STORE_ENT_U8:
		entry->set.u8(*((__u8 *) in));
		break;

	case UK_STORE_ENT_U64:
		entry->set.u64(*((__u64 *) in));
		break;

	case UK_STORE_ENT_UPTR:
	case UK_STORE_ENT_CHARP:
		entry->set.uptr(*((__uptr *) in));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#endif /* __STORE_INTERNAL_H__ */
