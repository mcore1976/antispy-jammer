# antispy-jammer
Simple ultrasonic antispy voice recording jammer based on ATTINY13 / ATTINY85 with resistor ladder based DAC and 2 transistors driving piezo ultrasonic transducers. It prevents from unauthorized human speech recording by hidden microphones and voice assistants.

The project is based on following concept presented here : https://sandlab.cs.uchicago.edu/jammer/  , 
here https://github.com/y-x-c/wearable-microphone-jamming  
and here  http://people.cs.uchicago.edu/~ravenben/publications/pdf/ultra-chi20.pdf , however my design is cost optimized and very simplified in comparison to the one above.

I do not use programmable signal generator ( AD9833 ) and separated amplifier module ( PAM8403). 
Instead I am using the simplest microcontroller ATMEGA ATTINY13A ( or ATTINY85 ) to construct sinusoidal wave from predefined lookup table and 
set of resistors to build 5-bit Digital to Analogue converter ( to create sine wave ) with R-2R resistor ladder check here https://www.electronics-tutorials.ws/combination/r-2r-dac.html, also  2 bipolar transistors (NPN+PNP bridge) are used for simple driving unit of piezzoelectric ultrasonic transducers. 

It all can be powered from 2,7V - 5,5 V power source ( it can operate even directly from LiPol 3,7V battery, the higher the voltage - the more output power you get ).
The microcontroller ATTINY13 has its fuses set to operate with 9,6MHz internal clock and that allows to send 32 samples of waveform over DAC with maximum frequency up to 27kHZ.
In the code it is set for sinusoidal waveform to swing around center frequency of ultrasonic transducer which is 25kHz. 
The Sinusoidal waveform parameters have been calculated using Libreoffice Calc / Microsoft Excell and can be changed to any other waveform if necessary.
The C code is utilizing whole available PINs in ATTINY PORTB (PB0-PB4) to create DAC for sine wave. 
The chip may be changed to ATTINY85, but internal clock of this chip cannot go higher than 8MHz and that will limit DAC frequency to 23kHz.  I strongly recommend using ATTINY13 as in original design.

Component list :

- 1 x Microcontroller ATTINY13 / ATTINY13A ( or ATTINY 85 ) 

- 6 x 1K OHm resistors ( lowest wattage )

- 4 x 470 Ohm resistors ( lowest wattage )

- 1 x 47 microfarad electrolytic capacitor ( it can be even bigger like 100uF or more)

- 1 x 10 microfarad electrolytic capacitor ( it can be 1 - 100uF - for blocking distortions on power lines)

- 1 x 100 nanofarad capacitor (it can be 47nf - 470nF - for blocking distortions on power lines)

- 1 x BC547 : NPN bipolar transistor

- 1 x BC557 : PNP bipolar transistor

- set of 25kHZ (resonance frequency is important !)  ultrasonic piezo transducers like NU25C16T-1, 25kHz or equivalent

- some power source 3 - 5,5 Volt (it may be LiPol battery or set of 3xAA batteries)

Code is prepared in AVR-GCC and has to be uploaded with AVRDUDE to the ATTINy chip.

To compile the code for ATTINY13 use "compileattiny" script ( under linux "chmod +rx compileattiny && ./compileattiny ") for AVR-GCC environment compilation and flashing with AVRDUDE and USBASP cable.  Script "compileattiny85" is used for ATTINY85 flashing.

