# Sistemas Operativos 2026/27 — Trabalho Prático 1

## Simulador de Escalonamento — Relatório

### 1. Identificação

| Nome | Número |
|---|---|
| Luís Almoster | a22402907 |

Trabalho individual.

**Repositório GitHub:** https://github.com/luisrdsba/Trabalho_1_Sistemas

---

### 2. Implementação

O escalonador está em `scheduler.c`. A função `scheduler()` é chamada pelo ciclo principal do
`ossim` uma vez por *tick* (`TICKS_MS` = 10 ms) e delega no algoritmo escolhido com
`--sched FIFO|SJF|RR|MLFQ` (`./build/ossim --sched MLFQ`). Em cada *tick* o processo que está
no CPU acumula 10 ms no burst (`ellapsed_time_ms`) e no *time-slice* atual (`slice_used_ms`);
quando o burst termina é enviado `DONE` à aplicação e o PCB volta à fila de comandos.

Cada decisão de escalonamento é escrita no ecrã com o instante de simulação
(`CHEGADA`, `DESPACHO`, `PREEMPCAO`, `QUANTUM`, `FIM BURST`, `BLOQUEIO`, `FIM E/S`, `BOOST`,
`SAIDA`), incluindo o conteúdo das filas no momento do despacho, para se poder verificar que a
escolha é a correta. Exemplo (MLFQ):

```
[   1.071 s] PREEMPCAO  A_batch gastou o time-slice todo do nivel 0 (500 ms), faltam 7500 ms -> desce para o nivel 1
[   1.071 s] DESPACHO   B_inter do nivel 0 (time-slice 500 ms, restam 500 ms dele; faltam 100 ms do burst) | N0:[B_inter(100) C_inter(150)] N1:[A_batch(7500)] N2:[]
```

#### 2.1 FIFO (fornecido)
Não preemptivo. Quando o CPU fica livre, despacha o primeiro processo da *ready queue*, que
corre até ao fim do burst. Só foram acrescentados os registos no ecrã.

#### 2.2 SJF (*Shortest Job First*)
Não preemptivo. Quando o CPU fica livre, percorre-se a *ready queue* e retira-se o processo
com o burst de CPU mais curto (`dequeue_shortest`). Em caso de empate fica o que chegou
primeiro (comparação estrita `<`). O simulador conhece a duração exata do burst porque ela
vem no pedido `RUN`; num SO real teria de ser estimada (p. ex. média exponencial dos bursts
anteriores). Um processo que já está no CPU nunca é interrompido, mesmo que chegue um mais
curto.

#### 2.3 RR (*Round-Robin*)
Preemptivo, *time-slice* de 500 ms (`TIME_SLICE_MS` em `scheduler.h`). Despacha o primeiro
da *ready queue*. Ao fim de 500 ms de CPU, se o burst não terminou:
- se há outro processo à espera, o atual é retirado do CPU e vai para o fim da fila (`PREEMPCAO`);
- se a fila está vazia, continua a correr com um *time-slice* novo (`QUANTUM`), evitando uma
  troca de contexto inútil.

Um processo que chega no mesmo *tick* em que outro é preemptido fica à frente deste (as
chegadas são tratadas antes do escalonador em cada *tick*).

#### 2.4 MLFQ (*Multi-Level Feedback Queue*)
Preemptivo, com três filas (níveis 0, 1 e 2; 0 = mais prioritário). Regras:

1. Corre sempre o primeiro processo do nível mais prioritário não vazio; dentro do mesmo nível
   o funcionamento é Round-Robin.
2. **Quanta por nível:** 500 ms no nível 0, 1000 ms no nível 1 e 2000 ms no nível 2
   (`MLFQ_QUANTUM_MS`). Os processos que descem são CPU-bound; dar-lhes *time-slices* maiores
   reduz as trocas de contexto sem prejudicar os interativos, que estão acima.
