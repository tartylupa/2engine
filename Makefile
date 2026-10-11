CC = gcc

CFLAGS = -Wall -Wextra $(shell pkg-config --flags vulkan sdl3 vma)
LIBS = $(shell pkg-config --libs vulkan sdl3)

SRCS = main.c $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

TARGET = ./2engine

all: $(TARGET) count

$(TARGET): $(OBJS)
	$(CC) $(LIBS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) -c $< -o $@

run:
	$(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

count:
	cloc .

.PHONY: clean run count
