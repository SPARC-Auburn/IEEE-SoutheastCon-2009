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

// Union arrays for antennas
union int_byte antCalibration[3] = {0,0,0};
union int_byte antMeasure[3] = {0,0,0};
// Unions for angular rate sensor
union int_byte arsCalibration = 0;
union int_byte arsMeasure = 0;

struct proc_status ProcStatus = {0,0,0,0,0};
unsigned char current_proc = 0;
unsigned char paramter_count = 0;

int EE_line_threshold = 20;							// 20
int EE_line_follow_threshold = 50;					// 50
unsigned char EE_corner_threshold = 30;				// 30
unsigned int EE_interrupt_throttle = 5000;				// 5000


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
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************
unsigned char c;
void main (void)
{
	Init();
	initQueue();
	
	Delay10KTCYx(10);		// Build in a delay to prevent weird serial characters

	if(isWDTTO())
	{
		TXString("RST SERVO - WDT\x0A\x0D");
	}
	else if(isMCLR())
	{
		TXString("RST SERVO - MCLR\x0A\x0D");	
	}
	else if(isPOR())	
	{
		TXString("RST SERVO - POR\x0A\x0D");
	}
	else if(isBOR())
	{
		TXString("RST SERVO - BOR\x0A\x0D");
	}
	else
	{
		TXString("RST SERVO\x0A\x0D");	
	}	
	StatusReset();
	
	// Make sure the antenna calibration runs the first time.
	ProcStatus.cal_ant_enabled = 1;
	while(1) {
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
				case CAL_ANT_OP:
					ProcStatus.cal_ant_enabled = 1;
					break;
				case LINE_FOLLOW_OP:
					if(parameter_count = 1) {
						ProcStatus.line_follow_enabled = current_parameters[0];
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case CORNER_DETECTION_OP:
					if(parameter_count = 1) {
						ProcStatus.corner_detection_enabled = current_parameters[0];
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				case LINE_DETECTION_OP:
					if(parameter_count = 1) {
						ProcStatus.line_detection_enabled = current_parameters[0];
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
			}
		}
		// *** Handle execution loop. *** //
		// If calibration needs to be run
		if(ProcStatus.cal_ant_enabled) {
			cal_ant();
			ProcStatus.cal_ant_enabled = 0;
		}
		
		// Read the Antennas
		read_antennas();
		
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
	}
}


void read_antennas(void) {
	// This method should read the antennas values and store them in global variables.
	// The other functions will use these readings to chack against thresholds set in the eeprom.
	unsigned char channel;
	channel = 0;
	while(channel < 3)
	{
		ADCON0 = ((channel << 2) & 0b00111100) |
          				(ADCON0 & 0b11000011);
		ADCON0bits.GO = 1;			//start the A/D converter 
		while(ADCON0bits.GO){}		//wait for the A/D converter to finish
		antMeasure[channel].bt[0] = ADRESL;
		antMeasure[channel].bt[1] = ADRESH;
		Delay10TCYx(1);		//delay before starting A/D converter again
		channel++;
	}		
}

void read_ars(void) {
	
}	

void interrupt(char c) {
	pushTXQueue(c);
}

void cal_ars(void) {
	unsigned char i = 0;
	while(i < 3)
	{		
		ADCON0 = ((3 << 2) & 0b00111100) |
           				(ADCON0 & 0b11000011);					 
		ADCON0bits.GO = 1;			//start the A/D converter
		while(ADCON0bits.GO){}		//wait for the A/D converter to finish
		arsCalibration.bt[0] = ADRESL;
		arsCalibration.bt[1] = ADRESH;
		i++;
		Delay10KTCYx(990);		//delay before starting A/D converter again		
	}
}		

void cal_ant(void) {
	unsigned char i = 0;
	unsigned char channel;	
	while(i < 3)
	{	
		channel = 0;
		while(channel < 3)
		{
			ADCON0 = ((channel << 2) & 0b00111100) |
           				(ADCON0 & 0b11000011);
			ADCON0bits.GO = 1;			//start the A/D converter 
			while(ADCON0bits.GO){}		//wait for the A/D converter to finish
			antCalibration[channel].bt[0] = ADRESL;
			antCalibration[channel].bt[1] = ADRESH;
			Delay10TCYx(1);		//delay before starting A/D converter again
			channel++;
		}
		i++;
		Delay10TCYx(990);		//delay before starting A/D converter again		
	}	
}

void line_follow() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
	int differenceLineFollow;
	differenceLineFollow = 0;
	
	if(antMeasure[0].lt < (antCalibration[0].lt + EE_line_threshold) && 
		antMeasure[1].lt  < (antCalibration[1].lt + EE_line_threshold))
	{
		interrupt(INT_LINE_ERROR);
	}	
	else
	{
		differenceLineFollow = antMeasure[0].lt - antMeasure[1].lt;
		if(differenceLineFollow > EE_line_follow_threshold)
		{
			interrupt(INT_LINE_LEFT);	
		}	
		else if(differenceLineFollow > EE_line_follow_threshold)
		{
			interrupt(INT_LINE_RIGHT);
		}	
		else
		{
			interrupt(INT_LINE_CENTER);	
		}	
	}
}

void corner_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	if(antMeasure[2].lt > antCalibration[2].lt + EE_corner_threshold)
	{
		interrupt(INT_CORNER_DETECT);
	}
	else
	{
		interrupt(INT_CORNER_NO_DETECT);			
	}
}

void line_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
}
	