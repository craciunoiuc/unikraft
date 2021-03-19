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

#ifndef __STORE_HASH_H__
#define __STORE_HASH_H__

#include <uk/config.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <uk/arch/lcpu.h>

/* This header includes only the needed empty calls */
#include <uk/store/store_empty.h>

#define _KVS_BUF_SZ CONFIG_LIBUKSTORE_BUFFER_SIZE
#define _KVS_RD_SZ CONFIG_LIBUKSTORE_READERS_SIZE

/*
 * Each entry stores a pointer to the string key to check for hash collisions
 * and a value variable for actual storage using malloc/calloc. It also stores
 * a "is_deleted" variable to ensure correct searching.
 */
struct _uk_store_value {
	char *string_key;
	void *value;
	char is_deleted;
};

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
 * This buffer contains pointers to generic memory locations generated with
 * malloc/calloc on demand.
 */
extern struct _uk_store_value uk_store_buffer[_KVS_BUF_SZ];

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

#else

#define UK_STORE_MUTEX_INIT()	\
	do {} while (0)

#endif /* CONFIG_LIBUKSTORE_THREAD_SAFE */

/**
 * Hashes a string into a number.
 * Taken and adapted from http://www.cse.yorku.ca/~oz/hash.html
 *
 * @param str the string to be hashed
 * @return an integer between 0 and _KVS_BUF_SZ
 */
static inline unsigned int _uk_store_hash(const char *str)
{
	unsigned int hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return (hash % _KVS_BUF_SZ);
}

/**
 * Finds the key that matches the given key.
 *
 * @param macro_key the key searched for
 * @return a pointer to the value or NULL if it does not exist
 */
static inline struct _uk_store_value *_uk_store_find(const char *macro_key)
{
	unsigned int key_pos = _uk_store_hash(macro_key);
	unsigned int orig_pos = key_pos;
	struct _uk_store_value entry = uk_store_buffer[key_pos];
	char first_step = 1;

	if (entry.string_key && !strcmp(entry.string_key, macro_key))
		return &uk_store_buffer[key_pos];
	while (first_step || (key_pos != orig_pos &&
		(entry.is_deleted || (entry.string_key &&
		strcmp(entry.string_key, macro_key))))) {
		first_step = 0;
		key_pos = (key_pos + 1) % _KVS_BUF_SZ;
		entry = uk_store_buffer[key_pos];
	}
	if (unlikely(!first_step && (key_pos == orig_pos
			|| !entry.string_key))) {
		errno = EINVAL;
		return NULL;
	}
	return &uk_store_buffer[key_pos];
}

/**
 * Finds the first empty position in the buffer.
 *
 * @param macro_key the key to be added
 * @return the empty position index or -1 if full
 */
static inline int _uk_store_find_empty(const char *macro_key)
{
	int key_pos = _uk_store_hash(macro_key);
	int orig_pos = key_pos;
	char first_step = 1;

	if (!uk_store_buffer[key_pos].string_key)
		return key_pos;
	while (first_step || (key_pos != orig_pos &&
		uk_store_buffer[key_pos].string_key)) {
		first_step = 0;
		key_pos = (key_pos + 1) % _KVS_BUF_SZ;
	}
	if (unlikely(!first_step && key_pos == orig_pos)) {
		errno = EINVAL;
		return -1;
	}
	return key_pos;
}

/**
 * The function uses linear probing to find the desired key returns it;
 *
 * @param macro_key the key in the buffer
 * @return the value
 */
static inline void *_uk_store_get_value(const char *macro_key)
{
	struct _uk_store_value *found_val;

	found_val = _uk_store_find((macro_key));
	if (unlikely(!found_val))
		return NULL;
	return found_val->value;
}

/**
 * The function uses linear probing to find the key and frees the value at
 * that position. It does nothing if the key is not found or the value is NULL.
 *
 * @param macro_key the key to be deleted (const char *)
 */
static inline void _uk_store_del(const char *macro_key)
{
	struct _uk_store_value *entry = _uk_store_find(macro_key);

	if (unlikely(!entry))
		return;

	entry->string_key = NULL;
	entry->is_deleted = 1;

	if (likely(entry->value)) {
		free(entry->value);
		entry->value = NULL;
	}
}

/**
 * The macro uses linear probing to find the first free space and stores a new
 * entry using malloc/calloc. It does nothing if the buffer is full.
 *
 * @param macro_key the key to be added (const char *)
 * @param type the element type (data type)
 */
