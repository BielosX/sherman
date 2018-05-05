#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jansson.h>

#include "global_config.h"

#define TRY(f, g) \
    if (f < 0) {\
        goto g; \
    } \


global_config_t global_config;

static int get_value_from_object(json_t* obj, const char* key, json_t** v) {
    json_t* value;
    int result = 0;

    value = json_object_get(obj, key);
    if (value == NULL) {
        printf("ERROR: %s is not specified\n", key);
        result = -1;
        *v = NULL;
    }
    else {
        *v = value;
    }
    return result;
}

int load_config(char* file_path) {
    FILE* config;
    int result;
    json_t* value;

    config = fopen(file_path, "r");
    if (config == NULL) {
        printf("Unable to open config file\n");
        exit(-1);
    }
    json_error_t error;
    json_t* config_json = json_loadf(config, 0, &error);
    if (config_json == NULL) {
        printf("Unable to decode config file\n");
        printf("%s\n", error.text);
    }
    TRY(get_value_from_object(config_json, "maxQueueSize", &value), parse_error);
    global_config.max_queue_size = json_integer_value(value);
    TRY(get_value_from_object(config_json, "maxNumberOfTopics", &value), parse_error);
    global_config.max_number_of_topics = json_integer_value(value);
    TRY(get_value_from_object(config_json, "maxListenersPerTopic", &value), parse_error);
    global_config.max_listeners_per_topic = json_integer_value(value);
    TRY(get_value_from_object(config_json, "maxTopicNameLength", &value), parse_error);
    global_config.max_topic_name_length = json_integer_value(value);
    TRY(get_value_from_object(config_json, "maxConnections", &value), parse_error);
    global_config.max_connections = json_integer_value(value);
    result = 0;
    goto close_file;
parse_error:
    result = -1;
close_file:
    json_decref(config_json);
    fclose(config);
    return result;
}

void print_global_config(void) {
    printf("max_queue_size: %d\n", global_config.max_queue_size);
    printf("max_number_of_topics: %d\n", global_config.max_number_of_topics);
    printf("max_listeners_per_topic: %d\n", global_config.max_listeners_per_topic);
    printf("max_topic_name_length: %d\n", global_config.max_topic_name_length);
    printf("max_connections: %d\n", global_config.max_connections);
}
