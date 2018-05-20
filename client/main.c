#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "msg_header.h"
#include "client_socket.h"
#include "hex.h"

void init_sockaddr(struct sockaddr_in* sockaddr, struct in_addr* addr, int port) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);
    memcpy(&sockaddr->sin_addr, addr, sizeof(struct in_addr));
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
    char buffer[32];
    printf("Topic: ");
    memset(buffer, 0, sizeof(buffer));
    msg_header_t header;
    scanf("%s", buffer);
    header.opcode = SUBSCRIBE;
    header.topic_len = htons(strlen(buffer));
    header.body_len = 0;
    client_socket_t* client_socket = client_socket_create(fd);
    client_socket_write(client_socket, (uint8_t*)&header, sizeof(header));
    client_socket_write(client_socket, (uint8_t*)buffer, strlen(buffer));
    print_hex((uint8_t*)buffer, sizeof(buffer));
    client_socket_destroy(client_socket);
    close(fd);
    return 0;
}
