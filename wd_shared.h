#ifndef __WATCH_DOG_SHARED_H__
#define __WATCH_DOG_SHARED_H__

#include <stddef.h> /*  size_t */
#include <stdatomic.h> /* atomic_int atomic_store atomic_fetch_add */ 
#include <semaphore.h> /*semaphor */

#include "uid.h"
#include "scheduler_heap.h" 

extern pid_t pid_to_signal;
extern scheduler_heap_t *scheduler;
extern volatile atomic_int counter;
extern size_t g_repetitions;
extern volatile atomic_int g_dnr_flag;
extern sem_t *thread_sem;
extern sem_t *wdt_process_sem;
extern size_t g_interval_in_seconds;
extern char *g_path_to_revive;
extern char *arg_list[5];

int InitScheduler(void);

void SendSignal(void *param);

void IncrementCounter(void *param);

void SignalHandler(int sig);

void DNRSigHandler(int sig);

int InitSem(void);

int Revive(void);

#endif /*__WATCH_DOG_SHARED_H__*/