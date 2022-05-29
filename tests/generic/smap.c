// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#define GENERIC_IMPL_STATIC

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ds/fmt.h>

static bool malloc_fail = false;
static void *custom_malloc(size_t size) {
	return malloc_fail ? NULL : malloc(size);
}
#define malloc(size) custom_malloc(size)

static bool strdup_fail = false;
static void *custom_strdup(const char *str) {
	return strdup_fail ? NULL : strdup(str);
}
#define strdup(str) custom_strdup(str)

#define GENERIC_TYPE int
#define GENERIC_NAME IntMap
#define GENERIC_PREFIX int_map
#include <ds/generic/smap.h>

#define GENERIC_TYPE size_t
#define GENERIC_NAME SizeTMap
#define GENERIC_PREFIX size_t_map
#include <ds/generic/smap.h>

typedef struct Test {
	size_t n;
	const char *s;
	double d;
} Test;

#define GENERIC_TYPE Test
#define GENERIC_NAME TestMap
#define GENERIC_PREFIX test_map
#include <ds/generic/smap.h>

int main() {
	fmt_init();

	IntMap m = int_map();
	// Insert
	for (size_t i = 10; i < 80; i++) {
		char buf[64];
		snprintf(buf, 64, "number: %d", (int)i);
		ERROR_ASSERT(int_map_set(&m, buf, (int)i));
	}
	assert(int_map_del(m, "number: 15"));
	assert(!int_map_del(m, "number: 15"));
	// Get
	for (size_t i = 10; i < 80; i++) {
		char buf[64];
		snprintf(buf, 64, "number: %d", (int)i);
		int *p = int_map_get(m, buf);
		if (i != 15) {
			assert(p);
			assert(*p == (int)i);
		}
	}
	// Replace
	for (size_t i = 10; i < 80; i++) {
		char buf[64];
		snprintf(buf, 64, "number: %d", (int)i);
		ERROR_ASSERT(int_map_set(&m, buf, (int)i));
	}
	// Use iterators get every item
	IntMapItem *i = NULL;
	while (int_map_it_next(m, &i)) {
		assert(i->key != 0);
		assert(i->val != 0);
	}
	// Print using fmt
	int_map_fmt_register("%d");
	char buf[2048];
	fmts(buf, 2048, "%{IntMap}", m);
	assert(strcmp(buf, "{\"number: 64\": 64, \"number: 29\": 29, \"number: 46\": 46, \"number: 63\": 63, \"number: 20\": 20, \"number: 55\": 55, \"number: 74\": 74, \"number: 27\": 27, \"number: 52\": 52, \"number: 14\": 14, \"number: 30\": 30, \"number: 65\": 65, \"number: 47\": 47, \"number: 21\": 21, \"number: 54\": 54, \"number: 73\": 73, \"number: 17\": 17, \"number: 37\": 37, \"number: 66\": 66, \"number: 44\": 44, \"number: 22\": 22, \"number: 57\": 57, \"number: 11\": 11, \"number: 72\": 72, \"number: 16\": 16, \"number: 36\": 36, \"number: 42\": 42, \"number: 67\": 67, \"number: 59\": 59, \"number: 45\": 45, \"number: 23\": 23, \"number: 56\": 56, \"number: 10\": 10, \"number: 71\": 71, \"number: 48\": 48, \"number: 19\": 19, \"number: 35\": 35, \"number: 43\": 43, \"number: 60\": 60, \"number: 58\": 58, \"number: 77\": 77, \"number: 24\": 24, \"number: 51\": 51, \"number: 13\": 13, \"number: 70\": 70, \"number: 33\": 33, \"number: 49\": 49, \"number: 79\": 79, \"number: 18\": 18, \"number: 34\": 34, \"number: 40\": 40, \"number: 61\": 61, \"number: 76\": 76, \"number: 68\": 68, \"number: 39\": 39, \"number: 25\": 25, \"number: 50\": 50, \"number: 12\": 12, \"number: 32\": 32, \"number: 78\": 78, \"number: 28\": 28, \"number: 41\": 41, \"number: 62\": 62, \"number: 75\": 75, \"number: 69\": 69, \"number: 38\": 38, \"number: 26\": 26, \"number: 53\": 53, \"number: 15\": 15, \"number: 31\": 31}") == 0);
	int_map_term(m);

	SizeTMap sm = size_t_map();
	size_t_map_term(sm);

	TestMap tm = test_map();
	// Error recovery (malloc fail)
	malloc_fail = true;
	Error err = test_map_set(&tm, "a", (Test){ .n = 22 });
	assert(err.kind == ErrorOutOfMemory);
	assert(tm.len == 0);
	assert(tm.cap == 0);
	// Error recovery (strdup fail)
	malloc_fail = false;
	strdup_fail = true;
	err = test_map_set(&tm, "a", (Test){ .n = 22 });
	assert(err.kind == ErrorOutOfMemory);
	assert(tm.len == 0);
	assert(tm.cap == 8);

	test_map_term(tm);

	fmt_term();
}
