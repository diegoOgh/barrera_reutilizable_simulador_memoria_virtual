#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "barrera.h"

typedef struct {
    int tid;
    int etapas;
    Barrera *barrera;
} ThreadArgs;

void* worker(void* arg) {
    ThreadArgs *data = (ThreadArgs*) arg;

    for (int e = 0; e < data->etapas; e++) {

        // Trabajo simulado
        usleep(100000 + rand() % 200000);

        // Antes de la barrera
        printf("Hebra %d esperando en etapa %d\n", data->tid, e);

        // Esperar en la barrera
        barrera_wait(data->barrera);

        // Después de la barrera
        printf("Hebra %d pasó barrera en etapa %d\n", data->tid, e);
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    // Valores por defecto
    int N = 5;
    int E = 4;

    // Leer desde línea de comandos
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    if (argc > 2) {
        E = atoi(argv[2]);
    }

    printf("\nPrueba de barrera reutilizable\n");
    printf("Hebras: %d , Etapas: %d\n", N, E);
    printf("----------------------------------\n");

    srand(time(NULL));

    pthread_t hilos[N];
    ThreadArgs args[N];
    Barrera b;

    barrera_init(&b, N);

    // Crear hebras
    for (int i = 0; i < N; i++) {
        args[i].tid = i;
        args[i].etapas = E;
        args[i].barrera = &b;

        pthread_create(&hilos[i], NULL, worker, &args[i]);
    }

    // Esperar a que terminen
    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    barrera_destroy(&b);

    printf("----------------------------------\n");
    printf("Todas las hebras terminaron.\n");

    return 0;
}
