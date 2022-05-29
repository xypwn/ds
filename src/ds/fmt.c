// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#define GENERIC_IMPL_STATIC

#include <ds/fmt.h>
#include <ds/types.h>

#include <stdio.h>
#include <string.h>

#define GENERIC_TYPE   FmtPrintFunc
#define GENERIC_NAME   _PrintFuncMap
#define GENERIC_PREFIX _print_func_map
#include <ds/generic/smap.h>

/* Contains all builtin and custom formatting functions. */
static _PrintFuncMap _print_funcs;
/* Whether the fmt library was initialized. */
static bool initialized = false;

/********************************\
|*    Builtin fmt Functions     *|
\********************************/
static FmtPrintFuncRet _print_string_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	int pad = 0, padchar = ' ';
	if (attrs != NULL) {
		size_t i;
		for (i = 0; i < attrs->len; i++) {
			if (strcmp(attrs->names[i], "p") == 0) { 
				pad = attrs->vals[i];
			} else if (strcmp(attrs->names[i], "c") == 0) { 
				padchar = attrs->vals[i];
			} else {
				return FMT_PRINT_FUNC_RET_INVALID_ATTR(i);
			}
		}
	}
	const char *s = va_arg(v, const char *);
	if (s == NULL)
		s = "(null)";
	size_t len = strlen(s);
	if (pad > len) {
		size_t j;
		for (j = 0; j < pad - len; j++)
			ctx->putc_func(ctx, padchar);
	}
	while (*s != 0)
		ctx->putc_func(ctx, *s++);
	return FMT_PRINT_FUNC_RET_OK();
}

/* Helper function to print any integer. */
static FmtPrintFuncRet _print_integer(FmtContext *restrict ctx, FmtAttrs *restrict attrs, unsigned long long int val, bool negative) {
	int pad = 0, padchar = ' ', base = 10;
	bool uppercase = false;
	if (attrs != NULL) {
		size_t i;
		for (i = 0; i < attrs->len; i++) {
			if (strcmp(attrs->names[i], "X") == 0) {
				uppercase = true;
				base = 16;
			} else if (strcmp(attrs->names[i], "x") == 0) {
				uppercase = false;
				base = 16;
			} else if (strcmp(attrs->names[i], "o") == 0) { 
				base = 8;
			} else if (strcmp(attrs->names[i], "p") == 0) { 
				pad = attrs->vals[i];
			} else if (strcmp(attrs->names[i], "c") == 0) { 
				padchar = attrs->vals[i];
			} else if (strcmp(attrs->names[i], "b") == 0) { 
				base = attrs->vals[i];
			} else {
				return FMT_PRINT_FUNC_RET_INVALID_ATTR(i);
			}
		}
	}
	if (base < 2 || base > 16)
		base = 10;
	size_t i = 0;
	char buf[65]; /* max: 64-bit binary number with negative sign */
	if (val == 0)
		buf[i++] = '0';
	else {
		char a_begin = uppercase ? 'A' : 'a';
		while (val != 0) {
			unsigned long long int rem = val % base;
			buf[i++] = rem > 9 ? (rem - 10) + a_begin : rem + '0';
			val /= base;
		}
		if (negative)
			buf[i++] = '-';
	}
	if (pad > i) {
		size_t j;
		for (j = 0; j < pad - i; j++)
			ctx->putc_func(ctx, padchar);
	}
	while (i--)
		ctx->putc_func(ctx, buf[i]);
	return FMT_PRINT_FUNC_RET_OK();
}

static FmtPrintFuncRet _print_int_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	int val = va_arg(v, int);
	if (val < 0)
		return _print_integer(ctx, attrs, -val, true);
	else
		return _print_integer(ctx, attrs, val, false);
}

static FmtPrintFuncRet _print_lint_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	long int val = va_arg(v, long int);
	if (val < 0)
		return _print_integer(ctx, attrs, -val, true);
	else
		return _print_integer(ctx, attrs, val, false);
}

static FmtPrintFuncRet _print_llint_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	long long int val = va_arg(v, long long int);
	if (val < 0)
		return _print_integer(ctx, attrs, -val, true);
	else
		return _print_integer(ctx, attrs, val, false);
}

static FmtPrintFuncRet _print_uint_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	return _print_integer(ctx, attrs, va_arg(v, unsigned int), false);
}

