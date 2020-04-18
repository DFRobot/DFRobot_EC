/*
 * file DFRobot_EC.cpp
 * @ https://github.com/DFRobot/DFRobot_EC
 *
 * Arduino library for Gravity: Analog Electrical Conductivity Sensor / Meter Kit V2 (K=1), SKU: DFR0300
 *
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * version  V1.01
 * date  2018-06
 * 
 * version  V1.1
 * date  2020-04
 * Changes the memory addressing to allow multiple EC sensors
 */


#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "DFRobot_EC.h"
#include <EEPROM.h>

#define EEPROM_write(address, value) {int i = 0; byte *pp = (byte*)&(value);for(; i < sizeof(value); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, value)  {int i = 0; byte *pp = (byte*)&(value);for(; i < sizeof(value); i++) pp[i]=EEPROM.read(address+i);}

// The start address of the K value stored in the EEPROM. Start at 96 after the 12 values for the pH sensors
#define KVALUEADDR 96    

#define RES2 820.0
#define ECREF 200.0

// Default construtor to ensure backwards compatibility
DFRobot_EC::DFRobot_EC()
{
    // As no pin was provided we will use the data for pin A0
    this->_pin            = A0;

    // Set the address
    // For each EC sensor we need to store 2 floats (EC 1.413us/cm and EC 12.88ms/cm)
    // This will be 8byte per sensor and the largest arduino has 12 analogue ports
    // So let's start at address 0 and go up by 8b for each analogue port. This will use upto 96b 
    // For the EC library we will start after PH addresses
    this->_address        = KVALUEADDR + (sizeof(float) * 2 * EPinToAddressMap[this->_pin]);

    // Buffer solution EC 1.413us/cm at 25C
    this->_kvalueLow              = 1.0;

    // Buffer solution EC 12.88ms/cm at 25C
    this->_kvalueHigh             = 1.0;

    // Initialise the rest of the values with an initial starting value
    this->_cmdReceivedBufferIndex = 0;
    this->_voltage                = 0.0;
    this->_temperature            = 25.0;
    this->_ecvalue                = 0.0;
    this->_kvalue                 = 1.0;
} 

// Updated construtor to allow multiple EC sensors
DFRobot_EC::DFRobot_EC(int ecPin);
{
    // Set the pin to the supplied value
    this->_pin                    = ecPin;

    // Set the address
    // For each EC sensor we need to store 2 floats (EC 1.413us/cm and EC 12.88ms/cm)
    // This will be 8byte per sensor and the largest arduino has 12 analogue ports
    // So let's start at address 0 and go up by 8b for each analogue port. This will use upto 96b 
    // For the EC library we will start after PH addresses
    this->_address                = KVALUEADDR + (sizeof(float) * 2 * EPinToAddressMap[this->_pin]);

    // Buffer solution EC 1.413us/cm at 25C
    this->_kvalueLow              = 1.0;

    // Buffer solution EC 12.88ms/cm at 25C
    this->_kvalueHigh             = 1.0;

    // Initialise the rest of the values with an initial starting value
    this->_cmdReceivedBufferIndex = 0;
    this->_voltage                = 0.0;
    this->_temperature            = 25.0;
    this->_ecvalue                = 0.0;
    this->_kvalue                 = 1.0;
} 

// Default destructor
DFRobot_EC::~DFRobot_EC()
{
}

// Initialiser
void DFRobot_EC::begin()
{
    // Read the calibrated K value from EEPROM
    EEPROM_read(this->_address, this->_kvalueLow); 
    
    // If the values are all 255 then  write a default value in       
    if(EEPROM.read(this->_address)==0xFF && EEPROM.read(this->_address+1)==0xFF && EEPROM.read(this->_address+2)==0xFF && EEPROM.read(this->_address+3)==0xFF){
        // For new EEPROM, write default value( K = 1.0) to EEPROM
        this->_kvalueLow = 1.0;                       
        EEPROM_write(this->_address, this->_kvalueLow);
    }

    // Read the calibrated K value from EEPRM
    EEPROM_read(this->_address+4, this->_kvalueHigh);     
    
    // If the values are all 255 then  write a default value in
    if(EEPROM.read(this->_address+4)==0xFF && EEPROM.read(this->_address+5)==0xFF && EEPROM.read(this->_address+6)==0xFF && EEPROM.read(this->_address+7)==0xFF){
        // For new EEPROM, write default value( K = 1.0) to EEPROM
        this->_kvalueHigh = 1.0;                      
        EEPROM_write(this->_address+4, this->_kvalueHigh);
    }

    // Set default K value: K = kvalueLow
    this->_kvalue =  this->_kvalueLow;                
}

