
CC = gcc
CFLAGS = -std=gnu17 -Iinclude -Wall -Werror -Wextra -Wpedantic -pthread -D_GNU_SOURCE

TARGET = bin/server.out

rwildcard = $(wildcard $1$2) $(foreach directory, $(wildcard $1*/), $(call rwildcard,$(directory)$2))

SRC = $(call rwildcard, src/,*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

.PHONY = default clean

default: $(TARGET)

clean :
	rm -rf bin/
	rm -rf obj/

$(TARGET) : $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
