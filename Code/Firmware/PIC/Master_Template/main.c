//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include <p18cxxx.h>
#include "Main.h"
#include "i2c.h"

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
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	Init();
	
	while(1)
	{
		IdleI2C();                      // ensure module is idle
  		StartI2C();                     // initiate START condition
  		while ( SSPCON2bits.SEN );      // wait until start condition is over
  		WriteI2C('a');
  		IdleI2C();
  		StopI2C();                      // send STOP condition
 		while ( SSPCON2bits.PEN );      // wait until stop condition is over 
	}	

}


//************************************************************
//							Functions
//************************************************************
int Init (void) 
{	
	Init_Oscillator();
	Init_Interrupts();
	Init_I2C();
	
	TRISB = 0x00;
	LATB = 0x01; // Turn on a little status LED;
	return 1;
}

void Init_Oscillator(void)
{
	OSCCON = 0b01110000; //configure PIC to primary oscillator block at 8MHz
}

void Init_Interrupts(void)
{	
	INTCONbits.GIEL = 1; //low priority interrupts enabler
	INTCONbits.GIEH = 1; //high priority interrupt enabler
	RCONbits.IPEN = 1; //enable high priority and low priority interrupts
}	

void Init_I2C(void)
{
	OpenI2C(MASTER,SLEW_OFF);
	// In Master Mode:
	// Clock = Fosc/(4 * (SSPADD + 1)
	// SSPADD = Fosc/(4 * Fi2c) - 1 = 8meg/4*100k - 1 = 19
	// 19 = 0x13
	// SSPADD = 19;
	SSPADD = 27; //This is so I can actually see stuff on my old O-Scope
}	