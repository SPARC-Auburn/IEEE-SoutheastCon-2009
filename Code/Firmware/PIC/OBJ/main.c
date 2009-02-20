//************************************************************************************************
//	Node:		MAGNETIC FEILD DETECTION
/*
	Wiring Guide for PIC18F2685:

	pin 2 - (i/o) signal line for Back Parallax Sonar Module 
	pin 3 - (i/o) signal line for Front-Back Parallax Sonar Module
	pin 4 - (i/o) signal line for Front-Front Parallax Sonar Module
	
	pin 6 - (input) microswitch (active high)
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
#include "reset.h"
#include <delays.h>
#include <math.h>

unsigned int pulseDuration = 0;
unsigned int distance[] = {50,50,50};
int sonarIndex = 0;
int i;
unsigned int thresholdBack = 10;
unsigned int thresholdFrontBack = 20;
unsigned int thresholdFrontFront = 20;
unsigned int switchCount = 0;

volatile struct proc_status ProcStatus = {0,0};
unsigned char current_proc = 0;
unsigned char paramter_count = 0;


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
				TXChar(c);
				TXString("\x0A\x0D");
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
			if(switchCount == SWITCH_THRESHOLD)
			{
				TXChar(0x70);
				TXString("\x0A\x0D");
				switchCount++;
			} else if (switchCount < SWITCH_THRESHOLD)
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



void poll_sonar(void)
{
								//start the 1st sonar measurement
		sonarIndex = 0;		

		TRISAbits.TRISA0 = 0; 	//set pin 2 to output for Parallax triggering sequence

		PORTAbits.RA0 = 0;		//bring pin 2 low
		Delay10TCYx(7);			//delay for ~2 microseconds

		PORTAbits.RA0 = 1;  	//bring pin 2 high
		Delay10TCYx(16);		//delay for ~5 microseconds

		PORTAbits.RA0 = 0;		//bring pin 2 low

		TRISAbits.TRISA0 = 1; 	//set pin 2 to input for pulse readin
		
		while(PORTAbits.RA0 == 0)
		{	
								//wait for the Parallax to bring pin 2 high				
		}
		
		WriteTimer0( 0 );		//reset Timer0

		while(PORTAbits.RA0 == 1)
		{
								//wait for the Parallax to bring pin 2 low
		}
								//read the value in Timer0
		pulseDuration = ReadTimer0();

								//divide the microseconds by 58 to convert to centimeters
		distance[sonarIndex] = pulseDuration / 58;						
		
								//output the duration of pulse received from the Parallax in centimeters
		TXString("Back Sonar Reading:");
		TXDec_Int(distance[sonarIndex]);		
		TXString("\x0D\x0A");

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
		distance[sonarIndex] = pulseDuration / 58;						

		
								//output the duration of pulse received from the Parallax in centimeters
		TXString("Front-Back Sonar Reading:");
		TXDec_Int(distance[sonarIndex]);
		TXString("\x0D\x0A");

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
		distance[sonarIndex] = pulseDuration / 58;						
		

								//output the duration of pulse received from the Parallax in centimeters
		TXString("Front-Front Sonar Reading:");
		TXDec_Int(distance[sonarIndex]);
		TXString("\x0D\x0A");


		
		if(distance[0] > thresholdBack)
		{
			TXChar(SONAR_NO_OBJECT);		//send code for "No Object"
			TXString("\x0D\x0A");			
		}	 
		else
		{
			if(distance[1] < thresholdFrontBack && distance[2] < thresholdFrontFront)
			{
				TXChar(SONAR_PLASTIC);	//send code for "Plastic Bottle"
				TXString("\x0D\x0A");			
	
			}
			else
			{
				if(distance[1] < thresholdFrontBack && distance[2] > thresholdFrontFront)
				{
					TXChar(SONAR_GLASS);	//send code for "Glass Bottle"
					TXString("\x0D\x0A");	
				}
				else
				{
					if(distance[1] > thresholdFrontBack && distance[2] > thresholdFrontFront)
					{
						TXChar(SONAR_ALUMINUM);	//send code for "Aluminum Can"
						TXString("\x0D\x0A");	
					}
					else
					{
						TXChar(SONAR_ERROR);	//send code for "Error - FrontFront Sonar is triggered, but FrontBack is not"
						TXString("\x0D\x0A");	
					}	
				}			
			}
								
		}
	
}	