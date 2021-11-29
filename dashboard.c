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

/* Constants declaration */

#define SHOW_FLAGS 1                    /* ID */
#define PLOT_FLAGS 1                    /* ID */
#define SENDCOM_FLAGS 3                 /* ID ACTUATOR VALUE */
#define SETTHRESHOLD_FLAGS 2            /* SENSOR VALUE */              /* OK */
#define RECORD_FLAGS 1                  /* ID */
#define DOWNLOAD_HISTORY_FLAGS 1        /* ID */
#define SHOW_HISTORY_FLAGS 2            /* ID LINES */
#define DISPSENSOR_FLAGS 1              /* ID */
#define DISPACTUATOR_FLAGS 1            /* ID */
#define PREDICT_FLAGS 3                 /* ID SENSOR TIME */

/* Global variables */

database_type database;    /* |time|temperature|humidity|pressure| */
current_type current;
double temperatureThreshold = 30.0;
double pressureThreshold = 150000.0;
double humidityThreshold = 75.0;

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

// TODO dashboard team
void receive_sensor_data(int factId, SensorData data) {
    database[factId][current[factId]][0] = data.time;
    database[factId][current[factId]][1] = data.temperature;
    database[factId][current[factId]][2] = data.humidity;
    database[factId][current[factId]][3] = data.pressure;
    current[factId] = (current[factId] + 1) % MAX_MEASURES_STORED;
    int alarm = 0;
    if (alarm) { // TODO
        trigger_alarm(factId);
    }
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
    int latest = (current[factoryId] - 1) % MAX_MEASURES_STORED;
    printf("Factory %d has measured at time %lf temperature: %lf humidity: %lf pressure: %lf \n",
           factoryId, database[factoryId][latest][0],database[factoryId][latest][1],
           database[factoryId][latest][2],database[factoryId][latest][3]);

}

void setTreshold(int sensor, double value) {

  switch (sensor) {

    case 1:
      temperatureThreshold = value;

    case 2:
      humidityThreshold = value;

    case 3:
      pressureThreshold = value;

    default:
      printf("[EEROR] You have not set up a new threshold");

  }

}

int plotParameter(int* param, int* time, int counter, char* title, char* nameFile, char* setting){

    char * commandsForGnuplot[] = {title, setting};
    FILE * file = fopen(nameFile, "w");
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i;
    for (i=0; i < counter; i++)
    {
        fprintf(file, "%d %d \n", *(time+i), *(param+i)); //Write the data to a temporary file
        fflush(file);
    }

    for (i=0; i < NUM_COMMANDS; i++)
    {

        fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);//Send commands to gnuplot one by one.
        fflush(gnuplotPipe);
    }
    return 0;

}

/* Main function */

