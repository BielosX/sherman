#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global_config.h"
#include "concurrent_queue.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("no path to config file specified\n");
        exit(-1);
    }
    if (load_config(argv[1]) == -1) {
        exit(-1);
    }
    print_global_config();
    concurrent_queue_t* queue;
    queue = concurrent_queue_new();
    int x = 5;
    int* y;
    concurrent_queue_push(queue, &x);
    y = (int*)concurrent_queue_pop(queue);
    printf("%d\n", *y);
    concurrent_queue_delete(queue);
    return 0;
}
