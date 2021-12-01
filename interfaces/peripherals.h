#ifndef FACTORY_OF_THE_FUTURE_PERIPHERALS_H
#define FACTORY_OF_THE_FUTURE_PERIPHERALS_H

#include <time.h>

typedef struct SensorData {
    time_t time;
    double temperature, humidity, pressure;
} SensorData;

// this should return 1 if the sensors are available and 0 otherwise
short has_sensors();

// this should return 1 if the led is available and 0 otherwise
short has_led();

// this should return 1 if the relay is available and 0 otherwise
short has_relay();

// this function should edit the SensorData's argument values to those read by the sensor
void init_sensor();
void read_sensor_data(SensorData *);

// this function takes as argument the desired led state
void set_led_state(short state);

// this function takes as argument the desired relay state
void set_relay_state(short state);

void trigger_factory_alarm(int, short *);

#endif //FACTORY_OF_THE_FUTURE_PERIPHERALS_H
