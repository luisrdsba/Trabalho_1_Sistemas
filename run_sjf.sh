#!/usr/bin/env bash
# Ponto 3a: 5 processos em que o SJF tem tempo medio de execucao muito menor que o FIFO
# Uso: ./run_sjf.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/sjf/A.csv" \
    "$DIR/scenarios/sjf/B.csv" \
    "$DIR/scenarios/sjf/C.csv" \
    "$DIR/scenarios/sjf/D.csv" \
    "$DIR/scenarios/sjf/E.csv"
