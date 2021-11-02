/* gcc -o led led.c -lwiringPi */
#include <stdio.h>
#include <wiringPi.h>
#pragma ide diagnostic ignored "EndlessLoop"

int relay_control(short state){
    if (!digitalWrite(RELAY_PIN, state)){
        printf("Cannot write to LED")
        return -1;
    }
    return 0;
}

int main(void) {
    wiringPiSetup();
    pinMode(RELAY_PIN, OUTPUT);

    // Todo: Add waiting for TCP signal to control relay
    return 0;
}
