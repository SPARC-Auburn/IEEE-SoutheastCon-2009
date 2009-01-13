//************************************************************************************************
//	Node:		Generic Template
//************************************************************************************************

#include <p18cxxx.h>
#include "main.h"
#include "i2c.h"
#include "usart.h"
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
#define TERMINAL_BUFFER 200
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

	init();
	//WriteUSART(0x11);
	putrsUSART("\x0D\x0A");
	while(BusyUSART());
	WriteUSART('>');
	b =0;
	while(1)
	{
		if(DataRdyUSART()){
			c=ReadUSART();	
			
			switch(c){
				case 0x0A:
				case 0x0D:
					putrsUSART("\x0D\x0A");
					if(b==0){
						putrsUSART("000 SYNTAX ERROR\x0D\x0A");
					}
					WriteUSART('>');
					b=0;
					break;
				case 0x08:
					if(b>0)b--;
					WriteUSART(c);
					break;
				default:
					sample[b]=c;
					b++;	
					if(b==TERMINAL_BUFFER){
						putrsUSART("099 TERMINAL BUFFER OVERFLOW\x0D\x0A");
						b=0;
					}
					break;
			}
		}
	}					
}


//************************************************************
//							Functions
//************************************************************
int init (void) 
{	
	Init_Oscillator();
//	Init_Interrupts();
	Init_I2C();
	Init_USART();
	
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

void Init_USART(void)
{
	OpenUSART(  USART_TX_INT_OFF  &
            	USART_RX_INT_OFF  &
            	USART_ASYNCH_MODE &
            	USART_EIGHT_BIT   &
            	USART_CONT_RX     &
            	USART_BRGH_HIGH,
            	25                );
}
