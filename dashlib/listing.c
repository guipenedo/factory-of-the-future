#include <stdio.h>
#include "listing.h"
#include "../interfaces/network_commands.h"

void listFactories (host_node * factory_list) {

    printf("Factory IDs: ");
    host_node * prev = factory_list;
    while (prev->next != NULL){
        printf("%d ", prev->next->host->host_id);
        prev = prev->next;
    }
    printf("\n");

}

void listAlarms(host_node * factory_list) {
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