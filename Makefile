CC = gcc
CFLAGS = -O2 -Wall
OMPFLAGS = -fopenmp

all: secuencial

secuencial: secuencial/bfs_secuencial

secuencial/bfs_secuencial: secuencial/bfs_secuencial.c common/graph_gen.h
	$(CC) $(CFLAGS) -o $@ secuencial/bfs_secuencial.c

# paralelo/bfs_paralelo.c todavia no existe (Parte 2, ver INSTRUCCIONES-PARTE2.md).
paralelo: paralelo/bfs_paralelo

paralelo/bfs_paralelo: paralelo/bfs_paralelo.c common/graph_gen.h
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ paralelo/bfs_paralelo.c

clean:
	rm -f secuencial/bfs_secuencial paralelo/bfs_paralelo

.PHONY: all clean secuencial paralelo
