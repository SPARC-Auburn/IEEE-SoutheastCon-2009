//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************
/*
Wiring Guide for PIC18F2685:

	pin 6	-	(input) microswitch (active high)

	pin 2	-	(output) LED
 	pin 3	-	(output) LED
	pin 4	-	(output) LED
	pin 5	-	(output) LED

	pin 21	-	(i/o) signal line for Parallax Sonar Module
	pin 22	-	(i/o) signal line for Parallax Sonar Module
	pin 23	-	(i/o) signal line for Parallax Sonar Module
	pin 24	-	(i/o) signal line for Parallax Sonar Module

Wiring Guide for PIC18F4685:

	pin 6	-	(input) microswitch (active high)

	pin 2	-	(output) LED
 	pin 3	-	(output) LED
	pin 4	-	(output) LED
	pin 5	-	(output) LED

	pin 36	-	(i/o) signal line for Parallax Sonar Module
	pin 35	-	(i/o) signal line for Parallax Sonar Module
	pin 34	-	(i/o) signal line for Parallax Sonar Module
	pin 33	-	(i/o) signal line for Parallax Sonar Module



 
*/
#include "main.h"
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"
#include <delays.h>
#include <timers.h>
#include <usart.h>
#include <stdlib.h>

unsigned int pulseDuration = 0;
unsigned int distance[] = {50,50,50,50};
int sonarIndex = 0;
int i;
char buffer[10];
unsigned int thresholdInCentimeters = 7;


#pragma config OSC = IRCIO67,WDT = OFF, MCLRE = OFF

#pragma code high_vector=0x08
void high_vec(void)
{
	_asm goto high_isr _endasm
}


#pragma code low_vector=0x18
void low_vec (void)
{
   _asm goto low_isr _endasm
}

#pragma code
volatile unsigned char pointer;

//***************************************************************************************************************
//							high_isr
//***************************************************************************************************************

#pragma interrupt high_isr
void high_isr(void)
{
	if(PIR1bits.SSPIF){		// If - SSP Module (I2C)
		unsigned char c;
		if(SSPSTATbits.R_W){	// If - Read from slave
			if(!isQueueEmpty()){// Check if QUEUE is EMPTY
				c = popQueue();	// Grab a char from the QUEUE
				SSPBUF = c;		// Write the byte to the I2C bus
			}
		} 
		else {				// Else - Write to Slave
			if(SSPSTATbits.D_A){	// If - Data
				if(SSPSTATbits.BF){		//If - Buffer Full
					c = SSPBUF;	// Grab a char from the I2C Buffer
					if(!isQueueFull()){	// Check if QUEUE is FULL
						pushQueue(c);	// Write the char to the QUEUE
					}
				}	
			} 
			else {					// Else - Address
				c = SSPBUF;	// Grab a char from the I2C Buffer (Dummy Read)	
			}
		}
		PIR1bits.SSPIF = 0;		// Clear SSP Module Interrupt
		//SSPCON1bits.CKP = 0;	// Release I2C Clock	
	} 
	else {				// Else - Bus Collision (I2C) 
		PIR2bits.BCLIF = 0; 	// Clear Bus Collision Flag
	}	
}

//***************************************************************************************************************
//							low_isr
//***************************************************************************************************************

