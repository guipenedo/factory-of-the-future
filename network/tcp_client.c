#include "tcp.h"

void send_command_to_server(int commandId, char * response, ClientThreadData * data) {
    // lock mutex
    pthread_mutex_lock(&(data->command_mutex));

    // set commandId
    data->commandId = commandId;

    // signal that there is a new command to handle
    pthread_cond_signal(&(data->command_condition));

    if (data->commandId == -1) {
        pthread_mutex_unlock(&(data->command_mutex));
        return;
    }

    // wait for response data write signal
    pthread_cond_wait(&(data->command_condition), &(data->command_mutex));

    // save response data
    strcpy(response, data->response);

    // unlock mutex
    pthread_mutex_unlock(&(data->command_mutex));
}


void * interact_with_server (void * p_data) {
    ClientThreadData * data = (ClientThreadData *) p_data;

    char buff[MAX_BUFFER_SIZE];

    while (1) {
        // lock the mutex
        pthread_mutex_lock(&(data->command_mutex));

        // wait for a command
        pthread_cond_wait(&(data->command_condition), &(data->command_mutex));

        bzero(buff, MAX_BUFFER_SIZE);
        sprintf(buff, "%d", data->commandId);

        write(data->sockfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (data->commandId == -1) {
            printf("Client Exit...\n");
            pthread_mutex_unlock(&(data->command_mutex));
            break;
        }

        read(data->sockfd, data->response, sizeof(data->response));

        // signal that the response data is written
        pthread_cond_signal(&(data->command_condition));

        pthread_mutex_unlock(&(data->command_mutex));
    }
    close(data->sockfd);
    return NULL;
}


void connect_to_tcp_server(const char * server_addr, ClientThreadData * data) {
    struct sockaddr_in servaddr;
    pthread_cond_init(&(data->command_condition), NULL);
    pthread_mutex_init(&(data->command_mutex), NULL);

    // socket create and varification
    data->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (data->sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(data->sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    pthread_create(&(data->interact_server_thread), NULL, interact_with_server, data);
}

