/*
 * file DFRobot_EC.h * @ https://github.com/DFRobot/DFRobot_EC
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

#ifndef _DFROBOT_EC_H_
#define _DFROBOT_EC_H_

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <map>

// Map from pin to a address number to be multiplied by the number of bytes offset
static std::map<uint8_t, uint8_t> EPinToAddressMap{
	{A0, 0}, 
	{A1, 1}, 
	{A2, 2}, 
	{A3, 3}, 
	{A4, 4}, 
	{A5, 5}, 
	{A6, 6}, 
	{A7, 7}, 
	{A8, 8}, 
	{A9, 9}, 
	{A10, 10}, 
	{A11, 11}, 
    };

// Length of the Serial CMD buffer
#define ReceivedBufferLength 10  

class DFRobot_EC
{
public:
    // Construtors
    DFRobot_EC();
    DFRobot_EC(int ecPin);

    // Destructors
    ~DFRobot_EC();
    
    // Initialization
    void begin();
    
    // Calibration by Serial CMD
    void calibration(float voltage, float temperature, char* cmd);
    void calibration(float voltage, float temperature);

    // Voltage to EC value with temperature compensation
    float readEC(float voltage, float temperature);
    
private:
    int    _pin;
    int    _address;
    float  _ecvalue;
    float  _kvalue;
    float  _kvalueLow;
    float  _kvalueHigh;
    float  _voltage;
    float  _temperature;
    float  _rawEC;

    // Store the Serial CMD
    char   _cmdReceivedBuffer[ReceivedBufferLength];  
    byte   _cmdReceivedBufferIndex;

private:
    // Calibration process, wirte key parameters to EEPROM
    void    ecCalibration(byte mode); 

    boolean cmdSerialDataAvailable();

    byte    cmdParse(const char* cmd);
    byte    cmdParse();
};

#endif
