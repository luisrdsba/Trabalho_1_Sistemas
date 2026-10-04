#!/usr/bin/env bash
# Cenario 4: 3 aplicacoes com CPU + E/S, maior contencao
# Uso: ./run_appsio2.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/4/A.csv" \
    "$DIR/scenarios/4/B.csv" \
    "$DIR/scenarios/4/C.csv"
