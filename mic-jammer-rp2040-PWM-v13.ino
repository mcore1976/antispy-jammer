/*
    wearable microphone jammer
  
    Version based on original concept &  development :
    https://github.com/y-x-c/wearable-microphone-jamming
  
    Boards needed : 
    Arduino Raspberry Pi Pico 2040 - RP2040-ZERO or RP2040-ONE from Waveshare
    +  TC4420 + MOSFET IRF4115

    Attention : Use Linux based PC and Arduino Pi Pico Earle Philhower Core for compilation
    Board Manager link for Arduino :
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

    This version uses bobble voice jamming technique
    
*/


#include <Arduino.h>
#include "hardware/pwm.h"   // Raspberry PICO SDK library
#include "hardware/clocks.h" // For function clock_get_hz

// pin #1 for GP1 on RP2040-zero board
const int PWM_PIN = 1;
uint slice_num;
uint channel;

// Middle of piezzo transducers resonance frequency
const uint32_t BASE_CARRIER = 25000; 

// function to gracefully reprogram PWM frequency without glitches
void update_frequency_safe(uint32_t freq) {
    uint32_t sys_clk = clock_get_hz(clk_sys);
   
    // Calculating wrap to reduce glitches while changing RP2040 PWM frequency
    // sys_clk / freq - 1
    uint32_t wrap = (sys_clk / freq) - 1;

    // This functions are from RP2040 SDK (double buffered)
    pwm_set_wrap(slice_num, wrap);
    // pwm_set_chan_level(slice_num, channel, (wrap + 1) / 2);

    // normally PWM cycle duty 35%-50% should give best result on LC generator
    // but setting up to 70% for highest voltage on the coil
    // the higher this value is the more DC current is pumped into coil by the MOSFET
    // both MOSFET and coil will get hotter 
    uint16_t level = (uint16_t)((wrap + 1) * 60 / 100);
    pwm_set_chan_level(slice_num, channel, level);
}

void setup() {
    // initializing RP2040 PWM pin from Pico SDK
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
   
    slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    channel = pwm_gpio_to_channel(PWM_PIN);

    // Configuration of the PWM defaults
    pwm_config config = pwm_get_default_config();

    // We want maximum precision of PWM we are not dividing the clock here
    pwm_config_set_clkdiv(&config, 1.0f); 
   
    // Starting PWM with defaults
    pwm_init(slice_num, &config, true);

    // Setting central frequency for piezo
    update_frequency_safe(25000);
   
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
