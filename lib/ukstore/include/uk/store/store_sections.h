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

#ifndef __STORE_SECTIONS_H__
#define __STORE_SECTIONS_H__

#include <uk/config.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <uk/arch/lcpu.h>

/* This header includes only the needed empty calls */
#include <uk/store/store_empty.h>

#define _KVS_RD_SZ CONFIG_LIBUKSTORE_READERS_SIZE

/*
 * Each entry stores a pointer to the value. In it formatted in this manner to
 * ensure consistency with the hashtable version.
 */
struct _uk_store_value {
	void *value;
};

/* The buffer is allocated in the .uk_store_buffer_list section */
extern struct _uk_store_value *uk_store_buffer;

#ifdef CONFIG_LIBUKSTORE_CALLBACK

/*
 * Each entry stores a pointer to writer call back and an array of pointers
 * to all the registered reader callbacks. The size of the readers array should
 * be small, as the macros iterate thorugh it.
 */
struct uk_store_callbacks {
	void *writer;
	void *readers[_KVS_RD_SZ];
};

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

/*
 * Thread-safe equivalents of the callback functions. These Macros have the
 * same name format, but end with "_SAFE". These versions use an internal mutex
 * that needs to be initialized before usage.
 */
#ifdef CONFIG_LIBUKSTORE_THREAD_SAFE

#include <uk/mutex.h>

/*
 * Mutex used to keep operations thread-safe. Needs to be initialized first.
 */
extern struct uk_mutex uk_store_mutex;

/*
 * Call only once to initialize the mutex
 */
#define UK_STORE_MUTEX_INIT()	\
	uk_mutex_init(&uk_store_mutex)

#endif /* CONFIG_LIBUKSTORE_THREAD_SAFE */

/* Used for registration and nothing else */
static struct _uk_store_value uk_store_buffer_head __unused;

/**
 * Adds an entry to the section. Not to be called directly.
 *
 * @param entry the entry in the section (defined int)
 */
#define UK_STORE_BUFFER_REGISTER(entry)				\
	__attribute((__section__(".uk_store_buffer_list")))	\
	static struct _uk_store_value *__ptr_##entry __used = &entry

/**
 * Registers an entry in the section. The entry is allocated with malloc so it
 * can have any size. Keys need to be consecutive.
 *
 * @param offset space from the head where to put the entry (defined int)
 * @param type the type of the stored value
 */
#define UK_STORE_REG(offset, type)					\
	do {								\
		struct _uk_store_value *to_reg =			\
			(uk_store_buffer + (offset) *			\
			sizeof(struct _uk_store_value));		\
									\
		to_reg->value = calloc(1, sizeof(type));		\
		if (!to_reg->value)					\
			break;						\
		UK_STORE_BUFFER_REGISTER(uk_store_buffer_head);	\
	} while (0)

/**
 * Frees the memory from a key.
 *
 * @param offset space from the head where the value is stored (defined int)
 */
static inline void _uk_store_del(int offset)
{
	struct _uk_store_value *to_free = (uk_store_buffer + (offset) *
					sizeof(struct _uk_store_value));

	if (likely(to_free && to_free->value))	{
		free(to_free->value);
		(uk_store_buffer + (offset) *
			sizeof(struct _uk_store_value))->value = NULL;
	}
}

/**
 * Frees the memory from a key.
 *
 * @param offset space from the head where the value is stored (defined int)
 */
#define UK_STORE_DEL(offset)	\
	_uk_store_del((offset))

/**
 * Copies the memory from the parameter to the entry.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_value the value to be copied (void *)
 * @param type the type of the value
 */
#define UK_STORE_SET_VALUE(offset, macro_value, type)		\
	do {							\
		struct _uk_store_value *entry =			\
			(uk_store_buffer + (offset) *		\
			sizeof(struct _uk_store_value));	\
								\
		if (likely(entry && entry->value)) {		\
			memcpy(entry->value, (macro_value),	\
				sizeof(type));			\
			break;					\
		}						\
		errno = EINVAL;					\
	} while (0)

