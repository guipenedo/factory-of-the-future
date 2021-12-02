#ifndef FACTORY_OF_THE_FUTURE_DATABASE_H
#define FACTORY_OF_THE_FUTURE_DATABASE_H

#include "../network/tcp.h"
#include "../interfaces/peripherals.h"
#include "../utils/host_list.h"

#define MAX_SIZE 25
#define NUM_COMMANDS 2
#define MAX_FACTORIES MAX_CLIENTS - 2
#define MAX_MEASURES_STORED 2000

typedef double database_type [MAX_FACTORIES][MAX_MEASURES_STORED][4];
typedef int current_type[MAX_FACTORIES];

void receive_sensor_data(database_type database, current_type current, int factId, SensorData data, double thresholds[3], host_node * factory_list);
void showCurrent(database_type database, current_type current, int factoryId);
void setThreshold(int sensor, double value, double thresholds[3]);

#endif //FACTORY_OF_THE_FUTURE_DATABASE_H
