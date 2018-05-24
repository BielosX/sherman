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
#include <poll.h>
#include <gmodule.h>

#include "global_config.h"
#include "concurrent_queue.h"
#include "error.h"
#include "client_socket.h"
#include "consumer.h"

#define UNUSED(x) (void)x
#define THIRTY_SECONDS 1000 * 30

void set_serv_addr(struct sockaddr_in* sockaddr) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(global_config.port);
    sockaddr->sin_addr.s_addr = htonl(INADDR_ANY);
}

int init_threads(concurrent_queue_t** queues, pthread_t* threads, size_t len) {
    bool thread_init_failed = false;
    int result;
    for (unsigned int x = 0; x < len; ++x) {
        if (pthread_create(&threads[x], NULL, consumer_thread_main, queues) != 0) {
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

void init_read_pollfd(struct pollfd* pl, int fd) {
    memset(pl, 0, sizeof(struct pollfd));
    pl->fd = fd;
    pl->events = POLLIN;
}

void init_write_pollfd(struct pollfd* pl, int fd) {
    memset(pl, 0, sizeof(struct pollfd));
    pl->fd = fd;
    pl->events = POLLOUT;
}

void handle_connect_event(struct pollfd* ptr, GArray* array, int server_fd) {
    int client_fd;

    if (ptr->fd == server_fd && (ptr->revents & POLLIN)) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("Unable to accept connection");
        }
        else {
            printf("Client connected\n");
            struct pollfd client_poll_fd;
            init_write_pollfd(&client_poll_fd, client_fd);
            g_array_append_val(array, client_poll_fd);
        }
    }
}

bool handle_write_event(struct pollfd* ptr, GArray* array, concurrent_queue_t* queue, int index) {
    bool handle = false;
    if (ptr->revents & POLLOUT) {
        printf("Write event\n");
        concurrent_queue_push(queue, client_socket_create(ptr->fd));
        g_array_remove_index(array, index);
        handle = true;
    }
    return handle;
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
    concurrent_queue_t* queues[2];
    queues[0] = concurrent_queue_new();
    queues[1] = concurrent_queue_new();
    TRY(init_threads(queues, threads, global_config.threads), delete_queue);
    GArray* active_file_descriptors = g_array_new(FALSE, FALSE, sizeof(struct pollfd));
    struct pollfd server_fd;
    init_read_pollfd(&server_fd, fd);
    g_array_append_val(active_file_descriptors, server_fd);
    int poll_res;
    while (true) {
        poll_res = poll((struct pollfd*)active_file_descriptors->data, active_file_descriptors->len, THIRTY_SECONDS);
        if (poll_res > 0) {
            for (unsigned int i = 0; i < active_file_descriptors->len; ++i) {
                struct pollfd* ptr = &((struct pollfd*)active_file_descriptors->data)[i];
                handle_connect_event(ptr, active_file_descriptors, fd);
                /* if item is being removed all elements after it are moved left.
                 * So if item on index 5 is removed then item on index 6 becomes item on index 5
                 * and index 5 needs to be handled again */
                if (handle_write_event(ptr, active_file_descriptors, queues[0], i)) {
                    i--;
                }
            }
        }
        int size = get_size(queues[1]);
        client_socket_t* client_socket;
        struct pollfd client_poll_fd;
        /* this is the only consumer thread that fetches from this queue
         * so it is not possible to get higher size and stall during pop procedure */
        for (int x = 0; x < size; ++x) {
            client_socket = (client_socket_t*)concurrent_queue_pop(queues[1]);
            init_write_pollfd(&client_poll_fd, client_socket->fd);
            printf("Client socket fd=%d request handled\n", client_socket->fd);
            g_array_append_val(active_file_descriptors, client_poll_fd);
            client_socket_destroy(client_socket);
        }
    }
    g_array_free(active_file_descriptors, FALSE);
    wait_for_all(threads, global_config.threads);
delete_queue:
    concurrent_queue_delete(queues[0]);
    concurrent_queue_delete(queues[1]);
    close(fd);
exit_main:
    free(threads);
    return result;
}
