#ifndef FACTORY_OF_THE_FUTURE_DATA_FILES_H
#define FACTORY_OF_THE_FUTURE_DATA_FILES_H

#define MAX_PATH_SIZE 50
#define HOURS_TRAIN_FILEPATH "./data/hours.csv"
#define TEMPERATURES_TRAIN_FILEPATH "./data/temperatures.csv"

void create_data_files_for_factory(int);
int os_copy_file(const char*, const char*);
void append_factory_data(int);
void get_data_file_path(int, char[], const char *);

#endif //FACTORY_OF_THE_FUTURE_DATA_FILES_H
