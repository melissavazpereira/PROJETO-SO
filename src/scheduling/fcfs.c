/*
 * FCFS (First Come First Served) Scheduler
 * Implementação de um escalonador FCFS simples
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} Process;

void fcfs_schedule(Process processes[], int n) {
    int current_time = 0;
    
    printf("\n=== FCFS Scheduling ===\n");
    printf("PID\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    
    for (int i = 0; i < n; i++) {
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }
        
        processes[i].completion_time = current_time + processes[i].burst_time;
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        
        current_time = processes[i].completion_time;
        
        printf("%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
    }
    
    // Calculate averages
    float avg_turnaround = 0, avg_waiting = 0;
    for (int i = 0; i < n; i++) {
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
    }
    
    printf("\nTempo médio de turnaround: %.2f\n", avg_turnaround / n);
    printf("Tempo médio de espera: %.2f\n", avg_waiting / n);
}

int main() {
    int n;
    Process processes[MAX_PROCESSES];
    
    printf("Número de processos: ");
    scanf("%d", &n);
    
    if (n > MAX_PROCESSES || n <= 0) {
        printf("Número inválido de processos!\n");
        return 1;
    }
    
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("Processo %d - Tempo de chegada: ", i + 1);
        scanf("%d", &processes[i].arrival_time);
        printf("Processo %d - Tempo de burst: ", i + 1);
        scanf("%d", &processes[i].burst_time);
    }
    
    fcfs_schedule(processes, n);
    
    return 0;
}
