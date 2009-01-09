#include "hardware.h"
#include "bm_uart.h"

/* SPBRG Settings @ 8Mhz
9.6k = 207
19.2k = 103
57.6k = 34
115.2k = 16 */

#define SPBRG_SET 207

void InitializeUART(void){
	TXSTA = 0;           // Reset USART registers to POR state
  	RCSTA = 0;

	TXSTAbits.SYNC = 0;	// Async Mode
	RCSTAbits.CREN = 1; // Continuous Reception

    TXSTAbits.BRGH = 1;
	BAUDCONbits.BRG16 = 1;

	SPBRG = SPBRG_SET;
	SPBRGH = SPBRG_SET >> 8; 
	
	TXSTAbits.TXEN = 1;  // Enable transmitter
	RCSTAbits.SPEN = 1;  // Enable receiver

	PIR1bits.TXIF = 0;
	PIR1bits.RCIF = 0;

	// Hardware UART Setup
	TRISCbits.TRISC7 = 1;
	TRISCbits.TRISC6 = 0;
}

unsigned char UARTRXRdy(void){
	if(PIR1bits.RCIF)
		return 1;
  	return 0;
}

unsigned char UARTRX(void){
	char c;
	while(PIR1bits.RCIF == 0); //Hold until receive
	c = RCREG;
	return c;
}

void UARTTX(char c){
	TXREG = c;
	while(TXSTAbits.TRMT == 0); //Hold until empty
}

void UARTTXString(char *s){
	char c;
	while((c = *s++)) UARTTX(c);
}

void UARTTXBin(unsigned char c){
	unsigned char i,j;
	j=0b10000000;
	
	UARTTXString("0b");
	
	for(i=0; i<8; i++){
		if(c&j){
			UARTTX('1');
		}else{
			UARTTX('0');
		}
		j>>=1;
	}
}

void  UARTTXDec(unsigned char c){
    unsigned char b;

    b = c;
    if(b/100) UARTTX(b/100 + '0');
    b = b - (b/100)*100;
    if(b/10) UARTTX(b/10 + '0');
    b = b - (b/10)*10;
    UARTTX(b + '0');
}

const unsigned char HEXASCII[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void UARTTXHex(unsigned int c){
    unsigned int b;

	UARTTXString("0x");
    b = (c>>4) & 0x0F;
    UARTTX(HEXASCII[b]);
    b = c & 0x0F;
    UARTTX(HEXASCII[b]);
    return;
}