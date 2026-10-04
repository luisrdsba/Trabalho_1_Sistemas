#!/usr/bin/env bash
# Corre cada cenário com cada algoritmo N vezes, guarda os logs e gera as
# tabelas do relatório (médias das N repetições).
#
#   ./run_all.sh                          # tudo: 8 cenários x 4 algoritmos x 3 vezes (~40 min)
#   ./run_all.sh -r 1 -s "sjf rr"         # só alguns cenários, 1 vez
#   ./run_all.sh -a "RR MLFQ" -s mlfq1    # só alguns algoritmos
#   ./run_all.sh -o results/final -s 1    # acrescenta a uma pasta já existente
#   ./run_all.sh -o results/final -s 1 -a RR -r 1 -k 3   # refaz só a repetição 3 (RR_run3)
#   ./run_all.sh -t results/final         # só volta a gerar as tabelas a partir do raw.csv
#
# Resultados em OUT (por omissão results/AAAAMMDD-HHMMSS/):
#   <cenario>/<ALGO>_run<k>.ossim.log   log completo do simulador (decisões + resumo)
#   <cenario>/<ALGO>_run<k>.apps.log    saída das applications (Elapsed/CPU/BLOCKED)
#   raw.csv                             uma linha por aplicação e repetição
#   tabelas.md                          tabelas por cenário + tabela resumo
#
# Cenários: 1 2 3 4 (enunciado), sjf (3a), rr (3b), mlfq1 (3c-i), mlfq2 (3c-ii)

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR" || exit 1

RUNS=3
FIRST_RUN=1     # número da primeira repetição (-k), para refazer uma repetição concreta
ALGOS="FIFO SJF RR MLFQ"
SCENS="1 2 3 4 sjf rr mlfq1 mlfq2"
OUT="results/$(date +%Y%m%d-%H%M%S)"
TABLES_ONLY=0
SOCK=/tmp/scheduler.sock

usage() { sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'; exit 1; }

while getopts "r:k:a:s:o:t:h" opt; do
    case $opt in
        r) RUNS="$OPTARG" ;;
        k) FIRST_RUN="$OPTARG" ;;
        a) ALGOS="$OPTARG" ;;
        s) SCENS="$OPTARG" ;;
        o) OUT="$OPTARG" ;;
        t) OUT="$OPTARG"; TABLES_ONLY=1 ;;
        *) usage ;;
    esac
done

script_for() {
    case "$1" in
        1) echo run_apps.sh ;;
        2) echo run_apps2.sh ;;
        3) echo run_appsio.sh ;;
        4) echo run_appsio2.sh ;;
        sjf) echo run_sjf.sh ;;
        rr) echo run_rr.sh ;;
        mlfq1) echo run_mlfq1.sh ;;
        mlfq2) echo run_mlfq2.sh ;;
        *) return 1 ;;
    esac
}

# ---------------------------------------------------------------------------
# Tabelas: médias por (cenário, aplicação, algoritmo) e resumo por cenário
# ---------------------------------------------------------------------------
make_tables() {
    awk -F',' -v algos="FIFO SJF RR MLFQ" '
    function v(sum, n) { return n ? sprintf("%.3f", sum / n) : "n.d." }
    NR == 1 { next }                                   # cabeçalho
    {
        s = $1; a = $2; app = $4
        if (!(s in seen_s)) { seen_s[s] = 1; scen[++ns] = s }
        k = s SUBSEP app
        if (!(k in seen_app)) { seen_app[k] = 1; napp[s]++; apps[s, napp[s]] = app }
        if ($5 != "-") { resp[k, a] += $5; nresp[k, a]++ }
        if ($6 != "-") { rb[k, a] += $6;  nrb[k, a]++ }
        if ($7 != "-") { ex[k, a] += $7;  nex[k, a]++ }
        runs[s, a, $3] = 1
    }
    END {
        na = split(algos, A, " ")
        hdr = "| Aplicação | Métrica (s) |"; sep = "|---|---|"
        for (j = 1; j <= na; j++) { hdr = hdr " " A[j] " |"; sep = sep "---|" }

        for (i = 1; i <= ns; i++) {
            s = scen[i]
            printf "## Cenário %s\n\n", s
            line = "Repetições:"
            for (j = 1; j <= na; j++) {
                c = 0; for (key in runs) { split(key, p, SUBSEP); if (p[1] == s && p[2] == A[j]) c++ }
                line = line " " A[j] "=" c
            }
            print line "\n"
            print hdr; print sep
            for (m = 1; m <= napp[s]; m++) {
                k = s SUBSEP apps[s, m]
                l1 = "| " apps[s, m] " | Tempo de execução |"
                l2 = "| " apps[s, m] " | Tempo de resposta |"
                l3 = "| " apps[s, m] " | Tempo de resposta médio por burst |"
                for (j = 1; j <= na; j++) {
                    a = A[j]
                    l1 = l1 " " v(ex[k, a], nex[k, a]) " |"
                    l2 = l2 " " v(resp[k, a], nresp[k, a]) " |"
                    l3 = l3 " " v(rb[k, a], nrb[k, a]) " |"
                    # média por algoritmo = média das médias de cada aplicação
                    if (nex[k, a])   { sex[s, a] += ex[k, a] / nex[k, a];     cex[s, a]++ }
                    if (nresp[k, a]) { sresp[s, a] += resp[k, a] / nresp[k, a]; cresp[s, a]++ }
                    if (nrb[k, a])   { srb[s, a] += rb[k, a] / nrb[k, a];      crb[s, a]++ }
                }
                print l1; print l2; print l3
            }
            l1 = "| **Média** | Tempo de execução |"
            l2 = "| **Média** | Tempo de resposta |"
            l3 = "| **Média** | Tempo de resposta médio por burst |"
            for (j = 1; j <= na; j++) {
                a = A[j]
                l1 = l1 " " v(sex[s, a], cex[s, a]) " |"
                l2 = l2 " " v(sresp[s, a], cresp[s, a]) " |"
                l3 = l3 " " v(srb[s, a], crb[s, a]) " |"
            }
            print l1; print l2; print l3; print ""
        }

        print "## Resumo: tempos médios por cenário (s)\n"
        h = "| Cenário | Métrica |"; sp = "|---|---|"
        for (j = 1; j <= na; j++) { h = h " " A[j] " |"; sp = sp "---|" }
        print h; print sp
        for (i = 1; i <= ns; i++) {
            s = scen[i]
            l1 = "| " s " | T. médio de execução |"
            l2 = "| " s " | T. médio de resposta |"
            l3 = "| " s " | T. médio de resposta por burst |"
            for (j = 1; j <= na; j++) {
                a = A[j]
                l1 = l1 " " v(sex[s, a], cex[s, a]) " |"
                l2 = l2 " " v(sresp[s, a], cresp[s, a]) " |"
                l3 = l3 " " v(srb[s, a], crb[s, a]) " |"
            }
            print l1; print l2; print l3
        }
        print "\nT. resposta = 1.ª execução - chegada. T. resposta por burst = média de (despacho - pedido RUN) em todos os bursts."
        print "T. execução = fim - chegada (= Elapsed da application). Valores = média das repetições."
    }' "$OUT/raw.csv" > "$OUT/tabelas.md"
}

