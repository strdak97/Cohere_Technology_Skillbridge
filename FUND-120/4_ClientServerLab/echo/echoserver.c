#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 4096

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -n                  Maximum pending connections\n"                         \
"  -p                  Port (Default: 8080)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"port",        required_argument, NULL, 'p'},
        {"maxnpending", required_argument, NULL, 'n'},
        {"help",        no_argument,       NULL, 'h'},
        {NULL, 0,                          NULL, 0}
};

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
    int option_char;
    int portno = 8080; /* port to listen on */
    int maxnpending = 5;

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "p:n:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'n': // server
                maxnpending = atoi(optarg);
                break;
            case 'h': // help
                fprintf(stdout, "%s", USAGE);
                exit(0);
                break;
            default:
                fprintf(stderr, "%s", USAGE);
                exit(1);
        }
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (maxnpending < 1) {
        fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
        exit(1);
    }

    /* socket: create a socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        error("ERROR opening socket");
    }

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
        (const void *)&optval , sizeof(int));

    /* build the server's internet address */
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; /* we are using the Internet */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
    serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

    /* bind: associate the listening socket with a port */
    if (bind(listenfd, (struct sockaddr *) &serveraddr, 
        sizeof(serveraddr)) < 0) {
            error("ERROR on binding");
    }

    /* listen: make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 5) < 0) {  /* allow 5 requests to queue up */ 
        error("ERROR on listen");
    }
    
    /* main loop: wait for a connection request, echo input line, 
    then close connection. */
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    while (1) {
        /* accept: wait for a connection request */
        int connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (connfd < 0) {
            error("ERROR on accept");
        } 

        /* gethostbyaddr: determine who sent the message */
        struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
            sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL) {
            error("ERROR on gethostbyaddr");
        }

        char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) {
            error("ERROR on inet_ntoa\n");
        }

        printf("Server established connection with %s (%s)\n", 
        hostp->h_name, hostaddrp);

        /* read: read input string from the client */
        char buf[BUFSIZE];
        int bytesRead = read(connfd, buf, BUFSIZE);
        if (bytesRead < 0) {
            error("ERROR reading from socket");
        }
        printf("Server received %d bytes: %s", bytesRead, buf);

        /* write: echo the input string back to the client */
        int bytesSent = write(connfd, buf, strlen(buf));
        if (bytesSent < 0) {
            error("ERROR writing to socket");
        }

        close(connfd);
    }
}