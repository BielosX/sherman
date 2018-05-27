#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <poll.h>

#include "msg_header.h"
#include "client_socket.h"
#include "request.h"

typedef struct {
    client_socket_t* client_socket;
    int pipe_read_fd;
} listener_args_t;

void init_listener_pollfds(struct pollfd fds[], listener_args_t* args) {
    memset(fds, 0, sizeof(struct pollfd) * 2);
    fds[0].fd = args->pipe_read_fd;
    fds[0].events = POLLIN;
    fds[1].fd = args->client_socket->fd;
    fds[1].events = POLLIN;
}

void* listener(void* args) {
    listener_args_t* listener_args = (listener_args_t*)args;
    msg_header_t header;
    uint8_t topic[128];
    uint8_t body[128];
    struct pollfd fds[2];
    init_listener_pollfds(fds, listener_args);
    bool exit = false;
    int poll_ret;
    do {
        poll_ret = poll(fds, 2, -1);
        if (poll_ret > 0) {
            if (fds[1].revents & POLLIN) {
                /* it is fine to use the same socket in two threads unless both use it to write */
                client_socket_read(listener_args->client_socket, (uint8_t*)&header, sizeof(msg_header_t));
                memset(topic, 0, sizeof(topic));
                memset(body, 0, sizeof(topic));
                client_socket_read(listener_args->client_socket, topic, ntohs(header.topic_len));
                client_socket_read(listener_args->client_socket, body, ntohs(header.body_len));
                printf("topic: %s\n", topic);
                printf("body: %s\n", body);
            }
            if (fds[0].revents & POLLIN) {
                if (read(fds[0].fd, &exit, sizeof(exit)) == -1) {
                    perror("Unable to read from the pipe");
                }
            }
        }
    } while (!exit);
    printf("Finishing thread\n");
    return NULL;
}

void init_sockaddr(struct sockaddr_in* sockaddr, struct in_addr* addr, int port) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);
    memcpy(&sockaddr->sin_addr, addr, sizeof(struct in_addr));
}

void subscribe(client_socket_t* client_socket) {
    char buffer[32];
    printf("Topic: \n");
    memset(buffer, 0, sizeof(buffer));
    scanf("%s", buffer);
    subscribe_to_topic(client_socket, buffer);
}

void send_to_all(client_socket_t* client_socket) {
    char topic[32];
    char body[32];
    memset(topic, 0, sizeof(topic));
    memset(body, 0, sizeof(body));
    printf("Topic: \n");
    scanf("%s", topic);
    printf("Body: \n");
    scanf("%s", body);
    send_to_topic(client_socket, topic, (uint8_t*)body, strlen(body) + 1);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("client server_ip server_port");
        exit(-1);
    }
    struct in_addr addr;
    int port = atoi(argv[2]);
    if (inet_pton(AF_INET, argv[1], &addr) == 0) {
        printf("wrong addr format\n");
        exit(-1);
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Unable to create socket");
        exit(-1);
    }
    struct sockaddr_in server_addr;
    init_sockaddr(&server_addr, &addr, port);
    if (connect(fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Unable to connect");
    }
    pthread_t thread;
    listener_args_t args;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Unable to create pipe");
        close(fd);
        exit(-1);
    }
    client_socket_t* client_socket = client_socket_create(fd);
    args.client_socket = client_socket;
    args.pipe_read_fd = pipefd[0];
    if (pthread_create(&thread, NULL, listener, &args) < 0) {
        printf("Unable to start thread\n");
        goto destroy_socket;
    }
    unsigned int choice;
    bool finish_thread;
    do {
    printf("1. Subscribe\n 2. Send\n 3. Exit\n");
    scanf("%u", &choice);
        switch(choice) {
            case 1:
                subscribe(client_socket);
                break;
            case 2:
                send_to_all(client_socket);
                break;
            case 3:
                finish_thread = true;
                if (write(pipefd[1], &finish_thread, sizeof(finish_thread)) == -1) {
                    perror("Unable to write to pipe");
                }
                break;
            default:
                break;
        }
    } while(choice != 3);
    if (pthread_join(thread, NULL) < 0) {
        printf("pthread_join failed\n");
    }
destroy_socket:
    client_socket_destroy(client_socket);
    close(fd);
    return 0;
}
