#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h> 

using namespace std;

#define INPUT   0
#define OUTPUT  1


int main(void)
{
	pid_t pid[3] = {1, 1, 1};;
	int fd[2];
	int i, read_count = 0;
	char inpipe[100] = {0};
	char outpipe[100] = {0};
	
	/*create pipe*/
	if(pipe(fd) < 0)
	{
		printf("create pipe error \n");
		return -1;
	}
	
	/*create process*/
	for(i = 0; i < 3; i++)
	{
		if((pid[i] = fork()) < 0)
		{
			printf("fork %d failed", i);
			return -1;
		}	
	}
	
	if(pid[0] == 0)
	{
		lockf(fd[OUTPUT], F_LOCK, 0);
		//printf("[child] 0 process id is: %d\n", getpid());
		sprintf(outpipe, "[child] 0 process is send message...\n");
		write(fd[OUTPUT], outpipe, sizeof(outpipe));
		sleep(1);
		lockf(fd[OUTPUT], F_ULOCK, 0);
		exit(0);
	}
	if(pid[1] == 0)
	{
		lockf(fd[OUTPUT], F_LOCK, 0);
		//printf("[child] 1 process id is: %d\n", getpid());
		sprintf(outpipe, "[child] 1 process is send message...\n");
		write(fd[OUTPUT], outpipe, sizeof(outpipe));
		sleep(1);
		lockf(fd[OUTPUT], F_ULOCK, 0);
		exit(0);
	}
	if(pid[2] == 0)
	{
		lockf(fd[INPUT], F_LOCK, 0);
		printf("[child] 2 process id is: %d\n", getpid());
		read(fd[INPUT], inpipe, sizeof(outpipe));
		sleep(1);
		printf("[child] 2 read message is: %s \n", inpipe);
		lockf(fd[INPUT], F_ULOCK, 0);
		exit(0);
	}
	
	/*father*/
	wait(0);
	read(fd[INPUT], inpipe, sizeof(outpipe));
	printf("[parent] read message is: %s \n", inpipe);
	exit(0);
}
