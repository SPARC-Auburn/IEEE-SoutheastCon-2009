//************************************************************************************************
//	Node:		Encoder
//************************************************************************************************

#include "main.h"
#include "i2c.h"
#include "init.h"
#include "serial.h"
#include "hardware.h"
#include "queue.h"

#define XDATA(arg) (arg + 0)
#define XCMD(arg) (arg + 1)
#define YDATA(arg) (arg + 2)
#define YCMD(arg) (arg + 3)

//RLD (Reset and Load Signal Decoders) Register Commands
#define RLD(arg) (arg | 0x80)
#define XRLD(arg) (arg | 0)
#define YRLD(arg) XRLD(arg)
#define Rst_BP 0x01     //Reset Byte Pointer (BP)
#define Rst_CNTR 0x02   //Reset Counter
#define Rst_FLAGS 0x04  //Reset Flags
#define Rst_E 0x06      //Reset E-flags
#define Trf_PR_CNTR 0x08  //Transfer Preset to Counter
#define Trf_CNTR_OL 0x10  //Transfer Counter to Output Latch
#define Trf_PS0_PSC 0x18  //Transfer Preset Byte 0 to PSC (Filter Clock Prescaler)

//CMR (Counter Mode) Register Commands
#define CMR(arg) (arg | 0xA0)
#define XCMR(arg) (arg | 0x20)
#define YCMR(arg) XCMR(arg)
#define BINCnt 0x00  //Binary Count
#define BCDCnt 0x01  //BCD Count
#define NrmCnt 0x00  //Normal Count
#define RngLmt 0x02  //Range Limit
#define Nrcyc 0x04   //Non-recycle
#define ModN 0x06    //Modulo-N
#define NQDX 0x00    //Nonquadratue
#define QDX1 0x08    //Quadrature clock x1
#define QDX2 0x10    //Quadrature clock x2
#define QDX4 0x18    //Quadrature clock x4

//IOR (Input/Output Control) Register Commands
#define IOR(arg) (arg | 0xC0)
#define XIOR(arg) (arg | 0x40)
#define YIOR(arg) XIOR(arg)
#define DisAB 0x00   //Disable Inputs A and B
#define EnAB 0x01    //Enable Inputs A and B
#define LCNTR 0x00   //Set LCNTR/LOL pin Load Counter input
#define LOL 0x02     //Set LCNTR/LOL pin Load Output Latch input
#define RCNTR 0x00   //Set RCNTR/ABG pin Reset Counter input
#define ABGate 0x04  //Set RCNTR/ABG pin is A and B Enable Gate 
#define CYBW 0x00    //Set FLG1 as carry, FLG2 as borrow
#define CPBW 0x08    //Set FLG1 as compare, FLG2 as borrow
#define CB_UPDN 0x10 //Set FLG1 as carry/borrow, FLG2 as U/D
#define IDX_ERR 0x18 //Set FLG1 as IDX, FLG2 as E flag

//IDR (Index Control) Register Commands
#define IDR(arg) (arg | 0xE0)
#define XIDR(arg) (arg | 0x60)
#define YIDR(arg) XIDR(arg)
#define DisIDX 0x00  //Disable Index
#define EnIDX 0x01   //Enable Index
#define NIDX 0x00    //Negative Index Polarity
#define PIDX 0x02    //Positive Index Polarity
#define LIDX 0x00    //LCNTR/LOL pin is indexed
#define RIDX 0x04    //RCNTR/ABG pin is indexed

#define Addr 0x00 	 
static int Count[6];

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
//					outp function
//***************************************************************************************************************

void outp(int addr, int cmd)
{
  PORTA = addr | 0x1C;				//A2 = RD = 1,  A3 = WR = 1, A4 = CS = 1
  PORTB = cmd;						//Apply Command Data to the Data Bus
  PORTAbits.RA3 = 0;				//Pull WR (Write Enable) low
  PORTAbits.RA4 = 0;				//Pull CS (Chip Select) low
  PORTAbits.RA3 = 1;				//Restore WR high
  PORTAbits.RA4 = 1;				//Restore CS high
}

//***************************************************************************************************************
//					inp function
//***************************************************************************************************************

