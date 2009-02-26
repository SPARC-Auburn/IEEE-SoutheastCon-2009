//************************************************************************************************
//	Node:		Object detection code
/*
	Wiring Guide for PIC18F2685:

	pin 2 - (i/o) signal line for Back Parallax Sonar Module 
	pin 3 - (i/o) signal line for Front-Back Parallax Sonar Module
	pin 4 - (i/o) signal line for Front-Front Parallax Sonar Module
	
	pin 6 - (input) microswitch (active high)
*/
//************************************************************************************************

#include "main.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"
#include "op_codes.h"
#include "eealloc.h"

unsigned int pulseDuration = 0;
unsigned int distance[] = {50,50,50};
int sonarIndex = 0;
int i;

unsigned int switchCount = 0;

// Variables that are stored in the EEPROM
unsigned int switch_threshold = 20;
unsigned int sonar_divider = 58;
unsigned int thresholdFrontFront = 812;
unsigned int thresholdFrontBack = 812;
unsigned int thresholdBack = 10;



volatile struct proc_status ProcStatus = {0,0};
unsigned char current_proc = 0;
unsigned char parameter_count = 0;
unsigned char current_parameters[32];

int EEP_count = -1;
union int_byte EEP_address;
unsigned char EEP_offset = 0;


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
	
//***************************************************************************************************************
//                          setup
//***************************************************************************************************************
	EEP_address.lt = 0;
	Refresh_EEPROM();
	Init();	
	initQueue();
	
	TRISAbits.TRISA4 = 1;		// set pin 6 as input for microswitch
	
	ADCON1 = 0X0F;				// make sure all pins function as digital
		
	OpenTimer0( TIMER_INT_OFF & //initialize timer0 for: - interupt disabled
            T0_16BIT &           //					 	 - 16 bit timer
            T0_SOURCE_INT &		//						 - based on internal oscillator
            T0_PS_1_2 );		//						 - 2X prescaler to give sonar measurement 1 microsecond accuracy

	
	Delay10KTCYx(99);
	
	if(isWDTTO())
	{
		TXString("RST OBJ - WDT\x0A\x0D");
	}
	else if(isMCLR())
	{
		TXString("RST OBJ - MCLR\x0A\x0D");	
	}
	else if(isPOR())	
	{
		TXString("RST OBJ - POR\x0A\x0D");
	}
	else if(isBOR())
	{
		TXString("RST OBJ - BOR\x0A\x0D");
	}
	else
	{
		TXString("RST OBJ\x0A\x0D");	
	}	
	StatusReset();
	
//***************************************************************************************************************
//                          loop
//***************************************************************************************************************
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
						
						TXString("0 ");
						TXChar(Read_b_eep(EEP_address.lt));
						ProcStatus.ProcessInProgress = 0;
						parameter_count = 0;
					}	
					break;
				case POLL_SONAR_OP:
					ProcStatus.sonar_poll_enabled = 1;
					ProcStatus.ProcessInProgress = 0;					
					break;
				default:
					ProcStatus.ProcessInProgress = 0;
					break;
			}
		}
		
		// *** Handle execution loop. *** //
		
		// If calibration needs to be run
		if(ProcStatus.sonar_poll_enabled) 
		{
			poll_sonar();
			ProcStatus.sonar_poll_enabled = 0;
		}
		
		//If microswitch is engaged then send respective value
		if(PORTAbits.RA4)
		{
			if(switchCount == switch_threshold)
			{
				TXChar(0x70);
				TXString("\x0A\x0D");
				switchCount++;
			} else if (switchCount < switch_threshold)
			{
				switchCount++;
			}	
			
		}
		else
		{
			switchCount = 0;	
		}			
	}
}


void Refresh_EEPROM(void)
{
	switch_threshold = ((int)Read_b_eep(EE_SWITCH_THRESHOLD_H) << 8) | (Read_b_eep(EE_SWITCH_THRESHOLD_L));
	sonar_divider = ((int)Read_b_eep(EE_SONAR_DIVIDER_H) << 8) | (Read_b_eep(EE_SONAR_DIVIDER_L));
	thresholdFrontFront = ((int)Read_b_eep(EE_FF_THRESHOLD_H) << 8) | (Read_b_eep(EE_FF_THRESHOLD_L));
	thresholdFrontBack = ((int)Read_b_eep(EE_FB_THRESHOLD_H) << 8) | (Read_b_eep(EE_FB_THRESHOLD_L));
	thresholdBack = ((int)Read_b_eep(EE_BACK_THRESHOLD_H) << 8) | (Read_b_eep(EE_BACK_THRESHOLD_L));
}	

void poll_sonar(void)
{
								//start the 1st sonar measurement
		sonarIndex = 0;		

								//start 2nd sonar measurement
		sonarIndex++;			
		
		TRISAbits.TRISA1 = 0; 	//set pin 3 to output for Parallax triggering sequence

		PORTAbits.RA1 = 0;		//bring pin 3 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTAbits.RA1 = 1;  	//bring pin 3 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTAbits.RA1 = 0;		//bring pin 3 low

		TRISAbits.TRISA1 = 1; 	//set pin 3 to input for pulse readin
		
		while(PORTAbits.RA1 == 0)
		{	
								//wait for the Parallax to bring pin 3 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTAbits.RA1 == 1)
		{
								//wait for the Parallax to bring pin 3 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration;						

								//start the 3rd sonar measurement
		sonarIndex++;

		TRISAbits.TRISA2 = 0; 	//set pin 4 to output for Parallax triggering sequence

		PORTAbits.RA2 = 0;		//bring pin 4 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTAbits.RA2 = 1;  	//bring pin 4 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTAbits.RA2 = 0;		//bring pin 4 low

		TRISAbits.TRISA2 = 1; 	//set pin 4 to input for pulse readin
		
		while(PORTAbits.RA2 == 0)
		{	
								//wait for the Parallax to bring pin 4 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTAbits.RA2 == 1)
		{
								//wait for the Parallax to bring pin 4 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration;						
		

		if(distance[1] < thresholdFrontBack && distance[2] < thresholdFrontFront)
		{
			TXChar(SONAR_PLASTIC);	//send code for "Plastic Bottle"
			TXChar(' ');
			TXDec_Int(distance[1]);
			TXChar(' ');
			TXDec_Int(distance[2]);
			TXString("\x0D\x0A");			

		}
		else
		{
			if(distance[1] < thresholdFrontBack && distance[2] > thresholdFrontFront)
			{
				TXChar(SONAR_GLASS);	//send code for "Glass Bottle"
				TXChar(' ');
				TXDec_Int(distance[1]);
				TXChar(' ');
				TXDec_Int(distance[2]);
				TXString("\x0D\x0A");	
			}
			else
			{
				if(distance[1] > thresholdFrontBack && distance[2] > thresholdFrontFront)
				{
					TXChar(SONAR_ALUMINUM);	//send code for "Aluminum Can"
					TXChar(' ');
					TXDec_Int(distance[1]);
					TXChar(' ');
					TXDec_Int(distance[2]);
					TXString("\x0D\x0A");	
				}
				else
				{
					TXChar(SONAR_ERROR);	//send code for "Error - FrontFront Sonar is triggered, but FrontBack is not"
					TXChar(' ');
					TXDec_Int(distance[1]);
					TXChar(' ');
					TXDec_Int(distance[2]);
					TXString("\x0D\x0A");	
				}	
			}			
		}
								
	
}	