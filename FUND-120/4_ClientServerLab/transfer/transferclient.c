#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFSIZE 4096

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  transferclient [options]\n"                                                \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -o                  Output file (Default foo.txt)\n"                       \
"  -p                  Port (Default: 8080)\n"                                \
"  -s                  Server (Default: localhost)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"server", required_argument, NULL, 's'},
        {"port",   required_argument, NULL, 'p'},
        {"output", required_argument, NULL, 'o'},
        {"help",   no_argument,       NULL, 'h'},
        {NULL, 0,                     NULL, 0}
};

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

/* Main ========================================================= */
int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buf[BUFSIZE];
    /* */
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 8080;
    char *filename = "foo.txt";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 's': // server
                hostname = optarg;
                break;
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'o': // message
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

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    /* socket Code Here */

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, 
    (char *)&serveraddr.sin_addr.s_addr, server->h_length);

    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
        error("ERROR connecting");

    /* test code */

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        error("ERROR opening file for writing");
    }

    // Receive data from the server and write it to the file
    int bytes_received;
    while ((bytes_received = read(sockfd, buf, BUFSIZE)) > 0) {
        if (fwrite(buf, sizeof(char), bytes_received, file) < bytes_received) {
            error("ERROR writing to file");
        }
    }

    // Close the file
    fclose(file);

    close(sockfd);
    return 0;
}