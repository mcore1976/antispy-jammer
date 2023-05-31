//
// CC1101 interactive terminal tool
// allows for sending / receiving data over serial port
// on selected radio channel, modulation, ..
//
// (C) Adam Loboda '2023 , adam.loboda@wp.pl
//  
// based on great SmartRC library by Little_S@tan
// Please download ZIP from 
// https://github.com/LSatan/SmartRC-CC1101-Driver-Lib
// and attach it as ZIP library for Arduino
//
// Also uses Arduino Command Line interpreter by Edgar Bonet
// from https://gist.github.com/edgar-bonet/607b387260388be77e96
//
// This code will ONLY work with Arduino Pro Micro 3.3V 8MHz
//



#include <ELECHOUSE_CC1101_SRC_DRV.h>


#define CCBUFFERSIZE 64

#define RECORDINGBUFFERSIZE 512

#include <avr/pgmspace.h>

#define BUF_LENGTH 256  /* Buffer for the incoming command. */

static bool do_echo = true;

// buffer for receiving  CC1101
byte ccreceivingbuffer[CCBUFFERSIZE] = {0};

// buffer for sending  CC1101
byte ccsendingbuffer[CCBUFFERSIZE] = {0};

// buffer for recording and replaying of many frames
byte bigrecordingbuffer[RECORDINGBUFFERSIZE] = {0};

// position in big recording buffer
int bigrecordingbufferpos = 0; 


// buffer for hex to ascii conversions 
byte textbuffer[BUF_LENGTH];



// The RX LED has a defined Arduino Pro Micro pin
int RXLED = 17; 

// check if CLI receiving mode enabled
int receivingmode = 0; 

// check if CLI jamming mode enabled
int jammingmode = 0; 

// check if CLI recording mode enabled
int recordingmode = 0; 


// convert char table to string with hex numbers

void asciitohex(byte *ascii_ptr, byte *hex_ptr,int len)
{
    byte i,j,k;
    for(i = 0; i < len; i++)
    {
      // high byte first
      j = ascii_ptr[i] / 16;
      if (j>9) 
         { k = j - 10 + 65; }
      else 
         { k = j + 48; }
      hex_ptr[2*i] = k ;
      // low byte second
      j = ascii_ptr[i] % 16;
      if (j>9) 
         { k = j - 10 + 65; }
      else
         { k = j + 48; }
      hex_ptr[(2*i)+1] = k ; 
    };
    hex_ptr[(2*i)+2] = '\0' ; 
}


// convert string with hex numbers to array of bytes

void  hextoascii(byte *ascii_ptr, byte *hex_ptr,int len)
{
    byte i,j;
    for(i = 0; i < (len/2); i++)
     { 
     j = hex_ptr[i*2];
     if ((j>47) && (j<58))  ascii_ptr[i] = (j - 48) * 16;
     if ((j>64) && (j<71))  ascii_ptr[i] = (j - 55) * 16;
     if ((j>96) && (j<103)) ascii_ptr[i] = (j - 87) * 16;
     j = hex_ptr[i*2+1];
     if ((j>47) && (j<58))  ascii_ptr[i] = ascii_ptr[i]  + (j - 48);
     if ((j>64) && (j<71))  ascii_ptr[i] = ascii_ptr[i]  + (j - 55);
     if ((j>96) && (j<103)) ascii_ptr[i] = ascii_ptr[i]  + (j - 87);
     };
    ascii_ptr[i++] = '\0' ;
}

