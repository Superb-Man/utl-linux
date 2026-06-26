CC     = gcc
TARGET = test3

.PHONY: all clean

all: $(TARGET)

$(TARGET): test.c include/uthread.c include/queue.c
	$(CC) -fsanitize=address -g $(TARGET).c include/uthread.c include/queue.c -o $(TARGET)

clean:
	rm -f $(TARGET)