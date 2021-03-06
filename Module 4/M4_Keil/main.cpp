/**----------------------------------------------------------------------------
 
   \file main.cpp
--                                                                           --
--              ECEN 5803 Mastering Embedded System Architecture             --
--                  Project 1 Module 4                                       --
--                Microcontroller Firmware                                   --
--                      main.cpp                                             --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--               
--                
--  Designed by:  Tim Scherr
--  Revised by:  David James and Ismail Yesildirek
-- 
-- Version: 2.0.2
-- Date of current revision:  2018-10-04   
-- Target Microcontroller: Freescale MKL25ZVMT4 
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--               
-- 
-- Functional Description:  Main code file generated by mbed, and then
--                           modified to implement a super loop bare metal OS.
--
--      Copyright (c) 2015, 2016 Tim Scherr  All rights reserved.
--
*/

#define MAIN
#include "shared.h"
#include "math.h"
#include "TestData.h"
#undef MAIN

#define ADC_0                   (0U)
#define CHANNEL_0               (0U)
#define CHANNEL_1               (1U)
#define CHANNEL_2               (2U)
#define LED_ON                  (0U)
#define LED_OFF                 (1U)
#define ADCR_VDD                (65535U)    /*! Maximum value when use 16b resolution */
#define V_BG                    (1000U)     /*! BANDGAP voltage in mV (trim to 1.0V) */
#define V_TEMP25                (716U)      /*! Typical VTEMP25 in mV */
#define M                       (1620U)     /*! Typical slope: (mV x 1000)/oC */
#define STANDARD_TEMP           (25)

/**************************************************************** 
 * Define constants bluff body width (d - inches) 
 * and pipe inner diameter (PID - inches)
 ***************************************************************/
#define d_width 0.5 //inches
#define PID 2.900 //inches
#define PIDm 0.07366 //meters
#define sample_period 0.0001 // 100us

unsigned char c_spi;
extern volatile uint16_t SwTimerIsrCounter; //! ISR counter
Ticker tick;             //! Creates a timer interrupt using mbed methods
 /****************      ECEN 5803 add code as indicated   ***************/
 
 uint32_t frequency = 0.0f; //for the frequency calculation
 uint32_t temperature = 2300; //room temperature, Celsius (x100)
 uint32_t St_int = 0; //St integration for averaging
 uint32_t Re = 1500000; //initialize Re between 10,000 and 10,000,000
 uint16_t iters = 0;	 //keep track of iterations for St average
 uint32_t Flow = 0; //<----the purpose of this whole program
 
 //These variables can be made available to other files
 //uint32_t viscosity = 0; 
 //uint32_t rho_density = 0;
 //uint32_t St_const = 0;
 //uint32_t velocity = 0;
 
/**************************************************************/
// ADC/SPI tutorial in book: Freescale ARM Cortex-M 
// Embedded Programming: Using C Language (ARM books Book 3)
/**************************************************************/
	void ADC0_init(void);
	void ADC1_init(void);
	void ADC2_init(void);
	void SPI0_init(void);
	void SPI0_write(unsigned char * data, int size);
/****************************************************************/ 
/// @brief Read raw analog data (frequency and temperature) 
/// from the flowmeter.
/***************************************************************/
void read_internal_temp() 
{
uint16_t internal_temp =0;
ADC0->SC1[0] = 26; /* start conversion on channel 26 temperature */
while(!(ADC0->SC1[0] & 0x80)) { } 
/* wait for COCO to be set to 1 at bit7 of register SC1*/
internal_temp = ADC0->R[0]; /* read conversion result and clear COCO flag */
//printf("Internal temp is: %d", internal_temp);
/*The sample number is internal_temp @16bit resolution*/
}

/****************************************************************/ 
/// @brief convert raw analog data 
/// from the flowmeter to frequency.
/***************************************************************/
void readFREQ() 
{
		//This algorithm mirrors the Simulink diagram in the report
	  uint16_t i = 0; //index
	  uint16_t prev_edge = 0; //for T calculation
	  uint16_t this_edge = 0; //for T calculation
	  uint16_t max = 0; //for the sample max
	  uint16_t edges = 0; //edge count
	  uint16_t period = 0; //current period
	  uint32_t period_int = 0;//running total, discrete integration
	  
	  bool high = false;
	// we've got a 1000 sample block to work with:
	  for(i = 0; i<1000; i++)
	  {//look for a value that hits 0.9*max
				if(ADCbuffer[i%25]>0.9*max) 
				{ 
					if(!high) //rising edge
					{
						high = !high; //easier to find the period of a square wave...
						prev_edge = this_edge; //update prev_edge
						this_edge = i;				//then update this_edge
						edges++; //keep count for the average
						period = this_edge - prev_edge; //period in samples
						period_int+=period; //running total for the average
					} //if the current value is the largest we've seen, set it as the max
					if(ADCbuffer[1] > max) max=ADCbuffer[i];
				}//if we drop below 90%, we aren't near a maximum anymore
				else high = false;
		}//convert from Z to t
		float period_avg = sample_period*(float)period_int/(edges-1);
    // f=1/T, 100 is a scalar for lossless integer math
    frequency = 100/period_avg; //
		//frequency = 39948; // uncomment for a constant frequency
		
	  // set it up so that the temperature doesn't change any more than plus/minus 1 degree 
    temperature += (powf(-1.0,(rand()%2)))*(rand()%2); // T in Celsius (x100)
		//temperature = 2300; // uncomment for constant room temperature
}

