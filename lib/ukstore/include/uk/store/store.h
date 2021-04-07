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

// TODO fix types
// TODO fix register failing for more than 2 elements
// TODO add static/dynamic registering
// TODO add wrapper calls to tree

enum uk_store_entry_type {
	UK_STORE_INIT,
	UK_STORE_BOOL,
	UK_STORE_INT,
	UK_STORE_SHORT,
	UK_STORE_CHAR,
	UK_STORE_LONG,
	UK_STORE_LONGLONG,
	UK_STORE_FLOAT,
	UK_STORE_DOUBLE,
	UK_STORE_UINT,
	UK_STORE_USHORT,
	UK_STORE_UCHAR,
	UK_STORE_ULONG,
	UK_STORE_ULONGLONG,
	UK_STORE_UFLOAT,
	UK_STORE_UDOUBLE,
};

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

struct uk_store_entry {
	enum uk_store_entry_type type;
	union {
		uk_store_get_bool_func_t      get_bool;
		uk_store_get_int_func_t       get_int;
		uk_store_get_short_func_t     get_short;
		uk_store_get_char_func_t      get_char;
		uk_store_get_long_func_t      get_long;
		uk_store_get_longlong_func_t  get_longlong;
		uk_store_get_float_func_t     get_float;
		uk_store_get_double_func_t    get_double;
		uk_store_get_uint_func_t      get_uint;
		uk_store_get_ushort_func_t    get_ushort;
		uk_store_get_uchar_func_t     get_uchar;
		uk_store_get_ulong_func_t     get_ulong;
		uk_store_get_ulonglong_func_t get_ulonglong;

		uk_store_set_bool_func_t      set_bool;
		uk_store_set_int_func_t       set_int;
		uk_store_set_short_func_t     set_short;
		uk_store_set_char_func_t      set_char;
		uk_store_set_long_func_t      set_long;
		uk_store_set_longlong_func_t  set_longlong;
		uk_store_set_float_func_t     set_float;
		uk_store_set_double_func_t    set_double;
		uk_store_set_uint_func_t      set_uint;
		uk_store_set_ushort_func_t    set_ushort;
		uk_store_set_uchar_func_t     set_uchar;
		uk_store_set_ulong_func_t     set_ulong;
		uk_store_set_ulonglong_func_t set_ulonglong;
	} f;
	struct uk_tree_node node;
};

extern struct uk_store_entry *uk_store_libs;

/* Used for registration and nothing else */
static struct uk_store_entry uk_store_section_head __unused;

/* Used for registration and nothing else */
static const struct uk_store_entry test = {
	.node = UK_STORE_INIT
};

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section (defined int)
 */
#define UK_STORE_ENTRY_REGISTER(entry)				\
	__attribute((__section__(".uk_store_libs_list")))	\
	static struct uk_store_entry __ptr_##entry __used = entry

#define UK_STORE_ENTRY_REGISTERE(entry)				\
	__attribute((__section__(".uk_store_libs_list")))	\
	static struct uk_store_entry *__ptr_##entry __used = &entry

#define UK_STORE_REG(offset)					\
	do {							\
		struct uk_store_entry *to_reg =			\
			(uk_store_libs + (offset) *		\
			sizeof(struct uk_store_entry));		\
		printf("%d %p\n", to_reg->node.next_nodes_nr, to_reg);	\
		UK_STORE_ENTRY_REGISTER(test);			\
		UK_STORE_ENTRY_REGISTERE(uk_store_section_head);\
	} while (0)

		//UK_TREE_ROOT_INIT(&to_reg->node);
#endif /* __STORE_INTERNAL_H__ */
