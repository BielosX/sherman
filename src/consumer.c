#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "consumer.h"
#include "concurrent_queue.h"
#include "client_socket.h"
#include "msg_header.h"

static void to_host_byte_order(msg_header_t* header) {
    header->topic_len = ntohs(header->topic_len);
    header->body_len = ntohs(header->body_len);
}

static void handle_subscribe(msg_header_t* header, client_socket_t* client) {
    uint8_t buffer[128];
    memset(buffer, 0, sizeof(buffer));
    if (header->topic_len < 128) {
        client_socket_read(client, buffer, header->topic_len);
        printf("topic: %s\n", buffer);
    }
}

static void handle_send(msg_header_t* header, client_socket_t* client) {
    uint8_t topic[128];
    uint8_t body[128];
    memset(topic, 0, sizeof(topic));
    memset(body, 0, sizeof(body));
    if (header->topic_len < 128) {
        client_socket_read(client, topic, header->topic_len);
        printf("topic: %s\n", topic);
        if (header->body_len < 128) {
            client_socket_read(client, body, header->body_len);
            printf("body: %s\n", body);
        }
    }
}

void* consumer_thread_main(void* args) {
    concurrent_queue_t* queue = (concurrent_queue_t*)args;
    while (true) {
        client_socket_t* client = (client_socket_t*)concurrent_queue_pop(queue);
        printf("[ThreadId=%lu] Queue pop\n", pthread_self());
        pthread_mutex_lock(&client->mutex);
        msg_header_t header;
        to_host_byte_order(&header);
        client_socket_read(client, (uint8_t*)&header, sizeof(msg_header_t));
        switch(header.opcode) {
            case SUBSCRIBE:
                printf("[ThreadId=%lu] Subscribe request\n", pthread_self());
                handle_subscribe(&header, client);
                break;
            case SEND:
                printf("[ThreadId=%lu] Send request\n", pthread_self());
                handle_send(&header, client);
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&client->mutex);
        close(client->fd);
        client_socket_destroy(client);
    }
    return NULL;
}
