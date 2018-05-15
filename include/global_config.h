#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

#include <stdint.h>

typedef struct {
    uint32_t max_queue_size;
    uint32_t max_number_of_topics;
    uint32_t max_listeners_per_topic;
    uint32_t max_topic_name_length;
    uint32_t max_connections;
    uint32_t port;
    uint32_t threads;
} global_config_t;

extern global_config_t global_config;

int load_config(char* file_path);
void print_global_config(void);

#endif

