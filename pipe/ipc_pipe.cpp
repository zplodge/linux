//main.c
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>


using namespace std;

//int pipe(int filedis[2]);

#define INPUT   0
#define OUTPUT  1


int main(void)
{
	int file_descriptors[2];
	
	//定义子进程号
	pid_t pid;
	char buf[256];
	int returned_count;
	
	memset(buf, 0, sizeof(buf));
	//创建无名管道
	pipe(file_descriptors);
	
	//创建子进程
	if((pid = fork()) == -1)
	{
		printf("error in fork \n");
		exit(1);
	}
	
	//执行子进程
	if(0 == pid)
	{
		printf("in the spawned child process... \n");
		
		//子进程向父进程写数据, 关闭管道的读端
		close(file_descriptors[INPUT]);
		write(file_descriptors[OUTPUT], "test data", strlen("test data"));
		
		exit(0);
	}
	else
	{
		//执行父进程
		printf("in the spawned parent process... \n");
		
		//父进程读取子进程写的数据， 关闭管道的写端
		close(file_descriptors[OUTPUT]);
		returned_count = read(file_descriptors[INPUT], buf, sizeof(buf));
		printf("%d bytes of data received from spawned process: %s \n", returned_count, buf);
	}
	
	return 0;
}
