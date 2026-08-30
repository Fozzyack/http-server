
CC = gcc
CFLAGS = -std=gnu17 -Iinclude -Wall -Werror -Wextra -Wpedantic -MP -MMD -pthread -D_GNU_SOURCE

TARGET = bin/server.out
DEBUG = debug/server.out

rwildcard = $(wildcard $1$2) $(foreach directory, $(wildcard $1*/), $(call rwildcard,$(directory)$2))

SRC = $(call rwildcard, src/,*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
DEPS = $(OBJ:.o=.d)

.PHONY: default clean clean-all clean-debug debug

default: $(TARGET)

debug: $(DEBUG)

$(DEBUG): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

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

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