int main(int argc, char **argv) {

    if (argc != 1) {
        printf("[ERROR] Dashboard does not take any arguments.\n");
        exit(-1);
    }

    /* Definition & Initialization of variables */

    int bytes_read, index;
    int nbcommands = 0;
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

            /* Introduce command */

            printf("Introduce a command >> ");

            bytes_read = getline (&string, &size, stdin);

        }

        nbcommands++;

        if (bytes_read == -1) {
            printf("\n[ERROR] The command is not valid. Try again.\n");
            continue;
        }

        else {

            /* Divide the input string in command + flags using the space delimiter */

            command = strtok (string, delimiter);

            /* Select between different commands */

            /* 1. SHOW command */

            if (strcmp (command, "show") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < SHOW_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    continue;
                }

                /* Do something here */

                showCurrent(((int) flags[0]));

                /* Do something here */


                continue;

            }

            else if (strcmp (command, "plot") == 0) {
                /* Capture 1 flags (id) */

                for (index = 0; index < PLOT_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    continue;
                }

                plot_sensors(flags[0], database, current);

                printf("\nThe user have selected the PLOT command:\n");
                printf("Factory ID >> %d\n",flags[0]);

            }

            else if (strcmp (command, "list\n") == 0) {
                /* Capture 0 flags */
                printf("\nThe user has selected the LIST command:\n");
                printf("Factory IDs: ");
                host_node * prev = factory_list;
                while (prev->next != NULL){
                    printf("%d ", prev->next->host->host_id);
                    prev = prev->next;
                }
                printf("\n");
                continue;

            }

            else if (strcmp (command, "sendcom") == 0) {

                /* Capture 3 flags (id, actuator, value) */

                for (index = 0; index < SENDCOM_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");

                    continue;
                }

                /* Do something here */
                host_node * factory = get_host_by_id(factory_list, flags[0]);
                if (factory == NULL) {
                    printf("\n[ERROR] Invalid factory ID.\n");
                    continue;
                }
                ClientThreadData * client = factory->host;

                if (flags[2] != 0 && flags[2] != 1) {
                    printf("\n[ERROR] Actuator state must be 0 or 1.\n");
                    continue;
                }

                sprintf(cmd_args, "%d", flags[2]);

                if (flags[1] == 0) {// LED
                    send_command_to_server(CMD_SET_LED_STATE, cmd_args, NULL, client);
                } else if(flags[1] == 1) {// RELAY
                    send_command_to_server(CMD_SET_RELAY_STATE, cmd_args, NULL, client);
                } else {
                    printf("\n[ERROR] 0 -> LED | 1 -> RELAY\n");
                    continue;
                }


                /* Do something here */

                /* Test */

                printf("\nThe user have selected the SENDCOM command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                printf("Actuator ID >> %d\n",flags[1]);
                printf("Value >> %d\n",flags[2]);


                continue;

            }

            /* 4. SETTHRESHOLD command */

            else if (strcmp (command, "setthreshold") == 0) {

                /* Capture 2 flags (sensor, value) */

                for (index = 0; index < SETTHRESHOLD_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");

                    continue;
                }

                /* Do something here */

                setTreshold (((int) flags[0]), flags[1]);

                /* Do something here */

                continue;

            }

            else if (strcmp (command, "downloadhistory") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < DOWNLOAD_HISTORY_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    continue;
                }

                host_node * factory = get_host_by_id(factory_list, flags[0]);
                if (factory == NULL) {
                    printf("\n[ERROR] Invalid factory ID.\n");
                    continue;
                }
                ClientThreadData * client = factory->host;
                printf("\nThe user have selected the DOWNLOAD HISTORY command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                send_command_to_server(CMD_SEND_SENSOR_HISTORY_FILE, NULL, NULL, client);
                receive_sensor_history_file(client);

                printf("File downloaded successfully!\n");

                continue;

            }

            else if (strcmp (command, "showhistory") == 0) {

                /* Capture 2 flags (id, lines) */

                for (index = 0; index < SHOW_HISTORY_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    continue;
                }

                host_node * factory = get_host_by_id(factory_list, flags[0]);
                if (factory == NULL) {
                    printf("\n[ERROR] Invalid factory ID.\n");
                    continue;
                }

                if (flags[1] <= 0 || flags[1] > 200) {
                    printf("\n[ERROR] Lines must be between 1 and 200.\n");
                    continue;
                }

                ClientThreadData * client = factory->host;
                printf("\nThe user have selected the SHOW HISTORY command:\n");
                printf("Factory ID >> %d\n", flags[0]);
                printf("Lines >> %d\n", flags[1]);


                SensorData * data = malloc(flags[1] * sizeof(SensorData));
                int lines = read_sensor_data_from_file(data, flags[1], flags[0]);
                printf("Read %d data lines\n", lines);
                for (int i = 0; i < lines; ++i) {
                    printf("Ts=%ld T=%lf H=%lf P=%lf\n", data[i].time, data[i].temperature, data[i].humidity, data[i].pressure);
                }
                free(data);
            }

            else if (strcmp (command, "dispsensor") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < DISPSENSOR_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");

                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the DISPSENSOR command:\n");
                printf("Factory ID >> %d\n",flags[0]);


                continue;

            }

            else if (strcmp (command, "dispactuator") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < DISPACTUATOR_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");

                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the DISPACTUATOR command:\n");
                printf("Factory ID >> %d\n",flags[0]);


                continue;

            }

            else if (strcmp (command, "predict") == 0) {

                /* Capture 3 flags (id, sensor, time) */

                for (index = 0; index < PREDICT_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (double) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");

                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the PREDICT command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                printf("Sensor ID >> %d\n",flags[1]);
                printf("Time >> %d\n",flags[2]);


                continue;

            }

            /* 10. HELP Command */

            else if (strcmp (command, "help\n") == 0) {

              printf("\n");
              printf("Introduce a valid command and the required parameters:\n");
              printf("  - show id >> show data captured by the sensors of one factory via its ID.\n");
              printf("  - plot id sensor >> plot data captured by a sensor of one factory.\n");
              printf("  - sendcom id actuator value >> send a command to one actuator of one factory.\n");
              printf("    - LED >> value = 0\n");
              printf("    - Relay >> value = 1\n");
              printf("  - settreshold sensor value >> set a threshold for a specific kind of sensor:\n");
              printf("    - temperature >> sensor = 1\n");
              printf("    - pressure >> sensor = 2\n");
              printf("    - humidity >> sensor = 3\n");
              printf("  - downloadhistory id >> download data of one factory.\n");
              printf("  - dispsensor id >> display the list of sensors of one factory.\n");
              printf("  - dispactuator id >> display the list of actuators of one factory.\n");
              printf("  - predict sensor time >> predict a future value of a sensor using ML.\n");

              continue;

            }

            /* 11. Exit command */

            else if (strcmp (command, "exit\n") == 0) {

              printf("\e[1;1H\e[2J");
              break;

            }

            else {
                printf("\n[ERROR] The command is not valid. Try again\n");

                continue;
            }

        }

    }

    free(string);

    /* Close network */

    close_all_connections(factory_list);
    free_host_list(factory_list);
    pthread_mutex_destroy(&fact_id_mutex);

    return 0;
}
