# antispy-jammer
Simple ultrasonic antispy voice recording jammer based on ATTINY13 / ATTINY85 / ATTINY45 / ATTINY25 with audio amplifier PAM8403 / TPA3116D2 module driving piezo ultrasonic transducers. It prevents from unauthorized human speech recording by hidden microphones and voice assistants.

The project is based on following concept presented here : https://sandlab.cs.uchicago.edu/jammer/  , 
here https://github.com/y-x-c/wearable-microphone-jamming  
and here  http://people.cs.uchicago.edu/~ravenben/publications/pdf/ultra-chi20.pdf , however my design evolved and using AD9833 and 25kHZ transducers is the only similarity with the original project.  

There are two versions of the jammer :
- version A with ATTINY85/Digispark only and audio amplifier TPA3116D2  + 20 transducers. This version is a bit audible and may not be preferred by some people
- version B with ATTINY85/Digispark + AD9833 Signal Generator + audio amplifier TPA3116D2 ( XH-M542 )  + 20 transducers . This version is barely audible and has outstanding jamming capability for newest mobile phones. 
Please notice that some audio amplifiers like TPA3110 do not work correctly with ultrasonic transducers (only some "clicking" sound and no ultrasound). So far I have found  only PAM8403 and TPA3116D2 modules to work properly.


History of Version A of the jammer :

