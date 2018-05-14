#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

typedef struct {
    int max_queue_size;
    int max_number_of_topics;
    int max_listeners_per_topic;
    int max_topic_name_length;
    int max_connections;
    int port;
    int threads;
} global_config_t;

extern global_config_t global_config;

int load_config(char* file_path);
void print_global_config(void);

#endif

