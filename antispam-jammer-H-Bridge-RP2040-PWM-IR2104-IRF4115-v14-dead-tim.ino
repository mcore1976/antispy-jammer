/*
    Wearable microphone jammer - Vowel Formant Modulation Version (Strict 24kHz-26kHz)
    Target: RP2040-ZERO / RP2040-ONE
    Core: Earle Philhower RP2040 Core
    (C) Adam Loboda 2026
    version 14 - FM random modulation (sinusoidal wave)  for bobble sounds,
                 chopper style overloading AGC in smartphones
*/

#include <Arduino.h>
#include "hardware/pwm.h"  
#include "hardware/clocks.h"
#include "hardware/adc.h"


const int PIN_NORMAL = 0;    // GP0
const int PIN_INVERTED = 1;  // GP1
uint slice_num;


// Middle of piezzo transducers resonance frequency
const uint32_t BASE_CARRIER = 25000; 

// Trackig Sinusoidal modulation signal phase for seamless FM modulation
float fm_phase = 0.0;

// Human speech formants defintiion for [A, E, I, O, U]
const uint16_t formants[] = {730, 530, 270, 570, 300, 1090, 1840, 840, 870};
const int VOWEL_COUNT = 9;

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



    // Randomization seed with ADC input
    adc_init();
    adc_gpio_init(26); 
    adc_select_input(0); // channel 0 (GP26) as an input

    uint32_t seed = 0;
    for (int i = 0; i < 16; i++) {
        seed = (seed << 2) ^ adc_read();
        delayMicroseconds(50); };
   
    randomSeed(seed);        
//    randomSeed(1234);
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
