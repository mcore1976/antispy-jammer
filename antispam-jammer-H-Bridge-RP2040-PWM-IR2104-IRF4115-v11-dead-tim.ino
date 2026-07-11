/*
    Wearable microphone jammer - H-Bridge Version (Dual Complementary Output)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
  Uses : RP2040, IR2104 H-bridge controller chip, UF Shottky diodes and IRFZ44N Mosfets
  Version 11: Dead-time implementation for IRFZ44N + IR2104 + FM/AM modulation randomization for best results
  (C) Adam Loboda 2026
*/

#include <Arduino.h>
#include "hardware/pwm.h"  
#include "hardware/clocks.h"

// We are using GP0 and GP1 pins, they both belong to WM Slice 0 
// and can form the H-bridge
const int PIN_NORMAL = 0;    // GP0
const int PIN_INVERTED = 1;  // GP1 is Negated GP0

// duty used to simulate human speech in AM modulation
uint16_t duty = 10;

uint slice_num;

void update_frequency_safe(uint32_t freq, uint16_t cycle_duty) {
    uint32_t sys_clk = clock_get_hz(clk_sys);
    
    // Phase Correct: freq = sys_clk / (2 * wrap)
    // Clock 125MHz & 25kHz: wrap = 2500
    uint32_t wrap = sys_clk / (2 * freq);
    pwm_set_wrap(slice_num, wrap);


    // DEAD TIME will last 50% of full cycle and will ensure that 
    // LC circuit transducers + coil unload the high voltage 
    // Channel A will be ON for 0.25 * wrap 
    // Channel B will be ON for 0.25 * wrap 
    // uint32_t level = wrap / 8;   // for 12,5%
    // uint32_t level = wrap / 2.1; // previous setting 
    
    // Randomizing CYCLE DUTY for AM modulation
    uint32_t level = ( wrap /  2 ) + cycle_duty ; 
 
    // Channel A: setting wrap level 
    pwm_set_chan_level(slice_num, PWM_CHAN_A, level);
    
    // Channel B: setting wrap level (negative setup of channels)
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
                              
    // Inversion of channel B (GP1) so they will not be both High witch channel A
    pwm_config_set_output_polarity(&config, false, true);     // this may be pwm_config_set_output_inverting in older libs
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_init(slice_num, &config, true);

    update_frequency_safe(25350, duty);
    randomSeed(1234);
}

void loop() {
    uint16_t i, j, x, z;

    // Swiping 26kHz -> 24kHz
    x = random(20, 50);
    j = 1;
    i = 25350;
 
    
    while (i >= 24650 ) {
        i = i - j;
        update_frequency_safe(i, duty);
        
        //  FM modulation
        j = (j + 1 ) % x;
        if (j == 0) j = 1;

        // adding random stops to the frequency swing
        if ( (i % random(2,10) ) == 0 )   delayMicroseconds(random(5,75));

        // adding random duty changes every N frequency
        if ( (i % random(2,5) ) == 0 ) duty = random (10,250);
      

        // when delay less than 50usec the jammer becomes audible but jams better
        delayMicroseconds(random(35,45));
    }

    // Swiping up 24kHz -> 26kHz
    x = random(20, 50);
    j = 1;
    i = 24650;

    while (i <= 25350 ) {
        i = i + j;
        update_frequency_safe(i, duty);
        
        //  FM modulation
        j = (j + random(1,5) ) % x;
        if (j == 0) j = 1;

        // adding random stops to the frequency swing
        if ( (i % random(2,10) ) == 0 )   delayMicroseconds(random(5,75));

        // adding random duty changes every N frequency
        if ( (i % random(2,5) ) == 0 ) duty = random (10,250);

        // when delay less than 50usec the jammer becomes audible but jams better
        delayMicroseconds(random(35,45));
    }
}
