#include <stdlib.h>
#include <errno.h>

#include "concurrent_queue.h"

void concurrent_queue_push(concurrent_queue_t* queue, void* item) {
    concurrent_queue_node_t* node;
    node = (concurrent_queue_node_t*)malloc(sizeof(concurrent_queue_node_t));
    node->value = item;
    node->next = NULL;
    int r = sem_trywait(&queue->tickets);
    int error = errno;
    pthread_mutex_lock(&queue->head_mutex);
    if (r == -1 && error == EAGAIN) {
        pthread_mutex_lock(&queue->tail_mutex);
    }
    if (queue->head == NULL || queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;
    }
    else {
        queue->head->next = node;
        queue->head = node;
        sem_post(&queue->tickets);
    }
    if (r == -1 && error == EAGAIN) {
        pthread_mutex_unlock(&queue->tail_mutex);
    }
    sem_post(&queue->tickets);
    pthread_mutex_unlock(&queue->head_mutex);
}

void* concurrent_queue_pop(concurrent_queue_t* queue) {
    concurrent_queue_node_t* node;
    void* ptr;
    sem_wait(&queue->tickets);
    pthread_mutex_lock(&queue->tail_mutex);
    node = queue->tail;
    ptr = node->value;
    queue->tail = node->next;
    free(node);
    pthread_mutex_unlock(&queue->tail_mutex);
    sem_post(&queue->tickets);
    return ptr;
}

concurrent_queue_t* concurrent_queue_new(void) {
    concurrent_queue_t* queue;
    queue = (concurrent_queue_t*)malloc(sizeof(concurrent_queue_t));
    queue->tail = NULL;
    queue->head = NULL;
    pthread_mutex_init(&queue->tail_mutex, NULL);
    pthread_mutex_init(&queue->head_mutex, NULL);
    sem_init(&queue->tickets, 0, 0);
    return queue;
}

void concurrent_queue_delete(concurrent_queue_t* queue) {
    concurrent_queue_node_t* node;
    concurrent_queue_node_t* temp;
    node = queue->tail;
    while(node != NULL) {
        temp = node->next;
        free(node);
        node = temp;
    }
    pthread_mutex_destroy(&queue->head_mutex);
    pthread_mutex_destroy(&queue->tail_mutex);
    sem_destroy(&queue->tickets);
    free(queue);
}

