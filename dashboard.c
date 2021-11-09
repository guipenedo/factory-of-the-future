#include "network/tcp.h"
#include "network/connection.h"
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

void handle_command(int commandId, char * args, char * response, char * client_ip) {
    if (commandId == CMD_INIT_NEW_FACTORY || commandId == CMD_INIT_ML) {
        int factory_id = ++number_of_factories;
        dashboard_init_new_host(factory_id, client_ip, factory_list, &ml_client, commandId == CMD_INIT_NEW_FACTORY);
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