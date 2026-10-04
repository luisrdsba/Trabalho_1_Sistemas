#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msg.h"
#include "stats.h"

static const char *SCHED_NAMES[] = { "FIFO", "SJF", "RR", "MLFQ", NULL };
static sched_algo_en sched_algo = SCHED_FIFO;

int set_sched_algo(const char *name) {
    for (int i = 0; SCHED_NAMES[i] != NULL; i++) {
        if (strcasecmp(SCHED_NAMES[i], name) == 0) {
            sched_algo = (sched_algo_en) i;
            return sched_algo;
        }
    }
    return -1;
}

const char *get_sched_algo_str(void) {
    return SCHED_NAMES[sched_algo];
}

/* =========================================================================
 * Utilitários comuns a todos os algoritmos
 * ========================================================================= */

#define QSTR_LEN 1024

// Tempo de CPU que ainda falta ao burst atual
static uint32_t remaining_ms(const pcb_t *t) {
    return (t->ellapsed_time_ms >= t->time_ms) ? 0 : t->time_ms - t->ellapsed_time_ms;
}

// Escreve uma fila como "[A(9500) B(15000)]" (nome e ms de CPU que faltam) — só para os logs
static void fmt_queue(char *buf, size_t size, const queue_t *q) {
    size_t off = 0;
    off += snprintf(buf, size, "[");
    for (queue_elem_t *e = q->head; e != NULL && off < size; e = e->next) {
        off += snprintf(buf + off, size - off, "%s%s(%u)",
                        e == q->head ? "" : " ", e->pcb->name, remaining_ms(e->pcb));
    }
    if (off < size) snprintf(buf + off, size - off, "]");
}

/**
 * Um tick de CPU para o processo em execução.
 * @return 1 se o burst de CPU terminou
 */
static int run_one_tick(pcb_t *t) {
    t->ellapsed_time_ms += TICKS_MS;
    t->slice_used_ms += TICKS_MS;
    return t->ellapsed_time_ms >= t->time_ms;
}

/**
 * Coloca um processo no CPU e regista o despacho (para o tempo de resposta).
 * @param new_slice 1 = começa um time-slice novo; 0 = continua o que já tinha
 *                  gasto (MLFQ, depois de ter sido interrompido por prioridade)
 */
static void start_on_cpu(uint32_t current_time_ms, pcb_t *task, int new_slice) {
    task->slice_start_ms = current_time_ms;
    if (new_slice) task->slice_used_ms = 0;
    stats_on_dispatch(task, current_time_ms);
}

/**
 * Send DONE to the application and move the finished burst to the command queue.
 */
