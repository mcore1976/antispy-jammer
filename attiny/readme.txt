These are old project files, currently obsolete. 

VERSION A & VERSION B :

- 1 x Microcontroller ATTINY13 / ATTINY13A (not recommended) or ATTINY 85/45/25 (best), DIGISPARK board or any other Arduino with SPI interface support may be used instead of microcontroller ( RP2040-ZERO or WEMOS 8266 D1 MINI boards). ATTENTION - if using RP2040 board and PWM option you do not need to bud AD9833
- 1 x 47 microfarad electrolytic capacitor ( it can be 1 - 470uF - for blocking distortions on power lines)
- 1 x 100 nanofarad capacitor (it can be 47nf - 680nF - for blocking distortions on power lines)
- set of 25kHZ (resonance frequency is important !)  ultrasonic piezo transducers like NU25C16T-1, 25kHz or equivalent
- some power source 3 - 5,5 Volt (it may be LiPol battery or set of 3xAA batteries) or 12-24Volts depending on audio amplifier board selected
- Audio amplifier : 1 x PAM4803 : 2 x 3Watt Amplifier module ( instead of 2 bipolar transistors), but I recommend to use TPA3116D2 mono module  ( XH-M542 ) for 100Watt audio power. ATTENTION ! For some reason TPA3110 and TPA3118 modules do not work properly with transducers. Only TPA3116D2 is good
- LM7805 - 5V voltage stabilizer when using boards : RP2040, ESP32, ESP8266 or discrete ATTINY85 chip
- 1 x 10K Ohm potentiometer ( or resistor divider ) may be put between ATTINY85/ARDUINO/AD9833 audio output pins and audio amplifier board input pins (please notice that some audio amplifier board like XH-M542 already have potentiometer therefore it may not be needed) < optional >
- AD9833 signal generator board for version B of the jammer, ATTENTION ! there is NO AD9833 option but only for RP2040 board
- MOSFET IRF 4115 < optional > + MOSFET DRIVER TC4420/IXDN614PI + ZENER DIODE 18V - if no Audio amplifier TPA3116D2/PAM4803 used 
- Diodes for 2AMP or more ( 1N400X series)  < optional >
- 2 x 10K resistors 0.25Watt < optional >
- When using MOSFET+DRIVER must additionally use High Current coils ( depending on availability : one 3.3 miliHenr coil per each 4 transducers OR one  4.7 miliHenr coil per each 5 transducers OR one  6.8 miliHenr coil per each 6 transducers ) -  < optional >
- Remember to use 18V ZENER DIODE to protect MOSFET IRF4115 from electrostatic discharge (ESD) and overvoltage on Gate pin. 

The transducer set may be connected directly to the TPA3116D2 output or through the diode and IRF MOSFET. 

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

Please configure first your Arduino IDE for the DIGISPARK board as described here : https://gist.github.com/Ircama/22707e938e9c8f169d9fe187797a2a2c

Version A :  ARDUINO DIGISPARK version - there is separated "mic-jammer.ino" version which is composed of ARDUINO DIGISPARK (ATTINY85) connected to PAM8403 MODULE and 20 transducers. It also gives same high range of jamming capability.  
- for DIGISPARK version please use schematic "arduino-mic-supresor-ultrasonic.png"  or "mic-jammer-TPA3116D2.png"  and Arduino script "mic-jammer.ino".
- In ARDUINO IDE go to File/Preferences/Additional board manager URL and put this URL : https://raw.githubusercontent.com/ArminJo/DigistumpArduino/master/package_digistump_index.json , then go to Menu Tools/Board and select Digistump AVR Boards / Digispark default 16.5 MHZ
 
Version B:  
- Digispark with AD9833 signal generator - please use schematic "arduino-mic-supresor-ultrasonic-v2.png"  or "mic-jammer-AD9833-TPA3116D2.png"  and INO  script "mic-jammer-ad9833-digispark.ino". 
- For different board than Digispark like Arduino Nano/Mini/Pro with AD9833 signal generator - please use schematic "arduino-mic-supresor-ultrasonic-v2-pro-mini.png" and INO script "mic-jammer-ad9833.ino".
- For boards like RP2040/ESP32C3/ESP8266 use appropriate diagram and source code with the name of this board
-  
Remember to use TPA3116D2 instead PAM8403 for better jamming capability and to use COILS and IRF 4115 MOSFET transistor + MOSFET driver chip TC4420.

If you want to use different boards than provided setup it is also possible as long as SPI library is available for your board , but you will need to change SPI communication pins to match your board.

For Arduino boards that do not have 5V voltage stabilizer on-board, you would also have to use Voltage Regulator LM7805 to feed particular board (like RP2040-Zero board , ESP32, ESP8266 )  and AD9833 module. The TPA3116D2 board needs to be powered from higher voltage - 12V taken before LM7805 is connected. If using TC4420 MOSFET driver and MOSFET IRF 4115 they need to be powered from higher voltage as well ( 18V is the maximum !!!)

Version C:
Only for RP2040 chips which take over signal generation entirely with perfect PWM function. It improves stability of the solution and further reduces cost of manufacturing. This version operates only with MOSFET drivers and MOSFETs, there is no option to use audio amplifier board here. Use free software KICAD to see the schematic & PCB. The software can be flashed directly by dragging UF2 file to RP2040 USB disk drive ( RP2040 module  must have pressed BOOT key while attaching to the PC over USB). You do not even need Arduino environment to flash the SW.

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




