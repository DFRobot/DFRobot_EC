## DFRobot_EC Library 
---------------------------------------------------------
This is the sample code for Gravity: Analog Electrical Conductivity Sensor / Meter Kit V2 (K=1.0), SKU: DFR0300.
## Table of Contents

* [Methods](#methods)
* [Compatibility](#Compatibility)
* [History](#history)
* [Credits](#credits)
<snippet>
<content>

## Methods

```C++
/*
 * @brief The new optional constructor to allow multiple EC sensors
 *
 * @param ecPin: The pin of the EC sensor i.e. A0
 */
DFRobot_EC(int ecPin);

/*
 * @brief Init The Analog Electrical Conductivity Sensor
 */
void begin();

/*
 * @brief Convert voltage to EC with temperature compensation
 *
 * @param voltage     : Voltage value
 *        temperature : Ambient temperature
 *
 * @return The EC value
 */
float readEC(float voltage, float temperature);

/*
 * @brief Calibrate the calibration data
 *
 * @param voltage     : Voltage value
 *        temperature : Ambient temperature
 *        cmd         : enterec -> enter the EC calibration mode
 *                      calec   -> calibrate with the standard buffer solution, two buffer solutions(1413us/cm and 12.88ms/cm) will be automaticlly recognized
 *                      exitec  -> save the calibrated parameters and exit from EC calibration mode
 */
void calibration(float voltage, float temperature, char* cmd);

```

## Compatibility

MCU                | Work Well | Work Wrong | Untested  | Remarks
------------------ | :----------: | :----------: | :---------: | -----
Arduino Uno  |      √       |             |            | 
Leonardo  |      √       |             |            | 
Meag2560 |      √       |             |            | 

## History

- date 2018-11-6
- version V1.0
- date 2020-04-18
- version V1.1
    - Update to allow multiple EC sensors each with their own calibration

## Credits

Written by Jiawei Zhang(Welcome to our [website](https://www.dfrobot.com/))
