#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "peripherals/digipot.h"
#include "peripherals/leds.h"
#include "peripherals/camera.h"
#include "peripherals/cell_ctrl.h"

// gpio pins
#define RED_LED             6
#define GREEN_LED           7
#define BLUE_LED            8
#define WHITE_LED           9
#define CURRENT_SRC         10
#define DISCHARGE_SW        11
#define VOLTAGE_SW          12
#define VOLTAGE_ADJ_PIN     13

// led on/off states
uint red_level, green_level, blue_level, white_level;

// adc channels and pins
#define ADC_VSHUNT          0
#define ADC_VBATT           1
#define VSHUNT_PIN          26
#define VBATT_PIN           27

// i2c addresses
#define i2c_cam_addr        0x3C

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

// callbacks
bool led_callback(__unused struct repeating_timer *t) {

    white_level = 100 - white_level;
    pwm_set_gpio_level(WHITE_LED, white_level);
    return true;

}

int main() {

    // initialize hardware
    hw_setup();

    while(1) {
        
        handshake();
        while(1) {

            char command[100];
            absolute_time_t t = make_timeout_time_ms(15 * 1000);
            sleep_ms(50);
            int r = stdio_get_until(command, 100, t);
            if(r != PICO_ERROR_TIMEOUT) {

                command[r - 1] = '\0';
                if(!strcmp(command, "test")) {

                    printf("Hello World!\n");

                } else if(!strcmp(command, "led_test")) {

                    struct repeating_timer timer;
                    int r = add_repeating_timer_ms(2000, led_callback, NULL, &timer);
                    if(r) {

                        printf("led is on\n");

                    } else {

                        printf("led timer failed\n");

                    }
                    // bool cancelled = cancel_repeating_timer(&timer);

                } else if(!strcmp(command, "adc_test")) {

                    adc_select_input(0);
                    uint16_t result0 = adc_read();
                    adc_select_input(1);
                    uint16_t result1 = adc_read();
                    printf("adc0: 0x%03hX, adc1: 0x%03hX\n", result0, result1);

                } else if(!strcmp(command, "spi_test")) {

                    // write 0xAA to register 0x00
                    uint8_t x = spi_cam_read(spi_reg_test);
                    spi_cam_write(spi_reg_test, 0x55);
                    uint8_t y = spi_cam_read(spi_reg_test);
                    printf("test reg before: 0x%02hhX, after write: 0x%02hhX\n", x, y);

                } else if(!strcmp(command, "i2c_test")) {

                    // todo
                    adc_select_input(1);
                    int ret = digipot_w(i2c_discharge0_bits, 0x01, 0x00);
                    if(ret) {
                        printf("|write1 failed|");
                    }
                    sleep_ms(500);
                    uint16_t result = adc_read();
                    sleep_ms(500);
                    printf("read1: 0x%03hX, ", result);
                    ret = digipot_w(i2c_discharge0_bits, 0x01, 0x3F);
                    if(ret) {
                        printf("|write2 failed|");
                    }
                    sleep_ms(500);
                    result = adc_read();
                    sleep_ms(500);
                    printf("read2: 0x%03hX, ", result);
                    ret = digipot_w(i2c_discharge0_bits, 0x01, 0x3F >> 1);
                    if(ret) {
                        printf("|write3 failed|");
                    }
                    sleep_ms(500);
                    result = adc_read();
                    sleep_ms(500);
                    printf("read3: 0x%03hX\n", result);

                } else if(!strcmp(command, "read_image")) {

                    //

                } else {

                    printf("unrecognized command\n");

                }

            } else break;

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
    for(int i = 6; i < 10; i++) {

        // enable pwm
        uint pwm_slice = pwm_gpio_to_slice_num(i);
        uint pwm_chan = pwm_gpio_to_channel(i);
        pwm_set_wrap(pwm_slice, 255);
        pwm_set_chan_level(pwm_slice, pwm_chan, 0);
        pwm_set_enabled(pwm_slice, true);

    }
    red_level = 0;
    green_level = 0;
    blue_level = 0;
    white_level = 0;

    // gp10 -> Constant Current Source (Active HIGH)
    // gp11 -> Discharge Switch (Active HIGH)
    // gp12 -> Constant Voltage Switch (Active LOW)
    // gp13 -> Constant Voltage Adjust Pin (Active LOW)
    for(int i = 10; i < 14; i++) {

        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        if(i < VOLTAGE_SW) {

            gpio_put(i, 0);
            gpio_pull_down(i);

        }

    }
    gpio_put(VOLTAGE_SW, 1);
    gpio_pull_up(VOLTAGE_SW);
    gpio_put(VOLTAGE_ADJ_PIN, 1);
    gpio_pull_up(VOLTAGE_ADJ_PIN);

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