// Initialize CC1101 board with default settings, you may change your preferences here
static void cc1101initialize(void)
{
     // Following section enables SmartRC CC1101 library 
     // to work with Arduino Pro Micro
     // if using different board, please remove it
     // defining PINs set for Arduino Pro Micro setup
     byte sck = 15;   
     byte miso = 14;
     byte mosi = 16;
     byte ss = 10;
     int gdo0 = 3;
     // initializing library with custom pins selected
     ELECHOUSE_cc1101.setSpiPin(sck, miso, mosi, ss);

    // Main part to tune CC1101 with proper frequency, modulation and encoding    
    ELECHOUSE_cc1101.Init();                // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setGDO0(gdo0);         // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setCCMode(1);          // set config for internal transmission mode.
    ELECHOUSE_cc1101.setModulation(2);      // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setMHZ(433.92);        // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setDeviation(47.60);   // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.
    ELECHOUSE_cc1101.setChannel(0);         // Set the Channelnumber from 0 to 255. Default is cahnnel 0.
    ELECHOUSE_cc1101.setChsp(199.95);       // The channel spacing is multiplied by the channel number CHAN and added to the base frequency in kHz. Value from 25.39 to 405.45. Default is 199.95 kHz.
    ELECHOUSE_cc1101.setRxBW(812.50);       // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
    ELECHOUSE_cc1101.setDRate(1.2);         // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!
    ELECHOUSE_cc1101.setPA(10);             // Set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!
    ELECHOUSE_cc1101.setSyncMode(2);        // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
    ELECHOUSE_cc1101.setSyncWord(127, 127); // Set sync word. Must be the same for the transmitter and receiver. (Syncword high, Syncword low)
    ELECHOUSE_cc1101.setAdrChk(0);          // Controls address check configuration of received packages. 0 = No address check. 1 = Address check, no broadcast. 2 = Address check and 0 (0x00) broadcast. 3 = Address check and 0 (0x00) and 255 (0xFF) broadcast.
    ELECHOUSE_cc1101.setAddr(0);            // Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).
    ELECHOUSE_cc1101.setWhiteData(0);       // Turn data whitening on / off. 0 = Whitening off. 1 = Whitening on.
    ELECHOUSE_cc1101.setPktFormat(0);       // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX. 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins. 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX. 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
    ELECHOUSE_cc1101.setLengthConfig(1);    // 0 = Fixed packet length mode. 1 = Variable packet length mode. 2 = Infinite packet length mode. 3 = Reserved
    ELECHOUSE_cc1101.setPacketLength(0);    // Indicates the packet length when fixed packet length mode is enabled. If variable packet length mode is used, this value indicates the maximum packet length allowed.
    ELECHOUSE_cc1101.setCrc(0);             // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.
    ELECHOUSE_cc1101.setCRC_AF(0);          // Enable automatic flush of RX FIFO when CRC is not OK. This requires that only one packet is in the RXIFIFO and that packet length is limited to the RX FIFO size.
    ELECHOUSE_cc1101.setDcFilterOff(0);     // Disable digital DC blocking filter before demodulator. Only for data rates ≤ 250 kBaud The recommended IF frequency changes when the DC blocking is disabled. 1 = Disable (current optimized). 0 = Enable (better sensitivity).
    ELECHOUSE_cc1101.setManchester(0);      // Enables Manchester encoding/decoding. 0 = Disable. 1 = Enable.
    ELECHOUSE_cc1101.setFEC(0);             // Enable Forward Error Correction (FEC) with interleaving for packet payload (Only supported for fixed packet length mode. 0 = Disable. 1 = Enable.
    ELECHOUSE_cc1101.setPRE(0);             // Sets the minimum number of preamble bytes to be transmitted. Values: 0 : 2, 1 : 3, 2 : 4, 3 : 6, 4 : 8, 5 : 12, 6 : 16, 7 : 24
    ELECHOUSE_cc1101.setPQT(0);             // Preamble quality estimator threshold. The preamble quality estimator increases an internal counter by one each time a bit is received that is different from the previous bit, and decreases the counter by 8 each time a bit is received that is the same as the last bit. A threshold of 4∙PQT for this counter is used to gate sync word detection. When PQT=0 a sync word is always accepted.
    ELECHOUSE_cc1101.setAppendStatus(0);    // When enabled, two status bytes will be appended to the payload of the packet. The status bytes contain RSSI and LQI values, as well as CRC OK.
}


