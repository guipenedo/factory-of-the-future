#include "../utils/host_list.h"
#include "../interfaces/network_commands.h"

void connect_to_dashboard(const char * dashboardAddr, host_node ** host_list, int * host_id, int factory) {
    // connect to the dashboard
    ClientThreadData * dashboardClient;
    connect_to_tcp_server(dashboardAddr, &dashboardClient);
    initialize_host_list(host_list);
    push_host(*host_list, 0, dashboardClient);

    printf("saved dashboard client\n");
    fflush(stdout);

    char response[20];
    printf("sending command to init\n");
    fflush(stdout);
    // send init_new_factory command
    send_command_to_server(factory ? CMD_INIT_NEW_FACTORY : CMD_INIT_ML, NULL, response, dashboardClient);

    printf("command sent!!! command to init\n");
    fflush(stdout);
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

void dashboard_init_new_host(int factory_id, const char * client_ip, host_node * factory_list, ClientThreadData ** ml_client, int factory) {
    if (factory)
        printf("New factory connected with ID=%d from %s\n.", factory_id, client_ip);
    else
        printf("ML module connected with ID=%d from %s\n.", factory_id, client_ip);

    char cmd_args[MAX_ARGS_BUFFER_SIZE];
    sprintf(cmd_args, "%s %d", client_ip, factory_id);

    // save this new factory to our list of factories
    // or save this as ml module
    ClientThreadData *newFactoryClient;
    connect_to_tcp_server(client_ip, &newFactoryClient);
    if (factory)
        push_host(factory_list, factory_id, newFactoryClient);
    else
        *ml_client = newFactoryClient;

    // broadcast this new host to every other host and send their IP and ID to the new host
    host_node *factory_node = factory_list;
    char cmd_args_2[MAX_ARGS_BUFFER_SIZE];
    while (factory_node->next != NULL) {
        factory_node = factory_node->next;
        send_command_to_server(CMD_ANNOUNCE_NEW_HOST, cmd_args, NULL, factory_node->host);
        // send to new host as well
        sprintf(cmd_args_2, "%s %d", factory_node->host->ip_address, factory_node->host_id);
        send_command_to_server(CMD_ANNOUNCE_NEW_HOST, cmd_args_2, NULL, newFactoryClient);
    }
}