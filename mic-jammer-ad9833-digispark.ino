/*
    microphone jammer 
    - ATTINY85 / DIGISPARK version
    
    CONNECTION
    --------
    AD9833 PCB to DIGISPARK:
    FDATA --> pin 1  (MOSI / DO ) 
    SCLK  --> pin 2 (SCK)
    FSYNC --> pin 3 (SS / USCK / DD)
    DGND  --> GND
    VCC   --> VCC 

    AD9833 PCB to TPA3116 AUDIO AMPLIFIER
    AOUT  --> LEFT AUDIO IN + RIGHT AUDIO IN
    AGND  --> AUDIO GND

    REMEMBER TO CONNECT AD9833 TO 5V AND GND PINS OF DIGISPARK
    DIGISPARK HAS TO BE POWERED FROM 12V THROUGH Vin PIN
    TPA3116 HAS TO BE CONNECTED TO 12V AND GND
    
    
*/

#include <Arduino.h>


/**
 * SPI pins - definition for ATTINY85
 */

#define SPI_DDR_PORT DDRB
#define USCK_DD_PIN DDB2
#define DO_DD_PIN DDB1
#define DI_DD_PIN DDB0


/** \name AD9833 waveform output modes
 * Parameters of \ref ad9833_set_mode()
 * @{
 */
#define AD_OFF      0
#define AD_TRIANGLE 1
#define AD_SQUARE   2
#define AD_SINE     3
/**@}*/

/** \name AD9833 command register bits
 * @{
 */

#define AD_B28     13
#define AD_HLB     12
#define AD_FSELECT 11
#define AD_PSELECT 10
#define AD_RESET   8
#define AD_SLEEP1  7
#define AD_SLEEP12 6
#define AD_OPBITEN 5
#define AD_DIV2    3
#define AD_MODE    1

/** @}*/

/** \name AD9833 register addresses
 * @{
 */
#define AD_FREQ0  (1<<14)
#define AD_FREQ1  (1<<15)
#define AD_PHASE0 (3<<14)
#define AD_PHASE1 ((3<<14)|(1<<13))

/** @}*/

/** \name AD9833 calculation macros
 * @{ */
#define AD_F_MCLK 25000000 ///<Clock speed of the ad9833 reference clock
#define AD_2POW28 268435456 ///<used in calculating output freq

/** Macro that calculates the value for a ad9833 frequency register from a frequency */
#define AD_FREQ_CALC(freq) (uint32_t)(((double)AD_2POW28/(double)AD_F_MCLK*freq)*4)

/** Macro that calculates value for Timer1 output compare from a frequency*/
#define AD_MOD_FREQ_CALC(freq) (F_CPU/(64*(uint32_t)freq))

/** Macro that calculates the value for a ad9833 phase register from a phase in degrees */
#define AD_PHASE_CALC(phase_deg) (uint16_t)((512*phase_deg)/45)
/** @} */

/** Struct that holds all the configuration it's initialized as a global variable
 * in the ad9833.c file */
typedef struct {
    float    freq[2]; ///<Holds the frequencies of 
    float    phase[2];
    float    mod_freq;
    uint8_t  freq_out;
    uint8_t  phase_out;
    uint8_t  mode;
    uint16_t command_reg;
} ad9833_settings_t;


ad9833_settings_t ad_settings; ///<This is used to store all settings.


