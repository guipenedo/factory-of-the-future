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
#include <unistd.h>
#include "dashlib/plot.h"
#include "dashlib/database.h"

/* Constants declaration */

#define SHOW_FLAGS 1                    /* ID */
#define PLOT_FLAGS 1                    /* ID */
#define SENDCOM_FLAGS 3                 /* ID ACTUATOR VALUE */
#define SETTHRESHOLD_FLAGS 3            /* ID SENSOR VALUE */
#define RECORD_FLAGS 1                  /* ID */
#define DISPSENSOR_FLAGS 1              /* ID */
#define DISPACTUATOR_FLAGS 1            /* ID */
#define PREDICT_FLAGS 3                 /* ID SENSOR TIME */

/* Global variables */

database_type database;    /* |time|temperature|humidity|pressure| */
current_type current;

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


int main(int argc, char **argv) {

    if (argc != 1) {
        printf("[ERROR] Dashboard does not take any arguments.\n");
        exit(-1);
    }

    /* Definition & Initialization of variables */

    int bytes_read, index = 0;
    int nbcommands = 0;
    int flags [3] = {0, 0, 0};
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
            sleep(5);
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

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */

                showCurrent(flags[0]);

                /* Do something here */

                sleep(10);

                continue;

            }

            else if (strcmp (command, "plot") == 0) {
                /* Capture 1 flags (id) */

                for (index = 0; index < PLOT_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                plot_sensors(flags[0], database, current);

                printf("\nThe user have selected the PLOT command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "sendcom") == 0) {

                /* Capture 3 flags (id, actuator, value) */

                for (index = 0; index < SENDCOM_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
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
                sleep(10);

                continue;

            }

            else if (strcmp (command, "setthreshold") == 0) {

                /* Capture 3 flags (id, sensor, value) */

                for (index = 0; index < SETTHRESHOLD_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the SETTHRESHOLD command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                printf("Sensor ID >> %d\n",flags[1]);
                printf("Value >> %d\n",flags[2]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "record") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < RECORD_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the RECORD command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "dispsensor") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < DISPSENSOR_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the DISPSENSOR command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "dispactuator") == 0) {

                /* Capture 1 flag (id) */

                for (index = 0; index < DISPACTUATOR_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the DISPACTUATOR command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "predict") == 0) {

                /* Capture 3 flags (id, sensor, time) */

                for (index = 0; index < PREDICT_FLAGS; index++) {

                    command = strtok (NULL, delimiter);

                    /* Transform from string to int */

                    flags[index] = (int) strtol(command, NULL, 10);

                }

                command = strtok (NULL, delimiter);
                if (command != NULL) {
                    printf("\n[ERROR] The command is not valid. Try again.\n");
                    sleep(5);
                    continue;
                }

                /* Do something here */



                /* Do something here */

                /* Test */

                printf("\nThe user have selected the PREDICT command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                printf("Sensor ID >> %d\n",flags[1]);
                printf("Time >> %d\n",flags[2]);
                sleep(10);

                continue;

            }

            else if (strcmp (command, "help\n") == 0) {

                /* Help - information about the tool */

                printf("\nThe user have selected the HELP command\n");
                sleep(10);

                continue;

            }

            else if (strcmp (command, "exit\n") == 0) {

                /* Exit main function */

                printf("\e[1;1H\e[2J");
                exit(0);

            }

            else {
                printf("\n[ERROR] The command is not valid. Try again\n");
                sleep(5);
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