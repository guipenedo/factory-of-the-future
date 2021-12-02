#include "network/tcp.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"
#include "utils/sensor_history.h"
#include "network/connection.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "mlearn/data_files.h"
#include "mlearn/fit.h"
#include "mlearn/predict_temp.h"

host_node * host_list;
pthread_t server_thread;

int host_ID = -1;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void handle_command(int commandId, char * args, char * response, int connfd, char * client_ip) {
    if (commandId == CMD_ANNOUNCE_NEW_HOST) {
        ClientThreadData * factory_client = connect_new_factory(args, host_list);
        create_data_files_for_factory(factory_client->host_id);
    } else if (commandId == CMD_PREDICT) {
        int factId, hours, minutes, seconds;
        char hours_filepath[MAX_PATH_SIZE], temperatures_filepath[MAX_PATH_SIZE], beta_filepath[MAX_PATH_SIZE];


        sscanf(args, "%d %d %d %d", &factId, &hours, &minutes, &seconds);

        host_node * factory = get_host_by_id(host_list, factId);
        if (factory == NULL) {
            sprintf(response, "%f", 0.0);
            return;
        }
        ClientThreadData * factory_client = factory->host;
        // request history file
        send_command_to_server(CMD_SEND_SENSOR_HISTORY_FILE, NULL, NULL, factory_client);
        receive_sensor_history_file(factory_client);
        // append data
        // TODO: fix
        //append_factory_data(factId);
        // get filepaths
        get_data_file_path(factId, hours_filepath, "hours.csv");
        get_data_file_path(factId, temperatures_filepath, "temperatures.csv");
        get_data_file_path(factId, beta_filepath, "beta.csv");
        // fit
        fit(hours_filepath, temperatures_filepath, beta_filepath);
        // predict
        double result = predict_temp(beta_filepath, hours, minutes, seconds);

        sprintf(response, "%f", result);
    }
}

int main(int argc, char **argv) {
//    setbuf(stdout, NULL);
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

    while(1);

    close_all_connections(host_list);
    free_host_list(host_list);

    return 0;
}