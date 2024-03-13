#include "threadtools.h"
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



/*
 * Print out the signal you received.
 * If SIGALRM is received, reset the alarm here.
 * This function should not return. Instead, call siglongjmp(sched_buf, 1).
 */
void sighandler(int signo) {
    // TODO
    if( signo == SIGALRM ){
        printf("caught SIGALRM\n");
        alarm(timeslice);
    }
    if( signo == SIGTSTP ){
        printf("caught SIGTSTP\n");
    }
    sigprocmask(SIG_SETMASK, &base_mask, NULL);
    longjmp(sched_buf, 1);
}



/*
 * Prior to calling this function, both SIGTSTP and SIGALRM should be blocked.
 */
void scheduler() {
    // TODO
   
    /* longjmp過來都會由此開始執行 */
    int ret = setjmp(sched_buf);

    /* 由main函數進入 執行第一個create的thread */
    if( ret == 0)    
        longjmp(RUNNING->environment, 1);
   
    /* 如果鎖空閒&&wq不空 */
    if( bank.lock_owner == -1 && wq_size != 0 ){
        ready_queue[rq_size++] = waiting_queue[0];

        /* 讓這個thread拿到鎖 */
        bank.lock_owner = waiting_queue[0]->id;

        for(int i = 1; i < wq_size; ++i)
            waiting_queue[i-1] = waiting_queue[i];
        wq_size--;
    }

    if( ret == 1 ){
        rq_current = (rq_current + 1) % rq_size; 
        longjmp(RUNNING->environment, 1);
    }
    else if( ret == 2 ){
        waiting_queue[wq_size++] = RUNNING;

        /* 如果移動的thread是rq最後面那一個的話 */
        if( rq_current == rq_size-1 ){
            /* 你不可能會是最後一個thread 因為還有其他thread拿著鎖 */
            /* 還有其他thread 從rq第一個開始 */
            rq_size--;
            rq_current = 0;
            longjmp(RUNNING->environment, 1);
        }
        /* 如果移動的thread不是rq最後面那一個的話 */
        else{
            /* 代表你後面還有人 除了RUNNING還有其他thread */
            RUNNING = ready_queue[rq_size - 1];
            rq_size--;
            longjmp(RUNNING->environment, 1);
        }
    }
    // ret == 3
    else{
        free(RUNNING);
        /* 如果刪掉的thread是rq最後面那一個的話 */
        if( rq_current == rq_size-1 ){
            /* 你同時也是最後一個thread */
            if( rq_size == 1 ){
                return;
            }
            /* 如果還有其他thread 從rq第一個開始 */
            else{
                rq_size--;
                rq_current = 0;
                longjmp(RUNNING->environment, 1);
            }
        }
        /* 如果刪掉的thread不是rq最後面那一個的話 */
        else{
            /* 代表你後面還有人 除了RUNNING還有其他thread */
            RUNNING = ready_queue[rq_size - 1];
            rq_size--;
            longjmp(RUNNING->environment, 1);
        }
    }

}

    