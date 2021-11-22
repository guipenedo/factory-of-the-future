#ifndef FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H
#define FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include "../interfaces/peripherals_network.h"

#define MAX_DATA_LEN                100
// The time that we wait before saving the data buffer in the .csv file, in sec
#define RECORD_TIME                 5*60
// Inter-measurment period in sec, please set it as a fraction of recording time
#define MEASUREMENT_PERIOD           5
#define SENSOR_HISTORY_FILENAME     "sensor_history.csv"

typedef struct sensor_history_buffer {
    time_t time [MAX_DATA_LEN];
    float temp [MAX_DATA_LEN];
    float humi [MAX_DATA_LEN];
    float pres [MAX_DATA_LEN];
    int currIdx;
    pthread_mutex_t data_mutex;
} sensor_history_buffer;

int init_sensor_data_buffer(sensor_history_buffer **);
void store_sensor_data(sensor_history_buffer *, SensorData);
void write_sensor_data_to_file(sensor_history_buffer *, short); //Write to a file without overwriting its contents
void free_sensor_data_buffer(sensor_history_buffer **);

#endif //FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H