// randomized infrasound 4096 values
const uint8_t  randomized[] PROGMEM = 
  {
17,21,23,22,25,9,10,9,6,13,20,12,11,10,20,13,19,16,9,15,20,15,5,6,
25,11,13,10,6,23,12,25,21,11,20,7,6,22,9,25,20,24,19,13,15,14,20,
9,5,19,13,17,13,13,25,7,20,21,20,17,21,17,13,9,24,20,20,16,21,9,5,
23,9,11,6,9,16,21,21,24,8,24,24,24,10,9,23,13,9,14,24,25,24,18,6,5,
17,22,21,24,20,16,16,25,15,21,21,11,6,25,15,11,18,18,20,25,13,18,
11,7,13,16,13,5,19,6,7,17,24,6,10,11,13,24,23,21,7,23,14,17,9,9,17,
16,13,13,6,21,19,22,10,8,5,10,22,24,23,15,5,7,17,20,6,7,14,20,10,
25,19,13,5,20,9,24,21,11,6,18,6,13,19,15,18,7,17,21,12,24,21,20,11,
19,20,19,13,19,17,15,18,18,6,20,20,6,20,8,14,14,19,10,20,15,18,6,
12,24,14,5,17,12,25,13,16,8,7,19,21,20,18,11,7,12,17,21,10,17,6,9,
23,14,24,19,12,19,11,18,23,14,20,10,8,10,16,18,12,14,25,12,17,21,
25,21,5,13,11,6,5,12,18,17,18,21,8,16,8,12,22,20,17,12,19,17,24,
14,17,20,22,24,19,16,13,7,21,21,17,5,15,10,23,25,6,17,11,11,15,
13,5,13,11,25,9,11,18,14,20,9,15,10,25,13,17,16,5,19,14,24,15,
19,7,15,18,13,10,21,10,15,19,24,18,19,16,15,19,14,15,23,24,7,11,
9,10,24,11,19,10,12,13,11,14,5,15,13,17,25,9,21,7,20,12,15,10,5,
14,10,10,16,23,20,11,18,17,7,8,11,23,9,12,8,13,14,6,20,18,20,24,
25,21,17,12,7,19,24,12,18,6,13,8,9,19,10,7,19,15,7,24,21,19,16,14,
8,23,20,20,18,24,25,21,10,17,19,21,18,16,21,14,19,15,18,8,23,17,13,
21,7,20,14,11,10,8,11,9,24,18,19,25,12,12,8,19,17,17,24,5,10,11,11,
11,25,12,19,13,6,9,21,10,12,8,14,5,7,22,21,7,15,8,22,18,17,6,16,7,
8,18,14,17,23,17,23,25,24,20,18,7,23,12,16,20,10,22,22,21,15,9,6,14,
24,12,9,21,24,12,21,22,20,16,7,25,25,17,8,7,15,16,22,14,15,22,14,
21,8,13,14,5,25,10,5,22,19,25,8,24,25,24,7,23,6,24,14,18,6,25,7,14,
13,12,15,20,5,11,9,25,15,21,22,23,11,25,5,12,15,5,15,7,17,23,19,7,24,
19,18,7,17,6,10,11,16,8,19,22,22,23,15,6,16,9,15,9,7,25,6,6,23,24,5,
11,17,6,16,11,12,11,25,16,15,16,5,11,22,5,12,12,18,25,18,6,6,21,23,
8,7,9,5,19,15,25,17,16,7,13,19,11,7,24,18,12,12,12,16,8,9,19,18,18,
14,20,22,10,8,25,15,19,23,15,10,10,19,7,21,12,6,10,22,20,9,10,20,12,
11,7,25,20,14,6,17,14,17,9,14,19,24,20,20,24,18,19,20,11,19,10,16,8,
17,5,13,12,23,22,22,5,23,6,10,17,7,20,14,22,17,11,11,8,21,14,6,16,12,
13,16,21,16,17,18,6,7,16,14,14,22,7,5,9,21,7,20,9,6,12,7,14,8,13,16,
19,10,9,25,16,19,24,7,9,20,13,6,24,15,16,10,15,14,18,14,19,10,16,24,
12,25,15,22,6,19,9,17,15,22,21,7,24,9,25,11,21,23,24,17,5,19,23,11,
21,20,6,10,22,25,16,21,8,19,9,17,13,15,6,12,17,17,8,6,18,25,13,16,9,
14,8,17,18,13,13,6,10,22,7,17,14,14,9,6,20,12,22,14,15,8,15,13,17,17,
9,21,14,8,22,9,7,17,11,8,25,18,21,19,17,18,21,19,12,18,9,17,9,12,12,
24,21,15,22,25,17,15,11,19,10,18,14,6,23,5,21,17,23,9,5,18,13,21,22,
23,15,7,24,12,22,9,15,17,5,20,13,18,18,12,6,7,25,9,6,25,8,9,17,14,
25,17,23,15,10,22,21,23,24,21,8,17,15,16,15,23,9,6,9,6,25,14,21,5,
17,17,16,8,10,21,24,19,23,19,24,17,16,20,13,14,14,25,15,25,15,14,25,
18,10,19,6,25,14,17,18,20,22,8,9,5,11,9,16,19,5,15,17,17,15,10,10,
6,12,11,11,21,18,19,18,20,19,24,23,14,19,20,15,19,12,12,15,7,6,17,
17,16,23,22,14,15,13,8,20,6,15,9,21,11,20,15,25,9,18,15,13,15,14,
11,19,11,23,22,22,20,10,9,13,5,10,24,14,7,18,14,25,18,21,25,11,23,
21,16,14,8,24,14,6,23,12,11,13,6,6,11,13,11,9,8,19,18,12,22,11,23,
10,16,11,23,16,15,14,10,11,21,6,15,11,9,16,7,25,6,21,21,15,5,12,
14,11,22,23,11,25,11,18,5,23,22,14,22,12,5,8,9,12,25,12,22,18,13,
21,19,12,22,11,12,19,6,7,19,21,23,10,14,12,10,21,10,7,6,25,9,13,
10,21,10,15,23,14,8,21,23,8,14,20,11,9,22,13,25,8,20,16,20,9,14,11,
16,8,16,25,22,16,5,7,23,10,24,19,11,17,23,19,10,13,12,24,8,14,9,17,
14,8,20,24,11,23,19,20,14,23,17,13,24,11,8,14,21,8,10,6,25,5,9,16,
13,14,24,23,9,12,16,24,25,7,14,17,8,11,17,16,7,7,6,14,23,17,24,7,
13,18,9,6,21,7,11,19,10,16,9,6,15,9,16,19,11,14,21,13,18,22,14,7,
7,5,10,16,5,8,24,9,9,23,15,25,7,13,18,7,16,18,23,10,6,7,22,13,19,
20,17,11,6,17,7,12,6,9,15,14,25,25,12,17,11,22,20,10,10,7,17,22,9,
21,12,15,21,8,13,10,15,19,18,16,20,10,7,11,22,6,11,10,20,24,25,15,
22,11,13,19,24,10,9,16,17,16,13,13,13,23,20,19,19,12,9,6,7,8,17,
12,20,14,12,6,13,19,12,15,8,11,5,20,20,19,10,10,11,9,21,17,5,5,12,
18,10,23,22,9,6,10,14,18,14,7,20,12,16,7,19,22,9,16,7,16,9,9,19,17,
23,5,22,25,6,19,9,10,5,21,9,18,25,9,10,5,8,9,7,15,8,21,11,14,24,19,
6,21,25,15,19,22,19,10,14,22,25,22,13,6,7,8,9,8,25,15,20,24,15,21,
10,10,21,12,15,17,21,5,9,23,24,17,21,13,11,8,18,18,7,21,8,23,19,11,
11,20,10,25,23,22,5,11,8,10,24,15,16,8,19,9,12,6,20,12,15,21,24,17,
24,25,11,19,25,8,20,13,5,8,10,20,9,12,8,5,18,22,25,22,23,7,5,22,14,
24,6,16,11,13,6,9,21,5,25,17,19,25,22,18,5,11,12,10,7,24,9,17,6,6,
6,10,20,10,17,5,9,19,9,17,19,25,8,23,20,8,24,7,22,10,14,10,6,18,24,
5,25,14,16,14,14,25,19,12,19,24,10,17,14,10,11,23,9,23,13,16,9,6,
15,10,16,23,20,20,9,9,17,9,15,13,14,16,13,19,15,22,16,5,5,16,22,15,
13,13,10,18,24,24,15,5,10,13,23,6,8,20,15,13,24,5,14,21,22,14,21,21,
7,6,20,7,15,22,9,22,5,18,21,10,18,14,11,15,20,23,21,6,15,13,17,13,23,
21,16,24,18,16,8,6,14,20,19,13,10,15,19,25,15,5,15,20,7,17,10,20,18,
16,14,17,10,13,18,6,5,13,18,6,17,19,19,20,7,22,20,12,21,12,12,16,22,
11,25,15,18,24,5,6,6,20,22,6,22,6,10,14,7,25,9,20,12,9,23,16,9,13,
10,13,20,18,24,10,16,20,7,18,7,20,23,19,5,17,14,6,24,18,22,5,17,20,
21,10,15,12,20,9,21,23,9,12,14,24,19,9,10,15,18,15,9,23,16,14,18,25,
16,19,5,17,15,16,15,24,21,20,23,25,5,11,5,5,7,23,5,15,19,16,16,14,
11,13,13,17,15,10,23,22,18,22,25,10,10,10,11,14,20,11,23,13,24,18,19,
6,18,16,8,17,18,22,12,7,15,18,25,15,10,17,7,22,15,12,10,16,12,5,12,
10,23,13,7,13,20,19,16,17,20,8,10,20,12,25,14,12,8,19,20,10,11,15,8,
22,10,17,25,18,10,9,15,7,24,20,16,15,12,15,23,13,8,18,18,20,19,9,9,
19,7,21,20,21,5,5,17,9,15,14,23,19,18,20,6,23,18,5,23,24,6,20,20,19,
10,8,20,14,10,16,9,17,12,17,14,22,14,13,6,20,9,18,15,5,22,18,19,11,
9,11,8,12,25,22,17,18,13,15,24,23,5,14,16,19,12,13,15,10,19,9,19,23,
7,5,16,11,17,18,11,7,6,6,10,18,9,22,10,16,23,11,24,18,21,24,17,18,13,
12,22,15,20,13,19,17,19,17,10,21,17,18,7,16,17,22,21,9,17,10,12,13,
24,8,25,6,9,19,16,12,14,15,15,15,19,15,12,17,7,7,17,19,8,19,12,24,21,
25,12,5,23,13,5,6,7,15,7,25,16,19,9,20,17,5,14,6,23,10,11,5,25,12,8,
12,17,23,8,13,15,25,13,15,14,8,17,11,7,20,19,19,7,14,7,19,21,17,17,9,
11,17,7,21,25,8,6,21,12,6,8,13,15,21,12,19,7,24,18,19,6,7,20,7,5,5,
14,5,18,25,20,14,19,14,16,23,11,25,25,22,23,9,11,14,12,22,8,11,14,17,
25,17,18,11,11,5,19,24,7,10,24,13,22,10,16,16,13,10,8,16,25,5,15,17,
23,14,13,5,19,24,9,16,5,16,6,15,16,17,18,23,21,5,9,24,9,14,25,6,5,10,
6,8,16,12,8,7,14,25,18,16,10,19,6,6,18,8,6,21,23,14,17,18,10,8,11,8,
12,6,13,16,16,25,12,16,8,20,20,23,16,7,12,23,7,17,8,10,5,11,8,18,18,
18,11,16,18,11,14,9,17,17,24,15,16,25,21,5,13,24,11,12,17,20,14,17,16,
23,21,21,24,14,16,6,19,14,15,13,15,24,21,11,16,17,23,25,7,18,8,17,10,
14,21,9,5,5,23,21,22,13,16,15,24,8,16,22,6,10,15,21,10,13,11,14,18,25,
8,19,18,11,23,5,8,22,11,20,12,19,10,23,25,9,12,5,7,6,15,24,15,20,19,10,
11,22,17,9,15,16,13,5,21,9,7,21,11,8,18,6,13,11,20,23,21,10,7,17,12,22,
22,20,12,21,15,25,24,9,5,10,15,15,9,7,22,16,18,12,17,22,14,5,17,5,14,
22,20,11,9,17,20,23,7,5,7,25,8,11,15,15,11,5,25,5,9,23,23,18,8,20,
24,6,13,13,6,22,23,23,21,17,21,9,12,21,10,8,22,25,8,23,14,20,6,10,9,
11,19,12,25,9,9,16,14,15,15,14,19,5,24,13,25,23,12,24,21,15,9,17,13,
10,9,10,8,18,25,11,22,9,13,5,5,23,15,19,21,11,24,7,20,17,17,15,7,5,
15,6,24,13,6,11,11,18,20,6,17,25,16,9,5,9,9,12,23,13,23,21,8,8,25,19,
22,7,20,14,23,5,21,12,24,6,8,25,18,15,19,23,14,22,20,17,7,10,22,19,5,
7,22,14,20,18,22,17,12,9,13,12,16,24,6,19,12,13,23,21,14,20,13,21,22,
15,14,12,23,19,5,15,6,8,23,24,11,20,20,16,20,15,18,6,12,22,21,8,10,6,
7,12,20,23,16,22,23,16,14,6,6,14,23,18,9,10,22,19,7,6,14,13,21,25,25,
25,17,5,16,25,17,16,12,14,9,20,8,14,18,13,12,16,17,12,21,6,20,7,15,
12,16,15,21,15,21,18,13,21,13,17,13,21,8,11,7,25,22,8,19,25,17,20,24,
22,21,18,24,15,18,8,18,21,17,13,6,14,9,10,10,23,10,23,14,23,8,6,22,
23,11,11,8,24,12,14,9,10,23,6,21,11,10,13,12,20,17,17,18,9,25,6,25,
16,19,21,19,7,9,10,5,25,19,18,9,8,20,20,8,8,20,14,21,5,23,15,11,10,
20,18,6,16,18,17,7,14,25,16,19,17,8,23,7,16,5,25,11,23,19,23,5,9,6,
14,18,23,10,8,17,14,11,10,6,9,18,20,15,17,19,8,18,7,25,8,5,18,14,
18,11,18,25,5,6,7,14,16,16,23,22,21,8,13,20,13,7,20,23,25,6,8,20,
11,17,13,20,18,19,8,21,10,14,7,5,5,25,14,15,9,19,8,17,24,6,8,25,21,
24,17,16,17,6,21,7,24,11,23,17,22,18,24,20,18,8,24,17,25,12,23,9,
22,13,18,15,20,14,14,10,11,16,8,12,25,7,11,11,22,23,12,7,21,14,8,
11,14,9,16,20,15,12,25,7,22,5,5,5,10,17,24,9,24,18,9,18,15,25,18,
17,17,5,22,16,20,7,10,14,25,12,22,9,23,18,13,13,5,16,17,11,21,5,24,
12,8,7,15,16,5,18,19,9,8,14,19,17,6,15,11,22,23,24,18,13,19,21,21,
13,14,8,25,16,9,22,19,8,25,18,14,21,24,6,16,23,14,8,24,7,21,16,10,
14,25,21,22,10,17,20,18,5,14,13,25,14,15,24,24,21,9,6,17,22,23,7,
18,16,7,17,12,23,24,13,23,19,14,5,5,17,16,13,10,5,22,8,15,23,25,15,
18,6,23,11,5,7,6,6,21,23,8,23,5,14,13,5,23,12,22,14,24,21,25,19,24,
19,11,20,14,11,10,12,16,22,10,14,10,5,15,9,12,16,6,20,15,8,14,12,17,
20,6,22,17,8,11,22,12,10,5,7,10,5,16,7,9,16,9,8,24,9,5,18,25,6,20,
16,21,8,9,9,7,12,10,18,16,24,18,16,17,14,5,23,15,20,8,8,15,22,12,16,
25,9,7,16,21,17,18,23,22,25,7,5,14,15,8,10,12,13,9,7,8,22,16,23,12,
5,8,23,13,18,21,11,22,9,8,25,17,18,12,25,21,18,11,21,21,13,25,14,11,
5,19,14,22,22,8,24,23,7,19,9,8,13,25,6,12,15,13,5,15,12,23,5,12,24,
16,8,13,12,17,7,17,12,25,13,8,14,19,18,7,11,11,24,14,14,17,7,17,8,11,
17,24,11,15,21,16,16,11,6,8,18,23,18,23,22,14,13,10,6,25,14,19,12,15,
22,5,18,10,13,13,21,10,20,13,9,19,20,20,12,20,23,23,14,19,23,16,22,
14,8,20,5,9,15,17,5,9,9,25,16,5,16,25,20,19,19,6,13,23,6,22,25,23,11,
24,10,6,14,8,24,5,8,11,20,22,21,20,9,12,5,10,13,9,13,17,14,15,24,16,
18,6,7,23,24,16,14,8,18,24,7,6,12,20,24,11,9,19,24,10,19,24,21,22,9,
14,22,5,5,16,22,13,19,18,16,10,6,21,9,12,22,16,14,22,7,19,8,12,17,7,
6,12,18,20,19,6,19,15,12,15,23,23,22,9,16,25,12,12,11,23,19,15,8,5,
14,21,10,24,17,5,21,25,13,10,7,18,17,8,8,6,6,25,6,10,21,9,15,13,13,
17,11,18,15,5,21,20,12,10,21,25,17,20,11,12,13,12,23,6,22,18,18,7,19,
25,8,10,24,17,7,20,20,9,11,12,24,23,21,16,13,24,24,10,9,15,25,10,14,
13,24,21,17,24,12,24,20,19,18,19,6,23,10,23,16,7,21,19,20,7,22,20,5,
24,17,15,13,23,7,12,19,16,22,8,7,5,18,15,15,18,7,5,5,10,10,16,8,8,10,
13,5,10,18,16,7,17,5,20,25,17,5,22,15,17,17,17,6,18,22,17,9,18,23,12,
20,23,14,21,7,22,22,21,13,11,25,8,16,6,11,16,13,12,17,13,12,5,12,18,6,
7,23,5,13,22,15,20,25,21,5,24,19,6,19,21,5,19,9,12,24,15,6,25,12,6,6,
14,15,8,15,10,16,9,21,15,22,11,16,16,16,8,8,7,9,20,16,17,20,16,20,10,
5,15,19,16,12,23,11,13,11,25,20,16,8,20,12,17,19,7,7,22,14,5,16,11,25,
21,19,25,17,11,13,24,24,5,20,17,22,9,24,13,11,13,25,10,17,22,21,19,11,
11,14,15,6,11,23,20,20,24,23,19,5,6,21,18,19,25,22,13,14,23,9,11,22,8,
23,7,21,23,16,6,24,9,18,12,8,7,15,10,15,20,20,24,20,7,23,5,21,19,24,24,
18,10,17,8,22,22,8,16,10,9,24,6,13,10,16,5,25,17,11,19,18,5,9,9,11,25,
12,20,25,7,19,23,21,9,21,25,12,18,5,17,23,6,21,20,18,21,7,23,23,15,23,
20,13,11,19,6,8,24,13,5,10,6,5,12,22,16,25,16,16,21,20,19,15,21,5,14,
25,17,5,6,23,20,7,13,16,9,19,14,12,17,17,5,8,9,17,23,18,22,13,19,7,7,
23,25,23,23,9,16,25,6,16,18,16,17,18,23,21,5,6,16,14,21,15,17,17,11,6,
19,10,20,12,22,12,20,17,10,18,13,6,16,20,20,25,13,9,23,10,19,14,16,17,
14,14,17,15,6,19,20,23,15,14,5,22,21,19,20,15,17,7,7,23,25,13,11,22,10,
20,19,16,22,21,22,24,23,17,21,5,7,23,14,8,16,10,19,23,23,20,7,22,17,6,
22,11,20,9,24,5,23,11,13,8,17,8,11,6,17,23,6,10,5,18,9,5,22,10,19,7,12,
14,17,8,11,8,15,20,10,20,8,20,20,10,19,22,20,23,7,5,20,21,5,21,7,18,16,
16,18,10,9,17,24,11,17,24,7,7,6,15,20,8,8,22,10,23,22,15,17,5,15,19,17,
11,23,22,12,11,16,6,14,15,8,18,5,19,18,10,18,6,23,14,19,14,9,14,14,8,9,
12,14,15,21,15,18,25,10,15,23,8,13,21,18,25,10,16,21,20,15,7,19,13,12,19 
  };


