#include "network/tcp_ip.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"

host_node * factory_list;
ClientThreadData * ml_client;
pthread_t server_thread;

int number_of_factories = 0;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void trigger_alarm(int factId) {
    sprintf(cmd_args, "%d", factId);

    host_node * factory = factory_list;
    while (factory->next != NULL) {
        factory = factory->next;
        send_command_to_server(CMD_TRIGGER_ALARM, cmd_args, NULL, factory->host);
    }
}

// TODO dashboard team
void receive_sensor_data(int factId, double temperature, double humidity, double pressure) {
    // TODO
    int alarm = 0;
    if (alarm) { // TODO
        trigger_alarm(factId);
    }
}

void handle_command(int commandId, char * args, char * response) {
    if (commandId == CMD_INIT_NEW_FACTORY || commandId == CMD_INIT_ML) {
        int factory_id = ++number_of_factories;
        char ip_address[20];
        sscanf(args, "%s", ip_address);
        if (commandId == CMD_INIT_NEW_FACTORY)
            printf("New factory connected with ID=%d from %s\n.", factory_id, ip_address);
        else
            printf("ML module connected with ID=%d from %s\n.", factory_id, ip_address);

        sprintf(cmd_args, "%s %d", ip_address, factory_id);

        // broadcast this new host to every other host
        host_node *factory = factory_list;
        while (factory->next != NULL) {
            factory = factory->next;
            send_command_to_server(CMD_ANNOUNCE_NEW_HOST, cmd_args, NULL, factory->host);
        }

        // save this new factory to our list of factories
        // or save this as ml module
        ClientThreadData *newFactoryClient;
        connect_to_tcp_server(ip_address, &newFactoryClient);
        if (commandId == CMD_INIT_NEW_FACTORY)
            push_host(factory_list, factory_id, newFactoryClient);
        else
            ml_client = newFactoryClient;
        sprintf(response, "%d", factory_id);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        double temperature, humidity, pressure;
        sscanf(args, "%d %lf %lf %lf", &factId, &temperature, &humidity, &pressure);

        receive_sensor_data(factId, temperature, humidity, pressure);
    }
}

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("Dashboard does not take any arguments.\n");
        exit(-1);
    }

    ml_client = NULL;
    initialize_host_list(&factory_list);
    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // TODO: dashboard TEAM actually do something with the dashboard
    while (1);

    close_all_connections(factory_list);
    free_host_list(factory_list);
    return 0;
}