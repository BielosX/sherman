#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "client_socket.h"

client_socket_t* client_socket_create(int fd) {
    client_socket_t* client_socket;
    client_socket = (client_socket_t*)malloc(sizeof(client_socket_t));
    client_socket->fd = fd;
    pthread_mutex_init(&client_socket->mutex, NULL);
    return client_socket;
}

void client_socket_destroy(client_socket_t* client_socket) {
    pthread_mutex_destroy(&client_socket->mutex);
    free(client_socket);
}

static int min(int a, int b) {
    if (a > b)
        return b;
    else
        return a;
}

void client_socket_read(client_socket_t* client_socket, uint8_t* buffer, size_t buffer_len) {
    uint8_t chunk[32];
    ssize_t fetched;
    uint8_t* buffer_ptr = buffer;
    int fd = client_socket->fd;
    int left = buffer_len;
    int to_read;
    printf("Reading from socket %lu bytes\n", buffer_len);
    do {
        memset(chunk, 0, sizeof(chunk));
        to_read = min(sizeof(chunk), buffer_len);
        fetched = read(fd, chunk, to_read);
        if (fetched < 0) {
            perror("Unable to read from socket");
            break;
        }
        memcpy(buffer_ptr, chunk, fetched);
        buffer_ptr += fetched;
        left -= fetched;
    } while(fetched > 0 && left > 0);
}

void client_socket_write(client_socket_t* client_socket, uint8_t* buffer, size_t buffer_len) {
    uint8_t* buffer_ptr = buffer;
    int fd = client_socket->fd;
    int left = buffer_len;
    ssize_t sent;
    printf("Writing to socket %lu bytes\n", buffer_len);
    do {
        sent = write(fd, buffer_ptr, left);
        if (sent == -1) {
            perror("Unable to write to socket");
            break;
        }
        left -= sent;
        buffer_ptr += sent;
    } while(left > 0);
}

