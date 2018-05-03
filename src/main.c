#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jansson.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("no path to config file specified\n");
        exit(-1);
    }
    FILE* config;
    config = fopen(argv[1], "r");
    if (config == NULL) {
        printf("Unable to open config file\n");
        exit(-1);
    }
    json_error_t error;
    json_t* config_json = json_loadf(config, 0, &error);
    if (config_json == NULL) {
        printf("Unable to decode config file\n");
    }
    json_decref(config_json);
    fclose(config);
}
