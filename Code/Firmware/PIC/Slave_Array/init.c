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
	Init_ADC();
	Init_Timers();
	
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
	OpenTimer1(     TIMER_INT_ON   			&
					T1_8BIT_RW              & 
					T1_SOURCE_INT   		& 
					T1_PS_1_2               & 
					T1_OSC1EN_OFF   		&
					T1_SYNC_EXT_OFF);
					
	PIR1bits.TMR1IF = 0;
	IPR1bits.TMR1IP = 0;
	PIE1bits.TMR1IE	= 1;	
}	

void Init_ADC(void)
{
	TRISAbits.TRISA0 = 1;		//set pins 2,3,4, AND 5 as inputs
	TRISAbits.TRISA1 = 1;
	TRISAbits.TRISA2 = 1;
	TRISAbits.TRISA3 = 1;		
	
	
	ADCON0 = 0x01;				//turn on the A/D converter, and configure pin 2 as the analog input for the A/D converter								
								
	ADCON1 = 0x0D;				//configure reference voltages to Vdd and Vss, and configure pins 2 and 3 as 
								//analog pins and configure pins 4 and 5 as digital pins
	
	ADCON2 = 0xA9;				//configure A/D converter result registers as right justified, acquisition time set to 12 times
								//the AD timer, AD timer set to 1/8th the oscillator frequency	
}	