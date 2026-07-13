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

    This version uses bobble voice jamming technique - chopper tricking AGC in smartphones
    
*/


#include <Arduino.h>
#include "hardware/pwm.h"   // Raspberry PICO SDK library
#include "hardware/clocks.h" // For function clock_get_hz
#include "hardware/adc.h"

// pin #1 for GP1 on RP2040-zero board
const int PWM_PIN = 1;
uint slice_num;
uint channel;

// Middle of piezzo transducers resonance frequency
const uint32_t BASE_CARRIER = 25000; 

// Trackig Sinusoidal modulation signal phase for seamless FM modulation
float fm_phase = 0.0;

// Human speech formants defintiion for [A, E, I, O, U]
const uint16_t formants[] = {730, 530, 270, 570, 300, 1090, 1840, 840, 870};
const int VOWEL_COUNT = 9;


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
    uint16_t level = (wrap + 1) * 60UL / 100UL;
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
    update_frequency_safe(BASE_CARRIER);


    // Randomization seed with ADC input
    adc_init();
    adc_gpio_init(26); 
    adc_select_input(0); // channel 0 (GP26) as an input

    uint32_t seed = 0;
    for (int i = 0; i < 16; i++) {
        seed = (seed << 2) ^ adc_read();
        delayMicroseconds(50); };
   
    randomSeed(seed);
}


void loop() {
    unsigned long startMicros, randomfrequencyDurationMicros;
    uint16_t randomfrequency ;

    // randomization seed with use of ADC input pin 0 - GP26
    uint32_t seed = 0;
    for (int i = 0; i < 16; i++) {
        seed = (seed << 2) ^ adc_read();
        delayMicroseconds(50);  };
   
    randomSeed(seed);
  
    // Here we pick a random INFRASOUND frequency for FM modulation
    //uint16_t randomfrequency = random(1,25);
    randomfrequency = random(20,50);
        
    // Random human speech wovel frequency to trick smartphones DSP
    //int vowelIdx = random(0, VOWEL_COUNT);
    //uint16_t randomfrequency = formants[vowelIdx];

    // Here we pick random time of this frequency duration
    // you may play with this range to have better jamming results on phones mic
    //unsigned long randomfrequencyDurationMicros = random(3, 7) * 1000;
    randomfrequencyDurationMicros = random(1, 10) * 1000;

    // We gather starting time in microseconds for the precision of FM modulation
    startMicros = micros();

    // Delta-time for the loop interval = 0.000040 sec  40 microseconds - has to match delay at the end of loop
    const float dt = 0.000040;
    
    while (micros() - startMicros < randomfrequencyDurationMicros) {

         // We calculate FM phase increase during this loop cycle for previusly selected modulation frequency
         //  dPhi = 2 * PI * f * dt
         float phase_increment = 6.28 * randomfrequency * dt;

         // Accumulating phase value so the FM modulation has no glithces 
         fm_phase += phase_increment;

         // Adjusting phase so it would match range 0 - 2PI
         if (fm_phase >= 6.28 ) fm_phase = fm_phase - 6.28;

        // FM modulation to create the sound upon bearer piezo frequency 
        // Weight 1000 adjusted to fit within piezo acceptable frequency  
        // piezo range is  24000 - 26000 Hz (25000 +/- 1000 Hz)
         long fm_offset = (sin(fm_phase) * 3000);
         uint32_t current_freq = BASE_CARRIER + fm_offset;
        
        // Update the frequency
        update_frequency_safe(current_freq);

        // time for 1 PWM cycle - should match value used for phase calculation
        delayMicroseconds( dt * 1000000);
    }
}
