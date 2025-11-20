# PROJETO-SO
Projeto de Sistemas Operacionais 2025/2026

## 📚 Descrição

Este projeto implementa conceitos fundamentais de Sistemas Operacionais em C, incluindo algoritmos de escalonamento, gerenciamento de memória e sincronização de threads.

## 🚀 Funcionalidades

### Escalonamento de Processos
- **FCFS** (First Come First Served) - Escalonamento não preemptivo
- **Round Robin** - Escalonamento preemptivo com quantum de tempo

### Gerenciamento de Memória
- **First Fit** - Primeira partição disponível
- **Best Fit** - Melhor partição (menor desperdício)
- **Worst Fit** - Maior partição disponível

### Sincronização de Threads
- **Produtor-Consumidor** - Sincronização com semáforos
- **Leitores-Escritores** - Controle de acesso compartilhado

## 🛠️ Compilação

```bash
make all
```

## 📖 Documentação

Consulte o [guia completo](docs/GUIA.md) para instruções detalhadas de uso e exemplos.

## 🏃 Execução Rápida

```bash
# Compilar todos os programas
make all

# Executar demonstração de todos os módulos
make run-all

# Executar programas individuais
./bin/fcfs
./bin/round_robin
./bin/memory_allocation
./bin/producer_consumer
./bin/readers_writers
```

## 📋 Requisitos

- GCC (GNU Compiler Collection)
- POSIX Threads (pthread)
- Make
- Linux/Unix

## 🗂️ Estrutura do Projeto

```
PROJETO-SO/
├── src/
│   ├── scheduling/          # Algoritmos de escalonamento
│   ├── memory/             # Gerenciamento de memória
│   └── synchronization/    # Sincronização de threads
├── bin/                    # Executáveis compilados
├── docs/                   # Documentação
└── Makefile               # Sistema de build
```

## 👨‍💻 Autor

Projeto desenvolvido para a disciplina de Sistemas Operacionais 2025/2026
