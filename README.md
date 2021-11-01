# antispy-jammer
Simple ultrasonic antispy voice recording jammer based on ATTINY13 / ATTINY85 with resistor ladder based DAC and amplification PAM8403 module driving piezo ultrasonic transducers. It prevents from unauthorized human speech recording by hidden microphones and voice assistants.

The project is based on following concept presented here : https://sandlab.cs.uchicago.edu/jammer/  , 
here https://github.com/y-x-c/wearable-microphone-jamming  
and here  http://people.cs.uchicago.edu/~ravenben/publications/pdf/ultra-chi20.pdf , however my design is cost optimized and very simplified in comparison to the one above.

I do not use programmable signal generator ( AD9833 ) here so the jamming capability may not be such good as original design. 
Instead I am using the simplest microcontroller ATMEGA ATTINY13A ( or ATTINY85 ) to construct sinusoidal wave from predefined lookup table and 
set of resistors to build 5-bit Digital to Analogue converter ( R-2R resistor ladder DAC : check here https://www.electronics-tutorials.ws/combination/r-2r-dac.html ), also in initial stage 2 bipolar transistors (NPN+PNP bridge - class B amplifier) were used for simple driving unit of piezzoelectric ultrasonic transducers : https://www.electronics-tutorials.ws/amplifier/amp_6.html (they introduce cross-over distortion  and have very small power).  
After testing with 2 transistor amplifier it turned out that the jamming power is too low. Please consider using PAM8403 amplifier module ( see diamgram with "enhanced" version) which gives 6 Watt output power and much higher range of jamming and please use "main4.c" (for ATTINY85) or "main5.c" (for ATTINY13) version as it has very improved jamming capability due to use of pseudo white-noise bias frequency that modulates center frequency of ultrasonic transducers. The 25kHz is randomly shifted in 0,4 kHz offsets within 23-27 kHz range, that gives awesome results in jamming (up to 4-5 meters of jamming). For "main4.c"/"main5.c" version the DAC resistor ladder is not necessary and you can directly connect one of PortB pins to potentiometer input.

It all can be powered from 2,7V - 5,5 V power source ( it can operate even directly from LiPol 3,7V battery, but remember the higher the voltage - the more output power you get ). The microcontroller ATTINY13 has its fuses set to operate with 9,6MHz internal clock and that allows to send 32 samples of waveform over DAC with maximum frequency up to 27kHZ. The ATTINY85 can go even higher up to 60kHz.

In first version (please do not use it ) the source code is set for sinusoidal waveform to match  center frequency of ultrasonic transducer which is 25kHz (also slightly shifted frequencies are available in the source code for experimentation, but commented out). The Sinusoidal waveform parameters have been calculated using Libreoffice Calc / Microsoft Excell and can be changed to any other waveform if necessary.The C code is utilizing whole available PINs in ATTINY PORTB (PB0-PB4) to create DAC for sine wave or pulse wave depending on source code version. 
In second version the source code is set to send square pulses to the ultrasonic transducers, but center frequency is modulated/shifted with random steps +/-2KHz around the center 25kHz frequency.

Available versions :
- AVR-GCC version for direct chip upload - ATTINY13 and ATTINY85 chips. 
In ATTINY85 version the internal clock has to be reconfigured to PLL clock, no DIV8 to 16MHz frequency ( AVRDUDE fuses : -U lfuse:w:0xf1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m  ) like for Digisparkdevice.  Also number of NOP commands is finetuned in the source code to fit exactly ~25kHZ frequency.
This solution is based on AVR-GCC environment and USBASP and AVRDUDE have to be used for ATTINY chip programming. This is not Arduino code, however some bigger chips like ATMEGA 328P ( the one from ARDUINO ) also can be used. 
- ARDUINO DIGISPARK version - there is separated "mic-jammer.ino" version which is composed of ARDUINO DIGISPARK (ATTINY85) connected to PAM8403 MODULE and 20 transducers. It also gives same high range of jamming capability.

-------------------------------------------------------------------------------------

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


-------------------------------------------------------------------------------------

AVR-GCC compilation and chip flashing :

"mainXX.c" source code is prepared in AVR-GCC and has to be uploaded with AVRDUDE to the ATTINY85/ATTINY13 chip.

If you have Linux based machine you would need to setup the environment by installing following packages : "gcc-avr", "binutils-avr" (or sometimes just "binutils"), "avr-libc", "avrdude". For Ubuntu / Linux Mint you may use following command : "sudo apt-get install gcc-avr binutils-avr binutils avr-libc gdb-avr avrdude"

If you have Windows machine, please follow this tutorial to install necessary packages : http://fab.cba.mit.edu/classes/863.16/doc/projects/ftsmin/windows_avr.html

If you do not know how to connect cables for ATMEL ATTINY chip programming please follow my tutorial here : https://www.youtube.com/watch?v=7klgyNzZ2TI


Compilation

- use "compileattiny" and "main.c" files for ATTINY13/ATTINY13A chip  (internal 9,6 MHz clock )
- use "compileattinyv2" and "main5.c" files for ATTINY13/ATTINY13A chip  (internal 9,6 MHz clock )
- use "compileattiny85" and "main2.c" files for ATTINY85 chip  (internal 16MHz PLL clock)
- use "compileattiny85v2" and "main4.c" files for ATTINY85 chip  (internal 16MHz PLL clock)


To compile the code for ATTINY use relevant "compileattinyXXX" script ( example : under linux "chmod +rx compileattiny && ./compileattiny ") for AVR-GCC environment compilation and flashing with AVRDUDE and USBASP cable.  

See the video showing how this device works :
- Old version : https://www.youtube.com/watch?v=YBQ7A4W0bTo   ( please do not use it , low jamming capability )
- New version for Arduino (and main4.c / main5.c for ATTINY) : https://youtu.be/JFtU2hQQ2vQ



