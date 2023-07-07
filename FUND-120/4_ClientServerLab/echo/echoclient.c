#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>

/* Be prepared accept a response of this length */
#define BUFSIZE 4096

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoclient [options]\n"                                                    \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -m                  Message to send to server (Default: \"Hello World!\"\n"\
"  -p                  Port (Default: 8080)\n"                                \
"  -s                  Server (Default: localhost)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"server",  required_argument, NULL, 's'},
        {"port",    required_argument, NULL, 'p'},
        {"message", required_argument, NULL, 'm'},
        {"help",    no_argument,       NULL, 'h'},
        {NULL, 0,                      NULL, 0}
};

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

/* Main ========================================================= */
int main(int argc, char **argv) {
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 8080;
    char *message = "Hello World!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 's': // server
                hostname = optarg;
                break;
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'm': // server
                message = optarg;
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

    if (NULL == message) {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /* socket: create the socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    } 

    /* gethostbyname: get the server's DNS entry */
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        error("ERROR connecting to server");
    }

    /* write: send the message line to the server */
    int bytesSent = write(sockfd, message, strlen(message));
    if (bytesSent < 0) {
        error("ERROR writing to socket");
    } 

    /* read: print the server's reply */
    char buf[BUFSIZE]; 
    int bytesRead = read(sockfd, buf, BUFSIZE);
    if (bytesRead < 0) {
        error("ERROR reading from socket");
    }
    printf("%.15s", buf);
    close(sockfd);
    return 0;

}