/*****************************************************************************
* Copyright (C) 2018
*
* Redistribution, modification or use of this software in source or binary
* forms is permitted as long as the files maintain this copyright. Users are
* permitted to modify this and use it to learn about the field of embedded
* software. David James,Ismail Yesildirek, and the University of Colorado are not liable for
* any misuse of this material.
*
*****************************************************************************/
/// @file Main.cpp
/// @brief This source file contains a c program that reads raw analog data
/// out of the built in accelerometer in the FRDM-KL25Z board. Based on the  
/// x, y, and z position the built in RGB LED will change color. This project
/// also utilizes the built in capacitive touch slider to control the RGB
///  LED dimmability. 
///
/// @author David James & Ismail Yesildirek
/// @date September 27 2018
/// @version 1.0
///
/*****************************************************************************/
#include "mbed.h"
#include "MMA8451Q.h"
#include "tsi_sensor.h"


/**
 * @brief main()
 *	this function contains the setup and main loop
 */
int main(void)
{
	  // instantiate accelerometer. SDA=PTE25, SCL=PTE24, i2c address=0x3A
    MMA8451Q acc(PTE25, PTE24, 0x3A); 
	  // instantiate touch slider (pins 9&10, adc range 0-40 --> 0.0f-1.0f)
	  TSIAnalogSlider tsi(9, 10, 40);
	  // set up PWM to LED pins
    PwmOut rled(LED1); //!< PWM output for RED LED
    PwmOut gled(LED2); //!< PWM output for GREEN LED
    PwmOut bled(LED3); //!< PWM output for BLUE LED
	  
	  float t; //!< Holds touch slider percentage
/* @brief Read touch slider and write to LED based on position */	
    while (1)
		{
			  // read touch slider percentage
			  t = tsi.readPercentage();	
				// generate RGB values from touch slider & accelerometer
        rled = t + abs(acc.getAccZ());
        gled = t + abs(acc.getAccY());
        bled = t + abs(acc.getAccX());
			  // 10 Hz update rate
        wait(0.1f);
			  i++;
    }
}
