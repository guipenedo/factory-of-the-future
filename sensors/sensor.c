// Main Include
#include "bme280.h"
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

// Threading Include
#include <pthread.h>

// IIC Include
#include <string.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

// Define types
void typeInt(int i);
void typeFloat(float myFloat);
void typeln(const char *s);
void typeChar(char val);

// Define I2C protocols
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

// Define data saver
void save_sensor_data(struct bme280_data *comp_data, char* buffer) {
    sprintf(buffer, "%f %f %f", comp_data->temperature, comp_data->pressure, comp_data->humidity);
}

// Define data fetcher
void read_sensor_data(struct bme280_dev *dev, char* buffer)
{
    int8_t rslt;
    uint8_t settings_sel;
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;
    dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel = BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_STANDBY_SEL;
    settings_sel |= BME280_FILTER_SEL;
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

    /* Delay while the sensor completes a measurement */
    dev->delay_ms(70);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    save_sensor_data(&comp_data, &buffer);
}


// float to string
void typeFloat(float myFloat)   {
    char buffer[20];
    sprintf(buffer, "%4.2f",  myFloat);
    typeln(buffer);
}

// int to string
void typeInt(int i)   {
    char array1[20];
    sprintf(array1, "%d",  i);
    typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void)   {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line)   {
    lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)   {

    lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char *s)   {

    while ( *s ) lcd_byte(*(s++), LCD_CHR);

}

int main(int argc, char* argv[])
{
    if(wiringPiSetup() < 0)
    {
        return 1;
    }

    pinMode (27,OUTPUT) ;

    SPI_BME280_CS_Low();//once pull down means use SPI Interface

    wiringPiSPISetup(channel,2000000);

    struct bme280_dev dev;
    int8_t rslt = BME280_OK;

    dev.dev_id = 0;
    dev.intf = BME280_SPI_INTF;
    dev.read = user_spi_read;
    dev.write = user_spi_write;
    dev.delay_ms = user_delay_ms;

    char[50] buffer;
    rslt = bme280_init(&dev);
    get_sensor_data(&dev, &buffer);
}