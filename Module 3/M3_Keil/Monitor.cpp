/**----------------------------------------------------------------------------
             \file Monitor.cpp
--                                                                           --
--              ECEN 5803 Mastering Embedded Systems Architecture             --
--                  Project 1 Module 3                                       --
--                Microcontroller Firmware                                   --
--                      Monitor.cpp                                            --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--               
--                
--  Designed by:  Tim Scherr
--  Revised by:  David James & Ismail Yesildirek
-- 
-- Version: 2.0.1
-- Date of current revision:  2018-10-03   
-- Target Microcontroller: Freescale MKL25ZVMT4 
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--							Keil uVision MDK v.5
--              Freescale FRDM-KL25Z Freedom Board
--               
-- 
   Functional Description: See below 
--
--      Copyright (c) 2015 Tim Scherr All rights reserved.
--
*/              

#include <stdio.h>
#include "shared.h"
DigitalOut greenLED(LED_GREEN); //<! Setup greenLED 
bool green_led_status = 1; //<! status variable for greenLED, default is on.

/*****************************************************************************/
/// \fn uint32_t getR0(void) 
/// @brief assembly routine which returns the unaltered contents of register r0
/*****************************************************************************/
__asm uint32_t getR0()
{
		BX lr	; returns r0 without altering it	
}

