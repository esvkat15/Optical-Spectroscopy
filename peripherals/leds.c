#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include "leds.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// gpio pins
#define RED_LED             6
#define GREEN_LED           7
#define BLUE_LED            8
#define WHITE_LED           9

// max brightness desired / 255
#define max_brightness 15

// led pwm levels
uint red_level, green_level, blue_level, white_level;

// hue / luminosity shift states
uint hue_state, lum_state;

void init_leds() {

	// init leds
    // gp6 -> Red LED (Active HIGH)
    // gp7 -> Green LED (Active HIGH)
    // gp8 -> Blue LED (Active HIGH)
    // gp9 -> White LED (Active HIGH)
    for(int i = RED_LED; i <= WHITE_LED; i++) {

        // enable pwm
        gpio_set_function(i, GPIO_FUNC_PWM);
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

    hue_state = 0;
    lum_state = 0;

}

void toggle_w() {

	white_level = 50 - white_level;
    pwm_set_gpio_level(WHITE_LED, white_level);

}

void hue_shift() {

    hue_state = (hue_state + 1) % 256;
    lum_state = (lum_state + 1) % 10;
    red_level = max_brightness / 2 * cos(2 * M_PI * hue_state / 255) + max_brightness;
    green_level = max_brightness / 2 * cos(2 * M_PI * hue_state / 255 + 2 * M_PI / 3) + max_brightness;
    blue_level = max_brightness / 2 * cos(2 * M_PI * hue_state / 255 + 4 * M_PI / 3) + max_brightness; // blue correction
    white_level = max_brightness / 2 + sin(2 * M_PI * lum_state / 9) + max_brightness;
    if(white_level < max_brightness / 2) {

        red_level *= 2 * white_level / max_brightness;
        green_level *= 2 * white_level / max_brightness;
        blue_level *= 2 * white_level / max_brightness;
        white_level = 0;

    } else 

        white_level -= max_brightness / 2;

    pwm_set_gpio_level(RED_LED, red_level);
    pwm_set_gpio_level(GREEN_LED, green_level);
    pwm_set_gpio_level(BLUE_LED, blue_level * (max_brightness < 85 ? 2.4 : 1)); // blue correction
    pwm_set_gpio_level(WHITE_LED, white_level);

}