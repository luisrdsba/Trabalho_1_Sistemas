#!/usr/bin/env bash
# Ponto 3c-i: MLFQ com 2 processos interativos + 1 batch
# Uso: ./run_mlfq1.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/mlfq1/A_batch.csv" \
    "$DIR/scenarios/mlfq1/B_inter.csv" \
    "$DIR/scenarios/mlfq1/C_inter.csv"
