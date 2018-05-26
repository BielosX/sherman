#ifndef __SUBSCRIBERS_H__
#define __SUBSCRIBERS_H__

#include <pthread.h>
#include <gmodule.h>

typedef struct {
    GHashTable* hash_table;
    pthread_mutex_t mutex;
} subscribers_t;

typedef struct {
    int fd;
    pthread_mutex_t mutex;
} subscriber_t;

typedef struct {
    GArray* subscribers;
    pthread_mutex_t mutex;
} subscribers_list_t;

subscribers_t* subscribers_new(void);
void subscribers_delete(subscribers_t* ptr);
subscribers_list_t* subscribers_list_new(void);
void subscribers_list_delete(subscribers_list_t* list);
subscribers_list_t* subscribers_get(subscribers_t* ptr, char* topic);
void subscribers_add(subscribers_t* ptr, char* topic, subscriber_t* subscriber);

#endif