// Execute a complete CC1101 command.


static void exec(char *cmdline)
{ 
        
    char *command = strsep(&cmdline, " ");
    int setting, len;
    float settingf1;
    float settingf2;
    
  // identification of the command & actions
      
    if (strcmp_P(command, PSTR("help")) == 0) {
        Serial.println(F(
          "setmodulation <mode>         // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.\r\n\r\n"
          "setmhz <frequency>           // Here you can set your basic frequency. default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ.\r\n\r\n"
          "setdeviation <deviation>     // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.\r\n\r\n"
          "setchannel <channel>         // Set the Channelnumber from 0 to 255. Default is cahnnel 0.\r\n\r\n"
          "setchsp <spacing>            // The channel spacing is multiplied by the channel number CHAN and added to the base frequency in kHz. Value from 25.39 to 405.45. Default is 199.95 kHz. \r\n\r\n"
          "setrxbw <Receive bandwidh>   // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.\r\n\r\n"
         ));
        Serial.println(F(
          "setdrate <datarate>          // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!\r\n\r\n"
          "setpa <power value>          // Set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!\r\n\r\n"
          "setsyncmode  <sync mode>     // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.\r\n\r\n"
          "setsyncword <LOW, HIGH>      // Set sync word. Must be the same for the transmitter and receiver. (Syncword high, Syncword low)\r\n\r\n"
          "setadrchk <address check>    // Controls address check configuration of received packages. 0 = No address check. 1 = Address check, no broadcast. 2 = Address check and 0 (0x00) broadcast. 3 = Address check and 0 (0x00) and 255 (0xFF) broadcast.\r\n\r\n"
          "setaddr <address>            // Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).\r\n\r\n"
         ));
        Serial.println(F(
          "setwhitedata <whitening>     // Turn data whitening on / off. 0 = Whitening off. 1 = Whitening on.\r\n\r\n"
          "setpktformat <pkt format>    // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX. 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins. 2 = Random TX mode; sends random data using PN9 generator.  3 = Asynchronous serial mode\r\n\r\n"
          "setlengthconfig <mode>       // 0 = Fixed packet length mode. 1 = Variable packet length mode. 2 = Infinite packet length mode. 3 = Reserved \r\n\r\n"
          "setpacketlength <mode>       // Indicates the packet length when fixed packet length mode is enabled. If variable packet length mode is used, this value indicates the maximum packet length allowed.\r\n\r\n"
          "setcrc <mode>                // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.\r\n\r\n"
          "setcrcaf <mode>              // Enable automatic flush of RX FIFO when CRC is not OK. This requires that only one packet is in the RXIFIFO and that packet length is limited to the RX FIFO size.\r\n\r\n"
          "setdcfilteroff <mode>        // Disable digital DC blocking filter before demodulator. Only for data rates ≤ 250 kBaud The recommended IF frequency changes when the DC blocking is disabled. 1 = Disable (current optimized). 0 = Enable (better sensitivity).\r\n\r\n"
         ));
        Serial.println(F(
          "setmanchester <mode>         // Enables Manchester encoding/decoding. 0 = Disable. 1 = Enable.\r\n\r\n"
          "setfec <mode>                // Enable Forward Error Correction (FEC) with interleaving for packet payload (Only supported for fixed packet length mode. 0 = Disable. 1 = Enable.\r\n\r\n"
          "setpre <mode>                // Sets the minimum number of preamble bytes to be transmitted. Values: 0 : 2, 1 : 3, 2 : 4, 3 : 6, 4 : 8, 5 : 12, 6 : 16, 7 : 24\r\n\r\n"
          "setpqt <mode>                // Preamble quality estimator threshold. \r\n\r\n"
          "setappendstatus <mode>       // When enabled, two status bytes will be appended to the payload of the packet. The status bytes contain RSSI and LQI values, as well as CRC OK.\r\n\r\n"
          "receive <mode>               // Enable or disable printing of received RF packets on serial terminal. 1 = enabled, 0 = disabled\r\n\r\n"
          "transmit <times> <hex-vals>  // Send the same packet of 64 hex values over RF \r\n\r\n"
           ));
        Serial.println(F(
         "jamming <mode>               // Enable or disable continous jamming on selected band. 1 = enabled, 0 = disabled\r\n\r\n"
         "record <mode>                // Enable or disable recording frames in the buffer. 1 = enabled, 0 = disabled\r\n\r\n"
         "replay <N>                   // Replay N last recorded frames.\r\n\r\n"
         "echo <mode>                  // Enable or disable Echo on serial terminal. 1 = enabled, 0 = disabled\r\n\r\n"
         "x                            // Stops jamming/receiving/recording packets.\r\n\r\n"
         "init                         // Restarts CC1101 board with default parameters\r\n\r\n"
         ));
            
    } else if (strcmp_P(command, PSTR("setmodulation")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setModulation(setting);
        Serial.print(F("\r\nModulation: "));
        if (setting == 0) { Serial.print(F("2-FSK")); }
        else if (setting == 1) { Serial.print(F("GFSK")); }
        else if (setting == 2) { Serial.print(F("ASK/OOK")); }
        else if (setting == 3) { Serial.print(F("4-FSK")); }
        else if (setting == 4) { Serial.print(F("MSK")); };  
        Serial.print(F(" \r\n"));
        
    } else if (strcmp_P(command, PSTR("setmhz")) == 0) {
        settingf1 = atof(cmdline);
        ELECHOUSE_cc1101.setMHZ(settingf1);
        Serial.print(F("\r\nFrequency: "));
        Serial.print(settingf1);
        Serial.print(F(" MHz\r\n"));
        
    } else if (strcmp_P(command, PSTR("setdeviation")) == 0) {
        settingf1 = atof(cmdline);
        ELECHOUSE_cc1101.setDeviation(settingf1);
        Serial.print(F("\r\nDeviation: "));
        Serial.print(settingf1);
        Serial.print(F(" KHz\r\n"));        
        
    } else if (strcmp_P(command, PSTR("setchannel")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setChannel(setting);
        Serial.print(F("\r\nChannel:"));
        Serial.print(setting);
        Serial.print(F("\r\n"));        
        
    } else if (strcmp_P(command, PSTR("setchsp")) == 0) {
        settingf1 = atof(cmdline);
        ELECHOUSE_cc1101.setChsp(settingf1);
        Serial.print(F("\r\nChann spacing: "));
        Serial.print(settingf1);
        Serial.print(F(" kHz\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setrxbw")) == 0) {
        settingf1 = atof(cmdline);
        ELECHOUSE_cc1101.setRxBW(settingf1);
        Serial.print(F("\r\nRX bandwidth: "));
        Serial.print(settingf1);
        Serial.print(F(" kHz \r\n"));  
        
    } else if (strcmp_P(command, PSTR("setdrate")) == 0) {
        settingf1 = atof(cmdline);
        ELECHOUSE_cc1101.setDRate(settingf1);
        Serial.print(F("\r\nDatarate: "));
        Serial.print(settingf1);
        Serial.print(F(" kbaud\r\n"));  

    } else if (strcmp_P(command, PSTR("setpa")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setPA(setting);
        Serial.print(F("\r\nTX PWR: "));
        Serial.print(setting);
        Serial.print(F(" dBm\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setsyncmode")) == 0) {
        int setting = atoi(cmdline);
        ELECHOUSE_cc1101.setSyncMode(setting);
        Serial.print(F("\r\nSynchronization: "));
        if (setting == 0) { Serial.print(F("No preamble")); }
        else if (setting == 1) { Serial.print(F("16 sync bits")); }
        else if (setting == 2) { Serial.print(F("16/16 sync bits")); }
        else if (setting == 3) { Serial.print(F("30/32 sync bits")); }
        else if (setting == 4) { Serial.print(F("No preamble/sync, carrier-sense")); }
        else if (setting == 5) { Serial.print(F("15/16 + carrier-sense")); }
        else if (setting == 6) { Serial.print(F("16/16 + carrier-sense")); }
        else if (setting == 7) { Serial.print(F("30/32 + carrier-sense")); };
        Serial.print(F("\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setsyncword")) == 0) {
        int lowword = atoi(strsep(&cmdline, " "));
        int highword = atoi(cmdline);
        ELECHOUSE_cc1101.setSyncWord(lowword, highword);
        Serial.print(F("\r\nSynchronization:\r\n"));
        Serial.print(F("high = "));
        Serial.print(highword);
        Serial.print(F("\r\nlow = "));
        Serial.print(lowword);
        Serial.print(F("\r\n"));  

    } else if (strcmp_P(command, PSTR("setadrchk")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setAdrChk(setting);
        Serial.print(F("\r\nAddress checking:"));
        if (setting == 0) { Serial.print(F("No adr chk")); }
        else if (setting == 1) { Serial.print(F("Adr chk, no bcast")); }
        else if (setting == 2) { Serial.print(F("Adr chk and 0 bcast")); }
        else if (setting == 3) { Serial.print(F("Adr chk and 0 and FF bcast")); };
        Serial.print(F("\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setaddr")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setAddr(setting);
        Serial.print(F("\r\nAddress: "));
        Serial.print(setting);
        Serial.print(F("\r\n"));  

    } else if (strcmp_P(command, PSTR("setwhitedata")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setWhiteData(setting);
        Serial.print(F("\r\nWhitening "));
        if (setting == 0) { Serial.print(F("OFF")); }
        else if (setting == 1) { Serial.print(F("ON")); }
        Serial.print(F("\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setpktformat")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setPktFormat(setting);
        Serial.print(F("\r\nPacket format: "));
        if (setting == 0) { Serial.print(F("Normal mode")); }
        else if (setting == 1) { Serial.print(F("Synchronous serial mode")); }
        else if (setting == 2) { Serial.print(F("Random TX mode")); }
        else if (setting == 3) { Serial.print(F("Asynchronous serial mode")); };
        Serial.print(F("\r\n"));  
  
    } else if (strcmp_P(command, PSTR("setlengthconfig")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setLengthConfig(setting);
        Serial.print(F("\r\nPkt length mode: "));
        if (setting == 0) { Serial.print(F("Fixed")); }
        else if (setting == 1) { Serial.print(F("Variable")); }
        else if (setting == 2) { Serial.print(F("Infinite")); }
        else if (setting == 3) { Serial.print(F("Reserved")); };
        Serial.print(F("\r\n"));  
  
    } else if (strcmp_P(command, PSTR("setpacketlength")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setPacketLength(setting);
        Serial.print(F("\r\nPkt length: "));
        Serial.print(setting);
        Serial.print(F(" bytes\r\n"));  
        
    } else if (strcmp_P(command, PSTR("setcrc")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setCrc(setting);
        Serial.print(F("\r\nCRC checking: "));
        if (setting == 0) { Serial.print(F("Disabled")); }
        else if (setting == 1) { Serial.print(F("Enabled")); };
        Serial.print(F("\r\n")); 
        
    } else if (strcmp_P(command, PSTR("setcrcaf")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setCRC_AF(setting);
        Serial.print(F("\r\nCRC Autoflush: "));
        if (setting == 0) { Serial.print(F("Disabled")); }
        else if (setting == 1) { Serial.print(F("Enabled")); };
         Serial.print(F("\r\n")); 
        
     } else if (strcmp_P(command, PSTR("setdcfilteroff")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setDcFilterOff(setting);
        Serial.print(F("\r\nDC filter: "));
        if (setting == 0) { Serial.print(F("Enabled")); }
        else if (setting == 1) { Serial.print(F("Disabled")); };
        Serial.print(F("\r\n")); 

     } else if (strcmp_P(command, PSTR("setmanchester")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setManchester(setting);
        Serial.print(F("\r\nManchester coding: "));
        if (setting == 0) { Serial.print(F("Disabled")); }
        else if (setting == 1) { Serial.print(F("Enabled")); };
        Serial.print(F("\r\n")); 

     } else if (strcmp_P(command, PSTR("setfec")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setFEC(setting);
        Serial.print(F("\r\nForward Error Correction: "));
        if (setting == 0) { Serial.print(F("Disabled")); }
        else if (setting == 1) { Serial.print(F("Enabled")); };
        Serial.print(F("\r\n")); 
        
     } else if (strcmp_P(command, PSTR("setpre")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setPRE(setting);
        Serial.print(F("\r\nMinimum preamble bytes:"));
        Serial.print(setting);
        Serial.print(F(" means 0 = 2 bytes, 1 = 3b, 2 = 4b, 3 = 6b, 4 = 8b, 5 = 12b, 6 = 16b, 7 = 24 bytes\r\n")); 
        Serial.print(F("\r\n")); 

      } else if (strcmp_P(command, PSTR("setpqt")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setPQT(setting);
        Serial.print(F("\r\nPQT: "));
        Serial.print(setting);
        Serial.print(F("\r\n")); 

       } else if (strcmp_P(command, PSTR("setappendstatus")) == 0) {
        setting = atoi(cmdline);
        ELECHOUSE_cc1101.setAppendStatus(setting);
        Serial.print(F("\r\nStatus bytes appending: "));
        if (setting == 0) { Serial.print(F("Enabled")); }
        else if (setting == 1) { Serial.print(F("Disabled")); };
        Serial.print(F("\r\n")); 

       } else if (strcmp_P(command, PSTR("receive")) == 0) {
        receivingmode = atoi(cmdline);
        Serial.print(F("\r\nReceiving and printing RF packet changed to "));
        if (receivingmode == 0) { Serial.print(F("Disabled")); }
        else if (receivingmode == 1)
               { Serial.print(F("Enabled")); 
                 jammingmode = 0; };
        Serial.print(F("\r\n")); 
 

       } else if (strcmp_P(command, PSTR("jamming")) == 0) {
        jammingmode = atoi(cmdline);
        Serial.print(F("\r\nJamming changed to "));
        if (jammingmode == 0) { Serial.print(F("Disabled")); }
        else if (jammingmode == 1) 
               { Serial.print(F("Enabled")); 
                 receivingmode = 0; };
        Serial.print(F("\r\n")); 
 

       } else if (strcmp_P(command, PSTR("transmit")) == 0) {
        int setting = atoi(strsep(&cmdline, " "));
        // convert hex array to set of bytes
        if ((strlen(cmdline)<64) && (strlen(cmdline)>0) )
        { 
                hextoascii((byte *)textbuffer, cmdline, strlen(cmdline));        
                memcpy(ccsendingbuffer, textbuffer, strlen(cmdline)/2 );
                ccsendingbuffer[strlen(cmdline)/2] = 0x00;       
                Serial.print("\r\nTransmitting RF packets.\r\n ");
                // blink LED RX - only for Arduino Pro Micro
                digitalWrite(RXLED, LOW);   // set the RX LED ON
                for (int i=0; i<setting; i++)  
                     {
                      // send these data to radio over CC1101
                      ELECHOUSE_cc1101.SendData(ccsendingbuffer, (byte)(strlen(cmdline)/2));
                      };
                // blink LED RX - only for Arduino Pro Micro
                digitalWrite(RXLED, HIGH);   // set the RX LED OFF    
                // for DEBUG only
                asciitohex((byte *)ccsendingbuffer, (byte *)textbuffer,  strlen(cmdline)/2 );
                Serial.print(F("Sent frame: "));
                Serial.print((char *)textbuffer);
                Serial.print(F("\r\n")); }
         else { Serial.print(F("Wrong parameters.\r\n")); };

    } else if (strcmp_P(command, PSTR("record")) == 0) {
        recordingmode = atoi(cmdline);
        Serial.print(F("\r\nRecording mode set to "));
        if (recordingmode == 0) 
            { 
               Serial.print(F("Disabled")); 
               bigrecordingbufferpos = 0; 
             }
        else if (recordingmode == 1)
            { Serial.print(F("Enabled"));
               bigrecordingbufferpos = 0;
               // flush buffer for recording 
               for (int i = 0; i < RECORDINGBUFFERSIZE; i++)
                    { bigrecordingbuffer[RECORDINGBUFFERSIZE] = 0; };
               };
        Serial.print(F("\r\n")); 
 

       } else if (strcmp_P(command, PSTR("replay")) == 0) {
        setting = atoi(strsep(&cmdline, " ")); 
        Serial.print(F("\r\nReplaying recorded frames.\r\n "));
        // blink LED RX - only for Arduino Pro Micro
        digitalWrite(RXLED, LOW);   // set the RX LED ON
        // rewind recording buffer position to the beginning
        bigrecordingbufferpos = 0;
        // start reading and sending frames from the buffer : FIFO
        for (int i=0; i<setting; i++)  
             { 
               // read length of the recorded frame first from the buffer
               len = bigrecordingbuffer[bigrecordingbufferpos];
               // take next frame from the buffer  for replay
               memcpy(ccsendingbuffer, &bigrecordingbuffer[bigrecordingbufferpos + 1], bigrecordingbuffer[bigrecordingbufferpos] );             
               // send these data to radio over CC1101
               ELECHOUSE_cc1101.SendData(ccsendingbuffer, (byte)bigrecordingbuffer[bigrecordingbufferpos]);
               // increase position to the buffer and check exception
               bigrecordingbufferpos = bigrecordingbufferpos + 1 + len;
               if ( bigrecordingbufferpos > RECORDINGBUFFERSIZE) break;
               // 
              };
        // blink LED RX - only for Arduino Pro Micro
        digitalWrite(RXLED, HIGH);   // set the RX LED OFF    
        Serial.print(F("Done.\r\n"));
        // rewind buffer position
        bigrecordingbufferpos = 0;

    } else if (strcmp_P(command, PSTR("echo")) == 0) {
        do_echo = atoi(cmdline);

        // command 'x' stops jamming, receiveing, recording...
    } else if (strcmp_P(command, PSTR("x")) == 0) {
        receivingmode = 0;
        jammingmode = 0;
        recordingmode = 0;
        Serial.print(F("\r\n"));

        // command 'init' initializes board with default settings
    } else if (strcmp_P(command, PSTR("init")) == 0) {
        // init cc1101
        cc1101initialize();
        // give feedback
        Serial.print(F("CC1101 initialized\r\n"));
          
    } else {
        Serial.print(F("Error: Unknown command: "));
        Serial.println(command);
    }
}


void setup() {

     // initialize USB Serial Port CDC
     Serial.begin(115200);

     while (!Serial) {
        ; // wait until USB CDC port connects... Needed for Leonardo only
                     }
     Serial.println(F("CC1101 terminal tool connected, use 'help' for list of commands...\n\r"));
     Serial.println(F("(C) Adam Loboda 2023\n\r  "));

     Serial.println();  // print CRLF

     // Arduino Pro Micro - RXLED diode will be used for debug blinking
     pinMode(RXLED, OUTPUT);  // Set RX LED as an output

     // initialize CC1101 module with preffered parameters
     cc1101initialize();

     // setup variables
     bigrecordingbufferpos = 0;
}


void loop() {

  // index for serial port characters
  int i = 0;

    /* Process incoming commands. */
    while (Serial.available()) {
        static char buffer[BUF_LENGTH];
        static int length = 0;

        int data = Serial.read();
        if (data == '\b' || data == '\177') {  // BS and DEL
            if (length) {
                length--;
                if (do_echo) Serial.write("\b \b");
            }
        }
        else if (data == '\r') {
            if (do_echo) Serial.write("\r\n");    // output CRLF
            buffer[length] = '\0';
            if (length) exec(buffer);
            length = 0;
        }
        else if (length < BUF_LENGTH - 1) {
            buffer[length++] = data;
            if (do_echo) Serial.write(data);
        }
    };

    /* Process RF received packets */
   
   //Checks whether something has been received.
  if (ELECHOUSE_cc1101.CheckReceiveFlag() && (receivingmode == 1 || recordingmode == 1) )
      {
       // blink LED RX - only for Arduino Pro Micro
       digitalWrite(RXLED, LOW);   // set the RX LED ON

       //CRC Check. If "setCrc(false)" crc returns always OK!
       if (ELECHOUSE_cc1101.CheckCRC())
          { 
            //Get received Data and calculate length
            int len = ELECHOUSE_cc1101.ReceiveData(ccreceivingbuffer);

            if ( (receivingmode == 1) && (len < CCBUFFERSIZE ) )
               {
                   // put NULL at the end of char buffer
                   ccreceivingbuffer[len] = '\0';
                   // flush textbuffer
                   for (int i = 0; i < BUF_LENGTH; i++)
                    { textbuffer[i] = 0; };
                   
                   //Print received packet as set of hex values
                   asciitohex((byte *)ccreceivingbuffer, (byte *)textbuffer,  len);
                   Serial.print(F("Received payload: "));
                   Serial.print((char *)textbuffer);
                   Serial.print(F("\r\n"));
                   // Serial.print((char *) ccreceivingbuffer);
               };   // end of handling receiving mode 
               
            if ( (recordingmode == 1) && (len < CCBUFFERSIZE ) )
               { 
                // copy the frame from receiving buffer for replay - only if it fits
                if (( bigrecordingbufferpos + len ) < RECORDINGBUFFERSIZE) 
                     { // put info about number of bytes
                      bigrecordingbuffer[bigrecordingbufferpos] = len; 
                      bigrecordingbufferpos++;
                      // next - copy current frame and increase 
                      memcpy(&bigrecordingbuffer[bigrecordingbufferpos], ccreceivingbuffer, len );
                      // increase position in big recording buffer for next frame
                      bigrecordingbufferpos = bigrecordingbufferpos + len; 
                      ccreceivingbuffer[len] = '\0';  
                      // flush textbuffer
                      for (int i = 0; i < BUF_LENGTH; i++)
                          { textbuffer[i] = 0; };           
                      asciitohex((byte *)ccreceivingbuffer, (byte *)textbuffer,  len);
                      Serial.print(F("Recorded: "));
                      Serial.print((char *)textbuffer);
                      Serial.print(F("\r\n"));
                     }
                else {
                    Serial.print(F("recording buffer full!\r\n"));
                     };
                
               };   // end of handling frame recording mode 
 
          };   // end of CRC check IF

       // blink LED RX - only for Arduino Pro Micro
       digitalWrite(RXLED, HIGH);   // set the RX LED OFF

      };   // end of Check receive flag if

      // if jamming mode activate continously send something over RF...
      if ( jammingmode == 1)
      { 
        // populate cc1101 sending buffer with random values
        randomSeed(analogRead(0));
        for (i = 0; i<64; i++)
           { ccsendingbuffer[i] = (byte)random(255);  };        
        // blink LED RX - only for Arduino Pro Micro
        digitalWrite(RXLED, LOW);   // set the RX LED ON
        // send these data to radio over CC1101
        ELECHOUSE_cc1101.SendData(ccsendingbuffer,60);
        // blink LED RX - only for Arduino Pro Micro
        digitalWrite(RXLED, HIGH);   // set the RX LED OFF    
      };
 
}  // end of main LOOP
