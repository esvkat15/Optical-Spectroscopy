#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

// gpio pins
#define RED_LED             6
#define GREEN_LED           7
#define BLUE_LED            8
#define WHITE_LED           9
#define CURRENT_SRC         10
#define DISCHARGE_SW        11
#define VOLTAGE_SW          12
#define VOLTAGE_ADJ_PIN     13

// adc channels and pins
#define ADC_VSHUNT          0
#define ADC_VBATT           1
#define VSHUNT_PIN          26
#define VBATT_PIN           27

// i2c addresses
#define i2c_cam_addr        0x3C
#define i2c_discharge0_addr 0x2C
#define i2c_discharge1_addr 0x2D
#define i2c_charge1_addr    0x2E
#define i2c_charge2_addr    0x2F

// spi registers
#define spi_write_bit       0x80
#define spi_reg_test        0x00
#define spi_reg_version     0x40

// initialization for hardware
void hw_setup();

// ping usb until acknowledged
void handshake();

/* spi camera functions */
// chip select
void spi_cam_select() { gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0); }
void spi_cam_unselect() { gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1); }

// write to an arducam register over spi
void spi_cam_write(uint8_t reg, uint8_t val);

// read from an arducam register over spi
uint8_t spi_cam_read(uint8_t reg);

// read arducam fifo into buffer, returns -1 on fail
int spi_cam_read_burst(uint8_t* buf, size_t len);

int main() {

    // initialize hardware
    hw_setup();

    while(1) {
        
        handshake();
        while(1) {

            char command[100];
            absolute_time_t t = make_timeout_time_ms(60 * 60 * 1000);
            sleep_ms(50);
            int r = stdio_get_until(command, 100, t);
            if(r != PICO_ERROR_TIMEOUT) {

                command[r - 1] = '\0';
                if(!strcmp(command, "test")) {

                    printf("Hello World!\n");

                }
                if(!strcmp(command, "spi_test")) {

                    // write 0xAA to register 0x00
                    uint8_t x = spi_cam_read(spi_reg_test);
                    spi_cam_write(spi_reg_test, 0x55);
                    uint8_t y = spi_cam_read(spi_reg_test);
                    printf("test reg before: 0x%02hhX, after write: 0x%02hhX\n", x, y);

                }

            }
            /*sleep_ms(1000);
            gpio_put(6, 1);
            sleep_ms(1000);
            gpio_put(6, 0);*/

        }

    }
    return 0;

}

// write to an arducam register over spi
void spi_cam_write(uint8_t reg, uint8_t val) {

    uint8_t buf[2];
    buf[0] = reg | spi_write_bit;
    buf[1] = val;
    spi_cam_select();
    spi_write_blocking(spi_default, buf, 2);
    spi_cam_unselect();
    sleep_ms(1);

}

// read from an arducam register over spi
uint8_t spi_cam_read(uint8_t reg) {

    uint8_t val = 0;
    reg &= ~spi_write_bit;
    spi_cam_select();
    spi_write_blocking(spi_default, &reg, 1);
    spi_read_blocking(spi_default, 0, &val, 1);
    spi_cam_unselect();
    return val;

}

// read arducam fifo into buffer, returns -1 on fail
int spi_cam_read_burst(uint8_t* buf, size_t len) {return -1;}

// ping usb until acknowledged
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
    //setup_default_uart();

    // init gpio
    // gp6 -> Red LED (Active HIGH)
    // gp7 -> Green LED (Active HIGH)
    // gp8 -> Blue LED (Active HIGH)
    // gp9 -> White LED (Active HIGH)
    // gp10 -> Constant Current Source (Active HIGH)
    // gp11 -> Discharge Switch (Active HIGH)
    // gp12 -> Constant Voltage Switch (Active LOW)
    // gp13 -> Constant Voltage Adjust Pin (Active LOW)
    for(int i = 6; i < 14; i++) {

        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        if(i < VOLTAGE_SW)

            gpio_put(i, 0);

    }
    gpio_put(VOLTAGE_SW, 1);
    gpio_put(VOLTAGE_ADJ_PIN, 1);

    // init adc
    // adc0 -> voltage across shunt resistor
    // adc1 -> voltage at positive node of battery
    adc_init();
    adc_gpio_init(VSHUNT_PIN);
    adc_gpio_init(VBATT_PIN);

    // init i2c
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // init spi
    // Enable SPI 0 at 4 MHz and connect to GPIOs
    spi_init(spi_default, 4 * 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    //gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    //gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

}