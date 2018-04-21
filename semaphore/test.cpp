// 加入信号量操作后的程序
#include "sem.h"
#include <stdio.h>
#include <unistd.h>

//#define PV

int main()
{
    int semid = create_sems(10); // 创建一个包含10个信号量的信号集
    init_sems(semid, 0, 1);  // 初始化编号为 0 的信号量值为1

    pid_t id = fork(); // 创建子进程
    if( id < 0)
    {
        perror("fork");
        return -1;
    }

#ifdef PV
    else if (0 == id)
    {// child 
        int sem_id = get_sems();
        while(1)
        {
            P(sem_id, 0); // 对该信号量集中的0号信号  做P操作
            printf("你");
            fflush(stdout);
            sleep(1);
            printf("好");
            printf(":");
            fflush(stdout);
            sleep(1);
            V(sem_id, 0);
        }
    }
    else
    {// father
        while(1)
        {
            P(semid,0);
            printf("在");
            sleep(1);
            printf("吗");
            printf("?");
            fflush(stdout);
            V(semid, 0);
        }
        wait(NULL);
    }
	destroy_sems(semid);
	
#else
	else if (0 == id)
    {// child 
        int sem_id = get_sems();
        while(1)
        {
            printf("你");
            fflush(stdout);
            sleep(1);
            printf("好");
            printf(":");
            fflush(stdout);
            sleep(1);
        }
    }
    else
    {// father
        while(1)
        {
            printf("在");
            sleep(1);
            printf("吗");
            printf("?");
            fflush(stdout);
        }
        wait(NULL);
    }
#endif

    return 0;
}
