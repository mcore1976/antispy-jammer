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

 
 //  created in Excell / Libreoffice - will randomly 
 // swing around 25 kHZ with 0,4 kHz steps
const uint8_t  waveshape[] PROGMEM = 
  {
5,9,9,2,3,2,5,9,3,5,9,8,0,10,9,8,1,7,0,9,
2,2,1,6,6,6,5,8,10,7,4,7,9,4,1,4,1,10,10,
1,8,9,1,6,7,4,5,9,10,0,4,7,1,10,2,1,7,0,
7,3,5,1,9,5,5,8,4,3,2,5,7,0,2,8,6,5,10,0,
7,5,6,0,8,9,2,2,6,4,10,8,8,4,10,2,3,3,3,
4,3,0,6,1,9,8,7,7,7,3,4,4,4,5,5,7,0,2,8,
3,8,7,8,7,2,3,8,10,7,4,1,7,4,9,8,6,6,5,2,
2,4,4,1,8,1,1,10,6,9,2,6,6,5,6,0,3,5,9,10,
6,6,10,10,2,1,4,7,0,3,4,1,5,7,5,9,0,4,10,
0,7,7,0,0,6,10,1,9,2,2,9,2,6,4,10,3,2,7
    };


int main(void)
{
        uint16_t sequence;
        uint8_t pulsesize, i;


        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

       //* neverending loop */
       while (1) 
       { 

        // generating WAVE on BIT0-BIT5 DAC outputs
        // with values taken from PROGMEM WAVE definition
 
 
          // 48 = 27,2 kHZ, added values (0-10) to drop randomly within 23-27kHz frequency
          // random values are taken from PROGMEM lookup table, step is 0,4 kHZ
          for(sequence=0; sequence<100; sequence++)
           {
           pulsesize = pgm_read_byte(waveshape + sequence) + 48 ;

           // send HIGH VOLTAGE - this time square wave
           OUTPUTPORT = 31; 
           // now delay to achieve desired frequency
           for(i=0; i<pulsesize; i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //
           // send LOW VOLTAGE - this time square wave
           OUTPUTPORT = 0; 
           // now delay to achieve desired frequency
           for(i=0; i<pulsesize; i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //
           };

      
        }; // neverending loop 

};  // end of MAIN



