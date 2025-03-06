#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

int main() {
    stdio_init_all();
    gpio_init(16);
    gpio_set_dir(16, GPIO_OUT);
    //setup_default_uart();
    while(1) {
        
        printf("Hello, world! 2\n");
        sleep_ms(1000);
        gpio_put(16, 1);
        sleep_ms(1000);
        gpio_put(16, 0);

    }
    return 0;
}