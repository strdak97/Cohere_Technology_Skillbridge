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

#include "gfclient.h"
#include "workload.h"

#define BUFFER_SIZE 4096

struct gfcrequest_t {
    char* server;
    unsigned short port;
    char* path;
    void (*headerfunc)(void*, size_t, void*);
    void* headerarg;
    void (*writefunc)(void*, size_t, void*);
    void* writearg;
    gfstatus_t status;
    size_t filelen;
    size_t bytesreceived;
};

void gfc_cleanup(gfcrequest_t *gfr) {
    if (gfr) {
        free(gfr->server);
        free(gfr->path);
        free(gfr);
    }
}

gfcrequest_t *gfc_create() {
    gfcrequest_t *gfr = (gfcrequest_t *)malloc(sizeof(gfcrequest_t));
    if (gfr) {
        gfr->server = NULL;
        gfr->port = 0;
        gfr->path = NULL;
        gfr->headerfunc = NULL;
        gfr->headerarg = NULL;
        gfr->writefunc = NULL;
        gfr->writearg = NULL;
        gfr->status = GF_INVALID;
        gfr->filelen = 0;
        gfr->bytesreceived = 0;
    }
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
    // Perform any global initialization here if required
}

void gfc_global_cleanup() {
    // Perform any global cleanup here if required
}

int gfc_perform(gfcrequest_t *gfr) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    int unlink_error = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Error opening socket");
        return -1;
    }

    server = gethostbyname(gfr->server);
    if (server == NULL) {
        printf("Error: No such host\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(gfr->port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error connecting");
        return -1;
    }

    // Prepare request headers
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GETFILE GET %s\r\n\r\n", gfr->path);

    // Send request
    ssize_t sent = send(sockfd, request, BUFFER_SIZE, 0);
    if (sent < 0) {
        printf("Error sending request");
        close(sockfd);
        return -1;
    }

    // Receive response headers
    char response[BUFFER_SIZE];
    ssize_t received = recv(sockfd, response, BUFFER_SIZE, 0);
    if (received < 0) {
        printf("Error receiving response");
        close(sockfd);
        return -1;
    }

    // Null-terminate the received data
    response[received] = '\0';

    // Parse response headers
    char *status_code = strtok(response, " ");
    if (status_code == NULL) {
        printf("Error: Invalid response format\n");
        close(sockfd);
        return -1;
    }

    if (strcmp(status_code, "OK") == 0) {
        gfr->status = GF_OK;

        char *file_length = strtok(NULL, " ");
        if (file_length != NULL)
            gfr->filelen = atoi(file_length);
    } else if (strcmp(status_code, "FILE_NOT_FOUND") == 0) {
        gfr->status = GF_FILE_NOT_FOUND;
    } else if (strcmp(status_code, "ERROR") == 0) {
        gfr->status = GF_ERROR;
    } else {
        gfr->status = GF_INVALID;
    }

    // Call the header callback function if set
    if (gfr->headerfunc != NULL)
        gfr->headerfunc(response, received, gfr->headerarg);

    // Receive response body and call the write callback function
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        if (gfr->writefunc != NULL)
            gfr->writefunc(buffer, bytes_read, gfr->writearg);

        gfr->bytesreceived += bytes_read;
    }

    gfstatus_t status = gfc_get_status(gfr);
    if (status == GF_OK || status == GF_FILE_NOT_FOUND || status == GF_ERROR) {
        close(sockfd);

        if (status == GF_ERROR) {
            if (unlink(gfr->path) == -1) {
                fprintf(stderr, "unlink failed on %s\n", gfr->path);
                unlink_error = 1;
            }
        }

        if (unlink_error) {
            return -1;
        } else {
            return 0; // Successful communication, including ERROR response
        }
    } else {
        close(sockfd);
        return -1; // Communication not successful
    }
}

void gfc_set_headerarg(gfcrequest_t *gfr, void *headerarg) {
    gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t *gfr, void (*headerfunc)(void *, size_t, void *)) {
    gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t *gfr, char *path) {
    gfr->path = strdup(path);
}

void gfc_set_port(gfcrequest_t *gfr, unsigned short port) {
    gfr->port = port;
}

void gfc_set_server(gfcrequest_t *gfr, char *server) {
    gfr->server = strdup(server);
}

void gfc_set_writearg(gfcrequest_t *gfr, void *writearg) {
    gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t *gfr, void (*writefunc)(void *, size_t, void *)) {
    gfr->writefunc = writefunc;
}

char *gfc_strstatus(gfstatus_t status) {
    switch (status) {
        case GF_OK:
            return "OK";
        case GF_FILE_NOT_FOUND:
            return "FILE_NOT_FOUND";
        case GF_ERROR:
            return "ERROR";
        case GF_INVALID:
        default:
            return "INVALID";
    }
}