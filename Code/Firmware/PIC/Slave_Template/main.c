//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "main.h"
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"

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
	if(SSPSTATbits.R_W == 0){
		SSPBUF = 0x30;	
	} else {
		pointer = SSPBUF;
	}	
	PIR1bits.SSPIF =0;
		
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;
	pointer = 0;
	Init();
	TXString("\x0D\x0A");		// Put out a new line
	TXChar('>');	
	
	while(1){
		if(RXReady()){
			TXChar(pointer);
			c = RXChar();
		}	
	}		

}
