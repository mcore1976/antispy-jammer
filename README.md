# antispy-jammer
Simple ultrasonic antispy voice recording jammer based on ATTINY13 / ATTINY85 with resistor ladder based DAC and 2 transistors (or PAM8403 module)  driving piezo ultrasonic transducers. It prevents from unauthorized human speech recording by hidden microphones and voice assistants.

The project is based on following concept presented here : https://sandlab.cs.uchicago.edu/jammer/  , 
here https://github.com/y-x-c/wearable-microphone-jamming  
and here  http://people.cs.uchicago.edu/~ravenben/publications/pdf/ultra-chi20.pdf , however my design is cost optimized and very simplified in comparison to the one above.

I do not use programmable signal generator ( AD9833 ) here. 
Instead I am using the simplest microcontroller ATMEGA ATTINY13A ( or ATTINY85 ) to construct sinusoidal wave from predefined lookup table and 
set of resistors to build 5-bit Digital to Analogue converter ( R-2R resistor ladder DAC : check here https://www.electronics-tutorials.ws/combination/r-2r-dac.html ), also  2 bipolar transistors (NPN+PNP bridge - class B amplifier) are used for simple driving unit of piezzoelectric ultrasonic transducers : https://www.electronics-tutorials.ws/amplifier/amp_6.html (they introduce cross-over distortion  and have very small power).  Instead of using 2 transistor amplifier consider using PAM8403 amplifier module ( see diamgram with "enhanced" version) which gives 6Watt output power and much higher range of jamming.

It all can be powered from 2,7V - 5,5 V power source ( it can operate even directly from LiPol 3,7V battery, but remember the higher the voltage - the more output power you get ).
The microcontroller ATTINY13 has its fuses set to operate with 9,6MHz internal clock and that allows to send 32 samples of waveform over DAC with maximum frequency up to 27kHZ. The ATTINY85 can go even higher up to 60kHz.
In this source code it is set for sinusoidal waveform to match  center frequency of ultrasonic transducer which is 25kHz (also slightly shifted frequencies are available in the source code for experimentation, but commented out). 
The Sinusoidal waveform parameters have been calculated using Libreoffice Calc / Microsoft Excell and can be changed to any other waveform if necessary.
The C code is utilizing whole available PINs in ATTINY PORTB (PB0-PB4) to create DAC for sine wave. 
The chip may be changed to ATTINY85, the internal clock has to be reconfigured to PLL clock, no DIV8 to 16MHz frequency ( AVRDUDE fuses : -U lfuse:w:0xf1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m  ) like for Digisparkdevice.  Also number of NOP commands have to be finetuned in the source code to fit correct ~25kHZ frequency.

The solution is based on AVR-GCC environment and USBASP and AVRDUDE tools may be used for ATMEL ATTINY chip programming. This is not Arduino code, however some bigger chips like ATMEGA 328P ( the one from ARDUINO ) also can be used. 

If you have Linux based machine you would need to setup the environment by installing following packages : "gcc-avr", "binutils-avr" (or sometimes just "binutils"), "avr-libc", "avrdude". For Ubuntu / Linux Mint you may use following command : "sudo apt-get install gcc-avr binutils-avr binutils avr-libc gdb-avr avrdude"

If you have Windows machine, please follow this tutorial to install necessary packages : http://fab.cba.mit.edu/classes/863.16/doc/projects/ftsmin/windows_avr.html

If you do not know how to connect cables for ATMEL ATTINY chip programming please follow my tutorial here : https://www.youtube.com/watch?v=7klgyNzZ2TI


Compilation

- use "compileattiny" and "main.c" files for ATTINY13/ATTINY13A chip  (internal 9,6 MHz clock )
- use "compileattiny85" and "main2.c" files for ATTINY85 chip  (internal 16MHz PLL clock)


Component list :

- 1 x Microcontroller ATTINY13 / ATTINY13A - or ATTINY 85  

- 6 x 1K OHm resistors ( lowest wattage )

- 4 x 470 Ohm resistors ( lowest wattage )


- 1 x 10 microfarad electrolytic capacitor ( it can be 1 - 100uF - for blocking distortions on power lines)

- 1 x 100 nanofarad capacitor (it can be 47nf - 470nF - for blocking distortions on power lines)

- set of 25kHZ (resonance frequency is important !)  ultrasonic piezo transducers like NU25C16T-1, 25kHz or equivalent

- some power source 3 - 5,5 Volt (it may be LiPol battery or set of 3xAA batteries)

AND 

- 1 x BC547 : NPN bipolar transistor

- 1 x BC557 : PNP bipolar transistor

- 1 x 47 microfarad electrolytic capacitor ( it can be even bigger like 100uF or more)
- 
OR (RECOMMENDED)

- 1 x PAM4803 : 2 x 3Watt Amplifier module ( instead of 2 bipolar transistors) 

- 1 x 10K Ohm potentiometer ( or resistor divider )


Code is prepared in AVR-GCC and has to be uploaded with AVRDUDE to the ATTINy chip.

To compile the code for ATTINY13 use "compileattiny" script ( under linux "chmod +rx compileattiny && ./compileattiny ") for AVR-GCC environment compilation and flashing with AVRDUDE and USBASP cable.  Script "compileattiny85" is used for ATTINY85 flashing.

See the video showing how this device works : https://www.youtube.com/watch?v=YBQ7A4W0bTo



