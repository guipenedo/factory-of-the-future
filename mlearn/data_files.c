#include "data_files.h"
#include <stdio.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

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
    sprintf(dirname, "./factories/factory%d", factoryId);
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