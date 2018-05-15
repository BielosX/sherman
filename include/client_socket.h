#ifndef __CLIENT_SOCKET_H__
#define __CLIENT_SOCKET_H__

typedef struct {
    int fd;
    pthread_mutex_t mutex;
} client_socket_t;

client_socket_t* client_socket_create(int fd);
void client_socket_destroy(client_socket_t* client_socket);
void client_socket_read(client_socket_t* client_socket, char* buffer, size_t buffer_len);

#endif
