/* CS360: race2.c, James S. Plank. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
  int id;
  int size;
  int iterations;
  char *s;
} Thread_struct;

void *infloop(void *x)
{
  int i, j;
  Thread_struct *t;

  t = (Thread_struct *) x;

  for (i = 0; i < t->iterations; i++) {
    for (j = 0; j < t->size-1; j++)  t->s[j] = 'A'+t->id;
    t->s[j] = '\0';
    printf("Thread %d: %s\n", t->id, t->s);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t *tid;
  Thread_struct *t;
  void *retval;
  int nthreads, size, iterations, i;
  char *s;

  if (argc != 4) {
    fprintf(stderr, "usage: race nthreads stringsize iterations\n");
    exit(1);
  }

  nthreads = atoi(argv[1]);
  size = atoi(argv[2]);
  iterations = atoi(argv[3]);

  tid = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
  t = (Thread_struct *) malloc(sizeof(Thread_struct) * nthreads);
  s = (char *) malloc(sizeof(char *) * size);

  for (i = 0; i < nthreads; i++) {
    t[i].id = i;
    t[i].size = size;
    t[i].iterations = iterations;
    t[i].s = s;
    if (pthread_create(tid+i, NULL, infloop, t+i) != 0) { perror("pthread_create"); exit(1); }
  }
  for (i = 0; i < nthreads; i++) {
    if (pthread_join(tid[i], &retval) != 0) { perror("pthread_join"); exit(1); }
  }
  return 0;
}

