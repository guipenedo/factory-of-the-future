#include "tcp.h"

void handle_command(int commandId, char * response) {
    if (commandId == 0) {
        strcpy(response, "Command 0, sensor data");
    }
    else if (commandId == 1) {
        strcpy(response, "Command 1, actuator data");
    }
}

int main(void) {
    int o, cmd;
    char buffer[MAX_BUFFER_SIZE];

    printf("Choose:\n 1- Server\t2- Client\n");
    scanf("%d", &o);

    if(o == 1){
        // server
        accept_tcp_connections(handle_command);

        // non blocking version:
        /*
        pthread_t accept_thread;
        accept_tcp_connections_non_blocking(handle_command, &accept_thread);
        printf("NON BLOCKING! :D\n");
        while (1);
         */
    } else {
        // client
        ClientThreadData data;
        connect_to_tcp_server("127.0.0.1", &data);
        while (1) {
            printf("Command to send:");
            scanf("%d", &cmd);
            send_command_to_server(cmd, buffer, &data);

            // exit
            if(cmd == -1)
                break;

            printf("Server response: %s\n", buffer);
        }
        pthread_join(data.interact_server_thread, NULL);
    }

    return 0;
}