/*
    wearable microphone jammer
  
    Version based on original concept &  development :
    https://github.com/y-x-c/wearable-microphone-jamming
  
    Boards needed : 
    Arduino Raspberry Pi Pico 2040 - RP2040-ZERO or RP2040-ONE from Waveshare
    TC4420 + MOSFET IRF4115

    Attention : Use Linux based PC and Arduino Pi Pico Earle Philhower Core for compilation
    Board Manager link for Arduino :
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

    CONNECTION EXAMPLE FOR ARDUINO RP2040-ZERO
    --------
    
    GP1 PIN OF RP2040-ZERO  -->  INPUT PIN OF TC4420 MOSFET DRIVER

    ARDUINO HAS TO BE POWERED FROM 5V THROUGH LM7805 VOLTAGE STABILIZER OR BUCK-DOWN VOLTAGE CONVERTER        
    
*/


#include <Arduino.h>
#include "hardware/pwm.h"   // Raspberry PICO SDK library
#include "hardware/clocks.h" // For function clock_get_hz

// pin #1 for GP1 on RP2040-zero board
const int PWM_PIN = 1;
uint slice_num;
uint channel;

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
 
    // uint16_t level = (uint16_t)((wrap + 1) * 70 / 100);
    // 
    // adding SLIGTHLY randomized cycle duty in 65-70 percent range 
    // uint16_t level = (uint16_t)((wrap + 1) * random(50,55) / 100);
    uint16_t level = (uint16_t)((wrap + 1) * 50 / 100);
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

    // Starting swipe from 26KHz down
    update_frequency_safe(26000);
   
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
        update_frequency_safe(i);
        
        // adding  FM modulation
        j = (j + 1 ) % x;
        if (j == 0) j = 1;

        // adding random stops to the frequency swing
        if ( (i % random(2,10)) == 0 )   delayMicroseconds(random(5,75));

        // when delay less than 50usec the jammer becomes audible but jams better
        delayMicroseconds(75);        
    }


    delayMicroseconds(random(10,100));

    // Swiping up 24kHz -> 26kHz
    x = random(20, 50);
    j = 1;
    i = 24650;

    while (i <= 25350 ) {
        i = i + j;
        update_frequency_safe(i);
        
        // adding FM modulation
        j = (j + 1 ) % x;
        if (j == 0) j = 1;

        // adding random stops to the frequency swing
        if ( (i % random(2,10) ) == 0 )   delayMicroseconds(random(5,75));

        // when delay less than 50usec the jammer becomes audible but jams better
        delayMicroseconds(75);
    }

    delayMicroseconds(random(10,100));
    
}