#define UK_STORE_REG(macro_key, type)					\
	do {								\
		int key_pos = _uk_store_find_empty((macro_key));	\
									\
		if (unlikely(key_pos < 0))				\
			break;						\
		uk_store_buffer[key_pos].string_key = macro_key;	\
		uk_store_buffer[key_pos].is_deleted = 0;		\
		if (uk_store_buffer[key_pos].value)			\
			free(uk_store_buffer[key_pos].value);		\
		uk_store_buffer[key_pos].value =			\
					calloc(1, sizeof(type));	\
	} while (0)

/**
 * The macro uses linear probing to find the desired key and frees the value at
 * that position. It does nothing if the key is not found or the value is NULL.
 *
 * @param macro_key the key to be deleted (const char *)
 */
#define UK_STORE_DEL(macro_key)	\
	_uk_store_del((macro_key))

/**
 * The macro uses linear probing to find the desired key and copies the new
 * information to that spot. It does nothing if the entry does not exist.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param macro_value the value to be set (type)
 * @param type the type of the pointer, used in casting
 */
#define UK_STORE_SET_VALUE(macro_key, macro_value, type)		\
	do {								\
		struct _uk_store_value *entry =				\
				_uk_store_find((macro_key));		\
									\
		if (likely(entry && entry->value)) {			\
			memcpy(entry->value, (macro_value),		\
				sizeof(type));				\
		}							\
	} while (0)

/**
 * The macro uses linear probing to find the desired key and copies to ret
 * the pointer to the value. ret is NULL if the key was not found.
 * This duplicates the value to ensure that the information is not modified.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param ret the returned value (type)
 * @param type the type of the pointer, used in copying
 */
#define UK_STORE_GET_VALUE(macro_key, ret, type)		\
	do {							\
		type *temp_value;				\
								\
		temp_value = _uk_store_get_value(macro_key);	\
		if (unlikely(!temp_value))			\
			break;					\
		memcpy((ret), temp_value, sizeof(type));	\
	} while (0)

#ifdef CONFIG_LIBUKSTORE_CALLBACK

/**
 * It finds takes the value at the given key and sets a reader in the array.
 *
 * @param macro_key the key where to put the reader
 * @param macro_val the reader function
 */
