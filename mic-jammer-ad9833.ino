/*
    wearable microphone jammer
	
    Version based on original concept &  development :
    https://github.com/y-x-c/wearable-microphone-jamming
	
    Boards needed : 
    Arduino Pro Mini/Micro
    AD9833 signal generator board
    PAM8403 or better - audio amplifier module + potentiometer

    CONNECTION
    --------
    AD9833 PCB to Arduino Pro Mini:
    SDATA --> pin 11 MOSI [note: pin 12 is MISO]
    SCLK --> pin 13 SCK
    FSYNC --> pin 14 A0 (as AD9833 SS)
    DGND --> GND
    VCC --> VCC
    
    AD9833 PCB to TPA3116 AUDIO AMPLIFIER
    AOUT  --> LEFT AUDIO IN + RIGHT AUDIO IN
    AGND  --> AUDIO GND

    REMEMBER TO CONNECT AD9833 TO 5V AND GND PINS OF ARDUINO BOARD
    ARDUINO HAS TO BE POWERED FROM 12V THROUGH Vin PIN
    TPA3116 HAS TO BE CONNECTED TO 12V AND GND
        
    AD8933 code borrowed from Tuomas Nylund function generator project :
    https://github.com/tuomasnylund/function-gen
		
*/

#include <Arduino.h>
#include <SPI.h>


// Definition of FSYNC , CHIP SELECT pin in Arduino board
#define FNC_PIN A0

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


// randomized infrasound 400 values to swipe between 24kHz - 26kHz
const uint8_t  randomized[] PROGMEM = 
  {
  /*
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
11,25,12,19,13,6,9,21,10,12,8,14,5,7,22,21,7,15,8,22,18,17,6,16,7 
*/
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,19,18,17,16,15,
14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,
4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,19,18,17,16,15,14,13,
12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,
6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,19,18,17,16,15,14,13,12,
11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,
6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,19,18,17,16,15,14,13,12,
11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,
6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,19,18,17,16,15,14,13,12,
11,10,9,8,7,6,5,4,3,2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,2,3,4,
5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20
};


/**
 * a wrapper function for sending 16-bit SPI packets.
 * \param packet 16-bit value to be sent over SPI.
 */
static inline void ad9833_send(uint16_t packet)
{

  digitalWrite(FNC_PIN,LOW);   // FNCpin low to write to AD9833 over SPI 

  //delayMicroseconds(2); // Some delay may be needed

  // TODO: Are we running at the highest clock rate?
  SPI.transfer((uint8_t)(packet>>8));  // Transmit 16 bits 8 bits at a time
  SPI.transfer((uint8_t)packet);

  digitalWrite(FNC_PIN,HIGH);    // Write to SPI done - bring up this pin
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
  pinMode(FNC_PIN,OUTPUT);
  digitalWrite(FNC_PIN,HIGH);   

  SPI.begin();

   //We set the mode here, because other hardware may be doing SPI also
  SPI.setDataMode(SPI_MODE2);
  
  delay(100);
  ad9833_init();
  delay(15);

  //set gen to SQUARE
  // you may also experiment with AD_SINE or AD_TRIANGLE
  ad9833_set_mode(AD_SQUARE);
  // set some center frequency for the start
  ad9833_set_frequency(0, 26000);
	
}

void loop() {
  
  uint16_t i,j, sequence ; 
    
           // generating RANDOMIZED 5-25Hz INFRASOUND over 25kHZ ULTRASOUND CARRIER 


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
