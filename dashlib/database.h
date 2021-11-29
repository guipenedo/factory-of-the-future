#ifndef FACTORY_OF_THE_FUTURE_DATABASE_H
#define FACTORY_OF_THE_FUTURE_DATABASE_H

#include "../network/tcp.h"

#define MAX_SIZE 25
#define NUM_COMMANDS 2
#define MAX_FACTORIES MAX_CLIENTS - 2
#define MAX_MEASURES_STORED 2000

typedef double database_type [MAX_FACTORIES][MAX_MEASURES_STORED][4];
typedef int current_type[MAX_FACTORIES];


#endif //FACTORY_OF_THE_FUTURE_DATABASE_H
