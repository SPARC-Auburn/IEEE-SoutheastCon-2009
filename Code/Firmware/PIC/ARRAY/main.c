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
#include "eealloc.h"

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


int EEP_count = -1;
union int_byte EEP_address;
unsigned char EEP_offset = 0;

union int_byte antCalibration[3] = {0,0,0};
union int_byte antMeasure[3] = {0,0,0};


// Unions for angular rate sensor
union int_byte arsCalibration = 0;
union int_byte arsMeasure = 0;

unsigned char adc_pointer = 0;
unsigned char ars_pointer = 0;

//**** ARS variables *******
int angle = 0;
int angle_out = 0;
int arsVariation = 0;
signed long arsVariationAccumulator = 0;
signed long tempAccumulator = 0;
int angleInteger = 0;

unsigned char adc_channel[4] = {0b10000111,0b10001111,0b10010111,0b10011111};


struct proc_status ProcStatus = {0,0,0,0,0,0,0,0};
unsigned char current_parameters[32];
unsigned char current_proc = 0;
unsigned char parameter_count = 0;

//***************  VARIABLES FOR EEPROM BEGIN *****************

int line_threshold = 30;							// 30
int line_follow_threshold = 40;						// 20
int corner_threshold = 40;							// 40

//**************  VARIABLES FOR EEPROM END  ***************

int antResults[3] = {0,0,0};
int differenceLineFollow = 0;


int antenna_adjustment = 35;                     // difference between left and right antenna readings initialized to 35, but should be set by calibration

float ars_magic;

char lineFollowCurrentState = 0x00;
char cornerDetectCurrentState = 0x00;

char desiredAngle;
int monitorAngleCounter;
int originalAngle;
int upperMonitorThreshold;
int lowerMonitorThreshold;

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
			