3. **Entrada:** todos os processos novos entram no nível 0. O campo `nice` dos `.csv` **não é
   usado** porque não chega ao simulador: a `application` (que não pode ser alterada) lê-o mas
   a mensagem `msg_t` não tem nenhum campo para o enviar ao `ossim`.
4. **Descida:** um processo que gasta o *time-slice* inteiro do seu nível desce um nível. Um
   processo que liberta o CPU antes (termina o burst e bloqueia em E/S ou pede outro burst)
   mantém o nível.
5. **Preempção por prioridade:** se aparece um processo pronto num nível mais prioritário do
   que o que está a correr (p. ex. um interativo que volta de E/S), o processo em execução é
   interrompido de imediato e volta ao fim da fila do seu nível, **sem descer**. Guarda, no
   entanto, o tempo que já gastou do *time-slice*: quando volta ao CPU continua a contar. Sem
   isto, um processo CPU-bound interrompido com frequência nunca chegaria a gastar um
   *time-slice* inteiro e nunca desceria (foi o que se observou numa primeira versão).
6. **Boost:** a cada 5 s (`MLFQ_BOOST_MS`) todos os processos voltam ao nível 0, com um
   *time-slice* novo. Isto evita a *starvation* dos processos CPU-bound e permite a um
   processo que mudou de comportamento (batch → interativo) recuperar prioridade. Os processos
   que estão bloqueados no momento do boost não estão nas filas do MLFQ; para não os esquecer,
   cada boost incrementa um contador (`boost_gen`) e um processo que volta à *ready queue* com
   um valor antigo sobe ao nível 0 nessa altura.

#### 2.5 Medição dos tempos
A `application` só imprime `Elapsed`, `CPU` e `BLOCKED`. Para obter o tempo de resposta, o
`ossim` regista por processo (módulo `stats.c`) a chegada (primeiro `RUN`), a primeira vez
que vai para o CPU e o último `DONE`, e imprime no fim (Ctrl-C) uma tabela por processo com as
médias. Todos os tempos usam o relógio do `ossim`, o mesmo que é enviado à `application`, por
isso o tempo de execução calculado coincide com o `Elapsed` que a aplicação imprime.

- **Tempo de execução** (*turnaround*) = fim − chegada.
- **Tempo de resposta** = primeira execução − chegada.
- **Tempo de resposta médio por burst** = média, em todos os bursts de um processo, do tempo
  entre o pedido `RUN` e o primeiro despacho desse burst. Nos cenários só com CPU é igual ao
  tempo de resposta; nos cenários com E/S mede a rapidez com que um processo interativo é
  atendido de cada vez que volta da E/S.

As aplicações são lançadas com 0,1 s de intervalo (`run_app.sh`), para a ordem de chegada ser
sempre a mesma. O script `run_all.sh` corre cada cenário com cada algoritmo, guarda os logs e
gera as tabelas abaixo.

---

### 3. Cenários

Os cenários 1 a 4 são os fornecidos. Os restantes foram criados para o ponto 3 do enunciado.
Todos os ficheiros estão em `scenarios/` e cada um tem um comentário a explicar o seu papel.

| Cenário | Script | Processos | Objetivo |
|---|---|---|---|
| 1 | `run_apps.sh` | A=10 s, B=15 s, C=20 s (só CPU) | Fornecido |
| 2 | `run_apps2.sh` | A=5, B=10, C=4, D=2, E=3, F=15 s (só CPU) | Fornecido |
| 3 | `run_appsio.sh` | 3 processos com CPU + E/S | Fornecido |
| 4 | `run_appsio2.sh` | 3 processos com CPU + E/S, mais contenção | Fornecido |
| sjf (3a) | `run_sjf.sh` | A=0,5 s, B=10 s, C=6 s, D=1,5 s, E=0,8 s | SJF com tempo médio de execução muito inferior ao FIFO |
| rr (3b) | `run_rr.sh` | A=5 s, B=4,5 s, C=4 s, D=5 s, E=4,5 s | RR com tempo de resposta muito inferior a FIFO e SJF |
| mlfq1 (3c-i) | `run_mlfq1.sh` | A_batch: 8 s seguidos; B_inter: 20×(100 ms CPU + 400 ms E/S); C_inter: 20×(150 ms CPU + 350 ms E/S) | MLFQ com 2 interativos + 1 batch |
| mlfq2 (3c-ii) | `run_mlfq2.sh` | A_misto: 5 s CPU → 12×(100 ms CPU + 400 ms E/S) → 5 s CPU; B_inter: 30×(100+400); C_inter: 30×(150+350) | MLFQ com 2 interativos + 1 batch→interativo→batch |

