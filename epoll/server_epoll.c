#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h> 

#define true 1
#define false 0
#define MAX_EPOLL_EVENT 10
#define MAX_LISTENFD   5
#define MAX_BUFFER     1024
#define MAX_SEDN_MSG   6


/*将文件描述符设置为非阻塞*/
int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}
 
/*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中。 参数enable_et 指定是否对fd采用ET模式*/
void addfd(int epollfd, int fd, int enable_et)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events  = EPOLLIN;
	if(enable_et){
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event );
	setnonblocking(fd);
}

static void epoll_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    int i = 0, nread = 0, ch = 0, ret = 0;
    char buffer[MAX_BUFFER] = {0};

    for(i=0; i<number; i++)
    {
        int socket_fd = events[i].data.fd;
        if(socket_fd == listen_fd)
        {
            struct sockaddr_in server_address;
            int connect_fd;

            socklen_t sockaddr_len = sizeof(struct sockaddr_in);
            connect_fd = accept(listen_fd, (struct sockaddr_in*)&server_address, &sockaddr_len);
            addfd(epoll_fd, connect_fd, true);
            printf("server fd(%d) add on client fd(%d) \n", listen_fd, connect_fd);
        }
        else if (events[i].events & EPOLLIN)
        {
            printf("event trigger; \n");
            memset(buffer, 0, MAX_BUFFER);
            ret = recv(socket_fd, buffer, MAX_BUFFER-1, 0);
            if(ret < 0)
            {
                close(socket_fd);
                continue;
            }
            printf("get %d bytes form client; \n", ret);
        }
        else
        {
            printf("socket error!!! \n");
        }
    }

}

static void epoll_process2(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    int i = 0, nread = 0;
    char rcv_ch[MAX_SEDN_MSG] = {0};
    char cfm_ch[MAX_SEDN_MSG] = "world";

    for(i=0; i<number; i++)
    {
        int socket_fd = events[i].data.fd;
        if(socket_fd == listen_fd)
        {
            struct sockaddr_in server_address;
            socklen_t sockaddr_len = sizeof(struct sockaddr_in);
            int connect_fd = accept(listen_fd, (struct sockaddr_in*)&server_address, &sockaddr_len);
            addfd(epoll_fd, connect_fd, true);
            printf("server fd(%d) add on client fd(%d) \n", listen_fd, connect_fd);
        }
        else if (events[i].events & EPOLLIN)
        {
            printf("accept event form server; \n");
            ioctl(socket_fd, FIONREAD, &nread); //FIONREAD TIOCINQ
            if(nread == 0)
            {
                close(socket_fd);
                printf("remove socket fd = %d \n", socket_fd);
                exit(1);
            }
            else
            {
                read(socket_fd, &rcv_ch, MAX_SEDN_MSG);
                printf("server read client socket:fd = %d, received: %s \n", socket_fd, rcv_ch);
                write(socket_fd, &cfm_ch, MAX_SEDN_MSG);
                printf("server write client socket:fd = %d, confirm: %s \n", socket_fd, cfm_ch);
            }
        }
        else
        {
            printf("socket error!!! \n");
            exit(1);
        }
    }

}

int main(int argc,char* argv[])
{
    int ret =0;
    struct sockaddr_in address;
    int listen_fd = 0, epoll_fd = 0;
    struct epoll_event events[MAX_EPOLL_EVENT] = {0};

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(8888);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listen_fd, MAX_LISTENFD);
    assert(ret != -1);

    epoll_fd = epoll_create(MAX_EPOLL_EVENT);
    assert(epoll_fd != -1);

    addfd(epoll_fd, listen_fd, true);

    while (1)
    {
        ret = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENT, -1);
        if(ret < 0)
        {
            printf("epoll wait error; \n");
            break;
        }

        //epoll_process(events, ret, epoll_fd, listen_fd);
        epoll_process2(events, ret, epoll_fd, listen_fd);
    }
    
    return 0;
 
}
