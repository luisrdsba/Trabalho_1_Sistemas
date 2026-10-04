#!/usr/bin/env bash
# Cenario 2: A=5 s, B=10 s, C=4 s, D=2 s, E=3 s, F=15 s (so CPU)
# Uso: ./run_apps2.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/2/A.csv" \
    "$DIR/scenarios/2/B.csv" \
    "$DIR/scenarios/2/C.csv" \
    "$DIR/scenarios/2/D.csv" \
    "$DIR/scenarios/2/E.csv" \
    "$DIR/scenarios/2/F.csv"
