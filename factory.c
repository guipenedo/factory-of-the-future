#include "network/tcp.h"
#include "interfaces/network_commands.h"
#include "interfaces/peripherals_network.h"
#include "utils/host_list.h"
#include "network/connection.h"

host_node * host_list;
pthread_t server_thread;

int fact_ID = -1;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void trigger_alarm(int fact_id_alarm) {
    // TODO
}

void handle_command(int commandId, char * args, char * response, char * client_ip) {
    if (commandId == CMD_ANNOUNCE_NEW_HOST) {
        connect_new_factory(args, host_list);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        double temperature, humidity, pressure;
        sscanf(args, "%d %lf %lf %lf", &factId, &temperature, &humidity, &pressure);
        printf("Sensor data from fact_ID=%d: T=%lf H=%lf P=%lf\n", factId, temperature, humidity, pressure);
    } else if (commandId == CMD_TRIGGER_ALARM) {
        int fact_id_alarm;
        sscanf(args, "%d", &fact_id_alarm);
        printf("[!] Alarm triggered because of factory ID %d\n.", fact_id_alarm);
        trigger_alarm(fact_id_alarm);
    }
}

void broadcast_sensor_data(SensorData sensorData) {
    sprintf(cmd_args, "%d %lf %lf %lf", fact_ID, sensorData.temperature, sensorData.humidity, sensorData.pressure);
    host_node * host = host_list;
    while (host->next != NULL){
        host = host->next;
        send_command_to_server(CMD_SEND_SENSOR_DATA, cmd_args, NULL, host->host);
    }
}

// TODO: replace with production version
void read_sensor_data(SensorData * data) {
    data->pressure = 1.34;
    data->temperature = 20.4;
    data->humidity = 99;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Factory takes exactly 1 command line argument: the dashboard's IP address\n");
        exit(-1);
    }

    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // connect to the dashboard
    const char * dashboardAddr = argv[1];
    printf("Starting factory! Dashboard IP address: %s\n", dashboardAddr);

    connect_to_dashboard(dashboardAddr, &host_list, &fact_ID, 1);

    while(1) {
        SensorData sensorData;
        read_sensor_data(&sensorData);
        broadcast_sensor_data(sensorData);
        sleep(5);
    }

    close_all_connections(host_list);
    free_host_list(host_list);

    return 0;
}