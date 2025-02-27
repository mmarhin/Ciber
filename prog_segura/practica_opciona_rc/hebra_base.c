#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int i;
    int die;
} Shared_Info;

typedef struct {
    int id;
    Shared_Info *SI;
} Info;

/* Here's the thread code. */
void *thread(void *x) {
    Info *I;
    I = (Info *) x;

    while (!I->SI->die)
        I->SI->i = I->id;

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tids[2];
    Shared_Info si;
    Info I[2];
    void *retval;

    /* Set up the data to send to the threads. */
    I[0].id = 0;
    I[0].SI = &si;

    I[1].id = 1;
    I[1].SI = &si;

    si.die = 0;

    /* Create the two threads and sleep */
    if (pthread_create(tids, NULL, thread, I) != 0) {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_create(tids+1, NULL, thread, I+1) != 0) {
        perror("pthread_create");
        exit(1);
    }
    sleep(2);

    /* Tell the threads to die, then print the shared info. */
    si.die = 1;
    printf("%d", si.i);

    /* Wait for the threads to die and print out the shared info again. */
    if (pthread_join(tids[0], &retval) != 0) {
        perror("pthread_join");
        exit(1);
    }
    if (pthread_join(tids[1], &retval) != 0) {
        perror("pthread_join");
        exit(1);
    }
    printf("%d\n", si.i);

    return 0;
}

