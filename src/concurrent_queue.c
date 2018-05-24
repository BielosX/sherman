#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "concurrent_queue.h"

void concurrent_queue_push(concurrent_queue_t* queue, void* item) {
    concurrent_queue_node_t* node;
    node = (concurrent_queue_node_t*)malloc(sizeof(concurrent_queue_node_t));
    node->value = item;
    node->next = NULL;
    /* this semaphore works as a tickets machine for poping threads.
     * Because of it it is possible to stall the thread if the queue is empty.
     * If the size of the queue is higher than one this semaphore allows
     * threads to pop and push concurrently without stalls */
    int r = sem_trywait(&queue->tickets);
    int error = errno;
    /* This mutex prevents from concurrent head pointer modification. It may happen if two threads
     * try to push new value at the same time */
    pthread_mutex_lock(&queue->head_mutex);
    /* there are two possibilities for the semaphore to be in zero state:
     * 1) queue is empty
     * 2) poping thread has just taken the last element
     * to prevent concurrent access to tail pointer (wich may happen if it is taken by poping thread)
     * tail_mutex is locked. Because it is not possible to disctinct
     *      it may be locked even if queue has not ever contained any item.
     * It is also possible that one thread that tries to push was preempted before head_mutex lock
     *      and during this time many threads pushed items. Then This thread will lock tail_mutex
     *      even if it is unnecessary (because of the r and error local variables that have not changed since preemption) */
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
    /* Wait for queue to be non-empty */
    sem_wait(&queue->tickets);
    /* It may happen that two threads try to pop a value. Because of this mutex
     * it is not possible to fetch the same item in two threads
     * also, because of the semaphore, if the size of the queue is equal to 1 only one
     * thread will take the item, the second one will stall and wait for next values */
    pthread_mutex_lock(&queue->tail_mutex);
    node = queue->tail;
    ptr = node->value;
    queue->tail = node->next;
    free(node);
    pthread_mutex_unlock(&queue->tail_mutex);
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

int get_size(concurrent_queue_t* queue) {
    int result;
    if (sem_getvalue(&queue->tickets, &result) == -1) {
        perror("Unable to get semaphore value");
    }
    return result;
}
