#ifndef FACTORY_OF_THE_FUTURE_TCP_IP_H
#define FACTORY_OF_THE_FUTURE_TCP_IP_H

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define DEBUG_NETWORK_COMMS 1

#define MAX_CLIENTS 8
#define MAX_BUFFER_SIZE 80
#define MAX_ARGS_BUFFER_SIZE MAX_BUFFER_SIZE - 4

#define PORT 8080
#define SA struct sockaddr

typedef struct ServerThreadData {
    void (* command_handler) (int, char *, char *);
    int connfd;
} ServerThreadData;

typedef struct ClientThreadData {
    pthread_cond_t command_condition;
    pthread_mutex_t command_mutex;
    int sockfd;
    char command[MAX_BUFFER_SIZE];
    char response[MAX_BUFFER_SIZE];
    pthread_t interact_server_thread;
} ClientThreadData;

void * serve_client(void *);
void * accept_tcp_connections(void (*) (int, char *, char *));
void accept_tcp_connections_non_blocking(void (*) (int, char *, char *), pthread_t *);
void send_command_to_server(int, char *, char *, ClientThreadData *);
void * interact_with_server (void *);
void connect_to_tcp_server(const char *, ClientThreadData **);

void get_ip_address(char *);

#endif //FACTORY_OF_THE_FUTURE_TCP_IP_H
