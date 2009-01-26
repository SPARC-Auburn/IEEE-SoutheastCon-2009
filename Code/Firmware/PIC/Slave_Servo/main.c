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
		if(servo[1].lt > servo[0].lt + 10)
		{
			WriteTimer1(0xFFFF - servo[0].lt);
			timer1_temp = 0xFFFF - (servo[1].lt - servo[0].lt);
			timer1_pointer = 0;
		}
		else if(servo[0].lt > servo[1].lt + 10)
		{
			WriteTimer1(0xFFFF - servo[1].lt);	
			timer1_temp = 0xFFFF - (servo[0].lt - servo[1].lt);
			timer1_pointer = 1;
		}
		else
		{
			WriteTimer1(0xFFFF - servo[1].lt);
			timer1_temp = 1;
			timer1_pointer = 10;	
		}	
		
		LATA = 0x0F;
		TMR0L = 100;
		INTCONbits.TMR0IF = 0;
		PIR1bits.TMR1IF = 0;
		PIR2bits.TMR3IF = 0;
		T0CONbits.TMR0ON = 1;
		T1CONbits.TMR1ON = 1;
		T3CONbits.TMR3ON = 1;
		return;
	}
	else if(PIR1bits.TMR1IF)
	{
		if(timer1_temp != 0)
		{
			if(timer1_pointer == 10)
			{
				LATA &= 0x03;
				T1CONbits.TMR1ON = 0;
				PIR1bits.TMR1IF = 0;
				return;
			}	
			else if(timer1_pointer == 0)
			{
				LATA &= 0x01;
				timer1_pointer = 1;
			}
			else if(timer1_pointer == 1)
			{
				LATA &= 0x02;
				timer1_pointer = 0;
			}
			WriteTimer1(timer1_temp);
			timer1_temp = 0;
			T1CONbits.TMR1ON = 1;
			PIR1bits.TMR1IF = 0;
			return;				
		}
		else
		{
			LATA &= ~(1 << timer1_pointer);
			T1CONbits.TMR1ON = 0;
			PIR1bits.TMR1IF = 0;
			return;
		}	
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

	servo[0].lt = 0;
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
