
CC = gcc
CFLAGS = -std=gnu17 -Iinclude -Wall -Werror -Wextra -Wpedantic -MP -MMD -pthread -D_GNU_SOURCE
DEBUG_FLAGS = $(CFLAGS) -g -O0

TARGET = bin/server.out
DEBUG = debug/server.out

rwildcard = $(wildcard $1$2) $(foreach directory, $(wildcard $1*/), $(call rwildcard,$(directory)$2))

SRC = $(call rwildcard, src/,*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
DEBUG_OBJ = $(patsubst src/%.c, obj/debug/%.o, $(SRC))
DEPS = $(OBJ:.o=.d)


# TEST DEPS
ROUTER_TEST = bin/router_test.out
ROUTER_TEST_OBJ = obj/tests/router_test.o obj/routes/router.o obj/routes/healthcheck.o obj/log/log.o
ROUTER_TEST_DEPS = $(ROUTER_TEST_OBJ:.o=.d)
PARSER_TEST = bin/parser_test.out
PARSER_TEST_OBJ = obj/tests/parser_test.o obj/http/parser.o obj/log/log.o
PARSER_TEST_DEPS = $(PARSER_TEST_OBJ:.o=.d)
# --------

.PHONY: default clean clean-all clean-debug debug test test-router test-parser

default: $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(DEBUG)


-include $(DEPS) $(ROUTER_TEST_DEPS) $(PARSER_TEST_DEPS)

all: $(TARGET) $(DEBUG)

clean-bin:
	rm -rf bin/
	rm -rf obj/

clean-debug:
	rm -rf debug/

clean:
	rm -rf debug/
	rm -rf bin/
	rm -rf obj/


$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(DEBUG): $(DEBUG_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) -o $@ $^

obj/debug/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) -c $< -o $@


# TEST BUILD / RUNNERS

test: test-router test-parser

test-router: $(ROUTER_TEST)
	./$(ROUTER_TEST)

test-parser: $(PARSER_TEST)
	./$(PARSER_TEST)

$(ROUTER_TEST): $(ROUTER_TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(PARSER_TEST): $(PARSER_TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
