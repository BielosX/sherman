#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "consumer.h"
#include "concurrent_queue.h"
#include "client_socket.h"
#include "msg_header.h"
#include "hex.h"
#include "subscribers.h"

static void to_host_byte_order(msg_header_t* header) {
    header->topic_len = ntohs(header->topic_len);
    header->body_len = ntohs(header->body_len);
}

static char* create_key(char buffer[]) {
    char* str;
    size_t len = strlen(buffer);
    size_t size = len * sizeof(char) + 1;
    str = (char*)malloc(size);
    memset(str, 0, size);
    memcpy(str, buffer, size - 1);
    return str;
}

static int handle_subscribe(msg_header_t* header, client_socket_t* client, subscribers_t* subscribers) {
    int result;
    uint8_t buffer[128];
    memset(buffer, 0, sizeof(buffer));
    printf("Topic len: %d\n", header->topic_len);
    if (header->topic_len < 128) {
        result = client_socket_read(client, buffer, header->topic_len);
        printf("topic: %s\n", buffer);
        char* key = create_key((char*)buffer);
        subscriber_t subscriber;
        pthread_mutex_init(&subscriber.mutex, NULL);
        subscriber.fd = client->fd;
        subscribers_add(subscribers, key, &subscriber);
    }
    return result;
}

static void handle_send(msg_header_t* header, client_socket_t* client, subscribers_t* subscribers) {
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
            subscribers_list_t* list = subscribers_get(subscribers, (char*)topic);
            if (list != NULL) {
                printf("Number of subscribers: %u\n", list->subscribers->len);
                pthread_mutex_lock(&list->mutex);
                msg_header_t hdr;
                memset(&hdr, 0, sizeof(msg_header_t));
                hdr.opcode = SEND;
                hdr.topic_len = htons(header->topic_len);
                hdr.body_len = htons(header->body_len);
                for(uint32_t i = 0; i < list->subscribers->len; ++i) {
                    subscriber_t sub = g_array_index(list->subscribers, subscriber_t, i);
                    client_socket_t* sub_socket = client_socket_create(sub.fd);
                    pthread_mutex_lock(&sub_socket->mutex);
                    client_socket_write(sub_socket, (uint8_t*)&hdr, sizeof(hdr));
                    client_socket_write(sub_socket, topic, header->topic_len);
                    client_socket_write(sub_socket, body, header->body_len);
                    pthread_mutex_unlock(&sub_socket->mutex);
                    client_socket_destroy(sub_socket);
                }
                pthread_mutex_unlock(&list->mutex);
            }
            else {
                printf("No subscriptions for topic: %s\n", topic);
            }
        }
    }
}

void* consumer_thread_main(void* args) {
    consumer_attr_t* attr = (consumer_attr_t*)args;
    concurrent_queue_t* queue = attr->request_queue;
    concurrent_queue_t* resp_queue = attr->socket_ret_queue;
    subscribers_t* subscribers = attr->subscribers;
    int result;
    while (true) {
        client_socket_t* client = (client_socket_t*)concurrent_queue_pop(queue);
        printf("[ThreadId=%lu] Queue pop\n", pthread_self());
        pthread_mutex_lock(&client->mutex);
        msg_header_t header;
        client_socket_read(client, (uint8_t*)&header, sizeof(msg_header_t));
        to_host_byte_order(&header);
        switch(header.opcode) {
            case SUBSCRIBE:
                printf("[ThreadId=%lu] Subscribe request\n", pthread_self());
                result = handle_subscribe(&header, client, subscribers);
                break;
            case SEND:
                printf("[ThreadId=%lu] Send request\n", pthread_self());
                handle_send(&header, client, subscribers);
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&client->mutex);
        if (result == -1 || result == 0) {
            printf("[ThreadId=%lu] Connection closed or error\n", pthread_self());
            close(client->fd);
            client_socket_destroy(client);
        }
        else {
            printf("[ThreadId=%lu] Sending fd=%d back\n", pthread_self(), client->fd);
            concurrent_queue_push(resp_queue, client);
        }
    }
    return NULL;
}
