# Comandos para las capturas de pantalla (requisito individual)

Cada integrante debe correr esto en **su propia máquina**, con **su propio
usuario/terminal**, y guardar el screenshot (o video) de la sesión en esta
carpeta (`docs/evidencia/`), por ejemplo `evidencia-<tu-nombre>.png`.

## 1. Compilar

Desde la raíz del repo:

```bash
make clean
make
make paralelo
```

(Necesitas `gcc` con soporte OpenMP: en Linux/WSL suele venir por defecto;
en Windows usa MinGW-w64. En macOS con Apple Clang, si `make paralelo`
falla por `-fopenmp`, instala `libomp` con Homebrew y compila así en su
lugar:

```bash
brew install libomp
clang -O2 -Wall -Xpreprocessor -fopenmp \
  -I$(brew --prefix libomp)/include -L$(brew --prefix libomp)/lib -lomp \
  -o paralelo/bfs_paralelo paralelo/bfs_paralelo.c
```
)

## 2. Correr las 5 ejecuciones que van en la tabla del informe

Usa el mismo `n`, origen, destino y semilla que aparecen en
`docs/informe.md` (n=5,000,000, origen=0, destino=4999999, seed=42) para
que tus números sean comparables con los de tu compañero:

```bash
./secuencial/bfs_secuencial 5000000 0 4999999 42

./paralelo/bfs_paralelo 5000000 0 4999999 42 1
./paralelo/bfs_paralelo 5000000 0 4999999 42 2
./paralelo/bfs_paralelo 5000000 0 4999999 42 4
./paralelo/bfs_paralelo 5000000 0 4999999 42 8
```

Deja las 5 líneas de salida visibles en la misma captura/terminal (o
grábalas en un solo video corriendo los comandos en orden) — eso es lo
que sirve como evidencia individual.

## 3. (Opcional pero recomendado) Repetir 3 veces y promediar

El ruido del sistema puede hacer variar el tiempo de una corrida a otra.
Para números más estables, corre cada comando 3 veces y promedia el
tiempo antes de llenar tu tabla:

```bash
for i in 1 2 3; do ./secuencial/bfs_secuencial 5000000 0 4999999 42; done

for T in 1 2 4 8; do
  echo "threads=$T"
  for i in 1 2 3; do ./paralelo/bfs_paralelo 5000000 0 4999999 42 $T; done
done
```

## 4. Llenar tu tabla en docs/informe.md

Con los tiempos obtenidos:

- Speedup = tiempo_secuencial / tiempo_paralelo
- Eficiencia = Speedup / n_threads

Reemplaza tu sección ("Integrante 1" o "Integrante 2") en
`docs/informe.md` con tus propios números y agrega el nombre de tu
archivo de evidencia (screenshot/video) junto a la tabla.
