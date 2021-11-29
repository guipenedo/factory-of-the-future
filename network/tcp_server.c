#include "tcp.h"
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

void * serve_client(void * p_data){
    ServerThreadData * thread_arg = (ServerThreadData *) p_data;
    int connfd = thread_arg->connfd;
    command_handler_f command_handler = thread_arg->command_handler;
    char client_ip[MAX_BUFFER_SIZE];
    strcpy(client_ip, thread_arg->client_ip);
    free(thread_arg);

    char buff[MAX_BUFFER_SIZE], res_buff[MAX_BUFFER_SIZE];
    while (1) {
        bzero(buff, MAX_BUFFER_SIZE);
        bzero(res_buff, MAX_BUFFER_SIZE);

        // read the message from client and copy it to the buffer
        read(connfd, buff, sizeof(buff));

        // actually handle whatever we have to do
        int commandId;
        int conv = sscanf(buff, "%d", &commandId);
        if (conv <= 0)
            continue;

        if (commandId == -1){
            printf("Client sent exit. Closing this client's thread...\n");
            break;
        }

        command_handler(commandId, buff + 3, res_buff, connfd, client_ip);

        // print buffer which contains the client contents
        if (DEBUG_NETWORK_COMMS)
            printf("-> From client: %s | To client : \"%s\"\n", buff, res_buff);

        // and send that buffer to client
        if (res_buff[0] != -1)
            write(connfd, res_buff, sizeof(res_buff));
    }

    close(connfd);
    return NULL;
}

void accept_tcp_connections_non_blocking(command_handler_f func, pthread_t * accept_thread) {
    if(pthread_create(accept_thread, NULL, (void * (*)(void *)) accept_tcp_connections, func) != 0)
        printf("Failed to create server accept thread\n");
}

void * accept_tcp_connections(command_handler_f func) {
    pthread_t client_t[MAX_CLIENTS];

    int sockfd, client_i = 0;
    socklen_t len;
    struct sockaddr_in servaddr, client;
    len = sizeof(struct sockaddr_in);

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // SO_REUSEADDR allows to use same port after a quick restart
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        printf("socket creation failed...\n");
        exit(0);
    } else if(DEBUG_NETWORK_COMMS)
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET; // TCP
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed... error code: %d\n", errno);
        exit(0);
    } else if (DEBUG_NETWORK_COMMS)
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else if (DEBUG_NETWORK_COMMS)
        printf("Server listening..\n");

    while (1) {
        ServerThreadData * args = malloc (sizeof (ServerThreadData));
        args->command_handler = func;
        //Accept call creates a new socket for the incoming connection
        args->connfd = accept(sockfd, (SA *) &client, &len);
        if (args->connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        } else if (DEBUG_NETWORK_COMMS)
            printf("server acccept the client...\n");

        // get the new client's IP address
        char * ip = inet_ntoa(client.sin_addr);
        strcpy(args->client_ip, ip);
        if (DEBUG_NETWORK_COMMS)
            printf("Client IP address: %s\n", ip);

        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        if(pthread_create(&client_t[client_i++], NULL, serve_client, args) != 0) {
            printf("Failed to create thread\n");
            free(args);
        }
    }
}
