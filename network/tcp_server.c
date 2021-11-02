#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 8

#define MAX 80

#define PORT 8080
#define SA struct sockaddr

#define GET_SENSOR_LIST 0
#define GET_ACTUATOR_LIST 1

typedef struct ClientThreadArg {
    void (* func) (int, char *);
    int connfd;
} ClientThreadArg;

void * serve_client(void * p_data){
    ClientThreadArg * thread_arg = (ClientThreadArg *) p_data;
    int connfd = thread_arg->connfd;
    void (* func) (int, char *) = thread_arg->func;
    free(thread_arg);

    char buff[MAX], res_buff[MAX];
    // infinite loop for chat
    while (1) {
        bzero(buff, MAX);
        bzero(res_buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));

        // actually handle whatever we have to do
        int commandId;
        int conv = sscanf(buff, "%d", &commandId);
        if (conv > 0)
            func(commandId, res_buff);

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }

        // print buffer which contains the client contents
        printf("From client: %s\n To client : %s\n", buff, res_buff);

        // and send that buffer to client
        write(connfd, res_buff, sizeof(res_buff));
    }

    close(connfd);
}

int accept_tcp_connections(void (* func) (int, char *)) {
    pthread_t client_t[MAX_CLIENTS];

    int sockfd, client_i = 0;
    socklen_t len;
    struct sockaddr_in servaddr;
    struct sockaddr_storage serverStorage;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
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
        printf("socket bind failed...\n");
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
        ClientThreadArg * args = malloc (sizeof (ClientThreadArg));
        args->func = func;
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

void test_commands (int commandId, char * response){
    if (commandId == GET_SENSOR_LIST){
        printf("SENSOR LIST IS HERE\n");
        strncpy(response, "SENSOR LIST IS HERE", 20);
    } else if (commandId == GET_ACTUATOR_LIST){
        printf("ACTUATOR LIST IS HERE\n");
        strncpy(response, "ACTUATOR LIST IS HERE", 22);
    }
}

// Driver function 
int main() {
    accept_tcp_connections(test_commands);
} 

