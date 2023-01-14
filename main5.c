/*
 * Antispy ultrasonic device for preventing voice recording 
 * with ultrasonic waves generation based on ATTINY13,
 * ATTINY13 fuses have been set to internal clock 9.6MHz 
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

 
// created in Excell / Libreoffice - will randomly 
// swing transducer frequency around 25 kHZ with 0,4 kHz steps
// to achieve kind of white noise
const uint8_t  randomized[] PROGMEM = 
  {
2,6,1,5,2,5,4,6,0,1,4,0,3,4,2,2,
5,3,3,5,1,0,3,0,0,5,3,5,4,3,4,3,
5,2,0,6,2,1,0,6,4,4,0,2,0,4,2,1,
4,0,3,2,0,5,0,6,0,5,3,4,3,3,0,6,
0,2,5,2,4,6,5,3,1,6,4,3,2,5,1,5,
4,0,0,3,2,2,1,6,2,1,3,4,2,3,3,3,
4,4,0,4,2,4,4,6,4,5,5,1,1,2,5,5,
1,3,3,1,6,0,5,6,0,4,0,6,1,6,2,2,
6,1,1,6,5,3,4,0,0,6,0,4,1,5,6,3,
5,3,3,6,0,6,6,3,1,1,6,2,0,0,2,1,
4,0,6,6,0,2,4,0,4,4,4,5,4,5,0,5,
5,5,3,5,1,3,3,2,2,6,1,0,2,4,4,1,
5,0,4,4,3,4,4,3,6,4,1,3,1,5,4,1,
6,5,2,6,6,4,1,6,3,5,1,0,5,0,5,5,
2,0,3,3,6,6,5,0,1,0,2,0,6,1,3,0,
2,0,3,2,1,4,3,0,0,5,3,3,1,5,5,1,
4,5,0,6,2,0,2,4,3,1,1,0,2,2,1,3,
6,6,2,0,3,3,5,5,6,4,2,1,3,6,6,4,
6,0,2,1,6,5,0,4,0,3,1,1,5,2,4,0,
1,5,3,4,0,1,4,6,3,1,2,3,5,0,1,4,
2,1,3,2,2,2,0,4,3,3,6,1,1,3,4,5,
6,1,4,2,3,2,2,4,3,2,2,0,3,4,0,0,
2,5,5,5,0,3,6,4,0,2,5,3,6,5,1,0,
2,6,4,1,0,2,4,4,6,5,4,1,6,6,5,0,
4,2,0,3,5,3,1,0,4,1,2,2,6,4,4,5,
3,2,1,1,5,5,5,4,2,0,4,6,3,2,6,4,
3,2,5,1,3,4,0,4,5,1,4,0,4,3,4,4,
6,0,3,1,1,3,5,4,3,6,4,1,0,5,0,2,
4,5,6,3,0,2,4,4,4,2,1,1,2,2,0,6,
6,6,1,4,2,2,0,3,0,3,6,4,3,5,6,4,
6,0,5,5,1,4,6,2,0,6,1,0,6,3,1,5,
1,4,4,0,3,5,0,4,3,1,4,0,5,6,4,6,
3,5,1,1,0,5,3,2,2,1,6,0,2,3,1,3,
4,5,2,0,4,6,0,1,5,3,1,5,0,1,4,2,
1,0,3,4,4,2,4,4,4,4,4,3,4,6,1,5,
4,5,0,0,2,5,6,2,0,6,3,4,3,0,2,4,
5,5,2,6,5,0,3,4,0,4,6,5,6,0,3,5,
3,6,2,4,5,3,2,3,2,2,0,2,6,1,0,2,
1,6,3,1,0,4,1,5,0,3,4,6,1,4,0,6,
3,6,0,5,1,2,3,2,1,6,5,2,2,0,0,0,
1,3,5,4,6,5,1,0,3,0,6,2,5,6,2,2,
6,2,6,3,1,3,4,5,3,2,2,4,3,1,0,2,
1,1,1,3,2,4,0,2,3,1,4,0,4,1,2,0,
2,0,6,5,4,3,3,2,0,4,6,4,2,6,0,5,
6,1,6,6,0,5,4,1,3,2,1,6,1,3,6,6,
2,2,0,3,6,1,2,6,2,1,1,3,5,0,3,6,
3,1,2,0,2,6,5,5,4,4,0,6,3,6,5,3,
6,5,4,1,4,2,5,0,0,5,4,2,6,3,6,3,
5,2,5,3,6,4,0,5,0,6,3,6,6,3,6,0,
4,5,4,1,6,4,1,3,4,4,5,3,5,0,6,4
};


int main(void)
{
        uint16_t sequence;
        uint8_t pulsewidth, i;


        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

       //* neverending loop */
       while (1) 
       { 

          // generating WAVE on BIT0-BIT5 DAC outputs
          // 27 = 27,5 kHZ, added values (0-6) to drop randomly within 23-27kHz frequency
          // random values are taken from PROGMEM lookup table, step is around 1 kHZ

          for(sequence=0; sequence<800; sequence++)
           {
           // calculate pulse width 
           pulsewidth = pgm_read_byte(randomized + sequence) + 27 ;

           // send HIGH VOLTAGE - this time square wave
           // PB0 may serve as GND for PAM8403 module audio input
           OUTPUTPORT = 0b00000110;
           
           // send amplitude modulated square wave through Digital to Analog converter 
           // PB0 may serve as GND for PAM8403 module audio input
           // OUTPUTPORT = pulsewidth; 
           
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
           for(i=0; i<pulsewidth; i++)
             {
              asm volatile (
           "    nop	\n" 
                );
              };
           //
           };

      
        }; // neverending loop 

};  // end of MAIN



