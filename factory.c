#include "network/tcp.h"
#include "interfaces/network_commands.h"
#include "interfaces/peripherals.h"
#include "utils/host_list.h"
#include "network/connection.h"
#include "utils/sensor_history.h"
#include "utils/sensor_data_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

host_node * host_list;
pthread_t server_thread;
SensorHistoryWriteBuffer * sensor_history;

int fact_ID = -1;
char cmd_args[MAX_ARGS_BUFFER_SIZE];
short alarm_state = 0;

void handle_command(int commandId, char * args, char * response, int connfd, char * client_ip) {
    if (commandId == CMD_ANNOUNCE_NEW_HOST) {
        connect_new_factory(args, host_list);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        SensorData data;
        sensor_data_from_command(args, &factId, &data);
        print_sensor_data(data, factId);
    } else if (commandId == CMD_TRIGGER_ALARM) {
        int fact_id_alarm;
        sscanf(args, "%d", &fact_id_alarm);
        printf("[!] Alarm triggered because of factory ID %d\n.", fact_id_alarm);
        trigger_factory_alarm(fact_id_alarm, &alarm_state);
    } else if (commandId == CMD_GET_PERIPHERALS) {
        sprintf(response, "%d %d %d", has_sensors(), has_led(), has_relay());
    } else if (commandId == CMD_SET_LED_STATE) {
        short state;
        sscanf(args, "%hd", &state);
        set_led_state(state);
    } else if (commandId == CMD_SET_RELAY_STATE) {
        short state;
        sscanf(args, "%hd", &state);
        set_relay_state(state);
    } else if (commandId == CMD_SEND_SENSOR_HISTORY_FILE) {
        printf("Client requested sensor history file. Sending...\n");
        send_sensor_history_file(sensor_history, connfd);
        response[0] = -1;
    } else if (commandId == CMD_GET_ALARM_STATE) {
        printf("Client requested alarm state.\n");
        sprintf(response, "%hd", alarm_state);
    }
}

void broadcast_sensor_data(SensorData sensorData) {
    sprintf(cmd_args, "%d %ld %lf %lf %lf", fact_ID, sensorData.time, sensorData.temperature, sensorData.humidity, sensorData.pressure);
    host_node * host = host_list;
    while (host->next != NULL){
        host = host->next;
        send_command_to_server(CMD_SEND_SENSOR_DATA, cmd_args, NULL, host->host);
    }
}

int main(int argc, char **argv) {
//    setbuf(stdout, NULL);
    if (argc != 2) {
        printf("Factory takes exactly 1 command line argument: the dashboard's IP address\n");
        exit(-1);
    }

    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // connect to the dashboard
    const char * dashboardAddr = argv[1];
    printf("Starting factory! Dashboard IP address: %s\n", dashboardAddr);

    init_sensor_data_buffer(&sensor_history);
    connect_to_dashboard(dashboardAddr, &host_list, &fact_ID, 1);
    init_sensor();

    short sensors_enabled = has_sensors();

    sleep(MEASUREMENT_PERIOD);
    while(1) {
        if (sensors_enabled && !alarm_state) {
            SensorData sensorData;
            read_sensor_data(&sensorData);
            store_sensor_data(sensor_history, sensorData);
            broadcast_sensor_data(sensorData);
        }
        sleep(MEASUREMENT_PERIOD);
    }

    write_sensor_data_to_file(sensor_history, 1);
    close_all_connections(host_list);
    free_host_list(host_list);
    free_sensor_data_buffer(&sensor_history);

    return 0;
}