/************************************************************************//**
* \file main.c
* \brief ECEN 5803 Project 1, Module 1
*
*	Authors: David Pasley, Ismail Yesildirek
*
* An assembly code subroutine was written to approximate the square root of an 
* argument using the bisection method. All math is done with integers, so the 
* resulting square root is a truncated integer
******************************************************************************/

 #include <MKL25Z4.H>
 
 /**
 * @brief my_sqrt is an assembly function which approximates the square root of 
 *				an integer using the bisection method.
 *
 * @param[in] x is the integer you wish to find a square root for
 *
 * @return The function returns a truncated integer approximation of sqrt(x).
 */
__asm int my_sqrt(int x)
{
	MOV		r3,r0				; x is now r3, r0 will be the return value c
	MOVS	r2,#1			 	; b is r2 initialized to 65536, the largest sqrt possible for 32 bits
	LSLS  r2,#16			; 65536 is 1 << 16 since the compiler wont allow more than 8 bit immediates.
	MOVS	r1,#0				; a is r1, initialized to 0
loop								; loop starts here
	MOV		r4,r0				; c_old <- c
	ADDS	r0,r1,r2		; c <- (a+b)
	LSRS	r0,r0,#1		; c <- c/2
	MOV  	r5,r0		  	; r5 <- c*c
	MULS  r5,r5
	CMP		r5,r3				; check c*c == x
	BEQ		end					; exit if it is the solution
	BLT		less				; branch to less if c*c < x
	MOV		r2,r0				; if c*c > x, b <- c
ret
	CMP		r0,r4				; check c == c_old
	BNE		loop				; return to loop if c has changed
end	
	BX		lr					; finished
less								; 
	MOV		r1,r0				; a <- c
	B			ret					; return
}

/*----------------------------------------------------------------------------
 MAIN function
 *----------------------------------------------------------------------------*/
 /**
 * @brief Main function
 * The main function tests my_sqrt with four values: 2, 4, 22, and 121.
 * once the values are computed, the program enters a while loop.
 */
int main(void){
	int r, j, k, l;                 
  r = my_sqrt(2);     // should be 1
  j = my_sqrt(4);     // should be 2
	k = my_sqrt(22); 	  // should be 4
	l = my_sqrt(121);   // should be 11
	while(1)
		;
}
