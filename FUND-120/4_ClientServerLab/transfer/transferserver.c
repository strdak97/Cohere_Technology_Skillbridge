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
#include <fcntl.h>

#if 0
#endif

#define BUFSIZE 4096

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  transferserver [options]\n"                                                \
"options:\n"                                                                  \
"  -f                  Filename (Default: bar.txt)\n"                         \
"  -h                  Show this help message\n"                              \
"  -p                  Port (Default: 8080)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"filename", required_argument, NULL, 'f'},
        {"port",     required_argument, NULL, 'p'},
        {"help",     no_argument,       NULL, 'h'},
        {NULL, 0,                       NULL, 0}
};

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
    int listenfd; /* listening socket */
    int connfd; /* connection socket */
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp; /* client host info */
    char buf[BUFSIZE]; /* message buffer */
    char *hostaddrp; /* dotted decimal host addr string */
    int optval; /* flag value for setsockopt */
    /* provided variables below */
    int option_char;
    int portno = 8080; /* port to listen on */
    char *filename = "bar.txt"; /* file to transfer */

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "f:p:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'f': // listen-port
                filename = optarg;
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

    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */

    /* socket: create a socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) 
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
            (const void *)&optval , sizeof(int));

    /* build the server's internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; /* we are using the Internet */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
    serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

    /* bind: associate the listening socket with a port */
    if (bind(listenfd, (struct sockaddr *) &serveraddr, 
        sizeof(serveraddr)) < 0) 
        error("ERROR on binding");

    /* listen: make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 5) < 0) /* allow 5 requests to queue up */ 
        error("ERROR on listen");

    clientlen = sizeof(clientaddr);

    /* accept: wait for a connection request */
    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (connfd < 0) 
    error("ERROR on accept");
    
    /* gethostbyaddr: determine who sent the message */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
            sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", 
    hostp->h_name, hostaddrp);

  /* test code */

    // Open the file in binary mode for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        error("ERROR opening file");
    }

    // Read the file and send its contents to the client
    int bytes_read;
    while ((bytes_read = fread(buf, sizeof(char), BUFSIZE, file)) > 0) {
        if (write(connfd, buf, bytes_read) < 0) {
            error("ERROR writing to socket");
        }
    }

    close(connfd);
    return 0;
}