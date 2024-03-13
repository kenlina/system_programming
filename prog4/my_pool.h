#include <pthread.h>
#include <stdbool.h>
#ifndef __MY_THREAD_POOL_H
#define __MY_THREAD_POOL_H

#define MAX_THREADS 10
#define MAX_JOBS 0x800000

typedef struct job { 
  void *(*func)(void *);
  void *arg;
} job;

typedef struct tpool {
  // TODO: define your structure
  int task_count;
  int head;
  int tail;
  int thread_count;
  bool no_more_job;

  pthread_mutex_t lock;
  pthread_cond_t not_empty;
  pthread_t *tid;
  job queue[MAX_JOBS];
} tpool;



tpool *tpool_init(int n_threads);
void tpool_add(tpool *, void *(*func)(void *), void *);
void tpool_wait(tpool *);
void tpool_destroy(tpool *);
void *worker(void *arg);

#endif