static void finish_burst(uint32_t current_time_ms, queue_t *cq, pcb_t *task, const char *note) {
    sim_log(current_time_ms, "FIM BURST  %s terminou o burst de CPU de %u ms -> DONE%s",
            task->name, task->time_ms, note);
    stats_on_done(task, current_time_ms);

    msg_t msg = {
        .pid = task->pid,
        .request = PROCESS_REQUEST_DONE,
        .time_ms = current_time_ms
    };
    if (write(task->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
        perror("write");
    }
    enqueue_pcb(cq, task);
}

/* =========================================================================
 * FIFO (First-In, First-Out) — não preemptivo.
 *
 * O primeiro processo a chegar à ready queue corre até o seu burst de CPU
 * terminar; só então o CPU fica livre para o processo seguinte.
 * ========================================================================= */
static int sched_fifo(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    if (*cpu_task && run_one_tick(*cpu_task)) {
        finish_burst(current_time_ms, cq, *cpu_task, "");
        *cpu_task = NULL;
    }

    if (*cpu_task == NULL && rq->head) {
        char fila[QSTR_LEN];
        fmt_queue(fila, sizeof(fila), rq);
        *cpu_task = dequeue_pcb(rq);
        start_on_cpu(current_time_ms, *cpu_task, 1);
        sim_log(current_time_ms, "DESPACHO   %s (burst %u ms): o primeiro da ready queue %s",
                (*cpu_task)->name, (*cpu_task)->time_ms, fila);
        return 1;
    }
    return 0;
}

/* =========================================================================
 * SJF (Shortest Job First) — não preemptivo.
 *
 * Quando o CPU fica livre, escolhe da ready queue o processo com o burst de CPU
 * mais curto (o simulador conhece-o, vem no pedido RUN). Em caso de empate fica
 * o que chegou primeiro. Uma vez no CPU, corre até ao fim do burst.
 * ========================================================================= */
static pcb_t *dequeue_shortest(queue_t *rq) {
    queue_elem_t *best = NULL;
    for (queue_elem_t *e = rq->head; e != NULL; e = e->next) {
        // '<' estrito: em empate mantém-se o primeiro encontrado (= o que chegou primeiro)
        if (best == NULL || remaining_ms(e->pcb) < remaining_ms(best->pcb)) {
            best = e;
        }
    }
    if (best == NULL) return NULL;

    remove_queue_elem(rq, best);
    pcb_t *task = best->pcb;
    free(best);
    return task;
}

static int sched_sjf(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    if (*cpu_task && run_one_tick(*cpu_task)) {
        finish_burst(current_time_ms, cq, *cpu_task, "");
        *cpu_task = NULL;
    }

    if (*cpu_task == NULL && rq->head) {
        char fila[QSTR_LEN];
        fmt_queue(fila, sizeof(fila), rq);
        *cpu_task = dequeue_shortest(rq);
        start_on_cpu(current_time_ms, *cpu_task, 1);
        sim_log(current_time_ms, "DESPACHO   %s (burst %u ms): o burst mais curto de %s",
                (*cpu_task)->name, (*cpu_task)->time_ms, fila);
        return 1;
    }
    return 0;
}

/* =========================================================================
 * RR (Round-Robin) — preemptivo, time-slice = TIME_SLICE_MS.
 *
 * Despacha o primeiro da ready queue. Se ao fim de TIME_SLICE_MS o burst ainda
 * não terminou e há outro processo à espera, o processo é retirado do CPU
 * (preempção) e vai para o fim da ready queue. Se não houver mais ninguém,
 * continua a correr com um time-slice novo.
 * ========================================================================= */
static int sched_rr(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    pcb_t *t = *cpu_task;

    if (t) {
        if (run_one_tick(t)) {
            finish_burst(current_time_ms, cq, t, "");
            *cpu_task = NULL;
        } else if (t->slice_used_ms >= TIME_SLICE_MS) {
            if (rq->head) {
                sim_log(current_time_ms, "PREEMPCAO  %s esgotou o time-slice de %d ms (faltam %u ms) -> fim da ready queue",
                        t->name, TIME_SLICE_MS, remaining_ms(t));
                enqueue_pcb(rq, t);
                *cpu_task = NULL;
            } else {
                sim_log(current_time_ms, "QUANTUM    %s esgotou o time-slice mas a ready queue esta vazia -> continua (faltam %u ms)",
                        t->name, remaining_ms(t));
                t->slice_used_ms = 0;
            }
        }
    }

    if (*cpu_task == NULL && rq->head) {
        char fila[QSTR_LEN];
        fmt_queue(fila, sizeof(fila), rq);
        *cpu_task = dequeue_pcb(rq);
        start_on_cpu(current_time_ms, *cpu_task, 1);
        sim_log(current_time_ms, "DESPACHO   %s (faltam %u ms, time-slice %d ms): o primeiro da ready queue %s",
                (*cpu_task)->name, remaining_ms(*cpu_task), TIME_SLICE_MS, fila);
        return 1;
    }
    return 0;
}

/* =========================================================================
 * MLFQ (Multi-Level Feedback Queue) — preemptivo.
 *
 * Regras:
 *  1. Há MLFQ_LEVELS filas; o nível 0 é o de maior prioridade. Corre sempre
 *     um processo do nível mais prioritário que não esteja vazio.
 *  2. Dentro do mesmo nível: Round-Robin.
 *  3. Um processo novo entra no nível 0 (o nice dos .csv não chega ao ossim).
 *  4. Se gastar o time-slice todo do seu nível, desce um nível (é CPU-bound).
 *     Se largar o CPU antes (fim do burst / bloqueio), mantém o nível.
 *  5. Se chegar um processo a um nível mais prioritário do que o que está a
 *     correr, este é interrompido (volta ao fim da fila do seu nível, sem descer)
 *     mas NÃO perde o que já gastou do time-slice: quando voltar ao CPU continua
 *     a contar. Senão um processo CPU-bound interrompido muitas vezes nunca
 *     chegava a gastar um time-slice inteiro e nunca descia de nível.
 *  6. De MLFQ_BOOST_MS em MLFQ_BOOST_MS todos os processos voltam ao nível 0
 *     (boost), para os CPU-bound não sofrerem starvation e para um processo que
 *     passou a ser interativo recuperar prioridade.
 *
 * Os processos que chegam à ready queue (rq) — novos ou a voltar de E/S — são
 * passados em cada tick para a fila do seu nível.
 * ========================================================================= */
#define MLFQ_LEVELS   3
#define MLFQ_BOOST_MS 5000

// Time-slice de cada nível: 500 ms no nível 0, duplica a cada nível abaixo
static const uint32_t MLFQ_QUANTUM_MS[MLFQ_LEVELS] = { TIME_SLICE_MS, 2 * TIME_SLICE_MS, 4 * TIME_SLICE_MS };

static queue_t mlfq[MLFQ_LEVELS];
static uint32_t boost_gen = 0;      // número de boosts feitos até agora
static uint32_t last_boost_ms = 0;

static void fmt_mlfq(char *buf, size_t size) {
    size_t off = 0;
    for (int l = 0; l < MLFQ_LEVELS && off < size; l++) {
        char fila[QSTR_LEN];
        fmt_queue(fila, sizeof(fila), &mlfq[l]);
        off += snprintf(buf + off, size - off, "%sN%d:%s", l ? " " : "", l, fila);
    }
}

// Nível mais prioritário com processos prontos, ou -1 se estão todos vazios
static int mlfq_top_level(void) {
    for (int l = 0; l < MLFQ_LEVELS; l++) {
        if (mlfq[l].head) return l;
    }
    return -1;
}

/**
 * Boost: todos os processos prontos e o que está no CPU voltam ao nível 0.
 * Os que estão bloqueados/à espera de comando não estão nestas filas: ficam
 * com mlfq_boost_gen desatualizado e sobem ao nível 0 quando voltarem à rq.
 */
static void mlfq_boost(uint32_t current_time_ms, pcb_t *running) {
    boost_gen++;
    last_boost_ms = current_time_ms;

    if (running == NULL && mlfq_top_level() < 0) return;   // ninguém para subir agora

    for (int l = 1; l < MLFQ_LEVELS; l++) {
        pcb_t *p;
        while ((p = dequeue_pcb(&mlfq[l])) != NULL) {
            p->mlfq_level = 0;
            enqueue_pcb(&mlfq[0], p);
        }
    }
    for (queue_elem_t *e = mlfq[0].head; e != NULL; e = e->next) {
        e->pcb->mlfq_boost_gen = boost_gen;
        e->pcb->slice_used_ms = 0;
    }
    if (running) {
        running->mlfq_level = 0;
        running->mlfq_boost_gen = boost_gen;
        running->slice_used_ms = 0;     // começa um time-slice novo do nível 0
    }

    char filas[QSTR_LEN];
    fmt_mlfq(filas, sizeof(filas));
    sim_log(current_time_ms, "BOOST      todos os processos voltam ao nivel 0 (CPU: %s) | %s",
            running ? running->name : "livre", filas);
}

static int sched_mlfq(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    pcb_t *t = *cpu_task;

    // 1. Contabiliza o tick do processo em execução. Se o burst terminou, liberta
    //    o CPU sem mudar de nível (largou o CPU antes de esgotar o time-slice).
    if (t && run_one_tick(t)) {
        char note[64];
        snprintf(note, sizeof(note), " (mantem o nivel %d)", t->mlfq_level);
        finish_burst(current_time_ms, cq, t, note);
        t->slice_used_ms = 0;           // o próximo burst começa um time-slice novo
        *cpu_task = t = NULL;
    }

    // 2. Boost periódico
    if (current_time_ms - last_boost_ms >= MLFQ_BOOST_MS) {
        mlfq_boost(current_time_ms, t);
    }

    // 3. Processos que chegaram à ready queue -> fila do seu nível
    pcb_t *p;
    while ((p = dequeue_pcb(rq)) != NULL) {
        if (p->mlfq_boost_gen != boost_gen) {
            if (p->mlfq_level != 0) {
                sim_log(current_time_ms, "BOOST      %s sobe do nivel %d ao nivel 0 (houve boost enquanto nao estava pronto)",
                        p->name, p->mlfq_level);
            }
            p->mlfq_level = 0;
            p->mlfq_boost_gen = boost_gen;
        }
        enqueue_pcb(&mlfq[p->mlfq_level], p);
        sim_log(current_time_ms, "MLFQ       %s entra na fila do nivel %d", p->name, p->mlfq_level);
    }

    // 4. Preempção do processo em execução
    if (t) {
        int lvl = t->mlfq_level;
        if (t->slice_used_ms >= MLFQ_QUANTUM_MS[lvl]) {
            // Gastou o time-slice todo: desce um nível (se ainda não estiver no último)
            int new_lvl = (lvl < MLFQ_LEVELS - 1) ? lvl + 1 : lvl;
            if (new_lvl != lvl) {
                sim_log(current_time_ms, "PREEMPCAO  %s gastou o time-slice todo do nivel %d (%u ms), faltam %u ms -> desce para o nivel %d",
                        t->name, lvl, MLFQ_QUANTUM_MS[lvl], remaining_ms(t), new_lvl);
            } else {
                sim_log(current_time_ms, "PREEMPCAO  %s gastou o time-slice todo do nivel %d (%u ms), faltam %u ms -> fim da fila do nivel %d (ultimo nivel)",
                        t->name, lvl, MLFQ_QUANTUM_MS[lvl], remaining_ms(t), lvl);
            }
            t->mlfq_level = new_lvl;
            t->slice_used_ms = 0;       // time-slice novo no nível para onde vai
            enqueue_pcb(&mlfq[new_lvl], t);
            *cpu_task = t = NULL;
        } else {
            int top = mlfq_top_level();
            if (top >= 0 && top < lvl) {
                // Há alguém mais prioritário pronto: cede o CPU, mas não é castigado
                sim_log(current_time_ms, "PREEMPCAO  %s (nivel %d) cede o CPU: ha processo pronto no nivel %d -> fim da fila do nivel %d (ja gastou %u de %u ms do time-slice)",
                        t->name, lvl, top, lvl, t->slice_used_ms, MLFQ_QUANTUM_MS[lvl]);
                enqueue_pcb(&mlfq[lvl], t);
                *cpu_task = t = NULL;
            }
        }
    }

    // 5. CPU livre: despacha o primeiro do nível mais prioritário
    if (*cpu_task == NULL) {
        int top = mlfq_top_level();
        if (top >= 0) {
            char filas[QSTR_LEN];
            fmt_mlfq(filas, sizeof(filas));
            p = dequeue_pcb(&mlfq[top]);
            start_on_cpu(current_time_ms, p, 0);   // slice_used_ms já foi posto a 0 quando devia
            *cpu_task = p;
            sim_log(current_time_ms, "DESPACHO   %s do nivel %d (time-slice %u ms, restam %u ms dele; faltam %u ms do burst) | %s",
                    p->name, top, MLFQ_QUANTUM_MS[top], MLFQ_QUANTUM_MS[top] - p->slice_used_ms,
                    remaining_ms(p), filas);
            return 1;
        }
    }
    return 0;
}

/* =========================================================================
 * Ponto de entrada: um passo de escalonamento por tick.
 * ========================================================================= */
int scheduler(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    switch (sched_algo) {
        case SCHED_SJF:  return sched_sjf(current_time_ms, rq, cq, cpu_task);
        case SCHED_RR:   return sched_rr(current_time_ms, rq, cq, cpu_task);
        case SCHED_MLFQ: return sched_mlfq(current_time_ms, rq, cq, cpu_task);
        case SCHED_FIFO:
        default:         return sched_fifo(current_time_ms, rq, cq, cpu_task);
    }
}
