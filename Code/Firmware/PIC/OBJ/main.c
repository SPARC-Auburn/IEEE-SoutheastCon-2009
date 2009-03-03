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
unsigned int distance[] = {0,0};
int sonarIndex = 0;
int i;

unsigned long switchCount = 0;
unsigned long startButtonCount = 0;
unsigned int startButtonIndicator = 0;

// Variables that are stored in the EEPROM
unsigned int ch_switch_threshold;
unsigned int on_switch_threshold;
unsigned int threshold_Sonar_Object;
unsigned int threshold_IR_Object;
unsigned int threshold_Sonar_Plastic_Low;
unsigned int threshold_Sonar_Plastic_High;
unsigned int threshold_IR_Plastic_Low;
unsigned int threshold_IR_Plastic_High;
unsigned int threshold_Sonar_Glass_Low;
unsigned int threshold_Sonar_Glass_High;
unsigned int threshold_IR_Glass_Low;
unsigned int threshold_IR_Glass_High;
unsigned int object_checks;

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
	TRISAbits.TRISA5 = 1;		// set pin 7 as input for start button
	TRISAbits.TRISA7 = 0;       // set pin 9 as output for the start button LED
	
	PORTAbits.RA7 = 1;    	//turn off LED for Pushbutton
		
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
				case POLL_SONAR_OP:
					ProcStatus.sonar_poll_enabled = 1;
					ProcStatus.ProcessInProgress = 0;					
					break;
				case START_BUTTON_OP:
					ProcStatus.start_button_enabled = 1;
					ProcStatus.ProcessInProgress = 0;
					break;					
				default:
					ProcStatus.ProcessInProgress = 0;
					break;
			}
		}
		
		// *** Handle execution loop. *** //

		if(ProcStatus.sonar_poll_enabled) 
		{
			unsigned char cycles = 0;
			unsigned char temp_readings;
			
			while(cycles < object_checks)
			{
				temp_readings += poll_sonar();
			}	
			temp_readings = temp_readings / object_checks;
			
			TXChar(temp_readings);
			TXChar(' ');
			TXDec_Int(distance[0]);
			TXChar(' ');
			TXDec_Int(distance[1]);
			TXString("\x0A\x0D");
			
			ProcStatus.sonar_poll_enabled = 0;
		}
		
		
		if(ProcStatus.start_button_enabled)
		{
			PORTAbits.RA7 = 0;					//turn on LED for Pushbutton
			
			if(PORTAbits.RA5)				//if the start button is engaged
			{
				if(startButtonCount == on_switch_threshold) 	//if the start button counter equals threshold
				{	
					startButtonCount++;
									
					TXChar(0x76);
					TXString("\x0A\x0D");
					
					//  ***  Toggle Start Button LED ***
					
					if(startButtonIndicator == 0)
					{
						PORTAbits.RA7 = 1;    	//turn off LED for Pushbutton
						startButtonIndicator = 1;
					}
					else if(startButtonIndicator == 1)
					{
						PORTAbits.RA7 = 0;    	//turn on LED for Pushbutton
						startButtonIndicator = 0;	
					}	
				}
				else if(startButtonCount < on_switch_threshold)
				{
					startButtonCount++;
				}	
			}
			else
			{
				startButtonCount = 0;	
				
			}	
				
		}	
			
		
		//If microswitch is engaged then send respective value
		if(PORTAbits.RA4)
		{
			if(switchCount == ch_switch_threshold)
			{
				TXChar(0x70);
				TXString("\x0A\x0D");
				switchCount++;
			} 
			else if(switchCount < ch_switch_threshold)
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
	ch_switch_threshold = 						((int)Read_b_eep(CH_SWITCH_THRESHOLD_H) << 8) 	| (Read_b_eep(CH_SWITCH_THRESHOLD_L));
	on_switch_threshold = 						((int)Read_b_eep(ON_SWITCH_THRESHOLD_H) << 8) 	| (Read_b_eep(ON_SWITCH_THRESHOLD_L));
	threshold_Sonar_Object = 					((int)Read_b_eep(SONAR_OBJECT_H) << 8) 			| (Read_b_eep(SONAR_OBJECT_L));
	threshold_IR_Object = 						((int)Read_b_eep(IR_OBJECT_H) << 8) 			| (Read_b_eep(IR_OBJECT_L));
	threshold_Sonar_Plastic_Low = 				((int)Read_b_eep(SONAR_PLASTIC_LOW_H) << 8) 	| (Read_b_eep(SONAR_PLASTIC_LOW_L));
	threshold_Sonar_Plastic_High = 				((int)Read_b_eep(SONAR_PLASTIC_HIGH_H) << 8) 	| (Read_b_eep(SONAR_PLASTIC_HIGH_L));
	threshold_IR_Plastic_Low = 					((int)Read_b_eep(IR_PLASTIC_LOW_H) << 8) 		| (Read_b_eep(IR_PLASTIC_LOW_L));
	threshold_IR_Plastic_High = 				((int)Read_b_eep(IR_PLASTIC_HIGH_H) << 8) 		| (Read_b_eep(IR_PLASTIC_HIGH_L));
	threshold_Sonar_Glass_Low = 				((int)Read_b_eep(SONAR_GLASS_LOW_H) << 8) 		| (Read_b_eep(SONAR_GLASS_LOW_L));
	threshold_Sonar_Glass_High = 				((int)Read_b_eep(SONAR_GLASS_HIGH_H) << 8) 		| (Read_b_eep(SONAR_GLASS_HIGH_L));
	threshold_IR_Glass_Low = 					((int)Read_b_eep(IR_GLASS_LOW_H) << 8) 			| (Read_b_eep(IR_GLASS_LOW_L));
	threshold_IR_Glass_High	=					((int)Read_b_eep(IR_GLASS_HIGH_H) << 8) 		| (Read_b_eep(IR_GLASS_HIGH_L));
	object_checks = 							((int)Read_b_eep(OBJECT_CHECKS_H) << 8) 		| (Read_b_eep(OBJECT_CHECKS_L));
}	

unsigned char poll_sonar(void)
{					
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
		
		pulseDuration = ReadTimer0();
		distance[0] = pulseDuration/4;	

		/* ===== IR Reading ===== */
		
		ConvertADC();         // Start conversion
  		while( BusyADC() );   // Wait for completion
  		distance[1] = ReadADC();   // Read result

		if(distance[0] < threshold_Sonar_Object && distance[1] < threshold_IR_Object)	// If it's under the threshold, then it's either plastic or glass
		{
			if(distance[0] > threshold_Sonar_Plastic_Low && distance[0] < threshold_Sonar_Plastic_High) // If sonar is within range for plastic
			{
				if(distance[1] > threshold_IR_Plastic_Low && distance[1] < threshold_IR_Plastic_Low)	// If IR is within range for plastic
				{
					return SONAR_PLASTIC;
				}	
			}
			
			if(distance[0] > threshold_Sonar_Glass_Low && distance[0] < threshold_Sonar_Glass_High)
			{
				if(distance[1] > threshold_IR_Glass_Low && distance[1] < threshold_IR_Glass_High)
				{
					return SONAR_GLASS;	
				}		
			}
	
			return SONAR_ERROR;			
		}
		
		return SONAR_ALUMINUM;							
}	