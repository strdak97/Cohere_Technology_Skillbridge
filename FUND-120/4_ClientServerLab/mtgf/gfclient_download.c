#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "steque.h"
#include "workload.h"
#include "gfclient.h"

#define DEFAULT_SERVER_ADDR "localhost"
#define DEFAULT_SERVER_PORT 8080
#define DEFAULT_WORKLOAD_PATH "workload.txt"
#define DEFAULT_NUM_REQUESTS 1
#define DEFAULT_NUM_THREADS 1

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server",        required_argument, NULL, 's'},
    {"port",          required_argument, NULL, 'p'},
    {"workload-path", required_argument, NULL, 'w'},
    {"nthreads",      required_argument, NULL, 't'},
    {"nrequests",     required_argument, NULL, 'n'},
    {"help",          no_argument,       NULL, 'h'},
    {NULL,            0,                 NULL,  0 }
};

/* struct that represents the configuration for 
   performing concurrent file download requests */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond_worker;
    steque_t queue;
    char* server;  
    unsigned short port;
    int num_requests;
} WebClient;

/* struct that represents each argument passed 
   to each worker thread in the program */
typedef struct {
    WebClient* client;
    int id;
} ThreadArgs;

typedef void (*WriteCallbackFunc)(void*, size_t, void*);

void usage() {
    fprintf(stdout,
        "Usage:\n"
        "  webclient [options]\n"
        "Options:\n"
        "  -h                     Show this help message\n"
        "  -n [num_requests]      Requests download per thread (Default: 1)\n"
        "  -p [server_port]       Server port (Default: 8080)\n"
        "  -s [server_addr]       Server address (Default: localhost)\n"
        "  -t [num_threads]       Number of threads (Default: 1)\n"
        "  -w [workload_path]     Path to workload file (Default: workload.txt)\n"
    );
}

/* generates a local path for storing downloaded 
   files based on the provided request path */
void localPath(const char* req_path, char* local_path) {
    static int counter = 0;
    snprintf(local_path, 512, "%s-%06d", &req_path[1], counter++);
}

/* opens a file for writing and creates any necessary directories 
   in the file path if they do not already exist */
FILE* openFile(const char* path) {
    char* cur;
    char* prev;
    FILE* file;
    
    prev = (char*)path;
    while (NULL != (cur = strchr(prev + 1, '/'))) {
        *cur = '\0';
        if (0 > mkdir(path, S_IRWXU) && errno != EEXIST) {
            perror("Unable to create directory");
            exit(EXIT_FAILURE);
        }
        *cur = '/';
        prev = cur;
    }

    if (NULL == (file = fopen(path, "w"))) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    return file;
}

/* handles the writing of received data to 
   a file during the file download process */
void writeCallback(void* data, size_t data_len, void* arg) {
    FILE* file = (FILE*)arg;
    fwrite(data, 1, data_len, file);
}

/* encapsulates the creation and configuration 
   of a gfcrequest object for a specific request */
gfcrequest_t* createRequest(WebClient* client, const char* req_path, FILE* file, unsigned short port, WriteCallbackFunc writeCallback) {
    gfcrequest_t* gfr = gfc_create();
    gfc_set_server(gfr, client->server);
    gfc_set_path(gfr, (char*)req_path); 
    gfc_set_port(gfr, port);
    gfc_set_writefunc(gfr, writeCallback);
    gfc_set_writearg(gfr, file);
    return gfr;
}

/* handles the processing of download requests in each worker thread 
   and is executed concurrently by multiple worker threads */
void* requestHandler(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    WebClient* client = thread_args->client;
    gfcrequest_t* gfr = NULL;
    FILE* file = NULL;
    int returncode = 0;
    char local_path[512];
    
    for (int i = 0; i < client->num_requests; i++) {
        pthread_mutex_lock(&client->mutex);
        
        /* syncs the worker threads when 
           the queue of requests is empty */
        while (steque_isempty(&client->queue)) {
            pthread_cond_wait(&client->cond_worker, &client->mutex);
        }
        
        /* retrieves a request path from the request 
           queue to be processed by the thread */
        char* req_path = steque_pop(&client->queue);

        pthread_mutex_unlock(&client->mutex);
        
        localPath(req_path, local_path);
        file = openFile(local_path);

        /* creates and configures a gfcrequest object for a specific request */
        gfr = createRequest(client, req_path, file, client->port, writeCallback);

        fprintf(stdout, "Requesting %s%s\n", client->server, req_path);

        if (0 > (returncode = gfc_perform(gfr))) {
            fprintf(stdout, "gfc_perform returned an error %d\n", returncode);
            fclose(file);
            if (0 > unlink(local_path))
                fprintf(stderr, "unlink failed on %s\n", local_path);
        } else {
            fclose(file);
        }

        if (gfc_get_status(gfr) != GF_OK) {
            if (0 > unlink(local_path))
                fprintf(stderr, "unlink failed on %s\n", local_path);
        }

        fprintf(stdout, "Status: %s\n", gfc_strstatus(gfc_get_status(gfr)));
        fprintf(stdout, "Received %zu of %zu bytes\n", gfc_get_bytesreceived(gfr), gfc_get_filelen(gfr));

        gfc_cleanup(gfr);
    }

    return NULL;
}

int main(int argc, char** argv) {
    WebClient client = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond_worker = PTHREAD_COND_INITIALIZER,
        .server = strdup(DEFAULT_SERVER_ADDR), 
        .port = DEFAULT_SERVER_PORT,
        .num_requests = DEFAULT_NUM_REQUESTS
    };

    char* workload_path = DEFAULT_WORKLOAD_PATH;
    int num_threads = DEFAULT_NUM_THREADS;

    int option_char = 0;
    while ((option_char = getopt_long(argc, argv, "s:p:w:n:t:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 's':
                client.server = optarg;
                break;
            case 'p':
                client.port = atoi(optarg);
                break;
            case 'w':
                workload_path = optarg;
                break;
            case 'n':
                client.num_requests = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (EXIT_SUCCESS != workload_init(workload_path)) {
        fprintf(stderr, "Unable to load workload file %s.\n", workload_path);
        exit(EXIT_FAILURE);
    }

    gfc_global_init();

    /* prepares a FIFO queue */
    steque_init(&client.queue);

    /* dynamically allocates memory for pthread and threadArg structures */
    pthread_t* workers = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs* thread_args = malloc(num_threads * sizeof(ThreadArgs));

    /* creates multiple threads to perform 
       the processing of requests concurrently */
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].client = &client;
        thread_args[i].id = i;
        pthread_create(&workers[i], NULL, requestHandler, &thread_args[i]);
    }

    for (int i = 0; i < client.num_requests * num_threads; i++) {
        char* req_path = workload_get_path();
        if (strlen(req_path) > 256) {
            fprintf(stderr, "Request path exceeded maximum of 256 characters.\n");
            exit(EXIT_FAILURE);
        }
        /* adds a request path to the end of a 
           shared queue in a thread safe manner */
        pthread_mutex_lock(&client.mutex);
        steque_enqueue(&client.queue, req_path);
        pthread_cond_signal(&client.cond_worker);
        pthread_mutex_unlock(&client.mutex);
    }

    /* waits for the completion of all threads 
       in the program before preceeding further */
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(workers[i], NULL) < 0) {
            printf("Joining thread failed %d\n", i);
        }
    }

    /* cleans up and deallocates resources 
       used by the program before exiting */
    gfc_global_cleanup();
    steque_destroy(&client.queue);
    free(client.server);
    free(workers);
    free(thread_args);

    return 0;
} 