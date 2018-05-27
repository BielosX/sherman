#include <stdio.h>

#include "subscribers.h"

#define UNUSED(x) (void)x

subscribers_t* subscribers_new(void) {
    subscribers_t* ptr = (subscribers_t*)malloc(sizeof(subscribers_t));
    pthread_mutex_init(&ptr->mutex, NULL);
    ptr->hash_table = g_hash_table_new(g_str_hash, g_str_equal);
    return ptr;
}

static void remove_subscribers(gpointer data, gpointer user_data) {
    UNUSED(user_data);
    subscribers_list_t* list = (subscribers_list_t*)data;
    subscribers_list_delete(list);
}

static void remove_keys(gpointer data, gpointer user_data) {
    UNUSED(user_data);
    char* str = (char*)data;
    free(str);
}

void subscribers_delete(subscribers_t* ptr) {
    pthread_mutex_lock(&ptr->mutex);
    GList* values = g_hash_table_get_values(ptr->hash_table);
    g_list_foreach(values, remove_subscribers, NULL);
    g_list_free(values);
    GList* keys = g_hash_table_get_keys(ptr->hash_table);
    g_list_foreach(keys, remove_keys, NULL);
    g_hash_table_destroy(ptr->hash_table);
    pthread_mutex_unlock(&ptr->mutex);
    pthread_mutex_destroy(&ptr->mutex);
    free(ptr);
}

subscribers_list_t* subscribers_list_new(void) {
    subscribers_list_t* ptr = (subscribers_list_t*)malloc(sizeof(subscribers_list_t));
    pthread_mutex_init(&ptr->mutex, NULL);
    ptr->subscribers = g_array_new(FALSE, FALSE, sizeof(subscriber_t));
    return ptr;
}

void subscribers_list_delete(subscribers_list_t* list) {
    pthread_mutex_destroy(&list->mutex);
    g_array_free(list->subscribers, FALSE);
    free(list);
}

subscribers_list_t* subscribers_get(subscribers_t* ptr, char* topic) {
    subscribers_list_t* list;
    pthread_mutex_lock(&ptr->mutex);
    list = g_hash_table_lookup(ptr->hash_table, topic);
    pthread_mutex_unlock(&ptr->mutex);
    return list;
}

void subscribers_add(subscribers_t* ptr, char* topic, subscriber_t* subscriber) {
    subscribers_list_t* list;
    pthread_mutex_lock(&ptr->mutex);
    list = g_hash_table_lookup(ptr->hash_table, topic);
    if (list == NULL) {
        printf("Adding first subscriber to topic: %s\n", topic);
        list = subscribers_list_new();
        g_array_append_val(list->subscribers, *subscriber);
    }
    else {
        printf("Adding subscriber to topic: %s\n", topic);
        pthread_mutex_lock(&list->mutex);
        g_array_append_val(list->subscribers, *subscriber);
        pthread_mutex_unlock(&list->mutex);
    }
    g_hash_table_insert(ptr->hash_table, topic, list);
    pthread_mutex_unlock(&ptr->mutex);
}
