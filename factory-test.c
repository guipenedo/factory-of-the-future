#include "network/tcp.h"
#include "interfaces/network_commands.h"
#include "interfaces/peripherals_network.h"
#include "utils/host_list.h"
#include "network/connection.h"
#include "utils/sensor_history.h"

host_node * host_list;
pthread_t server_thread;
sensor_history_buffer * sensor_history;

int fact_ID = -1;
char cmd_args[MAX_ARGS_BUFFER_SIZE];

void trigger_alarm(int fact_id_alarm) {
    // TODO
}

void handle_command(int commandId, char * args, char * response, char * client_ip) {
    if (commandId == CMD_ANNOUNCE_NEW_HOST) {
        connect_new_factory(args, host_list);
    } else if (commandId == CMD_SEND_SENSOR_DATA) {
        int factId;
        double temperature, humidity, pressure;
        sscanf(args, "%d %lf %lf %lf", &factId, &temperature, &humidity, &pressure);
        printf("Sensor data from fact_ID=%d: T=%lf H=%lf P=%lf\n", factId, temperature, humidity, pressure);
    } else if (commandId == CMD_TRIGGER_ALARM) {
        int fact_id_alarm;
        sscanf(args, "%d", &fact_id_alarm);
        printf("[!] Alarm triggered because of factory ID %d\n.", fact_id_alarm);
        trigger_alarm(fact_id_alarm);
    } else if (commandId == CMD_GET_PERIPHERALS) {
        sprintf(response, "%d %d %d", has_sensors(), has_led(), has_relay());
    } else if (commandId == CMD_SET_LED_STATE) {
        short state;
        sscanf(args, "%hd", &state);
        set_led_state(state);
    } else if (commandId == CMD_SET_RELAY_STATE) {
        short state;
        sscanf(args, "%hd", &state);
        set_relay_state(state);
    }
}

void broadcast_sensor_data(SensorData sensorData) {
    sprintf(cmd_args, "%d %lf %lf %lf", fact_ID, sensorData.temperature, sensorData.humidity, sensorData.pressure);
    host_node * host = host_list;
    while (host->next != NULL){
        host = host->next;
        send_command_to_server(CMD_SEND_SENSOR_DATA, cmd_args, NULL, host->host);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Factory takes exactly 1 command line argument: the dashboard's IP address\n");
        exit(-1);
    }

    // start tcp server
    accept_tcp_connections_non_blocking(handle_command, &server_thread);

    // connect to the dashboard
    const char * dashboardAddr = argv[1];
    printf("Starting factory! Dashboard IP address: %s\n", dashboardAddr);

    connect_to_dashboard(dashboardAddr, &host_list, &fact_ID, 1);
    init_sensor_data_buffer(&sensor_history);
    init_sensor();

    
    
    int i=0;
    for(i=0;i<10;i++) {
        SensorData sensorData;
        read_sensor_data(&sensorData);
        store_sensor_data(sensor_history, sensorData);
        broadcast_sensor_data(sensorData);
        
        int stat=0;
        
        if (has_sensors()==1) printf("sensors ready\n");
        else printf("can't detect sensors\n");
        if (has_led()==1) printf("led ready\n");
        else printf("can't detect led\n");
        if (has_relay()==1) printf("relay ready\n");
        else printf("can't detect relay\n");
       
        if (stat==0)
            stat=1;
        else
            stat=0;
        set_led_state(stat);
        set_relay_state(stat);
        if (i==5)
            trigger_factory_alarm(5);
           
        sleep(MEASUREMENT_PERIOD);
    }

    write_sensor_data_to_file(sensor_history, 1);
    close_all_connections(host_list);
    free_host_list(host_list);
    free_sensor_data_buffer(&sensor_history);

    return 0;
}

