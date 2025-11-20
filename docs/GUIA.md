# Guia do Projeto de Sistemas Operacionais

## Visão Geral

Este projeto implementa conceitos fundamentais de Sistemas Operacionais, incluindo:

1. **Escalonamento de Processos**
2. **Gerenciamento de Memória**
3. **Sincronização de Threads**

## Estrutura do Projeto

```
PROJETO-SO/
├── src/
│   ├── scheduling/          # Algoritmos de escalonamento
│   │   ├── fcfs.c          # First Come First Served
│   │   └── round_robin.c   # Round Robin
│   ├── memory/             # Gerenciamento de memória
│   │   └── memory_allocation.c  # First/Best/Worst Fit
│   └── synchronization/    # Sincronização
│       ├── producer_consumer.c  # Problema produtor-consumidor
│       └── readers_writers.c    # Problema leitores-escritores
├── bin/                    # Executáveis compilados
├── docs/                   # Documentação
└── Makefile               # Sistema de build
```

## Compilação

### Compilar todos os programas:
```bash
make all
```

### Compilar módulos específicos:
```bash
make scheduling      # Compila escalonadores
make memory         # Compila gerenciamento de memória
make synchronization # Compila sincronização
```

### Limpar binários:
```bash
make clean
```

## Execução

### Escalonamento de Processos

#### FCFS (First Come First Served)
```bash
./bin/fcfs
```

Exemplo de entrada:
```
Número de processos: 3
Processo 1 - Tempo de chegada: 0
Processo 1 - Tempo de burst: 5
Processo 2 - Tempo de chegada: 1
Processo 2 - Tempo de burst: 3
Processo 3 - Tempo de chegada: 2
Processo 3 - Tempo de burst: 8
```

#### Round Robin
```bash
./bin/round_robin
```

Exemplo de entrada:
```
Número de processos: 3
Quantum de tempo: 2
Processo 1 - Tempo de chegada: 0
Processo 1 - Tempo de burst: 5
Processo 2 - Tempo de chegada: 1
Processo 2 - Tempo de burst: 3
Processo 3 - Tempo de chegada: 2
Processo 3 - Tempo de burst: 8
```

### Gerenciamento de Memória

```bash
./bin/memory_allocation
```

Exemplo de entrada:
```
Número de blocos de memória: 4
Tamanho do bloco 0 (KB): 100
Tamanho do bloco 1 (KB): 200
Tamanho do bloco 2 (KB): 150
Tamanho do bloco 3 (KB): 300
Escolha o algoritmo:
1 - First Fit
2 - Best Fit
3 - Worst Fit
Algoritmo: 1
Número de processos: 3
Tamanho do processo 1 (KB): 120
Tamanho do processo 2 (KB): 80
Tamanho do processo 3 (KB): 180
```

### Sincronização

#### Problema Produtor-Consumidor
```bash
./bin/producer_consumer
```

Este programa demonstra a sincronização entre threads produtoras e consumidoras usando semáforos.

#### Problema Leitores-Escritores
```bash
./bin/readers_writers
```

Este programa demonstra a sincronização entre threads leitoras e escritoras com prioridade de leitura.

## Conceitos Implementados

### 1. Escalonamento de Processos

**FCFS (First Come First Served)**
- Processos são executados na ordem de chegada
- Não preemptivo
- Simples mas pode causar convoy effect

**Round Robin**
- Cada processo recebe um quantum de tempo
- Preemptivo
- Melhor para sistemas interativos

### 2. Gerenciamento de Memória

**First Fit**
- Aloca no primeiro bloco disponível que couber
- Rápido mas pode causar fragmentação

**Best Fit**
- Aloca no menor bloco que couber
- Minimiza desperdício mas pode ser lento

**Worst Fit**
- Aloca no maior bloco disponível
- Deixa fragmentos maiores

### 3. Sincronização

**Problema Produtor-Consumidor**
- Usa semáforos para controlar buffer compartilhado
- Demonstra coordenação entre threads

**Problema Leitores-Escritores**
- Múltiplos leitores podem acessar simultaneamente
- Escritores têm acesso exclusivo
- Usa mutexes para sincronização

## Requisitos

- GCC (GNU Compiler Collection)
- POSIX Threads (pthread)
- Make
- Sistema operacional Linux/Unix

## Autor

Projeto desenvolvido para a disciplina de Sistemas Operacionais 2025/2026
