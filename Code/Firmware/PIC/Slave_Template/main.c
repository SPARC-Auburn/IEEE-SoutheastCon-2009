//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "main.h"
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"

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
}

//***************************************************************************************************************
//							low_isr
//***************************************************************************************************************

#pragma interruptlow low_isr
void low_isr (void)
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
		SSPCON1bits.CKP = 0;	// Release I2C Clock	
	} 
	else {				// Else - Bus Collision (I2C) 
		PIR2bits.BCLIF = 0; 	// Clear Bus Collision Flag
	}	

}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;s
	pointer = 0;
	Init();
	TXString("\x0D\x0A");		// Put out a new line
	TXChar('>');	
	
	while(1){
		if(!isQueueEmpty()){
			c = popQueue();
			TXHex(c);
			TXString("\x0D\x0A");		
		}		
	}

}
