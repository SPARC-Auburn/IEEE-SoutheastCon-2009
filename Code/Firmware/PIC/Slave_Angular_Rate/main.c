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
#include <delays.h>
#include <math.h>
#include <timers.h>
#include <stdlib.h>





//*****************************  INITIALIZE VARIABLES BEGIN *************************

long calibrationAccumulator, readingAccumulator;

int calibrationAverage, readingAverage, differenceAD, i, result, angle;

float deltaAngle;



//****************************  INITIALIZE VARIABLES END ***********************







#pragma config OSC = IRCIO67,WDT = OFF, MCLRE = OFF

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
volatile unsigned char pointer;

//***************************************************************************************************************
//							high_isr
//***************************************************************************************************************

#pragma interrupt high_isr
void high_isr(void)
{
	if(PIR1bits.SSPIF){		// If - SSP Module (I2C)
		unsigned char c;
		if(SSPSTATbits.R_W){	// If - Read from slave
			if(!isQueueEmpty()){// Check if QUEUE is EMPTY
				c = popQueue();	// Grab a char from the QUEUE
				SSPBUF = c;		// Write the byte to the I2C bus
			}
		} 
		else {				// Else - Write to Slave
			if(SSPSTATbits.D_A){	// If - Data
				if(SSPSTATbits.BF){		//If - Buffer Full
					c = SSPBUF;	// Grab a char from the I2C Buffer
					if(!isQueueFull()){	// Check if QUEUE is FULL
						pushQueue(c);	// Write the char to the QUEUE
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






//*****************************************  DEFINE FUNCTIONS *****************************************************
void getAngle(void);







//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;
	Init();
	TXString("\x0D\x0A");		//Put out a new line
	TXChar('>');	


OpenTimer0( TIMER_INT_OFF & 	//initialize timer0 for: - interupt disabled
            T0_16BIT &           //					 	 - 8 bit timer
            T0_SOURCE_INT &		//						 - based on internal oscillator
            T0_PS_1_2 );		//						 - 8X prescaler to give sonar measurement 1 microsecond accuracy
            
OpenTimer1( TIMER_INT_OFF & 	//initialize timer0 for: - interupt disabled
            T0_16BIT &           //					 	 - 8 bit timer
            T0_SOURCE_INT &		//						 - based on internal oscillator
            T0_PS_1_2 );		//						 - 8X prescaler to give sonar measurement 1 microsecond accuracy
            



//************************************  SETUP A/D CONVERTER BEGIN  ***********************************
	
	TRISAbits.TRISA0 = 1;		//set pins 2,3,4, AND 5 as inputs
	TRISAbits.TRISA1 = 1;
	TRISAbits.TRISA2 = 1;
	TRISAbits.TRISA3 = 1;		
	
	
	ADCON0 = 0x01;				//turn on the A/D converter, and configure pin 2 as the analog input for the A/D converter								
								
	ADCON1 = 0x0D;				//configure reference voltages to Vdd and Vss, and configure pins 2 and 3 as 
								//analog pins and configure pins 4 and 5 as digital pins
	
	ADCON2 = 0xA9;				//configure A/D converter result registers as right justified, acquisition time set to 12 times
								//the AD timer, AD timer set to 1/8th the oscillator frequency
								
//*************************************  SETUP A/D CONVERTER END  **********************************								




Delay10KTCYx(9);

calibrationAccumulator = 0;

i = 0;

//******************************************  Calibration Cycle Begin	*********************************************	

while(i < 64)
{	
//**********************************************  Angular Rate Sensor	*********************************************	
			
		
							//configure pin 5 as the analog input for the A/D converter
		ADCON0bits.CHS0 = 1;
		ADCON0bits.CHS1 = 1;
		
							//start the A/D converter 
		ADCON0bits.GO = 1;
							
							//wait for the A/D converter to finish
		while(ADCON0bits.GO)
		{
		}
							
		result = (int)ADRESL;		//type cast and store low A/D register results in unsigned integer variable
		
		if(ADRESH == 0x01)			//perform if statements to handle high A/D register results
		{
			result += 256;
		}
		if(ADRESH == 0x02)
		{
			result += 512;	
		}
		if(ADRESH == 0x03)
		{
			result += 768;	
		}			
					
		calibrationAccumulator += result;
		
		Delay1TCY();				//MINIMUM delay before starting A/D converter again
		Delay1TCY();
		
		i++;
			
}

calibrationAverage = calibrationAccumulator/100;

TXDec_Int(calibrationAverage);

//**********************************************  Calibration Cycle End	*********************************************	





angle = 0;        						//intialize angle to zero
deltaAngle;
WriteTimer1(0);

//**********************************************  Main Loop Begin   *************************************************
while(1)
	{
		i = 0;
		readingAccumulator =0;
		WriteTimer0(0);
		
		while(ReadTimer0() < 1000)
		{
			ADCON0bits.GO = 1;			//start the A/D converter 
								
										
			while(ADCON0bits.GO)		//wait for the A/D converter to finish
			{
			}
								
			result = (int)ADRESL;		//type cast and store low A/D register results in unsigned integer variable
			
			if(ADRESH == 0x01)			//perform if statements to handle high A/D register results
			{
				result += 256;
			}
			if(ADRESH == 0x02)
			{
				result += 512;	
			}
			if(ADRESH == 0x03)
			{
				result += 768;	
			}
			
			readingAccumulator += result;
			
			i++;			
		}	
		
		//WriteTimer0(0);
										//get average reading over the sample period of 1 millisecond
		readingAverage = (readingAccumulator / i);
						
										//compute the variation from the calibration value
		differenceAD = readingAverage - calibrationAverage;
					
										
		deltaAngle += (float)(differenceAD/1366);//divide the variation by 1366 to get the angle change during
										//the sampled millisecond:  sensor has a "6.67mV/degree/second" 
										//change in its output and the PIC's A-to-D has a "4.883mV/LSB"
										//sensitivity which yeilds the 1366 LSB*ms number to yeild the 
										//angle
		if(ReadTimer1() > 5000)
		{
			
			angle += deltaAngle;			//add the change in the angle to the running value of the angle
				
			if(angle < 0)
			{
				angle += 360;
			}
			else
			{
				if(angle > 360)
				{
					angle = (angle - 360);
				}
			}	
		
		}
												
										
		//result = ReadTimer0();
		TXDec_Int(result);
		TXString("\x0D\x0A");
		//Delay10KTCYx(999);		
		
		
	}	




		
		
		
		
////		i = 0;
////		readingAccumulator = 0;
////	
////	
////		WriteTimer0(0);                 	//set timer zero value to zero
////		
////		while(i<50)
////		{		
////											//configure pin 5 as the analog input for the A/D converter
////				ADCON0bits.CHS0 = 1;
////				
////											//start the A/D converter 
////				ADCON0bits.GO = 1;
////									
////											//wait for the A/D converter to finish
////				while(ADCON0bits.GO)
////				{
////				}
////									
////				result = (int)ADRESL;		//type cast and store low A/D register results in unsigned integer variable
////				
////				if(ADRESH == 0x01)			//perform if statements to handle high A/D register results
////				{
////					result += 256;
////				}
////				if(ADRESH == 0x02)
////				{
////					result += 512;	
////				}
////				if(ADRESH == 0x03)
////				{
////					result += 768;	
////				}			
////				
////				readingAccumulator += (result - calibrationAverage);	
////							
////				Delay1TCY();				//MINIMUM delay before starting A/D converter again
////				Delay1TCY();
////											
////				i++;
////		}
////											//read in value from timer zero	and convert from milliseconds to seconds
////		timeElapsed = (float)(ReadTimer0()/1000);			
////		
////											//get average from sample period
////		readingAverage = readingAccumulator/50;			
////		
////											//determine the variation from the calibration value
////		differenceAD = readingAverage - calibrationArray[3];
////		
////		if(differenceAD < -2 || differenceAD > 2)
////		{
////												//Assumption: a variation of 1 in the ADC result is equal to 1 degree/second change in position.
////			deltaAngle = (int)((differenceAD * timeElapsed)/8);
////				
////			angle += deltaAngle;				//Add the change in angle to the running count of the angle
////				
////			if(angle < 0)
////			{
////				angle += 360;
////			}
////			else
////			{
////				if(angle > 360)
////				{
////					angle = (angle - 360);
////				}
////			}		
////		}
////		
		
}