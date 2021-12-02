#ifndef FACTORY_OF_THE_FUTURE_COMMANDS_H
#define FACTORY_OF_THE_FUTURE_COMMANDS_H

#define DEBUG_COMMANDS 1

/* Constants declaration */

#define SHOW_FLAGS 1                    /* ID */
#define LIST_PERIPHERALS_FLAGS 1        /* ID */
#define PLOT_FLAGS 1                    /* ID */
#define LIST_FLAGS 0
#define LIST_ALARMS_FLAGS 0
#define SENDCOM_FLAGS 3                 /* ID ACTUATOR VALUE */
#define SETTHRESHOLD_FLAGS 2            /* SENSOR VALUE */
#define DOWNLOAD_HISTORY_FLAGS 1        /* ID */
#define SHOW_HISTORY_FLAGS 2            /* ID LINES */
#define PREDICT_FLAGS 4                 /* ID HOURS MINUTES SECONDS */
#define HELP_FLAGS 0

int checkParameters (char * string);
void raiseError (int errorCode);
int captureFlags (int nbparams, int nbexpected, double flags[3], char* command);

#endif //FACTORY_OF_THE_FUTURE_COMMANDS_H
