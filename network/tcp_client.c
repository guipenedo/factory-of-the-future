// RSX101 CLIENT TCP
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>


#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX 80
#define PORT 8080
#define SA struct sockaddr

typedef struct ServerData {
    pthread_cond_t has_command;
    pthread_mutex_t command_mutex;
    int commandId;
    int sockfd;
    char response[MAX];
} ServerData;


void send_command_to_server(ServerData * data, int commandId, char * response) {
    pthread_mutex_lock(&(data->command_mutex));

    data->commandId = commandId;

    pthread_cond_signal(&(data->has_command));

    strcpy(response, data->response);

    pthread_mutex_unlock(&(data->command_mutex));
}


void * interact_with_server (void * p_data) {
    ServerData * data = (ServerData *) p_data;

    char buff[MAX];

    while (1) {
        pthread_mutex_lock(&(data->command_mutex));
        pthread_cond_wait(&(data->has_command), &(data->command_mutex));

        // if msg contains "Exit" then server exit and chat ended.
        if (data->commandId == - 1) {
            printf("Server Exit...\n");
            break;
        }

        bzero(buff, MAX);
        sprintf(buff, "%d", data->commandId);

        write(data->sockfd, buff, sizeof(buff));

        read(data->sockfd, data->response, sizeof(data->response));

        pthread_mutex_unlock(&(data->command_mutex));
    }
}


void connect_to_tcp_server(const char * server_addr, ServerData * data) {
    struct sockaddr_in servaddr;
    pthread_t interact_server_thread;
    pthread_cond_init(&(data->has_command), NULL);
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

    pthread_create(&interact_server_thread, NULL, interact_with_server, data);

    return data;
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.37");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
} 

