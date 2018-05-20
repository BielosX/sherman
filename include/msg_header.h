#ifndef __MSG_HEADER_H__
#define __MSG_HEADER_H__

#include <stdint.h>

#define SUBSCRIBE 1
#define SEND 2

typedef struct __attribute__((__packed__)) {
    uint8_t opcode;
    uint16_t topic_len;
    uint16_t body_len;
} msg_header_t;

#endif
