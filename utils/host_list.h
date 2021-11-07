#ifndef FACTORY_OF_THE_FUTURE_HOST_LIST_H
#define FACTORY_OF_THE_FUTURE_HOST_LIST_H

#include "../network/tcp_ip.h"

typedef struct host_n {
    ClientThreadData * host;
    int host_id;
    struct host_n * next;
} host_node;

int initialize_host_list(host_node **);
int push_host(host_node *, int, ClientThreadData *);
void pop_host(host_node *);
void pop_host_by_id(host_node *, int);
void free_host_list();

void close_connection(host_node *);
void close_all_connections(host_node *);


#endif //FACTORY_OF_THE_FUTURE_HOST_LIST_H
