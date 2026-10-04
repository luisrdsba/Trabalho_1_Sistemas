#!/usr/bin/env bash
# Ponto 3c-ii: MLFQ com 2 interativos + 1 batch->interativo->batch
# Uso: ./run_mlfq2.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/mlfq2/A_misto.csv" \
    "$DIR/scenarios/mlfq2/B_inter.csv" \
    "$DIR/scenarios/mlfq2/C_inter.csv"