**Justificação:**

- **sjf (3a):** os processos longos (B e C) chegam antes dos curtos (D e E). No FIFO, os curtos
  ficam à espera de 16 s de trabalho longo; no SJF passam à frente. A chega primeiro e é
  despachado logo em ambos (o CPU está livre e os dois são não preemptivos); é curto para
  que os restantes ainda estejam todos na fila quando ele termina, que é quando o SJF escolhe.
- **rr (3b):** cinco processos CPU-bound longos e de duração parecida. No FIFO e no SJF cada
  processo só começa quando os anteriores acabam (o último espera ~18 s); como as durações são
  parecidas, o SJF não consegue fazer muito melhor. No RR, ao fim de no máximo 4 × 0,5 s todos
  já correram.
- **mlfq1 (3c-i):** o processo batch nunca bloqueia e gasta sempre o *time-slice* inteiro,
  descendo até ao nível 2; os interativos usam 100–150 ms de CPU e bloqueiam, ficando sempre
  no nível 0 e interrompendo o batch sempre que voltam da E/S. O batch chega primeiro para se
  ver a preempção.
- **mlfq2 (3c-ii):** a fase interativa de A_misto (~6 s) é mais longa do que o período de boost
  (5 s), garantindo que ocorre pelo menos um boost durante ou imediatamente antes dessa fase.
  Como os bursts interativos são mais curtos do que o *time-slice*, o processo nunca sobe de
  nível sozinho: só o boost o traz de volta ao nível 0, e a partir daí mantém-se lá enquanto
  for interativo. Na fase batch final volta a gastar *time-slices* inteiros e desce outra vez.

---

### 4. Resultados

Resultados gerados com `./run_all.sh -o results/final` (logs completos em `results/final/`). Cada valor é a média das repetições indicadas em cada cenário. Por falta de tempo, só o cenário 1 (todos os algoritmos) e o FIFO do cenário 2 têm 3 repetições; os restantes casos têm 1 repetição. Uma repetição (cenário 1, RR, repetição 3) foi descartada e repetida porque, por engano, foram lançadas outras aplicações durante a sua execução (logs guardados em `results/final/contaminado/`).

#### Cenário 1

Repetições: FIFO=3 SJF=3 RR=3 MLFQ=3

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 10.607 | 10.557 | 31.110 | 31.078 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 0.000 | 0.000 | 0.000 | 0.000 |
| B | Tempo de execução | 26.233 | 26.181 | 42.114 | 44.465 |
| B | Tempo de resposta | 10.496 | 10.454 | 0.510 | 0.534 |
| B | Tempo de resposta médio por burst | 10.496 | 10.454 | 0.510 | 0.534 |
| C | Tempo de execução | 47.154 | 46.966 | 47.831 | 46.532 |
| C | Tempo de resposta | 26.096 | 26.051 | 0.938 | 1.031 |
| C | Tempo de resposta médio por burst | 26.096 | 26.051 | 0.938 | 1.031 |
| **Média** | Tempo de execução | 27.998 | 27.902 | 40.352 | 40.692 |
| **Média** | Tempo de resposta | 12.197 | 12.168 | 0.483 | 0.522 |
| **Média** | Tempo de resposta médio por burst | 12.197 | 12.168 | 0.483 | 0.522 |

#### Cenário 2

