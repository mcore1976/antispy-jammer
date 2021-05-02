/*
 * Antispy ultrasonic device for preventing voice recording 
 * with ultrasonic waves generation
 * based on ATTINY85/ATTINY13 chip,
 * ATTINY fuses have to be set for 9,6 MHz internal clock
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


// DEFINE 32 VALUES for ULTRASONIC WAVE SHAPE 
// VIA 5-BIT Digital to Analog conversion

// saw like wave 
/*const uint8_t  waveshape[] PROGMEM = 
  { 
   0, 1, 2, 3, 4, 5, 6, 7,
   8, 9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23,
   24, 25, 26, 27, 28, 29, 20, 31
 };
 */
 
 // sinusoidal waveform created in Excell / Libreoffice
const uint8_t  waveshape[] PROGMEM = 
  {  18, 21, 23, 26, 27, 29, 30, 30,
     30, 29, 27, 26, 23, 21, 18, 15, 
     12, 9, 7, 4, 3, 1, 0, 0,
     0, 1, 3, 4, 7, 9, 12, 15
     };


int main(void)
{
        uint8_t sequence;
        uint8_t frequency;
        uint8_t dacvalue;
        uint8_t delayusec;


        // SET OUTPUT PIN ON ALL OUTPUT PORTS
        
        OUTPUTPORTCTRL = 0b00011111;  //set all 5 LOWER bit pins as output
        OUTPUTPORT     = 0b00000000;  //set all 5 LOWER bit pins to ZERO

       //* neverending loop */
       while (1) { 

        // generating WAVE on BIT0-BIT5 DAC outputs
        // wirh values taken from PROGMEM WAVE definition
 
       /*
        // generate set of waves with different frequencies
        for(frequency=2; frequency<4; frequency++)
          {

              
          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;

          // setting output its the same as dacvalue; 
          OUTPUTPORT = dacvalue ;

          
             // now delay to achieve desired frequency
             for(delayusec=0; delayusec<frequency; delayusec++)
               {
                    asm volatile ( 
                                 "    nop	\n"
                                  );
               };   // end of delayusec loop
                     
           }; // end of sequence loop
        
         }; // end of frequency variable loop
      */

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile ("    nop	\n" );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
                        );
           };
           
           for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };
           
          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };



          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };



          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };

          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      




          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      



          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      



          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


          for(sequence=0; sequence<31; sequence++)
           {
           dacvalue = pgm_read_byte(waveshape + sequence) ;
           OUTPUTPORT = dacvalue; 
           // now delay to achieve desired frequency
           asm volatile (
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
           "    nop	\n" 
                        );
           };                      


        }; // neverending loop 

};  // end of MAIN



