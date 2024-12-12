#define _POSIX_C_SOURCE  200112L /* struct sa, setenv*/

#include <signal.h>     /* sigaction */
#include <sys/types.h>  /* pid_t */
#include <stdio.h>      /* printf size_t */
#include <semaphore.h>  /* semaphor */
#include <errno.h>      /* perror */
#include <sys/types.h>  /* getpid */
#include <stdlib.h>     /* strtoul */
#include <unistd.h>     /* getpid */
#include <time.h>       /* time*/
#include <stdatomic.h>  /* atomic_int atomic_load atomic_store */ 

/*scheduler and uid already declared at shared library*/ 
#include "wd_shared.h"  

#define SIZE_OF_STRING 20

static void InitEnv(char *pid_str);
 static void WatchDogProcess(void);

int main(int argc , char *argv[])

{   
    /*for the env var*/
    pid_t pid = getpid();
    size_t i = 0;
    
    char pid_str[SIZE_OF_STRING];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    printf("you entered to the watch dog process\n");
   
    (void)argc;

   /*init the global var*/
    g_repetitions = strtoul(argv[1], NULL, 10);
    printf("repetition is :%ld \n",g_repetitions );

    g_interval_in_seconds = strtoul(argv[2], NULL, 10);
    printf("interval is :%ld \n",g_interval_in_seconds );

    g_path_to_revive = argv[3];


    /*intilis arg_list for revive client*/

    printf("\n\nwatchdog the path to revive is %s\n", argv[3]);

    while(argv[i+3] != NULL && i < 20)
    {
        arg_list[i] = argv[i+3];

        ++i;
    }

    arg_list[i] = NULL;

    InitEnv(pid_str);
    
    WatchDogProcess();

    return 0;
}



 static void WatchDogProcess(void)
 {
    struct sigaction sa = {0}, dnr_sa = {0};
    /*link the semaphor*/
    wdt_process_sem = sem_open("wdt_process_sem", 0);
   
    /*client process pid */
    pid_to_signal = getppid();

    printf("the watch dog process pid is: %d and try to coommunicat with :%d,\n\n", getpid(), pid_to_signal);
    
    /*intilize the counter for the watchdogtimer*/
    atomic_store(&counter ,0);
    printf("counter is:%d\n",counter );

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SignalHandler;
    sa.sa_flags = 0;
    if( -1 ==  sigaction(SIGUSR1 , &sa, NULL))
    {
        perror("sigaction faild");

        return;
    }

    sigemptyset(&dnr_sa.sa_mask);
	dnr_sa.sa_handler = DNRSigHandler;
    dnr_sa.sa_flags = 0;
    if( -1 ==  sigaction(SIGUSR2 , &dnr_sa, NULL))
    {
        perror("sigaction faild");
        
        return;
    }

    /*creat the scheduler and add task*/
    if(-1 == InitScheduler())
    {
        perror("init scheduler");

        return ;
    }

    /*open the sem to the thread in thread function or in the revive*/
    sem_post(wdt_process_sem);
    
    SchedulerRun(scheduler);


    SchedulerDestroy(scheduler);
    
    /*tell the immortal to kill himself*/
    printf("scheduler run is back\n");

 }

static void InitEnv(char *pid_str)
{
   
    printf("\n you are inside the int env yhe arg pid = %s\n\n",pid_str);
    setenv("WD_PID", pid_str, 1);


}