Repetições: FIFO=3 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 5.352 | 5.339 | 24.568 | 25.459 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 0.000 | 0.000 | 0.000 | 0.000 |
| B | Tempo de execução | 15.709 | 25.209 | 35.786 | 41.286 |
| B | Tempo de resposta | 5.243 | 14.667 | 0.548 | 0.571 |
| B | Tempo de resposta médio por burst | 5.243 | 14.667 | 0.548 | 0.571 |
| C | Tempo de execução | 19.798 | 14.537 | 21.637 | 21.067 |
| C | Tempo de resposta | 15.587 | 10.340 | 0.940 | 1.099 |
| C | Tempo de resposta médio por burst | 15.587 | 10.340 | 0.940 | 1.099 |
| D | Tempo de execução | 21.811 | 7.109 | 11.579 | 14.346 |
| D | Tempo de resposta | 19.698 | 5.014 | 1.388 | 1.416 |
| D | Tempo de resposta médio por burst | 19.698 | 5.014 | 1.388 | 1.416 |
| E | Tempo de execução | 24.826 | 10.155 | 17.196 | 17.940 |
| E | Tempo de resposta | 21.704 | 7.003 | 1.801 | 1.843 |
| E | Tempo de resposta médio por burst | 21.704 | 7.003 | 1.801 | 1.843 |
| F | Tempo de execução | 40.348 | 40.583 | 41.171 | 40.813 |
| F | Tempo de resposta | 24.724 | 24.789 | 2.232 | 2.257 |
| F | Tempo de resposta médio por burst | 24.724 | 24.789 | 2.232 | 2.257 |
| **Média** | Tempo de execução | 21.307 | 17.155 | 25.323 | 26.819 |
| **Média** | Tempo de resposta | 14.492 | 10.302 | 1.151 | 1.198 |
| **Média** | Tempo de resposta médio por burst | 14.492 | 10.302 | 1.151 | 1.198 |

#### Cenário 3

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 20.774 | 14.235 | 9.054 | 6.508 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 2.982 | 1.630 | 0.609 | 0.074 |
| B | Tempo de execução | 17.315 | 19.624 | 20.892 | 20.513 |
| B | Tempo de resposta | 0.224 | 2.267 | 0.290 | 0.356 |
| B | Tempo de resposta médio por burst | 0.985 | 1.771 | 0.277 | 0.266 |
| C | Tempo de execução | 19.296 | 15.788 | 13.327 | 12.573 |
| C | Tempo de resposta | 4.331 | 0.208 | 0.705 | 0.890 |
| C | Tempo de resposta médio por burst | 3.769 | 2.588 | 0.341 | 0.297 |
| **Média** | Tempo de execução | 19.128 | 16.549 | 14.424 | 13.198 |
| **Média** | Tempo de resposta | 1.518 | 0.825 | 0.332 | 0.415 |
| **Média** | Tempo de resposta médio por burst | 2.579 | 1.996 | 0.409 | 0.212 |

#### Cenário 4

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 25.013 | 20.842 | 9.969 | 5.628 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 3.282 | 2.588 | 0.785 | 0.069 |
| B | Tempo de execução | 15.638 | 17.926 | 22.642 | 23.581 |
| B | Tempo de resposta | 0.256 | 2.539 | 0.180 | 0.160 |
| B | Tempo de resposta médio por burst | 1.137 | 2.369 | 0.189 | 0.447 |
| C | Tempo de execução | 23.812 | 23.316 | 20.336 | 21.331 |
| C | Tempo de resposta | 6.459 | 0.107 | 0.611 | 0.481 |
| C | Tempo de resposta médio por burst | 3.064 | 2.986 | 0.435 | 0.120 |
| **Média** | Tempo de execução | 21.488 | 20.695 | 17.649 | 16.847 |
| **Média** | Tempo de resposta | 2.238 | 0.882 | 0.264 | 0.214 |
| **Média** | Tempo de resposta médio por burst | 2.494 | 2.648 | 0.470 | 0.212 |

