/*
    wearable microphone jammer
	
    Version based on original concept &  development :
    https://github.com/y-x-c/wearable-microphone-jamming
	
    Boards needed : 
    ESP32C3 Super Mini board - please adjust SPI pin CS numbering 
    AD9833 signal generator board
    TPA3116D2 - audio amplifier module + potentiometer

    CONNECTION EXAMPLE FOR ESP32C3 SUPER MINI BOARD
    --------
    AD9833 PCB to ESP32C3 Super Mini :
    SDATA --> pin D6 MOSI  ( GPIO 6 )
    SCLK  --> pin D4 SCK  ( GPIO 4 ) 
    FSYNC --> pin D7 SS (GPIO 7 )  - adjust this pin  FNC_PIN for you Arduino board down below
    DGND  --> GND
    VCC   --> VCC
    
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


// Definition of FSYNC , CHIP SELECT pin in your Arduino board  - CS pin on your ESP32C3 board
#define FNC_PIN 7

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

  // letting watchdog to check
  yield(); 		  
	  
  delay(100);

  // letting watchdog to check
  yield(); 		  
	  
  ad9833_init();
  delay(15);

  // letting watchdog to check
  yield(); 		  
	  
  //set gen to SINE
  // you may also experiment with AD_SQUARE or AD_TRIANGLE
  ad9833_set_mode(AD_SQUARE);
  // set some center frequency for the start
  ad9833_set_frequency(0, 26000);

  // letting watchdog to check
  yield(); 		  
	  
  // set random seed for pseudonumbers generation
  randomSeed(1234);
    
}

void loop() {

     // i = current frequency for the generator
     // j = calculated gap for next frequency
     // x = cut off value - cap value for gap calculation	
	
     uint16_t i,j, x;

          // generating INFRASOUND FM modulation over 25kHZ ULTRASOUND CARRIER
          // for android best is random infrasound
          // for iPhone best is fixed 45Hz value
          // j = 45;
		  
          i = 26000;


	  // choosing random cut off for gap - you may play with this range
	  x = random(20,50);
          // resetting j value 
	  j = 1;
	
          // letting watchdog to check
          yield(); 		  
	
          // going down with frequency to 24000
          while (i >= 24000)
          {
           i = i - j;
           ad9833_set_frequency(0, (double)i );
           // INCREASING FREQUENCY GAP IN INTERVALS - you may play with this range
           // j = j + random(1,5);
           // increasing gap j up to x then from the beginning
           j = (j + 1) % x;          
           // letting watchdog to check
           yield(); 		  
          };

	  // letting watchdog to check
          yield(); 		  

	  // going up with frequency to 26000 again
          // starting frequency for swinging near low border
          i = 24000;
	  // choosing random cut off for gap - you may play with this range
	  x = random(20,50);
          // resetting j value 
	  j = 1;
		  
          // generating INFRASOUND FM modulation over 25kHZ ULTRASOUND CARRIER
          while (i <= 26000)
          {
           i = i + j;
           ad9833_set_frequency(0, (double)i );
           // INCREASING FREQUENCY GAP IN INTERVALS - you may play with this range
           // j = j + random(1,5);          
           // increasing gap j up to x then from the beginning
           j = (j + 1) % x;          
           // letting watchdog to check
           yield(); 			   
          };
 
}