/****************************************************************/ 
/// @brief calculate a flow based on temperature and frequency 
/***************************************************************/
void calculate_flow() 
{
	  iters += 1;
	 uint32_t temperatureK = temperature + 27315; //Kelvin (x100)
   //uint32_t temperatureD = (temperature * 9.0f/5.0f) + 3200; //Fahrenheit (x100)
	//Calculate values per equations provided.
	  uint32_t viscosity = 24*powf(10,24780.0f/((float)temperatureK - 14000.0f)); // (x1,000,000)
	  //viscosity = 24*powf(10,24780.0f/((float)temperatureK - 14000.0f)); // (x1,000,000)
    uint32_t rho_density = 1000*( 1- (((float)temperature+28894.14)/
																			(508929.2*((float) temperature+6812.963 )))
																				*powf((((float)temperature*0.01)-3.9863),2)) ; // 1:1 
	  //rho_density = 1000*( 1- (((float)temperature+28894.14)/
		//																	(508929.2*((float) temperature+6812.963 )))
		//																		*powf((((float)temperature*0.01)-3.9863),2)) ; // 1:1 
		uint32_t St = 2684-10356/powf(Re,0.5); // (x10,000)
	  St_int += St;
	  uint32_t St_const = St_int/iters;
		St_const = St_int/iters;
		if(iters>1000){ St_int=St_const; iters=1;}
		//velocity = 10000*frequency*d_width/St_const; // (x100)
	  uint32_t velocity = 10000*frequency*d_width/St_const; // (x100)
		//Re*1000000 to adjust for scaling of viscosity (x1,000,000)
    Re = 1000000*((float)rho_density*((float)velocity/3937)*(PIDm))/(float)viscosity;  
	  Flow = 2.45*PID*PID*velocity/12;
}

/****************************************************************/ 
/// @brief reads analog data from channel 30
///
/***************************************************************/
void read_vrefl() 
{
uint16_t ptb0_vrefl = 0;
ADC0->SC1[0] = 30; /* start conversion on channel 30 VREFL */
while(!(ADC0->SC1[0] & 0x80)) { } 
/* wait for COCO to be set to 1 at bit7 of register SC1*/
ptb0_vrefl = ADC0->R[0]; /* read conversion result and clear COCO flag */
/*ptb0_vrefl value is from 0 to 255*/
//printf("Internal VREFL is: %d", ptb0_vrefl);
}
/****************************************************************/
/// @brief TODO: empty function for ADC reads.
///
 /***************************************************************/
void readADC() 
{
	
}