#### Cenário sjf

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 0.748 | 0.722 | 0.705 | 0.730 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 0.000 | 0.000 | 0.000 | 0.000 |
| B | Tempo de execução | 11.196 | 19.828 | 19.872 | 19.940 |
| B | Tempo de resposta | 0.655 | 9.270 | 0.577 | 0.624 |
| B | Tempo de resposta médio por burst | 0.655 | 9.270 | 0.577 | 0.624 |
| C | Tempo de execução | 17.334 | 9.085 | 15.515 | 14.159 |
| C | Tempo de resposta | 11.000 | 2.847 | 0.942 | 1.149 |
| C | Tempo de resposta médio por burst | 11.000 | 2.847 | 0.942 | 1.149 |
| D | Tempo de execução | 18.917 | 2.821 | 5.998 | 7.923 |
| D | Tempo de resposta | 17.334 | 1.258 | 1.430 | 1.459 |
| D | Tempo de resposta médio por burst | 17.334 | 1.258 | 1.430 | 1.459 |
| E | Tempo de execução | 19.621 | 1.149 | 4.293 | 5.707 |
| E | Tempo de resposta | 18.762 | 0.299 | 1.844 | 1.888 |
| E | Tempo de resposta médio por burst | 18.762 | 0.299 | 1.844 | 1.888 |
| **Média** | Tempo de execução | 13.563 | 6.721 | 9.277 | 9.692 |
| **Média** | Tempo de resposta | 9.550 | 2.735 | 0.959 | 1.024 |
| **Média** | Tempo de resposta médio por burst | 9.550 | 2.735 | 0.959 | 1.024 |

#### Cenário rr

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A | Tempo de execução | 5.203 | 5.441 | 24.342 | 24.396 |
| A | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A | Tempo de resposta médio por burst | 0.000 | 0.000 | 0.000 | 0.000 |
| B | Tempo de execução | 9.754 | 14.329 | 22.651 | 19.158 |
| B | Tempo de resposta | 5.088 | 9.573 | 0.461 | 0.622 |
| B | Tempo de resposta médio por burst | 5.088 | 9.573 | 0.461 | 0.622 |
| C | Tempo de execução | 13.787 | 9.389 | 20.430 | 20.219 |
| C | Tempo de resposta | 9.616 | 5.151 | 0.884 | 1.187 |
| C | Tempo de resposta médio por burst | 9.616 | 5.151 | 0.884 | 1.187 |
| D | Tempo de execução | 18.926 | 24.202 | 24.554 | 24.334 |
| D | Tempo de resposta | 13.726 | 18.889 | 1.453 | 1.497 |
| D | Tempo de resposta médio por burst | 13.726 | 18.889 | 1.453 | 1.497 |
| E | Tempo de execução | 23.571 | 18.784 | 23.373 | 23.675 |
| E | Tempo de resposta | 18.822 | 14.016 | 1.868 | 1.928 |
| E | Tempo de resposta médio por burst | 18.822 | 14.016 | 1.868 | 1.928 |
| **Média** | Tempo de execução | 14.248 | 14.429 | 23.070 | 22.356 |
| **Média** | Tempo de resposta | 9.450 | 9.526 | 0.933 | 1.047 |
| **Média** | Tempo de resposta médio por burst | 9.450 | 9.526 | 0.933 | 1.047 |

#### Cenário mlfq1

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A_batch | Tempo de execução | 8.380 | 8.476 | 13.843 | 15.309 |
| A_batch | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A_batch | Tempo de resposta médio por burst | 0.000 | 0.000 | 0.000 | 0.000 |
| B_inter | Tempo de execução | 20.274 | 19.688 | 16.575 | 13.736 |
| B_inter | Tempo de resposta | 8.275 | 8.373 | 0.612 | 0.458 |
| B_inter | Tempo de resposta médio por burst | 0.414 | 0.419 | 0.227 | 0.080 |
| C_inter | Tempo de execução | 20.282 | 19.640 | 16.690 | 13.736 |
| C_inter | Tempo de resposta | 8.285 | 8.313 | 0.721 | 0.453 |
| C_inter | Tempo de resposta médio por burst | 0.414 | 0.416 | 0.248 | 0.073 |
| **Média** | Tempo de execução | 16.312 | 15.935 | 15.703 | 14.260 |
| **Média** | Tempo de resposta | 5.520 | 5.562 | 0.444 | 0.304 |
| **Média** | Tempo de resposta médio por burst | 0.276 | 0.278 | 0.158 | 0.051 |

