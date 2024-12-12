
#define  _POSIX_C_SOURCE 1  /* struct sigaction */
#include <assert.h>         /* assert */
#include <unistd.h>         /* fork execvp */
#include <pthread.h>        /* pthread_create */
#include <sys/types.h>      /* kill */
#include <signal.h>         /* kill sigaction sigemptyset */
#include <wait.h>           /* kill pid_t */
#include <stdlib.h>         /* exit */
#include <stdatomic.h>      /* atomic_int atomic_load atomic_store  */ 
#include <semaphore.h>      /* semaphor */
#include <fcntl.h>          /* O_ */
#include <stdio.h>          /* printf */

#include "wd_shared.h" 
#include "wd.h"

/*function declaration*/
static void *ThreadFunction(void *data);
static void CleanUp(void);
static void MakeArgList(char **argv);
void block_signals();

/*global variables*/
static struct sigaction sa = {0};
static struct sigaction old_sa = {0};

static struct sigaction dnr_sa = {0};
static struct sigaction old_dnr_sa = {0};
static pthread_t thread = 0;
static char *env_var = NULL;
static char interval_str[20];
static char repetitions_str[20];

/*thread function*/
static void *ThreadFunction(void *data)
{
    (void)data;

    printf("program entered to thread function\n");
    
    /*creat the scheduler and add task*/
    if(-1 ==  InitScheduler())
    {
        perror("init scheduler");

        return NULL;
    }
   
    atomic_store(&counter ,0);
    
    /*wait for wd_process to be ready*/
    if(!env_var)
    {
        sem_wait(wdt_process_sem);
        printf("the WDPT process continued the thread\n");

    }

    /*let the mmi return to client*/
    sem_post(thread_sem);

    SchedulerRun(scheduler);

    /*if the run return its mean DNR is calld and SIGUSR2 is send*/
    SchedulerDestroy(scheduler);

    /*sem_post(thread_sem);*/

    return NULL;
}

int MMI(size_t interval_in_seconds, size_t repetitions, char **argv)
{
 
    sigemptyset(&sa.sa_mask);
	sa.sa_handler = SignalHandler;
    sa.sa_flags = 0;
    sigaction(SIGUSR1 , &sa, &old_sa);
   
    sigemptyset(&dnr_sa.sa_mask);
    dnr_sa.sa_handler = DNRSigHandler;
    dnr_sa.sa_flags = 0;
    sigaction(SIGUSR2 , &dnr_sa, &old_dnr_sa); 

    printf("the MMI process pid is: %d \n\n", getpid());

    g_repetitions = repetitions;
    g_interval_in_seconds = interval_in_seconds;
    g_path_to_revive = "./wd_process";

    MakeArgList(argv);

    /*intilize the semaphor*/
    InitSem();
   
   /* the env if the wd is not alive*/
   env_var = getenv("WD_PID");
   if(!env_var)
   {
        printf("wd_process is about to get opened\n");
        if(-1 == Revive())
        {
            perror("mmi can open wd_process");

            return -1;
        }
        
   }
    /*if wd alive or not thread create */
    if(env_var)
    {   
        printf("\twd is alive and dont need revive reconnect in progress, the env is: %s\n",env_var);
        pid_to_signal = atoi(env_var);

        /*increment if wd alive and wait for threas */
        sem_post(wdt_process_sem);
    }

    /*parent process create the thread */

    printf("mmi new thread is about to be created\n");

    if(0 != pthread_create(&thread, NULL, ThreadFunction, NULL))
    {    
        CleanUp();

        return -1;
    }
    /*the main thread will return after the thread function ready*/
    sem_wait(thread_sem);
    block_signals();

    printf("the mmi is about to return to the thread \n\n ");

    return 0;
}

static void CleanUp(void)
{
    sigset_t mask;
    
    sem_close(thread_sem);
    sem_close(wdt_process_sem);
    sem_unlink("thread_sem");
    sem_unlink("wdt_process_sem");

    sigaction(SIGUSR1 , &sa, NULL);
    sigaction(SIGUSR2 , &old_dnr_sa, NULL);
    
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

}

void DNR()
{
 
    kill(pid_to_signal , SIGUSR2);

    kill(getpid() , SIGUSR2);

    /*sem_wait(thread_sem);*/
    pthread_join(thread, NULL);

    CleanUp();
}


void block_signals()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    
}
static void MakeArgList(char **argv)
{
    size_t i = 3;

    sprintf(interval_str, "%lu", (unsigned long)g_interval_in_seconds);
    sprintf(repetitions_str, "%lu", (unsigned long)g_repetitions);
    
    arg_list[0] = g_path_to_revive; /*the wd*/
    arg_list[1] = repetitions_str;
    arg_list[2] = interval_str;

    while( argv[i-3] != NULL && i < 20) 
    {
        arg_list[i] = argv[i - 3];

        ++i;
    }

    arg_list[i] = NULL;
}