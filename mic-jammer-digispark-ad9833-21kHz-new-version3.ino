/*
    microphone jammer 
    - ATTINY85 / DIGISPARK version

    THIS IS 21KHZ VERSION !!!

    
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
  // you may also experiment with AD_SINE or AD_TRIANGLE for better effects
  // AD_SINE is almost not audible for humans
  ad9833_set_mode(AD_SQUARE);
  ad9833_set_frequency(0, 22000);

  // set random seed for pseudonumbers generation
  randomSeed(1234);
    
}

void loop() {

     uint16_t i,j;

          // generating INFRASOUND FM modulation over 21kHZ ULTRASOUND CARRIER
          // for android best is random infrasound
          // for iPhone best is fixed 45Hz value
          // j = 45;
          // starting frequency for swinging near high border 
          i = random(21500,22000);
          // going down with frequency to 20000
          while (i >= 20000)
          {
           i = i - j;
           ad9833_set_frequency(0, (double)i );
           // INCREASING FREQUENCY GAP IN RANDOM INTERVALS - you may play wit this range
           j = j + random(1,5);          
          };

          // going up with frequency to 22000 again
          // starting frequency for swinging near low border
          i = random(20000,20500);
          // generating INFRASOUND FM modulation over 21kHZ ULTRASOUND CARRIER
          while (i <= 22000)
          {
           i = i + j;
           ad9833_set_frequency(0, (double)i );
           // INCREASING FREQUENCY GAP IN RANDOM INTERVALS - you may play wit this range
           j = j + random(1,5);          
          };
 
}
