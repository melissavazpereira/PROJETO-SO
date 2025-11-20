/*
 * Producer-Consumer Problem
 * Implementação do problema produtor-consumidor usando semáforos
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

void *producer(void *arg) {
    int producer_id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;
        
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        
        buffer[in] = item;
        printf("Produtor %d produziu: %d na posição %d\n", producer_id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        
        usleep(rand() % 1000000);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    int consumer_id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        
        int item = buffer[out];
        printf("Consumidor %d consumiu: %d da posição %d\n", consumer_id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        
        usleep(rand() % 1500000);
    }
    
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;
    int prod_id = 1, cons_id = 1;
    
    printf("=== Problema Produtor-Consumidor ===\n");
    printf("Tamanho do buffer: %d\n", BUFFER_SIZE);
    printf("Número de itens: %d\n\n", NUM_ITEMS);
    
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    
    pthread_create(&prod_thread, NULL, producer, &prod_id);
    pthread_create(&cons_thread, NULL, consumer, &cons_id);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    
    printf("\n=== Simulação concluída ===\n");
    
    return 0;
}
