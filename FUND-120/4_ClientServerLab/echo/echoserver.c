#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 4096
#define DEFAULT_PORT 8080

/* Function prototypes */
void error(const char *msg);
void parseCommandLineArgs(int argc, char **argv, int *port, int *maxPending);
void setupSocket(int *listenfd, int port);
void acceptConnections(int listenfd);

int main(int argc, char **argv) {
    int listenfd;        // Listening socket
    int port;            // Port to listen on
    int maxPending = 5;  // Maximum pending connections

    parseCommandLineArgs(argc, argv, &port, &maxPending);

    setupSocket(&listenfd, port);

    acceptConnections(listenfd);

    return 0;
}

/*
 * Prints an error message and exits the program.
 */
void error(const char *msg) {
    perror(msg);
    exit(1);
}

/*
 * Parses and sets the command line arguments.
 */
void parseCommandLineArgs(int argc, char **argv, int *port, int *maxPending) {
    int option_char;
    const char *usage =
        "usage:\n"
        "  echoserver [options]\n"
        "options:\n"
        "  -h                  Show this help message\n"
        "  -n                  Maximum pending connections\n"
        "  -p                  Port (Default: 8080)\n";

    struct option gLongOptions[] = {
        {"port", required_argument, NULL, 'p'},
        {"maxnpending", required_argument, NULL, 'n'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while ((option_char = getopt_long(argc, argv, "p:n:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p':
                *port = atoi(optarg);
                break;
            case 'n':
                *maxPending = atoi(optarg);
                break;
            case 'h':
                fprintf(stdout, "%s", usage);
                exit(0);
            default:
                fprintf(stderr, "%s", usage);
                exit(1);
        }
    }

    if (*port < 1025 || *port > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", *port);
        exit(1);
    }

    if (*maxPending < 1) {
        fprintf(stderr, "Invalid pending count: %d\n", *maxPending);
        exit(1);
    }
}

/*
 * Sets up the listening socket.
 */
void setupSocket(int *listenfd, int port) {
    int optval = 1;
    struct sockaddr_in serveraddr;

    *listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*listenfd < 0)
        error("ERROR opening socket");

    setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(*listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    if (listen(*listenfd, 5) < 0)
        error("ERROR on listen");
}

/*
 * Accepts incoming connections and handles each connection individually.
 */
void acceptConnections(int listenfd) {
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    struct hostent *hostp;
    char buf[BUFSIZE];
    int connfd;

    while (1) {
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0)
            error("ERROR on accept");

        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL)
            error("ERROR on gethostbyaddr");

        const char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            error("ERROR on inet_ntoa");

        printf("Server established connection with %s (%s)\n", hostp->h_name, hostaddrp);

        bzero(buf, BUFSIZE);
        int n = read(connfd, buf, BUFSIZE);
        if (n < 0)
            error("ERROR reading from socket");
        printf("Server received %d bytes: %s\n", n, buf);

        n = write(connfd, buf, strlen(buf));
        if (n < 0)
            error("ERROR writing to socket");

        close(connfd);
    }
}