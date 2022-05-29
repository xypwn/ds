################################
#            Config            #
################################
CFLAGS := -O2 -Wall -pedantic -Wno-gnu-zero-variadic-macro-arguments
#CFLAGS := -ggdb -Wall -pedantic -Wno-gnu-zero-variadic-macro-arguments
ifeq ($(OS),Windows_NT)
	EXE_EXT := .exe
else
	EXE_EXT :=
endif

################################
#           Library            #
################################
HDR := internal/generic/begin.h internal/generic/end.h generic/map.h generic/smap.h generic/vec.h error.h fmt.h types.h string.h
SRC := error.c fmt.c string.c

_HDR := $(addprefix include/ds/,$(HDR))
_SRC := $(addprefix src/ds/,$(SRC))
_OBJ := $(_SRC:.c=.o)

all: ds.a

ds.a: $(_OBJ)
	rm -f $@
	$(AR) rc $@ $^

src/%.o: src/%.c $(_HDR)
	$(CC) -c -o $@ $< -I./include $(CFLAGS)

################################
#           Testing            #
################################
define test_single
if ./$(1); then \
	echo "Passed test $(1)."; \
else \
	echo "Failed test $(1)!"; \
	exit 1; \
fi
endef

define test_single_leaks
printf "Checking for leaks in $(1)..."; \
if valgrind -q --error-exitcode=1 --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(1); then \
	echo "OK"; \
else \
	echo "Valgrind reported one or more errors!"; \
	exit 1; \
fi
endef

TEST_HDR := generic/vec.h
TESTS := generic/map generic/smap generic/vec error fmt

_TEST_HDR := $(addprefix tests/,$(TEST_HDR))
_TESTS := $(addsuffix $(EXE_EXT),$(addprefix tests/,$(TESTS)))

.PHONY: test test_outputs test_leaks test_full test_debug

test: $(_TESTS)
	@for i in $(_TESTS); do $(call test_single,$$i); done
	@echo "All tests passed."

test_leaks: $(_TESTS)
	@for i in $(_TESTS); do $(call test_single_leaks,$$i); done
	@echo "All tests passed."

test_full: clean $(_TESTS)
	@for i in $(_TESTS); do $(call test_single,$$i); done
	@for i in $(_TESTS); do $(call test_single_leaks,$$i); done
	@echo "All tests passed."

test_debug: tests/$(TEST_TO_DEBUG)
ifdef TEST_TO_DEBUG
	gdb -q --args ./tests/$(TEST_TO_DEBUG)
else
	@echo "Please run with TEST_TO_DEBUG=<test>."
	@echo "Options are: $(TESTS)"
endif

tests/%: tests/%.c ds.a $(_HDR) $(_TEST_HDR)
	$(CC) -o $@ $< ds.a -I./include $(CFLAGS) $(LDFLAGS)

################################
#           General            #
################################
.PHONY: clean

clean:
	rm -f ds.a $(_OBJ) $(_TESTS)