#### Cenário mlfq2

Repetições: FIFO=1 SJF=1 RR=1 MLFQ=1

| Aplicação | Métrica (s) | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| A_misto | Tempo de execução | 17.942 | 17.781 | 23.850 | 23.551 |
| A_misto | Tempo de resposta | 0.000 | 0.000 | 0.000 | 0.000 |
| A_misto | Tempo de resposta médio por burst | 0.019 | 0.007 | 0.019 | 0.019 |
| B_inter | Tempo de execução | 27.877 | 27.450 | 22.935 | 19.200 |
| B_inter | Tempo de resposta | 5.158 | 5.140 | 0.584 | 0.496 |
| B_inter | Tempo de resposta médio por burst | 0.336 | 0.330 | 0.174 | 0.047 |
| C_inter | Tempo de execução | 27.865 | 28.014 | 23.050 | 19.202 |
| C_inter | Tempo de resposta | 5.143 | 5.327 | 0.695 | 0.486 |
| C_inter | Tempo de resposta médio por burst | 0.335 | 0.351 | 0.175 | 0.046 |
| **Média** | Tempo de execução | 24.561 | 24.415 | 23.278 | 20.651 |
| **Média** | Tempo de resposta | 3.434 | 3.489 | 0.426 | 0.327 |
| **Média** | Tempo de resposta médio por burst | 0.230 | 0.229 | 0.123 | 0.037 |

#### Resumo: tempos médios por cenário (s)

| Cenário | Métrica | FIFO | SJF | RR | MLFQ |
|---|---|---|---|---|---|
| 1 | T. médio de execução | 27.998 | 27.902 | 40.352 | 40.692 |
| 1 | T. médio de resposta | 12.197 | 12.168 | 0.483 | 0.522 |
| 1 | T. médio de resposta por burst | 12.197 | 12.168 | 0.483 | 0.522 |
| 2 | T. médio de execução | 21.307 | 17.155 | 25.323 | 26.819 |
| 2 | T. médio de resposta | 14.492 | 10.302 | 1.151 | 1.198 |
| 2 | T. médio de resposta por burst | 14.492 | 10.302 | 1.151 | 1.198 |
| 3 | T. médio de execução | 19.128 | 16.549 | 14.424 | 13.198 |
| 3 | T. médio de resposta | 1.518 | 0.825 | 0.332 | 0.415 |
| 3 | T. médio de resposta por burst | 2.579 | 1.996 | 0.409 | 0.212 |
| 4 | T. médio de execução | 21.488 | 20.695 | 17.649 | 16.847 |
| 4 | T. médio de resposta | 2.238 | 0.882 | 0.264 | 0.214 |
| 4 | T. médio de resposta por burst | 2.494 | 2.648 | 0.470 | 0.212 |
| sjf | T. médio de execução | 13.563 | 6.721 | 9.277 | 9.692 |
| sjf | T. médio de resposta | 9.550 | 2.735 | 0.959 | 1.024 |
| sjf | T. médio de resposta por burst | 9.550 | 2.735 | 0.959 | 1.024 |
| rr | T. médio de execução | 14.248 | 14.429 | 23.070 | 22.356 |
| rr | T. médio de resposta | 9.450 | 9.526 | 0.933 | 1.047 |
| rr | T. médio de resposta por burst | 9.450 | 9.526 | 0.933 | 1.047 |
| mlfq1 | T. médio de execução | 16.312 | 15.935 | 15.703 | 14.260 |
| mlfq1 | T. médio de resposta | 5.520 | 5.562 | 0.444 | 0.304 |
| mlfq1 | T. médio de resposta por burst | 0.276 | 0.278 | 0.158 | 0.051 |
| mlfq2 | T. médio de execução | 24.561 | 24.415 | 23.278 | 20.651 |
| mlfq2 | T. médio de resposta | 3.434 | 3.489 | 0.426 | 0.327 |
| mlfq2 | T. médio de resposta por burst | 0.230 | 0.229 | 0.123 | 0.037 |