// Function to read the EC value
float DFRobot_EC::readEC(float voltage, float temperature)
{
    float value = 0;
    float valueTemp = 0;

    this->_rawEC = 1000 * voltage / RES2 / ECREF;
    valueTemp = this->_rawEC * this->_kvalue;

    // Automatic shift process
    // First Range:(0,2); Second Range:(2,20)
    if(valueTemp > 2.5){
        this->_kvalue = this->_kvalueHigh;
    }else if(valueTemp < 2.0){
        this->_kvalue = this->_kvalueLow;
    }

    // Calculate the EC value after automatic shift
    value = this->_rawEC * this->_kvalue;

    // Temperature compensation
    value = value / (1.0+0.0185*(temperature-25.0));  

    // Store the EC value for Serial CMD calibration
    this->_ecvalue = value;                           
    
    return value;
}

void DFRobot_EC::calibration(float voltage, float temperature,char* cmd)
{   
    this->_voltage = voltage;
    this->_temperature = temperature;
    strupr(cmd);

    // If received Serial CMD from the serial monitor, enter into the calibration mode
    ecCalibration(cmdParse(cmd));                     
}

void DFRobot_EC::calibration(float voltage, float temperature)
{   
    this->_voltage = voltage;
    this->_temperature = temperature;
    
    // If received Serial CMD from the serial monitor, enter into the calibration mode
    if(cmdSerialDataAvailable() > 0)
    {
        ecCalibration(cmdParse());  
    }
}

boolean DFRobot_EC::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available()>0) 
    {
        if(millis() - cmdReceivedTimeOut > 500U){
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer,0,(ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if(cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex==ReceivedBufferLength-1){
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }else{
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
}

byte DFRobot_EC::cmdParse(const char* cmd)
{
    byte modeIndex = 0;
    if(strstr(cmd, "ENTEREC")      != NULL){
        modeIndex = 1;
    }else if(strstr(cmd, "EXITEC") != NULL){
        modeIndex = 3;
    }else if(strstr(cmd, "CALEC")  != NULL){
        modeIndex = 2;
    }
    return modeIndex;
}

byte DFRobot_EC::cmdParse()
{
    byte modeIndex = 0;
    if(strstr(this->_cmdReceivedBuffer, "ENTEREC")     != NULL)
        modeIndex = 1;
    else if(strstr(this->_cmdReceivedBuffer, "EXITEC") != NULL)
        modeIndex = 3;
    else if(strstr(this->_cmdReceivedBuffer, "CALEC")  != NULL)
        modeIndex = 2;
    return modeIndex;
}

void DFRobot_EC::ecCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean ecCalibrationFinish  = 0;
    static boolean enterCalibrationFlag = 0;
    static float compECsolution;
    float KValueTemp;
    switch(mode){
        case 0:
        if(enterCalibrationFlag){
            Serial.println(F(">>>Command Error<<<"));
        }
        break;
        case 1:
        enterCalibrationFlag = 1;
        ecCalibrationFinish  = 0;
        Serial.println();
        Serial.println(F(">>>Enter EC Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 1413us/cm or 12.88ms/cm buffer solution<<<"));
        Serial.println();
        break;
        case 2:
        if(enterCalibrationFlag){
            if((this->_rawEC>0.9)&&(this->_rawEC<1.9)){                         //recognize 1.413us/cm buffer solution
                compECsolution = 1.413*(1.0+0.0185*(this->_temperature-25.0));  //temperature compensation
            }else if((this->_rawEC>9)&&(this->_rawEC<16.8)){                    //recognize 12.88ms/cm buffer solution
                compECsolution = 12.88*(1.0+0.0185*(this->_temperature-25.0));  //temperature compensation
            }else{
                Serial.print(F(">>>Buffer Solution Error Try Again<<<   "));
                ecCalibrationFinish = 0;
            }
            KValueTemp = RES2*ECREF*compECsolution/1000.0/this->_voltage;       //calibrate the k value
            if((KValueTemp>0.5) && (KValueTemp<1.5)){
                Serial.println();
                Serial.print(F(">>>Successful,K:"));
                Serial.print(KValueTemp);
                Serial.println(F(", Send EXITEC to Save and Exit<<<"));
                if((this->_rawEC>0.9)&&(this->_rawEC<1.9)){
                    this->_kvalueLow =  KValueTemp;
                }else if((this->_rawEC>9)&&(this->_rawEC<16.8)){
                    this->_kvalueHigh =  KValueTemp;
                }
                ecCalibrationFinish = 1;
          }
            else{
                Serial.println();
                Serial.println(F(">>>Failed,Try Again<<<"));
                Serial.println();
                ecCalibrationFinish = 0;
            }
        }
        break;
        case 3:
        if(enterCalibrationFlag){
                Serial.println();
                if(ecCalibrationFinish){   
                    if((this->_rawEC>0.9)&&(this->_rawEC<1.9)){
                        EEPROM_write(this->_address, this->_kvalueLow);
                    }else if((this->_rawEC>9)&&(this->_rawEC<16.8)){
                        EEPROM_write(this->_address+4, this->_kvalueHigh);
                    }
                    Serial.print(F(">>>Calibration Successful"));
                }else{
                    Serial.print(F(">>>Calibration Failed"));
                }
                Serial.println(F(",Exit EC Calibration Mode<<<"));
                Serial.println();
                ecCalibrationFinish  = 0;
                enterCalibrationFlag = 0;
        }
        break;
    }
}
