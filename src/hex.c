#include <stdio.h>

#include "hex.h"

void print_hex(uint8_t* buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        if (i != len - 1) {
            printf("0x%X ", buffer[i]);
        }
        else {
            printf("0x%X", buffer[i]);
        }
    }
    printf("\n");
}
