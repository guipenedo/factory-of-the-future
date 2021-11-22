#include "sensor_history.h"

int init_sensor_data_buffer(sensor_history_buffer ** empty_data_buff){
    sensor_history_buffer * data_buff = NULL;
    data_buff = (sensor_history_buffer *) malloc(sizeof(sensor_history_buffer));
    if (data_buff == NULL)
        return 1;

    pthread_mutex_init(&(data_buff->data_mutex), NULL);

    data_buff->currIdx = 0;
    int i = 0;
    while(i < MAX_DATA_LEN){
        data_buff->time[i] = 0;
        data_buff->temp[i] = 0;
        data_buff->humi[i] = 0;
        data_buff->pres[i] = 0;
        i++;
    }

    *empty_data_buff = data_buff;
    return 0;
}


void store_sensor_data(sensor_history_buffer * data_buffer, SensorData sensorData){
    time_t measurement_time;
    time(&measurement_time); //Get the time in sec since UTC 1/1/70.

    pthread_mutex_lock(&(data_buffer->data_mutex));

    data_buffer->time[data_buffer->currIdx] = measurement_time;
    data_buffer->temp[data_buffer->currIdx] = sensorData.temperature;
    data_buffer->humi[data_buffer->currIdx] = sensorData.humidity;
    data_buffer->pres[data_buffer->currIdx] = sensorData.pressure;
    data_buffer->currIdx++;

    if (MEASUREMENT_PERIOD * data_buffer->currIdx >= RECORD_TIME)
        write_sensor_data_to_file(data_buffer, 0);

    pthread_mutex_unlock(&(data_buffer->data_mutex));
}


void write_sensor_data_to_file(sensor_history_buffer * data_buffer, short lock){
    if (lock)
        pthread_mutex_lock(&(data_buffer->data_mutex));

    FILE *fp = fopen(SENSOR_HISTORY_FILENAME, "a");

    int i = 0;
    while (i < data_buffer->currIdx){
        fprintf(fp, "%2.2ld;%2.2f;%2.2f;%2.2f\n", data_buffer->time[i], data_buffer->temp[i], data_buffer->humi[i], data_buffer->pres[i]);
        i++;
    }

    fclose(fp);

    data_buffer->currIdx = 0;  // we can keep the old data, it will simply be overwritten as we go

    if (lock)
        pthread_mutex_unlock(&(data_buffer->data_mutex));
}


void free_sensor_data_buffer(sensor_history_buffer ** data_buffer) {
    pthread_mutex_destroy(&((*data_buffer)->data_mutex));
    free(*data_buffer);
    *data_buffer = NULL;
}