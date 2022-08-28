CC = gcc
EXECUTABLE=server_epoll client_epoll
CFLAGS = -O3 -Istdc++ -Wall -Wextra -Ideps

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.cpp=.o)

all: server_epoll client_epoll
server_epoll: server_epoll.o 
	$(CC) $(CFLAGS) $^ -o $@
client_epoll: client_epoll.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o *.gch $(EXECUTABLE) data head tail
leaks:
	valgrind --leak-check=yes ./$(EXECUTABLE)

.PHONY: server_epoll client_epoll  clean

