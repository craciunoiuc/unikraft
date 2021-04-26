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

#include <uk/store.h>
#include <errno.h>

// struct uk_store_folder *
// uk_store_get_folder(struct uk_store_folder **folders, const char *name)
// {
// 	struct uk_store_folder *fldr_start = *folders;
// 	struct uk_store_folder *fldr_end   = (struct uk_store_folder *) &uk_store_libs_end;

// 	// It's static
// 	if (fldr_start->folder_head.next == NULL) {
// 		for (; fldr_start != fldr_end; ++fldr_start) {
// 			if (!strcmp(fldr_start->folder_name, name)) {
// 				return 
// 			}
// 		}
// 	} else {
		
// 	}
// }

/**
 * Searches for an entry in a folder and returns it. Increases the refcount.
 *
 * @param folder the folder to search in
 * @param name the name of the entry to search for
 * @return the found entry or NULL
 */
struct uk_store_entry *
uk_store_get_entry(struct uk_store_folder *folder, const char *name)
{
	struct uk_store_folder_entry *res = NULL;

	uk_list_for_each_entry(res, &folder->entry_head, list_head)
		if (!strcmp(res->entry->entry_name, name))
			break;

	if (res) {
		ukarch_inc(&res->refcount.counter);
		return res->entry;
	}

	return NULL;
}

/**
 * Calls the saved setter with the value in `in`
 *
 * @param entry the entry to call from
 * @param in the value to give to the setter
 */
int
uk_store_set_value(struct uk_store_entry *entry, void *in)
{
	if (unlikely(!entry))
		return -EINVAL;

	switch (entry->type) {
	case s32:
		entry->set.s32(*((__s32 *) in));
		break;

	case s16:
		entry->set.s16(*((__s16 *) in));
		break;

	case s8:
		entry->set.s8(*((__s8 *) in));
		break;

	case s64:
		entry->set.s64(*((__s64 *) in));
		break;

	case u32:
		entry->set.u32(*((__u32 *) in));
		break;

	case u16:
		entry->set.u16(*((__u16 *) in));
		break;

	case u8:
		entry->set.u8(*((__u8 *) in));
		break;

	case u64:
		entry->set.u64(*((__u64 *) in));
		break;

	case uptr:
		entry->set.uptr(*((__uptr *) in));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * Calls the saved getter and saves the returned value in out
 *
 * @param entry the entry to call from
 * @param out the value to return
 */
int
uk_store_get_value(struct uk_store_entry *entry, void *out)
{
	if (unlikely(!entry))
		return -EINVAL;

	switch (entry->type) {
	case s32:
		*((__s32 *) out) = entry->get.s32();
		break;

	case s16:
		*((__s16 *) out) = entry->get.s16();
		break;

	case s8:
		*((__s8 *) out) = entry->get.s8();
		break;

	case s64:
		*((__s64 *) out) = entry->get.s64();
		break;

	case u32:
		*((__u32 *) out) = entry->get.u32();
		break;

	case u16:
		*((__u16 *) out) = entry->get.u16();
		break;

	case u8:
		*((__u8 *) out) = entry->get.u8();
		break;

	case u64:
		*((__u64 *) out) = entry->get.u64();
		break;

	case uptr:
		*((__uptr *) out) = entry->get.uptr();
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
