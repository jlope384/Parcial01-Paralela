# Parte 2 — Implementación paralela (OpenMP)

Estas instrucciones son para pegarlas en Claude Code (u otro asistente)
para completar la segunda mitad del proyecto. Léelas también tú antes de
correr nada, para poder explicar el código en la sustentación.

## Contexto del examen

Examen Parcial 1 de Computación Paralela (HPC / OpenMP), grupo 12,
**Problema 4: Búsqueda de Ruta Mínima (Grafos)**. Dado el mapa de una red
social (nodos = usuarios, aristas = amistades, grado muy desigual: de 2 a
10,000 amigos), hay que encontrar el camino más corto entre el Usuario X
y el Usuario Y explorando vecinos paso a paso con una cola (BFS), sin que
un thread termine en un segundo y otro se quede procesando a un usuario
con 10,000 amistades él solo.

Se trabaja en parejas y cada integrante debe hacer al menos un commit
propio. La Parte 1 (ya hecha, ver commits del repo) cubrió la
infraestructura y el baseline secuencial. Esta Parte 2 es tu commit.

## Qué ya existe (no lo reimplementes, reutilízalo)

- `common/graph_gen.h` — generador de un grafo sintético tipo red social
  en formato CSR (`Graph { n, adj_start, adj_list }`), con una fracción
  de nodos "hub" de grado alto para reproducir el desbalance de carga.
  `generate_graph(n, seed)` siempre produce el mismo grafo para la misma
  semilla — **inclúyelo tal cual** desde `paralelo/` para que las
  mediciones de speedup comparen exactamente el mismo grafo que usó la
  versión secuencial.
- `secuencial/bfs_secuencial.c` — BFS secuencial de referencia. Úsalo
  como base de interfaz (mismos argumentos de línea de comandos, mismo
  formato de salida) para que los tiempos sean comparables.
- `Makefile` — ya tiene un target `paralelo` que compila
  `paralelo/bfs_paralelo.c` con `-fopenmp` en cuanto exista el archivo.

## Qué tienes que hacer

1. **Crear `paralelo/bfs_paralelo.c`**, un BFS paralelo con OpenMP que:
   - Incluya `../common/graph_gen.h` y reciba los mismos argumentos que
     `secuencial/bfs_secuencial.c`: `<n_nodos> <origen> <destino> <seed>`,
     más opcionalmente `<n_threads>`.
   - Implemente BFS **por niveles** (level-synchronous): se procesa toda
     la frontera actual en paralelo antes de avanzar al siguiente nivel
     (el BFS es inherentemente por niveles, no se puede paralelizar nodo
     por nodo de forma independiente).
   - Reparta los nodos de la frontera entre threads con
     `#pragma omp parallel for schedule(dynamic, N)` (no `static`):
     como el grado varía de 2 a 10,000, un reparto estático dejaría a un
     thread solo con un nodo "hub" mientras los demás ya terminaron.
     Justifica en el informe por qué `dynamic` resuelve esto.
   - Evite condiciones de carrera: varios threads pueden intentar marcar
     visitado al mismo vecino `v` al mismo tiempo. Usa una operación
     atómica de compara-e-intercambia (por ejemplo
     `__sync_bool_compare_and_swap(&dist[v], -1, nivel_actual + 1)`, o el
     equivalente con `#pragma omp atomic` / `omp_lock_t` si prefieres esa
     API) para que solo un thread "gane" cada nodo. Evita un
     `#pragma omp critical` que envuelva todo el cuerpo del ciclo interno
     — eso serializaría el trabajo y anularía el paralelismo.
   - Imprima el mismo tipo de salida que la versión secuencial (distancia
     encontrada y tiempo de ejecución con `omp_get_wtime()`), agregando el
     número de threads usado.
   - No modifiques `common/graph_gen.h` ni `secuencial/bfs_secuencial.c`
     salvo que encuentres un bug real — si lo haces, dilo explícitamente
     al equipo antes de commitear.

2. **Compilar y correr benchmarks** con `make paralelo` (necesitas un
   compilador con soporte OpenMP: `gcc` en Linux/WSL, o MinGW-w64 en
   Windows). Corre ambas versiones (`secuencial` y `paralelo`) sobre el
   mismo `n_nodos`/`seed` que se haya usado en la sección "Contexto y
   Datos" de `docs/informe.md`, variando threads (al menos 1, 2, 4, 8).

3. **Completar `docs/informe.md`**:
   - Sección "Estrategia de Paralelización": ya tiene una redacción base
     que puedes ajustar a lo que realmente implementaste.
   - Sección "Resultados y Métricas": llenar la tabla de speedup y
     eficiencia **por cada integrante, con sus propias corridas**
     (requisito individual del examen — no basta con correrlo una sola
     vez y copiar los números). Adjuntar screenshot o video de las
     ejecuciones de cada quien (puede ir en `docs/evidencia/`).
   - Sección "Análisis": comentar dónde se pierde eficiencia (overhead de
     threads, el merge secuencial de buffers entre niveles, contención en
     el CAS sobre nodos muy compartidos, etc.).

4. **README.md**: reemplaza los placeholders `[Nombre Consultora HPC]` y
   los dos `[Nombre completo Integrante N]` si todavía no se hizo.

5. **Commit**: usa tu propio `git config user.name`/`user.email` (no el
   de tu compañero) al clonar y commitear, para que el commit quede a tu
   nombre — es un requisito individual del examen.

## Cómo probar que funciona

Corre primero con un grafo chico para verificar correctud contra la
versión secuencial (misma semilla ⇒ mismo grafo ⇒ misma distancia):

```
make
make paralelo
./secuencial/bfs_secuencial 10000 0 9999 42
./paralelo/bfs_paralelo     10000 0 9999 42 4
```

Las dos deben imprimir la misma distancia (el tiempo sí puede diferir).
Si difieren, hay un bug en el manejo de la frontera o en la condición de
carrera del BFS paralelo.
