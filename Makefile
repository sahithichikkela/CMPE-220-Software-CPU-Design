CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = cpu_emulator
OBJS = main.o cpu.o assembler.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c cpu.h assembler.h
	$(CC) $(CFLAGS) -c main.c

cpu.o: cpu.c cpu.h
	$(CC) $(CFLAGS) -c cpu.c

assembler.o: assembler.c assembler.h cpu.h
	$(CC) $(CFLAGS) -c assembler.c

clean:
	rm -f $(OBJS) $(TARGET) *.bin

test: $(TARGET)
	@echo "=== Testing Fibonacci Demo ==="
	./$(TARGET) demo fibonacci
	@echo ""
	@echo "=== Testing Hello World Demo ==="
	./$(TARGET) demo hello
	@echo ""
	@echo "=== Testing Timer Demo ==="
	./$(TARGET) demo timer

.PHONY: all clean test
