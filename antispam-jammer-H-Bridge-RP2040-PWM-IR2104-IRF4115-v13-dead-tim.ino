/*
    Wearable microphone jammer - Vowel Formant Modulation Version (Strict 24kHz-26kHz)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
    (C) Adam Loboda 2026
    version 13 - FM random modulation (sinusoidal wave)  for bobble sounds
*/

#include <Arduino.h>
#include "hardware/pwm.h"  
#include "hardware/clocks.h"

const int PIN_NORMAL = 0;    // GP0
const int PIN_INVERTED = 1;  // GP1
uint slice_num;


// Middle of piezzo transducers resonance frequency
const uint32_t BASE_CARRIER = 25000; 

void update_frequency_safe(uint32_t freq) {
    uint32_t sys_clk = clock_get_hz(clk_sys);
    
    uint32_t wrap = sys_clk / (2 * freq);
    pwm_set_wrap(slice_num, wrap);

    // set the PWM duty level
    uint32_t level = (wrap / 2.1); 
 
    pwm_set_chan_level(slice_num, PWM_CHAN_A, level);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, wrap - level);
}

void setup() {
    gpio_set_function(PIN_NORMAL, GPIO_FUNC_PWM);
    gpio_set_function(PIN_INVERTED, GPIO_FUNC_PWM);
  
    slice_num = pwm_gpio_to_slice_num(PIN_NORMAL);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_phase_correct(&config, true);
    pwm_config_set_output_polarity(&config, false, true); 
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_init(slice_num, &config, true);

    update_frequency_safe(BASE_CARRIER);
    randomSeed(1234);
}

void loop() {
    // Here we pick a random INFRASOUND frequency for FM modulation
    uint16_t randomfrequency = random(2,25);

    // Here we pick random time of this frequency duration
    // you may play with this range to have better jamming results on phones mic
    unsigned long randomfrequencyDuration = random(1, 7);

    // note the start of the cycle
    unsigned long startTime = millis();

    while (millis() - startTime < randomfrequencyDuration) {
        
        // FM modulation to create the sound upon bearer piezo frequency 
        // Weight 1000 adjusted to fit within piezo acceptable frequency  
        // piezo range is  24000 - 26000 Hz (25000 +/- 1000 Hz)
        float timeSec = millis() / 1000.0;
        
        long fm_offset = (sin(timeSec * randomfrequency) * 1000);
        uint32_t current_freq = BASE_CARRIER + fm_offset;

        // Update the frequency
        update_frequency_safe(current_freq);

        // time for 1 PWM cycle 
        delayMicroseconds(40);
    }
}