// ***********************************
// ******* SPI SPECIFIC ROUTINES
// ***********************************

/**
 * init
 * 
 * SPI initialization
 */
void spi_init() 
{
 
  // ATTINY85 pin assignments
  // SPI_SS = PORTB3, 
  // SPI_MOSI = PORTB1
  // SPI_MISO = PORTB0, 
  // SPI_SCK = PORTB2

  // Configure SPI pins
  //pinMode(SPI_SS, OUTPUT);
  //pinMode(SPI_MOSI, OUTPUT);
  //pinMode(SPI_MISO, INPUT);
  //pinMode(SPI_SCK, OUTPUT);

  //digitalWrite(SPI_SCK, HIGH);  
  //digitalWrite(SPI_MOSI, LOW);
  //digitalWrite(SPI_SS, HIGH); 


    DDRB |= _BV(3);   // configure output for SPI CS SS
 
    USICR &= ~(_BV(USISIE) | _BV(USIOIE) | _BV(USIWM1));

    USICR |= _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);

    SPI_DDR_PORT |= _BV(USCK_DD_PIN);   // set the USCK pin as output

    SPI_DDR_PORT |= _BV(DO_DD_PIN);     // set the DO pin as output

    SPI_DDR_PORT &= ~_BV(DI_DD_PIN);    // set the DI pin as input
    
    // spi mode 1 
    USICR |= _BV(USICS0);
    
 

}

