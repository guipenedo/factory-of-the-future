#ifndef FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H
#define FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H

#include <pthread.h>
#include "../interfaces/peripherals.h"
#include "../network/tcp.h"

#define MAX_DATA_LEN                100
// The time that we wait before saving the data buffer in the .csv file, in sec
#define RECORD_TIME                 (5*60)
// Inter-measurment period in sec, please set it as a fraction of recording time
#define MEASUREMENT_PERIOD           5
#define SENSOR_HISTORY_FILENAME     "sensor_history.dat"
#define SEND_DATA_SIZE 1024

typedef struct SensorHistoryWriteBuffer {
    SensorData data[MAX_DATA_LEN];
    int currIdx;
    pthread_mutex_t data_mutex;
} SensorHistoryWriteBuffer;

int init_sensor_data_buffer(SensorHistoryWriteBuffer **);
void store_sensor_data(SensorHistoryWriteBuffer *, SensorData);
void write_sensor_data_to_file(SensorHistoryWriteBuffer *, short); //Write to a file without overwriting its contents
void free_sensor_data_buffer(SensorHistoryWriteBuffer **);
// returns number of lines read
int read_sensor_data_from_file(SensorData *, int, int);
void send_sensor_history_file(SensorHistoryWriteBuffer *, int);
void receive_sensor_history_file(ClientThreadData * data);

#endif //FACTORY_OF_THE_FUTURE_SENSOR_HISTORY_H
