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
volatile unsigned int timer1_reload, timer3_reload;
volatile unsigned char timer1_mask, timer3_mask;

unsigned char pwm_deadband = 15;		// Default value, can be varied through I2C

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
	if(PIR1bits.TMR1IF)
	{
		LATA &= timer1_mask;
		if(timer1_reload != 0)
		{
			WriteTimer1(timer1_reload);
			timer1_mask = SERVO0 & SERVO1;
		}	
		else
		{
			T1CONbits.TMR1ON = 0;
			timer1_reload = 0;
		}	
		PIR1bits.TMR1IF = 0;
		return;
	}
	else if(PIR2bits.TMR3IF)
	{
		LATA &= timer3_mask;
		if(timer3_reload != 0)
		{
			WriteTimer3(timer3_reload);
			timer3_mask = SERVO2 & SERVO3;
		}	
		else
		{
			T3CONbits.TMR3ON = 0;
			timer3_reload = 0;
		}	
		PIR2bits.TMR3IF = 0;
		return;
	}	
	else if(INTCONbits.TMR0IF)	// If - Timer 0
	{
		if(servo[0].lt > (servo[1].lt + pwm_deadband))
		{
			WriteTimer1(0xFFFF - servo[1].lt);
			timer1_reload = 0xFFFF - (servo[0].lt - servo[1].lt);
			timer1_mask = SERVO1; 
		}
		else if(servo[1].lt > (servo[0].lt + pwm_deadband))
		{
			WriteTimer1(0xFFFF - servo[0].lt);
			timer1_reload = 0xFFFF - (servo[1].lt - servo[0].lt);
			timer1_mask = SERVO0;	
		}	
		else
		{
			WriteTimer1(0xFFFF - servo[0].lt);
			timer1_reload = 0;
			timer1_mask = SERVO0 & SERVO1;	
		}
		
		if(servo[2].lt > (servo[3].lt + pwm_deadband))
		{
			WriteTimer3(0xFFFF - servo[3].lt);
			timer3_reload = 0xFFFF - (servo[2].lt - servo[3].lt);
			timer3_mask = SERVO3; 
		}
		else if(servo[3].lt > (servo[2].lt + pwm_deadband))
		{
			WriteTimer3(0xFFFF - servo[2].lt);
			timer3_reload = 0xFFFF - (servo[3].lt - servo[2].lt);
			timer3_mask = SERVO2;	
		}	
		else
		{
			WriteTimer3(0xFFFF - servo[2].lt);
			timer3_reload = 0;
			timer3_mask = SERVO2 & SERVO3;	
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
	servo[1].lt = 1500;
	servo[2].lt = 1500;
	servo[3].lt = 1500;
	
	TXString("\x0D\x0A");		// Put out a new line
	TXString("Servos Initialized to 1500");	
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
