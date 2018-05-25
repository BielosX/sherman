#ifndef __CONSUMER_H__
#define __CONSUMER_H__

#include "concurrent_queue.h"

typedef struct {
    concurrent_queue_t* request_queue;
    concurrent_queue_t* socket_ret_queue;
} consumer_attr_t;

void* consumer_thread_main(void* args);

#endif
