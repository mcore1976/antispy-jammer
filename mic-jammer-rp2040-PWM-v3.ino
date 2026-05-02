/*
    wearable microphone jammer
  
    Version based on original concept &  development :
    https://github.com/y-x-c/wearable-microphone-jamming
  
    Boards needed : 
    Arduino Raspberry Pi Pico 2040 - RP2040-ZERO or RP2040-ONE from Waveshare
    TPA3116D2 board - audio amplifier module + potentiometer
    OR TC4420 + MOSFET IRF4115

    Attention : Use Linux based PC and Arduino Pi Pico Earle Philhower Core for compilation
    Board Manager link for Arduino :
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

    CONNECTION EXAMPLE FOR ARDUINO RP2040-ZERO
    --------
    
    GP1 PIN OF RP2040-ZERO  --> LEFT AUDIO IN + RIGHT AUDIO IN   / or INPUT PIN OF TC4420 MOSFET DRIVER
    GND  --> AUDIO GND

    ARDUINO HAS TO BE POWERED FROM 5V THROUGH LM7805 VOLTAGE STABILIZER OR BUCK-DOWN VOLTAGE CONVERTER
    TPA3116 HAS TO BE CONNECTED TO 12V AND GND


    THIS VERSION USES PWM DUTY CYCLE LINEAR CHANGE TO GET ULTIMATE JAMMING RESULT
    
*/


#include <Arduino.h>
#include "hardware/pwm.h"   // Raspberry PICO SDK library
#include "hardware/clocks.h" // For function clock_get_hz

// pin #1 for GP1 on RP2040-zero board
const int PWM_PIN = 1;
uint slice_num;
uint channel;

// Procedure to update the FREQUENCY and DUTY of PWM signal from RP2040

void update_pwm_advanced(uint32_t freq, uint8_t duty_percent) {
  
    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = (sys_clk / freq) - 1;

    // Obliczamy poziom logiczny dla zadanego procentu wypełnienia
    uint32_t level = (uint32_t)(((float)(wrap + 1) * duty_percent) / 100.0f);

    pwm_set_wrap(slice_num, wrap);
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
   
    randomSeed(1234);
}

void loop() {
    uint16_t i, j, x, y;
    uint8_t current_duty;

    // Swiping  26kHz -> 24kHz
    x = random(20, 50);
    j = 1;
    i = 26000;

    while (i >= 24000) {

        // FM modulation - calculating new frequency
        i = i - j;

        // AM modulation - randomizing Duty of the PWM wave
        // attention it is best jamming when it is change linearly between 25-35 percent
        // current_duty = random(25, 35);
        // below it will swipe duty between 50-65 it is tailored to TC4420 chip
        current_duty = 50 + ((j * 15) / x);


        // changing the PWM frequency 
        update_pwm_advanced(i, current_duty);

        // y = random(1, 4);
        // j = (j + y) % x;
         j = (j + 1) % x;
        
        if (j == 0) j = 1;  // safeguard
       
       // you may want to adjust these
       delayMicroseconds(20);
    }

    // swiping up 24kHz -> 26kHz
    x = random(20, 50);
    j = 1;
    i = 24000;

    while (i <= 26000) {

        // FM modulation - calculating new frequency
        i = i + j;

        // AM modulation - randomizing Duty of the PWM wave
        // attention it is best jamming when it is change linearly between 25-35 percent
        // current_duty = random(25, 35);
        // below it will swipe duty between 50-65 it is tailored to TC4420 chip
        current_duty = 50 + ((j * 15) / x);

        // changing the PWM frequency 
        update_pwm_advanced(i, current_duty);
       
        // y = random(1, 4);
        // j = (j + y) % x;
         j = (j + 1) % x;
        
        if (j == 0) j = 1; // safeguard

       // you may want to adjust these
        delayMicroseconds(20);
    }
}