/**
 * 
 * Send uint8_t via SPI
 * 
 * 'value'  Value to be sent
 * 
 * Return:
 *  Response received from SPI slave
 */
uint8_t spi_send(uint8_t value) 
{
    
    USIDR = value;

    USISR = _BV(USIOIF);                // clear counter and counter overflow interrupt flag

    while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC);

    return USIDR;
}


// ----------------------------------------------------------------
// AD8933 programmable signal generator specific routines
// ----------------------------------------------------------------

/**
 * a wrapper function for sending 16-bit SPI packets.
 * \param packet 16-bit value to be sent over SPI.
 */
static inline void ad9833_send(uint16_t packet)
{


   PORTB |= _BV(2);    // set PIN PB2 : SCLK HIGH
 
   PORTB &= ~_BV(3);   // set PIN PB3 : SS / CS LOW to write over SPI
  
  //delayMicroseconds(2); // Some delay may be needed

  // TODO: Are we running at the highest clock rate?
  spi_send((uint8_t)(packet>>8));  // Transmit 16 bits 8 bits at a time
  spi_send((uint8_t)packet);

  PORTB |= _BV(3);    // set PIN PB3 : SS / CS HIGH again

  PORTB |= _BV(2);    // set PIN PB2 : SCLK HIGH


}



