#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global_config.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("no path to config file specified\n");
        exit(-1);
    }
    if (load_config(argv[1]) == -1) {
        exit(-1);
    }
    print_global_config();
    return 0;
}
