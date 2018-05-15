#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include "global_config.h"
#include "concurrent_queue.h"
#include "error.h"
#include "client_socket.h"

#define UNUSED(x) (void)x

void set_serv_addr(struct sockaddr_in* sockaddr) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(global_config.port);
    sockaddr->sin_addr.s_addr = htonl(INADDR_ANY);
}

void* consumer_thread_main(void* args) {
    char buffer[128];
    concurrent_queue_t* queue = (concurrent_queue_t*)args;
    while (true) {
        client_socket_t* client = (client_socket_t*)concurrent_queue_pop(queue);
        printf("[ThreadId=%lu] Queue pop\n", pthread_self());
        client_socket_read(client, buffer, 128);
        buffer[127] = '\0';
        printf("%s\n", buffer);
        close(client->fd);
        client_socket_destroy(client);
    }
    return NULL;
}

int init_threads(concurrent_queue_t* queue, pthread_t* threads, size_t len) {
    bool thread_init_failed = false;
    int result;
    for (unsigned int x = 0; x < len; ++x) {
        if (pthread_create(&threads[x], NULL, consumer_thread_main, queue) != 0) {
            printf("ERROR: Unable to create thread\n");
            thread_init_failed = true;
            break;
        }
    }
    result = 0;
    if (thread_init_failed) {
        for (unsigned int x = 0; x < global_config.threads; ++x) {
            pthread_cancel(threads[x]);
        }
        result = -1;
    }
    return result;
}

int inet_socket_bind(int fd, struct sockaddr_in* addr) {
    int result = 0;
    if (bind(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Unable to bind socket");
        result = -1;
    }
    return result;
}

int inet_socket_listen(int fd) {
    int result = 0;
    if (listen(fd, 5) == -1) {
        perror("Unable to mark socket as pasive");
        result = -1;
    }
    return result;
}

void wait_for_all(pthread_t* threads, size_t len) {
    void* thread_result;
    for (unsigned int x = 0; x < len; ++x) {
        pthread_join(threads[x], &thread_result);
    }
    UNUSED(thread_result);
}

int main(int argc, char** argv) {
    int result = 0;
    if (argc == 1) {
        printf("no path to config file specified\n");
        goto exit_main;
    }
    if (load_config(argv[1]) == -1) {
        goto exit_main;
    }
    struct sockaddr_in addr_in;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * global_config.threads);
    if (fd == -1) {
        perror("Unable to create socket");
        result = -1;
        goto exit_main;
    }
    set_serv_addr(&addr_in);
    TRY(inet_socket_bind(fd, &addr_in), exit_main);
    TRY(inet_socket_listen(fd), exit_main);
    print_global_config();
    concurrent_queue_t* queue;
    queue = concurrent_queue_new();
    TRY(init_threads(queue, threads, global_config.threads), delete_queue);
    int client_fd;
    while (true) {
        client_fd = accept(fd, NULL, NULL);
        if (client_fd == -1) {
            perror("Unable to accept connection");
        }
        else {
            printf("Client connected\n");
            concurrent_queue_push(queue, client_socket_create(client_fd));
        }
    }
    wait_for_all(threads, global_config.threads);
delete_queue:
    concurrent_queue_delete(queue);
    close(fd);
exit_main:
    free(threads);
    return result;
}
