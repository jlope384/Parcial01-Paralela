# Informe — Problema 4: Búsqueda de Ruta Mínima (Grafos)

## 1. Contexto y Datos

- **Problema elegido:** camino más corto entre dos usuarios (Usuario X,
  Usuario Y) en una red social, explorando vecinos con BFS por niveles.
- **Propuesta secuencial:** BFS clásico con cola FIFO (`secuencial/bfs_secuencial.c`),
  deteniéndose en cuanto se extrae el nodo destino de la cola.
- **Datos de prueba:** grafo sintético generado en `common/graph_gen.h`
  (formato CSR), con distribución de grado desigual para imitar una red
  social real: ~0.1% de nodos "hub" con hasta 10,000 vecinos y el resto
  con 2-10 vecinos. Se genera con una semilla fija para que secuencial y
  paralelo trabajen sobre el mismo grafo.
  - Tamaño de muestra: [completar — n_nodos usado en las pruebas].
  - Origen: sintético (no hay dataset real disponible); [justificar por
    qué el grafo sintético es representativo, o reemplazar por un
    dataset real como SNAP si se usa uno].

## 2. Estrategia de Paralelización

- **Directivas de OpenMP usadas:** `#pragma omp parallel` /
  `#pragma omp for schedule(dynamic, 64)` sobre los nodos de la frontera
  actual en cada nivel del BFS (`paralelo/bfs_paralelo.c`).
- **Por qué `dynamic`:** el grado de los nodos varía mucho (2 a 10,000
  vecinos); con `schedule(static)` un thread podría recibir un nodo hub
  y quedar procesando miles de vecinos mientras los demás threads
  terminan casi de inmediato. `dynamic` redistribuye trabajo en chunks
  pequeños a medida que cada thread se desocupa.
- **Condiciones de carrera:** dos nodos de la misma frontera pueden
  compartir un vecino `v`. Se usa `__sync_bool_compare_and_swap` sobre
  `dist[v]` para que solo un thread "gane" la visita de `v`. Cada thread
  acumula los nodos que gana en un buffer local (sin sincronización); el
  merge de los buffers a la siguiente frontera se hace secuencialmente
  entre niveles, evitando contención en una estructura compartida.

## 3. Resultados y Métricas (por integrante)

Cada integrante debe correr ambas versiones con el mismo tamaño de grafo
y reportar sus propias mediciones.

### Integrante 1: [Nombre completo]

| Threads | Tiempo secuencial (s) | Tiempo paralelo (s) | Speedup | Eficiencia |
|---------|------------------------|----------------------|---------|------------|
| 1       |                        |                      |         |            |
| 2       |                        |                      |         |            |
| 4       |                        |                      |         |            |
| 8       |                        |                      |         |            |

- Speedup = tiempo_secuencial / tiempo_paralelo
- Eficiencia = Speedup / n_threads
- [Adjuntar screenshot o video de las corridas.]

### Integrante 2: [Nombre completo]

| Threads | Tiempo secuencial (s) | Tiempo paralelo (s) | Speedup | Eficiencia |
|---------|------------------------|----------------------|---------|------------|
| 1       |                        |                      |         |            |
| 2       |                        |                      |         |            |
| 4       |                        |                      |         |            |
| 8       |                        |                      |         |            |

- [Adjuntar screenshot o video de las corridas.]

### Análisis

[Discutir: ¿el speedup es cercano al ideal? ¿Dónde se pierde eficiencia
(overhead de crear threads, merge secuencial de buffers, contención de
CAS en nodos muy compartidos)? ¿Cómo cambia con el tamaño del grafo o el
número de threads?]
