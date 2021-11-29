#include <stdio.h>
#include "../interfaces/peripherals.h"

void sensor_data_from_command(char * cmd_args, int * fact_id, SensorData * data) {
    sscanf(cmd_args, "%d %ld %lf %lf %lf", fact_id, &(data->time), &(data->temperature), &(data->humidity), &(data->pressure));
}

void print_sensor_data(SensorData data, int fact_id) {
    printf("Sensor data from fact_ID=%d: ts=%ld T=%lf H=%lf P=%lf\n", fact_id, data.time, data.temperature, data.humidity, data.pressure);
}
