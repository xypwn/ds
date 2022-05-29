// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#include <assert.h>
#include <string.h>

#include <ds/fmt.h>

int main() {
	fmt_init();
	char buf[128];
	size_t size = fmts(buf, 128, "%hhu, %s %c%c %{uint:X,p=1024,c='.'}", ' ', "abc123", 'A', 65, 99999999);
	assert(size == 1038);
	assert(strcmp(buf, "32, abc123 AA .................................................................................................................") == 0);
	size = fmts(NULL, 0, "%%");
	assert(size == 1);
	fmt_term();
}
