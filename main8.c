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
const uint8_t  freqrandom[] PROGMEM = 
  {
2,1,5,3,6,6,3,3,2,4,6,6,5,4,0,3,6,5,3,1,5,1,5,4,
0,2,5,2,2,5,5,6,2,2,2,2,4,0,1,3,3,5,0,1,6,1,4,5,
3,2,2,5,6,3,2,0,4,0,4,6,3,3,0,1,6,4,1,0,1,6,0,4,
5,1,3,2,4,5,2,0,0,4,6,0,1,4,3,5,6,1,6,2,5,0,0,1,
4,0,5,1,6,6,4,5,0,6,0,1,6,6,3,0,3,4,6,2,3,2,1,3,
5,1,3,2,3,2,5,4,1,5,4,6,4,6,0,5,1,3,2,0,4,6,6,5,
6,4,4,0,1,4,5,5,0,2,0,2,3,3,1,4,4,5,3,4,3,5,0,3,
1,2,0,3,2,5,1,5,4,2,3,5,3,0,2,4,5,1,2,6,0,4,6,1,
3,1,0,6,0,6,5,4,5,5,0,1,5,6,5,5,4,1,2,3,1,4,1,0,
5,5,4,5,4,6,1,3,0,2,6,3,5,0,2,5,3,3,4,4,0,1,0,0,
0,3,6,6,2,1,6,0,1,0,3,6,5,3,1,0,2,2,4,4,4,4,3,5,
3,3,3,5,1,1,0,0,3,0,2,0,3,3,0,3,2,4,5,2,3,0,3,4,
6,0,0,0,0,6,3,1,0,0,0,2,1,5,2,1,2,5,1,5,4,2,1,2,
3,0,6,4,3,6,0,2,0,3,6,2,2,1,2,2,2,2,5,2,3,4,0,4,
3,3,4,5,5,1,1,3,3,4,1,0,2,4,3,4,0,5,1,6,2,3,4,3,
5,3,6,4,3,4,3,1,4,3,4,0,0,5,0,1,6,3,5,0,1,3,3,3,
4,5,0,1,4,0,2,2,3,1,1,1,3,2,3,1 };

// created in Excell / Libreoffice - will randomly 
// swing transducer amplitude coded in 5 bits
// to achieve kind of white noise
const uint8_t  amprandom[] PROGMEM = 
  {
11,1,4,10,30,10,4,17,11,18,28,10,25,12,10,28,15,9,
28,15,17,9,8,20,29,28,2,17,19,14,7,20,18,18,1,26,
21,4,13,26,19,29,21,28,12,17,31,17,20,26,27,7,6,14,
23,29,7,28,3,18,13,31,13,28,23,28,4,9,8,22,22,22,6,
22,12,4,29,12,14,5,18,24,8,20,26,7,21,5,8,22,26,18,
22,2,29,9,19,19,6,16,9,22,1,21,23,6,11,9,21,22,30,
15,6,9,22,30,18,19,18,9,21,15,12,27,3,29,14,9,17,
11,18,25,14,5,2,20,5,4,19,26,31,18,4,20,3,16,3,26,
2,14,22,7,7,29,18,18,29,18,16,13,30,20,9,27,27,15,
1,1,27,21,20,1,3,3,18,3,7,4,25,15,22,16,27,8,8,30,
31,20,24,4,11,28,27,24,24,29,8,1,31,21,19,8,27,1,12,
10,24,3,6,5,22,29,8,4,11,2,29,7,27,30,4,14,16,24,19,
14,3,14,9,30,21,15,19,16,13,27,13,25,18,14,12,22,22,
26,14,28,21,6,23,1,14,23,12,21,7,23,15,1,3,18,19,13,
4,28,17,27,6,10,5,1,18,23,21,25,24,12,3,31,18,9,18,
10,1,9,13,31,6,24,12,5,8,23,7,26,18,5,5,21,5,10,16,
5,28,2,10,11,13,5,24,12,12,8,13,20,7,21,1,15,31,26,
24,29,28,6,9,10,22,14,12,13,23,13,29,9,26,7,28,27,1,
23,8,23,2,28,19,21,15,18,3,2,31,29,28,8,27,27,16,4,
22,5,28,28,1,4,3,6,25,2,31,18,25,28,6,5,21,11,11,24,
6,17,1,25,24,1,20,6,12,13,4,18,23,19,14,13,7,9,13,8,20,21 };




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
          // 24 = 27,5 kHZ, added values (0-6) to drop randomly within 23-27kHz frequency
          // random values are taken from PROGMEM lookup table, step is around 1 kHZ

          for(sequence=0; sequence<400; sequence++)
           {
           // calculate pulse width 
           // value 24 is frequency offset to achieve 25kHz center freq
           pulsewidth = pgm_read_byte(freqrandom + sequence) + 24 ;

           // send amplitude from lookup table to resistor based DAC
           OUTPUTPORT = pgm_read_byte(amprandom + sequence); 
           
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