char inp(int addr)
{
	char byte;
	DDRB = 0x00;					//Set PORTB to intput
	PORTA = addr | 0x1C;			//A2 = RD = 1, A3 = WR =1, A4 = CS = 1
	PORTAbits.RA2 = 0;				//Pull RD (Read Data) low
	PORTAbits.RA4 = 0;				//Pull CS (Chip Select) low
	byte = PORTB;					//Read Information on Data Bus
	PORTAbits.RA2 = 1;				//Restore RD high
	PORTAbits.RA4 = 1;				//Restore CS high
	DDRB = 0xFF;					//Set PORTB to output
	return byte;					//Return Data
}

//***************************************************************************************************************
//							Init_7266
//***************************************************************************************************************

void Init_7266(int addr)
{
  //Setup IOR Register
  outp(XCMD(addr),IOR(DisAB + LOL + ABGate + CYBW));  	//Disable Counters and Set CY BW mode

  //Setup RLD Register
  outp(XCMD(addr),RLD(Rst_BP + Rst_FLAGS));				//Reset Byte Pointer(BP) and Flags
  outp(XDATA(addr), 0x01);  							//Load 1 to PR0 to setup Transfer to PS0
  outp(YDATA(addr), 0x01);  							//Load 1 to PR0 to setup Transfer to PS0
  outp(XCMD(addr), RLD(Rst_E + Trf_PS0_PSC));   		//Reset E Flagand Transfer PR0 to PSC
  outp(XCMD(addr), RLD(Rst_BP + Rst_CNTR));    			//Reset BP and Counter

  //Setup IDR Register
  outp(XCMD(addr),IDR(EnIDX + NIDX + LIDX));    		//Enable Negative Index on LCNTR/LOL Input

  //Setup CMR Register
  outp(XCMD(addr), CMR(BINCnt + NrmCnt + QDX1)); 		//Set Binary Normal Count Quadrature x1

  //Enable Counters
  outp(XCMD(addr), IOR(EnAB));    //Enable Counters
}

//***************************************************************************************************************
//							Write_7266_PR
//***************************************************************************************************************

void Write_7266_PR(int addr, int Data[3])
{
  outp(XCMD(addr), RLD(Rst_BP));    //Reset Byte Pointer to Synchronize Byte Writing
  outp(XDATA(addr), Data[0]);       //Write Byte to PR0
  outp(XDATA(addr), Data[1]);       //Write Byte to PR1
  outp(XDATA(addr), Data[2]);		//Write Byte to PR2
}

//***************************************************************************************************************
//							Read_7266_OL	
//***************************************************************************************************************

void Read_7266_OL(int addr)
{
  outp(XCMD(addr), RLD(Rst_BP + Trf_CNTR_OL));	//Reset Byte Pointer and Transfer Counter to Output Latch
  Count[0] = inp(XDATA(addr));					//Load Lowest Byte of X to Count 0
  Count[1] = inp(XDATA(addr));					//Load Next Byte of X to Count 1
  Count[2] = inp(XDATA(addr));					//Load Highest Byte of X to Count 2
  Count[3] = inp(YDATA(addr));					//Load Lowest Byte of Y to Count 3
  Count[4] = inp(YDATA(addr));					//Load Next Byte of Y to Count 4
  Count[5] = inp(YDATA(addr));					//Load Highest Byte of Y to Count 5
}

//***************************************************************************************************************
//							Get_7266_Flags
//***************************************************************************************************************

/*unsigned char Get_7266_Flags(int addr)
{ 
	return(inp(CMD(addr)));
} */

//***************************************************************************************************************
//							Clear_Counter
//***************************************************************************************************************

void Clear_Counter(int addr)
{
	outp(XCMD(addr), RLD(Rst_CNTR));  			//Reset Counter
}

//***************************************************************************************************************
//							main
//***************************************************************************************************************

void main (void)
{
	
    char c;
	int i;
	DDRA = 0x1F;				//Set Port A bits 0-4 as output
	DDRB = 0xFF;				//Set Port B to all output	

	Init();
	TXString("\x0D\x0A");		// Put out a new line
	TXChar('>');	
	
	Init_7266(Addr);			//Initialize the 7266 Counter

	while(1){
//		if(!isQueueEmpty()){
//			c = popQueue();
//			TXHex(c);
//			TXString("\x0D\x0A");		
//		}		

		Read_7266_OL(Addr);
		TXString("X count\n");
		TXBin(Count[2]);
		TXBin(Count[1]);
		TXBin(Count[0]);
		TXChar("\n");
		TXString("Y count\n");
		TXBin(Count[5]);
		TXBin(Count[4]);
		TXBin(Count[3]);
		TXChar("\n");
		for(i=0; i<20000; i++);
	}

	

}
  






