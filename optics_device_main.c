#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

// gpio pins
#define RED_LED 6
#define GREEN_LED 7
#define BLUE_LED 8
#define WHITE_LED 9
#define CURRENT_SRC 10
#define DISCHARGE_SW 11
#define VOLTAGE_SW 12
#define VOLTAGE_ADJ_PIN 13

// initialization for hardware
void hw_setup();

// ping usb until handshake recieved
void handshake();

int main() {

    // initialize hardware
    hw_setup();

    //setup_default_uart();
    while(1) {
        
        handshake();
        char command[100];
        absolute_time_t t = make_timeout_time_ms(60 * 60 * 1000);
        sleep_ms(50);
        int r = stdio_get_until(command, 100, t);
        if(r != PICO_ERROR_TIMEOUT) {

            command[r - 1] = '\0';
            if(!strcmp(command, "test")) {

                printf("Hello World!\n");

            }

        }
        /*sleep_ms(1000);
        gpio_put(6, 1);
        sleep_ms(1000);
        gpio_put(6, 0);*/

    }
    return 0;

}

// ping usb until handshake complete
void handshake() {

    while(1) {

        printf("picoping\n");
        char buf[8];
        absolute_time_t t = make_timeout_time_ms(3 * 1000);
        sleep_ms(50);
        int r = stdio_get_until(buf, 8, t);
        if(r != PICO_ERROR_TIMEOUT) {
            
            buf[7] = '\0';
            if(!strcmp(buf, "picoack")) {

                printf("picoack\n");
                break;

            }

        }

    }

}

// initialization for hardware
void hw_setup() {

    // init usb
    stdio_init_all();

    // init gpio
    // gp6 -> Red LED (Active HIGH)
    // gp7 -> Green LED (Active HIGH)
    // gp8 -> Blue LED (Active HIGH)
    // gp9 -> White LED (Active HIGH)
    // gp10 -> Constant Current Source (Active HIGH)
    // gp11 -> Discharge Switch (Active HIGH)
    // gp12 -> Constant Voltage Switch (Active LOW)
    // gp13 -> Constant Voltage Adjust Pin (Active ???)
    for(int i = 6; i < 14; i++) {

        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        if(i != VOLTAGE_SW)

            gpio_put(i, 0);

    }
    gpio_put(VOLTAGE_SW, 1);

    // init adc
    // TODO
    // init i2c
    // TODO

    // init spi
    // Enable SPI 0 at 8 MHz and connect to GPIOs
    spi_init(spi_default, 8 * 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

}