static inline void _uk_store_new_callback_reader(const char *macro_key,
							void *macro_val)
{
	struct uk_store_callbacks *callbacks;

	callbacks = _uk_store_get_value(macro_key);
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
 * It finds takes the value at the given key and sets the writer.
 *
 * @param macro_key the key where to put the writer
 * @param macro_val the writer function
 */
static inline void _uk_store_new_callback_writer(const char *macro_key,
							void *macro_val)
{
	struct uk_store_callbacks *callbacks = NULL;

	callbacks = _uk_store_get_value(macro_key);
	if (unlikely(!callbacks))
		return;

	callbacks->writer = macro_val;
}

/**
 * The function deletes the reader from the key.
 *
 * @param macro_key the key where to delete the reader from
 * @param macro_val the reader to be deleted
 */
static inline void _uk_store_del_callback_reader(const char *macro_key,
							void *macro_val)
{
	struct uk_store_callbacks *callbacks = NULL;

	callbacks = _uk_store_get_value(macro_key);
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
 * The function deletes the writer from the key.
 *
 * @param macro_key the key where to delete the writer from
 * @param macro_val the writer to be deleted
 */
static inline void _uk_store_del_callback_writer(const char *macro_key)
{
	struct uk_store_callbacks *callbacks = NULL;

	callbacks = _uk_store_get_value(macro_key);
	if (unlikely(!callbacks))
		return;

	callbacks->writer = NULL;
	for (int reader = 0; reader < _KVS_RD_SZ; ++reader)
		callbacks->readers[reader] = NULL;
}

/**
 * The macro stores the new callback at the given macro_key, if it was
 * registered beforehand.
 * Linear probing is done to find the first free position.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param macro_val the reader function pointer (void *)
 */
#define UK_STORE_NEW_CALLBACK_READER(macro_key, macro_val)	\
	_uk_store_new_callback_reader((macro_key), (macro_val))

/**
 * The macro stores the new callback at the given macro_key, if it was
 * registered beforehand.
 * Linear probing is done to find the first available position.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param macro_val the writer function pointer (void *)
 */
#define UK_STORE_NEW_CALLBACK_WRITER(macro_key, macro_val)	\
	_uk_store_new_callback_writer((macro_key), (macro_val))


/**
 * The macro deletes the value at the given key. It replaces the entry with
 * NULL in the array.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param macro_val the reader function to be deleted (void *)
 */
#define UK_STORE_DEL_CALLBACK_READER(macro_key, macro_val)	\
	_uk_store_del_callback_reader((macro_key), (macro_val))

/**
 * The macro deletes the value at the given key. It replaces the writer with
 * NULL and the Readers, as there can't be a reader without a writer.
 *
 * @param macro_key - the key in the buffer (const char *)
 */
#define UK_STORE_DEL_CALLBACK_WRITER(macro_key)	\
	_uk_store_del_callback_writer((macro_key))

/**
 * The macro find the key in the buffer and calls the writer callback, giving
 * it's return value to the ret parameter.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param ret the value returned by the writer callback (type)
 * @param type the type of the return
 */
#define UK_STORE_CALL_WRITER(macro_key, ret, type)		\
	do {							\
		struct uk_store_callbacks *callbacks;		\
								\
		callbacks = _uk_store_get_value(macro_key);	\
		if (unlikely(!callbacks || !callbacks->writer))	\
			break;					\
								\
		type (*writer_cb)(void) = callbacks->writer;	\
		ret = writer_cb();				\
	} while (0)

/**
 * The macro find the key in the buffer and calls each reader callback, with
 * the parameters in param.
 *
 * @param macro_key the key in the buffer (const char *)
 * @param param the parameter to be given to the reader callback (type)
 * @param type the type of the parameter
 */
#define UK_STORE_CALL_READERS(macro_key, param, type)			\
	do {								\
		struct uk_store_callbacks *callbacks;			\
									\
		callbacks = _uk_store_get_value(macro_key);		\
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
 * @param macro_key the key in the buffer (const char *)
 * @param type the type of the value
 */
#define UK_STORE_START_CALLBACK(macro_key, type)		\
	do {							\
		type ret;					\
								\
		memset(&ret, 0, sizeof(type));			\
		UK_STORE_CALL_WRITER(macro_key, ret, type);	\
		UK_STORE_CALL_READERS(macro_key, ret, type);	\
	} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

/*
 * Thread-safe equivalents of the callback functions. These Macros have the
 * same name format, but end with "_safe". These versions use an internal mutex
 * that needs to be initialized before usage.
 */
#ifdef CONFIG_LIBUKSTORE_THREAD_SAFE

#define UK_STORE_REG_SAFE(macro_key, type)		\
	do {						\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_REG(macro_key, type);		\
		uk_mutex_unlock(&uk_store_mutex);	\
	} while (0)

#define UK_STORE_DEL_SAFE(macro_key)			\
	do {						\
		uk_mutex_lock(&uk_store_mutex);		\
		UK_STORE_DEL(macro_key);		\
		uk_mutex_unlock(&uk_store_mutex);	\
	} while (0)

#define UK_STORE_SET_VALUE_SAFE(macro_key, macro_value, type)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_SET_VALUE(macro_key, macro_value, type);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_GET_VALUE_SAFE(macro_key, ret, type)		\
	do {							\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_GET_VALUE(macro_key, ret, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#ifdef CONFIG_LIBUKSTORE_CALLBACK

#define UK_STORE_NEW_CALLBACK_READER_SAFE(macro_key, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_NEW_CALLBACK_READER(macro_key, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_NEW_CALLBACK_WRITER_SAFE(macro_key, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_NEW_CALLBACK_WRITER(macro_key, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_DEL_CALLBACK_READER_SAFE(macro_key, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_DEL_CALLBACK_READER(macro_key, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_DEL_CALLBACK_WRITER_SAFE(macro_key, macro_val)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_DEL_CALLBACK_WRITER(macro_key, macro_val);	\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_CALL_WRITER_SAFE(macro_key, ret, type)		\
	do {							\
		uk_mutex_lock(&uk_store_mutex);			\
		UK_STORE_CALL_WRITER(macro_key, ret, type);	\
		uk_mutex_unlock(&uk_store_mutex);		\
	} while (0)

#define UK_STORE_CALL_READERS_SAFE(macro_key, param, type)		\
	do {								\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_CALL_READERS(macro_key, param, type);		\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#define UK_STORE_START_CALLBACK_SAFE(macro_key, type)			\
	do {								\
		type ret;						\
									\
		memset(&ret, 0, sizeof(type));				\
		uk_mutex_lock(&uk_store_mutex);				\
		UK_STORE_CALL_WRITER(macro_key, ret, type);		\
		UK_STORE_CALL_READERS(macro_key, ret, type);		\
		uk_mutex_unlock(&uk_store_mutex);			\
	} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

#endif /* CONFIG_LIBUKSTORE_THREAD_SAFE */

#endif /* __STORE_HASH_H__ */
