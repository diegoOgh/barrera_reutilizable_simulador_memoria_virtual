#ifndef BARRERA_H
#define BARRERA_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//estructura barrera
typedef struct {
    pthread_mutex_t mutex; // mutex
    pthread_cond_t cond;   // Variable para wait
    int count;             // ebras que han llegado
    int N;                 // total de hebras requeridas
    int etapa;             // Identificador de la etapa (generacion)
} Barrera;

// Inicialización de la barrera
void barrera_init(Barrera *b, int n_hebras) {
    b->N = n_hebras;
    b->count = 0;
    b->etapa = 0;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
}

// destruccion de la barrera
void barrera_destroy(Barrera *b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

void barrera_wait(Barrera *b) {
    pthread_mutex_lock(&b->mutex);

    // captura la etapa actual en una variable local para reusar barrera.
    int mi_etapa = b->etapa;

    b->count++;

    if (b->count == b->N) { // para ultima hebra
        b->etapa++; 
        b->count = 0; 
        // para despertar a todas
        pthread_cond_broadcast(&b->cond); 
    } else { // para que esperen las hebras
        // no usar solo count, puede haber problemas con hebras rápidas que entren antes de que las otras salgan.
        while (mi_etapa == b->etapa) {
            pthread_cond_wait(&b->cond, &b->mutex);
        }
    }
    pthread_mutex_unlock(&b->mutex);
}
#endif