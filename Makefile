CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = pthreads_sync

all: $(TARGET)

$(TARGET): pthreads_sync.c
	$(CC) $(CFLAGS) -o $(TARGET) pthreads_sync.c

clean:
	rm -f $(TARGET)