// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#include <stdbool.h>
#include <stddef.h>

#include <ds/error.h>
#include <ds/fmt.h>
#include <ds/string.h>

/* Example Usage:

// something.h:
#define GENERIC_TYPE int              // Value type
#define GENERIC_NAME StringIntMap     // Name of the resulting map type
#define GENERIC_PREFIX string_int_map // Prefix for functions
#include "smap.h"

// something.c:
#define GENERIC_IMPL // We want something.c to define the actual function implementations
#include "something.h"

*/

#define GENERIC_REQUIRE_TYPE
#include "../internal/generic/begin.h"
#define ITEM_TYPE GENERIC_CONCAT(NAME, Item)
#define TOMBSTONE ((char*)UINTPTR_MAX)

typedef struct ITEM_TYPE {
	char *key;
	TYPE val;
} ITEM_TYPE;

typedef struct NAME {
	ITEM_TYPE *data;
	size_t cap, len;
} NAME;

VARDECL(const char *, __val_fmt);

FUNCDECL(NAME, )();
FUNCDECL(void, _term)(NAME m);
FUNCDECL(void, _fmt_register)(const char *val_fmt);
FUNCDECL(TYPE *, _get)(NAME m, const char *key);
FUNCDECL(Error, _set)(NAME *m, const char *key, TYPE val);
FUNCDECL(bool, _del)(NAME m, const char *key);
FUNCDECL(Error, _rehash)(NAME *m, size_t new_minimum_cap);
FUNCDECL(bool, _it_next)(NAME m, ITEM_TYPE **restrict it);

#ifdef GENERIC_IMPL
VARDEF(const char *, __val_fmt) = NULL;

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _GENERIC_MAP_IMPL_ONCE
#define _GENERIC_MAP_IMPL_ONCE
static size_t _pow_of_2_from_minimum(size_t n) {
	n--; /* we want to handle the case of n already being a power of 2 */
	/* We use the leftmost bit set to 1 to also set any bits to its right
	 * to 1. Then we just increment to carry and get our desired power of 2. */
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
#if INTPTR_MAX == INT64_MAX /* only do the last shift on 64-bit systems */
	n |= n >> 32;
#endif
	return n + 1;
}

static uint32_t _fnv1a32(const void *data, size_t n) {
	uint32_t res = 2166136261u;
	for (size_t i = 0; i < n; i++) {
		res ^= ((uint8_t*)data)[i];
		res *= 16777619u;
	}
	return res;
}
#endif

static FUNCDEF(FmtPrintFuncRet, __print_func)(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	if (attrs != NULL) {
		if (attrs->len != 0)
			return FMT_PRINT_FUNC_RET_INVALID_ATTR(0);
	}
	NAME m = va_arg(v, NAME);
	ctx->putc_func(ctx, '{');
	ITEM_TYPE *it = NULL;
	bool first = true;
	while (FUNC(_it_next)(m, &it)) {
		if (!first)
			fmtc(ctx, ", ");
		fmtc(ctx, "\"%s\": ", it->key);
		fmtc(ctx, VAR(__val_fmt), it->val);
		first = false;
	}
	ctx->putc_func(ctx, '}');
	return FMT_PRINT_FUNC_RET_OK();
}

FUNCDEF(NAME, )() {
	return (NAME){0};
}

FUNCDEF(void, _term)(NAME m) {
	ITEM_TYPE *it = NULL;
	while (FUNC(_it_next)(m, &it)) {
		GENERIC_TERM_ITEM((it->val));
		free(it->key);
	}
	free(m.data);
}

FUNCDEF(void, _fmt_register)(const char *val_fmt) {
	VAR(__val_fmt) = val_fmt;
	fmt_register(NAME_STR, FUNC(__print_func));
}

FUNCDEF(TYPE *, _get)(NAME m, const char *key) {
	size_t i = _fnv1a32(key, strlen(key)) & (m.cap - 1);
	while (m.data[i].key) {
		if (m.data[i].key != TOMBSTONE && strcmp(m.data[i].key, key) == 0)
			return &m.data[i].val;
		i = (i + 1) % m.cap;
	}
	return NULL;
}

FUNCDEF(Error, _set)(NAME *m, const char *key, TYPE val) {
	if (m->cap == 0 || (float)m->len / (float)m->cap > 0.7f)
		TRY(FUNC(_rehash)(m, m->cap == 0 ? 8 : m->cap * 2), );

	size_t i = _fnv1a32(key, strlen(key)) & (m->cap - 1);
	while (m->data[i].key) {
		if (m->data[i].key == TOMBSTONE) {
			char *new_key = strdup(key);
			if (new_key == NULL)
				return ERROR_OUT_OF_MEMORY();
			m->data[i].key = new_key;
			m->data[i].val = val;
			return OK();
		} else if (strcmp(m->data[i].key, key) == 0) {
			m->data[i].val = val;
			return OK();
		}
		i = (i + 1) % m->cap;
	}
	char *new_key = strdup(key);
	if (new_key == NULL)
		return ERROR_OUT_OF_MEMORY();
	m->data[i].key = new_key;
	m->data[i].val = val;
	m->len++;
	return OK();
}

FUNCDEF(bool, _del)(NAME m, const char *key) {
	size_t i = _fnv1a32(key, strlen(key)) & (m.cap - 1);
	while (m.data[i].key) {
		if (m.data[i].key != TOMBSTONE && strcmp(m.data[i].key, key) == 0) {
			GENERIC_TERM_ITEM((m.data[i].val));
			free(m.data[i].key);
			m.data[i].key = TOMBSTONE;
			return true;
		}
		i = (i + 1) % m.cap;
	}
	return false;
}

FUNCDEF(Error, _rehash)(NAME *m, size_t new_minimum_cap) {
	size_t new_cap = _pow_of_2_from_minimum(new_minimum_cap > m->len ? new_minimum_cap : m->len);
	NAME new_m = {
		.data = malloc(sizeof(ITEM_TYPE) * new_cap),
		.cap = new_cap,
		.len = 0,
	};
	if (new_m.data == NULL)
		return ERROR_OUT_OF_MEMORY();
	for (size_t i = 0; i < new_m.cap; i++)
		new_m.data[i].key = NULL;

	for (size_t i = 0; i < m->cap; i++) {
		if (m->data[i].key && m->data[i].key != TOMBSTONE) {
			size_t j = _fnv1a32(m->data[i].key, strlen(m->data[i].key)) & (new_m.cap - 1);
			while (new_m.data[j].key) { j = (j + 1) % new_m.cap; }
			new_m.data[j].key = m->data[i].key;
			new_m.data[j].val = m->data[i].val;
			new_m.len++;
		}
	}

	free(m->data);
	*m = new_m;
	return OK();
}

FUNCDEF(bool, _it_next)(NAME m, ITEM_TYPE **restrict it) {
	*it == NULL ? *it = m.data : (*it)++;
	while (*it < m.data + m.cap && (!(*it)->key || (*it)->key == TOMBSTONE)) { (*it)++; }
	return *it < m.data + m.cap;
}
#endif

#undef ITEM_TYPE
#undef TOMBSTONE

#include "../internal/generic/end.h"
