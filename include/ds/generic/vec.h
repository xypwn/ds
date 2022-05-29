// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

/* Example Usage:

// something.h:
#define GENERIC_TYPE int           // Key type
#define GENERIC_NAME IntVec        // Name of the resulting vector type
#define GENERIC_PREFIX int_vec     // Prefix for functions
#include "vec.h"

// something.c:
#define GENERIC_IMPL // We want something.c to define the actual function implementations
#include "something.h"

*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <ds/error.h>
#include <ds/fmt.h>

#define GENERIC_REQUIRE_TYPE
#include "../internal/generic/begin.h"

#ifndef _GENERIC_VEC_ONCE
#define _GENERIC_VEC_ONCE
size_t vec_len(const void *v);
size_t vec_cap(const void *v);
#endif

typedef TYPE *NAME;

VARDECL(const char *, __val_fmt);

FUNCDECL(NAME, )();
FUNCDECL(void, _term)(NAME v);
FUNCDECL(void, _fmt_register)(const char *val_fmt);
static inline FUNCDEF(size_t, _len)(const NAME v) { return vec_len((const void*)v); }
static inline FUNCDEF(size_t, _cap)(const NAME v) { return vec_cap((const void*)v); }
FUNCDECL(Error, _fit)(NAME *v, size_t new_minimum_cap);
FUNCDECL(Error, _push)(NAME *v, TYPE val);
FUNCDECL(TYPE, _pop)(NAME v);
FUNCDECL(TYPE *, _back)(NAME v);
FUNCDECL(TYPE, _del)(NAME v, size_t idx);
FUNCDECL(Error, _insert)(NAME *v, size_t idx, TYPE val);

#ifdef GENERIC_IMPL
VARDEF(const char *, __val_fmt) = NULL;

#define _VEC_HEADER(vec) ((_VecHeader*)(vec) - 1)

#ifndef _GENERIC_VEC_IMPL_ONCE
#define _GENERIC_VEC_IMPL_ONCE
typedef struct _VecHeader {
	size_t cap, len;
} _VecHeader;

size_t vec_len(const void *v) {
	return v == NULL ? 0 : _VEC_HEADER(v)->len;
}

size_t vec_cap(const void *v) {
	return v == NULL ? 0 : _VEC_HEADER(v)->cap;
}
#endif

static FUNCDEF(FmtPrintFuncRet, __print_func)(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	if (attrs != NULL) {
		if (attrs->len != 0)
			return FMT_PRINT_FUNC_RET_INVALID_ATTR(0);
	}
	NAME vec = va_arg(v, NAME);
	ctx->putc_func(ctx, '{');
	for (size_t i = 0; i < vec_len(vec); i++) {
		if (i != 0)
			fmtc(ctx, ", ");
		fmtc(ctx, VAR(__val_fmt), vec[i]);
	}
	ctx->putc_func(ctx, '}');
	return FMT_PRINT_FUNC_RET_OK();
}

FUNCDEF(NAME, )() {
	return NULL;
}

FUNCDEF(void, _term)(NAME v) {
	free(_VEC_HEADER(v));
}

FUNCDEF(void, _fmt_register)(const char *val_fmt) {
	VAR(__val_fmt) = val_fmt;
	fmt_register(NAME_STR, FUNC(__print_func));
}

FUNCDEF(Error, _fit)(NAME *v, size_t new_minimum_cap) {
	size_t new_cap;
	size_t len;
	_VecHeader *h;
	if (*v == NULL) {
		h = NULL;
		new_cap = new_minimum_cap;
		len = 0;
	} else {
		h = _VEC_HEADER(*v);
		new_cap = new_minimum_cap < h->len ? h->len : new_minimum_cap;
		len = h->len;
	}
	_VecHeader *new_h = realloc(h, sizeof(_VecHeader) + sizeof(TYPE) * new_cap);
	if (new_h == NULL)
		return ERROR_OUT_OF_MEMORY();
	new_h->len = len;
	new_h->cap = new_minimum_cap;
	*v = (NAME)(new_h + 1);
	return OK();
}

FUNCDEF(Error, _push)(NAME *v, TYPE val) {
	if (vec_len(*v) + 1 > vec_cap(*v))
		TRY(FUNC(_fit)(v, vec_cap(*v) == 0 ? 8 : vec_cap(*v) * 2), );
	(*v)[_VEC_HEADER(*v)->len++] = val;
	return OK();
}

FUNCDEF(TYPE, _pop)(NAME v) {
	return v[--_VEC_HEADER(v)->len];
}

FUNCDEF(TYPE *, _back)(NAME v) {
	return &v[vec_len(v) - 1];
}

FUNCDEF(TYPE, _del)(NAME v, size_t idx) {
	TYPE val = v[idx];
	memmove(v + idx, v + idx + 1, sizeof(TYPE) * (--_VEC_HEADER(v)->len - idx));
	return val;
}

FUNCDEF(Error, _insert)(NAME *v, size_t idx, TYPE val) {
	if (vec_len(*v) + 1 > vec_cap(*v))
		TRY(FUNC(_fit)(v, vec_cap(*v) == 0 ? 8 : vec_cap(*v) * 2), );
	memmove(*v + idx + 1, *v + idx, sizeof(TYPE) * (_VEC_HEADER(*v)->len++ - idx));
	(*v)[idx] = val;
	return OK();
}

#undef _VEC_HEADER

#endif

#include "../internal/generic/end.h"
