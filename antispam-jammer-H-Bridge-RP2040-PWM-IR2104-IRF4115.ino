/*
    Wearable microphone jammer - H-Bridge Version (Dual Complementary Output)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
	Uses : RP2040, IR2104 H-bridge controller chip, UF Shottky diodes and IRFZ44N Mosfets
	(C) Adam Loboda 2026
*/

#include <Arduino.h>
#include "hardware/pwm.h"  
#include "hardware/clocks.h"

// We are using GP0 and GP1 pins, they both belong to WM Slice 0 and can be used in H-bridge
const int PIN_NORMAL = 0;    // GP0
const int PIN_INVERTED = 1;  // GP1 is Negated GP0

uint slice_num;

void update_frequency_safe(uint32_t freq) {
    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = (sys_clk / freq) - 1;

    // Setting PWM wrap for the whole slice
    pwm_set_wrap(slice_num, wrap);
   
    // We are using 50% PWM cycle duty for both channels A & B
    // Channel B will have automatically reverted polarity
    uint32_t level = (wrap + 1) / 2;
    pwm_set_chan_level(slice_num, PWM_CHAN_A, level);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, level);
}

void setup() {
    // Initializing GP0/GP1 as PWM outputs
    gpio_set_function(PIN_NORMAL, GPIO_FUNC_PWM);
    gpio_set_function(PIN_INVERTED, GPIO_FUNC_PWM);
  
    slice_num = pwm_gpio_to_slice_num(PIN_NORMAL);

    // Polarity configuration - we can swap channel A with B if needed
    // pwm_set_output_polarity(slice, invert_A, invert_B)
    // If PIN_NORMAL is GP1 (Channel B), and PIN_INVERTED is GP0 (Channel A):
    pwm_set_output_polarity(slice_num, true, false);
    // Now Channel A (GP0) is inverted channel B (GP1)

    // Default configuration 
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_init(slice_num, &config, true);

    update_frequency_safe(26000);
    randomSeed(1234);
}

void loop() {
    uint16_t i, j, x;

    // Swiping 26kHz -> 24kHz
    x = random(20, 50);
    j = 1;
    i = 26000;

    while (i >= 24000) {
        i = i - j;
        update_frequency_safe(i);
        j = (j + 1) % x;
        if (j == 0) j = 1;
        delayMicroseconds(140);
    }

    // Swiping up 24kHz -> 26kHz
    x = random(20, 50);
    j = 1;
    i = 24000;

    while (i <= 26000) {
        i = i + j;
        update_frequency_safe(i);
        j = (j + 1) % x;
        if (j == 0) j = 1;
        delayMicroseconds(140);
    }
}