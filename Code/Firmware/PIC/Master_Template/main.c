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

#pragma config OSC = IRCIO67,WDT = OFF, WDTPS = 32768,MCLRE = ON, PBADEN = OFF

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
#define TERMINAL_BUFFER 200
unsigned char sample[TERMINAL_BUFFER];
unsigned int sampleCount,b;


#pragma code
volatile unsigned char INT0_count;
volatile unsigned char INT1_count;
volatile unsigned char INT2_count;

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

	if(INTCONbits.INT0IF == 1)
	{
		INT0_count++;
		INTCONbits.INT0IF = 0;	
	}
	else if(INTCON3bits.INT1IF == 1)
	{
		INT1_count++;;
		INTCON3bits.INT1IF = 0;	
	}		
	else if(INTCON3bits.INT2IF == 1)
	{
		INT2_count++;
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
	
	INT0_count = 0;
	INT1_count = 0;
	INT2_count = 0;

	interrupt_0_addr = Read_b_eep(0x0000);
	interrupt_1_addr = Read_b_eep(0x0001);
	interrupt_2_addr = Read_b_eep(0x0002);
	
	while(1)
	{	
		if(INT0_count > 0)
		{	
			#ifdef __DEBUG
				TXString("I ");
				TXDec(interrupt_0_addr);
				TXString(" ");
				StartI2C();
				WriteI2C(interrupt_0_addr);
				TXDec(ReadI2C());
				StopI2C();
				TXString("\x0A\x0D");
				INT0_count--;
			#endif
		}
		if(INT1_count > 0)
		{
			#ifdef __DEBUG
				TXString("INT1, Reading: ");
				TXHex(interrupt_1_addr);
				TXString(" ==== Returned: ");
				//StartI2C();
				//WriteI2C(interrupt_1_addr);
				//TXHex(ReadI2C());
				//StopI2C();
				TXString("\x0A\x0D");
				INT1_count--;
			#endif
		}
		if(INT2_count > 0)
		{
			#ifdef __DEBUG
				TXString("INT2, Reading: ");
				TXHex(interrupt_2_addr);
				TXString(" ==== Returned: ");
				//StartI2C();
				//WriteI2C(interrupt_2_addr);
				//TXHex(ReadI2C());
				//StopI2C();
				TXString("\x0A\x0D");
				INT2_count--;
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
