CC = gcc
CFLAGS = -O2 -Wall

# En Linux/WSL, gcc trae soporte OpenMP con -fopenmp. En macOS, "gcc" es
# en realidad Apple Clang, que no soporta -fopenmp directamente: hace
# falta libomp (brew install libomp) y flags distintos.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
	OMPFLAGS = -Xpreprocessor -fopenmp -I$(OMP_PREFIX)/include -L$(OMP_PREFIX)/lib -lomp
else
	OMPFLAGS = -fopenmp
endif

all: secuencial

secuencial: secuencial/bfs_secuencial

secuencial/bfs_secuencial: secuencial/bfs_secuencial.c common/graph_gen.h
	$(CC) $(CFLAGS) -o $@ secuencial/bfs_secuencial.c

paralelo: paralelo/bfs_paralelo

paralelo/bfs_paralelo: paralelo/bfs_paralelo.c common/graph_gen.h
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ paralelo/bfs_paralelo.c

clean:
	rm -f secuencial/bfs_secuencial paralelo/bfs_paralelo

.PHONY: all clean secuencial paralelo
