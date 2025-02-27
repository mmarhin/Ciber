#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int i;
    int die;
} Shared_Info;

typedef struct {
    int id;
    Shared_Info *SI;
} Info;

/* Código de los hilos */
void *thread(void *x) {
    Info *I = (Info *) x;

    while (!I->SI->die)
        I->SI->i = I->id;

    return NULL;
}

int main(int argc, char **argv) {
    int iterations = 10; // Número de ejecuciones para observar salidas
    int result0 = 0, result1 = 0; // Contadores para las salidas
    for (int iter = 0; iter < iterations; iter++) {
        pthread_t tids[2];
        Shared_Info si;
        Info I[2];
        void *retval;

        I[0].id = 0;
        I[0].SI = &si;

        I[1].id = 1;
        I[1].SI = &si;

        si.die = 0;

        // Crear los hilos
        if (pthread_create(tids, NULL, thread, I) != 0) {
            perror("pthread_create");
            exit(1);
        }
        if (pthread_create(tids + 1, NULL, thread, I + 1) != 0) {
            perror("pthread_create");
            exit(1);
        }

        sleep(1); // Esperar para permitir condiciones de carrera

        si.die = 1;

        if (pthread_join(tids[0], &retval) != 0) {
            perror("pthread_join");
            exit(1);
        }
        if (pthread_join(tids[1], &retval) != 0) {
            perror("pthread_join");
            exit(1);
        }

        // Registrar el resultado
        if (si.i == 0) {
            result0++;
        } else if (si.i == 1) {
            result1++;
        }
    }

    // Imprimir resultados
    printf("Iteraciones: %d\n", iterations);
    printf("Resultado 0: %d veces\n", result0);
    printf("Resultado 1: %d veces\n", result1);

    return 0;
}

