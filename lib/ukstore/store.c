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

#include <uk/store/store.h>

void uk_store_set_value(struct uk_store_entry *entry, void *in)
{
	if (unlikely(!entry))
		return;

	switch (entry->type) {
	case UK_STORE_ENT_S32:
		entry->set.s32(*((__s32 *) in));
		break;

	case UK_STORE_ENT_S16:
		entry->set.s16(*((__s16 *) in));
		break;

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
		entry->set.uptr(*((__uptr *) in));
		break;
	default:
		break;
	}
}

void uk_store_get_value(struct uk_store_entry *entry, void *out)
{
	if (unlikely(!entry))
		return;

	switch (entry->type) {
	case UK_STORE_ENT_S32:
		*((__s32 *) out) = entry->get.s32();
		break;

	case UK_STORE_ENT_S16:
		*((__s16 *) out) = entry->get.s16();
		break;

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
		*((__uptr *) out) = entry->get.uptr();
		break;
	default:
		break;
	}
}