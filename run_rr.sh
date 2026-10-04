#!/usr/bin/env bash
# Ponto 3b: 5 processos em que o RR tem tempo de resposta muito menor que FIFO e SJF
# Uso: ./run_rr.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/rr/A.csv" \
    "$DIR/scenarios/rr/B.csv" \
    "$DIR/scenarios/rr/C.csv" \
    "$DIR/scenarios/rr/D.csv" \
    "$DIR/scenarios/rr/E.csv"
