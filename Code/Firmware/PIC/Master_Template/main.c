//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "command.h"
#include "hardware.h"
#include "init.h"
#include "main.h"
#include "serial.h"


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

#pragma udata BUFFER
#define TERMINAL_BUFFER 250
unsigned char sample[TERMINAL_BUFFER];
unsigned int sampleCount,b;


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
	unsigned char c;

	Init();						// Run Hardware Initialization
	TXString("\x0D\x0A");		// Put out a new line
	setBusMode();
	TXChar('>');			// Print Command Prompt
	b =0;
	while(1)
	{
		if(RXReady()){		// Wait for input
			c=RXChar();	
			
			switch(c){
				case 0x0A:
				case 0x0D:		// End of Line
					TXString("\x0D\x0A");
					if(b==0){
						TXString("000 SYNTAX ERROR\x0D\x0A");
					} else if (b==1 && checkCommand(sample[0])){
						// Do Nothing
					} else {
						processCommandString(b, sample);
					}		
					TXChar('>');
					b=0;
					break;
				case 0x08:		// Backspace
					if(b>0)b--;
					TXChar(c);
					break;
				default:
					sample[b]=c;
					b++;	
					if(b==TERMINAL_BUFFER){
						TXString("099 TERMINAL BUFFER OVERFLOW\x0D\x0A");
						b=0;
					}
					break;
			}
		}
	}					
}
