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

// TODO Save uk_tree_node-s in the section insteam of uk_store_entry-s?
// TODO refcount -> hash path into integer and store it?

/* All types used by the structure */
enum uk_store_entry_type {
	UK_STORE_NONE,
	UK_STORE_ENT_BOOL,
	UK_STORE_ENT_INT,
	UK_STORE_ENT_SHORT,
	UK_STORE_ENT_CHAR,
	UK_STORE_ENT_LONG,
	UK_STORE_ENT_LONGLONG,
	UK_STORE_ENT_FLOAT,
	UK_STORE_ENT_DOUBLE,
	UK_STORE_ENT_UINT,
	UK_STORE_ENT_USHORT,
	UK_STORE_ENT_UCHAR,
	UK_STORE_ENT_ULONG,
	UK_STORE_ENT_ULONGLONG,
	UK_STORE_ENT_VOID,
	UK_STORE_ENT_VOIDSTAR,
};

/* Getter definitions */
typedef _Bool (*uk_store_get_bool_func_t)(void);
typedef int (*uk_store_get_int_func_t)(void);
typedef short (*uk_store_get_short_func_t)(void);
typedef char (*uk_store_get_char_func_t)(void);
typedef long (*uk_store_get_long_func_t)(void);
typedef long long (*uk_store_get_longlong_func_t)(void);
typedef float (*uk_store_get_float_func_t)(void);
typedef double (*uk_store_get_double_func_t)(void);
typedef unsigned int (*uk_store_get_uint_func_t)(void);
typedef unsigned short (*uk_store_get_ushort_func_t)(void);
typedef unsigned char (*uk_store_get_uchar_func_t)(void);
typedef unsigned long (*uk_store_get_ulong_func_t)(void);
typedef unsigned long long (*uk_store_get_ulonglong_func_t)(void);
typedef void (*uk_store_get_void_func_t)(void);
typedef void *(*uk_store_get_voidstar_func_t)(void);

/* Setter definitions */
typedef void (*uk_store_set_bool_func_t)(_Bool);
typedef void (*uk_store_set_int_func_t)(int);
typedef void (*uk_store_set_short_func_t)(short);
typedef void (*uk_store_set_char_func_t)(char);
typedef void (*uk_store_set_long_func_t)(long);
typedef void (*uk_store_set_longlong_func_t)(long long);
typedef void (*uk_store_set_float_func_t)(float);
typedef void (*uk_store_set_double_func_t)(double);
typedef void (*uk_store_set_uint_func_t)(unsigned int);
typedef void (*uk_store_set_ushort_func_t)(unsigned short);
typedef void (*uk_store_set_uchar_func_t)(unsigned char);
typedef void (*uk_store_set_ulong_func_t)(unsigned long);
typedef void (*uk_store_set_ulonglong_func_t)(unsigned long long);
typedef void (*uk_store_set_void_func_t)(void);
typedef void (*uk_store_set_voidstar_func_t)(void *);


/* Stores functions and their types and a node connection to the tree */
struct uk_store_entry {
	enum uk_store_entry_type get_type, set_type;
	union {
		uk_store_get_bool_func_t      _bool;
		uk_store_get_int_func_t       _int;
		uk_store_get_short_func_t     _short;
		uk_store_get_char_func_t      _char;
		uk_store_get_long_func_t      _long;
		uk_store_get_longlong_func_t  _longlong;
		uk_store_get_float_func_t     _float;
		uk_store_get_double_func_t    _double;
		uk_store_get_uint_func_t      _uint;
		uk_store_get_ushort_func_t    _ushort;
		uk_store_get_uchar_func_t     _uchar;
		uk_store_get_ulong_func_t     _ulong;
		uk_store_get_ulonglong_func_t _ulonglong;
		uk_store_get_void_func_t      _void;
		uk_store_get_voidstar_func_t  _voidstar;
	} get;
	union {
		uk_store_set_bool_func_t      _bool;
		uk_store_set_int_func_t       _int;
		uk_store_set_short_func_t     _short;
		uk_store_set_char_func_t      _char;
		uk_store_set_long_func_t      _long;
		uk_store_set_longlong_func_t  _longlong;
		uk_store_set_float_func_t     _float;
		uk_store_set_double_func_t    _double;
		uk_store_set_uint_func_t      _uint;
		uk_store_set_ushort_func_t    _ushort;
		uk_store_set_uchar_func_t     _uchar;
		uk_store_set_ulong_func_t     _ulong;
		uk_store_set_ulonglong_func_t _ulonglong;
		uk_store_set_void_func_t      _void;
		uk_store_set_voidstar_func_t  _voidstar;
	} set;
	struct uk_tree_node node;
};

/* Section array start point */
extern struct uk_store_entry *uk_store_libs;

/* Used for registration and nothing else */
static struct uk_store_entry uk_store_section_head __unused;
static const struct uk_store_entry uk_store_section_entry __unused = {
	.get_type = UK_STORE_NONE
};

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section
 */
