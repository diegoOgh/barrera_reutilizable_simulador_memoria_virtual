# Tarea 2 - Sistemas Operativos 2025-2

### Departamento de Informática y Cs. de la Computación
### Facultad de Ingeniería
### Universidad de Concepción

**Docentes**: Prof. Cecilia Hernández y Prof. Juan Felipe González
**Estudiantes**:
- Gabriel Huerta Torres  
- Jocabed López Flores  
- Diego Oyarzo Navia
- Valentina Serón Canales  

### Parte I: Barrera Reutilizable
Implementación de una barrera de sincronización reutilizable usando monitores con pthreads, que permite coordinar múltiples hebras en diferentes puntos de encuentro.

## Compilación y modo de uso

# Requisitos del sistema
- **Sistema Operativo**: Linux 
- **Compilador**: GCC (GNU Compiler Collection)

`gcc -o main main.c`
`./main`

### Parte II: Simulador de Memoria Virtual  
Simulador que implementa traducción de direcciones virtuales a físicas utilizando paginación simple y el algoritmo de reemplazo "Reloj" (Clock Algorithm).

## Compilación y modo de uso

`gcc -o sim sim.c`
`./sim Nmarcos tamaño_marco [--verbose] archivo_trace.txt`