/**
 * Returns the pointer to the value. Not to be called directly.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @return pointer to the value copied (void *)
 */
static inline void *_uk_store_get_value(int offset)
{
	struct _uk_store_value *to_ret = (uk_store_buffer + offset *
					sizeof(struct _uk_store_value));

	if (unlikely(!to_ret || !to_ret->value)) {
		errno = EINVAL;
		return NULL;
	}

	return to_ret->value;
}

/**
 * Returns the value from the section.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param ret the value copied (void *)
 * @param type the type of the value
 */
#define UK_STORE_GET_VALUE(offset, ret, type)			\
	do {							\
		type *temp_value;				\
								\
		temp_value = _uk_store_get_value((offset));	\
								\
		if (unlikely(!temp_value))			\
			break;					\
								\
		memcpy((ret), temp_value, sizeof(type));	\
	} while (0)

#ifdef CONFIG_LIBUKSTORE_CALLBACK

/**
 * Registers a new writer for the given offset. Not to be called directly.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val the function pointer to be stored (void *)
 */
static inline void _uk_store_new_callback_writer(int offset, void *macro_val)
{
	struct uk_store_callbacks *callbacks = _uk_store_get_value(offset);

	if (unlikely(!callbacks))
		return;

	callbacks->writer = macro_val;
}

/**
 * Deletes the pointer to the writer. Not to be called directly.
 *
 * @param offset space from the head where the value is stored (defined int)
 */
static inline void _uk_store_del_callback_writer(int offset)
{
	struct uk_store_callbacks *callbacks = _uk_store_get_value(offset);

	if (unlikely(!callbacks))
		return;

	callbacks->writer = NULL;
	for (int reader = 0; reader < _KVS_RD_SZ; ++reader)
		callbacks->readers[reader] = NULL;
}

/**
 * Registers a new reader for a writer. Not to be called directly.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val pointer to the reader function (void *)
 */
static inline void _uk_store_new_callback_reader(int offset, void *macro_val)
{
	struct uk_store_callbacks *callbacks = _uk_store_get_value(offset);

	if (unlikely(!callbacks || !callbacks->writer))
		return;

	for (int reader = 0; reader < _KVS_RD_SZ; ++reader) {
		if (!callbacks->readers[reader]) {
			callbacks->readers[reader] = macro_val;
			return;
		}
	}
}

/**
 * Deletes a reader. Not to be called directly.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val pointer to the reader function (void *)
 */
static inline void _uk_store_del_callback_reader(int offset, void *macro_val)
{
	struct uk_store_callbacks *callbacks = _uk_store_get_value(offset);

	if (unlikely(!callbacks))
		return;

	for (int reader = 0; reader < _KVS_RD_SZ; ++reader) {
		if (callbacks->readers[reader] == macro_val) {
			callbacks->readers[reader] = NULL;
			return;
		}
	}
}

/**
 * Registers a new reader for a writer.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val pointer to the reader function (void *)
 */
#define UK_STORE_NEW_CALLBACK_READER(offset, macro_val)	\
	_uk_store_new_callback_reader((offset), (macro_val))

/**
 * Registers a new writer for the given offset.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val the function pointer to be stored (void *)
 */
#define UK_STORE_NEW_CALLBACK_WRITER(offset, macro_val)	\
	_uk_store_new_callback_writer((offset), (macro_val))

/**
 * Deletes a reader.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param macro_val pointer to the reader function (void *)
 */
#define UK_STORE_DEL_CALLBACK_READER(offset, macro_val)	\
	_uk_store_del_callback_reader((offset), (macro_val))

/**
 * Deletes the pointer to the writer.
 *
 * @param offset space from the head where the value is stored (defined int)
 */
#define UK_STORE_DEL_CALLBACK_WRITER(offset)	\
	_uk_store_del_callback_writer((offset))

/**
 * Calls the writer for the given offset and returns it through ret.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param ret the value returned by the writer (type)
 * @param type the type of the value returned by the writer
 */
