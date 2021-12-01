#include "../interfaces/peripherals.h"
#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////
#define LED  1
int i = 0;

short has_sensors() {
    return 1;
}

short has_led() {
    return 1;
}

short has_relay() {
    return 1;
}

void set_led_state(short state)
{
    printf("Changed led state to %d\n", state);
}
//////////////////////////////////////////////
#define RELAY 0

void set_relay_state(short state)
{
    printf("Changed relay state to %d\n", state);
}
//////////////////////////////////////////////

void init_sensor(){
    printf("Init sensors\n");
}

void read_sensor_data(SensorData *data)
{
    time_t measurement_time;
    time(&measurement_time); //Get the time in sec since UTC 1/1/70.

    data->time = measurement_time;
    data->temperature = 21.43 + i * 0.01;
    data->humidity = 84.2 + i * 0.01;
    data->pressure = 1.423 + i * 0.01;
    i++;
}

void trigger_factory_alarm(int factID, short * trigger_factory_alarm)
{
    printf("Alarm triggered");
}