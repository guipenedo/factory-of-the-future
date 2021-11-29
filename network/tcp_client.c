#include "tcp.h"
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void send_command_to_server(int commandId, char * arguments, char * response, ClientThreadData * data) {
    // lock mutex
    pthread_mutex_lock(&(data->command_mutex));

    // set commandId
    if (arguments == NULL)
        arguments = "";
    sprintf(data->command, "%03d %s", commandId, arguments);

    // signal that there is a new command to handle
    data->command_to_handle = 1;
    pthread_cond_signal(&(data->command_condition));

    if (commandId == -1) {
        pthread_mutex_unlock(&(data->command_mutex));
        return;
    }

    // wait for response data write signal
    if(!data->response_to_handle)
        pthread_cond_wait(&(data->command_condition), &(data->command_mutex));
    data->response_to_handle = 0;

    // save response data
    if (response != NULL)
        strcpy(response, data->response);

    if (DEBUG_NETWORK_COMMS)
        printf("<- To Server: %s | From Server : \"%s\"\n", data->command, data->response);

    // unlock mutex
    pthread_mutex_unlock(&(data->command_mutex));
}


void * interact_with_server (void * p_data) {
    ClientThreadData * data = (ClientThreadData *) p_data;

    int commandId;

    while (1) {
        // lock the mutex
        pthread_mutex_lock(&(data->command_mutex));

        // wait for a command
        if (!data->command_to_handle)
            pthread_cond_wait(&(data->command_condition), &(data->command_mutex));
        data->command_to_handle = 0;

        sscanf(data->command, "%d", &commandId);

        write(data->sockfd, data->command, sizeof(data->command));

        // if msg contains "Exit" then server exit and chat ended.
        if (commandId == -1) {
            printf("Client Exit...\n");
            pthread_mutex_unlock(&(data->command_mutex));
            break;
        }

        read(data->sockfd, data->response, sizeof(data->response));

        // signal that the response data is written
        data->response_to_handle = 1;
        pthread_cond_signal(&(data->command_condition));

        pthread_mutex_unlock(&(data->command_mutex));
    }
    close(data->sockfd);
    return NULL;
}


void connect_to_tcp_server(const char * server_addr, ClientThreadData ** pointer_data) {
    ClientThreadData * data = (ClientThreadData *) malloc(sizeof(ClientThreadData));
    struct sockaddr_in servaddr;
    pthread_cond_init(&(data->command_condition), NULL);
    pthread_mutex_init(&(data->command_mutex), NULL);
    data->command_to_handle = 0;
    data->response_to_handle = 0;
    strcpy(data->ip_address, server_addr);

    // socket create and varification
    data->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (data->sockfd == -1) {
        printf("socket creation failed...\n");
        free(data);
        exit(0);
    }
    else if (DEBUG_NETWORK_COMMS)
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(data->sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed with error %m...\n");
        free(data);
        exit(0);
    }
    else if (DEBUG_NETWORK_COMMS)
        printf("connected to the server..\n");

    pthread_create(&(data->interact_server_thread), NULL, interact_with_server, data);
    *pointer_data = data;
}

