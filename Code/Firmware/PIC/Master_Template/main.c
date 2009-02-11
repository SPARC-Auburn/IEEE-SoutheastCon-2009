//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include "command.h"
#include "hardware.h"
#include "init.h"
#include "main.h"
#include "serial.h"
#include "queue.h"
#include "eep.h"

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
volatile unsigned char external_interrupts = 0;

//***************************************************************************************************************
//							high_isr
//***************************************************************************************************************
#pragma interrupt high_isr
void high_isr(void)
{	
	if(PIR1bits.TXIF || PIR1bits.RCIF)
	{
		SerialISR();
	}
	else if(INTCONbits.INT0IF)
	{
		external_interrupts |= 0x01;
		INTCONbits.INT0IF = 0;	
	}
	else if(INTCON3bits.INT1IF)
	{
		external_interrupts |= 0x02;
		INTCON3bits.INT1IF = 0;	
	}		
	else if(INTCON3bits.INT2IF)
	{
		external_interrupts |= 0x04;
		INTCON3bits.INT2IF = 0;
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
	unsigned char c, interrupt_0_addr, interrupt_1_addr, interrupt_2_addr;

	Init();						// Run Hardware Initialization
	TXString("\x0D\x0A");		// Put out a new line
	setBusMode();
	TXChar('>');			// Print Command Prompt
	b = 0;
	
	interrupt_0_addr = Read_b_eep(0x0000);
	interrupt_1_addr = Read_b_eep(0x0001);
	interrupt_2_addr = Read_b_eep(0x0002);
	
	while(1)
	{	
		if(external_interrupts&0x01)
		{	
			#ifdef __DEBUG
				TXString("External Interrupt 0 called:, Reading:  ");
				TXBin(interrupt_0_addr);
			#endif
		}
		if(external_interrupts&0x02)
		{
			#ifdef __DEBUG
				TXString("External Interrupt 1 called:, Reading:  ");
				TXBin(interrupt_0_addr);
			#endif
		}
		if(external_interrupts&0x04)
		{
			#ifdef __DEBUG
				TXString("External Interrupt 2 called:, Reading:  ");
				TXBin(interrupt_0_addr);
			#endif
		}			
		if(!isRXEmpty()){		// Wait for input
			popRXQueue(&c);	
			
			switch(c){
				case 0x0A:
				case 0x0D:		// End of Line
					TXString("\x0D\x0A");
					if(b==0){
						TXString("000\x0D\x0A"); // syntax Error
					} else if (b==1 && checkCommand(sample[0])){
						// Do Nothing
					} else {
						processCommandString(b, sample);
					}		
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
						TXString("099\x0D\x0A"); // Buffer overflow
						b=0;
					}
					break;
			}
		}
	}					
}
