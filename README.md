# antispy-jammer
Simple ultrasonic antispy voice recording jammer based on ATTINY13 / ATTINY85 with resistor ladder based DAC and 2 transistors driving piezo ultrasonic transducers

The project is based on following concept presented here : https://sandlab.cs.uchicago.edu/jammer/  , 
here https://github.com/y-x-c/wearable-microphone-jamming  
and here  http://people.cs.uchicago.edu/~ravenben/publications/pdf/ultra-chi20.pdf

However my designed is cost optimized and simplified.  
I do not use programmable signal generator ( AD9833 ) and separated amplifier module ( PAM8403). 

Instead I am using the simplest microcontroller ATMEGA ATTINY13A ( or ATTINY85 ) , 

set of resistors to build 5-bit Digital to Analog converter ( to create sine wave ), 

two bipolar transistor to create simplest driving unit for piezzoelectric ultrasonic transducers. 

It all can be powered from 5V power source ( it can operate even directly from LiPol 3,7V battery )
The microcontroller has its fuses set to operate on 9,6MHz clock, that allows to send 32 samples of waveform over DAC with maximum frequency up to 40kHZ.
In the code it is set for sinusoidal waveform to swing around center frequency of ultrasonic transducer which is 25kHz. 
The Sinusoidal waveform parameters have been calculated using Libreoffice Calc / Microsoft Excell and can be changed to any other waveform if necessary.


Component list :
1 x Microcontroller ATTINY13 / ATTINY13A ( or ATTINY 85 ) 
6 x 1K OHm resistors ( lowest wattage )
4 x 470 Ohm resistors ( lowest wattage )
1 x 47 microfarad electrolytic capacitor ( it can be even bigger like 100uF or more)
1 x 10 microfarad electrolytic capacitor ( it can be 1 - 100uF - for blocking distortions on power lines)
1 x 100 nanofarad capacitor (it can be 47nf - 470nF - for blocking distortions on power lines)
set of 25kHZ (resonance frequency is important !)  ultrasonic piezo transducers like NU25C16T-1, 25kHz or equivalent
some power source 3 - 5,5 Volt 
