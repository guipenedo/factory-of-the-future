#include "peripherals_network.h"
#include "bme280.h"
#include <stdio.h>
#include <wiringPi.h>
int main(int argc, char* argv[])
{
  init_sensor();
  while(1){
    SensorData d;
    //read_sensor();
    read_sensor_data(&d);
    printf("temp:%0.2f*C   press:%0.2fhPa   humid:%0.2f%%\r\n",d.temperature, d.pressure/100, d.humidity);
    set_relay_state(0);
    set_led_state(0);
    delay(500);
    set_relay_state(1);
    set_led_state(1);
    delay(500);
    
  }
}
