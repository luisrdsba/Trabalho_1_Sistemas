# Sistemas Operativos 2026/27 — Trabalho Prático 1

## Simulador de Escalonamento — Relatório

### 1. Identificação

| Nome | Número |
|---|---|
| _(preencher)_ | _(preencher)_ |
| _(preencher)_ | _(preencher)_ |

**Repositório GitHub:** _(preencher: https://github.com/UTILIZADOR/REPOSITORIO)_

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
  (5 s), garantindo que ocorre um boost durante essa fase. Observa-se que A_misto entra na fase
  interativa ainda no nível 2 (bursts curtos não sobem de nível), sobe ao nível 0 com o boost,
  mantém-se lá enquanto é interativo e volta a descer até ao nível 2 na fase batch final.

---

### 4. Resultados

<!-- TABELAS -->

---

### 5. Análise

<!-- ANALISE -->
