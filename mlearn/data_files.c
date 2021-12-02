#include "data_files.h"
#include "../interfaces/peripherals.h"
#include "../utils/sensor_history.h"
#include <stdio.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <malloc.h>

// taken from https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
int os_copy_file(const char* source, const char* destination) {
    int input, output;
    if ((input = open(source, O_RDONLY)) == -1)
    {
        return -1;
    }
    if ((output = creat(destination, 0660)) == -1)
    {
        close(input);
        return -1;
    }

    //Here we use kernel-space copying for performance reasons
    //sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+
    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    fstat(input, &fileinfo);
    int result = sendfile(output, input, &bytesCopied, fileinfo.st_size);

    close(input);
    close(output);

    return result;
}

void get_data_file_path(int factoryId, char path[], const char * filename) {
    char dirname[MAX_PATH_SIZE];
    sprintf(dirname, "./factory%d", factoryId);
    // ensure directory exists
    mkdir(dirname, 0777);
    sprintf(path, "%s/%s", dirname, filename);
}

void create_data_files_for_factory(int factoryId){
    // copy X_train and Y_train
    char data_file_path[MAX_PATH_SIZE];
    get_data_file_path(factoryId, data_file_path, "hours.csv");
    os_copy_file(HOURS_TRAIN_FILEPATH, data_file_path);
    get_data_file_path(factoryId, data_file_path, "temperatures.csv");
    os_copy_file(TEMPERATURES_TRAIN_FILEPATH, data_file_path);
}

void append_factory_data(int factoryId) {
    // filepaths
    char hours_filepath[MAX_PATH_SIZE], temperatures_filepath[MAX_PATH_SIZE];
    get_data_file_path(factoryId, hours_filepath, "hours.csv");
    get_data_file_path(factoryId, temperatures_filepath, "temperatures.csv");
    // open csv
    FILE * hours_file = fopen(hours_filepath, "a");
    FILE * temperatures_file = fopen(temperatures_filepath, "a");
    // read from downloaded file
    SensorData * data = malloc(1000 * sizeof(SensorData));
    int lines = read_sensor_data_from_file(data, 1000, factoryId);
    // append to datafiles
    for (int i = 0; i < lines; ++i) {
        struct tm tm = *localtime(&(data[i].time));
        double hour = tm.tm_hour + (tm.tm_min / 60.0) + (tm.tm_sec / 3600.0);
        fprintf(hours_file, "%lf", hour);
        fprintf(temperatures_file, "%lf", data[i].temperature);
    }
    free(data);
    fclose(hours_file);
    fclose(temperatures_file);
}