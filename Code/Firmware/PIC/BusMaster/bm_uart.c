#include "hardware.h"
#include "bm_uart.h"
#include "usart.h"

void InitializeUART(void){
	OpenUSART(USART_TX_INT_OFF & 
				USART_RX_INT_OFF & 
				USART_ASYNCH_MODE &
				USART_EIGHT_BIT &
				USART_CONT_RX &
				USART_BRGH_HIGH,
				25);
}

void UARTTXString(char *s){
	putrsUSART(s);
}

void UARTTXBin(unsigned char c){
	unsigned char i,j;
	j=0b10000000;
	
	UARTTXString("0b");
	
	for(i=0; i<8; i++){
		if(c&j){
			WriteUSART('1');
		}else{
			WriteUSART('0');
		}
		j>>=1;
	}
}

void  UARTTXDec(unsigned char c){
    unsigned char b;

    b = c;
    if(b/100) WriteUSART(b/100 + '0');
    b = b - (b/100)*100;
    if(b/10) WriteUSART(b/10 + '0');
    b = b - (b/10)*10;
    WriteUSART(b + '0');
}

const rom unsigned char HEXASCII[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void UARTTXHex(unsigned int c){
    unsigned int b;

	UARTTXString("0x");
    b = (c>>4) & 0x0F;
    WriteUSART(HEXASCII[b]);
    b = c & 0x0F;
    WriteUSART(HEXASCII[b]);
    return;
}