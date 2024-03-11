#include <stdio.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <sys/socket.h>

#define MAX_SEDN_MSG   6

int main(int argc,char* argv[])
{
    int client_socketfd = 0;
    int len, result;
    char send_ch[MAX_SEDN_MSG] = "hello";
    char rcv_ch[MAX_SEDN_MSG] = {0}; 
    struct sockaddr_in address;

    client_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8888);
    len = sizeof(address);

    result = connect(client_socketfd, (struct sockaddr*)&address, len);
    if(result == -1)
    {
        printf("connect error!! \n");
        exit(1);
    }

    printf("client socket fd:%d \n", client_socketfd);
    write(client_socketfd, &send_ch, strlen(send_ch));
    printf("first client write: %s \n", send_ch);
    read(client_socketfd, &rcv_ch, sizeof(rcv_ch));
    printf("first client read: %s \n", rcv_ch);
    sleep(2);

    write(client_socketfd, &send_ch, strlen(send_ch));
    printf("second client write: %s \n", send_ch);
    read(client_socketfd, &rcv_ch, sizeof(rcv_ch));
    printf("second client read: %s  \n", rcv_ch);

    close(client_socketfd);

    return 0;
}
