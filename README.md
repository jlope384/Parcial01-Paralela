# [Nombre Consultora HPC]

**Integrantes:**
- [Nombre completo Integrante 1]
- [Nombre completo Integrante 2]

**Grupo 12 — Problema 4: Búsqueda de Ruta Mínima (Grafos)**

Búsqueda del camino más corto entre dos usuarios en una red social (grafo con
distribución de grado muy desigual: algunos nodos con 2 vecinos, otros con
miles) usando BFS por niveles, secuencial y paralelizado con OpenMP.

## Estructura del repositorio

- `/secuencial` — Implementación BFS secuencial (baseline).
- `/paralelo` — Implementación BFS paralela con OpenMP.
- `/common` — Generador de grafo sintético compartido, para que ambas
  versiones se midan sobre exactamente el mismo grafo (misma semilla).
- `/docs` — Enunciado, informe y resultados (speedup/eficiencia por
  integrante).

## Compilación

```
make            # compila la version secuencial
make paralelo   # una vez exista paralelo/bfs_paralelo.c (Parte 2)
make clean
```

## Ejecución

```
./secuencial/bfs_secuencial <n_nodos> <origen> <destino> <seed>
./paralelo/bfs_paralelo     <n_nodos> <origen> <destino> <seed> [n_threads]
```

Ambos programas imprimen la distancia (número de saltos) encontrada y el
tiempo de ejecución.

## Estado del trabajo

- **Parte 1 (lista):** estructura del repo, generador de grafo compartido
  (`common/graph_gen.h`), implementación secuencial (`secuencial/`),
  Makefile y esqueleto del informe.
- **Parte 2 (pendiente):** implementación paralela con OpenMP
  (`paralelo/`) y las secciones de estrategia/resultados del informe.
  Instrucciones completas en [`INSTRUCCIONES-PARTE2.md`](INSTRUCCIONES-PARTE2.md).
