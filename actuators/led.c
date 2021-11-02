/* gcc -o led led.c -lwiringPi */
#include <stdio.h>
#include <wiringPi.h>
#pragma ide diagnostic ignored "EndlessLoop"

int led_control(short state){
    if (!digitalWrite(LED_PIN, state)){
        printf("Cannot write to LED")
        return -1;
    }
    return 0;
}

int main(void) {
    wiringPiSetup();
    pinMode(LED_PIN, OUTPUT);
    // Todo: Add waiting for TCP signal to control led
    return 0;
}


