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
#define LIST_FLAGS 0
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
      continue;
  }
  ClientThreadData * client = factory->host;
  printf("\nThe user have selected the DOWNLOAD HISTORY command:\n");
  printf("Factory ID >> %d\n",flags[0]);
  send_command_to_server(CMD_SEND_SENSOR_HISTORY_FILE, NULL, NULL, client);
  receive_sensor_history_file(client);

  printf("File downloaded successfully!\n");

}

void showhistory (host_node * factory_list, double flags[3]) {

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

void sendcommand (host_node * factory_list, double flags[3]) {

  host_node * factory = get_host_by_id(factory_list, (int) flags[0]);
  if (factory == NULL) {
      printf("\n[ERROR] Invalid factory ID.\n");
      continue;
  }
  ClientThreadData * client = factory->host;

  if ((int) flags[2] != 0 && (int) flags[2] != 1) {
      printf("\n[ERROR] Actuator state must be 0 or 1.\n");
      continue;
  }

  sprintf(cmd_args, "%d", (int) flags[2]);

  if ((int) flags[1] == 0) {// LED
      send_command_to_server(CMD_SET_LED_STATE, cmd_args, NULL, client);
  } else if((int) flags[1] == 1) {// RELAY
      send_command_to_server(CMD_SET_RELAY_STATE, cmd_args, NULL, client);
  } else {
      printf("\n[ERROR] 0 -> LED | 1 -> RELAY\n");
      continue;
  }

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

/* Check parameters function ------------------------------------------------ */

int checkParameters (char * string) {

  /* Function parameters */

  int index = 0;
  int nbparameters = 0;
  char separator = " ";
  char endline = "\n"

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

  if (nbparms != nbexpected) {
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
  int test = 1;
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

      if (test == 1) {
        printf("\n");
        printf("The user have selected the SHOW command:\n");
        printf("Factory ID >> %d\n",flags[0]);
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

      if (test == 1) {
        printf("\n");
        printf("The user have selected the PLOT command:\n");
        printf("Factory ID >> %d\n",flags[0]);
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

      if (test == 1) {
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

      if (test == 1) {
        printf("\n");
        printf("The user have selected the SENDCOM command:\n");
        printf("Factory ID >> %d\n",flags[0]);
        printf("Actuator ID >> %d\n",flags[1]);
        printf("Value >> %d\n",flags[2]);
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

      setTreshold ((int) flags[0], flags[1]);

      /* Test */

      if (test == 1) {
        printf("\n");
        printf("The user have selected the SETTHRESHOLD command:\n");
        printf("Sensor >> %d\n",flags[0]);
        printf("Value >> %d\n",flags[1]);
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

      if (test == 1) {
        printf("\n");
        printf("The user have selected the DOWNLOADHISTORY command:\n");
        printf("Factory ID >> %d\n",flags[0]);
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

      showhistory (factory_list, flags[3]);

      /* Test */

      if (test == 1) {
        printf("\n");
        printf("The user have selected the TEST command:\n");
        printf("Factory ID >> %d\n",flags[0]);
        printf("Lines >> %d\n",flags[1]);
      }

    }

    /* ---------------------------------------------------------------------- */

    /* DISPSENSOR COMMAND --------------------------------------------------- */

    else if (strcmp (command, "dispsensor") == 0) {

      /* Capture flags */

      status = captureFlags (nbparameters, DISPSENSOR_FLAGS, flags, command);

      if (status == 1) {
        continue;
      }

      /* Command implementation */

      // Put a function here

      /* Test */

      if (test == 1) {
        printf("\n");
        printf("The user have selected the DISPSENSOR command:\n");
        printf("Factory ID >> %d\n",flags[0]);
      }

    }

    /* ---------------------------------------------------------------------- */

    /* DISPACTUATOR COMMAND ------------------------------------------------- */

    else if (strcmp (command, "dispactuator") == 0) {

      /* Capture flags */

      status = captureFlags (nbparameters, DISPACTUATOR_FLAGS, flags, command);

      if (status == 1) {
        continue;
      }

      /* Command implementation */

      // Put a function here

      /* Test */

      if (test == 1) {
        printf("\n");
        printf("The user have selected the DISPACTUATOR command:\n");
        printf("Factory ID >> %d\n",flags[0]);
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

      if (test == 1) {
        printf("\n");
        printf("The user have selected the PREDICT command:\n");
        printf("Factory ID >> %d\n",flags[0]);
        printf("Sensor ID >> %d\n",flags[1]);
        printf("Time >> %d\n",flags[2]);
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
