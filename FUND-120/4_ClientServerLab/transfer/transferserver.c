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

    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    /* socket: create a socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        error("ERROR opening socket\n");
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
            error("ERROR on binding\n");
    }

    /* listen: make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 5) < 0) {  /* allow 5 requests to queue up */ 
        error("ERROR on listen\n");
    }
    
    /* wait for a connection request, transfer 
       file, then close connection. */
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);

    /* accept: wait for a connection request */
    int connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (connfd < 0) {
        error("ERROR on accept\n");
    } 

    /* gethostbyaddr: determine who sent the message */
    struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
        sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL) {
        error("ERROR on gethostbyaddr\n");
    }

    /* retrieves the IP address of client */
    char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL) {
        error("ERROR on inet_ntoa\n");
    }

    printf("Server established connection with %s (%s)\n", 
    hostp->h_name, hostaddrp);

    // Open the file in binary mode for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        error("ERROR opening file\n");
    }

    // Read the file and send its contents to the client
    int bytesRead = 0;
    char buf[BUFSIZE];
    while ((bytesRead = fread(buf, sizeof(char), BUFSIZE, file)) > 0) {
        if (write(connfd, buf, bytesRead) < 0) {
            error("ERROR writing to socket\n");
        }
    }

    close(connfd);
    return 0;
}