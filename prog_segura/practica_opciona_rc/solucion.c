#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int i;
    int die;
    pthread_mutex_t lock;  // Mutex para sincronización
    int active_thread;     // Hebra activa
} Shared_Info;

typedef struct {
    int id;
    Shared_Info *SI;
} Info;

/* Código de los hilos */
void *thread(void *x) {
    Info *I = (Info *) x;

    while (1) {
        pthread_mutex_lock(&I->SI->lock); // Bloquea el mutex

        // Verifica si es su turno y si debe terminar
        if (I->SI->die) {
            pthread_mutex_unlock(&I->SI->lock); // Libera el mutex y termina
            break;
        }

        if (I->SI->active_thread == I->id) {
            I->SI->i = I->id;  // Modifica el recurso compartido
            printf("Soy la hebra %d\n", I->id);
            printf("%d%d\n", I->id, I->id);

            // Cambia el turno a la otra hebra
            I->SI->active_thread = 1 - I->id;
        }

        pthread_mutex_unlock(&I->SI->lock); // Libera el mutex
        usleep(100);  // Evita que un hilo monopolice el CPU
    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tids[2];
    Shared_Info si;
    Info I[2];
    void *retval;

    // Inicializa la estructura compartida
    pthread_mutex_init(&si.lock, NULL);
    si.die = 0;
    si.active_thread = 0;  // Comienza con la hebra 0

    // Configura los datos para los hilos
    I[0].id = 0;
    I[0].SI = &si;

    I[1].id = 1;
    I[1].SI = &si;

    // Crea los dos hilos
    pthread_create(&tids[0], NULL, thread, &I[0]);
    pthread_create(&tids[1], NULL, thread, &I[1]);

    sleep(2);  // Permite que las hebras trabajen durante 2 segundos

    // Detiene a las hebras
    pthread_mutex_lock(&si.lock);
    si.die = 1;
    pthread_mutex_unlock(&si.lock);

    // Espera a que las hebras terminen
    pthread_join(tids[0], &retval);
    pthread_join(tids[1], &retval);

    // Limpia los recursos
    pthread_mutex_destroy(&si.lock);

    return 0;
}

