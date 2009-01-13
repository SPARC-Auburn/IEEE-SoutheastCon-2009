#include "hardware.h"
#include "command.h"
#include "bm_uart.h"
#include "usart.h"

#pragma config OSC = IRCIO67,WDT = OFF, MCLRE = ON

void Initialize(void);

#pragma udata BUFFER
#define TERMINAL_BUFFER 250
unsigned char sample[TERMINAL_BUFFER];
unsigned int sampleCount,b = 0;


/** DECLARATIONS ***************************************************/
#pragma code

void main(void){
	unsigned char c;
	Initialize();
	InitializeUART();
	setBusMode();
	UARTTXString("\x0D\x0A");
	WriteUSART('>'); // Echo a prompt

	while(1){
		if(DataRdyUSART()){
			c=ReadUSART();
			
			switch(c){
				case 0x0A:
				case 0x0D:		// End of the line
					UARTTXString("\x0D\x0A");
					if(b==0){
						UARTTXString("000 SYNTAX ERROR\x0D\x0A");
					} else if ((b==1) && checkCommand(sample[0])){
					} else {
						processCommandString(b, sample);
					}

					WriteUSART('>');
					b=0;
					break;
				case 0x08:		// Backspace
					if(b>0){
						b--;
					}
					break;
				default:		// Default Behavior
					sample[b]=c;
					b++;
					if(b==TERMINAL_BUFFER){
						UARTTXString("\x0D\x0A");
						UARTTXString("099 TERMINAL BUFFER OVERFLOW, TRY SMALLER SEGMENTS");
						b=0;
					}
					break;
			}				
		}
	}
}

void Initialize(void){
	OSCCON = 0b01110000; //configure PIC to primary oscillator block at 8MHz
}
