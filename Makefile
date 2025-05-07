CC = gcc
CFLAGS = -Iinclude -Ilib -Wall -Wextra -std=c11
LDFLAGS = lib/raylib/src/libraylib.a -lGL -lX11 -lpthread -lm -ldl

TARGET = audio_visualizer
BUILD_DIR = build

SRCS = $(wildcard src/*.c) main.c
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)