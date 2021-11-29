#include "host_list.h"
#include "../interfaces/network_commands.h"
#include <stdlib.h>

int initialize_host_list(host_node ** empty_head) {
    host_node * head = NULL;
    head = (host_node *) malloc(sizeof(host_node));
    if (head == NULL)
        return 1;

    head->next = NULL;
    head->host = NULL;

    *empty_head = head;
    return 0;
}

int push_host(host_node * head, ClientThreadData * host) {
    host_node * new_host = NULL;
    new_host = (host_node *) malloc(sizeof(host_node));
    if (new_host == NULL)
        return 1;

    new_host->host = host;
    new_host->next = head->next;
    head->next = new_host;
    return 0;
}

void pop_host(host_node * prev) {
    host_node * host_to_pop = prev->next;
    prev->next = host_to_pop->next;
    pthread_cond_destroy(&(host_to_pop->host->command_condition));
    pthread_mutex_destroy(&(host_to_pop->host->command_mutex));
    free(host_to_pop->host);
    free(host_to_pop);
}

void pop_host_by_id(host_node * head, int host_id) {
    host_node * prev = head;
    while (prev->next != NULL && prev->next->host->host_id != host_id)
        prev = prev->next;
    if (prev->next != NULL)
        pop_host(prev);
}

void free_host_list(host_node * head) {
    while (head->next != NULL)
        pop_host(head);
    free(head);
}

void close_connection(host_node * node){
    send_command_to_server(CMD_CLOSE_CONNECTION, NULL, NULL, node->host);
    pthread_join(node->host->interact_server_thread, NULL);
}

void close_all_connections(host_node * head){
    while(head->next != NULL){
        head = head->next;
        close_connection(head);
    }
}

host_node * get_host_by_id(host_node * head, int host_id) {
    host_node * prev = head;
    while (prev->next != NULL && prev->next->host->host_id != host_id)
        prev = prev->next;
    return prev->next;
}