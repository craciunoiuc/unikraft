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

#ifndef __STORE_EMPTY_H__
#define __STORE_EMPTY_H__

#include <uk/config.h>

#ifndef CONFIG_LIBUKSTORE

#define UK_STORE_REG(macro_key, type)	\
	do {} while (0)

#define UK_STORE_DEL(macro_key)	\
	do {} while (0)

#define UK_STORE_SET_VALUE(macro_key, macro_value, type)	\
	do {} while (0)

#define UK_STORE_GET_VALUE(macro_key, ret, type)	\
	do {} while (0)

#endif /* CONFIG_LIBUKSTORE */

#ifndef CONFIG_LIBUKSTORE_CALLBACK

struct uk_store_callbacks {
};

#define UK_STORE_NEW_CALLBACK_READER(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_NEW_CALLBACK_WRITER(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_DEL_CALLBACK_READER(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_DEL_CALLBACK_WRITER(offset)	\
	do {} while (0)

#define UK_STORE_CALL_WRITER(offset, ret, type)	\
	do {} while (0)

#define UK_STORE_CALL_READERS(offset, param, type)	\
	do {} while (0)

#define UK_STORE_START_CALLBACK(offset, type)	\
	do {} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

#ifndef CONFIG_LIBUKSTORE_THREAD_SAFE

#define UK_STORE_MUTEX_INIT()	\
	do {} while (0)

#define UK_STORE_REG_SAFE(offset, type)	\
	do {} while (0)

#define UK_STORE_DEL_SAFE(offset)	\
	do {} while (0)

#define UK_STORE_SET_VALUE_SAFE(offset, macro_value, type)	\
	do {} while (0)

#define UK_STORE_GET_VALUE_SAFE(offset, ret, type)	\
	do {} while (0)

#ifndef CONFIG_LIBUKSTORE_CALLBACK

#define UK_STORE_NEW_CALLBACK_READER_SAFE(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_NEW_CALLBACK_WRITER_SAFE(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_DEL_CALLBACK_READER_SAFE(offset, macro_val)	\
	do {} while (0)

#define UK_STORE_DEL_CALLBACK_WRITER_SAFE(offset)	\
	do {} while (0)

#define UK_STORE_CALL_WRITER_SAFE(offset, ret, type)	\
	do {} while (0)

#define UK_STORE_CALL_READERS_SAFE(offset, param, type)	\
	do {} while (0)

#define UK_STORE_START_CALLBACK_SAFE(offset, type)	\
	do {} while (0)

#endif /* CONFIG_LIBUKSTORE_CALLBACK */

#endif /* CONFIG_LIBUKSTORE_THREAD_SAFE */

#endif /* __STORE_EMPTY_H__ */
