//
// Created by Martijn Kuipers on 20/10/2025.
//

#ifndef PCB_H
#define PCB_H
#include <stdint.h>

#include "msg.h"

#define PCB_NAME_LEN 32

typedef enum  {
    TASK_COMMAND = 0,   // Task has connected and is waiting for instructions
    TASK_BLOCKED,       // Task is blocked (waiting/IO wait)
    TASK_RUNNING,       // Task is in the ready queue or currently running
    TASK_STOPPED,       // Task has finished execution (sent DONE), waiting for more messages
    TASK_TERMINATED,    // Task has been terminated and will be removed
} task_status_en;

// Define the Process Control Block (PCB) structure
typedef struct pcb_st{
    int32_t pid;                   // Process ID
    task_status_en status;         // Current status of the task defined by the pcb
    uint32_t time_ms;              // Time requested by application in milliseconds
    uint32_t ellapsed_time_ms;     // Time ellapsed since start in milliseconds
    uint32_t slice_start_ms;       // Time when the current time slice started
    uint32_t sockfd;               // Socket file descriptor for communication with the application
    uint32_t last_update_time_ms;  // Last time the PCB was updated

    // --- Acrescentado no TP1 ---
    char name[PCB_NAME_LEN];       // Nome da aplicação (nome do .csv), só para os logs
    uint32_t slice_used_ms;        // CPU já gasto no time-slice atual (RR / MLFQ)
    int mlfq_level;                // Nível atual no MLFQ (0 = prioridade mais alta)
    uint32_t mlfq_boost_gen;       // Último boost "visto" por este processo (MLFQ)
    uint32_t burst_ready_ms;       // Instante em que o burst atual entrou na ready queue
    int burst_dispatched;          // 1 se o burst atual já foi posto no CPU pelo menos uma vez
    int stats_id;                  // Índice do registo de estatísticas (stats.c), -1 se ainda não tem
} pcb_t;

#endif //PCB_H
