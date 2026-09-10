/*
 * BFS secuencial: distancia minima (numero de saltos) entre dos usuarios
 * de una red social representada como grafo.
 *
 * Uso: ./bfs_secuencial <n_nodos> <origen> <destino> <seed>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../common/graph_gen.h"

int main(int argc, char **argv) {
    int n      = argc > 1 ? atoi(argv[1]) : 1000000;
    int source = argc > 2 ? atoi(argv[2]) : 0;
    int target = argc > 3 ? atoi(argv[3]) : n - 1;
    unsigned int seed = argc > 4 ? (unsigned int)atoi(argv[4]) : 42u;

    if (source < 0 || source >= n || target < 0 || target >= n) {
        fprintf(stderr, "origen/destino fuera de rango [0, %d)\n", n);
        return 1;
    }

    Graph g = generate_graph(n, seed);

    int *dist = (int *)malloc(sizeof(int) * g.n);
    for (int i = 0; i < g.n; i++) dist[i] = -1;

    int *queue = (int *)malloc(sizeof(int) * g.n);
    int head = 0, tail = 0;

    dist[source] = 0;
    queue[tail++] = source;

    clock_t t0 = clock();

    while (head < tail && dist[target] == -1) {
        int u = queue[head++];
        for (int e = g.adj_start[u]; e < g.adj_start[u + 1]; e++) {
            int v = g.adj_list[e];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                queue[tail++] = v;
            }
        }
    }

    clock_t t1 = clock();
    double elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;

    if (dist[target] != -1)
        printf("Distancia %d -> %d: %d saltos (tiempo: %f s)\n",
               source, target, dist[target], elapsed);
    else
        printf("No existe camino entre %d y %d (tiempo: %f s)\n",
               source, target, elapsed);

    free(dist);
    free(queue);
    free_graph(&g);
    return 0;
}
