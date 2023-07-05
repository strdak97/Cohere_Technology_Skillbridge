#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h> 
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include "gfclient.h"

#define BUFSIZE 65536

struct gfcrequest_t {
    void *headerarg;
    void (*headerfunc)(void *, size_t, void *);
    char* path;
    unsigned short port;
    char* server;
    void *writearg;
    void (*writefunc)(void*, size_t, void *);
    size_t bytesreceived;
    size_t filelen;
    gfstatus_t status;
};

void gfc_cleanup(gfcrequest_t *gfr){
    free(gfr);
}

gfcrequest_t *gfc_create() {
    gfcrequest_t *gfr = (gfcrequest_t*)malloc(sizeof(gfcrequest_t));
    gfr->headerarg = NULL;
    gfr->headerfunc = NULL;
    gfr->path = NULL;
    gfr->port = 0;
    gfr->server = NULL;
    gfr->writearg = NULL;
    gfr->writefunc = NULL;
    gfr->bytesreceived = 0;
    gfr->filelen = 0;
    gfr->status = GF_INVALID;
    return gfr;
}

size_t gfc_get_bytesreceived(gfcrequest_t *gfr) {
    return gfr->bytesreceived;
}

size_t gfc_get_filelen(gfcrequest_t *gfr) {
    return gfr->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t *gfr) {
    return gfr->status;
}

void gfc_global_init() {

}

void gfc_global_cleanup() {

}

/*
 * Performs the transfer as described in the options.  Returns a value of 0
 * if the communication is successful, including the case where the server
 * returns a responseHeader with a FILE_NOT_FOUND or ERROR responseHeader.  If the 
 * communication is not successful (e.g. the connection is closed before
 * transfer is complete or an invalid header is returned), then a negative 
 * integer will be returned.
 */

int gfc_perform(gfcrequest_t *gfr) {
    //create client / server connection
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[+] Error creating socket [-] ");
    }
    

    struct hostent *server = gethostbyname(gfr->server);
    if (server == NULL) {
        printf("[+] Error resolving hostname [-] ");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(gfr->port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("[+] Error connecting to server [-] ");
    }

    //build and make request to server
    char request[BUFSIZE];

    sprintf(request, "GETFILE GET %s\r\n\r\n", gfr->path);
    printf("[+] Client REQUEST: %s [-] ", request);

    size_t bytesSent;
    bytesSent = write(sockfd, request, strlen(request));
    if(bytesSent < 0) {
        int errnum = errno;
        printf("[+] Error sending data: %s [-] ", strerror( errnum ));
    }

    //receive response from server
    char response[50];
    size_t totalBytesRead = 0;
    int headerEnd = 0;

    while (totalBytesRead < 49 && !headerEnd) {
        // Read one byte from the server
        ssize_t bytesRead = read(sockfd, response + totalBytesRead, 1);
        
        if (bytesRead < 0) {
            // Error occurred while reading
            perror("Error reading from socket");
            break;
        } else if (bytesRead == 0) {
            // Connection closed by the server
            printf("Connection closed by the server\n");
            break;
        }
        
        // Update the total bytes read
        totalBytesRead++;
        
        // Check if the header end delimiter (\r\n\r\n) is present in the response
        if (totalBytesRead >= 4 &&
            response[totalBytesRead - 4] == '\r' &&
            response[totalBytesRead - 3] == '\n' &&
            response[totalBytesRead - 2] == '\r' &&
            response[totalBytesRead - 1] == '\n') {
            headerEnd = 1;
        }
    }

    response[totalBytesRead] = '\0'; // Null-terminate the response string

    printf("[+] Server RESPONSE: %s ", response);
    char *filePtr = NULL; 

    if(strstr(response, "ERROR") != NULL) {
        gfr->status = GF_ERROR;
    }
    else if(strstr(response, "FILE_NOT_FOUND") != NULL) {
        gfr->status = GF_FILE_NOT_FOUND;
    }
    else if(strstr(response, "GETFILE") != NULL && strstr(response + 8, "OK") != NULL) {
        filePtr = response + 11;
        gfr->filelen = atoi(filePtr);
        gfr->status = GF_OK;
    }
    else {
        gfr->status = GF_INVALID;
        return -1;
    }

    // Receive response from server
    char responseData[gfr->filelen];

    // Read until the expected file length is reached
    while (gfr->bytesreceived < gfr->filelen) {
        size_t bytesRead = read(sockfd, responseData, BUFSIZE);
        if (bytesRead < 0) {
            printf("Error reading bytes\n");
        } else if (bytesRead == 0) {
            printf("Connection error\n");
            gfr->status = GF_ERROR;
            return -1;
        }

        // Call the writefunc with the received data
        gfr->writefunc(responseData, bytesRead, gfr->writearg);

        gfr->bytesreceived += bytesRead;
    }

    close(sockfd);
    return 0;
}

void gfc_set_headerarg(gfcrequest_t *gfr, void *headerarg){
    gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t *gfr, void (*headerfunc)(void*, size_t, void *)){
    gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t *gfr, char* path){
    gfr->path = path;
}

void gfc_set_port(gfcrequest_t *gfr, unsigned short port){
    gfr->port = port;
}

void gfc_set_server(gfcrequest_t *gfr, char* server){
    gfr->server = server;
}

void gfc_set_writearg(gfcrequest_t *gfr, void *writearg){
    gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t *gfr, void (*writefunc)(void*, size_t, void *)){
    gfr->writefunc = writefunc;
}

char* gfc_strstatus(gfstatus_t status) {
    switch (status) {
        case GF_OK:
            return "OK";
        case GF_FILE_NOT_FOUND:
            return "FILE_NOT_FOUND";
        case GF_ERROR:
            return "ERROR";
        case GF_INVALID:
            return "INVALID";
        default:
            return "UNKNOWN";
    }
}
