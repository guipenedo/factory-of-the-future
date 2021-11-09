#include "network/tcp.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"
#include "network/connection.h"

host_node * host_list;
pthread_t server_thread;

int host_ID = -1;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void handle_command(int commandId, char * args, char * response, char * client_ip) {
    if (commandId == CMD_ANNOUNCE_NEW_HOST) {
        connect_new_factory(args, host_list);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        double temperature, humidity, pressure;
        sscanf(args, "%d %lf %lf %lf", &factId, &temperature, &humidity, &pressure);
        // TODO
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ML Module takes exactly 1 command line argument: the dashboard's IP address\n");
        exit(-1);
    }

    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // connect to the dashboard
    const char * dashboardAddr = argv[1];
    printf("Starting ML module! Dashboard IP address: %s\n", dashboardAddr);

    connect_to_dashboard(dashboardAddr, &host_list, &host_ID, 0);

    while(1) {

    }

    close_all_connections(host_list);
    free_host_list(host_list);

    return 0;
}