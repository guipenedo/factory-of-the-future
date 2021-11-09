#ifndef FACTORY_OF_THE_FUTURE_CONNECTION_H
#define FACTORY_OF_THE_FUTURE_CONNECTION_H

void connect_to_dashboard(const char *, host_node **, int *, int);
ClientThreadData * connect_new_factory(char *, host_node *);
void send_connect_back(int, ClientThreadData *);

#endif //FACTORY_OF_THE_FUTURE_CONNECTION_H
