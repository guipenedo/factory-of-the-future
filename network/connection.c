#include "../utils/host_list.h"
#include "../interfaces/network_commands.h"

char cmd_args[MAX_ARGS_BUFFER_SIZE];

void connect_to_dashboard(const char * dashboardAddr, host_node ** host_list, int * host_id, int factory) {
    // connect to the dashboard
    ClientThreadData * dashboardClient;
    connect_to_tcp_server(dashboardAddr, &dashboardClient);
    initialize_host_list(host_list);
    push_host(*host_list, 0, dashboardClient);

    char ip_address[20], response[20];
    get_ip_address(ip_address);
    // send init_new_factory command
    send_command_to_server(factory ? CMD_INIT_NEW_FACTORY : CMD_INIT_ML, ip_address, response, dashboardClient);

    // get our new host ID
    sscanf(response, "%d", host_id);
    printf("Successfully connected to the dashboard. We now have host ID=%d\n", *host_id);
}

ClientThreadData * connect_new_factory(char * args, host_node * host_list) {
    char ip_address[20];
    int host_id;
    sscanf(args, "%s %d", ip_address, &host_id);
    printf("Connecting to new factory ID=%d at %s\n.", host_id, ip_address);

    ClientThreadData * newFactoryClient;
    connect_to_tcp_server(ip_address, &newFactoryClient);
    push_host(host_list, host_id, newFactoryClient);
    return newFactoryClient;
}

void send_connect_back(int fact_ID, ClientThreadData * newFactoryClient) {
    // tell new host to also connect to our server
    char ip_address[20];
    get_ip_address(ip_address);
    sprintf(cmd_args, "%s %d", ip_address, fact_ID);
    // send CMD_CONNECT_BACK_HOST
    send_command_to_server(CMD_CONNECT_BACK_HOST, cmd_args, NULL, newFactoryClient);
}
