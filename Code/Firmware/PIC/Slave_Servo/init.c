#include "hardware.h"
#include "init.h"
#include "serial.h"
#include "i2c.h"
#include "timers.h"

int Init (void) 
{	
	Init_Oscillator();
	Init_I2C();
	Init_Interrupts();
	Init_USART();
	Init_Timers();
	
	TRISA = 0x00;
	LATA = 0x00;
	
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

void Init_I2C(void)
{
	#ifdef MASTER
		OpenI2C(MASTER,SLEW_OFF);
		// In Master Mode:
		// Clock = Fosc/(4 * (SSPADD + 1)
		// SSPADD = Fosc/(4 * Fi2c) - 1 = 8meg/4*100k - 1 = 19
		// 19 = 0x13
		// SSPADD = 19;
		SSPADD = 27; //This is so I can actually see stuff on my old O-Scope
	#endif
	//SSPCON2bits.SEN = 1;
	OpenI2C(SLAVE_7,SLEW_OFF);
	SSPADD = 0x10;
	PIR1bits.SSPIF = 0; 
	PIE1bits.SSPIE = 1;		//Enable Interrupt	
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

void Init_Timers(void)
{
	OpenTimer0(	TIMER_INT_ON 	& 
				T0_8BIT 		&
				T0_SOURCE_INT	&
				T0_PS_1_256);
	WriteTimer0(100);
	
	INTCONbits.TMR0IF = 0;
	INTCON2bits.TMR0IP = 0;	// Low Priority
	INTCONbits.TMR0IE = 1;	//Enable Interrupt
	
	// Timer 1 Setup
	OpenTimer1(	TIMER_INT_ON 	& 
				T1_8BIT_RW 		& 
				T1_SOURCE_INT 	& 
				T1_PS_1_2		& 
				T1_OSC1EN_OFF  	&
				T1_SYNC_EXT_OFF);
	WriteTimer1(64035);
	
	PIR1bits.TMR1IF = 0;
	IPR1bits.TMR1IP = 0;	// Low Priority
	PIE1bits.TMR1IE = 1;
	
	OpenTimer3(	TIMER_INT_ON	&
				T3_8BIT_RW		&
				T3_SOURCE_INT	&
				T3_PS_1_2		&
				T3_OSC1EN_OFF	&
				T3_SYNC_EXT_OFF	);
	WriteTimer3(64035);
	
	PIR2bits.TMR3IF = 0;
	IPR2bits.TMR3IP = 0;
	PIE2bits.TMR3IE = 1;
}	
