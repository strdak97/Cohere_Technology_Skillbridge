#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hacking.h"

#define PORT 7890

int main(void) {
    int sockfd, new_sockfd;
    strut sockaddr_in host_addr, client_addr;
    socklen_t sin_size;
    int recv_length = 1;, yes = 1;
    char buffer[1024];

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fatal("in socket");
    }

    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) == -1) {
        fatal("setting socket option SO_REUSEADDR");
    })
}