static FmtPrintFuncRet _print_ulint_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	return _print_integer(ctx, attrs, va_arg(v, unsigned long int), false);
}

static FmtPrintFuncRet _print_ullint_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	return _print_integer(ctx, attrs, va_arg(v, unsigned long long int), false);
}

static FmtPrintFuncRet _print_size_t_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	return _print_integer(ctx, attrs, va_arg(v, size_t), false);
}

static FmtPrintFuncRet _print_ssize_t_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	return _print_integer(ctx, attrs, va_arg(v, ssize_t), false);
}

static void _print_error(FmtContext *restrict ctx, Error val, bool destroy) {
	if (val.has_location) {
		fmtc(ctx, "%s:%zu: ", val.file, val.line);
	}
	switch (val.kind) {
		case ErrorNone:
			fmtc(ctx, "Success");
			break;
		case ErrorOutOfMemory:
			fmtc(ctx, "Out of memory");
			break;
		case ErrorString:
			fmtc(ctx, "%s", val.str);
			if (val.str_on_heap && destroy)
				free(val.str);
			break;
	}
	if (val.has_annex) {
		fmtc(ctx, ": ");
		_print_error(ctx, *val.annex, destroy);
		if (destroy)
			free(val.annex);
	}
}

static FmtPrintFuncRet _print_error_func(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v) {
	bool destroy = false;
	if (attrs != NULL) {
		size_t i;
		for (i = 0; i < attrs->len; i++) {
			if (strcmp(attrs->names[i], "destroy") == 0) {
				destroy = true;
			} else {
				return FMT_PRINT_FUNC_RET_INVALID_ATTR(i);
			}
		}
	}
	Error val = va_arg(v, Error);
	_print_error(ctx, val, destroy);
	return FMT_PRINT_FUNC_RET_OK();
}

/********************************\
|*       Helper Functions       *|
\********************************/
typedef enum {
	UnclosedCharLiteral,
	End,
	Colon,
	Equals,
	Comma,
	Asterisk,
	String,
	Number,  /* also returned as a string, but guaranteed to be a valid number */
	CharLiteral, /* char literal encased in '' */
} _Token;

static _Token _next_token(const char **restrict fmt, char *restrict strbuf, size_t strbuf_size) {
	const char *c = *fmt;
	if (strbuf_size == 0)
		strbuf = NULL;
	switch (*c) {
		case '}':
		case 0:
			*fmt = ++c;
			return End;
		case ':':
			*fmt = ++c;
			return Colon;
		case '=':
			*fmt = ++c;
			return Equals;
		case ',':
			*fmt = ++c;
			return Comma;
		case '*':
			*fmt = ++c;
			return Asterisk;
		case '\'':
			c++;
			if (!(c[0] != 0 && c[1] == '\''))
				return UnclosedCharLiteral;
			else if (strbuf != NULL) {
				*strbuf++ = *c++;
				*strbuf++ = 0;
			}
			*fmt = ++c;
			return CharLiteral;
	}
	_Token ret = (*c == '-') || (*c >= '0' && *c <= '9') ? Number : String;
	for (size_t i = 0;; c++, i++) {
		if (*c == 0 || *c == '}' || *c == ':' || *c == '=' || *c == ',' || *c == '*')
			break;
		if (ret == Number && !(*c >= '0' && *c <= '9'))
			ret = String;
		if (strbuf != NULL && i < strbuf_size - 1)
			*strbuf++ = *c;
	}
	if (strbuf != NULL)
		*strbuf++ = 0;
	*fmt = c;
	return ret;
}

static void _fmtf_putc_func(FmtContext *restrict ctx, char c) {
	FILE *f = ctx->ctx_data;
	fputc(c, f);
}

typedef struct _FmtsContext {
	char *buf;
	size_t size, written;
} _FmtsContext;

static void _fmts_putc_func(FmtContext *restrict ctx, char c) {
	_FmtsContext *ctxs = ctx->ctx_data;
	if (ctxs->size > 0 && ctxs->written < ctxs->size - 1)
		ctxs->buf[ctxs->written] = c;
	ctxs->written++;
}


