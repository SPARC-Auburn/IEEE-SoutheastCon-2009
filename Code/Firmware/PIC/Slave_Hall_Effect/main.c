//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "main.h"
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"
#include <delays.h>

int high,low;


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
	unsigned char c;
	Init();
	TXString("\x0D\x0A");		//Put out a new line
	TXChar('>');	
	
	TRISAbits.TRISA0 = 1;		//set pins 2 and 3 as inputs
	TRISAbits.TRISA1 = 1;
	TRISAbits.TRISA2 = 0;		//set pins 4 and 5 as outputs
	TRISAbits.TRISA3 = 0;
	
	ADCON0 = 0x01;				//turn on the A/D converter, and configure pin 2 as the analog input for the A/D converter								
								
	ADCON1 = 0x0D;				//configure reference voltages to Vdd and Vss, and configure pins 2 and 3 as 
								//analog pins and configure pins 4 and 5 as digital pins
	
	ADCON2 = 0xA9;				//configure A/D converter result registers as right justified, acquisition time set to 12 times
								//the AD timer, AD timer set to 1/8th the oscillator frequency
		
	while(1)
		{						//start the A/D converter 
			ADCON0bits.GO = 1;
								
								//wait for the A/D converter to finish
			while(ADCON0bits.GO)
			{
			}
								
			high = ADRESH;		//store A/D results in character variables
			low = ADRESL;		
			
			TXString("HIGH:");	
			TXDec_Int(high);
			TXString("\x0D\x0A");
			TXDec_Int(low);		
			TXString("\x0D\x0A");		
								
			Delay10TCYx(1);		//delay before starting A/D converter again
		}
}