/*****************************************************************************/
///  \fn void getRn(uint32_t[16]) 
/// @brief assembly routine which fills a uint32_t[16] array's 1-15 elements with r1-r15
/*****************************************************************************/
__asm void getRn(uint32_t reglist[16])
{
	  STR r1, [r0, #4]	; r1->reglist[1]
		STR r2, [r0, #4]	; r2->reglist[2]
		STR r3, [r0, #12]	; r3->reglist[3]
		STR r4, [r0, #16]	; r4->reglist[4]
		STR r5, [r0, #20] ; r5->reglist[5]
		STR r6, [r0, #24] ; r6->reglist[6]
		STR r7, [r0, #28] ; r7->reglist[7]
		MOV r1, r8				;	STR only takes r0-r7	
		STR r1, [r0, #32] ; r8->r1->reglist[8]
		MOV r1, r9				; STR only takes r0-r7
		STR r1, [r0, #36] ; r9->r1->reglist[9]
		MOV r1, r10				; STR only takes r0-r7
		STR r1, [r0, #40] ; r10->r1->reglist[10]
		MOV r1, r11				; STR only takes r0-r7
		STR r1, [r0, #44] ; r11->r1->reglist[11]
		MOV r1, r12				;	STR only takes r0-r7
		STR r1, [r0, #48] ; r12->r1->reglist[12]
		MOV r1, sp				; STR only takes r0-r7
		STR r1, [r0, #52] ; sp(r13)->r1->reglist[13]
		MOV r1, lr				; STR only takes r0-r7
		STR r1, [r0, #56] ; lr(r14)->r1->reglist[14]
		MOV r1, pc				; STR only takes r0-r7
		STR r1, [r0, #60] ; pc(r15)->r1->reglist[15]
	  BX lr							;	return
}

/*****************************************************************************/
/// @brief uint32_t getWord(uint32_t) assembly routine
/// returns the 32 bit word stored at the 32 bit address in the argument
/*****************************************************************************/
__asm uint32_t getWord(uint32_t address)
{
	LDR r0, [r0]	; data @ r0 -> r0
	BX lr					; return
}

/*****************************************************************************/
///  \fn void show_regs_and_mem()
/// @brief prints registers r0-r15 to debug port,
/// and shows the contents of 16 words worth of memory.
/// The memory addresses cycle from 0x0 to 0x6000
/*****************************************************************************/
void show_regs_and_mem()
{
	// The top line of the display
	UART_direct_msg_put("\r\n\r\nRegister contents:");
	UART_direct_msg_put("\t|\tADDRESS:\tDATA:");
	
	// retrieve the contents of registers r0-r15
	uint32_t reg[16];	//!< array to hold r0-r15
	reg[0] = getR0(); // because passing any variable would alter r0
	getRn(reg); 			// fill the rest of the array
	uint8_t i=0;			//!< index variable
	
	// addr is declared static so we can cycle through addresses
	static uint32_t addr = 0x0; //!< address for memory block reads.
																//< starts at 0x00000000 and increments each iteration
	uint32_t data;				//<! a variable to hold the data
	// print 16 lines:
	for(i=0;i<16;i++)
	{
		UART_direct_msg_put("\r\nr"); //start a new line with r
		//now, to print the register number:
		if(i/10)	// if the index is greater than 10...
		{ // convert the hex value to decimal
			UART_direct_hex_put((i/10 << 4) + i%10);
		}
		else 
		{ // otherwise just print the bottom nibble
			UART_low_nibble_direct_put(i&0xF);
		} 																//add tab for alignment,
		UART_direct_msg_put(" \t0x"); 		// format hex values with 0x########
		UART_direct_word_hex_put(reg[i]); //print the whole word
		UART_direct_msg_put("\t|\t0x");		// tabs and a divider, plus 0x########
		UART_direct_word_hex_put(addr); 	// first print the address
		UART_direct_msg_put("\t0x");			// tab and add 0x########
		data = getWord(addr);							// get the data @ addr
		UART_direct_word_hex_put(data);		// print the data
		if(addr < 0x6000) addr +=4;				// increment the address by 4
		else addr = 0;										// roll over at 0x6000
	}
	return;
}

/*****************************************************************************/
/// \fn void set_display_mode(void)
///
///	Set Display Mode Function
/// @brief Function determines the correct display mode.  
///
///		The 3 display modes operate as follows:
///
///  NORMAL MODE       Outputs only mode and state information changes   
///                     and calculated outputs
///
///  QUIET MODE        No Outputs
///
//  DEBUG MODE        Outputs mode and state information, error counts,
///                    register displays, sensor states, and calculated output
///
///
/// There is deliberate delay in switching between modes to allow the RS-232 cable 
/// to be plugged into the header without causing problems. 
/*****************************************************************************/
void set_display_mode(void)   
{
  UART_direct_msg_put("\r\nSelect Mode");
  UART_direct_msg_put("\r\n Hit NOR - Normal");
  UART_direct_msg_put("\r\n Hit QUI - Quiet");
  UART_direct_msg_put("\r\n Hit DEB - Debug" );
  UART_direct_msg_put("\r\n Hit V - Version#");
	/********************************************* 
	 * Added instruction for GREEN LED control
	 * and defined LED under shared.h 
	 ********************************************/
	UART_direct_msg_put("\r\n Hit L - Toggle Green LED\r\n");
  UART_direct_msg_put("\r\nSelect:  ");
  
}

/*****************************************************************************/
/// \fn void chk_UART_msg(void) 
/// @brief checks for messages in serial port
/*****************************************************************************/
void chk_UART_msg(void)    
{ 
   UCHAR j; //<! placeholder for character
   while( UART_input() )      // becomes true only when a byte has been received
   {                                    // skip if no characters pending
      j = UART_get();                 // get next character

      if( j == '\r' || j == '\n')          // on a enter (return) key press
      {                // complete message (all messages end in carriage return)
         UART_msg_put("->");
         UART_msg_process();
      }
      else 
      {
         if ((j != 0x02) )         // if not ^B
         {                             // if not command, then   
            UART_direct_put(j);              // echo the character   
         }
         else
         {
           ;
         }
         if( j == '\b' ) 
         {                             // backspace editor
            if( msg_buf_idx != 0) 
            {                       // if not 1st character then destructive 
               UART_msg_put(" \b");// backspace
               if(msg_buf_idx > 0) msg_buf_idx--;
            }
         }
         else if( msg_buf_idx >= MSG_BUF_SIZE )  
         {                                // check message length too large
            UART_msg_put("\r\nToo Long!");
            msg_buf_idx = 0;
         }
         else if ((display_mode == QUIET || display_mode == DEBUG) && (msg_buf[0] != 0x02) && 
                  (msg_buf[0] != 'D') && (msg_buf[0] != 'N') && 
                  (msg_buf[0] != 'V') && (msg_buf[0] != 'L') &&
									(msg_buf[0] != 'd') && (msg_buf[0] != 'n') &&
									(msg_buf[0] != 'v') && (msg_buf[0] != 'l') &&
                  (msg_buf_idx != 0))
         {                          // if first character is bad in Quiet mode
            msg_buf_idx = 0;        // then start over
         }
         else {                        // not complete message, store character
 
            msg_buf[msg_buf_idx] = j;
            msg_buf_idx++;
         }
      }
   }
}

/*****************************************************************************/
///  \fn void UART_msg_process(void) 
/// @brief UART Input Message Processing
/*****************************************************************************/
void UART_msg_process(void)
{
   UCHAR chr,err=0; //<!   unsigned char for input and error codes;

   chr = msg_buf[0]; 
      switch( chr ) 
      {
         case 'D'://Debug mode
            if((msg_buf[1] == 'E') && (msg_buf[2] == 'B') && (msg_buf_idx == 3)) 
            {
               display_mode = DEBUG;
               UART_msg_put("\r\nMode=DEBUG\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;
						
         case 'N': //normal mode
            if((msg_buf[1] == 'O') && (msg_buf[2] == 'R') && (msg_buf_idx == 3)) 
            {
               display_mode = NORMAL;
               UART_msg_put("\r\nMode=NORMAL\n");
               //display_timer = 0;
            }
            else
               err = 1;
            break;
						
         case 'Q': //quiet mode
            if((msg_buf[1] == 'U') && (msg_buf[2] == 'I') && (msg_buf_idx == 3)) 
            {
               display_mode = QUIET;
               UART_msg_put("\r\nMode=QUIET\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;
									
         case 'V': // VERSION;
            UART_msg_put("\r\n");
            UART_msg_put( CODE_VERSION ); 
            UART_msg_put("\r\nSelect  ");
            display_timer = 0;
            break;
				 
         case 'L': //toggle LED;
            greenLED = !greenLED;	
						green_led_status = !green_led_status;
				    UART_msg_put("\r\n");
						if (green_led_status ==0){
				    UART_msg_put("\r\n Green LED OFF");
						}
						else 
						{
							UART_msg_put("\r\n Green LED ON");
						}
            display_timer = 0;
            break;	
				//added all lower case input to improve user interface.
         case 'd':
            if((msg_buf[1] == 'e') && (msg_buf[2] == 'b') && (msg_buf_idx == 3)) 
            {
               display_mode = DEBUG;
               UART_msg_put("\r\nMode=DEBUG\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;
									
         case 'n':
            if((msg_buf[1] == 'o') && (msg_buf[2] == 'r') && (msg_buf_idx == 3)) 
            {
               display_mode = NORMAL;
               UART_msg_put("\r\nMode=NORMAL\n");
               //display_timer = 0;
            }
            else
               err = 1;
            break;	
         
				 case 'q':
            if((msg_buf[1] == 'u') && (msg_buf[2] == 'i') && (msg_buf_idx == 3)) 
            {
               display_mode = QUIET;
               UART_msg_put("\r\nMode=QUIET\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;		
						
         case 'v':
            //display_mode = VERSION;
            UART_msg_put("\r\n");
            UART_msg_put( CODE_VERSION ); 
            //UART_msg_put("\r\nSelect  ");
            display_timer = 0;
            break;

         case 'l':
            greenLED = !greenLED;	
						green_led_status = !green_led_status;
						//display_mode = LED;
				    UART_msg_put("\r\n");
						if (green_led_status ==0){
				    UART_msg_put("\r\n Green LED OFF");
						//UART_msg_put("\r\nSelect  ");   
						}
						else if(green_led_status ==1)
						{
						UART_msg_put("\r\n Green LED ON");
						//UART_msg_put("\r\nSelect  ");
						}
            display_timer = 0;
            break;	
				 
				default:
         err = 1;
      }
   
 
   if( err == 1 )
   {
      UART_msg_put("\n\rError!");
   }   
   else if( err == 2 )
   {
      UART_msg_put("\n\rNot in DEBUG Mode!");
   }   
   else
   {
    msg_buf_idx = 0;          // put index to start of buffer for next message
      ;
   }
   msg_buf_idx = 0;          // put index to start of buffer for next message


}


/*****************************************************************************/
///   \fn   is_hex
/// @brief Function takes 
///  @param a single ASCII character and returns 
///  @return 1 if hex digit, 0 otherwise.
///    
/*****************************************************************************/
UCHAR is_hex(UCHAR c)
{
   if( (((c |= 0x20) >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'))  )
      return 1;
   return 0;
}

/*****************************************************************************/
///   @brief  DEBUG and DIAGNOSTIC Mode UART Operation
/*****************************************************************************/
void monitor(void)
{

/**********************************/
/*     Spew outputs               */
/**********************************/

   switch(display_mode)
   {
      case(QUIET):
         {
             UART_msg_put("\r\n ");
             display_flag = 0;
         }  
         break;
      case(VERSION):
         {
             display_flag = 0;
         }  
         break;   
      case(LED):
         {
					 display_flag = 0;
         }  
         break; 				 
      case(NORMAL):
         {
            if (display_flag == 1)
            {
               UART_msg_put("\r\nNORMAL ");
               UART_msg_put(" Flow: ");
               // ECEN 5803 add code as indicated
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
							 UART_hex_put('2');
               UART_msg_put(" Temp: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
							 UART_hex_put('7');
               UART_msg_put(" Freq: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
							 UART_hex_put('5');
               display_flag = 0;
            }
         }  
         break;
      case(DEBUG):
         {
            if (display_flag == 1)
            {
               UART_msg_put("\r\nDEBUG ");
               UART_msg_put(" Flow: ");
               // ECEN 5803 add code as indicated               
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
							 UART_hex_put('2');
               UART_msg_put(" Temp: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
							 UART_hex_put('7');
               UART_msg_put(" Freq: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               UART_hex_put('5');
               
 /****************      ECEN 5803 add code as indicated   ***************/             
               //  Create a display of  error counts, sensor states, and
               //  ARM Registers R0-R15
							 //UART_msg_put("\r\n Error: ");
							 //UART_msg_put("\r\n Sensor: ");
							
							 show_regs_and_mem(); // function displays register contents over UART
               
               //  Create a command to read 16 words from the current stack 
               // and display it in reverse chronological order.
              
              
               // clear flag to ISR      
               display_flag = 0;
             }   
         }  
         break;

      default:
      {
         UART_msg_put("Mode Error");
      }  
   }
}  
