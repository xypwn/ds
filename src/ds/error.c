// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#include <ds/error.h>

void *error_reserved_for_error = NULL;

void error_init() {
	if (error_reserved_for_error != NULL) {
		fprintf(stderr, "error: error_init() can only be called once.\n");
		exit(1);
	}
	error_reserved_for_error = malloc(ERROR_RESERVATION_SIZE);
}

void error_term() {
	free(error_reserved_for_error);
}

size_t error_to_string(char *buf, size_t size, Error e, bool destroy) {
	size_t written = 0;
	if (e.has_location) {
		size_t left = size > written ? size - written : 0;
		written += snprintf(buf + written, left, "%s:%zu: ", e.file, e.line);
	}
	switch (e.kind) {
		case ErrorNone: {
				size_t left = size > written ? size - written : 0;
				written += snprintf(buf + written, left, "Success");
			}
			break;
		case ErrorOutOfMemory: {
				size_t left = size > written ? size - written : 0;
				written += snprintf(buf + written, left, "Out of memory");
			}
			break;
		case ErrorString: {
				size_t left = size > written ? size - written : 0;
				written += snprintf(buf + written, left, "%s", e.str);
				if (e.str_on_heap && destroy)
					free(e.str);
			}
			break;
	}
	if (e.has_annex) {
		{
			size_t left = size > written ? size - written : 0;
			written += snprintf(buf + written, left, ": ");
		}
		{
			size_t left = size > written ? size - written : 0;
			written += error_to_string(buf + written, left, *e.annex, destroy);
		}
		if (destroy)
			free(e.annex);
	}
	return written;
}

Error *_error_heapify(Error e) {
	if (error_reserved_for_error == NULL) {
		fprintf(stderr, "error: error_init() must be called at the beginning of the program before working with heap-allocated errors.\n");
		return NULL;
	}
	Error *res = malloc(sizeof(*res));
	if (res == NULL) {
		free(error_reserved_for_error);
		error_reserved_for_error = NULL;
		res = malloc(sizeof(*res));
		if (res == NULL) {
			/* we should now have enough memory, if not we're seriously fucked */
			fprintf(stderr, "error: Could not obtain enough memory to construct the appropriate error.");
			return NULL;
		}
	}
	*res = e;
	return res;
}

Error _error_hereify(const char *file, size_t line, Error e) {
	e.has_location = true;
	e.file = file;
	e.line = line;
	return e;
}