/**
 * Initializes the AD9833 and the relevant variables.
 */

void ad9833_init(void)
    {

    // set half awake state
    ad9833_send((1<<AD_SLEEP12)|(1<<AD_RESET));
    ad_settings.command_reg |= (1<<AD_SLEEP12);

    //set some nice default values
    ad9833_set_frequency(0, 0);
    ad9833_set_frequency(1, 0);
    ad9833_set_phase(0, 0);
    ad9833_set_phase(1, 0);
    ad9833_set_freq_out(0);
    ad9833_set_phase_out(0);

}


/** 
 * Sets the ad9833 output waveform to the one given as a parameter.
 * \param mode possible values:
 *      - AD_OFF
 *      - AD_TRIANGLE
 *      - AD_SQUARE
 *      - AD_SINE
 */
void ad9833_set_mode(uint8_t mode){
    ad_settings.mode = mode;
    switch (mode){
        case AD_OFF:
            ad_settings.command_reg |= (1<<AD_SLEEP12);
            ad_settings.command_reg |= (1<<AD_SLEEP1);
            break;
        case AD_TRIANGLE:
            ad_settings.command_reg &= ~(1<<AD_OPBITEN);
            ad_settings.command_reg |=  (1<<AD_MODE);
            ad_settings.command_reg &= ~(1<<AD_SLEEP12);
            ad_settings.command_reg &= ~(1<<AD_SLEEP1);
            break;
        case AD_SQUARE:
            ad_settings.command_reg |=  (1<<AD_OPBITEN);
            ad_settings.command_reg &= ~(1<<AD_MODE);
            ad_settings.command_reg |=  (1<<AD_DIV2);
            ad_settings.command_reg &= ~(1<<AD_SLEEP12);
            ad_settings.command_reg &= ~(1<<AD_SLEEP1);
            break;
        case AD_SINE:
            ad_settings.command_reg &= ~(1<<AD_OPBITEN);
            ad_settings.command_reg &= ~(1<<AD_MODE);
            ad_settings.command_reg &= ~(1<<AD_SLEEP12);
            ad_settings.command_reg &= ~(1<<AD_SLEEP1);
            break;
    }
    ad9833_send(ad_settings.command_reg);
}