/***************************************************************/ 
/// @brief send flow data to LCD display
///
/***************************************************************/
void LCD_Display() 
{
	unsigned char display_flow[14] = {'F','l','o','w','(','G','P','M',')',' ',
		'1','2','8','3'};
		unsigned char display_frequency[15] = {'F','r','e','q','(','H','z',')',' ',
		'3','9','9','.','5','9'};
		unsigned char display_temp[13] = {'T','e','m','p','(','C',')',' ',
		'2','3','.','4','0'};
	//	unsigned char flow_char [7];
	//	unsigned char freq_char [6];
	//	unsigned char temp_char[5];
 /*Send flow data to LCD*/				
			for(int i =0;i<13;i++)
			  {
					c_spi = display_flow[i];
					//printf("Flow is: %c", display_flow[i]);
			  }
				SPI0_write(display_flow,11); //send data through SPI to LCD
				
				/*Send frequency data to LCD*/
				for (int i =0;i<14;i++)
				{ 
					c_spi = display_frequency[i];
			//		SPI0_write(c_spi);
				}
				SPI0_write(display_frequency,15); //send data through SPI to LCD
				/*Send temp data to LCD*/
				for (int i =0;i<12;i++)
				{ 
					c_spi = display_temp[i];
				}
				SPI0_write(display_temp,13); //send data through SPI to LCD   
}
/***************************************************************/ 
/// @brief main function, setup and loop
///
/***************************************************************/
int main() 
{
/****************      ECEN 5803 add code as indicated   ***************/
                    //  Add code to call timer0 function every 100 uS
    tick.attach(&timer0, 0.0001); // setup ticker to call flip every 100 microseconds
    uint32_t  count = 0;   
    
// initialize serial buffer pointers
   rx_in_ptr =  rx_buf; //! pointer to the receive in data 
   rx_out_ptr = rx_buf; //! pointer to the receive out data*/
   tx_in_ptr =  tx_buf; //! pointer to the transmit in data*/
   tx_out_ptr = tx_buf; //! pointer to the transmit out */
    
   //UART_direct_msg_put("\r\nCode ver. ");
   //UART_direct_msg_put( CODE_VERSION );
   //UART_direct_msg_put("\r\n");
   //UART_direct_msg_put( COPYRIGHT );
   //UART_direct_msg_put("\r\n");	
	
   set_display_mode();                                      
   ADC0_init();
		ADC1_init();
		ADC2_init();
		SPI0_init(); /* enable SPI0 */ 
		
    while(1)       // Cyclical Executive Loop
    {
			  readADC();
				readFREQ();				//reads ADC buffer and calculates the frequency
			  read_vrefl(); //reads ADC ch0
		    read_internal_temp(); //reads ADC ch2
		    calculate_flow();   //calculates volumentric flow in Gallons per minute
        count++;                  // counts the number of times through the loop
        serial();            // Polls the serial port
        chk_UART_msg();     // checks for a serial port message received
        monitor();           // Send output messages depending
        LCD_Display();        //  on commands received and display mode
    }     
}
/***************************************************************/ 
/// @brief Initialize ADC0
///
/***************************************************************/
void ADC0_init(void)
{
SIM->SCGC5 |= 0x0400; /* clock to PORTB */
PORTB->PCR[0] = 0; /* PTB0 analog input */
SIM->SCGC6 |= 0x8000000; /* clock to ADC0 */
ADC0->SC2 &= ~0x40; /* software trigger */
/* no low power, clock div by 1, long sample time, single ended 8 bit, bus clock */
ADC0->CFG1 = 0x01 | 0x10 | 0x00 | 0x00; 
}
/*Re use ADC0 to set up other pins*/
/***************************************************************/ 
/// @brief Initialize ADC1
///
/***************************************************************/
	void ADC1_init(void)
{
SIM->SCGC5 |= 0x0400; /* clock to PORTB */
PORTB->PCR[1] = 0; /* PTB1 analog input */
SIM->SCGC6 |= 0x8000000; /* clock to ADC1 */
ADC0->SC2 &= ~0x40; /* software trigger */
/* no low power, clock div by 1, long sample time, single ended 16 bit, bus clock */
ADC0->CFG1 = 0x01 | 0x10 | 0x03 | 0x00; 
}
/***************************************************************/ 
/// @brief Initialize ADC2
///
/***************************************************************/
	void ADC2_init(void)
{
SIM->SCGC5 |= 0x0400; /* clock to PORTB */
PORTB->PCR[2] = 0; /* PTB2 analog input */
SIM->SCGC6 |= 0x8000000; /* clock to ADC2 */
ADC0->SC2 &= ~0x40; /* software trigger */
/* no low power, clock div by 1, long sample time, single ended 16 bit, bus clock */
ADC0->CFG1 = 0x01 | 0x10 | 0x03 | 0x00; 
}
/****************************************************************/ 
/// @brief Initialize SPI
///
/***************************************************************/
void SPI0_init(void) {
SIM->SCGC5 |= 0x0800; /* enable clock to Port C */
	/*Set ports to alternative 2 for SPI*/
PORTC->PCR[4] = 0x200; /* make PTC4 pin as SPI0 PCS0 */
PORTC->PCR[5] = 0x200; /* make PTC5 pin as SPI0 SCK */
PORTC->PCR[6] = 0x200; /* make PTC6 pin as SPI0 MOSI */
PORTC->PCR[7] = 0x200; /* make PTC7 pin as SPI0 MISO */
PTC->PDDR |= 0x01; /* make PTC0 as output pin for /SS */
PTC->PSOR = 0x01; /* make PTC0 idle high */
SIM->SCGC4 |= 0x400000; /* enable clock to SPI0 */
SPI0->C1 = 0x10; /* disable SPI and make SPI0 master */
SPI0->BR = 0x60; /* set Baud rate to 1 MHz */
SPI0->C1 |= 0x40; /* Enable SPI module */
}

/***************************************************************/ 
/// @brief send characters through SPI
///
/***************************************************************/
void SPI0_write(unsigned char * data,int size) {
volatile char dummy;
PTC->PCOR = 1; /* assert /SS */
while(!(SPI0->S & 0x20)) { } /* wait until tx ready */
//send all data then clear and exit.
for (int i=0;i<size;i++){
SPI0->D = data[i]; /* send data byte */
while(!(SPI0->S & 0x80)) { } /* wait until tx complete */
}
dummy = SPI0->D; /* clear SPRF */
PTD->PSOR = 1; /* deassert /SS */
}

