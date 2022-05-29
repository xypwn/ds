// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#include <ds/error.h>
#include <ds/fmt.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int iq;
static char iqstr[1024];

static Error no_error() {
	return OK();
}

static Error no_brain() {
	return ERROR_STRING("You do not have sufficient neural functionality to pass this test");
}

static Error no_brain_heap() {
	char *s = malloc(128);
	assert(s != NULL);
	snprintf(s, 128, "Insufficient IQ: %d (required: 55)", iq);
	return ERROR_HEAP_STRING(s);
}

static Error start_javascript_engine() {
	return ERROR_OUT_OF_MEMORY();
}

static Error nested() {
	return ERROR_NESTED(ERROR_STRING("Human error"), ERROR_NESTED(no_brain(), no_brain_heap()));
}

static Error idiot_takes_iq_test() {
	return nested();
}

static Error dennis_takes_iq_test() {
	return OK();
}

static Error try_to_take_the_iq_test(bool *failed) {
	*failed = false;
	TRY(idiot_takes_iq_test(), *failed = true);
	return OK();
}

static Error force_dennis_to_take_the_iq_test(bool *failed) {
	*failed = false;
	TRY(dennis_takes_iq_test(), *failed = true);
	return OK();
}

int main() {
	srand(time(NULL));
	iq = rand() % 55;
	snprintf(iqstr, 1024, "Insufficient IQ: %d (required: 55)", iq);

	fmt_init();
	error_init();

	Error e;
	char *s = malloc(2048);
	assert(s != NULL);

	e = no_error();
	assert(e.kind == ErrorNone);
	assert(error_to_string(s, 2048, e, true) == strlen("Success"));
	assert(strcmp(s, "Success") == 0);

	e = no_brain();
	assert(e.kind == ErrorString);
	assert(!e.str_on_heap);
	assert(error_to_string(s, 2048, e, true) == strlen("You do not have sufficient neural functionality to pass this test"));
	assert(strcmp(s, "You do not have sufficient neural functionality to pass this test") == 0);

	e = no_brain_heap();
	assert(e.kind == ErrorString);
	assert(e.str_on_heap);
	assert(error_to_string(s, 2048, e, true) == strlen(iqstr));
	assert(strcmp(s, iqstr) == 0);

	e = start_javascript_engine();
	assert(e.kind == ErrorOutOfMemory);
	assert(error_to_string(s, 2048, e, true) == strlen("Out of memory"));
	assert(strcmp(s, "Out of memory") == 0);

	snprintf(iqstr, 1024, "Human error: You do not have sufficient neural functionality to pass this test: Insufficient IQ: %d (required: 55)", iq);
	e = nested();
	assert(e.kind == ErrorString);
	assert(e.has_annex);
	assert(error_to_string(NULL, 0, e, false) == strlen(iqstr)); /* should still give us the right size, also shouldn't free/destroy anything */
	assert(error_to_string(s, 2048, e, true) == strlen(iqstr));
	assert(strcmp(s, iqstr) == 0);

	bool failed;
	e = try_to_take_the_iq_test(&failed);
	assert(e.kind == ErrorString);
	assert(failed);
	assert(error_to_string(s, 2048, e, true) == strlen(iqstr));
	assert(strcmp(s, iqstr) == 0);

	e = ERROR_HEREIFY(e);
	assert(e.kind == ErrorString);
	assert(e.has_location);
	assert(e.file != NULL);
	assert(e.line != 0);

	e = force_dennis_to_take_the_iq_test(&failed);
	assert(e.kind == ErrorNone);
	assert(!failed);
	assert(error_to_string(s, 2048, e, true) == strlen("Success"));
	assert(strcmp(s, "Success") == 0);

	e = nested();
	fmts(s, 2048, "%{Error}", e);
	assert(strcmp(s, iqstr) == 0);
	fmts(NULL, 0, "%{Error:destroy}", e);

	fmts(s, 2048, "%{Error:destroy}", nested());
	assert(strcmp(s, iqstr) == 0);

	free(s);

	error_term();
	fmt_term();
}
