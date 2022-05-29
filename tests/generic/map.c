// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#define GENERIC_IMPL_STATIC

#include <stdbool.h>
#include <stdlib.h>

#include <ds/fmt.h>

static bool malloc_fail = false;
static void *custom_malloc(size_t size) {
	return malloc_fail ? NULL : malloc(size);
}
#define malloc(size) custom_malloc(size)

#define GENERIC_KEY_TYPE int
#define GENERIC_VALUE_TYPE int
#define GENERIC_NAME IntIntMap
#define GENERIC_PREFIX int_int_map
#include <ds/generic/map.h>

#define GENERIC_KEY_TYPE int
#define GENERIC_VALUE_TYPE float
#define GENERIC_NAME IntFloatMap
#define GENERIC_PREFIX int_float_map
#include <ds/generic/map.h>

int main() {
	fmt_init();

	IntIntMap m = int_int_map();
	int_int_map_rehash(&m, 10);
	// Insert
	for (size_t i = 10; i < 80; i++)
		ERROR_ASSERT(int_int_map_set(&m, i, i + 20));
	assert(int_int_map_del(m, 15));
	assert(!int_int_map_del(m, 15));
	// Get
	for (size_t i = 10; i < 80; i++) {
		int *p = int_int_map_get(m, i);
		if (i == 15) {
			assert(p == NULL);
		} else {
			assert(p != NULL);
			assert(*p == i + 20);
		}
	}
	// Invalid rehash
	int_int_map_rehash(&m, 2);
	// Replace
	for (size_t i = 10; i < 80; i++)
		ERROR_ASSERT(int_int_map_set(&m, i, i + 20));
	// Use iterators to access every item
	IntIntMapItem *i = NULL;
	while (int_int_map_it_next(m, &i)) {
		assert(i->key != 0);
		assert(i->val != 0);
	}
	// Print using fmt
	int_int_map_fmt_register("%d", "%d");
	char buf[2048];
	fmts(buf, 2048, "%{IntIntMap}", m);
	assert(strcmp(buf, "{69: 89, 52: 72, 39: 59, 22: 42, 77: 97, 60: 80, 47: 67, 30: 50, 68: 88, 55: 75, 38: 58, 17: 37, 76: 96, 63: 83, 46: 66, 25: 45, 71: 91, 54: 74, 33: 53, 16: 36, 79: 99, 62: 82, 41: 61, 24: 44, 11: 31, 70: 90, 49: 69, 32: 52, 19: 39, 78: 98, 57: 77, 40: 60, 27: 47, 10: 30, 65: 85, 48: 68, 35: 55, 18: 38, 13: 33, 73: 93, 56: 76, 43: 63, 26: 46, 21: 41, 64: 84, 51: 71, 34: 54, 29: 49, 12: 32, 72: 92, 59: 79, 42: 62, 37: 57, 20: 40, 67: 87, 50: 70, 45: 65, 28: 48, 15: 35, 75: 95, 58: 78, 53: 73, 36: 56, 23: 43, 66: 86, 61: 81, 44: 64, 31: 51, 14: 34, 74: 94}") == 0);
	int_int_map_term(m);
	// Error recovery
	malloc_fail = true;
	IntFloatMap fm = int_float_map();
	Error err = int_float_map_set(&fm, 10, 10.f);
	assert(err.kind == ErrorOutOfMemory);
	assert(fm.len == 0);
	assert(fm.cap == 0);
	int_float_map_term(fm);

	fmt_term();
}
