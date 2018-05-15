#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

void client_socket_read(client_socket_t* client_socket, char* buffer, size_t buffer_len) {
    char chunk[32];
    ssize_t fetched;
    char* buffer_ptr = buffer;
    int fd = client_socket->fd;
    int left = buffer_len;
    int to_copy;
    pthread_mutex_lock(&client_socket->mutex);
    printf("Reading from socket\n");
    do {
        memset(chunk, 0, sizeof(chunk));
        fetched = read(fd, chunk, sizeof(chunk));
        if (fetched < 0) {
            perror("Unable to read from socket");
            break;
        }
        to_copy = min(fetched, buffer_len);
        memcpy(buffer_ptr, chunk, to_copy);
        buffer_ptr += to_copy;
        left -= to_copy;
    } while(fetched > 0 && left > 0);
    pthread_mutex_unlock(&client_socket->mutex);
}
