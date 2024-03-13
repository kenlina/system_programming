#ifndef THREADTOOL
#define THREADTOOL
#include <setjmp.h>
#include <sys/signal.h>
#include "bank.h"


#define THREAD_MAX 16  // maximum number of threads created
#define BUF_SIZE 512
typedef struct tcb {
    int id;  // the thread id
    jmp_buf environment;  // where the scheduler should jump to
    int arg;  // argument to the function
    int i, x, y;  // declare the variables you wish to keep between switches
} tcb;



extern int timeslice;
extern jmp_buf sched_buf;
extern struct tcb *ready_queue[THREAD_MAX], *waiting_queue[THREAD_MAX];
extern struct Bank bank;
/*
 * rq_size: size of the ready queue
 * rq_current: current thread in the ready queue
 * wq_size: size of the waiting queue
 */
extern int rq_size, rq_current, wq_size;
/*
* base_mask: blocks both SIGTSTP and SIGALRM
* tstp_mask: blocks only SIGTSTP
* alrm_mask: blocks only SIGALRM
*/
extern sigset_t base_mask, tstp_mask, alrm_mask;
/*
 * Use this to access the running thread.
 */
#define RUNNING (ready_queue[rq_current])

void sighandler(int signo);
void scheduler();



// TODO
#define thread_create(func, id, arg) {\
    func(id, arg);    \
}

#define thread_setup(id, arg) {\
    tcb *ptr = (tcb *)malloc(sizeof(tcb));     \
    ptr->id = id;   \
    ptr->arg = arg; \
    ready_queue[rq_size++] = ptr;  \
    printf("%d %s\n", id, __func__); \
    if( setjmp(ptr->environment) == 0 ) return;\
}

#define thread_exit() { \
    longjmp(sched_buf, 3); \
}

#define thread_yield() {\
    if ( setjmp(RUNNING->environment) == 0){\
        sigprocmask(SIG_UNBLOCK, &tstp_mask, NULL);\
        sigprocmask(SIG_UNBLOCK, &alrm_mask, NULL);\
        sigprocmask(SIG_SETMASK, &base_mask, NULL);\
    }\
}

#define lock(){\
    if( bank.lock_owner == -1){\
        bank.lock_owner = RUNNING->id;\
    }\
    else{\
        if( setjmp(RUNNING->environment) == 0 ) longjmp(sched_buf, 2);\
    }\
}

#define unlock() ({\
    if( bank.lock_owner != RUNNING->id) printf("You are not the owner of the lock\n");\
    bank.lock_owner = -1;\
})

#endif // THREADTOOL


