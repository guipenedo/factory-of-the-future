#include <stdio.h>
#include <stdlib.h>
#include "factory_data.h"
#include "../interfaces/network_commands.h"
#include "../interfaces/peripherals.h"
#include "../utils/sensor_history.h"

void downloadhistory (host_node * factory_list, double flags[3]) {

    host_node * factory = get_host_by_id(factory_list, flags[0]);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    ClientThreadData * client = factory->host;
    send_command_to_server(CMD_SEND_SENSOR_HISTORY_FILE, NULL, NULL, client);
    receive_sensor_history_file(client);

    printf("File downloaded successfully!\n");

}

void showhistory (host_node * factory_list, double flags[3]) {

    host_node * factory = get_host_by_id(factory_list, flags[0]);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }

    if (flags[1] <= 0 || flags[1] > 200) {
        printf("\n[ERROR] Lines must be between 1 and 200.\n");
        return;
    }

    SensorData * data = malloc(flags[1] * sizeof(SensorData));
    int lines = read_sensor_data_from_file(data, flags[1], flags[0]);
    printf("Read %d data lines\n", lines);
    for (int i = 0; i < lines; ++i) {
        printf("Ts=%ld T=%lf H=%lf P=%lf\n", data[i].time, data[i].temperature, data[i].humidity, data[i].pressure);
    }
    free(data);

}