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






//*****************************  INITIALIZE VARIABLES BEGIN *************************

unsigned int result, thingOne, thingTwo;

unsigned int resultArray[3] = {0,0,0};

unsigned int calibrationArray[3] = {0,0,0};

int differenceLineFollow, differenceCorner;


								//  SET THRESHOLDS FOR THE LINE FOLLOWING AND CORNER DETECTION ALGORITHMS
const unsigned int thresholdLineFollow = 50;

const unsigned int thresholdCorner = 30;

const unsigned int thresholdLine = 20;


							
unsigned int i = 0;				//  SET INDEX VALUE TO CONTROL NUMBER OF TIMES CALIBRATION CYCLE SHOULD RUN

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

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	unsigned char c;
	Init();
	TXString("\x0D\x0A");		//Put out a new line
	TXChar('>');	





//************************************  SETUP A/D CONVERTER BEGIN  ***********************************
	
	TRISAbits.TRISA0 = 1;		//set pins 2,3, and 4 as inputs
	TRISAbits.TRISA1 = 1;
	TRISAbits.TRISA2 = 1;		
	
	
	ADCON0 = 0x01;				//turn on the A/D converter, and configure pin 2 as the analog input for the A/D converter								
								
	ADCON1 = 0x0D;				//configure reference voltages to Vdd and Vss, and configure pins 2 and 3 as 
								//analog pins and configure pins 4 and 5 as digital pins
	
	ADCON2 = 0xA9;				//configure A/D converter result registers as right justified, acquisition time set to 12 times
								//the AD timer, AD timer set to 1/8th the oscillator frequency
								
//*************************************  SETUP A/D CONVERTER END  **********************************								


while(i < 3)
{
	ADCON0 = 0x01;		//configure pin 2 as the analog input for the A/D converter
			
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
		
		resultArray[0] = result;
					

		Delay10TCYx(1);		//delay before starting A/D converter again
		
		
		
							//configure pin 3 as the analog input for the A/D converter
		ADCON0bits.CHS0 = 1;
		
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
		
		resultArray[1] = result;
							
		Delay10TCYx(1);		//delay before starting A/D converter again
		
		
		
							//configure pin 4 as the analog input for the A/D converter
		ADCON0bits.CHS0 = 0;
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
					
		resultArray[2] = result;
	
	
		i++;
		
		Delay10KTCYx(990);		//delay before starting A/D converter again			
}	


calibrationArray[0] = resultArray[0];
calibrationArray[1] = resultArray[1];
calibrationArray[2] = resultArray[2];
		
		
//*********************  MAIN LOOP BEGIN (a.k.a. line following/corner detection)  **************************************		
	while(1)
		{	
			ADCON0 = 0x01;		//configure pin 2 as the analog input for the A/D converter
			
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
			
			resultArray[0] = result;
						

			Delay10TCYx(1);		//delay before starting A/D converter again
			
			
			
								//configure pin 3 as the analog input for the A/D converter
			ADCON0bits.CHS0 = 1;
			
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
			
			resultArray[1] = result;
								
			Delay10TCYx(1);		//delay before starting A/D converter again
			
			
			
								//configure pin 4 as the analog input for the A/D converter
			ADCON0bits.CHS0 = 0;
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
						
			resultArray[2] = result;						
						

		
		
		
					
//******************************  LINE FOLLOWING ALGORITHM BEGIN **********************************
		
			
			if(resultArray[0] < (calibrationArray[0]+thresholdLine) && resultArray[1] < (calibrationArray[1]+thresholdLine))
			{
				TXString("********  NO WIRE DETECTED  ***********");
				TXString("\x0D\x0A");
			}	
			else
			{
										//get ready to determine line following instructions by calculating the difference 
										//between the power received on the left and right antennas
				differenceLineFollow = (int)resultArray[0] - (int)resultArray[1];
				
				if(differenceLineFollow > (int)thresholdLineFollow)
				{
					TXString("*********   GO LEFT   *********");
					TXString("\x0D\x0A");	
				}	
				else
				{	
					if(differenceLineFollow < (int)(thresholdLineFollow*-1))
					{
						TXString("********	  GO RIGHT  *********");
						TXString("\x0D\x0A");
					}
					else
					{
						TXString("**********   DEAD ON   ************");
						TXString("\x0D\x0A");
					}		
				}
			
			}
//******************************  LINE FOLLOWING ALGORITHM END **********************************




				
			
//******************************  CORNER DETECTION ALGORITHM BEGIN **********************************
										
										//get ready to determine corner detection by calculating the difference between the power recieved
										//by the corner antenna at startup and current readings
//			differenceCorner = (int)resultArray[2] - (int)calibrationArray[2];	
								
			
			thingOne = calibrationArray[2] + thresholdCorner;
			thingTwo = calibrationArray[2] - 5;
								
										//perform if statements based on "differenceCorner" value to provide appropriate instuctions
			if(resultArray[2] > thingOne)
			{
				TXString("***********  Reached Corner **************");
				TXString("\x0D\x0A");
			}
			else
			{
				if(resultArray[2] < thingTwo)
				{
					TXString("***********   ERROR   *************");	
					TXString("\x0D\x0A");
				}
//				else
//				{
//					TXString("***********   No Corner   *************");	
//					TXString("\x0D\x0A");	
//				}			
			}
//******************************  CORNER DETECTION ALGORITHM END **********************************
										
			
			
			
			
			
			
//***********************************  DEBUGGING SERIAL OUTPUT BEGIN *********************************			
			TXDec_Int(resultArray[0]);
			TXString("\x0D\x0A");
			TXDec_Int(resultArray[1]);
			TXString("\x0D\x0A");
			TXString("\x0D\x0A");
			TXDec_Int(calibrationArray[0]);
			TXString("\x0D\x0A");
			TXDec_Int(calibrationArray[1]);
			TXString("\x0D\x0A");
			TXString("\x0D\x0A");
			TXDec_Int(differenceLineFollow);
			
//***********************************  DEBUGGING SERIAL OUTPUT END *********************************
								
								
								
								
								
								
			Delay10KTCYx(99);		//delay before starting A/D converter again
			
			
			
		}
		
//***********************  END MAIN LOOP (a.k.a. line following)  ************************************		
}
