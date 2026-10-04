#include "stats.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char name[PCB_NAME_LEN];
    int32_t pid;
    uint32_t arrival_ms;        // 1.º RUN
    int has_first_run;
    uint32_t first_run_ms;      // 1.ª vez no CPU
    uint32_t last_done_ms;      // último DONE enviado
    uint64_t sum_burst_resp_ms; // soma de (despacho - RUN) de todos os bursts
    uint32_t n_burst_resp;
    uint32_t cpu_ms;            // soma dos bursts de CPU pedidos
    uint32_t blocked_ms;        // soma dos tempos de bloqueio pedidos
    int finished;               // a application já fechou a ligação
} proc_stats_t;

static proc_stats_t stats[STATS_MAX_PROCS];
static int n_stats = 0;

void sim_log(uint32_t now_ms, const char *fmt, ...) {
    va_list ap;
    printf("[%8.3f s] ", now_ms / 1000.0);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
}

static proc_stats_t *get_rec(const pcb_t *pcb) {
    if (pcb->stats_id < 0 || pcb->stats_id >= n_stats) return NULL;
    return &stats[pcb->stats_id];
}

/*
 * O msg_t não traz o nome da aplicação, mas o ossim e as applications correm na
 * mesma máquina: lemos /proc/<pid>/cmdline ("./build/application\0scenarios/1/A.csv\0")
 * e usamos o nome do .csv sem extensão (o mesmo que a application imprime).
 * Serve apenas para os logs serem legíveis; não influencia o escalonamento.
 */
static void resolve_name(pcb_t *pcb) {
    char path[64];
    char buf[512];
    size_t n = 0;

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pcb->pid);
    FILE *f = fopen(path, "r");
    if (f) {
        n = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
    }
    buf[n] = '\0';

    size_t len0 = strlen(buf);          // argv[0]
    if (len0 + 1 < n) {
        const char *arg = buf + len0 + 1;  // argv[1] = ficheiro .csv
        const char *slash = strrchr(arg, '/');
        const char *base = slash ? slash + 1 : arg;
        const char *dot = strrchr(base, '.');
        size_t len = dot ? (size_t)(dot - base) : strlen(base);
        if (len >= PCB_NAME_LEN) len = PCB_NAME_LEN - 1;
        memcpy(pcb->name, base, len);
        pcb->name[len] = '\0';
    } else {
        snprintf(pcb->name, PCB_NAME_LEN, "pid%d", pcb->pid);
    }
}

int stats_on_run(pcb_t *pcb, uint32_t now_ms) {
    int is_new = 0;

    if (pcb->stats_id < 0) {
        is_new = 1;
        resolve_name(pcb);
        if (n_stats < STATS_MAX_PROCS) {
            proc_stats_t *r = &stats[n_stats];
            memset(r, 0, sizeof(*r));
            memcpy(r->name, pcb->name, PCB_NAME_LEN);
            r->pid = pcb->pid;
            r->arrival_ms = now_ms;
            r->last_done_ms = now_ms;
            pcb->stats_id = n_stats++;
        }
    }

    // Início de um novo burst: conta para o tempo de resposta por burst
    pcb->burst_ready_ms = now_ms;
    pcb->burst_dispatched = 0;

    proc_stats_t *r = get_rec(pcb);
    if (r) r->cpu_ms += pcb->time_ms;
    return is_new;
}

void stats_on_block(pcb_t *pcb) {
    proc_stats_t *r = get_rec(pcb);
    if (r) r->blocked_ms += pcb->time_ms;
}

void stats_on_dispatch(pcb_t *pcb, uint32_t now_ms) {
    proc_stats_t *r = get_rec(pcb);

    // Só a 1.ª vez que o burst vai para o CPU conta para a resposta
    // (no RR/MLFQ um burst pode ser despachado várias vezes).
    if (!pcb->burst_dispatched) {
        pcb->burst_dispatched = 1;
        if (r) {
            r->sum_burst_resp_ms += now_ms - pcb->burst_ready_ms;
            r->n_burst_resp++;
        }
    }
    if (r && !r->has_first_run) {
        r->has_first_run = 1;
        r->first_run_ms = now_ms;
    }
}

void stats_on_done(pcb_t *pcb, uint32_t now_ms) {
    proc_stats_t *r = get_rec(pcb);
    if (r) r->last_done_ms = now_ms;
}

void stats_on_exit(pcb_t *pcb) {
    proc_stats_t *r = get_rec(pcb);
    if (r) r->finished = 1;
}

void stats_print_summary(const char *algo_name) {
    double sum_resp = 0, sum_resp_burst = 0, sum_exec = 0;
    int n_resp = 0, n_exec = 0;

    printf("\n===== Resumo (%s) - tempos em segundos =====\n", algo_name);
    printf("| App | PID | Chegada | 1a exec | Fim | T. resposta | T. resp. medio/burst | T. execucao | CPU | Bloqueado |\n");
    printf("|---|---|---|---|---|---|---|---|---|---|\n");

    for (int i = 0; i < n_stats; i++) {
        proc_stats_t *r = &stats[i];

        printf("| %s | %d | %.3f | ", r->name, r->pid, r->arrival_ms / 1000.0);

        if (r->has_first_run) printf("%.3f | ", r->first_run_ms / 1000.0);
        else                  printf("- | ");

        if (r->finished) printf("%.3f | ", r->last_done_ms / 1000.0);
        else             printf("- | ");

        if (r->has_first_run) {
            double resp = (r->first_run_ms - r->arrival_ms) / 1000.0;
            double resp_burst = (double) r->sum_burst_resp_ms / r->n_burst_resp / 1000.0;
            printf("%.3f | %.3f | ", resp, resp_burst);
            sum_resp += resp;
            sum_resp_burst += resp_burst;
            n_resp++;
        } else {
            printf("- | - | ");
        }

        if (r->finished) {
            double exec = (r->last_done_ms - r->arrival_ms) / 1000.0;
            printf("%.3f | ", exec);
            sum_exec += exec;
            n_exec++;
        } else {
            printf("- | ");     // ainda não terminou quando se fez Ctrl-C
        }

        printf("%.3f | %.3f |\n", r->cpu_ms / 1000.0, r->blocked_ms / 1000.0);
    }

    printf("| **Media** | | | | | ");
    if (n_resp) printf("%.3f | %.3f | ", sum_resp / n_resp, sum_resp_burst / n_resp);
    else        printf("- | - | ");
    if (n_exec) printf("%.3f | | |\n", sum_exec / n_exec);
    else        printf("- | | |\n");

    printf("T. resposta = 1a exec - chegada; T. resp. medio/burst = media de (despacho - pedido RUN) "
           "por burst; T. execucao = fim - chegada (= Elapsed da application).\n");
}