/********************************\
|* The guts of any fmt function *|
\********************************/
static Error _fmt_main(bool use_err_location, const char *file, size_t line, FmtContext *ctx, const char *restrict format, va_list args) {
	if (!initialized) {
		const char *err_str = "fmt: fmt_init() must be called at the beginning of the program before using any other fmt functions, and fmt_term() must be called when done\n";
		return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
	}
	const char *c = format;
	while (*c != 0) {
		if (*c == '%')  {
			c++;
			if (*c == '{') {
				FmtAttrs attrs = {0};
				c++;
				_Token t;
				char name[128];
				t = _next_token(&c, name, 128);
				if (t != String) {
					const char *err_str = "fmt: Expected valid type name after %{";
					return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
				}
				FmtPrintFunc *print_func = _print_func_map_get(_print_funcs, name);
				if (print_func == NULL) {
					char errbuf[256];
					snprintf(errbuf, 256, "fmt: Unrecognized type name: '%s'", name);
					return use_err_location ? ERROR_HEAP_STRING_LOCATION(file, line, strdup(errbuf)) : ERROR_HEAP_STRING(strdup(errbuf));
				}
				t = _next_token(&c, NULL, 0);
				if (t == Colon) {
					while (attrs.len < FMT_MAX_ATTRS) {
						t = _next_token(&c, attrs.names[attrs.len], FMT_MAX_ATTR_LEN);
						if (t != String) {
							const char *err_str = "fmt: Expected valid attribute name after %{<type>:";
							return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
						}
						t = _next_token(&c, NULL, 0);
						if (t == Equals) {
							char num[64];
							t = _next_token(&c, num, 64);
							if (t != Number && t != CharLiteral && t != Asterisk) {
								if (t == UnclosedCharLiteral) {
									const char *err_str = "fmt: Char literal after %{<type>:<attr>= unclosed or containing more than one character";
									return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
								} else {
									const char *err_str = "fmt: Expected number, char literal or asterisk after %{<type>:<attr>=";
									return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
								}
							}
							if (t == Number)
								attrs.vals[attrs.len] = atoi(num);
							else if (t == CharLiteral)
								attrs.vals[attrs.len] = num[0];
							else if (t == Asterisk)
								attrs.vals[attrs.len] = va_arg(args, int);
							t = _next_token(&c, NULL, 0);
						} else
							attrs.vals[attrs.len] = -1;
						attrs.len++;
						if (t == End)
							break;
						if (t != Comma) {
							const char *err_str = "fmt: Expected ',' or '}' after %{<type>:<attr>=<val>";
							return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
						}
					}
				}
				FmtPrintFuncRet res = (*print_func)(ctx, &attrs, args);
				if (res.invalid_attr) {
					char errbuf[256];
					snprintf(errbuf, 256, "fmt: Invalid attribute: '%s'", attrs.names[res.invalid_attr_idx]);
					return use_err_location ? ERROR_HEAP_STRING_LOCATION(file, line, strdup(errbuf)) : ERROR_HEAP_STRING(strdup(errbuf));
				}
				if (t != End) {
					const char *err_str = "fmt: Expected end";
					return use_err_location ? ERROR_STRING_LOCATION(file, line, err_str) : ERROR_STRING(err_str);
				}
			} else {
				enum {
					MOD_NONE,
					MOD_Z,
					MOD_L,
					MOD_LL,
					MOD_H,
					MOD_HH,
				};
				int mod = MOD_NONE;
				switch (*c) {
					case 'z':
						c++;
						mod = MOD_Z;
						break;
					case 'l':
						c++;
						if (*c == 'l') {
							c++;
							mod = MOD_LL;
						} else
							mod = MOD_L;
						break;
					case 'h':
						c++;
						if (*c == 'h') {
							c++;
							mod = MOD_HH;
						} else
							mod = MOD_H;
						break;
				}
				switch (*c) {
					case '%':
						c++;
						ctx->putc_func(ctx, '%');
						break;
					case 's': {
						c++;
						_print_string_func(ctx, NULL, args);
						break;
					}
					case 'c': {
						c++;
						ctx->putc_func(ctx, va_arg(args, int));
						break;
					}
					case 'i':
					case 'd':
						c++;
						switch (mod) {
							default:
							case MOD_H:
							case MOD_HH:
								_print_int_func(ctx, NULL, args);
								break;
							case MOD_Z:
								_print_ssize_t_func(ctx, NULL, args);
								break;
							case MOD_L:
								_print_lint_func(ctx, NULL, args);
								break;
							case MOD_LL:
								_print_llint_func(ctx, NULL, args);
								break;
						}
						break;
					case 'u':
						c++;
						switch (mod) {
							default:
							case MOD_H:
							case MOD_HH:
								_print_uint_func(ctx, NULL, args);
								break;
							case MOD_Z:
								_print_size_t_func(ctx, NULL, args);
								break;
							case MOD_L:
								_print_ulint_func(ctx, NULL, args);
								break;
							case MOD_LL:
								_print_ullint_func(ctx, NULL, args);
								break;
						}
						break;
					case 'x':
						c++;
						FmtAttrs custom_attrs = {
							.len   = 1,
							.names = { { 'x' } },
						};
						switch (mod) {
							default:
							case MOD_H:
							case MOD_HH:
								_print_uint_func(ctx, &custom_attrs, args);
								break;
							case MOD_Z:
								_print_size_t_func(ctx, &custom_attrs, args);
								break;
							case MOD_L:
								_print_ulint_func(ctx, &custom_attrs, args);
								break;
							case MOD_LL:
								_print_ullint_func(ctx, &custom_attrs, args);
								break;
						}
						break;
					case 'X': {
						c++;
						FmtAttrs custom_attrs = {
							.len   = 1,
							.names = { { 'X' } },
						};
						switch (mod) {
							default:
							case MOD_H:
							case MOD_HH:
								_print_uint_func(ctx, &custom_attrs, args);
								break;
							case MOD_Z:
								_print_size_t_func(ctx, &custom_attrs, args);
								break;
							case MOD_L:
								_print_ulint_func(ctx, &custom_attrs, args);
								break;
							case MOD_LL:
								_print_ullint_func(ctx, &custom_attrs, args);
								break;
						}
						break;
					}
					case 'p': {
						c++;
						ctx->putc_func(ctx, '0');
						ctx->putc_func(ctx, 'x');
						FmtAttrs custom_attrs = {
							.len   = 3,
							.names = { { 'p' }, { 'b' }, { 'c' } },
							.vals  = {    12,      16,     '0'   },
						};
						_print_size_t_func(ctx, &custom_attrs, args);
						break;
					}
				}
			}
		} else
			ctx->putc_func(ctx, *c++);
	}
	return OK();
}

