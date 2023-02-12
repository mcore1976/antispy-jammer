/*
 * Antispy ultrasonic device for preventing voice recording 
 * with ultrasonic waves generation based on ATTINY13,
 * ATTINY85 fuses have been set to internal PLL clock 9.6MHz 
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
// swing pulse width to achieve noise effect around the center 25kHz frequrency
const uint8_t  randomized[] PROGMEM = 
  {
33,31,40,38,19,6,17,37,36,6,39,20,7,34,2,28,32,2,31,
32,33,22,30,12,31,40,9,18,14,14,40,29,34,37,4,8,6,3,
14,22,37,31,34,26,29,2,36,15,33,15,36,26,40,21,20,11,
26,2,27,3,8,8,35,29,7,18,11,8,11,16,17,40,35,35,7,38,
16,2,11,17,40,29,38,25,15,8,29,13,30,38,8,27,36,12,
28,14,23,24,23,26,1,28,32,17,15,35,23,21,5,40,24,13,
31,38,28,1,37,27,36,21,22,18,29,4,36,18,18,32,7,39,17,
4,10,12,14,13,21,3,37,32,3,15,33,32,39,35,6,36,37,17,
6,12,36,36,36,31,40,22,37,5,39,34,2,27,32,11,17,33,40,
7,39,20,14,24,36,1,34,21,14,15,11,30,21,36,31,4,31,2,
36,12,4,24,31,2,11,20,9,3,29,15,10,7,11,31,34,38,36,
18,29,23,24,17,12,22,23,26,29,16,26,7,10,35,21,37,5,
1,12,2,7,12,10,39,13,7,21,28,18,16,5,28,22,26,16,40,
39,8,40,30,34,5,12,34,4,26,33,6,14,14,16,29,19,5,1,
12,38,34,28,27,34,21,10,32,38,37,25,16,40,10,2,16,37,
30,33,19,7,25,31,15,25,18,1,16,8,21,11,27,35,35,16,10,
36,26,34,32,2,39,13,39,1,28,6,34,13,19,5,10,2,4,28,18,
32,35,40,30,9,5,34,8,28,21,14,12,36,31,31,8,17,40,1,
32,27,4,8,30,36,12,28,3,29,4,4,15,3,11,13,26,27,7,36,
31,6,2,12,26,23,23,26,26,3,5,12,12,14,11,35,11,10,31,
40,19,16,4,27,15,37,34,9,40,35,17,27,16,22,32,32,15,27,
19,2,11,18,29,4,13,20,6,25,38,7,25,1,1,24,23,2,14,25,
19,10,29,21,29,37,10,19,9,30,23,7,34,24,8,35,40,19,20,
38,40,8,32,24,22,7,22,37,38,24,6,21,2,33,27,8,21,17,34,
23,36,2,19,21,23,15,32,37,9,21,39,24,16,4,26,12,8,19,15,
26,24,8,11,37,29,25,10,14,1,24,12,2,29,32,9,8,22,40,12,
15,2,30,15,12,33,17,2,9,14,7,28,26,2,18,30,24,18,39,7,9,
20,20,15,11,13,18,6,9,7,20,3,25,10,26,33,36,37,30,25,4,
3,7,12,23,3,9,7,9,22,24,29,32,24,23,40,32,38,39,22,39,
4,22,21,36,9,22,28,17,17,36,28,7,7,25,16,30,35,11,20,
13,38,34,26,15,32,7,26,36,14,32,26,10,20,40,1,35,18,16,
36,32,2,18,31,14,10,1,35,1,1,30,16,21,22,20,38,15,23,1,
9,9,3,27,11,14,6,7,6,9,10,3,1,30,31,9,3,40,22,23,39,21,
32,16,36,5,5,23,3,20,16,26,10,2,38,15,16,38,7,33,5,31,
11,17,10,38,27,40,8,3,40,8,14,19,8,26,22,33,15,13,22,26,
24,15,23,35,14,1,8,29,37,34,9,30,4,5,27,27,40,29,33,29,
3,4,39,1,8,3,11,21,32,36,22,34,15,37,36,7,14,23,23,17,33,
19,20,40,15,22,14,17,9,28,40,30,2,33,8,5,15,29,14,23,8,25,
28,17,29,30,1,2,12,29,6,3,39,35,32,15,4,2,24,26,6,10,15,8,
11,7,26,37,33,33,34,1,4,9,16,27,10,22,30,16,9,27,21,3,
32,37,30,38,24,3,1,39,8,28,7,22,30,13,10,3,5,4,9
};


int main(void)
{
        uint16_t sequence;
        uint8_t pulsewidth, i, bulk;


        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

       //* neverending loop */
       while (1) 
       { 

          // generating WAVE on BIT0-BIT5 DAC outputs
          // 48 = 27,2 kHZ, added values (0-10) to drop randomly within 23-27kHz frequency
          // random values are taken from PROGMEM lookup table, step is 0,4 kHZ

          for(sequence=0; sequence<800; sequence++)
           {
           // calculate pulse width 
           pulsewidth = pgm_read_byte(randomized + sequence);

             // sending a bulk of pulses with defined pulsewidth
             // you may play with this value for better effect
             for(bulk=0; bulk<25; bulk++)
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
              for(i=0; i<(47-pulsewidth); i++)
                  {
                        asm volatile (
                                        "  nop	\n" 
                                     );
                  };
            // End of 'bulk' loop
            };            
           // end of 'sequence' loop;
          };
        // end of neverending loop  
        };

};  // end of MAIN



