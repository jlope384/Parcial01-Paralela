#ifndef GRAPH_GEN_H
#define GRAPH_GEN_H

#include <stdlib.h>

/*
 * Grafo dirigido (lista de "sigue a" / amistades salientes) en formato CSR
 * (Compressed Sparse Row):
 *   adj_start[u] .. adj_start[u+1]-1  son los indices en adj_list
 *   correspondientes a los vecinos de u.
 */
typedef struct {
    int n;
    int *adj_start;
    int *adj_list;
} Graph;

/*
 * Genera un grafo sintetico tipo "red social": la mayoria de nodos tiene
 * pocos vecinos (2-10) y una pequena fraccion de nodos "hub" tiene miles
 * de vecinos, para reproducir el desbalance de carga descrito en el
 * enunciado (Problema 4). La misma semilla siempre produce el mismo
 * grafo, para que secuencial y paralelo se comparen sobre datos identicos.
 */
static Graph generate_graph(int n, unsigned int seed) {
    srand(seed);

    int hub_spacing = n / 1000;
    if (hub_spacing < 1) hub_spacing = 1;

    int *degree = (int *)malloc(sizeof(int) * n);

    for (int i = 0; i < n; i++) {
        int is_hub = (i % hub_spacing) == 0;
        int max_hub_degree = n - 1 < 10000 ? n - 1 : 10000;
        degree[i] = is_hub ? (2 + rand() % max_hub_degree)
                            : (2 + rand() % 9);
        if (degree[i] > n - 1) degree[i] = n - 1;
        if (degree[i] < 0) degree[i] = 0;
    }

    int *adj_start = (int *)malloc(sizeof(int) * (n + 1));
    adj_start[0] = 0;
    for (int i = 0; i < n; i++) adj_start[i + 1] = adj_start[i] + degree[i];

    int *adj_list = (int *)malloc(sizeof(int) * adj_start[n]);
    int *fill_pos = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) fill_pos[i] = adj_start[i];

    for (int u = 0; u < n; u++) {
        while (fill_pos[u] < adj_start[u + 1]) {
            int v = rand() % n;
            if (v == u) continue;
            adj_list[fill_pos[u]++] = v;
        }
    }

    free(degree);
    free(fill_pos);

    Graph g;
    g.n = n;
    g.adj_start = adj_start;
    g.adj_list = adj_list;
    return g;
}

static void free_graph(Graph *g) {
    free(g->adj_start);
    free(g->adj_list);
    g->adj_start = NULL;
    g->adj_list = NULL;
}

#endif
