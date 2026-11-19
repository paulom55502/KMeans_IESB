#!/usr/bin/env bash
set -euo pipefail
mkdir -p resultados
CSV="resultados/medicoes.csv"
echo "versao,N,D,K,max_iter,workers,tempo_segundos" > "$CSV"

extrair_tempo() {
  grep '^N=' | sed -E 's/.*TEMPO=([0-9.]+).*/\1/'
}
for n in 10000 50000 100000; do
  out=$(./sequencial/kmeans "$n" 4 5 100)
  tempo=$(printf '%s\n' "$out" | extrair_tempo)
  echo "sequencial,$n,4,5,100,1,$tempo" >> "$CSV"

  for t in 1 2 4 8; do
    out=$(OMP_NUM_THREADS="$t" ./openmp/kmeans_openmp "$n" 4 5 100)
    tempo=$(printf '%s\n' "$out" | extrair_tempo)
    echo "openmp,$n,4,5,100,$t,$tempo" >> "$CSV"
  done

  for p in 1 2 4; do
    out=$(mpirun -np "$p" ./mpi/kmeans_mpi "$n" 4 5 100)
    tempo=$(printf '%s\n' "$out" | extrair_tempo)
    echo "mpi,$n,4,5,100,$p,$tempo" >> "$CSV"
  done
done
echo "CSV salvo em $CSV"
