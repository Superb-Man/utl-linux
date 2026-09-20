CC     = gcc
TARGET = test3
PC_TARGET = test3

UTL_SRCS = include/uthread.c include/queue.c include/mutex.c include/cond.c include/semaphore.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): test.c include/uthread.c include/queue.c
	$(CC) -fsanitize=address -g $(TARGET).c include/uthread.c include/queue.c -o $(TARGET)

$(PC_TARGET): $(PC_TARGET).c $(UTL_SRCS)
	$(CC) -fsanitize=address -g $(PC_TARGET).c $(UTL_SRCS) -o $(PC_TARGET)

clean:
	rm -f $(TARGET) $(PC_TARGET)