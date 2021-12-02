#include "plot.h"
#include <stdio.h>
#include "database.h"

char * titles[] = {"set title \"Temperature\"", "set title \"Pressure\"",  "set title \"Humidity\""};
char * filenames[] = {"temp.data", "press.data",  "hum.data"};
char * settings[] = {"plot 'temp.data' with lines", "plot 'press.data' with lines",  "plot 'hum.data' with lines"};

int plot_parameter(int factoryId, int type, database_type database, current_type current) {
    int ti = current[factoryId];

    char * commandsForGnuplot[] = {titles[type], settings[type]};
    FILE * file = fopen(filenames[type], "w");
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");

    int i;
    for(i = 0; i < MAX_MEASURES_STORED; i++) {
        int real_i = (ti + i) % MAX_MEASURES_STORED;
        if(database[factoryId][real_i][0] != 0)
            fprintf(file, "%f %f \n", database[factoryId][real_i][0], database[factoryId][real_i][type + 1]); //Write the data to a temporary file
    }
    fflush(file);

    for (i=0; i < NUM_COMMANDS; i++)
        fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);//Send commands to gnuplot one by one.
    fflush(gnuplotPipe);
}


void plot_sensors(host_node * factory_list, int factoryId, database_type database, current_type current) {
    host_node * factory = get_host_by_id(factory_list, factoryId);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    for (int type = 0; type < 3; ++type) {
        plot_parameter(factoryId, type, database, current);
    }
}