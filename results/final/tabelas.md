## Cenário 1

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

## Cenário 2

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

## Cenário 3

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

## Cenário 4

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

## Cenário sjf

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

## Cenário rr

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

## Cenário mlfq1

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

## Cenário mlfq2

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

## Resumo: tempos médios por cenário (s)

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
