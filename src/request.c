#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include "request.h"
#include "msg_header.h"

void subscribe_to_topic(client_socket_t* socket, char* topic) {
    msg_header_t header;
    header.opcode = SUBSCRIBE;
    header.topic_len = htons(strlen(topic) + 1);
    header.body_len = 0;
    pthread_mutex_lock(&socket->mutex);
    client_socket_write(socket, (uint8_t*)&header, sizeof(header));
    client_socket_write(socket, (uint8_t*)topic, strlen(topic) + 1);
    pthread_mutex_unlock(&socket->mutex);
}

void send_to_topic(client_socket_t* socket, char* topic, uint8_t* body, uint16_t body_len) {
    msg_header_t header;
    header.opcode = SEND;
    header.topic_len = htons(strlen(topic) + 1);
    header.body_len = htons(body_len);
    pthread_mutex_lock(&socket->mutex);
    client_socket_write(socket, (uint8_t*)&header, sizeof(header));
    client_socket_write(socket, (uint8_t*)topic, strlen(topic) + 1);
    client_socket_write(socket, body, body_len);
    pthread_mutex_unlock(&socket->mutex);
}
