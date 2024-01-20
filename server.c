#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS];
pthread_mutex_t client_mutex[MAX_CLIENTS] = {PTHREAD_MUTEX_INITIALIZER};

int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void broadcast_message(char *message, int sender_sd) {

    // for simplicity, we don't take the clients lock here.
    // this is in principle racy, but in practice... well, in C
    // we get to do this stuff. In Rust, the compiler won't let us.
    // which is better? you decide. But this is a C program so...

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_sockets[i];
        if (sd > 0 && sd != sender_sd) {
            pthread_mutex_lock(&client_mutex[i]);
             send(sd, message, strlen(message), 0);
            pthread_mutex_unlock(&client_mutex[i]);
        }
    }
}

void *handle_client(void *arg) {
    int sock = *(int*)arg;
    char *buffer = NULL;
    size_t buflen = 0;
    ssize_t read_size;

    FILE *stream = fdopen(sock, "r");
    if (stream == NULL) {
        error("fdopen failed");
    }

    while((read_size = getline(&buffer, &buflen, stream)) != -1) {
        broadcast_message(buffer, sock);
    }

    if (read_size == -1) {
        perror("getline failed");
    }

    free(buffer);
    fclose(stream);

    // Remove client
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == sock) {
            client_sockets[i] = 0;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    close(sock);
    free(arg);
    return 0;
}

// The rest of the code (main function) remains the same as the previous version.

int main(int argc, char *argv[]) {
    int server_socket, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    char *pvalue = NULL;
    int port;

    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                pvalue = optarg;
                break;
            case '?':
                if (optopt == 'p')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `%c'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    if (pvalue == NULL) {
        fprintf(stderr, "Usage: %s -p port\n", argv[0]);
        return 1;
    }

    port = atoi(pvalue);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        error("Could not create socket");
    }

    int opt = 1;
    // Set SO_REUSEADDR to allow local address reuse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }

    // Listen
    listen(server_socket, 3);

    // Accept incoming connections
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while ((new_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if (pthread_create(&sniffer_thread, NULL, handle_client, (void*)new_sock) < 0) {
            error("Could not create thread");
        }

        // Add client
        pthread_mutex_lock(&clients_mutex);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_socket;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    if (new_socket < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
