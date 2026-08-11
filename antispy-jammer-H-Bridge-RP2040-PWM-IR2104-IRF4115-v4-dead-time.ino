/*
    Wearable microphone jammer - H-Bridge Version (Dual Complementary Output)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
  Uses : RP2040, IR2104 H-bridge controller chip, UF Shottky diodes and IRFZ44N Mosfets
  Version 3: Dead-time implementation for IRFZ44N + IR2104, PINK NOISE generation for modulation
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

    // DEAD TIME will last 50% of full cycle and will ensure that 
    // LC circuit transducers + coil unload the high voltage 
    // Channel A will be ON for 0.25 * wrap 
    // Channel B will be ON for 0.25 * wrap 
    //uint32_t level = wrap / 8;   // for 12,5%
    uint32_t level = wrap / 2.1; 
 
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

    update_frequency_safe(26000);
    randomSeed(1234);
}

// Implementing Vossa-McCartney algorythm for pink noise generation
// 5 random generators to achieve decrease of -3dB/octave

#define PINK_ROWS 5

uint16_t pink_components[PINK_ROWS] = {0, 0, 0, 0, 0};
uint32_t pink_running_sum = 0;
uint8_t pink_index = 0;

uint16_t generate_pink_noise_value() {
    pink_index++;
    // Fractal method - counting how many zeros index is having
    uint8_t count = __builtin_ctz(pink_index);
    if (count >= PINK_ROWS) count = PINK_ROWS - 1;

    // Only one of the components is random at a time
    pink_running_sum -= pink_components[count];
    pink_components[count] = random(0, 2000); // The range is adjusted to transducers bandwidth (2000 Hz)
    pink_running_sum += pink_components[count];

    // Normalizing the value withing 0 do 2000 range
    return (pink_running_sum / PINK_ROWS);
}


void loop() {

   // Random value from pink noise generator will be used as a base
    uint16_t pink_offset = generate_pink_noise_value();

    // Modululating 24KHz bearer with pink noise offset: 24000 Hz + (0 - 2000 Hz)
    // The target value is then : 24000 Hz - 26000 Hz
    uint32_t target_freq = 24000 + pink_offset;

    update_frequency_safe(target_freq);

    // You may play with this value
    delayMicroseconds(140);
 
}
