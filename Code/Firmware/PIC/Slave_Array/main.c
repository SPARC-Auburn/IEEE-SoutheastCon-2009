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


unsigned int pulseDuration = 0;
unsigned int distance[] = {50,50,50};
int sonarIndex = 0;
int i;
unsigned int thresholdBack = 10;
unsigned int thresholdFrontBack = 20;
unsigned int thresholdFrontFront = 20;


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
			switch(current_proc) 
			{
				case RESET_OP:
					Reset();
					ProcStatus.ProcessInProgress = 0;
					break;
				case EEPROM_WR_OP:
					// Do EEPROM_WR stuff here
					ProcStatus.ProcessInProgress = 0;
					parameter_count = 0;
					break;
				case POLE_SONAR_OP:
					ProcStatus.sonar_poll_enabled = 1;
					ProcStatus.ProcessInProgress = 0;					
					break;
			}
		}
		// *** Handle execution loop. *** //
		// if poll sonar is enabled
		if(ProcStatus.sonar_poll_enabled) 
		{
			poll_sonar();
			ProcStatus.sonar_poll_enabled = 0;
		}
		
		
		
		
	}
}

void sonar_poll(void)
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
//		TXString("Sonar Reading4:");
//		TXDec_Int(distance[sonarIndex]);		
//		TXString("\x0D\x0A");

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
//		TXString("Sonar Reading3:");
//		TXDec_Int(distance[sonarIndex]);
//		TXString("\x0D\x0A");

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
//		TXString("Sonar Reading:");
//		TXDec_Int(distance[sonarIndex]);
//		TXString("\x0D\x0A");


		
		if(distance[0] > thresholdBack)
		{
			TXChar(0x41);		//send code for "No Object"
			TXString("\x0D\x0A");			
		}	 
		else
		{
			if(distance[1] < thresholdFrontBack && distance[2] < thresholdFrontFront)
			{
				TXChar(0x44);	//send code for "Plastic Bottle"
				TXString("\x0D\x0A");			
	
			}
			else
			{
				if(distance[1] < thresholdFrontBack && distance[2] > thresholdFrontFront)
				{
					TXChar(0x43);	//send code for "Glass Bottle"
					TXString("\x0D\x0A");	
				}
				else
				{
					if(distance[1] > thresholdFrontBack && distance[2] > thresholdFrontFront)
					{
						TXChar(0x42);	//send code for "Aluminum Can"
						TXString("\x0D\x0A");	
					}
					else
					{
						TXChar(0x45);	//send code for "Error - FrontFront Sonar is triggered, but FrontBack is not"
						TXString("\x0D\x0A");	
					}	
				}			
			}
								
		}
	
}	