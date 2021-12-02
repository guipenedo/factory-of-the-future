#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    fflush(stdout);
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