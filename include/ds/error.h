// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#ifndef __DS_ERROR_H__
#define __DS_ERROR_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* It's pretty important that error messages get through, so we reserve 32kiB
 * which we can free if malloc fails. Then we should hopefully at least have
 * enough memory to allocate for the error message. */
#define ERROR_RESERVATION_SIZE 32768
extern void *error_reserved_for_error;

/* Some useful shortcuts for constructing errors. */

/* Essentially just a "no error" error. */
#define OK() (Error){ .kind = ErrorNone }
/* Use when malloc fails, for example. */
#define ERROR_OUT_OF_MEMORY()   (Error){ .kind = ErrorOutOfMemory }
/* Use for string constants ("this is a string constant"). */
#define ERROR_STRING(_str)      (Error){ .kind = ErrorString, .str = (char *)_str }
/* Use for heap-allocated strings (e.g. by malloc(), strdup() etc.). */
#define ERROR_HEAP_STRING(_str) (Error){ .kind = ErrorString, .str = _str, .str_on_heap = true }

/* Embed the current file and line information in the error message. */
#define ERROR_OUT_OF_MEMORY_HERE()   (Error){ .kind = ErrorOutOfMemory,                              .has_location = true, .file = __FILE__, .line = __LINE__ }
#define ERROR_STRING_HERE(_str)      (Error){ .kind = ErrorString, .str = (char *)_str,              .has_location = true, .file = __FILE__, .line = __LINE__ }
#define ERROR_HEAP_STRING_HERE(_str) (Error){ .kind = ErrorString, .str = _str, .str_on_heap = true, .has_location = true, .file = __FILE__, .line = __LINE__ }

/* Embed custom file and line information in the error message. */
#define ERROR_OUT_OF_MEMORY_LOCATION(_file, _line)     (Error){ .kind = ErrorOutOfMemory,                              .has_location = true, .file = _file, .line = _line }
#define ERROR_STRING_LOCATION(_file, _line, _str)      (Error){ .kind = ErrorString, .str = (char *)_str,               .has_location = true, .file = _file, .line = _line }
#define ERROR_HEAP_STRING_LOCATION(_file, _line, _str) (Error){ .kind = ErrorString, .str = _str, .str_on_heap = true, .has_location = true, .file = _file, .line = _line }

/* Append one error to another (will show as "error1: error2" in the error message). */
#define ERROR_NESTED(base, _annex)  (Error){ .kind = base.kind, .str = base.str, .has_annex = true, .annex = _error_heapify(_annex) }

/* Write the current file and line to an error and return it. */
#define ERROR_HEREIFY(_err) _error_hereify(__FILE__, __LINE__, _err)

/* Propagates any potential errors. Use the otherwise field for any uninitialization code. */
#define TRY(expr, otherwise) { Error _err = expr; if (_err.kind != ErrorNone) { otherwise; return _err; } }

/* If x is an error, this prints a nice error message and then exits. */
#define ERROR_ASSERT(x) /* use with moderation */ { \
	Error _err = x; \
	if (_err.kind != ErrorNone) { \
		if (!_err.has_location) { \
			_err.has_location = true; \
			_err.file = __FILE__; \
			_err.line = __LINE__; \
		} \
		char _buf[512]; error_to_string(_buf, 512, _err, true); \
		fprintf(stderr, "Fatal error: %s\n", _buf); \
		exit(1); \
	} \
}

typedef enum {
	ErrorNone = 0,
	ErrorOutOfMemory,
	ErrorString,
} ErrorKind;

typedef struct Error {
	ErrorKind kind;
	char *str;
	struct Error *annex;
	const char *file;
	size_t line;
	bool str_on_heap;
	bool has_annex;
	bool has_location;
} Error;

/* Returns true if the given error structure actually contains a real error. */
static inline bool error_is(Error e) { return e.kind != ErrorNone; }
/* Same as error_is but takes a pointer and does NULL checking. */
static inline bool error_ptr_is(Error *e) { return e != NULL && e->kind != ErrorNone; }

/* Call this at the beginning of any program that makes use of any of the more
 * advanced error features (those which involve heap allocation). */
void error_init();
/* Call this at the end of any program that used error_init() to clean up. */
void error_term();

/* Writes an error to a string. It is recommended to use ds/fmt.h in most cases, e.g.:
#include <ds/fmt.h>;
fmt("%{Error:destroy}", err);
*/
size_t error_to_string(char *buf, size_t size, Error e, bool destroy);

Error *_error_heapify(Error e);
Error _error_hereify(const char *file, size_t line, Error e);

#endif
