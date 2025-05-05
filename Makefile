CC = gcc
CFLAGS = -Iinclude -Ilib -Wall -Wextra -lpthread -lm -lfftw3 -lraylib -ldl -lrt -lGL -lX11 -std=c11 

TARGET = my_program

SRCS = ${wildcard src/*.c} main.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