/**
 * Selects which frequency register is used to generate the output.
 * Also used to select FSK.
 * \param phase_out possible values:
 *      - 0 = use phase register 0
 *      - 1 = use phase register 1
 *      - 2 = PSK
 */
void    ad9833_set_freq_out(uint8_t freq_out){
    ad_settings.freq_out = freq_out;
    switch (freq_out){
        case 0:
            ad_settings.command_reg &= ~(1<<AD_FSELECT);
            break;
        case 1:
            ad_settings.command_reg |= (1<<AD_FSELECT);
            break;
        case 2:
            //TODO
            break;
    }
    ad9833_send(ad_settings.command_reg);
}


/**
 * sets the desired ad9833 internal phase register to a value that
 * produces the desired phase.
 *
 * \param reg the desired phase register to be manipulated, either 0 or 1
 * \param phase the desired phase
 */
void ad9833_set_phase(uint8_t reg, double phase){
    uint16_t reg_reg; //probably should be renamed...
    if (reg==1)
        reg_reg = AD_PHASE1;
    else
        reg_reg = AD_PHASE0;

    ad_settings.phase[reg] = phase;

    ad9833_send(reg_reg | AD_PHASE_CALC(ad_settings.phase[reg]));

}

/**
 * Selects which phase register is used to generate the output
 * Also used to select PSK
 * \param phase_out possible values:
 *  - 0 = use phase register 0
 *  - 1 = use phase register 1
 *  - 2 = PSK
 */
