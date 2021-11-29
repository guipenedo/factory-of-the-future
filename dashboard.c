#include "network/tcp.h"
#include "network/connection.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"
#include "interfaces/peripherals.h"
#include "utils/sensor_data_utils.h"
#include <stdio.h>
#include <stdlib.h>

host_node * factory_list;
ClientThreadData * ml_client = NULL;
pthread_t server_thread;

int number_of_factories = 0;
pthread_mutex_t fact_id_mutex;
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
void receive_sensor_data(int factId, SensorData data) {
    // TODO
    int alarm = 0;
    if (alarm) { // TODO
        trigger_alarm(factId);
    }
}

void handle_command(int commandId, char * args, char * response, int connfd, char * client_ip) {
    if (commandId == CMD_INIT_NEW_FACTORY || commandId == CMD_INIT_ML) {
        pthread_mutex_lock(&fact_id_mutex);
        int factory_id = ++number_of_factories;
        dashboard_init_new_host(factory_id, client_ip, factory_list, &ml_client, commandId == CMD_INIT_NEW_FACTORY);
        sprintf(response, "%d", factory_id);
        pthread_mutex_unlock(&fact_id_mutex);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        SensorData data;
        sensor_data_from_command(args, &factId, &data);

        receive_sensor_data(factId, data);
    }
}

int main(int argc, char **argv) {
//    setbuf(stdout, NULL);
    if (argc != 1) {
        printf("Dashboard does not take any arguments.\n");
        exit(-1);
    }

    ml_client = NULL;
    initialize_host_list(&factory_list);
    pthread_mutex_init(&fact_id_mutex, NULL);
    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // TODO: dashboard TEAM actually do something with the dashboard
    while (1);

    close_all_connections(factory_list);
    free_host_list(factory_list);
    pthread_mutex_destroy(&fact_id_mutex);
    return 0;
}