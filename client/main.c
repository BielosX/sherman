#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "msg_header.h"

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
    close(fd);
    return 0;
}
