#ifndef FACTORY_OF_THE_FUTURE_CONNECTION_H
#define FACTORY_OF_THE_FUTURE_CONNECTION_H

#include "../utils/host_list.h"

void connect_to_dashboard(const char *, host_node **, int *, int);
ClientThreadData * connect_new_factory(char *, host_node *);
void dashboard_init_new_host(int, const char *, host_node *, ClientThreadData **, int);

#endif //FACTORY_OF_THE_FUTURE_CONNECTION_H