//			#ifdef __DEBUG
//				TXString("ARS Variation");
//				TXDec_Int(arsVariation);
//				TXString("\r\n");
//			#endif
					
			if(arsVariation > 6 || arsVariation < -6 )
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
	EEP_address.lt = 0;
	Refresh_EEPROM();
	Init();
	initQueue();
		
	ars_magic = (float)(4.883/(1.75*((float)ADC_DELAY/1000000)*12.75));	
	
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
				
				#ifdef __DEBUG
				TXString("DEBUG - Parameter added: ");
				TXChar(c);
				TXString("\r\n");
				#endif
			}
			else {
				ProcStatus.ProcessInProgress = 1;
				popRXQueue(&c);
				current_proc = c;
				#ifdef __DEBUG
				TXString("DEBUG - Current Process: ");
				TXChar(current_proc);
				TXString("\r\n");
				#endif
			}
			switch(current_proc) {
				case RESET_OP:
					Reset();
					ProcStatus.ProcessInProgress = 0;
					break;

				case EEPROM_WR_OP:
					if(parameter_count == 3)
					{
						EEP_count = current_parameters[2];	
					}	
					
					if(EEP_count != -1 && (parameter_count == EEP_count + 3))
					{
						EEP_address.bt[1] = current_parameters[0];
						EEP_address.bt[0] = current_parameters[1];
						EEP_offset = 0;
						while(EEP_offset < EEP_count)
						{
							Write_b_eep(EEP_address.lt, current_parameters[EEP_offset + 3]);
							Busy_eep();
							EEP_address.lt++;
							EEP_offset++;	
						}
						EEP_count = -1;
						EEP_address.lt = 0;
						Refresh_EEPROM();
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;	
					}	
					break;
				case EEPROM_RD_OP:
					if(parameter_count == 2)
					{
						EEP_address.bt[1] = current_parameters[0];
						EEP_address.bt[0] = current_parameters[1];
						EEP_offset = Read_b_eep(EEP_address.lt);
						TXString("0 ");
						TXHex(EEP_offset);
						TXString("\x0A\x0D");
						EEP_address.lt = 0;
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					} 
					else if(parameter_count > 2)
					{
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;					
					}
					break;
				case LINE_FOLLOW_OP:
					if(parameter_count == 1) {
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
					if(parameter_count == 1) {
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
					if(parameter_count == 1) {
						if(current_parameters[0] == 0x31)
						{
							#ifdef __DEBUG
							TXString("DEBUG - Line Detection Enabled.\r\n");
							#endif
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
//					if(parameter_count == 1) {
//						if(current_parameters[0] == 0x31)
//						{
							ProcStatus.get_angle_enabled = 1;
//						}
//						else 
//						{
//							ProcStatus.get_angle_enabled = 0;
//						
//						}
//											
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
//					}
					break;
				case ZERO_ANGLE_OP:
					ProcStatus.zero_angle_enabled = 1;
					ProcStatus.ProcessInProgress = 0;
					parameter_count = 0;
					break;
				case MONITOR_ANGLE_OP:
					if(parameter_count == 1) 
					{
						if(current_parameters[0] == 0x30)
						{
							ProcStatus.monitor_angle_enabled = 0;
						}
						else 
						{
							ProcStatus.monitor_angle_enabled = 1;
							desiredAngle = current_parameters[0];
							monitorAngleCounter = 0; 
						}		
											
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}
					break;
				default:
					ProcStatus.ProcessInProgress = 0;
					break;
											
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
		
		if(ProcStatus.monitor_angle_enabled)
		{
			monitor_angle();
		}		

	}		
}	


void Refresh_EEPROM(void)
{
	
	line_threshold = ((int)Read_b_eep(EE_LINE_THRESHOLD_H) << 8) | (Read_b_eep(EE_LINE_THRESHOLD_L));
	line_follow_threshold = ((int)Read_b_eep(EE_LINE_FOLLOW_THRESHOLD_H) << 8) | (Read_b_eep(EE_LINE_FOLLOW_THRESHOLD_L));
	corner_threshold = ((int)Read_b_eep(EE_CORNER_THRESHOLD_H) << 8) | (Read_b_eep(EE_CORNER_THRESHOLD_L));
	
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
	
//	TXString("calibration_one: ");
//	TXDec_Int(antCalibration[0].lt);
//	TXString("\x0D\x0A");
//	TXString("calibration_two: ");
//	TXDec_Int(antCalibration[1].lt);
//	TXString("\x0D\x0A");
//	TXString("adjustment: ");
//	TXDec_Int(antenna_adjustment);
//	TXString("\x0D\x0A");	

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
	
//	TXString("reading_one: ");
//	TXDec_Int(antResults[0]);
//	TXString("\x0D\x0A");
//	TXString("reading_two: ");
//	TXDec_Int(antResults[1]);
//	TXString("\x0D\x0A");
//	TXString("threshold: ");
//	TXDec_Int(line_threshold);
//	TXString("\x0D\x0A");
	
		
	if(antResults[0] < (antCalibration[0].lt + line_threshold) && antResults[1]  < (antCalibration[1].lt + line_threshold))
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
//		differenceLineFollow = antResults[0] - (antResults[1] + antenna_adjustment);
		
		
		//  ******  line follow threshold adjusts the amount
		if(antResults[0] > (antCalibration[0].lt + line_threshold))
		{
			if(INT_LINE_LEFT != lineFollowCurrentState)
			{
				interrupt(INT_LINE_LEFT);
				lineFollowCurrentState = INT_LINE_LEFT;				
				ProcStatus.line_follow_enabled = 0;
			}
				
		}	
		
		if(antResults[1]  > (antCalibration[1].lt + line_threshold))
		{
			if(INT_LINE_RIGHT != lineFollowCurrentState)
			{
				interrupt(INT_LINE_RIGHT);
				lineFollowCurrentState = INT_LINE_RIGHT;
				ProcStatus.line_follow_enabled = 0;
			}
		}	
//		else 
//		{
//			if(INT_LINE_CENTER != lineFollowCurrentState)
//			{
//				interrupt(INT_LINE_CENTER);
//				lineFollowCurrentState = INT_LINE_CENTER;
//				ProcStatus.line_follow_enabled = 0;
//			}	
//		}	
	}
}

void corner_detection() {
	// This function simply checks the antenna values against the threshholds in the eeprom and 
	// interrupts if nessisary.  The interrupt return messages are on the wiki and should kept up to date
	// with any changes.
	
	antResults[2] = antMeasure[2].lt;
	
	if(antResults[2] > (antCalibration[2].lt + corner_threshold))
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
	
	if(antResults[0] > (antCalibration[0].lt + line_threshold)) 
	{
		interrupt(INT_LINE_DETECT_LEFT);
		ProcStatus.line_detection_enabled = 0;		
	}
	
	if(antResults[1] > (antCalibration[1].lt + line_threshold))
	{
		interrupt(INT_LINE_DETECT_RIGHT);
		ProcStatus.line_detection_enabled = 0;
	}
	
	if(antResults[2] > (antCalibration[2].lt + corner_threshold))
	{
		interrupt(INT_LINE_DETECT_FRONT);
		ProcStatus.line_detection_enabled = 0;
	}		
}
	
void get_angle()
{
	tempAccumulator = arsVariationAccumulator;
	angle = (int)((float)tempAccumulator/ars_magic);
	
	#ifdef __DEBUG
		TXString("Variation Accumulator: ");
		TXDec_Int((int)arsVariationAccumulator);
		TXString("\x0A\x0D");
	
		TXString("Angle: ");
		TXDec_Int((int)angle);
		TXString("\x0A\x0D");
	#endif
	
	
	// ***** Check magnitude of angle ****
	angleInteger = angle/360;
	
	//  *****  If the magnitude of angle is greater than 360 *********
	if(angleInteger >= 1 || angleInteger <= -1)
	{
		//  ***** if so, subtract to get a number less than 360 in magnitude *******
		angle_out = angle - (360 * angleInteger);
	}
	else
	{
		angle_out = angle;
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
	
	// disable get_angle()
	ProcStatus.get_angle_enabled = 0;		
}
void zero_angle()
{
	arsVariationAccumulator = 0;
}	
void monitor_angle()
{
	tempAccumulator = arsVariationAccumulator;
	angle = (int)((float)tempAccumulator/ars_magic);
	
	// ***** Check magnitude of angle ****
	angleInteger = angle/360;
	
	//  *****  If the magnitude of angle is greater than 360 *********
	if(angleInteger >= 1 || angleInteger <= -1)
	{
		//  ***** if so, subtract to get a number less than 360 in magnitude *******
		angle_out = angle - (360 * angleInteger);
	}
	else
	{
		angle_out = angle;
	}	
	

	// ***** if the counter is zero    *****
	if(monitorAngleCounter == 0)
	{
		// then store the original angle
		originalAngle = angle_out;
		
		// calculate the upper and lower monitor thresholds
		upperMonitorThreshold = originalAngle + desiredAngle;
				
		lowerMonitorThreshold = originalAngle - desiredAngle;
		
		//  ****  increment the counter	*****			
		monitorAngleCounter ++;
			
	}	
	else if(angle_out > upperMonitorThreshold)
		{
			TXChar(INT_EXCEED_LEFT);
			TXString("\r\n");
			ProcStatus.monitor_angle_enabled = 0;
		}	
		else if(angle_out < lowerMonitorThreshold)
			{
				TXChar(INT_EXCEED_RIGHT);
				TXString("\r\n");
				ProcStatus.monitor_angle_enabled = 0;
			}	
	

	// clear value of angle_out for next iteration...	
	angle_out = 0;
}

//		// make sure upper and lower monitor thresholds are within the 0 to 360 range 
//		if(upperMonitorThreshold < 0)
//		{
//			upperMonitorThreshold += 360;	
//		}
//		else if(upperMonitorThreshold > 360)
//		{
//			upperMonitorThreshold -= 360;	
//		}
//		
//		if(lowerMonitorThreshold < 0)
//		{
//			lowerMonitorThreshold += 360;	
//		}
//		else if(lowerMonitorThreshold > 360)
//		{
//			lowerMonitorThreshold -= 360;	
//		}		
								