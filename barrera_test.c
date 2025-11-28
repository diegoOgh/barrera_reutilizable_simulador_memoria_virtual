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

// --- CÓDIGO DE PRUEBA (NO ES PARTE ESTRICTA DE LA BARRERA PERO NECESARIO PARA PROBAR) ---

// Datos para pasar a los hilos
typedef struct {
    int id;
    Barrera *barrera;
    int iteraciones;
} ThreadArgs;

void* worker(void* args) {
    ThreadArgs *datos = (ThreadArgs*) args;
    
    for (int i = 0; i < datos->iteraciones; i++) {
        printf("Hebra %d: Trabajando en etapa %d...\n", datos->id, i);
        
        // Simular trabajo aleatorio
        usleep(rand() % 200000); 

        printf("Hebra %d: Llegó a la barrera (iteración %d)\n", datos->id, i);
        
        // Sincronización
        barrera_wait(datos->barrera);
        
        printf("Hebra %d: Cruzó la barrera (iteración %d)\n", datos->id, i);
    }
    return NULL;
}

int main() {
    int N = 5; // Número de hebras
    int ITERS = 3; // Número de veces que usaremos la barrera (reusabilidad)
    
    pthread_t hilos[N];
    ThreadArgs args[N];
    Barrera mi_barrera;

    // Inicializar semilla random
    srand(time(NULL));

    printf("Iniciando prueba de barrera para %d hebras y %d etapas.\n", N, ITERS);
    printf("-------------------------------------------------------\n");

    barrera_init(&mi_barrera, N);

    for (int i = 0; i < N; i++) {
        args[i].id = i;
        args[i].barrera = &mi_barrera;
        args[i].iteraciones = ITERS;
        pthread_create(&hilos[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    barrera_destroy(&mi_barrera);
    
    printf("-------------------------------------------------------\n");
    printf("Todas las hebras terminaron. Prueba exitosa.\n");

    return 0;
}