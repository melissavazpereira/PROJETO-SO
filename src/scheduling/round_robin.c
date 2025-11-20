/*
 * Round Robin Scheduler
 * Implementação de um escalonador Round Robin
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} Process;

void round_robin_schedule(Process processes[], int n, int quantum) {
    int current_time = 0;
    int completed = 0;
    int queue[MAX_PROCESSES];
    int front = 0, rear = 0;
    int visited[MAX_PROCESSES] = {0};
    
    printf("\n=== Round Robin Scheduling (Quantum = %d) ===\n", quantum);
    
    // Initialize remaining time
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
    }
    
    // Add first process
    queue[rear++] = 0;
    visited[0] = 1;
    
    while (completed < n) {
        if (front == rear) {
            // Queue empty, advance time
            current_time++;
            for (int i = 0; i < n; i++) {
                if (!visited[i] && processes[i].arrival_time <= current_time) {
                    queue[rear++] = i;
                    visited[i] = 1;
                }
            }
            continue;
        }
        
        int idx = queue[front++];
        
        if (processes[idx].remaining_time > quantum) {
            current_time += quantum;
            processes[idx].remaining_time -= quantum;
        } else {
            current_time += processes[idx].remaining_time;
            processes[idx].remaining_time = 0;
            processes[idx].completion_time = current_time;
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }
        
        // Add newly arrived processes
        for (int i = 0; i < n; i++) {
            if (!visited[i] && processes[i].arrival_time <= current_time) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }
        
        // Re-add current process if not finished
        if (processes[idx].remaining_time > 0) {
            queue[rear++] = idx;
        }
    }
    
    printf("PID\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
    }
    
    float avg_turnaround = 0, avg_waiting = 0;
    for (int i = 0; i < n; i++) {
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
    }
    
    printf("\nTempo médio de turnaround: %.2f\n", avg_turnaround / n);
    printf("Tempo médio de espera: %.2f\n", avg_waiting / n);
}

int main() {
    int n, quantum;
    Process processes[MAX_PROCESSES];
    
    printf("Número de processos: ");
    scanf("%d", &n);
    
    if (n > MAX_PROCESSES || n <= 0) {
        printf("Número inválido de processos!\n");
        return 1;
    }
    
    printf("Quantum de tempo: ");
    scanf("%d", &quantum);
    
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("Processo %d - Tempo de chegada: ", i + 1);
        scanf("%d", &processes[i].arrival_time);
        printf("Processo %d - Tempo de burst: ", i + 1);
        scanf("%d", &processes[i].burst_time);
    }
    
    round_robin_schedule(processes, n, quantum);
    
    return 0;
}
