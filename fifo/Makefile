CC = g++
EXECUTABLE=ipc_fifo_write ipc_fifo_read
CFLAGS = -O3 -Istdc++ -Wall -Wextra -Ideps


SRCS = $(wildcard *.cpp)

OBJS = $(SRCS:.cpp=.o)

all: ipc_fifo_write ipc_fifo_read
ipc_fifo_write: ipc_fifo_write.o
	$(CC) $(CFLAGS) ipc_fifo_write.cpp -o $@

ipc_fifo_read: ipc_fifo_read.o
	$(CC) $(CFLAGS) ipc_fifo_read.cpp -o $@	
clean:
	rm -rf *.o *.gch $(EXECUTABLE)
leaks:
	valgrind --leak-check=yes ./$(EXECUTABLE)

.PHONY: test  clean install uninstall

