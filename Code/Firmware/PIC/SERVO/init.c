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
	Init_Timers();
	
	TRISA = 0x00;		// Make PORTA all outputs
	LATA = 0x00;		// Initialize PORTA to 0
	
	TRISB = 0x00;		// Make PORTB all outputs
	LATB = 0x01; 		// Turn on a little status LED;
	return 1;
}

void Init_Oscillator(void)
{
	OSCCON = 0b01110000; 	// Configure PIC to primary oscillator block at 8MHz
	OSCTUNEbits.PLLEN = 1;	// Enable the PLL for 32MHz
}

void Init_Interrupts(void)
{	
	RCONbits.IPEN = 1; 		//enable high priority and low priority interrupts
	INTCONbits.GIEL = 1; 	//low priority interrupts enabler
	INTCONbits.GIEH = 1; 	//high priority interrupt enabler
}	

void Init_USART(void)
{
	// SPBRG = (Fosc/Baud)/4 - 1
	// Fosc = 32000000
	// Baud = 115200
	// SPBRG = (32000000/115200)/4 - 1
	// SPBRG = 277.778/4 - 1 = 69.4444 - 1 = 68.4444
	
	// Actual Baud = Fosc/(4*(SPBRG + 1)) = 32000000/(4*(68 + 1)) = 115942 baud
	// Error = (115942-115200)/115200 * 100% = 0.6%
	unsigned int baud=416;
	
	TXSTA = 0;		// Reset registers
	RCSTA = 0;
	RCSTAbits.CREN = 1;		// Continuous Reception
	TXSTAbits.BRGH = 1;		// High Baud Rate
	BAUDCONbits.BRG16 = 1;	// 16-bit Baud Rate counter
	
	TXSTAbits.TXEN = 0;
	RCSTAbits.SPEN = 0;	

	SPBRG = baud;			// Write the baud rate
	SPBRGH = baud >> 8;
	
	TXSTAbits.TXEN = 1;		// Enable TX
	RCSTAbits.SPEN = 1;		// Enable RX
	
	TRISCbits.TRISC6 = 0;	// Set Data Directions for PortC pins
	TRISCbits.TRISC7 = 1;

	PIR1bits.RCIF = 0;
	PIE1bits.RCIE = 1;		// Enable Receive Interrupt
	IPR1bits.RCIP = 1;		// High Priority
	PIR1bits.TXIF = 0;
	PIE1bits.TXIE = 1;		// Enable Transmit Interrupt
	IPR1bits.TXIP = 1;		// High Priority	
	
}

void Init_Timers(void)
{
	// Timer 0 Setup
	OpenTimer0(	TIMER_INT_ON 	& 
				T0_16BIT 		&
				T0_SOURCE_INT	&
				T0_PS_1_256);
	WriteTimer0(64910);
	
	INTCONbits.TMR0IF = 0;
	INTCON2bits.TMR0IP = 0;	// Low Priority
	INTCONbits.TMR0IE = 1;	//Enable Interrupt
	
	// Timer 1 Setup
	OpenTimer1(	TIMER_INT_ON 	& 
				T1_8BIT_RW 		& 
				T1_SOURCE_INT 	& 
				T1_PS_1_8		& 
				T1_OSC1EN_OFF  	&
				T1_SYNC_EXT_OFF);
	WriteTimer1(64035);
	
	PIR1bits.TMR1IF = 0;
	IPR1bits.TMR1IP = 0;	// Low Priority
	PIE1bits.TMR1IE = 1;
	
	OpenTimer3(	TIMER_INT_ON	&
				T3_8BIT_RW		&
				T3_SOURCE_INT	&
				T3_PS_1_8		&
				T3_OSC1EN_OFF	&
				T3_SYNC_EXT_OFF	);
	WriteTimer3(64035);
	
	PIR2bits.TMR3IF = 0;
	IPR2bits.TMR3IP = 0;
	PIE2bits.TMR3IE = 1;
}	
