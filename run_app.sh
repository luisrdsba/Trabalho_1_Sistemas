#!/usr/bin/env bash
# Lança uma application por cada ficheiro .csv, pela ordem dada, com um pequeno
# atraso entre lançamentos (DELAY, por omissão 0.1 s) para que a ordem de
# chegada ao ossim seja sempre a mesma. Espera que todas terminem.
#
#   ./run_app.sh f1.csv                  # uma aplicação
#   ./run_app.sh f1.csv f2.csv f3.csv    # várias em simultâneo
#   ./run_app.sh -n 3 f1.csv             # 3 cópias da mesma aplicação
#   DELAY=0.5 ./run_app.sh f1.csv f2.csv # outro atraso entre lançamentos
#
# O simulador (./build/ossim --sched ...) tem de estar já a correr.

DIR="$(cd "$(dirname "$0")" && pwd)"
APP="$DIR/build/application"
DELAY="${DELAY:-0.1}"
N=1

if [ "${1:-}" = "-n" ]; then
    N="$2"
    shift 2
fi

if [ $# -lt 1 ]; then
    echo "Uso: $0 [-n N] ficheiro.csv [ficheiro2.csv ...]" >&2
    exit 1
fi
if [ ! -x "$APP" ]; then
    echo "Erro: $APP nao existe. Compile primeiro: cmake -S . -B build && cmake --build build" >&2
    exit 1
fi
if [ ! -S /tmp/scheduler.sock ]; then
    echo "Aviso: /tmp/scheduler.sock nao existe - o ossim esta a correr?" >&2
fi

first=1
for f in "$@"; do
    for ((i = 0; i < N; i++)); do
        [ $first -eq 1 ] || sleep "$DELAY"
        first=0
        "$APP" "$f" &
    done
done
wait
