# Exemplos de Entrada

Este diretório contém exemplos de entrada para os programas do projeto.

## Uso

### FCFS Scheduler
```bash
./bin/fcfs < examples/fcfs_input.txt
```

### Round Robin Scheduler
```bash
./bin/round_robin < examples/round_robin_input.txt
```

### Memory Allocation
```bash
./bin/memory_allocation < examples/memory_input.txt
```

### Producer-Consumer (sem entrada)
```bash
./bin/producer_consumer
```

### Readers-Writers (sem entrada)
```bash
./bin/readers_writers
```

## Formato das Entradas

### FCFS / Round Robin
```
<número de processos>
[Para Round Robin: <quantum>]
<tempo de chegada do processo 1>
<tempo de burst do processo 1>
<tempo de chegada do processo 2>
<tempo de burst do processo 2>
...
```

### Memory Allocation
```
<número de blocos>
<tamanho do bloco 1>
<tamanho do bloco 2>
...
<algoritmo: 1=First Fit, 2=Best Fit, 3=Worst Fit>
<número de processos>
<tamanho do processo 1>
<tamanho do processo 2>
...
```
