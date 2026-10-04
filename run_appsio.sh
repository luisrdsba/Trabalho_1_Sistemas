#!/usr/bin/env bash
# Cenario 3: 3 aplicacoes com CPU + E/S
# Uso: ./run_appsio.sh   (com o ossim ja a correr noutro terminal)
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/run_app.sh" \
    "$DIR/scenarios/3/A.csv" \
    "$DIR/scenarios/3/B.csv" \
    "$DIR/scenarios/3/C.csv"
