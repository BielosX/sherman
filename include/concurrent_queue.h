#ifndef __CONCURRENT_QUEUE_H__
#define __CONCURRENT_QUEUE_H__

#include <pthread.h>
#include <semaphore.h>

struct concurrent_queue_node {
    void* value;
    struct concurrent_queue_node* next;
};

typedef struct concurrent_queue_node concurrent_queue_node_t;

typedef struct {
    concurrent_queue_node_t* head;
    concurrent_queue_node_t* tail;
    pthread_mutex_t head_mutex;
    pthread_mutex_t tail_mutex;
    sem_t tickets;
} concurrent_queue_t;

void concurrent_queue_push(concurrent_queue_t* queue, void* item);
void* concurrent_queue_pop(concurrent_queue_t* queue);
concurrent_queue_t* concurrent_queue_new(void);
void concurrent_queue_delete(concurrent_queue_t* queue);
int get_size(concurrent_queue_t* queue);

#endif
