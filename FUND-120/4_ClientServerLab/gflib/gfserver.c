#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gfserver.h"

#define BUFSIZE 4096

//struct used to store information regarding the server
struct gfserver_t {
    unsigned short port;
    int maxConnections;
    ssize_t (*handler)(gfcontext_t *, char *, void*);
    void* handlerArg;
};

//struct used to store information regarding the context of the client
struct gfcontext_t {
    int clientSocket;
};

//create and initialize a gfserver_t structure and return a pointer to the newly created structure.
gfserver_t* gfserver_create() {
    gfserver_t *gfs = (gfserver_t*)malloc(sizeof(gfserver_t));
    gfs->port = 0;
    gfs->maxConnections = 0;
    gfs->handler = NULL;
    gfs->handlerArg = NULL;
    return gfs;
}

//terminates connection to a client and frees memory related to client context
void gfs_abort(gfcontext_t *clientContext) {
    free(clientContext);
}

ssize_t totalBytesSentSend = 0;
//sends data from the server to the client over the client socket connection.
ssize_t gfs_send(gfcontext_t *clientContext, void *data, size_t len) {
    ssize_t bytesSent;
    bytesSent = write(clientContext->clientSocket, data, len);
        if (bytesSent < 0) {
            int errnum = errno;
            printf("[+] Error sending data: %s [-] ", strerror( errnum ));
        }
    totalBytesSentSend += bytesSent;
    return bytesSent;
}

ssize_t totalBytesSentHeader = 0;
// Sends the header to the client containing the GETFILE protocol
ssize_t gfs_sendheader(gfcontext_t *clientContext, gfstatus_t status, size_t file_len) {
    char header[BUFSIZE];
    ssize_t bytesSent = 0;
    char *statusStr;
    int header_len;
    if (status == 200) {
        statusStr = "OK";
        header_len = sprintf(header, "GETFILE %s %zu\r\n\r\n", statusStr, file_len);
    }
    else if (status == 400) {
        statusStr = "FILE_NOT_FOUND";
        header_len = sprintf(header, "GETFILE %s\r\n\r\n", statusStr);
    }
    else {
        statusStr = "ERROR";
        header_len = sprintf(header, "GETFILE %s\r\n\r\n", statusStr);
    }

    if (header_len >= BUFSIZE || header_len < 0) {
        printf("[+] Error sending header [-] ");
    }
    printf("[+] Server RESPONSE: %s [-]", header);

    bytesSent = write(clientContext->clientSocket, header, header_len);
    totalBytesSentHeader += bytesSent;

    if (bytesSent < 0) {
        int errnum = errno;
        printf("[+] Error sending header: %s (errno: %d) [-] ", strerror(errnum), errnum);
    }

    return bytesSent;
}

char* checkHeader(char* request) {
    char* filePtr = NULL;
    if (strstr(request, "GETFILE") != NULL &&
        strstr(request + 7, "GET") != NULL &&
        strstr(request, "\r\n\r\n") != NULL &&
        strstr(request + 11, " /") != NULL) { 
        filePtr = strstr(request, "\r\n\r\n");
        *filePtr = '\0';
        // Extract the file path
        filePtr = strstr(request, " /");
        if (filePtr != NULL) {
            filePtr += 1; 
        }
    }
    else {
        filePtr = "filenotfound";
    }

    return filePtr;
}

void gfserver_serve(gfserver_t *gfs) {
    /* Create the listening socket */
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        printf("[+] Error creating socket [-] ");
    }

    /* Build the server's internet address */
    struct sockaddr_in serverAddr;
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET; /* We are using the Internet */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept requests to any IP address */
    serverAddr.sin_port = htons(gfs->port); /* Port to listen on */

    /* Bind the listening socket */
    if (bind(listenSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        printf("[+] Error binding socket [-] ");
        close(listenSocket);
    }

    /* Allow the socket to listen for connection requests */
    if (listen(listenSocket, gfs->maxConnections) < 0) {
        printf("[+] Error listening [-] ");
        close(listenSocket);
    }

    while (1) {
        /* Accept a new client connection */
        struct sockaddr_in clientAddr;
        socklen_t clientLength = sizeof(clientAddr);
        int commSocket = accept(listenSocket, (struct sockaddr *) &clientAddr, &clientLength);
        if (commSocket < 0) {
            printf("[+] Error accepting socket [-] ");
            close(listenSocket);
            close(commSocket);
            free(gfs);
            break;
        }

        /* Create an instance of the client context */
        gfcontext_t *clientContext = (gfcontext_t *) malloc(sizeof(gfcontext_t));
        clientContext->clientSocket = commSocket;

        /* Read and process the client's request */
        char request[MAX_REQUEST_LEN];
        ssize_t bytesRead = read(clientContext->clientSocket, request, sizeof(request) - 1);
        if (bytesRead < 0) {
            printf("[+] Error receiving client request [-] ");
            close(listenSocket);
            gfs_abort(clientContext);
            free(gfs);
            break;
        }
        /* Null-terminate the received data */
        request[bytesRead] = '\0';

        printf("[+] Client's request: %s [-]\n", request);

        char* filePtr = checkHeader(request);

        printf("[+] Filepath: %s [-] ", filePtr);
        gfs->handler(clientContext, filePtr, gfs->handlerArg);

        printf("[+] Total bytes header sent: %ld [-] ", totalBytesSentHeader);
        printf("[+] Total bytes send sent: %ld [-] ", totalBytesSentSend);

        /* Cleanup the client context and close the connection */
        gfs_abort(clientContext);
    }

    /* Cleanup the listening socket */
    close(listenSocket);
}

void gfserver_set_handlerarg(gfserver_t *gfs, void* arg) {
    gfs->handlerArg = arg;
}

void gfserver_set_handler(gfserver_t *gfs, ssize_t (*handler)(gfcontext_t *, char *, void*)) {
    gfs->handler = handler;
}

void gfserver_set_maxpending(gfserver_t *gfs, int max_npending) {
    gfs->maxConnections = max_npending;
}

void gfserver_set_port(gfserver_t *gfs, unsigned short port) {
    gfs->port = port;
}