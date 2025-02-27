#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int i;
    int die;
    pthread_mutex_t lock; // Mutex para sincronización
} Shared_Info;

typedef struct {
    int id;
    Shared_Info *SI;
} Info;

void *thread(void *x) {
    Info *I = (Info *) x;

    while (!I->SI->die) {
        pthread_mutex_lock(&I->SI->lock); // Bloquea el mutex
        I->SI->i = I->id;
        pthread_mutex_unlock(&I->SI->lock); // Desbloquea el mutex
    }
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tids[2];
    Shared_Info si;
    Info I[2];
    void *retval;

    pthread_mutex_init(&si.lock, NULL); // Inicializa el mutex
    si.die = 0;

    I[0].id = 0;
    I[0].SI = &si;

    I[1].id = 1;
    I[1].SI = &si;

    pthread_create(&tids[0], NULL, thread, &I[0]);
    pthread_create(&tids[1], NULL, thread, &I[1]);

    sleep(2);

    si.die = 1;

    pthread_join(tids[0], &retval);
    pthread_join(tids[1], &retval);

    pthread_mutex_destroy(&si.lock); // Destruye el mutex

    printf("Soy la hebra %d\n", si.i);
    return 0;
}

