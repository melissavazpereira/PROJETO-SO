/*
 * Readers-Writers Problem
 * Implementação do problema leitores-escritores
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_READERS 3
#define NUM_WRITERS 2
#define NUM_OPERATIONS 5

int shared_data = 0;
int reader_count = 0;

pthread_mutex_t mutex;
pthread_mutex_t write_mutex;

void *reader(void *arg) {
    int reader_id = *(int *)arg;
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        reader_count++;
        if (reader_count == 1) {
            pthread_mutex_lock(&write_mutex);
        }
        pthread_mutex_unlock(&mutex);
        
        // Reading
        printf("Leitor %d leu: %d\n", reader_id, shared_data);
        usleep(rand() % 500000);
        
        pthread_mutex_lock(&mutex);
        reader_count--;
        if (reader_count == 0) {
            pthread_mutex_unlock(&write_mutex);
        }
        pthread_mutex_unlock(&mutex);
        
        usleep(rand() % 1000000);
    }
    
    return NULL;
}

void *writer(void *arg) {
    int writer_id = *(int *)arg;
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&write_mutex);
        
        // Writing
        shared_data++;
        printf("Escritor %d escreveu: %d\n", writer_id, shared_data);
        usleep(rand() % 700000);
        
        pthread_mutex_unlock(&write_mutex);
        
        usleep(rand() % 1500000);
    }
    
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];
    
    printf("=== Problema Leitores-Escritores ===\n");
    printf("Número de leitores: %d\n", NUM_READERS);
    printf("Número de escritores: %d\n", NUM_WRITERS);
    printf("Operações por thread: %d\n\n", NUM_OPERATIONS);
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&write_mutex, NULL);
    
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&write_mutex);
    
    printf("\n=== Simulação concluída ===\n");
    printf("Valor final do dado compartilhado: %d\n", shared_data);
    
    return 0;
}
