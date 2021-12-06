#include "sensor_history.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int init_sensor_data_buffer(SensorHistoryWriteBuffer ** empty_data_buff){
    SensorHistoryWriteBuffer * data_buff = NULL;
    data_buff = (SensorHistoryWriteBuffer *) malloc(sizeof(SensorHistoryWriteBuffer));
    if (data_buff == NULL)
        return 1;

    pthread_mutex_init(&(data_buff->data_mutex), NULL);

    data_buff->currIdx = 0;
    // initialize with 0
    memset(&(data_buff->data), 0, MAX_DATA_LEN * sizeof(SensorData));

    *empty_data_buff = data_buff;
    return 0;
}

void store_sensor_data(SensorHistoryWriteBuffer * data_buffer, SensorData sensorData){
    pthread_mutex_lock(&(data_buffer->data_mutex));

    data_buffer->data[data_buffer->currIdx++] = sensorData;

    if (MEASUREMENT_PERIOD * data_buffer->currIdx >= RECORD_TIME || MEASUREMENT_PERIOD * data_buffer->currIdx >= MAX_DATA_LEN)
        write_sensor_data_to_file(data_buffer, 0);

    pthread_mutex_unlock(&(data_buffer->data_mutex));
}

void write_sensor_data_to_file(SensorHistoryWriteBuffer * data_buffer, short lock){
    if (lock)
        pthread_mutex_lock(&(data_buffer->data_mutex));

    FILE *fp = fopen(SENSOR_HISTORY_FILENAME, "a");

    if (data_buffer->currIdx > 0)
        fwrite(data_buffer->data, sizeof(SensorData), data_buffer->currIdx, fp);

    fclose(fp);

    data_buffer->currIdx = 0;  // we can keep the old data, it will simply be overwritten as we go

    if (lock)
        pthread_mutex_unlock(&(data_buffer->data_mutex));
}

void free_sensor_data_buffer(SensorHistoryWriteBuffer ** data_buffer) {
    pthread_mutex_destroy(&((*data_buffer)->data_mutex));
    free(*data_buffer);
    *data_buffer = NULL;
}

FILE * open_sensor_history_file(int factId, const char * modes){
    char filename[50];
    sprintf(filename, "sensor_history_%d.dat", factId);
    return fopen(filename, modes);
}

int read_sensor_data_from_file(SensorData * read_buffer, int buffer_size, int factId){
    FILE * file = open_sensor_history_file(factId, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening sensor data for factID=%d\n", factId);
        return 0;
    }
    int i = 0;
    while(i < buffer_size && fread(read_buffer + i, sizeof(SensorData), 1, file)) i++;
    fclose(file);
    return i;
}

void send_sensor_history_file(SensorHistoryWriteBuffer * data_buffer, int connfd) {
    pthread_mutex_lock(&(data_buffer->data_mutex));

    //flush cached data
    write_sensor_data_to_file(data_buffer, 0);

    char data[SEND_DATA_SIZE] = {0};
    write(connfd, data, 1);

    FILE *fp = fopen(SENSOR_HISTORY_FILENAME, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening sensor data\n");
        data[0] = EOF;
        write(connfd, data, 1);
        pthread_mutex_unlock(&(data_buffer->data_mutex));
        return;
    }

    int n;
    while((n = fread(data, 1, SEND_DATA_SIZE, fp)) > 0) {
        if (write(connfd, data, n) == -1) {
            perror("[-] Error in sending file.");
        }
        bzero(data, SEND_DATA_SIZE);
    }
    data[0] = EOF;
    write(connfd, data, 1);

    fclose(fp);

    pthread_mutex_unlock(&(data_buffer->data_mutex));
}

void receive_sensor_history_file(ClientThreadData * data) {
    FILE * fp = open_sensor_history_file(data->host_id, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening sensor file for writing for factID=%d\n", data->host_id);
        return;
    }

    int n;
    char buffer[SEND_DATA_SIZE];

    do {
        n = read(data->sockfd, buffer, SEND_DATA_SIZE);

        if (buffer[0] == EOF)
            break;

        fwrite(buffer, 1, n, fp);
        bzero(buffer, SEND_DATA_SIZE);
    } while(n == SEND_DATA_SIZE);
    fclose(fp);
}