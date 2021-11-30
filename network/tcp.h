#ifndef FACTORY_OF_THE_FUTURE_TCP_IP_H
#define FACTORY_OF_THE_FUTURE_TCP_IP_H

#include <pthread.h>

#define DEBUG_NETWORK_COMMS 0

#define MAX_CLIENTS 8
#define MAX_BUFFER_SIZE 80
#define MAX_ARGS_BUFFER_SIZE (MAX_BUFFER_SIZE - 4)
#define IP_ADDRESS_SIZE 20

#define PORT 8080
#define SA struct sockaddr

typedef void (* command_handler_f) (int, char *, char *, int, char *);

typedef struct ServerThreadData {
    command_handler_f command_handler;
    int connfd;
    char client_ip[IP_ADDRESS_SIZE];
} ServerThreadData;

typedef struct ClientThreadData {
    pthread_cond_t command_condition;
    pthread_mutex_t command_mutex;
    int sockfd;
    int host_id;
    char ip_address[IP_ADDRESS_SIZE];
    char command[MAX_BUFFER_SIZE];
    char response[MAX_BUFFER_SIZE];
    short command_to_handle, response_to_handle;
    pthread_t interact_server_thread;
} ClientThreadData;

void * serve_client(void *);
void * accept_tcp_connections(command_handler_f);
void accept_tcp_connections_non_blocking(command_handler_f, pthread_t *);
void send_command_to_server(int, char *, char *, ClientThreadData *);
void * interact_with_server (void *);
void connect_to_tcp_server(const char *, ClientThreadData **);

#endif //FACTORY_OF_THE_FUTURE_TCP_IP_H
