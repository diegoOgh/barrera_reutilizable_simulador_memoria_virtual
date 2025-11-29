#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/** ** ** **
 * Para ejecutar: ./sim Nmarcos tamañomarco [--verbose] traza.txt
 * * * * * * 
 * Algortimo de Reemplazo 4: LRU Reloj (eligiendo página aleatoriamente y verificando bit R en 0 para reemplazo)
  
 * Reemplazar una página que sea de las más viejas
 * Organizar los marcos de páginas en una cola circular FIFO y recorrer cola buscando víctima de la siguiente manera:
 * (esta parte es la importante)
   Si bit R = 0 página es elegida para reemplazo
   Si bit R = 1 página ha sido usada, darle segunda oportunidad y poner bit en 0 y seguir con la siguiente página 
 * Puntero que recorre cola se mueve rápido cuando hay necesidad de páginas
 * Si hay mucha memoria libre… baja sobrecarga
 * Si la memoria física es grande mas difícil de lograrlo bien
   Solución: agregar más punteros para elegir víctima 
   https://youtu.be/FJMl7v_x6DI un video de apoyo 
   */
//la estructura del marco
typedef struct {
    int marco_id;
    int pagina_virtual; // la pagina que esté en el marco
    int bit_referencia; // el bit R 
    int valido; // si el marco está ocupado o no, clave para usar el algoritmo jiji
} marco;
// estrurctura para la tabla de páginas
typedef struct {
    int numero_pagina;
    int marco_asignado;
    int valido; // 1 si la entrada es válida
    int bit_referencia; // bit R 
} entrada_tabla_paginas;

// variables globales que se usarán:
marco *marcos; // arreglo de marcos
int puntero_reloj; // el puntero que recorre circularmente
entrada_tabla_paginas *tabla_paginas;  // tabla de páginas
int *marcos_libres; // lista de marcos disponibles
int cantidad_marcos_libres;
int total_marcos; // bueno, use nombres entendibles
int tamano_pagina;
int bits_desplazamiento; // número de bits para el offset

// recorremos cola para buscar "páginas victimas", cuando haya un fallo de página HIT
int buscar_victima() {
    while (1) {
        marco *marco_actual = &marcos[puntero_reloj];
        
        if (marco_actual->bit_referencia == 0) {
            // Encontramos víctima - marco no usado recientemente
            int victima = puntero_reloj;
            puntero_reloj = (puntero_reloj + 1) % total_marcos;
            return victima;
        } else {
            // dar segunda oportunidad
            marco_actual->bit_referencia = 0;
            puntero_reloj = (puntero_reloj + 1) % total_marcos;
        }
    }
}
// Cuando hay acierto poner bit R = 1
void actualizar_bit_r(int marco_id) {
    marcos[marco_id].bit_referencia = 1;
}
void iniciar_sim(int nmarcos, int tamano_pag) {
    total_marcos = nmarcos;
    tamano_pagina = tamano_pag;
    
    // Calcular bits para el desplazamiento (ej.- 4096 = 2^12 -> 12 bits)
    bits_desplazamiento = 0;
    int temp = tamano_pagina;
    while (temp > 1) {
        temp >>= 1;
        bits_desplazamiento++;
    }
   // Inicializar tabla de paginas (2^16 entradas para direcciones de 32 bits)
    int entradas_tabla = 65536;  // 2^16 paginas
    tabla_paginas = malloc(entradas_tabla * sizeof(entrada_tabla_paginas));
    for (int i = 0; i < entradas_tabla; i++) {
        tabla_paginas[i].valido = 0;
        tabla_paginas[i].bit_referencia = 0;
        tabla_paginas[i].numero_pagina = i;
    }
    
    // inicializar marcos
    marcos = malloc(total_marcos * sizeof(marco));
    marcos_libres = malloc(total_marcos * sizeof(int));
    
    for (int i = 0; i < total_marcos; i++) {
        marcos[i].valido = 0;
        marcos[i].bit_referencia = 0;
        marcos[i].marco_id = i;
        marcos_libres[i] = i;  // Todos los marcos inicialmente libres
    }
    
    cantidad_marcos_libres = total_marcos;
    puntero_reloj = 0;
    
    printf("Simulador inicializado con %d marcos de %d bytes cada uno\n", total_marcos, tamano_pagina);
}
// le doy fin al simulador
void finalizar_sim(){
    free(tabla_paginas);
    free(marcos);
    free(marcos_libres);
}

