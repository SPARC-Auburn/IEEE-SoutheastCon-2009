#include "hardware.h"
#include "init.h"
#include "serial.h"
#include "i2c.h"
#include "timers.h"

int Init (void) 
{	
	Init_Oscillator();
	Init_Interrupts();
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
	RCONbits.IPEN = 1; //enable high priority and low priority interrupts
	INTCONbits.GIEL = 1; //low priority interrupts enabler
	INTCONbits.GIEH = 1; //high priority interrupt enabler
}	

void Init_USART(void)
{
	int baud = 25;
	
	TXSTA = 0;		// Reset registers
	RCSTA = 0;
	RCSTAbits.CREN = 1;		// Continuous Reception
	TXSTAbits.BRGH = 1;		// High Baud Rate
	PIR1bits.TXIF = 0;
	PIE1bits.RCIE = 0;		// No Receive Interrupt
	PIR1bits.RCIF = 0;
	PIE1bits.TXIE = 0;		// No Transmit Interrupt
	
	SPBRG = baud;			// Write the baud rate
	SPBRGH = baud >> 8;
	
	TXSTAbits.TXEN = 1;		// Enable TX
	RCSTAbits.SPEN = 1;		// Enable RX
	
	TRISCbits.TRISC6 = 0;	// Set Data Directions for PortC pins
	TRISCbits.TRISC7 = 1;	
	
}
