/* Include required libraries */

#include "network/tcp.h"
#include "network/connection.h"
#include "interfaces/network_commands.h"
#include "utils/host_list.h"
#include "interfaces/peripherals.h"
#include "utils/sensor_data_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dashlib/plot.h"
#include "dashlib/database.h"
#include "dashlib/factory_comm.h"
#include "dashlib/commands.h"
#include "dashlib/listing.h"
#include "dashlib/factory_data.h"

/* Global variables definition */

database_type database;    /* |time|temperature|humidity|pressure| */
current_type current;
double thresholds[3] = {30.0, 90.0, 150000.0};

host_node * factory_list;
ClientThreadData * ml_client = NULL;
pthread_t server_thread;

int number_of_factories = 0;
pthread_mutex_t fact_id_mutex;

void handle_command(int commandId, char * args, char * response, int connfd, char * client_ip);

/* -------------------------------------------------------------------------- */

/* Main function ------------------------------------------------------------ */

int main (int argc, char **argv) {

    /* Print an error message if args are provided */

    if (argc != 1) {
        printf("[ERROR 01] Dashboard does not take any arguments.\n");
        exit(-1);
    }

    /* Definition & initialization of variables */

    int bytes_read, status;
    int nbcommands, nbparameters = 0;
    double flags [3] = {0.0, 0.0, 0.0};
    size_t size = MAX_SIZE * sizeof (char);
    char *string = malloc (size);
    char *command;
    char *delimiter = " ";

    /* Network initialization */

    ml_client = NULL;
    initialize_host_list(&factory_list);
    pthread_mutex_init(&fact_id_mutex, NULL);
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    /* Welcome window */

    printf("\e[1;1H\e[2J");
    printf("\n%61s\n\n", "<< THE FACTORY OF THE FUTURE - DASHBOARD >>");
    printf("%62s\n\n", "Created by MAE 2 ES students in December 2021");
    printf("%67s\n\n","Introduce the command help to learn more about the tool");
    printf("%61s\n\n\n","Introduce the command exit to stop the tool");

    /* First command */

    printf("Introduce a command >> ");
    bytes_read = getline (&string, &size, stdin);

    /* Infinite loop */

    while (1) {

        if (nbcommands > 0) {

            /* Introduce a command */

            printf("Introduce a command >> ");
            bytes_read = getline (&string, &size, stdin);

        }

        nbcommands++;

        /* Error while reading data from keyboard */

        if (bytes_read == -1) {
            raiseError (2);
            continue;
        }



        nbparameters = checkParameters (string);
        command = strtok (string, delimiter);

        /* Select between different commands */

        /* SHOW COMMAND --------------------------------------------------------- */

        if (strcmp (command, "show") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, SHOW_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            showCurrent(database,current,(int) flags[0]);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the SHOW command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
            }

        }

        /* LIST PERIPHERALS COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "listperipherals") == 0 || strcmp (command, "listsensors") == 0 || strcmp (command, "listactuators") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, LIST_PERIPHERALS_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            listPeripherals(factory_list,(int) flags[0]);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the LIST PERIPHERALS command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
            }

        }

        /* LIST PERIPHERALS COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "listalarms\n") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, LIST_ALARMS_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            listAlarms(factory_list);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the LIST ALARMS command.\n");
            }

        }

            /* ---------------------------------------------------------------------- */

            /* PLOT COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "plot") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, PLOT_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            plot_sensors(factory_list, (int) flags[0], database, current);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the PLOT command:\n");
                printf("Factory ID >> %d\n", (int) flags[0]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* LIST COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "list\n") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, LIST_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            listFactories (factory_list);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the LIST command:\n");
            }

        }

            /* ---------------------------------------------------------------------- */

            /* SENDCOM COMMAND ------------------------------------------------------ */

        else if (strcmp (command, "sendcom") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, SENDCOM_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            sendcommand (factory_list, flags);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the SENDCOM command:\n");
                printf("Factory ID >> %d\n", (int) flags[0]);
                printf("Actuator ID >> %d\n",(int) flags[1]);
                printf("Value >> %d\n",(int) flags[2]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* SETTHRESHOLD COMMAND ------------------------------------------------- */

        else if (strcmp (command, "setthreshold") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, SETTHRESHOLD_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            setThreshold ((int) flags[0], flags[1], thresholds);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the SETTHRESHOLD command:\n");
                printf("Sensor >> %d\n",(int) flags[0]);
                printf("Value >> %d\n",(int) flags[1]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* DOWNLOAD HISTORY COMMAND --------------------------------------------- */

        else if (strcmp (command, "downloadhistory") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, DOWNLOAD_HISTORY_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            downloadhistory (factory_list, flags);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the DOWNLOADHISTORY command:\n");
                printf("Factory ID >> %d\n", (int) flags[0]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* SHOW HISTORY COMMAND ------------------------------------------------- */

        else if (strcmp (command, "showhistory") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, SHOW_HISTORY_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            showhistory (factory_list, flags);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the TEST command:\n");
                printf("Factory ID >> %d\n", (int) flags[0]);
                printf("Lines >> %d\n", (int) flags[1]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* PREDICT COMMAND ------------------------------------------------------ */

        else if (strcmp (command, "predict") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, PREDICT_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            getPrediction(factory_list,ml_client,(int) flags[0],(int) flags[1],(int) flags[2],(int) flags[3]);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the PREDICT command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
                printf("Time >> %d %d %d\n",(int) flags[1], (int) flags[2], (int) flags[3]);
            }

        }

            /* ---------------------------------------------------------------------- */

            /* HELP COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "help\n") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, HELP_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            printf("\n");
            printf("Introduce a valid command and the required parameters:\n");
            printf("  - show id >> show data captured by the sensors of one factory via its ID.\n");
            printf("  - plot id sensor >> plot data captured by a sensor of one factory.\n");
            printf("  - sendcom id actuator value >> send a command to one actuator of one factory.\n");
            printf("    - LED >> actuator = 0\n");
            printf("    - Relay >> actuator = 1\n");
            printf("    - OFF >> value = 0\n");
            printf("    - ON >> value = 1\n");
            printf("  - setthreshold sensor value >> set a threshold for a specific kind of sensor:\n");
            printf("    - temperature >> sensor = 1\n");
            printf("    - pressure >> sensor = 2\n");
            printf("    - humidity >> sensor = 3\n");
            printf("  - downloadhistory id >> download data of one factory.\n");
            printf("  - listperipherals id >> display the list of sensors and actuators of one factory.\n");
            printf("  - listalarms >> show all the factories with alarm.\n");
            printf("  - predict id hh mm ss >> predict a future value of the temperature in a factory using ML.\n");

        }

            /* ---------------------------------------------------------------------- */

            /* EXIT COMMAND --------------------------------------------------------- */

        else if (strcmp (command, "exit\n") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, HELP_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            printf("\e[1;1H\e[2J");
            break;

        }

            /* ---------------------------------------------------------------------- */

        else {
            raiseError(2);
            continue;
        }

    }

    /* Close network */

    close_all_connections(factory_list);
    free_host_list(factory_list);
    pthread_mutex_destroy(&fact_id_mutex);

    return 0;

}

void handle_command(int commandId, char * args, char * response, int connfd, char * client_ip) {
    if (commandId == CMD_INIT_NEW_FACTORY || commandId == CMD_INIT_ML) {
        pthread_mutex_lock(&fact_id_mutex);
        int factory_id = ++number_of_factories;
        dashboard_init_new_host(factory_id, client_ip, factory_list, &ml_client, commandId == CMD_INIT_NEW_FACTORY);
        sprintf(response, "%d", factory_id);
        pthread_mutex_unlock(&fact_id_mutex);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        SensorData data;
        sensor_data_from_command(args, &factId, &data);

        receive_sensor_data(database,current,factId, data, thresholds, factory_list);
    }
}