void    ad9833_set_phase_out(uint8_t phase_out){
    ad_settings.phase_out = phase_out;
    switch (phase_out){
        case 0:
            ad_settings.command_reg &= ~(1<<AD_PSELECT);
            break;
        case 1:
            ad_settings.command_reg |= (1<<AD_PSELECT);
            break;
        case 2:
            //TODO
            break;
    }
    ad9833_send(ad_settings.command_reg);
}


/**
 * sets the desired ad9833 internal frequency register to a value that
 * produces the desired frequency.
 *
 * \param reg the desired frequency register to be manipulated, either 0 or 1
 * \param freq the desired frequency
 */
void ad9833_set_frequency(uint8_t reg, double freq){
    uint32_t freq_reg;
    uint16_t reg_reg; //probably should be renamed...
    freq_reg = AD_FREQ_CALC(freq);
    ad_settings.freq[reg] = freq;

    if (reg==1)
        reg_reg = AD_FREQ1;
    else
        reg_reg = AD_FREQ0;

    ad9833_send((1<<AD_B28) | ad_settings.command_reg);
    ad9833_send(reg_reg | (0x3FFF&(uint16_t)(freq_reg>>2 )));
    ad9833_send(reg_reg | (0x3FFF&(uint16_t)(freq_reg>>16)));

}




// -----------------------------------------------------------------------
//      MAIN CODE OF THE JAMMER 
// -----------------------------------------------------------------------


void setup() 
  {

  //initializing SPI and AD9833 generator
  spi_init();
  
  delay(100);
  ad9833_init();
  delay(15);

  //set gen to SQUARE
  ad9833_set_mode(AD_SQUARE);
  ad9833_set_frequency(0, 26000);
}

void loop() {

  uint16_t i,j, sequence ; 
    
          // generating RANDOM 5-25Hz INFRASOUND over 25kHZ ULTRASOUND CARRIER 

           sequence = 0;

           for(i=26000; i>24000; i=i - pgm_read_byte(randomized + sequence), sequence++)
                  {
                   ad9833_set_frequency(0, (double)i );
                  }; 
                
           sequence = 0;     
                
           for(i=24000; i<26000; i=i + pgm_read_byte(randomized + sequence), sequence++ )
                  {
                   ad9833_set_frequency(0, (double)i );
                  };                
 
}