#pragma interruptlow low_isr
void low_isr (void)
{	
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	Init();
	TXString("\x0D\x0A");		// Put out a new line
	TXChar('>');	
	
	TRISAbits.TRISA0 = 0;				//set pin 2 as input for microswitch, and pins 3,4,5,6 as outputs for LEDs
	TRISAbits.TRISA1 = 0;
	TRISAbits.TRISA2 = 0;
	TRISAbits.TRISA3 = 0;
	TRISAbits.TRISA4 = 1;
	
	ADCON1 = 0X0F;
		
	OpenTimer0( TIMER_INT_OFF & //initialize timer0 for: - interupt disabled
            T0_16BIT &           //					 	 - 8 bit timer
            T0_SOURCE_INT &		//						 - based on internal oscillator
            T0_PS_1_2 );		//						 - 8X prescaler to give sonar measurement 1 microsecond accuracy


while(1)
{
	if(PORTAbits.RA4)
	{
		TXString("Microswitch Engaged");
		TXString("\x0D\x0A");
		
								//start the 1st sonar measurement
		sonarIndex = 0;		

		TRISBbits.TRISB0 = 0; 	//set pin 21 to output for Parallax triggering sequence

		PORTBbits.RB0 = 0;		//bring pin 21 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTBbits.RB0 = 1;  	//bring pin 21 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTBbits.RB0 = 0;		//bring pin 21 low

		TRISBbits.TRISB0 = 1; 	//set pin 21 to input for pulse readin
		
		while(PORTBbits.RB0 == 0)
		{	
								//wait for the Parallax to bring pin 21 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTBbits.RB0 == 1)
		{
								//wait for the Parallax to bring pin 21 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration / 58;						
		
		if(distance[sonarIndex] < thresholdInCentimeters)
		{						//illuminate pin 3 LED if reading is less than 7cm
			PORTAbits.RA3 = 1;
		}
		else
		{
			PORTAbits.RA3 = 0;
		}	
		
								//output the duration of pulse received from the Parallax in centimeters
//		TXString("Sonar Reading4:");
//		TXDec_Int(distance[sonarIndex]);		
//		TXString("\x0D\x0A");

								//start 2nd sonar measurement
		sonarIndex++;			
		
		TRISBbits.TRISB1 = 0; 	//set pin 22 to output for Parallax triggering sequence

		PORTBbits.RB1 = 0;		//bring pin 22 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTBbits.RB1 = 1;  	//bring pin 22 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTBbits.RB1 = 0;		//bring pin 22 low

		TRISBbits.TRISB1 = 1; 	//set pin 22 to input for pulse readin
		
		while(PORTBbits.RB1 == 0)
		{	
								//wait for the Parallax to bring pin 22 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTBbits.RB1 == 1)
		{
								//wait for the Parallax to bring pin 22 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration / 58;						

								
		if(distance[sonarIndex] < thresholdInCentimeters)
		{						//illuminate pin 4 LED if reading is less than 7cm
			PORTAbits.RA2 = 1;
		}
		else
		{
			PORTAbits.RA2 = 0; 
		}		

								//output the duration of pulse received from the Parallax in centimeters
//		TXString("Sonar Reading3:");
//		TXDec_Int(distance[sonarIndex]);
//		TXString("\x0D\x0A");

								//start the 3rd sonar measurement
		sonarIndex++;

		TRISBbits.TRISB2 = 0; 	//set pin 23 to output for Parallax triggering sequence

		PORTBbits.RB2 = 0;		//bring pin 23 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTBbits.RB2 = 1;  	//bring pin 23 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTBbits.RB2 = 0;		//bring pin 23 low

		TRISBbits.TRISB2 = 1; 	//set pin 23 to input for pulse readin
		
		while(PORTBbits.RB2 == 0)
		{	
								//wait for the Parallax to bring pin 23 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTBbits.RB2 == 1)
		{
								//wait for the Parallax to bring pin 23 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration / 58;						
		
		if(distance[sonarIndex] < thresholdInCentimeters)
		{						//illuminate pin 5 LED if reading is less than 7cm
			PORTAbits.RA1 = 1;
		}
		else
		{
			PORTAbits.RA1 = 0; 
		}		


								//output the duration of pulse received from the Parallax in centimeters
//		TXString("Sonar Reading:");
//		TXDec_Int(distance[sonarIndex]);
//		TXString("\x0D\x0A");

								//start the 4rd sonar measurement
		sonarIndex++;

		TRISBbits.TRISB3 = 0; 	//set pin 24 to output for Parallax triggering sequence

		PORTBbits.RB3 = 0;		//bring pin 24 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTBbits.RB3 = 1;  	//bring pin 24 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTBbits.RB3 = 0;		//bring pin 24 low

		TRISBbits.TRISB3 = 1; 	//set pin 24 to input for pulse readin
		
		while(PORTBbits.RB3 == 0)
		{	
								//wait for the Parallax to bring pin 24 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTBbits.RB3 == 1)
		{
								//wait for the Parallax to bring pin 24 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration / 58;	

		if(distance[sonarIndex] < thresholdInCentimeters)
		{						//illuminate pin 6 LED if reading is less than 7cm
			PORTAbits.RA0 = 1;
		}
		else
		{
			PORTAbits.RA0 = 0; 
		}					

								//output the duration of pulse received from the Parallax in centimeters
//		TXString("Sonar Reading:");
//		TXDec_Int(distance[sonarIndex]);
//		TXString("\x0D\x0A");
		
		if(PORTAbits.RA0 & PORTAbits.RA1 & PORTAbits.RA2 & PORTAbits.RA3)
		{
			TXString("Plastic Bottle");			
		}	 
		else
		{
			if(PORTAbits.RA0 & PORTAbits.RA1 & PORTAbits.RA2 & !PORTAbits.RA3)
			{
				TXString("Glass Bottle");	
			}
			else
			{
				if(PORTAbits.RA0 & PORTAbits.RA1 & !PORTAbits.RA2 & !PORTAbits.RA3)
				{
					TXString("Aluminum Can");	
				}
				else
				{
					TXString("Error");	
				}			
			}
								
		}	
		
	}
	else
	{
		TXString("Microswitch NOT Engaged");
		TXString("\x0D\x0A");
	}
	
	//TXString("IT is HERE");
	//Delay10KTCYx(8000);
}	

}
