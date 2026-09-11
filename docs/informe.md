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
  - Tamaño de muestra: 5,000,000 de nodos, semilla 42 (ajustable en las
    corridas de cada integrante; usar la misma semilla y n en ambas
    versiones para que las mediciones sean comparables).
  - Origen: sintético, generado por `common/graph_gen.h`. Se usa un grafo
    sintético (en vez de un dataset real como SNAP) porque el objetivo del
    ejercicio es medir el efecto del desbalance de carga y la
    paralelización, no analizar una red social real; el generador permite
    controlar explícitamente esa distribución de grado (hubs de hasta
    10,000 vecinos vs. nodos normales de 2-10) y reproducirla de forma
    determinista.
  - Estructura en memoria: representación CSR (`Graph { n, adj_start,
    adj_list }`) — un arreglo de enteros con los vecinos de todos los
    nodos concatenados y un arreglo de offsets por nodo. Se eligió CSR
    (en vez de listas de adyacencia con punteros) porque es contigua en
    memoria, cache-friendly, y evita punteros dispersos que serían
    costosos de recorrer en paralelo.

## 2. Estrategia de Paralelización

- **Directivas de OpenMP usadas:** `#pragma omp parallel` /
  `#pragma omp for schedule(dynamic, 64)` sobre los nodos de la frontera
  actual en cada nivel del BFS (`paralelo/bfs_paralelo.c`). El BFS es
  inherentemente por niveles (level-synchronous): toda la frontera actual
  se procesa en paralelo antes de avanzar al siguiente nivel, porque el
  nivel N+1 depende de haber terminado de expandir el nivel N.
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
y reportar sus propias mediciones. **Nota:** los datos de esta sección
son una corrida de referencia (n=5,000,000, origen=0, destino=n-1,
seed=42, promedio de 3 repeticiones) hecha en una sola máquina (Apple
M3, 8 cores, macOS). Cada integrante debe **reemplazar su tabla** con
sus propias mediciones y su propio screenshot/video — es el requisito
individual del examen; los comandos exactos están en
`docs/evidencia/comandos.md`.

### Integrante 1: [Nombre completo]

| Threads | Tiempo secuencial (s) | Tiempo paralelo (s) | Speedup | Eficiencia |
|---------|------------------------|----------------------|---------|------------|
| 1       | 0.172895               | 0.207193             | 0.83    | 0.83       |
| 2       | 0.172895               | 0.117680             | 1.47    | 0.73       |
| 4       | 0.172895               | 0.061275             | 2.82    | 0.71       |
| 8       | 0.172895               | 0.050289             | 3.44    | 0.43       |

- Speedup = tiempo_secuencial / tiempo_paralelo
- Eficiencia = Speedup / n_threads
- [Reemplazar con tu propia corrida y adjuntar screenshot/video en
  `docs/evidencia/`.]

### Integrante 2: [Nombre completo]

| Threads | Tiempo secuencial (s) | Tiempo paralelo (s) | Speedup | Eficiencia |
|---------|------------------------|----------------------|---------|------------|
| 1       |                        |                      |         |            |
| 2       |                        |                      |         |            |
| 4       |                        |                      |         |            |
| 8       |                        |                      |         |            |

- [Correr los mismos comandos en tu propia máquina y adjuntar
  screenshot/video de las ejecuciones en `docs/evidencia/`.]

### Análisis

Con n=5,000,000 nodos (seed=42) el speedup crece de forma sub-lineal
respecto al ideal (speedup = n_threads):

- **1 thread vs. secuencial:** el paralelo con 1 thread es más lento que
  el secuencial puro (0.83x) por el overhead propio de OpenMP (crear la
  región paralela en cada nivel del BFS, aunque solo haya un thread) y
  por el costo extra de la operación atómica `__sync_bool_compare_and_swap`
  sobre `dist[v]`, que el secuencial no necesita.
- **2 y 4 threads:** el speedup escala bien (1.47x y 2.82x, eficiencia
  ~0.73-0.71), porque el BFS por niveles tiene bastante paralelismo
  disponible en niveles intermedios grandes y el `schedule(dynamic, 64)`
  reparte los nodos "hub" sin que un solo thread quede saturado.
- **8 threads:** el speedup sigue subiendo (3.44x) pero la eficiencia cae
  a 0.43. Con más threads, más de ellos compiten por el mismo nodo `v`
  vía CAS cuando varios nodos de la frontera comparten vecinos (más
  probable en un grafo con hubs de grado muy alto), y aumenta el
  porcentaje de tiempo relativo que toma el merge secuencial de los
  buffers locales a `next_frontier` entre niveles (esa parte no está
  paralelizada). También el hardware de prueba tiene 8 cores totales
  (4 de rendimiento + 4 de eficiencia en Apple M3), por lo que no todos
  los threads corren a la misma velocidad, lo cual reduce la eficiencia
  esperada al usar todos los cores.
- **Efecto del tamaño del grafo:** con grafos más pequeños el overhead de
  crear/sincronizar threads en cada nivel pesa más relativo al trabajo
  útil, por lo que el punto de equilibrio (donde paralelo empieza a ganar
  al secuencial) aparece en grafos de varios millones de nodos; con
  grafos aún más grandes se espera que la eficiencia mejore ligeramente
  porque el trabajo por nivel amortiza mejor el overhead fijo.
