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

#define BUFSIZE 4096

/* represents a request object 
   for the client library */
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

/* deallocates memory after client is 
   finished with a request object */
void gfc_cleanup(gfcrequest_t *gfr){
    free(gfr);
}

/* creates and initializes a new request object */
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

/* getters */
size_t gfc_get_bytesreceived(gfcrequest_t *gfr) {
    return gfr->bytesreceived;
}

size_t gfc_get_filelen(gfcrequest_t *gfr) {
    return gfr->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t *gfr) {
    return gfr->status;
}

/* sets up any global data structured 
   needed for the library */
void gfc_global_init() {

}

/* cleans up any global data structures 
   needed for the library */
void gfc_global_cleanup() {

}

/* performs the actual GETFILE request by establishing a 
   connection to the server, sending the request, 
   receiving the response, and handling the received data 
   according to the provided callbacks. */
int gfc_perform(gfcrequest_t *gfr) {
    /* socket: create the socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("ERROR creating socket\n");
    }
    
    /* gethostbyname: get the server's DNS entry */
    struct hostent *server = gethostbyname(gfr->server);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", gfr->server);
        exit(0);
    }

    /* build the server's Internet address */
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(gfr->port);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        printf("ERROR connecting to server\n");
    }

    //build GETFILE request
    char request[BUFSIZE];
    sprintf(request, "GETFILE GET %s\r\n\r\n", gfr->path);
    printf("Client REQUEST: %s \n", request);

    //send GETFILE request
    ssize_t bytesSent = write(sockfd, request, strlen(request));
    if(bytesSent < 0) {
        int errnum = errno;
        printf("Error sending data: %s \n", strerror( errnum ));
    }

    //receive GETFILE response from server
    char response[50];
    ssize_t totalBytesRead = 0;
    ssize_t bytesRead = 0;
    int headerEnd = 0;

    //parse server response
    while (totalBytesRead < 49 && !headerEnd) {
        bytesRead = read(sockfd, response + totalBytesRead, 1);
        
        if (bytesRead < 0) {
            printf("Error reading from socket\n");
            break;
        } 
        else if (bytesRead == 0) {
            printf("Connection closed by the server\n");
            break;
        }
        
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

    response[totalBytesRead] = '\0'; 

    printf("Server RESPONSE: %s \n", response);

    //take parsed response and set status appropriately 
    if(strstr(response, "ERROR") != NULL) {
        gfr->status = GF_ERROR;
    }
    else if(strstr(response, "FILE_NOT_FOUND") != NULL) {
        gfr->status = GF_FILE_NOT_FOUND;
    }
    else if(strstr(response, "GETFILE") != NULL && strstr(response + 8, "OK") != NULL) {
        char *filePtr = response + 11;
        gfr->filelen = atoi(filePtr);
        gfr->status = GF_OK;
    }
    else {
        gfr->status = GF_INVALID;
        return -1;
    }

    //if server response is OK, continue receving data from server
    char responseData[gfr->filelen];
    bytesRead = 0;

    while (gfr->bytesreceived < gfr->filelen) {
        bytesRead = read(sockfd, responseData, BUFSIZE);

        if (bytesRead < 0) {
            printf("Error reading bytes\n");
        } 
        else if (bytesRead == 0) {
            printf("Connection error\n");
            gfr->status = GF_ERROR;
            return -1;
        }

        gfr->writefunc(responseData, bytesRead, gfr->writearg);

        gfr->bytesreceived += bytesRead;
    }

    close(sockfd);
    return 0;
}

/* setters */
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

/* sets status as needed */
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
