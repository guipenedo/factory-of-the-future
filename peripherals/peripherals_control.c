/*
  This is tested on Raspberry PI 3B+
  The default Address is 0X77
*/
#include "../interfaces/peripherals.h"
#include "bme280.h"
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <time.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
//Raspberry 3B+ platform's default I2C device file
#define IIC_Dev  "/dev/i2c-1"

#define RELAY 0
#define LED  1
#define buzzer  21
#define led  22

pthread_t alarm_thread;

//////////////////////////////////////////////
short sensor_connected = 0;

short has_sensors(){
    init_sensor(); //re initialize to check if sensor is conected
    return sensor_connected;
}

short has_led(){
    short s;
    short flag = 1; //assume its conected
    pinMode (LED, INPUT);
    pullUpDnControl(LED,PUD_OFF);
    for (int i = 0; i < 10; i++){
        delay(1);
        s = digitalRead(LED); // when its 0, its connected
        if (s != 0){ //an unconected will be changing between 1 & 0
            flag = 0;
            break;
        }
    }
    return flag;
}

short has_relay(){
    short s;
    short flag = 1; //assume its conected
    pinMode (RELAY, INPUT);
    pullUpDnControl(RELAY,PUD_OFF);
    for (int i = 0; i < 10; i++){
        delay(1);
        s = digitalRead(RELAY); // when its 1, its connected
        if (s != 1){ //an unconected will be changing between 1 & 0
            flag = 0;
            break;
        }
    }
    return flag;
}
//////////////////////////////////////////////
void set_led_state(short state)
{
    pinMode (LED, OUTPUT);
    if(state==0)
        digitalWrite (LED, LOW);
    else
        digitalWrite(LED,HIGH);
}
//////////////////////////////////////////////
#define RELAY 0

void set_relay_state(short state)
{
    pinMode (RELAY, OUTPUT);
    if(state==0)
        digitalWrite (RELAY, LOW);
    else
        digitalWrite(RELAY, HIGH);
}
//////////////////////////////////////////////

int fd;

void user_delay_ms(uint32_t period)
{
    usleep(period*1000);
}

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    write(fd, &reg_addr,1);
    read(fd, data, len);
    return 0;
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int8_t *buf;
    buf = malloc(len +1);
    buf[0] = reg_addr;
    memcpy(buf +1, data, len);
    write(fd, buf, len +1);
    free(buf);
    return 0;
}

struct bme280_dev dev;

void init_sensor(){
    // init actuators
    wiringPiSetup () ;
    int8_t rslt = BME280_OK;
    // init sensors
    if ((fd = open(IIC_Dev, O_RDWR)) < 0) {
        printf("Failed to open the i2c bus. \n");
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, 0x77) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
    //dev.dev_id = BME280_I2C_ADDR_PRIM;//0x76
    dev.dev_id = BME280_I2C_ADDR_SEC; //0x77
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_ms = user_delay_ms;

    rslt = bme280_init(&dev);
    //printf("\r\n BME280 Init Result is:%d \r\n",rslt);

    if (rslt==0) sensor_connected=1;
    else return;

    //////stream_sensor_data_normal_mode(&dev);

    uint8_t settings_sel;

    /* Recommended mode of operation: Indoor navigation */
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;
    dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel = BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_STANDBY_SEL;
    settings_sel |= BME280_FILTER_SEL;
    bme280_set_sensor_settings(settings_sel, &dev);
    bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
}

void read_sensor_data(SensorData *data)
{
    struct bme280_data comp_data;
    bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

    time_t measurement_time;
    time(&measurement_time); //Get the time in sec since UTC 1/1/70.

    data->time = measurement_time;
    data->temperature = comp_data.temperature;
    data->humidity = comp_data.humidity;
    data->pressure = comp_data.pressure;
}

void run_alarm(short * alarm_state){
    *alarm_state = 1;
    pinMode(buzzer, OUTPUT);
    pinMode(led, OUTPUT);
    int i = 0;
    for (; i<75; i++) { // 75*0.8 = 60 s
        digitalWrite(buzzer, HIGH);  // On
        digitalWrite(led, HIGH);
        delay(200);               // mS
        digitalWrite(buzzer, LOW);   // Off
        digitalWrite(led, LOW);
        delay(500);
    }
    *alarm_state = 0;
}

void trigger_factory_alarm(int factID, short * alarm_state)
{
    pthread_create(&alarm_thread, NULL, (void * (*) (void *)) run_alarm, alarm_state);
}


