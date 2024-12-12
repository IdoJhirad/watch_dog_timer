#include <stdio.h> /*printf*/
#include <semaphore.h> /*semaphor */

#include "wd.h"

int main(int argc , char *argv[])
{
   unsigned int sleep_time = 30;
   
    printf("entered client\n\t argv[1]: %s , argv[2]: %s\n\n",argv[1], argv[2]);
    sem_unlink("thread_sem");
    sem_unlink("wdt_process_sem");


    MMI(2,3,argv);

    printf("mmi come back\n");
    while(sleep_time)
    {
        sleep_time = sleep(sleep_time);
    }
    printf("DNR is about to be calld\n\n  ");
    DNR();
    printf("DNR is calld\n\n");

    return 0;
}