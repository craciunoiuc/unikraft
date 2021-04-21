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

// TODO Reduce implementation to bare minimum
// TODO Move this to `include/store.h`?
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
	const char *entry_name;
	enum uk_store_entry_type type;
	// struct uk_tree_node node;
}; // TODO look up align macro essentials.h

struct uk_store_folder {
	uk_list_head head;
};

struct uk_store_folder_entry {
	struct uk_store_entry;
	uk_list_head head;
	int32_t refcount; // use __atomic
};
// list of folders
// folder with list of entries

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
#define __UK_STORE_ENTRY_REG(entry, todo_later)				\
	__attribute((__section__(".uk_store_libs_list")))		\
	static struct uk_store_entry __ptr_##entry __used = {
		.todo_later
	};
// do init here
// not a pointer

// dynamic
#define uk_store_entry_init(entry, macro_name, macro_type, getter, setter)	\
	do {							\
		entry->entry_name = macro_name;			\
		entry->type = macro_type;			\
		entry->get.macro_type = getter;			\
		entry->set.macro_type = setter;			\
	} while(0)


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
		to_reg->refcount = 0;					\
		UK_TREE_NODE_INIT(&to_reg->node);			\
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
	UK_TREE_NODE_INIT(&entry->node);
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
	struct uk_store_entry *ret = uk_store_get_entry_by_path(root, path);

	if (ret)
		ret->refcount++;

	return ret;
}

/**
 * Returns a saved entry
 *
 * @param entry the entry where to start the search
 * @param path the path to follow
 * @return the found entry or NULL
 */
static inline void
uk_store_release_entry(struct uk_store_entry **entry)
{
	if (unlikely(!entry))
		return;

	(*entry)->refcount--;
	if((*entry)->refcount <= 0) {
		(*entry)->refcount = 0;
		*entry = NULL;
	}
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
		entry->set.__s32(*((__s32 *) in));
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
