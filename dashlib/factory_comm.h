#ifndef FACTORY_OF_THE_FUTURE_FACTORY_COMM_H
#define FACTORY_OF_THE_FUTURE_FACTORY_COMM_H

#include "../utils/host_list.h"
#include "database.h"

void trigger_alarm(int factId, host_node * factory_list);
void sendcommand (host_node * factory_list, double flags[3]);
void listPeripherals(host_node * factory_list, int factoryId);

#endif //FACTORY_OF_THE_FUTURE_FACTORY_COMM_H
