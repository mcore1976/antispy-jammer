/*
 * Antispy ultrasonic device for preventing voice recording 
 * with ultrasonic waves generation based on ATTINY85,
 * ATTINY85 fuses have been set to internal PLL clock 16MHz 
 * with Resistor Ladder 5-bit Digital-to-Analog converter
 * and piezzo-electric driver
 * (C) Adam Loboda 2021, adam.loboda@wp.pl
 */

#include <stdint.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// PIN - PORT ASSIGNMENT DEFINITION FOR 5-BIT DAC DEVICE
#define	OUTPUTPORT		PORTB
#define	OUTPUTPORTCTRL		DDRB

 

int main(void)
{
        uint16_t sequence;
        uint8_t pulsewidth, amplitude, i;

        // seeding random number generator
        srand(1234);
        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

       //* neverending loop */
       while (1) 
       { 

          // generating WAVE on BIT0-BIT5 DAC outputs
          // randomized length of the pulse to fit within 23-27kHz frequency
          pulsewidth = 20 + (rand() % 50) ;
   
          // randomized amplitude of the pulse to fit within 5-bit value
          amplitude = rand() % 16;
          amplitude = amplitude +15;
 
          // sending bulk of these pulses before switching frequency and amplitude
          for(sequence=0; sequence<100; sequence++)
           {


           // send HIGH VOLTAGE - this time square wave
           // PB0 may serve as GND for PAM8403 module audio input
           // OUTPUTPORT = 0b00000111;
           OUTPUTPORT = amplitude; 

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
           for(i=0; i<(90 - pulsewidth); i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //
           };

      
        }; // neverending loop 

};  // end of MAIN


