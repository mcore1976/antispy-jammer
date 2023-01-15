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

 
// created in Excell / Libreoffice - will randomly 
// swing transducer frequency around 25 kHZ with 0,4 kHz steps
// to achieve kind of white noise
const uint8_t  randomized[] PROGMEM = 
  {
5,10,0,6,2,6,2,2,2,6,3,4,3,7,6,10,
2,1,4,0,3,8,4,10,5,1,1,1,5,0,7,10,
9,10,9,2,0,10,6,3,0,7,8,4,10,1,2,
1,3,5,2,4,2,5,9,3,0,5,0,10,0,5,5,6,
6,10,5,2,7,3,8,4,5,5,3,0,0,4,8,10,
6,4,10,0,1,7,4,6,3,10,9,0,4,2,10,8,
8,6,3,1,10,9,10,4,1,7,3,10,0,6,3,1,
2,5,2,10,0,0,3,9,9,8,8,3,8,1,10,3,
5,7,4,10,5,2,0,7,5,2,6,7,7,10,9,2,
4,6,6,1,3,4,6,9,7,5,1,2,8,4,2,5,9,
5,1,3,1,8,4,10,5,4,6,8,7,10,9,6,5,
7,1,2,0,1,5,2,3,7,5,8,9,1,7,10,7,2,
5,10,6,7,6,2,0,8,8,5,1,7,8,7,5,9,8,
7,1,7,2,7,4,0,9,3,0,2,5,0,4,3,6,4,1,
10,7,5,0,8,8,9,1,0,8,8,7,2,8,0,6,7,
5,6,5,5,5,4,0,5,0,4,4,5,2,7,6,6,10,
4,7,5,1,7,7,10,6,2,0,0,1,6,3,0,2,7,
1,1,5,0,6,10,8,2,1,1,4,10,1,0,7,3,2,
5,6,1,6,2,1,3,2,6,8,10,2,7,9,1,9,6,0,
2,8,2,1,6,9,7,9,10,8,4,1,7,4,2,2,8,9,
9,3,0,7,3,8,0,7,2,10,8,3,4,9,4,0,0,0,
1,8,5,8,2,9,4,6,8,3,5,8,9,3,10,3,0,10,
7,3,10,3,6,7,6,4,4,7,4,2,5,3,4,9,1,5,
1,9,0,2,9,8,6,5,0,0,4,10,3,5,9,4,0,7,
10,1,7,0,10,4,0,3,9,7,6,7,4,8,6,5,1,7,
1,2,2,7,8,3,0,10,3,0,3,0,4,1,0,1,4,0,1,
10,7,5,2,3,10,9,2,0,9,7,2,2,2,2,5,8,10,
10,10,7,10,4,6,8,4,10,5,7,9,7,9,6,4,10,
4,0,8,7,7,9,2,4,5,10,9,4,6,2,10,1,10,2,
9,10,9,4,10,9,8,1,5,9,0,0,1,5,6,4,9,1,
9,10,10,8,3,4,2,2,0,3,7,10,4,4,8,2,8,7,
9,10,8,4,7,5,8,8,7,6,2,6,2,6,5,5,7,7,8,
1,4,9,7,6,10,10,7,8,2,5,3,10,4,4,3,5,10,
9,3,4,2,4,10,7,5,10,7,10,9,8,10,4,2,8,4,
8,4,0,10,0,5,4,4,6,7,2,9,8,2,3,4,5,7,8,7,
9,6,0,6,1,7,3,9,0,10,6,10,10,2,2,2,8,8,
8,7,9,0,2,9,10,8,0,6,6,4,1,7,1,8,4,0,4,
3,3,0,1,0,5,9,6,0,7,6,4,10,10,7,1,4,0,
10,2,7,9,4,9,7,9,8,2,9,3,2,10,9,8,6,9,
9,3,3,3,7,5,9,3,6,9,1,5,2,3,9,1,1,0,9,
4,8,4,9,4,4,0,3,2,4,0,4,4,1,0,10,2,2,
0,9,5,5,9,0,8,3,6,0,5,2,2,4,4,1,10,10,
0,8,5,4,10,5,4,9,0,2,7,0,4,1,1,6,0,0,
3,0,4,4,1,1,1,0,1,4,8,9,9,8,3,7,9,0,4,
4,1,8,9,1,2,0,8,9,5,10,3,7,7,1,1,9,0,7,
5,7,8,0,3,9,0,9,3,5,6,5,10,9,1,1,4,6,1,
10,7,10,8,3,10,7,4,3,10,0,2,10,10,9,2,
1,8,10,8,9,3,2,2,4,2,1,9,4,8,2,0,7,1,3,
2,0,1,3,8,0,4,0,9,2,1,5,0,5,8,4,9,9,0,
4,9,8,6,0,1,0,5,6,6,2,5,2,10,1,2,4,2,2,
3,7,1,7,3,8,5,5,7,10,6,10,6,0,8,0,5,6,8,
3,1,2,8,6,4,0,3,1,8,0,7,2,5,7,3,6,6,5,10,
5,0,6,8,9,6,7,7,0,5,3,6,10,3,7,9,5,8,3,1,
0,4,1,10,4,1,5,5,0,2,6,8,2,5,2,7,7,6,8,9,
0,10,1,4,7,3,5,6,3,8,7,0,10,1,9,4,7,1,8,
8,4,1,6,6,5,9,7,4,1,3,10,1,8,2,2,6,7,10,3    
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
          // 48 = 27,2 kHZ, added values (0-10) to drop randomly within 23-27kHz frequency
          // random values are taken from PROGMEM lookup table, step is 0,4 kHZ

          for(sequence=0; sequence<1000; sequence++)
           {
           // calculate pulse width 
           pulsewidth = pgm_read_byte(randomized + sequence) + 48 ;

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



