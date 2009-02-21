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
signed long angle = 0;
signed long angle_out = 0;

int arsVariation = 0;
signed long arsVariationAccumulator = 0;
signed long tempAccumulator = 0;
int angleInteger = 0;

unsigned char adc_channel[4] = {0b10000111,0b10001111,0b10010111,0b10011111};


struct proc_status ProcStatus = {0,0,0,0,0,0};
unsigned char current_parameters[32];
unsigned char current_proc = 0;
unsigned char parameter_count = 0;


int EE_line_threshold = 30;							// 30
int EE_line_follow_threshold = 20;					// 20
unsigned char EE_corner_threshold = 40;				// 40
unsigned int EE_interrupt_throttle = 5000;			// 5000

int antResults[3] = {0,0,0};
int differenceLineFollow = 0;


int antenna_adjustment = 35;                     // difference between left and right antenna readings initialized to 35, but should be set by calibration
float ars_magic;

char lineFollowCurrentState;
char cornerDetectCurrentState;

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
			arsVariation = (int)(temp - arsCalibration.lt);
					
			if(arsVariation > 2 || arsVariation < -2 )
			{
				arsVariationAccumulator += arsVariation;
				adc_pointer = 0;
			}	
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
		
	ars_magic = (float)(4*(ADC_DELAY/1000000)*(13.33/4.883));	
	
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
	
	//  ********* Wait 20,000 instruction cycles for all analog signals (mainly the antennas) to settle out before calibration cycles************
	Delay10KTCYx(2);
	
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
		
		// *** Handle everything currently in the queue. *** //
		while(!isRXEmpty()) {
			if(ProcStatus.ProcessInProgress) {
				popRXQueue(&c);
				current_parameters[parameter_count] = c;
				parameter_count++;
			}
			else {
				ProcStatus.ProcessInProgress = 1;
				popRXQueue(&c);
				current_proc = c;
			}
			switch(current_proc) {
				case RESET_OP:
					Reset();
					ProcStatus.ProcessInProgress = 0;
					break;
				case EEPROM_WR_OP:
					// Do EEPROM_WR stuff here
					ProcStatus.ProcessInProgress = 0;
					parameter_count = 0;
					break;
				case LINE_FOLLOW_OP:
					if(parameter_count = 1) {
						if(current_parameters[0] == 0x31)
						{
							ProcStatus.line_follow_enabled = 1;
						}
						else 
						{
							ProcStatus.line_follow_enabled = 0;
						}	
						
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case CORNER_DETECTION_OP:
					if(parameter_count = 1) {
						if(current_parameters[0] == 0x31)
						{
							ProcStatus.corner_detection_enabled = 1;
						}
						else 
						{
							ProcStatus.corner_detection_enabled = 0;
						}	
						
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case LINE_DETECTION_OP:
					if(parameter_count = 1) {
						if(current_parameters[0] == 0x31)
						{
							ProcStatus.line_detection_enabled = 1;
						}
						else 
						{
							ProcStatus.line_detection_enabled = 0;
						}	
												
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case GET_ANGLE_OP:
					if(parameter_count = 1) {
						if(current_parameters[0] == 0x31)
						{
							ProcStatus.get_angle_enabled = 1;
						}
						else 
						{
							ProcStatus.get_angle_enabled = 0;
						}
											
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case ZERO_ANGLE_OP:
					ProcStatus.zero_angle_enabled = 1;
					ProcStatus.ProcessInProgress = 0;
					parameter_count = 0;
						
			}
		}
		
		// *** Handle execution loop. *** //
				
		// If line following is enabled
		if(ProcStatus.line_follow_enabled) {
			line_follow();
		}
		
		// If corner detection is enabled
		if(ProcStatus.corner_detection_enabled) {
			corner_detection();
		}
		
		// If line detection is enabled
		if(ProcStatus.line_detection_enabled) {
			line_detection();
		}
        
		// If get angle is enabled
		if(ProcStatus.get_angle_enabled)
		{
			get_angle();
		}	
		
		// If zero angle is enabled
		if(ProcStatus.zero_angle_enabled)
		{
			zero_angle();
			ProcStatus.zero_angle_enabled = 0;	
		}	

	}		
}	


void cal_ant(void)
{
	adc_pointer = 0;

	while(adc_pointer < 3)
	{
		SetChanADC(adc_channel[adc_pointer]);
		ConvertADC();
		while( BusyADC() );
		antCalibration[adc_pointer].lt = ReadADC();
		adc_pointer++;
	}
	
	//  ******  Get difference between left and right antenna base readings (essential for line following)  *********
	antenna_adjustment = antCalibration[0].lt - antCalibration[1].lt;	
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

void interrupt(char c) {
	TXChar(c);
	TXString("\x0D\x0A");
}

void line_follow() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
	//left antenna to antResults[0]
	//right antenna to antResults[1]
	
	antResults[0] = antMeasure[0].lt;
	antResults[1] = antMeasure[1].lt;
	
		
	if(antResults[0] < (antCalibration[0].lt + EE_line_threshold) && 
	   antResults[1]  < (antCalibration[1].lt + EE_line_threshold))
	{
		if(INT_LINE_ERROR != lineFollowCurrentState)
		{
			interrupt(INT_LINE_ERROR);
			lineFollowCurrentState = INT_LINE_ERROR;
			ProcStatus.line_follow_enabled = 0;
		}	
	}	
	else
	{
		//   ******  First adjust right antenna reading, and then get differnece between the left and right antenna readings
		differenceLineFollow = antResults[0] - (antResults[1] + antenna_adjustment);
		
		
		//  ******  line follow threshold adjusts the amount
		if(differenceLineFollow > EE_line_follow_threshold)
		{
			if(INT_LINE_LEFT != lineFollowCurrentState)
			{
				interrupt(INT_LINE_LEFT);
				lineFollowCurrentState = INT_LINE_LEFT;				
				ProcStatus.line_follow_enabled = 0;
			}
				
		}	
		else if(differenceLineFollow < (EE_line_follow_threshold * -1))
		{
			if(INT_LINE_RIGHT != lineFollowCurrentState)
			{
				interrupt(INT_LINE_RIGHT);
				lineFollowCurrentState = INT_LINE_RIGHT;
				ProcStatus.line_follow_enabled = 0;
			}
		}	
		else
		{
			if(INT_LINE_CENTER != lineFollowCurrentState)
			{
				interrupt(INT_LINE_CENTER);
				lineFollowCurrentState = INT_LINE_CENTER;
				ProcStatus.line_follow_enabled = 0;
			}	
		}	
	}
}

void corner_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
	antResults[2] = antMeasure[2].lt;
	
	if(antResults[2] > (antCalibration[2].lt + EE_corner_threshold))
	{	
		if(INT_LINE_CENTER != cornerDetectCurrentState)
			{
				interrupt(INT_CORNER_DETECT);
				cornerDetectCurrentState = INT_CORNER_DETECT;
				ProcStatus.corner_detection_enabled = 0;
			}	
		
		
	}
	else
	{
		//interrupt(INT_CORNER_NO_DETECT);
		cornerDetectCurrentState = INT_CORNER_NO_DETECT;			
	}
}

void line_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
	antResults[0] = antMeasure[0].lt;
	antResults[1] = antMeasure[1].lt;
	antResults[2] = antMeasure[2].lt;
	
	if(antResults[0] > (antCalibration[0].lt + EE_line_threshold)) 
	{
		interrupt(INT_LINE_DETECT_LEFT);
		ProcStatus.line_detection_enabled = 0;		
	}
	
	if(antResults[1] > (antCalibration[1].lt + EE_line_threshold))
	{
		interrupt(INT_LINE_DETECT_RIGHT);
		ProcStatus.line_detection_enabled = 0;
	}
	
	if(antResults[2] > (antCalibration[2].lt + EE_corner_threshold))
	{
		interrupt(INT_LINE_DETECT_FRONT);
		ProcStatus.line_detection_enabled = 0;
	}	
			
}
	
void get_angle()
{
	tempAccumulator = arsVariationAccumulator;
	angle = tempAccumulator*ars_magic;
	
	// ***** Check magnitude of angle ****
	angleInteger = angle/360;
	
	//  *****  If the magnitude of angle is greater than 360 *********
	if(1 <= angleInteger || -1 >= angleInteger)
	{
		//  ***** if so, subtract to get a number less than 360 in magnitude *******
		angle_out = angle - (360 * angleInteger);
	}
	
	// ***** make sure output stays between 0 and 360  ******
	if(angle_out < 0)
	{
		angle_out += 360;
	}
	
	// ***** transmit necessary alert character, then a space, then the integer angle  *****
	TXChar(INT_ANGLE);
	TXChar(' ');
	TXDec_Int(angle_out);				
	
	// clear value of angle_out for next iteration...	
	angle_out = 0;		
}
void zero_angle()
{
	arsVariationAccumulator = 0;
}					