/********************************\
|*       Public functions       *|
\********************************/
void fmt_register(const char *restrict keyword, FmtPrintFunc print_func) {
	_print_func_map_set(&_print_funcs, keyword, print_func);
}

void _fmtv(const char *file, size_t line, const char *restrict format, va_list args) {
	FmtContext ctx = {
		.ctx_data = stdout,
		.putc_func = _fmtf_putc_func,
	};
	_fmtcv(file, line, &ctx, format, args);
}

void _fmt(const char *file, size_t line, const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	_fmtv(file, line, format, args);
	va_end(args);
}

size_t _fmtsv(const char *restrict file, size_t line, char *restrict buf, size_t size, const char *restrict format, va_list args) {
	_FmtsContext ctxs = {
		.buf = buf,
		.size = size,
	};
	FmtContext ctx = {
		.ctx_data = &ctxs,
		.putc_func = _fmts_putc_func,
	};
	memset(buf, 0, size); /* TODO: Remove this without valgrind complaining. */
	_fmtcv(file, line, &ctx, format, args);
	if (size > 0)
		buf[ctxs.written] = 0;
	return ctxs.written;
}

size_t _fmts(const char *restrict file, size_t line, char *restrict buf, size_t size, const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	size_t res = _fmtsv(file, line, buf, size, format, args);
	va_end(args);
	return res;
}

void _fmtcv(const char *restrict file, size_t line, FmtContext *ctx, const char *restrict format, va_list args) {
	ERROR_ASSERT(_fmt_main(true, file, line, ctx, format, args));
}

void _fmtc(const char *restrict file, size_t line, FmtContext *ctx, const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	_fmtcv(file, line, ctx, format, args);
	va_end(args);
}

void fmt_init() {
	if (initialized) {
		ERROR_ASSERT(ERROR_STRING("fmt: fmt_init() can only be called once"));
	}
	initialized = true;
	_print_funcs = _print_func_map();

	fmt_register("str", _print_string_func);

	fmt_register("int", _print_int_func);

	fmt_register("unsigned int", _print_uint_func);
	fmt_register("uint", _print_uint_func);

	fmt_register("size_t", _print_size_t_func);
	fmt_register("ssize_t", _print_ssize_t_func);

	fmt_register("Error", _print_error_func);
}

void fmt_term() {
	_print_func_map_term(_print_funcs);
}
