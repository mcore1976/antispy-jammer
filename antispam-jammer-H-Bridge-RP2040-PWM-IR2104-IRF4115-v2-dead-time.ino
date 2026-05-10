/*
    Wearable microphone jammer - H-Bridge Version (Dual Complementary Output)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
	Uses : RP2040, IR2104 H-bridge controller chip, UF Shottky diodes and IRFZ44N Mosfets
	Version 2: Dead-time implementation for IRFZ44N + IR2104
	(C) Adam Loboda 2026
*/

#include <Arduino.h>
#include "hardware/pwm.h"  
#include "hardware/clocks.h"

// We are using GP0 and GP1 pins, they both belong to WM Slice 0 
// and can form the H-bridge
const int PIN_NORMAL = 0;    // GP0
const int PIN_INVERTED = 1;  // GP1 is Negated GP0

uint slice_num;

void update_frequency_safe(uint32_t freq) {
    uint32_t sys_clk = clock_get_hz(clk_sys);
    
    // Phase Correct: freq = sys_clk / (2 * wrap)
    // Clock 125MHz & 25kHz: wrap = 2500
    uint32_t wrap = sys_clk / (2 * freq);
    pwm_set_wrap(slice_num, wrap);

    // DEAD TIME: 50 CPU cycles (400ns at 125MHz)
    // Safe margin for IRFZ44N (turn-off 150ns) + IR2104
    uint32_t dead_time_ticks = 50; 
    
    // Calculating wrap levels so they wont overlap 
    uint32_t level = (wrap / 2) - (dead_time_ticks / 2);

    // Channel A: High logic at lower part of the counter
    pwm_set_chan_level(slice_num, PWM_CHAN_A, level);
    
    // Channel B: High logic at upper part of the counter (negative setup of channels)
    pwm_set_chan_level(slice_num, PWM_CHAN_B, wrap - level);
}

void setup() {
    // Initializing GP0/GP1 as PWM outputs
    gpio_set_function(PIN_NORMAL, GPIO_FUNC_PWM);
    gpio_set_function(PIN_INVERTED, GPIO_FUNC_PWM);
  
    slice_num = pwm_gpio_to_slice_num(PIN_NORMAL);

    // Default configuration 
    pwm_config config = pwm_get_default_config();
	// Enabling Phase Correct mode for PWM (counter up-down)
    pwm_config_set_phase_correct(&config, true);
		 												  
    // Inversion of channel B (GP1) so they will not be both High wiych channel A
    pwm_config_set_output_inverting(&config, false, true); 
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