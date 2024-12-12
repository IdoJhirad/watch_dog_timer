#define _POSIX_C_SOURCE  200112L /* kill struct sa, setenv*/

#include <unistd.h>         /* fork execvp */
#include <pthread.h>        /* pthread_create */
#include <signal.h>         /*  sigaction sigemptyset  */
#include <sys/types.h>      /* kill */
#include <wait.h>           /* kill pid_t */
#include <stdlib.h>         /* exit */
#include <semaphore.h>      /*semaphor */
#include <fcntl.h>          /* O_ */
#include <stdio.h>          /*printf*/
#include <stdatomic.h>      /* atomic_int atomic_fetch_add */ 

#include "wd_shared.h"


pid_t pid_to_signal = 0 ;
scheduler_heap_t *scheduler = NULL;
volatile atomic_int counter = 0;
size_t g_repetitions = 0;
volatile atomic_int g_dnr_flag = 0;
sem_t *thread_sem = NULL;
sem_t *wdt_process_sem = NULL;
size_t g_interval_in_seconds= 0;
char *g_path_to_revive = NULL;
char *arg_list[5] = {NULL}; 

int InitScheduler(void)
{
    ilrd_uid_t uid_send_signal = BadUID , uid_increment = BadUID;
    scheduler = SchedulerCreate();
    if(!scheduler)
    {
        fprintf(stderr, "scheduler creat faild\n");
        perror(NULL);

        return -1;
    }

    uid_increment = SchedulerAdd(scheduler , time(NULL), g_interval_in_seconds 
                                 ,IncrementCounter, NULL);
    uid_send_signal = SchedulerAdd(scheduler , time(NULL), g_interval_in_seconds
                                   ,SendSignal, NULL);
    
    if(IsSameUID(BadUID , uid_send_signal) ||IsSameUID(BadUID , uid_increment))
    {
        fprintf(stderr, "somthing went worng with scheduler add : ");
        perror(NULL);

        return -1;
    }
    
    return 0;
}

void SendSignal(void *param)
{
    printf("pid: %d\t send SIGUSR1 to pid:%d\n", getpid(), pid_to_signal);
    kill(pid_to_signal , SIGUSR1);
    (void)param;
}

void IncrementCounter(void *param)
{
    if(!g_dnr_flag)
    {
        atomic_fetch_add(&counter, 1);  
        (void)param;

        printf("pid: %d counter incrment to: %d\n",getpid(), counter);

        if((size_t)atomic_load(&counter) >= g_repetitions)
        {
            printf("\n process: %d", getpid());
            printf("\tunable to communicate with process: %d ", pid_to_signal);
            printf("\nstart revive\n");

            atomic_store(&counter, 0);
            if(-1 == Revive())
            {
                perror("revive faild");

                return;
            }    
             printf("patrent process wait \n\n");
             sem_wait(wdt_process_sem);  
        }
        else
        {
            printf(" \n\nprocess: %d, counter = %d , repetitions = %ld \n",
                                             getpid(), counter,g_repetitions );

        }
    }
    else if(1 == atomic_load(&g_dnr_flag))
    {   
        SchedulerStop(scheduler);
    }
    (void)param;

}

int Revive(void)
{
    pid_t new_child_process = fork();
    if(-1 == new_child_process)
    {
        perror("fork in revive");

        return -1;
    }
    /*this is the futher process*/
    if (new_child_process)
    {
        pid_to_signal = new_child_process;
       
    }
    /*new child process*/
    else
    {
        printf("new child process created  with pid: %d\n", getpid());

        if(-1 == execvp(arg_list[0], arg_list))
        {
            perror("execvp");

            return -1;
        }
    }
    return 0;
}

void SignalHandler(int sig)
{
    atomic_store(&counter ,0);
    write(STDOUT_FILENO, "counter is resete\n", 19);
    (void)sig;
}
void DNRSigHandler(int sig)
{
    write(STDOUT_FILENO, "\n\t DNRSigHandler activet \n", 27);

    atomic_store(&g_dnr_flag, 1);

    (void)sig;
}
 
int InitSem(void)
{
    thread_sem = sem_open("thread_sem", O_CREAT, 0666 , 0);   
    if(SEM_FAILED == thread_sem)
	{
        perror("sem open");
		return -1;
    }
    
    wdt_process_sem = sem_open("wdt_process_sem", O_CREAT, 0666 , 0);
    if(SEM_FAILED == wdt_process_sem)
	{
        perror("sem open");
		return -1;
    }

    return 0;
}