At the beginning of this project ("main.c" and "main2.c" source files)  there was a set of resistors used to build 5-bit Digital to Analogue converter ( R-2R resistor ladder DAC : check here https://www.electronics-tutorials.ws/combination/r-2r-dac.html ) to create sinusoidal audio wave and audio amplification stage with 2 bipolar transistors (NPN+PNP bridge - class B amplifier) for driving piezzoelectric ultrasonic transducers : https://www.electronics-tutorials.ws/amplifier/amp_6.html (they introduce cross-over distortion  and have very small power). The Sinusoidal waveform parameters have been calculated using Libreoffice Calc / Microsoft Excell and can be changed to any other waveform if necessary.The C code was utilizing whole available PINs in ATTINY PORTB (PB0-PB4) to create DAC for sine wave or pulse wave depending on source code version.  I have uploaded this code here only for reference, maybe someone would like to play with it.

However after testing first prototype  it turned out that the jamming power is too low because center frequency 25kHz has to be FM modulated in random manner. Finally I had to use PAM8403 amplifier module ( see diagram with "enhanced" version) for 6 Watt output power and modify lookup table and code to construct square audio wave with pseudo white-noise bias that modulates center frequency of ultrasonic transducers. In this version the 25kHz frequency is randomly shifted in 0,4 kHz offsets within 23-27 kHz range and that gives awesome results in jamming (up to 4-5 meters of jamming).  Please notice that even 6 Watt of audio power is too low to sucesfully jamm from greater distance so I ended up with using TPA3116D2 module - 100Watt audio power.

Files "main4.c"/"main5.c"  are using Digital to Analog Converter for FM 25kHz signal modulation only , while files "main7.c"/"main8.c" are using random PWM and amplitude modulation to generate the noise (in test main7.c/main8.c work better than main4.c/main5.c files). 
If using files with AM modulation please tune signal gain using potentiometer not to get it distorted.

The microcontroller ATTINY13 has its fuses set to operate with 9,6MHz internal clock while ATTINY85 has fuses set to operate on 16MHz clock as in the Digispark module. 


History of version B of the jammer :

20.02.2022 - I have managed to re-create original design with only : ATTINY85 chip + AD9833 signal generator + PAM8403 audio amplifier and set of transducers.
The code "main6.c" and the script "compileattiny6" is prepared for this purpose, the diagram will be "arduino-mic-supresor-ultrasonic-v2-ATTINY85.png". 
Also relevant INO scripts and schematic of this re-created design are available for any other Arduino supporting SPI serial bus connectivity. For ARDUINO version please use diagrams "arduino-mic-supresor-ultrasonic-v2.png" and "arduino-mic-supresor-ultrasonic-v2-pro-mini.png"

27.01.2023 - Updated original programming - together with random FREQUENCY shifting I am also using random PHASE shifting which gives better effectivenes for high-end phones despite the signal is a little bit audible..  This is only valid for versions which use  AD9833 signal generator.

12.04.2023 - The code for Digispark/ATTINY85 + AD9833 + TPA3116/PAM8403 was changed to support INFRASOUND 5-25Hz random FM modulation over 25KHz ULTRASOUND carrier. This gives the best result for jamming iPhones and on the high-end phones and the signal is barely audible by human ear when demodulated on obstacles. However such approach cannot be introduced into the design which does not use AD9833 due to speed limitation of the ATTINY85 chip. If you want to jam iPhones I would suggest to use design : Digispark + AD9833 + TPA3116 + at least 12 transducers
The code uses SQUARE PULSE to generate the wave ( AD_SQUARE option in the code ) which gives best overall jamming result, but you can easily change it to less audible option SINUSOIDAL WAVE ( AD_SINE ) or TRIANGLE WAVE ( AD_TRIANGLE ). Sinusoidal signal has lower jamming capability but it is almost not audible at all.

24.08.2023 - The code for Digispark/ATTINY85 + AD9833 + TPA3116/PAM8403 was changed to use pseudo-number generator ( rand() function ) for infrasound 7Hz-15Hz and AD_SINE option on AD9833. The audio signal is now barely hearable but the device keeps good jamming efficiency.

17.09.2023 - The code for Digispark/ATTINY85 + AD9833 + TPA3116/PAM8403 was changed to use pseudo-number generator ( rand() function ) for both frequency swing 24000-26000 Hz and infrasound 5Hz-25Hz and AD_SINE option on AD9833. This code combines both types of jamming infrasound + white noise. However please find that random infrasound is good only for Android based phones, for iPhones the best is to use static 45-50Hz FM modulation of ultrasound (found empirically).

IF YOU WANT TO INCREASE JAMMING CAPABILITY USE BETTER AUDIO AMPLIFIER LIKE TPA3116D2 WHICH GIVES 50WATT OF AUDIO POWER AND MORE TRANSDUCERS LIKE 50 PER AUDIO CHANNEL ! 

-------------------------------------------------------------------------------------

Component list :

- 1 x Microcontroller ATTINY13 / ATTINY13A (not recommended) or ATTINY 85/45/25 (best), DIGISPARK board or any other Arduino may be used instead of microcontroller
- 1 x 47 microfarad electrolytic capacitor ( it can be 1 - 470uF - for blocking distortions on power lines)
- 1 x 100 nanofarad capacitor (it can be 47nf - 680nF - for blocking distortions on power lines)
- set of 25kHZ (resonance frequency is important !)  ultrasonic piezo transducers like NU25C16T-1, 25kHz or equivalent
- some power source 3 - 5,5 Volt (it may be LiPol battery or set of 3xAA batteries) or 12-24Volts depending on audio amplifier board selected
- Audio amplifier : 1 x PAM4803 : 2 x 3Watt Amplifier module ( instead of 2 bipolar transistors), but I recommend to use TPA3116D2 mono module  ( XH-M542 ) for 100Watt audio power. ATTENTION ! For some reason TPA3110 and TPA3118 modules do not work properly with transducers. Only TPA3116D2 is good
- 1 x 10K Ohm potentiometer ( or resistor divider ) may be put between ATTINY85/ARDUINO/AD9833 audio output pins and audio amplifier board input pins (please notice that some audio amplifier board already have potentiometer therefore it may not be needed)
- AD9833 signal generator board for version B of the jammer

-------------------------------------------------------------------------------------

Available versions of the source code :

-------------------------------------------------------------------------------------

AVR-GCC VERSIONS

You only need the ATTINY13 or ATTINY85 chip not full Arduino module therefore the device can be smaller. In ATTINY85 version the internal clock has to be reconfigured to PLL clock, no DIV8 to 16MHz frequency ( AVRDUDE fuses : -U lfuse:w:0xf1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m  ) like for Digispark device.  Also number of NOP commands is finetuned in the source code to fit exactly ~25kHZ frequency. USBASP and AVRDUDE have to be used for ATTINY chip programming. This is not Arduino code, however some bigger chips like ATMEGA 328P ( the one from ARDUINO ) also can be used.

Version A : Please use schematic "antispy-jammer-enhanced-schematic.png" and following combinations of source code and compilation script :
- for ATTINY85 : main4.c   +  compileattiny85v2 , main7.c   +  compileattiny85v3 
- for ATTINY13 : main5.c   +  compileattinyv2 , main8.c  +   compileattinyv3

Version B : This version uses AD9833 signal generator and ATTINY85 (ATTINY13 is too small)  therefore 
- please use combination of  "main6.c"   +  "compileattiny6"  and schematic "arduino-mic-supresor-ultrasonic-v2-ATTINY85.png" or the one with TPA3116D2 - "arduino-mic-supresor-ultrasonic-v2-ATTINY85-TPA3116.png" .
 
 -------------------------------------------------------------------------------------

ARDUINO VERSIONS :

Version A :  ARDUINO DIGISPARK version - there is separated "mic-jammer.ino" version which is composed of ARDUINO DIGISPARK (ATTINY85) connected to PAM8403 MODULE and 20 transducers. It also gives same high range of jamming capability.  
- for DIGISPARK version please use schematic "arduino-mic-supresor-ultrasonic.png"  or "mic-jammer-TPA3116D2.png"  and Arduino script "mic-jammer.ino".  If you are having doubts how to connect and program Digispark board please follow this tutorial : http://digistump.com/wiki/digispark/tutorials/connecting
 
Version B:  
- Digispark with AD9833 signal generator - please use schematic "arduino-mic-supresor-ultrasonic-v2.png"  or "mic-jammer-AD9833-TPA3116D2.png"  and INO  script "mic-jammer-ad9833-digispark.ino". 
- For different board than Digispark like Arduino Nano/Mini/Pro with AD9833 signal generator - please use schematic "arduino-mic-supresor-ultrasonic-v2-pro-mini.png" and INO script "mic-jammer-ad9833.ino". 
Remember to use TPA3116D2 instead PAM8403 for better jamming capability.

-------------------------------------------------------------------------------------

AVR-GCC compilation information and chip flashing :

"mainXX.c" source code is prepared in AVR-GCC and has to be uploaded with AVRDUDE to the ATTINY85/ATTINY13 chip.

If you have Linux based machine you would need to setup the environment by installing following packages : "gcc-avr", "binutils-avr" (or sometimes just "binutils"), "avr-libc", "avrdude". For Ubuntu / Linux Mint you may use following command : "sudo apt-get install gcc-avr binutils-avr binutils avr-libc gdb-avr avrdude"

If you have Windows machine, please follow this tutorial to install necessary packages : http://fab.cba.mit.edu/classes/863.16/doc/projects/ftsmin/windows_avr.html

If you do not know how to connect cables for ATMEL ATTINY chip programming please follow my tutorial here : https://www.youtube.com/watch?v=7klgyNzZ2TI


Compilation

First version of the project :
- files "compileattiny" and "main.c" for ATTINY13/ATTINY13A chip  (internal 9,6 MHz clock )   - initial version please do not use it anymore !
- files "compileattiny85" and "main2.c" for ATTINY85 chip  (internal 16MHz PLL clock)  - initial version please do not use it anymore !

Version A  : 
- use "compileattinyv2" and "main5.c" files for ATTINY13/ATTINY13A chip - FM modulation only (internal 9,6 MHz clock )
- use "compileattinyv3" and "main8.c" files for ATTINY13/ATTINY13A chip - AM/PWM signal modulation (internal 9,6 MHz clock )
- use "compileattiny85v2" and "main4.c" files for ATTINY85 chip - FM modulation only  (internal 16MHz PLL clock)
- use "compileattiny85v3" and "main7.c" files for ATTINY85 chip  - AM/FM signal modulation (internal 16MHz PLL clock)
 

Version B (with AD9833 board) :
- use "compileattiny6" and "main6.c" files for ATTINY85 chip  (internal 16MHz PLL clock) + AD9833 signal generator

To compile the code for ATTINY use relevant "compileattinyXXX" script ( example : under linux "chmod +rx compileattiny && ./compileattiny ") for AVR-GCC environment compilation and flashing with AVRDUDE and USBASP cable.  


-------------------------------------------------------------------------------------


See the video showing how this device works :
- Old version : https://www.youtube.com/watch?v=YBQ7A4W0bTo   ( please do not use it , low jamming capability )
- New version for Arduino (and main4.c / main5.c for ATTINY) : https://youtu.be/JFtU2hQQ2vQ
- Re-created original version for Digispark : https://youtu.be/PcTkMWJb_Gs
- The newest version microphone jamming beast with TPA3116D2 audio amplifier board - jamms even iPhones : https://youtu.be/8RgoYlrgW4s



