#include "tcp.h"

void * serve_client(void * p_data){
    ServerThreadData * thread_arg = (ServerThreadData *) p_data;
    int connfd = thread_arg->connfd;
    void (* command_handler) (int,  char *, char *) = thread_arg->command_handler;
    free(thread_arg);

    char buff[MAX_BUFFER_SIZE], res_buff[MAX_BUFFER_SIZE];
    while (1) {
        bzero(buff, MAX_BUFFER_SIZE);
        bzero(res_buff, MAX_BUFFER_SIZE);

        // read the message from client and copy it in buffer
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

        command_handler(commandId, buff + 3, res_buff);

        // print buffer which contains the client contents
        printf("From client: %s\nTo client : \"%s\"\n", buff, res_buff);

        // and send that buffer to client
        write(connfd, res_buff, sizeof(res_buff));
    }

    close(connfd);
    return NULL;
}

void accept_tcp_connections_non_blocking(void (* func) (int, char *, char *), pthread_t * accept_thread) {
    if(pthread_create(accept_thread, NULL, accept_tcp_connections, func) != 0)
        printf("Failed to create server accept thread\n");
}

void * accept_tcp_connections(void (* func) (int, char *, char *)) {
    pthread_t client_t[MAX_CLIENTS];

    int sockfd, client_i = 0;
    socklen_t len;
    struct sockaddr_in servaddr;
    struct sockaddr_storage serverStorage;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // SO_REUSEADDR allows to use same port after a quick restart
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        printf("socket creation failed...\n");
        exit(0);
    } else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET; // TCP
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed... error code: %d\n", errno);
        exit(0);
    } else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else
        printf("Server listening..\n");

    while (1) {
        ServerThreadData * args = malloc (sizeof (ServerThreadData));
        args->command_handler = func;
        //Accept call creates a new socket for the incoming connection
        len = sizeof serverStorage;
        args->connfd = accept(sockfd, (SA *) &serverStorage, &len);
        if (args->connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        } else
            printf("server acccept the client...\n");

        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        if(pthread_create(&client_t[client_i++], NULL, serve_client, args) != 0) {
            printf("Failed to create thread\n");
            free(args);
        }
    }
}
