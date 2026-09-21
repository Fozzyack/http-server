
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
ROUTER_TEST = bin/router_test
ROUTER_TEST_OBJ = obj/tests/router_test.o obj/routes/router.o obj/routes/healthcheck.o obj/log/log.o
ROUTER_TEST_DEPS = $(ROUTER_TEST_OBJ:.o=.d)
# --------

.PHONY: default clean clean-all clean-debug debug test test-router

default: $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(DEBUG)


-include $(DEPS) $(ROUTER_TEST_DEPS)

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

test: test-router

test-router: $(ROUTER_TEST)
	./$(ROUTER_TEST)

$(ROUTER_TEST): $(ROUTER_TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
