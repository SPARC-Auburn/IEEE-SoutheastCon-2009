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

union Servo servo[4];


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
	if(INTCONbits.TMR0IF)	// If - Timer 0
	{
				
	}	
	else if(PIR1bits.TMR1IF)	// Else - Timer 1
	{
		
	}
	else if(PIR2bits.TMR3IF)	// Else - Timer 3
	{
		
	}
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;
	unsigned char pointer = 0, count = 0, upper = 0, lower = 0;
	Init();

	servo[0].lt = 0;
	servo[1].lt = 0;
	servo[2].lt = 0;
	servo[3].lt = 0;
	
	TXString("\x0D\x0A");		// Put out a new line
	TXString("Servos Initialized to 0");	
	
	
	while(1)
	{
		if(!isQueueEmpty())
		{
			c = popQueue();
			if(count == 0)
			{
				if(c < 4)		// Make sure that we are dealing with a pointer to a servo
				{
					pointer = c;
					count++;	
				}	
			} 
			else {
				servo[pointer].bt[count-1];
				if(count == 3)
				{
					if(servo[pointer].lt > 2500 || servo[pointer].lt < 500) servo[pointer].lt = 0;
					
					TXString(servo[pointer].lt);
				}
				count++;	
			}	
		}		
	}

}