T. resposta = 1.ª execução - chegada. T. resposta por burst = média de (despacho - pedido RUN) em todos os bursts.
T. execução = fim - chegada (= Elapsed da application). Valores = média das repetições.

---

### 5. Análise

#### 5.1 SJF: menor tempo médio de execução

O SJF minimiza o tempo médio de execução porque o tempo de espera de cada processo é a soma
dos bursts que correm antes dele: pondo os curtos primeiro, poucos processos esperam pelos
longos e muitos esperam pouco. No cenário **sjf** o tempo médio de execução cai de 13,56 s
(FIFO) para 6,72 s (−50 %): D e E passam de ~19 s para 2,8 s e 1,1 s. O custo é para o
processo mais longo: B passa de 11,2 s para 19,8 s, porque é ultrapassado por todos. Com um
fluxo contínuo de processos curtos, um processo longo poderia nunca correr (*starvation*).

O ganho depende da ordem de chegada. No **cenário 1** os processos já chegam do mais curto para
o mais longo (10, 15, 20 s), pelo que SJF e FIFO fazem exatamente a mesma escolha (27,9 s vs
28,0 s). No **cenário 2** o SJF ganha (17,2 s vs 21,3 s), mas não reordena A: como é não
preemptivo e A chega com o CPU livre, A é despachado antes de os outros chegarem. No cenário
**rr**, com durações parecidas, também não há o que reordenar (14,4 s vs 14,2 s).

#### 5.2 RR: melhor tempo de resposta, pior tempo de execução

No RR nenhum processo espera mais do que (n − 1) × 500 ms para correr pela primeira vez. No
cenário **rr** o tempo médio de resposta é 0,93 s, contra 9,45 s (FIFO) e 9,53 s (SJF): ~10×
menor. O mesmo acontece nos cenários 1 (0,48 s vs 12,2 s) e 2 (1,15 s vs 14,5 s e 10,3 s).

Em contrapartida, o tempo médio de execução piora: 23,1 s no cenário rr contra 14,2 s no FIFO
(+62 %), e 40,4 s contra 28,0 s no cenário 1. Como todos avançam "em paralelo", todos acabam
perto do fim: no cenário rr os cinco processos terminam entre 20 e 25 s, enquanto no FIFO A
termina aos 5 s, B aos 10 s, etc. O trabalho total é o mesmo, mas os processos que no FIFO
acabariam cedo ficam à espera de fatias dos outros. Quando há processos com E/S (cenários 3 e
4) o RR já melhora também o tempo de execução face ao FIFO, porque um processo longo deixa de
bloquear o CPU enquanto os outros poderiam estar a fazer E/S em paralelo.

#### 5.3 MLFQ: interativos vs batch

Nos cenários só com CPU (1, 2, sjf, rr) o MLFQ comporta-se quase como o RR (tempos de resposta
e de execução muito próximos): todos os processos são CPU-bound, descem juntos de nível e
dentro de cada nível o escalonamento é Round-Robin. Os *time-slices* maiores nos níveis baixos
e os boosts periódicos introduzem alguma variação entre repetições (p. ex. no cenário 1 o tempo
médio de execução variou entre 37,7 s e 42,2 s conforme a altura em que os boosts calharam).

A diferença aparece quando há processos interativos:

