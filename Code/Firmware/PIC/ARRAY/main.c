//************************************************************************************************
//	Node:		MAGNETIC FEILD DETECTION
/*
	Wiring Instructions:
	
	pin 2:   left antenna
	pin 3:   right antenna
	pin 4:   front antenna
*/
//************************************************************************************************

#include "main.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"
#include "op_codes.h"

// Use Internal Oscillator, Watchdog Timer, MCLR, and Brown Out Detect
#pragma config OSC = IRCIO67
#pragma config WDT = ON, WDTPS=32768
#pragma config MCLRE = ON
#pragma config BOREN = BOHW,BORV = 3



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

// Union arrays for antennas
union int_byte antCalibration[3] = {0,0,0};
union int_byte antMeasure[3] = {0,0,0};

// Unions for angular rate sensor
union int_byte arsCalibration = 0;
union int_byte arsMeasure = 0;

unsigned char adc_pointer = 0;
unsigned char ars_pointer = 0;
long angle = 0;
long angle_temp = 0;

unsigned char adc_channel[4] = {0b10000111,0b10001111,0b10010111,0b10011111};

int EE_line_threshold = 20;							// 20
int EE_line_follow_threshold = 50;					// 50
unsigned char EE_corner_threshold = 30;				// 30
unsigned int EE_interrupt_throttle = 5000;				// 5000

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
}

//***************************************************************************************************************
//							low_isr
//***************************************************************************************************************

#pragma interruptlow low_isr
void low_isr (void)
{
	if(INTCONbits.TMR0IF)
	{
		int temp = ReadADC(); 
		if(adc_pointer < 3)
		{
			antMeasure[adc_pointer].lt = temp;
			adc_pointer++;
		} 
		else 
		{
			angle += temp - arsCalibration.lt;
			adc_pointer = 0;
		}	
		SelChanConvADC(adc_channel[adc_pointer]);		
		WriteTimer0(65536-ADC_DELAY);
		INTCONbits.TMR0IF = 0;
	}	
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************
unsigned char c;

void main (void)
{
	Init();
	initQueue();
		
	Delay10KTCYx(1);		// Build in a delay to prevent weird serial characters

	if(isWDTTO())
	{
		TXString("RST ARRAY - WDT\x0A\x0D");
	}
	else if(isMCLR())
	{
		TXString("RST ARRAY - MCLR\x0A\x0D");	
	}
	else if(isPOR())	
	{
		TXString("RST ARRAY - POR\x0A\x0D");
	}
	else if(isBOR())
	{
		TXString("RST ARRAY - BOR\x0A\x0D");
	}
	else
	{
		TXString("RST ARRAY\x0A\x0D");	
	}	
	StatusReset();
	
	cal_ant();
	cal_ars();
	#ifdef __DEBUG
		TXString("Calibration Complete, Antenna Values: ");
		TXDec_Int(antCalibration[0].lt);
		TXChar(' ');
		TXDec_Int(antCalibration[1].lt);
		TXChar(' ');
		TXDec_Int(antCalibration[2].lt);
		TXChar(' ');
		TXString("\x0D\x0A ARS Value: ");
		TXDec_Int(arsCalibration.lt);
		TXString("\x0D\x0A");
	#endif
		
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 1;
	
	while(1)
	{
		ClrWdt();
		Delay10KTCYx(20);
        
        TXString("Calibration Complete, Antenna Values: ");
		TXDec_Int(antMeasure[0].lt);
		TXChar(' ');
		TXDec_Int(antMeasure[1].lt);
		TXChar(' ');
		TXDec_Int(antMeasure[2].lt);
		TXChar(' ');
		angle_temp = angle/MagicNumber;
		// Here is where we can put an angle/magicNumber command.
		//TXString(" ARS Value: ");
		//TXDec_Int(arsCalibration.lt);
		TXString("\x0D\x0A");

	}		
}	


void cal_ant(void)
{
	adc_pointer = 0;
	Delay10KTCYx(2);
	while(adc_pointer < 3)
	{
		SetChanADC(adc_channel[adc_pointer]);
		ConvertADC();
		while( BusyADC() );
		antCalibration[adc_pointer].lt = ReadADC();
		adc_pointer++;
	}	
}

void cal_ars(void)
{
	int temp = 0;
	char i = 0;
	for(i = 0; i < 8; i++)
	{
		adc_pointer = 3;
		SetChanADC(adc_channel[adc_pointer]);
		ConvertADC();
		while( BusyADC() );
		temp += ReadADC();
	}
	temp = (temp >> 3);	
	arsCalibration.lt = temp;
}			