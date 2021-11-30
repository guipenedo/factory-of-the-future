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
#include "utils/sensor_history.h"

#define DEBUG_COMMANDS 1

/* Constants declaration */

#define SHOW_FLAGS 1                    /* ID */
#define LIST_PERIPHERALS_FLAGS 1        /* ID */
#define PLOT_FLAGS 1                    /* ID */
#define LIST_FLAGS 0
#define LIST_ALARMS_FLAGS 0
#define SENDCOM_FLAGS 3                 /* ID ACTUATOR VALUE */
#define SETTHRESHOLD_FLAGS 2            /* SENSOR VALUE */
#define RECORD_FLAGS 1                  /* ID */
#define DOWNLOAD_HISTORY_FLAGS 1        /* ID */
#define SHOW_HISTORY_FLAGS 2            /* ID LINES */
#define DISPSENSOR_FLAGS 1              /* ID */
#define DISPACTUATOR_FLAGS 1            /* ID */
#define PREDICT_FLAGS 3                 /* ID SENSOR TIME */
#define HELP_FLAGS 0

/* Global variables */

database_type database;    /* |time|temperature|humidity|pressure| */
current_type current;
double temperatureThreshold = 30.0;
double pressureThreshold = 150000.0;
double humidityThreshold = 90.0;

host_node * factory_list;
ClientThreadData * ml_client = NULL;
pthread_t server_thread;

int number_of_factories = 0;
pthread_mutex_t fact_id_mutex;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void trigger_alarm(int factId) {
    sprintf(cmd_args, "%d", factId);

    host_node * factory = factory_list;
    while (factory->next != NULL) {
        factory = factory->next;
        send_command_to_server(CMD_TRIGGER_ALARM, cmd_args, NULL, factory->host);
    }
}

void receive_sensor_data(int factId, SensorData data) {
    database[factId][current[factId]][0] = data.time;
    database[factId][current[factId]][1] = data.temperature;
    database[factId][current[factId]][2] = data.humidity;
    database[factId][current[factId]][3] = data.pressure;
    current[factId] = (current[factId] + 1) % MAX_MEASURES_STORED;
    if (data.temperature >= temperatureThreshold || data.humidity >= humidityThreshold || data.pressure >= pressureThreshold)
        trigger_alarm(factId);
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

        receive_sensor_data(factId, data);
    }
}

/* Dashboard important functions */

void showCurrent(int factoryId) {
    /* Displays for the factory ID given the measures done by the sensor at a time */
    int i;
    for (i = 0; i < 10; i++){
        int id = (current[factoryId] - 10 + i) % MAX_MEASURES_STORED;
        if (database[factoryId][id][0] != 0)
            printf("Factory %d has measured at time %lf temperature: %lf humidity: %lf pressure: %lf \n",
                   factoryId, database[factoryId][id][0],database[factoryId][id][1],
                   database[factoryId][id][2],database[factoryId][id][3]);
    }
}

void listPeripherals(int factoryId) {
    host_node * factory = get_host_by_id(factory_list, factoryId);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    ClientThreadData * client = factory->host;
    char res[MAX_BUFFER_SIZE];
    send_command_to_server(CMD_GET_PERIPHERALS, NULL, res, client);
    int sensors, led, relay;
    sscanf(res, "%d %d %d", &sensors, &led, &relay);
    printf("Factory %d: SENSORS: %s | LED: %s | RELAY: %s\n", factoryId, (sensors ? "YES" : "NO"), (led ? "YES" : "NO"), (relay ? "YES" :
    "NO"));
}

void listAlarms() {
    printf("Factory ID | ALARM :\n");
    host_node * prev = factory_list;
    char res[MAX_BUFFER_SIZE];
    int state;
    while (prev->next != NULL){
        send_command_to_server(CMD_GET_ALARM_STATE, NULL, res, prev->next->host);
        sscanf(res, "%d", &state);
        printf("%d | %s\n", prev->next->host->host_id, (state ? "YES" : "NO"));
        prev = prev->next;
    }
}

void listFactories (host_node * factory_list) {

    printf("Factory IDs: ");
    host_node * prev = factory_list;
    while (prev->next != NULL){
        printf("%d ", prev->next->host->host_id);
        prev = prev->next;
    }
    printf("\n");

}

void downloadhistory (host_node * factory_list, double flags[3]) {

    host_node * factory = get_host_by_id(factory_list, flags[0]);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    ClientThreadData * client = factory->host;
    send_command_to_server(CMD_SEND_SENSOR_HISTORY_FILE, NULL, NULL, client);
    receive_sensor_history_file(client);

    printf("File downloaded successfully!\n");

}

