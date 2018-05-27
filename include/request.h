#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdint.h>

#include "client_socket.h"

void subscribe_to_topic(client_socket_t* socket, char* topic);
void send_to_topic(client_socket_t* socket, char* topic, uint8_t* body, uint16_t body_len);

#endif
