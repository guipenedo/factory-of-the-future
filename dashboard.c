#include "network/tcp_ip.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"

host_node * factory_list;
pthread_t server_thread;

int number_of_factories = 0;

void handle_command(int commandId, char * args, char * response) {
    if (commandId == CMD_INIT_NEW_FACTORY) {
        int factory_id = ++number_of_factories;
        char ip_address[20];
        sscanf(args, "%s", ip_address);
        printf("New factory connected with ID=%d from %s\n.", factory_id, ip_address);

        char args[MAX_ARGS_BUFFER_SIZE];
        sprintf(args, "%s %d", ip_address, factory_id);

        // broadcast this new factory to every other factory
        host_node * factory = factory_list;
        while (factory->next != NULL) {
            factory = factory->next;
            send_command_to_server(CMD_ANNOUNCE_NEW_FACTORY, args, NULL, factory->host);
        }

        // save this new factory to our list of factories
        ClientThreadData * newFactoryClient;
        connect_to_tcp_server(ip_address, &newFactoryClient);
        push_host(factory_list, factory_id, newFactoryClient);
    }
}

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("Dashboard does not take any arguments.\n");
        exit(-1);
    }

    initialize_host_list(&factory_list);
    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // TODO: actually do something with the dashboard
    while (1);

    close_all_connections(factory_list);
    free_host_list(factory_list);
    return 0;
}