# Makefile para o Projeto de Sistemas Operacionais
CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
SRCDIR = src
BINDIR = bin

# Criar diretório bin se não existir
$(shell mkdir -p $(BINDIR))

# Alvos principais
all: scheduling memory synchronization

scheduling: fcfs round_robin

memory: memory_allocation

synchronization: producer_consumer readers_writers

# Escalonadores
fcfs: $(SRCDIR)/scheduling/fcfs.c
	$(CC) $(CFLAGS) $< -o $(BINDIR)/fcfs

round_robin: $(SRCDIR)/scheduling/round_robin.c
	$(CC) $(CFLAGS) $< -o $(BINDIR)/round_robin

# Gerenciamento de memória
memory_allocation: $(SRCDIR)/memory/memory_allocation.c
	$(CC) $(CFLAGS) $< -o $(BINDIR)/memory_allocation

# Sincronização
producer_consumer: $(SRCDIR)/synchronization/producer_consumer.c
	$(CC) $(CFLAGS) $< -o $(BINDIR)/producer_consumer

readers_writers: $(SRCDIR)/synchronization/readers_writers.c
	$(CC) $(CFLAGS) $< -o $(BINDIR)/readers_writers

# Limpar binários
clean:
	rm -rf $(BINDIR)/*

# Executar todos os programas (modo demonstração)
run-all: all
	@echo "\n=== Executando FCFS ==="
	@echo "3\n0\n5\n1\n3\n2\n8" | $(BINDIR)/fcfs
	@echo "\n=== Executando Round Robin ==="
	@echo "3\n2\n0\n5\n1\n3\n2\n8" | $(BINDIR)/round_robin
	@echo "\n=== Executando Memory Allocation ==="
	@echo "4\n100\n200\n150\n300\n1\n3\n120\n80\n180" | $(BINDIR)/memory_allocation
	@echo "\n=== Executando Producer-Consumer ==="
	@$(BINDIR)/producer_consumer
	@echo "\n=== Executando Readers-Writers ==="
	@$(BINDIR)/readers_writers

.PHONY: all scheduling memory synchronization clean run-all
