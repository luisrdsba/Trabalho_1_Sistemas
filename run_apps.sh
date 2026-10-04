#!/usr/bin/env bash
# Cenario 1: A=10 s, B=15 s, C=20 s (so CPU)
# Uso: ./run_apps.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/1/A.csv" \
    "$DIR/scenarios/1/B.csv" \
    "$DIR/scenarios/1/C.csv"
