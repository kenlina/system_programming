#include "my_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void tpool_add(tpool *pool, void *(*func)(void *), void *arg) {
    // TODO
    pthread_mutex_lock(&pool->lock);
    job task = {func, arg};
    pool->queue[pool->tail++] = task;
    pool->task_count++;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
}

void tpool_wait(tpool *pool) {
    // TODO
    pthread_mutex_lock(&pool->lock);
    pool->no_more_job = 1;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);

    for(int i = 0; i < pool->thread_count; ++i){
        pthread_join(pool->tid[i], NULL);
    }
}

void tpool_destroy(tpool *pool) {
    // TODO
    free(pool->tid);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    free(pool);
}

tpool *tpool_init(int n_threads) {
    // initialize
    tpool *pool = malloc(sizeof(tpool));
    pool->head = 0;
    pool->tail = 0;
    pool->task_count = 0;
    pool->thread_count = n_threads;
    pool->no_more_job = 0;
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pool->tid = malloc(n_threads * sizeof(pthread_t));

    for(int i = 0; i < n_threads; ++i){
        pthread_create(&pool->tid[i], NULL, worker, pool);
    }

    return pool;
}

void *worker(void *arg){
    tpool *pool = (tpool *)arg;
    while(1){
        pthread_mutex_lock(&pool->lock);
        while(pool->task_count == 0){
            if(pool->no_more_job) {
                pthread_mutex_unlock(&pool->lock);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        }
        job task = pool->queue[pool->head++];
        pool->task_count--;
        pthread_mutex_unlock(&pool->lock);
        task.func(task.arg);
    }
    return NULL;
}