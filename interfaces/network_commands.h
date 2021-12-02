#ifndef FACTORY_OF_THE_FUTURE_NETWORK_COMMANDS_H
#define FACTORY_OF_THE_FUTURE_NETWORK_COMMANDS_H

#define CMD_CLOSE_CONNECTION            (-1)
#define CMD_INIT_NEW_FACTORY            0  // (IP) -> (ID)
#define CMD_ANNOUNCE_NEW_HOST           1  // (IP, ID) -> OK
#define CMD_GET_PERIPHERALS             2  // () -> (SENSORS), (LED), (RELAY)
#define CMD_SEND_SENSOR_DATA            3  // (ID, TEMPERATURE, HUMIDITY, PRESSURE) -> OK
#define CMD_TRIGGER_ALARM               4  // (ID) -> OK
#define CMD_INIT_ML                     5  // (IP) -> (ID)
#define CMD_SEND_SENSOR_HISTORY_FILE    6  // (no parameters) -> file stream
#define CMD_SET_LED_STATE               7  // state -> OK
#define CMD_SET_RELAY_STATE             8  // state -> OK
#define CMD_GET_ALARM_STATE             9  // () -> 0 or 1
#define CMD_PREDICT                     10 // (ID, TIME) -> float

#endif //FACTORY_OF_THE_FUTURE_NETWORK_COMMANDS_H