if [ $TABLES_ONLY -eq 1 ]; then
    [ -f "$OUT/raw.csv" ] || { echo "Nao existe $OUT/raw.csv" >&2; exit 1; }
    make_tables
    echo "Tabelas em $OUT/tabelas.md"
    exit 0
fi

# ---------------------------------------------------------------------------
# Execução
# ---------------------------------------------------------------------------
# (ignora processos zombie: no contentor o PID 1 é "sleep infinity" e não os recolhe)
running=$(ps -eo pid=,stat=,comm= | awk '$2 !~ /^Z/ && $3 == "ossim" { print $1 }' | tr '\n' ' ')
if [ -n "$running" ]; then
    echo "Ja ha um ossim a correr (pid $running). Termine-o primeiro." >&2
    exit 1
fi
if [ ! -x build/ossim ] || [ ! -x build/application ]; then
    cmake -S . -B build > /dev/null && cmake --build build || exit 1
fi
for s in $SCENS; do
    script_for "$s" > /dev/null || { echo "Cenario desconhecido: $s" >&2; exit 1; }
done

mkdir -p "$OUT"
RAW="$OUT/raw.csv"
[ -f "$RAW" ] || echo "cenario,algoritmo,run,app,resposta,resp_burst,execucao" > "$RAW"

OSSIM_PID=""
cleanup() {
    [ -n "$OSSIM_PID" ] && kill -TERM "$OSSIM_PID" 2> /dev/null
    pkill -f "$DIR/build/application" 2> /dev/null
    exit 130
}
trap cleanup INT TERM

total=0; for s in $SCENS; do for a in $ALGOS; do total=$((total + RUNS)); done; done
n=0

for s in $SCENS; do
    script=$(script_for "$s")
    mkdir -p "$OUT/$s"
    for a in $ALGOS; do
        for r in $(seq "$FIRST_RUN" $((FIRST_RUN + RUNS - 1))); do
            n=$((n + 1))
            base="$OUT/$s/${a}_run$r"
            printf '[%2d/%d] cenario %-6s %-5s run %d ... ' "$n" "$total" "$s" "$a" "$r"

            rm -f "$SOCK"
            ./build/ossim --sched "$a" > "$base.ossim.log" 2>&1 &
            OSSIM_PID=$!
            for _ in $(seq 50); do [ -S "$SOCK" ] && break; sleep 0.1; done

            start=$(date +%s)
            timeout 300 "./$script" > "$base.apps.log" 2>&1
            sleep 0.2
            kill -TERM "$OSSIM_PID"; wait "$OSSIM_PID"; OSSIM_PID=""

            # Linhas da tabela de resumo do ossim: | App | PID | ... |
            awk -F'|' -v s="$s" -v a="$a" -v r="$r" '
                function trim(x) { gsub(/^ +| +$/, "", x); return x }
                /^\| / { app = trim($2); if (app == "App" || app ~ /^\*\*/) next
                         print s "," a "," r "," app "," trim($7) "," trim($8) "," trim($9) }
            ' "$base.ossim.log" >> "$RAW"

            avg=$(awk -F'|' '/^\| \*\*Media/ { gsub(/ /, "", $9); gsub(/ /, "", $7); print "exec=" $9 " resp=" $7 }' "$base.ossim.log")
            echo "$(( $(date +%s) - start )) s  $avg"
        done
    done
done

make_tables
echo
echo "Logs em $OUT/, dados em $RAW, tabelas em $OUT/tabelas.md"
