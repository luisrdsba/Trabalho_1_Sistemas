#ifndef STATS_H
#define STATS_H

#include <stdint.h>

#include "pcb.h"

/*
 * Métricas por processo medidas no próprio simulador (a application só imprime
 * Elapsed/CPU/BLOCKED, não o tempo de resposta).
 *
 * Todos os tempos são do relógio do ossim (now_ms()), o mesmo que é enviado à
 * application nos ACK/DONE — por isso o "T. execução" aqui é igual ao "Elapsed"
 * que a application imprime.
 *
 *   T. resposta          = 1.ª vez no CPU - chegada (1.º RUN)
 *   T. resp. médio/burst = média, por burst, de (1.º despacho do burst - pedido RUN)
 *   T. execução          = último DONE - chegada
 */

#define STATS_MAX_PROCS 256

// printf com o instante de simulação à cabeça: "[  12.340 s] ..."
void sim_log(uint32_t now_ms, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

// Chamada quando chega um RUN. Na 1.ª vez cria o registo e descobre o nome.
// Devolve 1 se o processo é novo, 0 se é só um novo burst de um processo conhecido.
int  stats_on_run(pcb_t *pcb, uint32_t now_ms);
void stats_on_block(pcb_t *pcb);                    // chegou um BLOCK
void stats_on_dispatch(pcb_t *pcb, uint32_t now_ms);// o escalonador pôs o processo no CPU
void stats_on_done(pcb_t *pcb, uint32_t now_ms);    // foi enviado DONE (fim de burst ou de E/S)
void stats_on_exit(pcb_t *pcb);                     // a application fechou a ligação

// Tabela final (formato Markdown) + médias. Chamada no Ctrl-C.
void stats_print_summary(const char *algo_name);

#endif //STATS_H
