CC = g++
EXECUTABLE=server client
CFLAGS = -O3 -Istdc++ -Wall -Wextra -Ideps


SRCS = $(wildcard *.cpp)

OBJS = $(SRCS:.cpp=.o)

all: server client
server: server.o 
	$(CC) $(CFLAGS) $^ -o $@
client: client.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o *.gch $(EXECUTABLE) data head tail
leaks:
	valgrind --leak-check=yes ./$(EXECUTABLE)

.PHONY: server client  clean install uninstall

