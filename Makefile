
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

.PHONY: default clean clean-all clean-debug debug

default: $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(DEBUG)


-include $(DEPS)

clean:
	rm -rf bin/
	rm -rf obj/

clean-debug:
	rm -rf debug/

clean-all:
	rm -rf debug/
	rm -rf bin/
	rm -rf obj/

$(DEBUG): $(DEBUG_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/debug/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) -c $< -o $@
