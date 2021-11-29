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

/* Constants declaration */

#define MAX_SIZE 25
#define NUM_COMMANDS 2
#define MAX_FACTORIES MAX_CLIENTS - 2
#define MAX_MEASURES_STORED 20

#define SHOW_FLAGS 1                    /* ID */
#define PLOT_FLAGS 2                    /* ID SENSOR */
#define SENDCOM_FLAGS 3                 /* ID ACTUATOR VALUE */
#define SETTHRESHOLD_FLAGS 3            /* ID SENSOR VALUE */
#define RECORD_FLAGS 1                  /* ID */
#define DISPSENSOR_FLAGS 1              /* ID */
#define DISPACTUATOR_FLAGS 1            /* ID */
#define PREDICT_FLAGS 3                 /* ID SENSOR TIME */

/* Global variables */

double database[MAX_FACTORIES][MAX_MEASURES_STORED][4];    /* |time|temperature|humidity|pressure| */
int current[MAX_FACTORIES];

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

    printf("Factory %d has measured at time %lf temperature: %lf humidity: %lf pressure: %lf \n",
           factoryId, database[factoryId][current[factoryId]][0],database[factoryId][current[factoryId]][1],
           database[factoryId][current[factoryId]][2],database[factoryId][current[factoryId]][3]);

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

            /* Clear previous commands */

            printf("\e[1;1H\e[2J");

            /* User interface */

            printf("\n%61s\n\n", "<< THE FACTORY OF THE FUTURE - DASHBOARD >>");

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

                /* Capture 2 flags (id, sensor) */

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

                /* Do something here */

                /* Temperature*/
                int temp_data[] = {1,3,5,6,6,5,2,4,5};
                int time_data[] = {1,2,3,4,5,6,7,8,9};
                int *temper = temp_data;
                int *time = time_data;
                plotParameter(temper, time, sizeof(temp_data)/sizeof(temp_data[0]), "set title \"Temperature\"", "data.temp", "plot 'data.temp' with lines");

                /* Pressure*/
                int press_data[] = {13,34,25,16,26,15,12,14,15};
                int* press = press_data;
                plotParameter(press, time, sizeof(press_data)/sizeof(press_data[0]), "set title \"Pressure\"", "data.press", "plot 'data.press' with lines");


                /* Humidity*/
                int humidity_data[] = {32,14,5,12,26,35,12,24,5};
                int* hum = humidity_data;
                plotParameter(hum, time, sizeof(humidity_data)/sizeof(humidity_data[0]), "set title \"Humidity\"", "data.hum", "plot 'data.hum' with lines");






                /* Do something here */

                /* Test */

                printf("\nThe user have selected the PLOT command:\n");
                printf("Factory ID >> %d\n",flags[0]);
                printf("Sensor ID >> %d\n",flags[1]);
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