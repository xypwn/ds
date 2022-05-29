// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#include <string.h>
#include <stdlib.h>

char *strdup(const char *s) {
	size_t len = strlen(s);
	char *res = malloc(len + 1);
	if(res == NULL)
		return NULL;
	memcpy(res, s, len+1);
	return res;
}
