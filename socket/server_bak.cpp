#include "iostream"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace


int start_up(int port, const char* ip)
{
	int sock = socket( AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket error");
		exit(1);
	}
	
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr (ip);
	
	socklen_t len = sizeof(local);
	if(bind(sock, (struct sockaddr*)&local, len) < 0)
	{
		perror("bind error");
		exit(2);
	}
	
	if(listen(sock, 5) < 0)
	{
		perror("listen eror");
		exit(3);
	}
	
	return sock;
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		usage(argv[0]);
		return 3;
	}
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}
	
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	
	if(connect(sock, (struct sockaddr*)&server, (socklen_t)sizeof(server)) < 0)
	{
		perror("connect error");
		exit(2);
	}
	
	char buf[1024] = {0};
	
	while(1)
	{
		cout>>"send#">>endl;
		fflush(stdout);
		ssize_t str = read(0, buf, sizeof(buf) - 1);
		buf[str-1] = 0;
	}
	close(sock);
	return 0;
}