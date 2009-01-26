  //************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "main.h"
#include "i2c.h"
#include "init.h"
#include "timers.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"

#pragma config OSC = IRCIO67,WDT = OFF, MCLRE = ON

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
volatile unsigned int timer1_temp;
volatile unsigned int timer3_temp;
volatile unsigned char timer1_pointer;
volatile unsigned char timer3_pointer;

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
		WriteTimer1(0xFFFF - servo[0].lt);	
		
		LATA = 0x01;
		TMR0L = 100;
		
		INTCONbits.TMR0IF = 0;
		PIR1bits.TMR1IF = 0;
		T0CONbits.TMR0ON = 1;
		T1CONbits.TMR1ON = 1;
		return;
	}
	else if(PIR1bits.TMR1IF)
	{
		LATA = 0x00;
		T1CONbits.TMR1ON = 0;
		PIR1bits.TMR1IF = 0;
	}				
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;
	unsigned char pointer = 0, count = 0;
	union Servo temp;
	
	Init();

	servo[0].lt = 1500;
	servo[1].lt = 0;
	servo[2].lt = 0;
	servo[3].lt = 0;
	
	TXString("\x0D\x0A");		// Put out a new line
	TXString("Servos Initialized to 0");	
	TXString("\x0D\x0A");
	
	while(1)
	{
		if(!isQueueEmpty())
		{
			c = popQueue();
			#ifdef __DEBUG
				TXString("popQueue() = ");
				TXHex(c);
				TXString("\x0A\x0D");
			#endif
			
			if(count == 0 && c < 5 && c > 0)
			{
				pointer = c - 1;
				count++;
				#ifdef __DEBUG
					TXString("pointer = ");
					TXHex(c);
					TXString("\x0A\x0D");
				#endif
			}
			else if(count == 1)
			{
				temp.bt[1] = c; 
				count++;
			}
			else if(count == 2)
			{
				temp.bt[0] = c;
				servo[pointer].lt = temp.lt;
				
				#ifdef __DEBUG
					TXString("Servo Set: ");
					TXDec(pointer);
					TXString("  To Position: ");
					TXDec_Int(servo[pointer].lt);
					TXString("\x0A\x0D");
				#endif	

				count = 0;			
			}	
			
					
		}		
	}
}
