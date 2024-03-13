#include "my_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <sys/times.h>

typedef long long LL;
void *Collatz(void *args) {
  LL x = *(LL *)args;
  LL cnt = 0;
  while (x != 1) {
    if (x & 1)
      x = x * 3 + 1;
    else
      x /= 2;
    cnt++;
    // try uncomment printf
    // printf("%lld\n", x);
  }
  // try uncomment printf
  // printf("%lld\n", cnt);
  return NULL;
}

// void p_times(clock_t real, struct tms *tstart, struct tms *tend){
//     long clktck = sysconf(_SC_CLK_TCK);
//     printf(" real: %7.3f\n", real/(double)clktck);
//     printf(" user: %7.3f\n", (tend->tms_utime-tstart->tms_utime)/(double)clktck);
//     printf("  sys: %7.3f\n", (tend->tms_stime-tstart->tms_stime)/(double)clktck);
// }

#define N 10
#define M 0x800000

int main() {

  // struct tms tstart, tend;
  // clock_t start, end;

  tpool *pool = tpool_init(N);
  LL *arg = malloc(M * sizeof(LL));

  // start = times(&tstart);

  for (int i = 0; i < M; i++) {
    arg[i] = 0x10000000ll + i;
    tpool_add(pool, Collatz, (void *)&arg[i]);
  }
  tpool_wait(pool);
  tpool_destroy(pool);
  free(arg);

  // end = times(&tend);
  // p_times(end-start, &tstart, &tend);

  return 0;
}