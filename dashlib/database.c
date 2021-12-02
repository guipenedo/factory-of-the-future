#include <stdio.h>
#include "database.h"
#include "factory_comm.h"

void receive_sensor_data(database_type database, current_type current, int factId, SensorData data, double thresholds[3], host_node * factory_list) {
    database[factId][current[factId]][0] = data.time;
    database[factId][current[factId]][1] = data.temperature;
    database[factId][current[factId]][2] = data.humidity;
    database[factId][current[factId]][3] = data.pressure;
    current[factId] = (current[factId] + 1) % MAX_MEASURES_STORED;
    if (data.temperature >= thresholds[0] || data.humidity >= thresholds[1] || data.pressure >= thresholds[2])
        trigger_alarm(factId, factory_list);
}

void showCurrent(database_type database, current_type current, int factoryId) {
    /* Displays for the factory ID given the measures done by the sensor at a time */
    int i;
    for (i = 0; i < 10; i++){
        int id = (current[factoryId] - 10 + i) % MAX_MEASURES_STORED;
        if (database[factoryId][id][0] != 0)
            printf("Factory %d has measured at time %lf temperature: %lf humidity: %lf pressure: %lf \n",
                   factoryId, database[factoryId][id][0],database[factoryId][id][1],
                   database[factoryId][id][2],database[factoryId][id][3]);
    }
}


void setThreshold(int sensor, double value, double thresholds[3]) {
    if (sensor < 1 || sensor > 3)
        printf("[ERROR] You have not set up a new threshold\n");
    else
        thresholds[sensor - 1] = value;
}