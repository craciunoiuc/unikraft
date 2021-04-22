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

#include <uk/essentials.h>
#include <string.h>
#include <uk/list.h>
#include <uk/arch/atomic.h>

/*
uk_store_folder -> uk_store_folder -> uk_store_folder (static)
      |
      V
uk_store_folder_entry
      |
      V
uk_store_folder_entry

[uk_store_entry uk_store_entry uk_store_entry uk_store_entry] static
uk_store_entry | uk_store_entry | uk_store_entry | uk_store_entry dynamic

-------------------------------------------------------------------------------

[uk_alloc uk_sched uk_netdev]
    |
    V
"total_mem"
    |
    V
"1/alloc_mem"
    |
    V
"2/alloc_mem"

*/


#ifdef __cplusplus
extern "C" {
#endif

/* All types used by the structure */
enum uk_store_entry_type { // TODO Give up these types?
	UK_STORE_ENT_NONE  = 0,
	UK_STORE_ENT_S8    = 1,
	UK_STORE_ENT_U8    = 2,
	UK_STORE_ENT_S16   = 3,
	UK_STORE_ENT_U16   = 4,
	UK_STORE_ENT_S32   = 5,
	UK_STORE_ENT_U32   = 6,
	UK_STORE_ENT_S64   = 7,
	UK_STORE_ENT_U64   = 8,
	UK_STORE_ENT_UPTR  = 9,
	UK_STORE_ENT_INT   = 5,
	UK_STORE_ENT_CHAR  = 1,
	UK_STORE_ENT_CHARP = 9
};

/* All basic types that exist */
enum uk_store_entry_basic_type {
	none = 0,
	s8   = 1,
	u8   = 2,
	s16  = 3,
	u16  = 4,
	s32  = 5,
	u32  = 6,
	s64  = 7,
	u64  = 8,
	uptr = 9,
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
	__u16 flags;
	__u16 type;
} __align8;

struct uk_store_folder {
	// struct uk_list_head head;
	struct uk_list_head folder_head;
} __align8;
// TODO Should folders be static or dynamic? (static -> no need for a uk_list of folders)

struct uk_store_folder_entry {
	struct uk_store_entry *entry;
	struct uk_list_head list_head;
	__atomic refcount;
} __align8;

#define UK_STORE_FLAG_STATIC	1
#define UK_STORE_FLAG_DYNAMIC	2

#define	uk_store_get_folder_entry(ptr)	\
	__containerof(ptr, struct uk_store_folder_entry, entry)

/* Static entry array start+end points */
extern struct uk_store_entry *uk_store_entries_start;
extern struct uk_store_entry *uk_store_entries_end;

/* Static folders array start+end points */
extern struct uk_store_folder *uk_store_libs_start;
extern struct uk_store_folder *uk_store_libs_end;
// TODO How to get library indexes? I forgot :(

#define UK_STORE_INITREG_FOLDER(fldr)					\
	static struct uk_store_folder					\
	__used __section(".uk_store_libs_list") __align8		\
	__uk_store_folder_list ## _ ## fldr  = {			\
		.folder_head = UK_LIST_HEAD_INIT(			\
		__uk_store_folder_list ## _ ## fldr.folder_head)	\
	}

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section
 */
#define _UK_STORE_INITREG_ENTRY(entry, e_name, e_type, e_get, e_set)	\
	static const struct uk_store_entry				\
	__used __section(".uk_store_entries_list") __align8		\
	__uk_store_entries_list ## _ ## entry  = {			\
		.entry_name = (e_name),					\
		.type       = (e_type),					\
		.get.e_type = (e_get),					\
		.set.e_type = (e_set),					\
		.flags      = UK_STORE_FLAG_STATIC,			\
	}

#define UK_STORE_INITREG_ENTRY(entry, e_name, e_type, e_get, e_set)	\
	_UK_STORE_INITREG_ENTRY(entry, e_name, e_type, e_get, e_set)

#define uk_store_entry_init(entry, e_name, e_type, getter, setter)	\
	do {								\
		(entry)->entry_name = (e_name);				\
		(entry)->type       = (e_type);				\
		(entry)->get.e_type = getter;				\
		(entry)->set.e_type = setter;				\
		(entry)->flags      = UK_STORE_FLAG_DYNAMIC;		\
	} while (0)

#define uk_store_folder_entry_init(folder_entry, new_entry)		\
	do {								\
		(folder_entry)->entry = new_entry;			\
		ukarch_store_n(&(folder_entry)->refcount.counter, 0);	\
	} while (0)