#define UK_STORE_CALL_WRITER(offset, ret, type)		\
	do {							\
		struct uk_store_callbacks *callbacks;		\
								\
		callbacks = _uk_store_get_value((offset));	\
		if (unlikely(!callbacks || !callbacks->writer))	\
			break;					\
								\
		type (*writer_cb)(void) = callbacks->writer;	\
		ret = writer_cb();				\
	} while (0)
/**
 * Calls the readers from an offset with the param.
 *
 * @param offset space from the head where the value is stored (defined int)
 * @param param the value for the readers (type)
 * @param type the type of the parameter
 */
#define UK_STORE_CALL_READERS(offset, param, type)			\
	do {								\
		struct uk_store_callbacks *callbacks;			\
									\
		callbacks = _uk_store_get_value((offset));		\
		if (unlikely(!callbacks))				\
			break;						\
									\
		for (int reader = 0; reader < _KVS_RD_SZ; ++reader) {	\
			if (callbacks->readers[reader]) {		\
				void (*reader_cb)(type);		\
				reader_cb = callbacks->readers[reader];	\
				reader_cb(param);			\
			}						\
		}							\
	} while (0)

/**
 * This contracts the two calls (CALL_WRITER & CALL_READERS) into one
 * for ease of use.
 *
 * @param offset the offset in the buffer (defined int)
 * @param type the type of the value
 */
#define UK_STORE_START_CALLBACK(offset, type)			\
	do {							\
		type ret;					\
								\
		memset(&ret, 0, sizeof(type));			\
		UK_STORE_CALL_WRITER(offset, ret, type);	\
		UK_STORE_CALL_READERS(offset, ret, type);	\
	} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

/*
 * Thread-safe equivalents of the callback functions. These Macros have the
 * same name format, but end with "_safe". These versions use an internal mutex
 * that needs to be initialized before usage.
 */
#ifdef CONFIG_LIBUKSTORE_THREAD_SAFE

#define UK_STORE_REG_SAFE(offset, type)		\
	do {						\
		uk_mutex_lock(&uk_store_mutex);	\
		UK_STORE_REG(offset, type);		\
		uk_mutex_unlock(&uk_store_mutex);	\
	} while (0)

#define UK_STORE_DEL_SAFE(offset)			\
	do {						\
		uk_mutex_lock(&uk_store_mutex);	\
		UK_STORE_DEL(offset);			\
		uk_mutex_unlock(&uk_store_mutex);	\
	} while (0)

#define UK_STORE_SET_VALUE_SAFE(offset, macro_value, type)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_SET_VALUE(offset, macro_value, type);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_GET_VALUE_SAFE(offset, ret, type)		\
	do {							\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_GET_VALUE(offset, ret, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#ifdef CONFIG_LIBUKSTORE_CALLBACK

#define UK_STORE_NEW_CALLBACK_READER_SAFE(offset, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_NEW_CALLBACK_READER(offset, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_NEW_CALLBACK_WRITER_SAFE(offset, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_NEW_CALLBACK_WRITER(offset, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_DEL_CALLBACK_READER_SAFE(offset, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_DEL_CALLBACK_READER(offset, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_DEL_CALLBACK_WRITER_SAFE(offset, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_DEL_CALLBACK_WRITER(offset, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_CALL_WRITER_SAFE(offset, ret, type)		\
	do {							\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_CALL_WRITER(offset, ret, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#define UK_STORE_CALL_READERS_SAFE(offset, param, type)	\
	do {							\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_CALL_READERS(offset, param, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#define UK_STORE_START_CALLBACK_SAFE(offset, type)		\
	do {							\
		type ret;					\
								\
		memset(&ret, 0, sizeof(type));			\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_CALL_WRITER(offset, ret, type);	\
		UK_STORE_CALL_READERS(offset, ret, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

#endif /* CONFIG_LIBUKSTORE_THREAD_SAFE */

#endif /* __STORE_SECTIONS_H__ */
