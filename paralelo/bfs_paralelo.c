/*
 * BFS paralelo (OpenMP): distancia minima (numero de saltos) entre dos
 * usuarios de una red social representada como grafo.
 *
 * BFS por niveles (level-synchronous): se procesa toda la frontera actual
 * en paralelo antes de avanzar al siguiente nivel. La frontera se reparte
 * entre threads con schedule(dynamic, N) porque el grado de los nodos
 * varia mucho (2 a 10,000 vecinos) y un reparto estatico dejaria a un
 * thread solo con un nodo "hub" mientras los demas ya terminaron.
 *
 * Condicion de carrera: varios threads pueden intentar marcar visitado al
 * mismo vecino v al mismo tiempo. Se usa __sync_bool_compare_and_swap
 * sobre dist[v] para que solo un thread "gane" cada nodo (evita un
 * critical que serializaria todo el ciclo interno). Cada thread acumula
 * los nodos que gana en un buffer local sin sincronizacion; el merge de
 * los buffers a la siguiente frontera se hace secuencialmente entre
 * niveles.
 *
 * Uso: ./bfs_paralelo <n_nodos> <origen> <destino> <seed> [n_threads]
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "../common/graph_gen.h"

#define CHUNK 64

int main(int argc, char **argv) {
    int n      = argc > 1 ? atoi(argv[1]) : 1000000;
    int source = argc > 2 ? atoi(argv[2]) : 0;
    int target = argc > 3 ? atoi(argv[3]) : n - 1;
    unsigned int seed = argc > 4 ? (unsigned int)atoi(argv[4]) : 42u;
    int n_threads = argc > 5 ? atoi(argv[5]) : omp_get_max_threads();

    if (source < 0 || source >= n || target < 0 || target >= n) {
        fprintf(stderr, "origen/destino fuera de rango [0, %d)\n", n);
        return 1;
    }

    omp_set_num_threads(n_threads);

    Graph g = generate_graph(n, seed);

    int *dist = (int *)malloc(sizeof(int) * g.n);
    for (int i = 0; i < g.n; i++) dist[i] = -1;

    /* Frontera actual y siguiente, cada una con capacidad maxima n. */
    int *frontier = (int *)malloc(sizeof(int) * g.n);
    int *next_frontier = (int *)malloc(sizeof(int) * g.n);
    int frontier_size = 0;
    int next_size = 0;

    dist[source] = 0;
    frontier[frontier_size++] = source;

    /* Buffers locales por thread para acumular los nodos ganados en el
     * nivel actual sin contencion; se mezclan a next_frontier despues
     * de la region paralela. */
    int **local_buf = (int **)malloc(sizeof(int *) * n_threads);
    int *local_count = (int *)malloc(sizeof(int) * n_threads);
    for (int t = 0; t < n_threads; t++)
        local_buf[t] = (int *)malloc(sizeof(int) * g.n);

    double t0 = omp_get_wtime();

    int level = 0;
    while (frontier_size > 0 && dist[target] == -1) {
        for (int t = 0; t < n_threads; t++) local_count[t] = 0;

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int *buf = local_buf[tid];
            int cnt = 0;

            #pragma omp for schedule(dynamic, CHUNK)
            for (int i = 0; i < frontier_size; i++) {
                int u = frontier[i];
                for (int e = g.adj_start[u]; e < g.adj_start[u + 1]; e++) {
                    int v = g.adj_list[e];
                    if (dist[v] == -1) {
                        if (__sync_bool_compare_and_swap(&dist[v], -1, level + 1))
                            buf[cnt++] = v;
                    }
                }
            }

            local_count[tid] = cnt;
        }

        next_size = 0;
        for (int t = 0; t < n_threads; t++) {
            for (int i = 0; i < local_count[t]; i++)
                next_frontier[next_size++] = local_buf[t][i];
        }

        int *tmp = frontier;
        frontier = next_frontier;
        next_frontier = tmp;
        frontier_size = next_size;
        level++;
    }

    double t1 = omp_get_wtime();
    double elapsed = t1 - t0;

    if (dist[target] != -1)
        printf("Distancia %d -> %d: %d saltos (tiempo: %f s, threads: %d)\n",
               source, target, dist[target], elapsed, n_threads);
    else
        printf("No existe camino entre %d y %d (tiempo: %f s, threads: %d)\n",
               source, target, elapsed, n_threads);

    for (int t = 0; t < n_threads; t++) free(local_buf[t]);
    free(local_buf);
    free(local_count);
    free(dist);
    free(frontier);
    free(next_frontier);
    free_graph(&g);
    return 0;
}