/**
 * Adds a folder entry to a folder
 *
 * @param folder the place where to add the new entry
 * @param folder_entry the entry to add
 */
static inline void
uk_store_add_folder_entry(struct uk_store_folder *folder,
			struct uk_store_folder_entry *folder_entry)
{
	uk_list_add(&folder_entry->list_head, &folder->folder_head);
}

/**
 * Removes a folder entry from a folder
 *
 * @param folder_entry the entry to delete
 */
static inline void
uk_store_del_folder_entry(struct uk_store_folder_entry *folder_entry)
{
	uk_list_del(&folder_entry->list_head);
}


/**
 * Searches for a name in a folder
 *
 * @param folder the folder to search in
 * @param name the name to search for
 * @return the entry or NULL if not found
 */
static inline struct uk_store_folder_entry *
_uk_store_find_entry(struct uk_store_folder *folder, const char *name) // TODO Move to .c?
{
	struct uk_store_folder_entry *iter;

	uk_list_for_each_entry(iter, &folder->folder_head, list_head)
		if (!strcmp(iter->entry->entry_name, name))
			return iter;

	return NULL;
}

/**
 * Searches for an entry in a folder and returns it
 *
 * @param entry the entry where to start the search
 * @param path the path to follow
 * @return the found entry or NULL
 */
static inline struct uk_store_entry *
uk_store_get_entry(struct uk_store_folder *folder, const char *name)
{
	struct uk_store_folder_entry *res = _uk_store_find_entry(folder, name);

	if (res) {
		ukarch_inc(&res->refcount.counter);
		return res->entry;
	}

	return NULL;
}

/**
 * Returns a saved entry
 *
 * @param entry the entry where to start the search
 * @return the found entry or NULL
 */
static inline void
uk_store_release_entry(struct uk_store_entry **entry) // TODO also delete from list?
{
	struct uk_store_folder_entry *res = uk_store_get_folder_entry(entry);

	ukarch_dec(&res->refcount.counter);
	/*
	if(ukarch_load(res->refcount) <= 0) {
		uk_store_del_folder_entry(res);
	}
	*/
	*entry = NULL;
}


/**
 * Saves a new getter function in the entry
 *
 * @param entry the place where to store the function
 * @param e_type the new type of the function
 * @param func the function
 * @return 0 on success or < 0 on fail
 */
#define uk_store_update_getter(entry, e_type, func)	\
	do {						\
		if (unlikely(!(entry) || !(e_type)))	\
			break;				\
		if ((entry)->type != (__u8)(e_type))	\
			(entry)->set.e_type = NULL;	\
		(entry)->type = (e_type);		\
		(entry)->get.e_type = func;		\
	} while (0)

/**
 * Saves a new setter function in the entry
 *
 * @param entry the place where to save the new function
 * @param type the new type of the function
 * @param func the function
 * @return 0 on success or < 0 on fail
 */
#define uk_store_update_setter(entry, e_type, func)	\
	do {						\
		if (unlikely(!(entry) || !(e_type)))	\
			break;				\
		if ((entry)->type != (__u8)(e_type))	\
			(entry)->get.e_type = NULL;	\
		(entry)->type = (e_type);		\
		(entry)->set.e_type = func;		\
	} while (0)

/**
 * Gets the value returned by the saved function and puts it in `out`
 *
 * @param entry the entry to use
 * @param e_type the type of the function (basic type)
 * @param out the place where to store the result
 * @return 0 on success or < 0 on fail
 */
// TODO Remove e_type from args? Go back to the switch?
// standard = 0.055, inline = 0.035, macro = 0.025
#define uk_store_get_value_macro(entry, e_type, out)			\
	do {								\
		if (unlikely(!(entry) || !(e_type)))			\
			break;						\
		*((__##e_type *) (out)) = (entry)->get.e_type();	\
	} while (0)

extern void uk_store_get_value(struct uk_store_entry *entry, void *out);

/**
 * Sets the value from `in` with the saved function
 *
 * @param entry the entry to use
 * @param e_type the type of the function (basic type)
 * @param in the value to give to the setter
 * @return 0 on success or < 0 on fail
 */
// TODO Remove e_type from args? Go back to the switch?
// standard = 0.055, inline = 0.035, macro = 0.025
#define uk_store_set_value_macro(entry, e_type, in)			\
	do {								\
		if (unlikely(!(entry) || !(e_type)))			\
			break;						\
		(entry)->set.e_type(*((__##e_type *) (in)));		\
	} while (0)

extern void uk_store_set_value(struct uk_store_entry *entry, void *in);

#ifdef __cplusplus
}
#endif

#endif /* __STORE_INTERNAL_H__ */