- **mlfq1:** no FIFO e no SJF, os interativos esperam pelo burst de 8 s do batch antes de
  correrem pela primeira vez (resposta ~8,3 s). No RR esperam no máximo uma fatia do batch
  (resposta média por burst 0,23–0,25 s). No MLFQ, o batch desce até ao nível 2 e é
  interrompido de imediato sempre que um interativo volta da E/S: a resposta média por burst
  dos interativos cai para 0,07–0,08 s (3× melhor que o RR e 5× melhor que o FIFO) e o seu tempo
  de execução cai de 20,3 s (FIFO) para 13,7 s. O preço é pago pelo batch, que passa de 8,4 s
  (FIFO) para 15,3 s, mas o tempo médio global é o melhor dos quatro (14,3 s). O boost a cada
  5 s garante que o batch nunca fica parado indefinidamente.
- **mlfq2:** os interativos têm novamente a melhor resposta no MLFQ (0,05 s por burst, contra
  0,17 s no RR e 0,33 s no FIFO/SJF) e o menor tempo de execução (19,2 s contra 27,9 s no FIFO).
  O log mostra a adaptação de A_misto ao seu comportamento: na fase batch inicial desce
  0 → 1 → 2 (0,8 s e 3,1 s), o boost dos 5 s devolve-o ao nível 0 e volta a descer; o boost dos
  10,0 s apanha-o no fim da fase batch e toda a fase interativa (10,5 s a 17,8 s) decorre no
  nível 0, porque os bursts de 100 ms nunca esgotam o *time-slice*; na fase batch final volta a
  descer (0 → 1 aos 18,4 s; boost aos 20 s; 0 → 1 → 2 aos 20,5 s e 21,6 s). A resposta por burst
  de A_misto é baixa em todos os algoritmos (~0,02 s) porque os interativos passam a maior parte
  do tempo bloqueados e o CPU está muitas vezes livre quando A_misto pede um burst curto.
- **Cenários 3 e 4:** o MLFQ tem a melhor resposta média por burst (0,21 s nos dois, contra
  0,41–0,47 s no RR e ~2,5 s no FIFO) e o melhor tempo médio de execução. A aplicação
  interativa A termina em 6,5 s / 5,6 s no MLFQ contra 20,8 s / 25,0 s no FIFO. No cenário 4 o
  SJF chega a ter pior resposta por burst do que o FIFO (2,65 s vs 2,49 s): por ser não
  preemptivo, quando o burst de 6 s de B começa, todos esperam por ele.

#### 5.4 Conclusões

- **FIFO:** simples e justo na ordem de chegada, mas os processos curtos e interativos ficam
  presos atrás dos longos (efeito comboio).
- **SJF:** ótimo para o tempo médio de execução, mas exige conhecer a duração dos bursts, não
  ajuda a resposta de quem chega com um longo a correr e pode causar *starvation* dos longos.
- **RR:** o melhor tempo de resposta para processos CPU-bound semelhantes, à custa de um tempo de
  execução maior; o *time-slice* define o compromisso (menor → melhor resposta, mais trocas de
  contexto).
- **MLFQ:** sem conhecer as durações, aproxima o comportamento ideal: dá prioridade aos
  interativos (resposta baixa, como um SJF por bursts) e deixa os CPU-bound em níveis baixos com
  *time-slices* maiores; o boost evita a *starvation* e permite a um processo recuperar
  prioridade quando muda de comportamento.

#### 5.5 Limitações das medições

- A maioria dos casos tem apenas 1 repetição (ver secção 4); os valores têm variações de
  algumas centenas de milissegundos entre repetições.
- O ciclo do simulador dorme 10 ms por *tick* mas cada *tick* demora um pouco mais (tempo de
  processamento e de E/S do socket); por isso os tempos de execução são ~5–6 % superiores à soma
  dos bursts (p. ex. 10,6 s para um processo com 10 s de CPU sozinho no CPU).
- O campo `nice` não é usado (não chega ao simulador), pelo que no MLFQ não é possível definir
  prioridades iniciais diferentes.
