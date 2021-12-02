#include <stdio.h>
#include "factory_comm.h"
#include "../interfaces/network_commands.h"

char cmd_args[MAX_ARGS_BUFFER_SIZE];

void trigger_alarm(int factId, host_node * factory_list) {
    sprintf(cmd_args, "%d", factId);

    host_node * factory = factory_list;
    while (factory->next != NULL) {
        factory = factory->next;
        send_command_to_server(CMD_TRIGGER_ALARM, cmd_args, NULL, factory->host);
    }
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

void listPeripherals(host_node * factory_list, int factoryId) {
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

void getPrediction(host_node * factory_list, ClientThreadData * ml_client, int fact_id, int h, int m, int s) {
    host_node * factory = get_host_by_id(factory_list, fact_id);
    if (factory == NULL) {
        printf("\n[ERROR] Invalid factory ID.\n");
        return;
    }
    else if ( (h < 0 || h > 23) || (m < 0 || m > 59) || (s < 0 || s > 59))
    {
        printf("\n[ERROR] Invalid time input, please use hh mm ss.\n");
        return;
    }
    

    sprintf(cmd_args, "%d %d %d %d", fact_id, h, m, s);
    char res[MAX_BUFFER_SIZE];
    send_command_to_server(CMD_PREDICT, cmd_args, res, ml_client);
    printf("Prediction for Factory %d at %d:%d:%d = %s\n", fact_id, h, m, s, res);
}
