// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#ifndef __DS_FMT_H__
#define __DS_FMT_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <ds/error.h>

/* Limit definitions. */
#define FMT_MAX_ATTRS    8
#define FMT_MAX_ATTR_LEN 32 /* including null termination */

/* An FmtContext defines how and where a print function should write its output.
 *
 * putc_func is the fundamental function which "writes" a char in whatever
 * way the context requires.
 * ctx_data serves as input, output or both to putc_func.
 *
 * For example, in the fmt functions which write to string buffers, ctx_data
 * holds the buffer, its size and how many bytes were written so far. */
typedef struct FmtContext {
	void *ctx_data;
	void (*putc_func)(struct FmtContext *restrict ctx, char c);
} FmtContext;

/* FmtAttrs holds attributes that are passed like additional parameters to a
 * print function. Which attributes are valid is entirely dependent on the
 * print function and the type being printed.
 *
 * len is the number of attributes, names contains the attribute names and
 * vals contains the attribute values (default is -1 if no value is given).
 *
 * For example, when printing an integer, you could use: %{int:p=4,c='0'}, where
 * p is the width, the rest of which is filled in by c. Resulting in an integer
 * with a width of at least 4 characters padded by zeroes. Attributes can have
 * integer or char values, or no value at all (e.g. %{int:X} just sets the "X"
 * attribute with a default value of -1).
 *
 * When creating a custom print function, you have to iterate over the FmtAttrs
 * manually. See src/ds/fmt.c for reference examples.
 * */
typedef struct FmtAttrs {
	size_t len;
	char   names[FMT_MAX_ATTRS][FMT_MAX_ATTR_LEN];
	int    vals[FMT_MAX_ATTRS]; /* number/char value */
} FmtAttrs;

/* FmtPrintFuncRet is the return value of a print function. It can either
 * return no error or specify an unrecognized attribute name. You should
 * always use the macros FMT_PRINT_FUNC_RET_OK() and
 * FMT_PRINT_FUNC_RET_INVALID_ATTR() to construct this class in a return
 * statement. */
typedef struct FmtPrintFuncRet {
	bool invalid_attr;
	size_t invalid_attr_idx;
} FmtPrintFuncRet;

#define FMT_PRINT_FUNC_RET_OK()               (FmtPrintFuncRet){0}
#define FMT_PRINT_FUNC_RET_INVALID_ATTR(_idx) (FmtPrintFuncRet){ .invalid_attr = true, .invalid_attr_idx = _idx }

/* An FmtPrintFunc describes any function that is designed to print a specific
 * type so fmt functions can output it. */
typedef FmtPrintFuncRet (*FmtPrintFunc)(FmtContext *restrict ctx, FmtAttrs *restrict attrs, va_list v);

/* Add a formatter for your custom types. See src/ds/fmt.c for some examples. */
void fmt_register(const char *keyword, FmtPrintFunc print_func);

void _fmtv(const char *restrict file, size_t line, const char *restrict format, va_list args);
void _fmt(const char *restrict file, size_t line, const char *restrict format, ...);
size_t _fmtsv(const char *restrict file, size_t line, char *restrict buf, size_t size, const char *restrict format, va_list args);
size_t _fmts(const char *restrict file, size_t line, char *restrict buf, size_t size, const char *restrict format, ...);
void _fmtcv(const char *restrict file, size_t line, FmtContext *ctx, const char *restrict format, va_list args);
void _fmtc(const char *restrict file, size_t line, FmtContext *ctx, const char *restrict format, ...);

/* Format to stdout. */
#define fmtv(format, ...) _fmtv(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define fmt(format, ...) _fmt(__FILE__, __LINE__, format, ##__VA_ARGS__)
/* Format to char buffer. */
#define fmtsv(buf, size, format, ...) _fmtsv(__FILE__, __LINE__, buf, size, format, ##__VA_ARGS__)
#define fmts(buf, size, format, ...) _fmts(__FILE__, __LINE__, buf, size, format, ##__VA_ARGS__)
/* Format with custom context (e.g. for custom output functions). */
#define fmtcv(ctx, format, ...) _fmtcv(__FILE__, __LINE__, ctx, format, ##__VA_ARGS__)
#define fmtc(ctx, format, ...) _fmtc(__FILE__, __LINE__, ctx, format, ##__VA_ARGS__)

void fmt_init();
void fmt_term();

#endif
