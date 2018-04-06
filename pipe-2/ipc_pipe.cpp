#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>


using namespace std;

#define INPUT   0
#define OUTPUT  1


int main(void)
{
	pid_t pid;
	int fd[2];
	int i, read_count = 0;
	char read_buffer[10] = {0};
	char write_buffer[10] = {0};
	
	/*create pipe*/
	if(pipe(fd) < 0)
	{
		printf("create pipe error \n");
		return -1;
	}
	
	/*create process*/
	if((pid = fork()) < 0)
	{
		printf("fork  failed");
		return -1;
	}
	
	/*
	在语句pid=fork()之前，只有一个进程在执行这段代码，但在这条语句之后，就变成两个进程在执行了，这两个进程的几乎完全相同，将要执行的下一条语句都是if(fpid<0)……
    为什么两个进程的fpid不同呢，这与fork函数的特性有关。fork调用的一个奇妙之处就是它仅仅被调用一次，却能够返回两次，它可能有三种不同的返回值：
    1）在父进程中，fork返回新创建子进程的进程ID；
    2）在子进程中，fork返回0；
    3）如果出现错误，fork返回一个负值；
	*/
	/*child*/
	if(pid == 0)
	{
		printf("[child] close read endpoint...\n");
		printf("[child] process id is: %d\n", getpid());
		close(fd[INPUT]);
		
		for(i=0; i<10; i++)
		{
			write_buffer[i] = i;
		}
		write(fd[OUTPUT], write_buffer, sizeof(write_buffer));
	}
	else /*father*/
 	{
		printf("[parent] close read endpoint...\n");
		printf("[parent] process id is: %d\n", getpid());
		close(fd[OUTPUT]);
		read_count = read(fd[INPUT], read_buffer, sizeof(read_buffer));
		
		for(i=0; i<read_count; i++)
		{
			printf("read_buffer[%d]: %d \n\r", i, read_buffer[i]);
		}
	}
	return 0;
}