#define __UK_STORE_ENTRY_REG(entry)				\
	__attribute((__section__(".uk_store_libs_list")))	\
	static struct uk_store_entry __ptr_##entry __used = entry

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section
 */
#define __UK_STORE_ENTRY_REG_H(entry)				\
	__attribute((__section__(".uk_store_libs_list")))	\
	static struct uk_store_entry *__ptr_##entry __used = &entry

/**
 * Registers an entry in the section and initializes the tree structure.
 *
 * @param offset the offset where to register
 */
#define UK_STORE_STATIC_REGISTER_INIT(offset)				\
	do {								\
		struct uk_store_entry *to_reg =				\
			(uk_store_libs + (offset));			\
		to_reg->get_type = to_reg->set_type = UK_STORE_NONE;	\
		UK_TREE_ROOT_INIT(&to_reg->node);			\
		__UK_STORE_ENTRY_REG(uk_store_section_entry);		\
		__UK_STORE_ENTRY_REG_H(uk_store_section_head);		\
	} while (0)

/**
 * Registers an entry in the section.
 *
 * @param offset the offset where to register
 */
#define UK_STORE_STATIC_REGISTER(offset)			\
	do {							\
		struct uk_store_entry *to_reg =			\
			(uk_store_libs + (offset));		\
		__UK_STORE_ENTRY_REG(uk_store_section_entry);	\
		__UK_STORE_ENTRY_REG_H(uk_store_section_head);	\
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
	entry->get_type = entry->set_type = UK_STORE_NONE;
}

/**
 * Initializes an empty entry and adds it to the place next pointers
 *
 * @param place the place where to add the new entry
 * @param entry the entry to initialize and add
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_add_and_init_entry(struct uk_store_entry *place,
				struct uk_store_entry *entry)
{
	uk_store_init_entry(entry);
	return uk_tree_add_new(&place->node, &entry->node);
}

/**
 * Adds an entry to the place next pointers
 *
 * @param place the place where to add the new entry
 * @param entry the entry to add
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_add_entry(struct uk_store_entry *place, struct uk_store_entry *entry)
{
	return uk_tree_add_existing(&place->node, &entry->node);
}


/**
 * Removes the old entry and replaces it with the new one
 *
 * @param old the entry to remove
 * @param new the entry to add
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_repl_entry(struct uk_store_entry *old, struct uk_store_entry *new)
{
	old->get_type = old->set_type = UK_STORE_NONE;
	return uk_tree_replace(&old->node, &new->node);
}

/**
 * Deletes an entry
 *
 * @param entry the entry to delete
 * @return 0 on success or < 0 on fail
 */
static inline int
uk_store_del_entry(struct uk_store_entry *entry)
{
	entry->get_type = entry->set_type = UK_STORE_NONE;
	return uk_tree_del(&entry->node);
}

/**
 * Returns an entry after following the given path
 *
 * @param root the node where to start the search
 * @param path the path to follow
 * @param path_len the length of the path
 * @return 0 on success or < 0 on fail
 */
