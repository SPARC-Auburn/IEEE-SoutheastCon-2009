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
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"
#include "op_codes.h"
#include "timers.h"
#include <delays.h>
#include <math.h>


// Union arrays for antennas
union int_byte antCalibration[3] = {0,0,0};
union int_byte antMeasure[3] = {0,0,0};
// Unions for angular rate sensor
union int_byte arsCalibration = 0;
union int_byte arsMeasure = 0;

volatile struct proc_status ProcStatus = {0,0,0,0,0,1,1,1};
unsigned char current_proc = 0;
unsigned char paramter_count = 0;

int EE_line_threshold = 20;							// 20
int EE_line_follow_threshold = 50;					// 50
unsigned char EE_corner_threshold = 30;				// 30
unsigned int EE_interrupt_throttle = 5000;				// 5000

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

#pragma code

//***************************************************************************************************************
//							high_isr
//***************************************************************************************************************

#pragma interrupt high_isr
void high_isr(void)
{
	if(PIR1bits.SSPIF){		// If - SSP Module (I2C)
		unsigned char c;
		if(SSPSTATbits.R_W){	// If - Read from slave
			popTXQueue(&c);
			SSPBUF = c;
		} 
		else {				// Else - Write to Slave
			if(SSPSTATbits.D_A){	// If - Data
				if(SSPSTATbits.BF){		//If - Buffer Full
					c = SSPBUF;	// Grab a char from the I2C Buffer
					if(!isRXFull()){	// Check if QUEUE is FULL
						pushRXQueue(c);	// Write the char to the QUEUE
					}
				}	
			} 
			else {					// Else - Address
				c = SSPBUF;	// Grab a char from the I2C Buffer (Dummy Read)	
			}
		}
		PIR1bits.SSPIF = 0;		// Clear SSP Module Interrupt
		//SSPCON1bits.CKP = 0;	// Release I2C Clock	
	} 
	else {				// Else - Bus Collision (I2C) 
		PIR2bits.BCLIF = 0; 	// Clear Bus Collision Flag
	}	
}

//***************************************************************************************************************
//							low_isr
//***************************************************************************************************************

#pragma interruptlow low_isr
void low_isr (void)
{
	if(PIR1bits.TMR1IF)
	{
		ProcStatus.line_follow_int = 1;
		ProcStatus.corner_detect_int = 1;
		ProcStatus.line_detect_int = 1;
		WriteTimer1(65535 - EE_interrupt_throttle);	
		PIR1bits.TMR1IF = 0;
	}
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************
unsigned char c;
void main (void)
{
	WriteTimer1(65535 - EE_interrupt_throttle);
	// Make sure the antenna calibration runs the first time.
	ProcStatus.cal_ant_enabled = 1;
	while(1) {
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

	LATB &= 0b11111110;
	Delay10TCYx(1);
	LATB |= 0b00000001;
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
		if(ProcStatus.line_follow_int)
		{
			interrupt(INT_LINE_ERROR);
			ProcStatus.line_follow_int = 0;	
		}
	}	
	else
	{
		differenceLineFollow = antMeasure[0].lt - antMeasure[1].lt;
		if(differenceLineFollow > EE_line_follow_threshold)
		{
			if(ProcStatus.line_follow_int)
			{
				interrupt(INT_LINE_LEFT);
				ProcStatus.line_follow_int = 0;		
			}
		}	
		else if(differenceLineFollow > EE_line_follow_threshold)
		{
			if(ProcStatus.line_follow_int)
			{
				interrupt(INT_LINE_RIGHT);
				ProcStatus.line_follow_int = 0;	
			}
		}	
		else
		{
			if(ProcStatus.line_follow_int)
			{
				interrupt(INT_LINE_CENTER);	
				ProcStatus.line_follow_int = 0;	
			}
		}	
	}
}

void corner_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	if(antMeasure[2].lt > antCalibration[2].lt + EE_corner_threshold)
	{
		if(ProcStatus.corner_detect_int)
		{
			interrupt(INT_CORNER_DETECT);
			ProcStatus.corner_detect_int = 0;
		}
	}
	else
	{
	if(ProcStatus.corner_detect_int)
		{
			interrupt(INT_CORNER_NO_DETECT);
			ProcStatus.corner_detect_int = 0;
		}			
	}
}

void line_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
}
