/*
 * Antispy ultrasonic device for preventing voice recording 
 * with ultrasonic waves generation based on ARDUINO DIGISPARK,
 * connections : 
 * PAM8403 module AUDIO LEFT INPUT connected to Port 2 of DIGISPARK
 * PAM8403 module AUDIO RIGHT INPUT connected to Port 1 of DIGISPARK
 * PAM8403 module AUDIO GROUND INPUT connected to Port 0 of DIGISPARK
 * 10 piezzo-electric ultrasonic transducers connected in parallel to PAM8403 OUTPUT LEFT
 * 10 piezzo-electric ultrasonic transducers connected in parallel to PAM8403 OUTPUT RIGHT
 * (C) Adam Loboda 2021, adam.loboda@wp.pl
 */

#include <stdint.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// PIN - PORT ASSIGNMENT DEFINITION FOR OUTPUT
#define  OUTPUTPORT        PORTB
#define  OUTPUTPORTCTRL    DDRB



void setup() {
         uint16_t sequence;
        uint8_t pulsewidth, i;


        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

}

void loop() {

          // generating SQUARE WAVE on PB1 and PB2 outputs
          // randomized length of the pulse - PWM but with the center of 25KHz frequency
          pulsewidth = 20 + random(0, 50) ;
   
 
          // sending bulk of these pulses before switching frequency and amplitude
          // you may play with number of pulses send - default is 100
         
          for(sequence=0; sequence<100; sequence++)
           {

           // send HIGH VOLTAGE - this time square wave
           // PB0 may serve as GND for PAM8403 module audio input
           OUTPUTPORT = 0b00000110;

           // now delay to achieve desired frequency
           for(i=0; i<pulsewidth; i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //

           // send LOW VOLTAGE - this time square wave
           OUTPUTPORT = 0; 
           
           // now delay to achieve desired frequency
           for(i=0; i<(85 - pulsewidth); i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //
           };   // end of SEQUENCE loop


}