void manejo_fallo_pag(int npagina, uint32_t *dir_fisica, int *marco_usado){
    int id_marco;
    
    if (cantidad_marcos_libres > 0) {
        // hay marcos libres? - usar uno
        id_marco = marcos_libres[--cantidad_marcos_libres];
    } else {
        // si no - usar algoritmo Reloj para encontrar víctima
        id_marco = buscar_victima();

        // luego se invalida la página anterior que se estaba en ese marco
        if (marcos[id_marco].valido) {
            tabla_paginas[marcos[id_marco].pagina_virtual].valido = 0;
        }

    }
    
    // Asignar el marco a la nueva página
    marcos[id_marco].pagina_virtual = npagina;
    marcos[id_marco].valido = 1;
    marcos[id_marco].bit_referencia = 1;  // Acaba de ser referenciada
    
    // Actualizar tabla de páginas
    tabla_paginas[npagina].marco_asignado = id_marco;
    tabla_paginas[npagina].valido = 1;
    tabla_paginas[npagina].bit_referencia = 1;
    
    *marco_usado = id_marco;
    actualizar_bit_r(id_marco);
}
// listo
void ejecutar_sim(const char *archivo_trace, int verbose){
    FILE *archivo = fopen(archivo_trace, "r");
    if (!archivo) {
        printf("Error: No se pudo abrir el archivo %s\n", archivo_trace);
        return;
    }  
    printf("Iniciando simulacion con archivo: %s\n", archivo_trace);  
    if (verbose) {
        printf("\n=== MODO VERBOSE ===\n");
        printf("DV = Direccion Virtual | PV = Pagina Virtual | Despl = Desplazamiento\n");
        printf("DF = Direccion Fisica | Marco = Marco Fisico Asignado\n");
        printf("------------------------------------------------------------\n");
    }
    uint32_t direccion;
    int total_referencias = 0;
    int fallos_pagina = 0;
    while (fscanf(archivo, "%x", &direccion) != EOF) {
        uint32_t dir_fisica;
        int numero_pagina, desplazamiento, hit, marco_usado;
        
        traducir_direccion(direccion, &dir_fisica, &numero_pagina, &desplazamiento, &hit, &marco_usado);      
        total_referencias++;
        
        if (!hit) { 
            fallos_pagina++;
        }
        // aqui mostramos lo que se pide en la tarea:
        // Con –verbose: por cada DV, imprimir: DV, nvp, offset, HIT/FALLO, marco usado, DF calculada.
        if (verbose) { // esto se lo pedí a la IA, aun no lo pruebo
            printf("DV: 0x%08x | PV: %5d | Despl: 0x%04x | %s | Marco: %2d | DF: 0x%08x\n",
                   direccion, numero_pagina, desplazamiento,
                   hit ? "HIT " : "FALLO", marco_usado, dir_fisica);
        }
    }
    
    fclose(archivo);
    
    // salida de la simulacion:
    printf("\n=== REPORTE FINAL ===\n");
    printf("Total referencias: %d\n", total_referencias);
    printf("Fallos de pagina: %d\n", fallos_pagina);
    printf("Tasa de fallos: %.2f\n", (float)fallos_pagina / total_referencias * 100);
}
void traducir_direccion(uint32_t dir_virtual, uint32_t *dir_fisica, int *npagina, int *desplazamiento, int *hit, int *marco_usado);
// falta esto, que es como el nucleo del simulador, porque hay que descomponer la direccion virtual en numero de pagina y desplazamiento.
// hay que ver en la tabla de paginas si hay hit o fallo, y ahí se actualiza el bit R o se usa la funcion de manejo de fallo respectivamente

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s Nmarcos tamano_marco [--verbose] traza.txt\n", argv[0]);
        printf("Ejemplo: %s 8 4096 traza.txt\n", argv[0]);
        printf("Ejemplo verbose: %s 8 4096 --verbose traza.txt\n", argv[0]);
        return 1;
    }
    
    int num_marcos = atoi(argv[1]);
    int tamano_marco = atoi(argv[2]);
    int verbose = 0;
    char *archivo_trace;
    
    // Parsear argumentos
    if (argc == 5 && strcmp(argv[3], "--verbose") == 0) {
        verbose = 1;
        archivo_trace = argv[4];
    } else {
        archivo_trace = argv[3];
    }
    
    // validar parámetros
    if (num_marcos <= 0 || tamano_marco <= 0) {
        printf("Error: Numero de marcos y tamano deben ser positivos\n");
        return 1;
    }
    
    // Verificamos que el tamaño de página sea potencia de 2
    int temp = tamano_marco;
    while (temp > 1 && (temp % 2 == 0)) {
        temp /= 2;
    }
    if (temp != 1) {
        printf("Error: El tamano de marco debe ser potencia de 2\n");
        return 1;
    }
    
    iniciar_sim(num_marcos, tamano_marco);
    ejecutar_sim(archivo_trace, verbose);
    finalizar_sim();
    
    return 0;
}