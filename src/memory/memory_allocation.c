/*
 * Memory Allocation Algorithms
 * Implementação de First Fit, Best Fit e Worst Fit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS 20
#define MAX_PROCESSES 20

typedef struct {
    int size;
    int allocated;
    int process_id;
} MemoryBlock;

void display_memory(MemoryBlock blocks[], int n) {
    printf("\n+-----+----------+-----------+------------+\n");
    printf("| Bloco | Tamanho | Alocado | Processo |\n");
    printf("+-----+----------+-----------+------------+\n");
    for (int i = 0; i < n; i++) {
        printf("| %3d | %8d | %9s | %10d |\n",
               i, blocks[i].size,
               blocks[i].allocated ? "Sim" : "Não",
               blocks[i].allocated ? blocks[i].process_id : -1);
    }
    printf("+-----+----------+-----------+------------+\n");
}

void first_fit(MemoryBlock blocks[], int n_blocks, int process_id, int process_size) {
    for (int i = 0; i < n_blocks; i++) {
        if (!blocks[i].allocated && blocks[i].size >= process_size) {
            blocks[i].allocated = 1;
            blocks[i].process_id = process_id;
            printf("Processo %d (%d KB) alocado no bloco %d (First Fit)\n",
                   process_id, process_size, i);
            return;
        }
    }
    printf("Processo %d (%d KB) não pôde ser alocado (sem espaço)\n",
           process_id, process_size);
}

void best_fit(MemoryBlock blocks[], int n_blocks, int process_id, int process_size) {
    int best_idx = -1;
    int min_diff = __INT_MAX__;
    
    for (int i = 0; i < n_blocks; i++) {
        if (!blocks[i].allocated && blocks[i].size >= process_size) {
            int diff = blocks[i].size - process_size;
            if (diff < min_diff) {
                min_diff = diff;
                best_idx = i;
            }
        }
    }
    
    if (best_idx != -1) {
        blocks[best_idx].allocated = 1;
        blocks[best_idx].process_id = process_id;
        printf("Processo %d (%d KB) alocado no bloco %d (Best Fit)\n",
               process_id, process_size, best_idx);
    } else {
        printf("Processo %d (%d KB) não pôde ser alocado (sem espaço)\n",
               process_id, process_size);
    }
}

void worst_fit(MemoryBlock blocks[], int n_blocks, int process_id, int process_size) {
    int worst_idx = -1;
    int max_diff = -1;
    
    for (int i = 0; i < n_blocks; i++) {
        if (!blocks[i].allocated && blocks[i].size >= process_size) {
            int diff = blocks[i].size - process_size;
            if (diff > max_diff) {
                max_diff = diff;
                worst_idx = i;
            }
        }
    }
    
    if (worst_idx != -1) {
        blocks[worst_idx].allocated = 1;
        blocks[worst_idx].process_id = process_id;
        printf("Processo %d (%d KB) alocado no bloco %d (Worst Fit)\n",
               process_id, process_size, worst_idx);
    } else {
        printf("Processo %d (%d KB) não pôde ser alocado (sem espaço)\n",
               process_id, process_size);
    }
}

int main() {
    int n_blocks, n_processes, algorithm;
    MemoryBlock blocks[MAX_BLOCKS];
    
    printf("=== Simulador de Alocação de Memória ===\n\n");
    
    printf("Número de blocos de memória: ");
    scanf("%d", &n_blocks);
    
    if (n_blocks > MAX_BLOCKS || n_blocks <= 0) {
        printf("Número inválido de blocos!\n");
        return 1;
    }
    
    for (int i = 0; i < n_blocks; i++) {
        printf("Tamanho do bloco %d (KB): ", i);
        scanf("%d", &blocks[i].size);
        blocks[i].allocated = 0;
        blocks[i].process_id = -1;
    }
    
    printf("\nEscolha o algoritmo:\n");
    printf("1 - First Fit\n");
    printf("2 - Best Fit\n");
    printf("3 - Worst Fit\n");
    printf("Algoritmo: ");
    scanf("%d", &algorithm);
    
    printf("\nNúmero de processos: ");
    scanf("%d", &n_processes);
    
    if (n_processes > MAX_PROCESSES || n_processes <= 0) {
        printf("Número inválido de processos!\n");
        return 1;
    }
    
    for (int i = 0; i < n_processes; i++) {
        int size;
        printf("Tamanho do processo %d (KB): ", i + 1);
        scanf("%d", &size);
        
        switch (algorithm) {
            case 1:
                first_fit(blocks, n_blocks, i + 1, size);
                break;
            case 2:
                best_fit(blocks, n_blocks, i + 1, size);
                break;
            case 3:
                worst_fit(blocks, n_blocks, i + 1, size);
                break;
            default:
                printf("Algoritmo inválido!\n");
                return 1;
        }
    }
    
    display_memory(blocks, n_blocks);
    
    return 0;
}
