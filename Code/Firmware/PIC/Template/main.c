//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include <p18cxxx.h>
#include "Main.h"

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


}


//************************************************************
//							Functions
//************************************************************
int Init (void) 
{	
	Init_Oscillator();
	Init_Interrupts();
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