void showhistory (host_node * factory_list, double flags[3]) {

    host_node * factory = get_host_by_id(factory_list, flags[0]);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }

    if (flags[1] <= 0 || flags[1] > 200) {
        printf("\n[ERROR] Lines must be between 1 and 200.\n");
        return;
    }

    SensorData * data = malloc(flags[1] * sizeof(SensorData));
    int lines = read_sensor_data_from_file(data, flags[1], flags[0]);
    printf("Read %d data lines\n", lines);
    for (int i = 0; i < lines; ++i) {
        printf("Ts=%ld T=%lf H=%lf P=%lf\n", data[i].time, data[i].temperature, data[i].humidity, data[i].pressure);
    }
    free(data);

}

void sendcommand (host_node * factory_list, double flags[3]) {
    int fact_id = (int) flags[0], actuator = (int) flags[1], state = (int) flags[2];
    host_node * factory = get_host_by_id(factory_list, fact_id);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    ClientThreadData * client = factory->host;

    if (state != 0 && state != 1) {
        printf("\n[ERROR] Actuator state must be 0 or 1.\n");
        return;
    }

    sprintf(cmd_args, "%d", state);

    if (actuator == 0) {// LED
        send_command_to_server(CMD_SET_LED_STATE, cmd_args, NULL, client);
    } else if(actuator == 1) {// RELAY
        send_command_to_server(CMD_SET_RELAY_STATE, cmd_args, NULL, client);
    } else {
        printf("\n[ERROR] 0 -> LED | 1 -> RELAY\n");
    }
}

void setThreshold(int sensor, double value) {

    switch (sensor) {

        case 1:
            temperatureThreshold = value;

        case 2:
            humidityThreshold = value;

        case 3:
            pressureThreshold = value;

        default:
            printf("[ERROR] You have not set up a new threshold\n");

    }

}

/* Check parameters function ------------------------------------------------ */

int checkParameters (char * string) {

    /* Function parameters */

    int index = 0;
    int nbparameters = 0;
    char separator = ' ';
    char endline = '\n';

    /* Function logics */

    while (string[index] != endline) {

        if (string[index] == separator) {
            nbparameters++;
        }

        index++;

    }

    return nbparameters;

}

/* -------------------------------------------------------------------------- */

/* Raise error function ----------------------------------------------------- */

void raiseError (int errorCode) {

    printf("\n");

    switch (errorCode) {

        case 1:
            printf("[ERROR 1] Dashboard does not take any arguments.\n");
            break;

        case 2:
            printf("[ERROR 2] The command is not valid. Try again.\n");
            break;

        case 3:
            printf("[ERROR 3] The arguments are not valid. Try again.\n");
            break;

        default:
            printf("[ERROR 4] Unknown error. Try again.\n");

    }

}

/* -------------------------------------------------------------------------- */

/* Capture flags function --------------------------------------------------- */

int captureFlags (int nbparams, int nbexpected, double flags[3], char* command) {

    /* Function parameters */

    char* delimiter = " ";

    /* Function logics */

    if (nbparams != nbexpected) {
        raiseError (3);
        return 1;
    }

    for (int index = 0; index < nbparams; index++) {
        command = strtok (NULL, delimiter);
        flags[index] = strtol(command, NULL, 10);
    }

    return 0;

}

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

            showCurrent((int) flags[0]);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the SHOW command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
            }

        }

        /* LIST PERIPHERALS COMMAND --------------------------------------------------------- */

        if (strcmp (command, "listperipherals") == 0 || strcmp (command, "listsensors") == 0 || strcmp (command, "listactuators") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, LIST_PERIPHERALS_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            listPeripherals((int) flags[0]);

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the LIST PERIPHERALS command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
            }

        }

        /* LIST PERIPHERALS COMMAND --------------------------------------------------------- */

        if (strcmp (command, "listalarms\n") == 0) {

            /* Capture flags */

            status = captureFlags (nbparameters, LIST_ALARMS_FLAGS, flags, command);

            if (status == 1) {
                continue;
            }

            /* Command implementation */

            listAlarms();

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

            plot_sensors(flags[0], database, current);

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

            setThreshold ((int) flags[0], flags[1]);

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

            // Put a function here

            /* Test */

            if (DEBUG_COMMANDS) {
                printf("\n");
                printf("The user have selected the PREDICT command:\n");
                printf("Factory ID >> %d\n",(int) flags[0]);
                printf("Sensor ID >> %d\n",(int) flags[1]);
                printf("Time >> %d\n",(int) flags[2]);
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
            printf("  - predict sensor time >> predict a future value of a sensor using ML.\n");

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