static inline struct uk_store_entry *
uk_store_get_entry_by_path(struct uk_store_entry *root,
			const uint16_t *path, const uint16_t path_len)
{
	struct uk_tree_node *node = uk_tree_find(&root->node, path, path_len);

	return node ? uk_tree_entry(node, struct uk_store_entry, node) : NULL;
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
uk_store_save_getter(struct uk_store_entry *entry,
			enum uk_store_entry_type type, void *func)
{
	entry->get_type = type;
	switch(entry->get_type) {
	case UK_STORE_ENT_BOOL:
		entry->get._bool = func;
		break;

	case UK_STORE_ENT_INT:
		entry->get._int = func;
		break;

	case UK_STORE_ENT_SHORT:
		entry->get._short = func;
		break;

	case UK_STORE_ENT_CHAR:
		entry->get._char = func;
		break;

	case UK_STORE_ENT_LONG:
		entry->get._long = func;
		break;

	case UK_STORE_ENT_LONGLONG:
		entry->get._longlong = func;
		break;

	case UK_STORE_ENT_FLOAT:
		entry->get._float = func;
		break;

	case UK_STORE_ENT_DOUBLE:
		entry->get._double = func;
		break;

	case UK_STORE_ENT_UINT:
		entry->get._uint = func;
		break;
	
	case UK_STORE_ENT_USHORT:
		entry->get._ushort = func;
		break;
	
	case UK_STORE_ENT_UCHAR:
		entry->get._uchar = func;
		break;
	
	case UK_STORE_ENT_ULONG:
		entry->get._ulong = func;
		break;
	
	case UK_STORE_ENT_ULONGLONG:
		entry->get._ulonglong = func;
		break;

	case UK_STORE_ENT_VOID:
		entry->get._void = func;
		break;

	case UK_STORE_ENT_VOIDSTAR:
		entry->get._voidstar = func;
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
uk_store_save_setter(struct uk_store_entry *entry,
			enum uk_store_entry_type type, void *func)
{
	entry->set_type = type;
	switch(entry->set_type) {
	case UK_STORE_ENT_BOOL:
		entry->set._bool = func;
		break;

	case UK_STORE_ENT_INT:
		entry->set._int = func;
		break;

	case UK_STORE_ENT_SHORT:
		entry->set._short = func;
		break;

	case UK_STORE_ENT_CHAR:
		entry->set._char = func;
		break;

	case UK_STORE_ENT_LONG:
		entry->set._long = func;
		break;

	case UK_STORE_ENT_LONGLONG:
		entry->set._longlong = func;
		break;

	case UK_STORE_ENT_FLOAT:
		entry->set._float = func;
		break;

	case UK_STORE_ENT_DOUBLE:
		entry->set._double = func;
		break;

	case UK_STORE_ENT_UINT:
		entry->set._uint = func;
		break;
	
	case UK_STORE_ENT_USHORT:
		entry->set._ushort = func;
		break;
	
	case UK_STORE_ENT_UCHAR:
		entry->set._uchar = func;
		break;
	
	case UK_STORE_ENT_ULONG:
		entry->set._ulong = func;
		break;
	
	case UK_STORE_ENT_ULONGLONG:
		entry->set._ulonglong = func;
		break;

	case UK_STORE_ENT_VOID:
		entry->set._void = func;
		break;

	case UK_STORE_ENT_VOIDSTAR:
		entry->set._voidstar = func;
		break;	
	default:
		return -EINVAL;
	}
	return 0;
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
	return uk_tree_is_leaf(&entry->node);
}

/**
 * Checks if an entry is a folder (not leaf)
 *
 * @param entry the entry to check
 * @return 1 if file, 0 if not, or < 0 on fail
 */
static inline int
uk_store_is_folder(struct uk_store_entry *entry)
{
	int ret = uk_store_is_file(entry);

	return (ret < 0) ? ret : !ret;
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
	switch(entry->get_type) {
	case UK_STORE_ENT_BOOL:
		*((_Bool *) out) = entry->get._bool();
		break;

	case UK_STORE_ENT_INT:
		*((int *) out) = entry->get._int();
		break;

	case UK_STORE_ENT_SHORT:
		*((short *) out) = entry->get._short();
		break;

	case UK_STORE_ENT_CHAR:
		*((char *) out) = entry->get._char();
		break;

	case UK_STORE_ENT_LONG:
		*((long *) out) = entry->get._long();
		break;

	case UK_STORE_ENT_LONGLONG:
		*((long long *) out) = entry->get._longlong();
		break;

	case UK_STORE_ENT_FLOAT:
		*((float *) out) = entry->get._float();
		break;

	case UK_STORE_ENT_DOUBLE:
		*((double *) out) = entry->get._double();
		break;

	case UK_STORE_ENT_UINT:
		*((unsigned int *) out) = entry->get._uint();
		break;
	
	case UK_STORE_ENT_USHORT:
		*((unsigned short *) out) = entry->get._ushort();
		break;	
	
	case UK_STORE_ENT_UCHAR:
		*((unsigned char *) out) = entry->get._uchar();
		break;	
	
	case UK_STORE_ENT_ULONG:
		*((unsigned long *) out) = entry->get._ulong();
		break;	
	
	case UK_STORE_ENT_ULONGLONG:
		*((unsigned long long *) out) = entry->get._ulonglong();
		break;	
	
	case UK_STORE_ENT_VOID:
		entry->get._void();
		break;

	case UK_STORE_ENT_VOIDSTAR:
		*((void  **) out) = entry->get._voidstar();
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
	switch(entry->set_type) {
	case UK_STORE_ENT_BOOL:
		entry->set._bool(*((_Bool *) in));
		break;

	case UK_STORE_ENT_INT:
		entry->set._int(*((int *) in));
		break;

	case UK_STORE_ENT_SHORT:
		entry->set._short(*((short *) in));
		break;

	case UK_STORE_ENT_CHAR:
		entry->set._char(*((char *) in));
		break;

	case UK_STORE_ENT_LONG:
		entry->set._long(*((long *) in));
		break;

	case UK_STORE_ENT_LONGLONG:
		entry->set._longlong(*((long long *) in));
		break;

	case UK_STORE_ENT_FLOAT:
		entry->set._float(*((float *) in));
		break;

	case UK_STORE_ENT_DOUBLE:
		entry->set._double(*((double *) in));
		break;

	case UK_STORE_ENT_UINT:
		entry->set._uint(*((unsigned int *) in));
		break;
	
	case UK_STORE_ENT_USHORT:
		entry->set._bool(*((unsigned short *) in));
		break;
	
	case UK_STORE_ENT_UCHAR:
		entry->set._uchar(*((unsigned char *) in));
		break;
	
	case UK_STORE_ENT_ULONG:
		entry->set._ulong(*((unsigned long *) in));
		break;
	
	case UK_STORE_ENT_ULONGLONG:
		entry->set._ulonglong(*((unsigned long long *) in));
		break;

	case UK_STORE_ENT_VOID:
		entry->set._void();
		break;

	case UK_STORE_ENT_VOIDSTAR:
		entry->set._voidstar(in);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#endif /* __STORE_INTERNAL_H__ */
