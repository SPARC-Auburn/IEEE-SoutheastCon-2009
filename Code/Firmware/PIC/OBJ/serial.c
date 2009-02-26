#include "hardware.h"
#include "serial.h"
#include "queue.h"

char RXReady(void){
	if(PIR1bits.RCIF)
		return 1;
	return 0;
}

char RXChar(void){
	char c;
	c = RCREG;
	return c;
}	

void TXChar(char c){
	 pushTXQueue(c);
}

void TXString(const rom char *s){
	do{
		TXChar(*s);
	} while( *(++s));	
}		

void TXBin(unsigned char c){
	unsigned char i, j;
	j=0b10000000;

	TXString("0b");

	for(i=0;i<8;i++){
		if(c&j){
			TXChar('1');
		}else{
			TXChar('0');
		}
		j>>=1;
	}
}

void TXBin_Int(unsigned int c){
	unsigned char i;
	unsigned int j;
	j=0b1000000000000000;

	TXString("0b");

	for(i=0;i<16;i++){
		if(c&j){
			TXChar('1');
		}else{
			TXChar('0');
		}
		j>>=1;
	}
}


void TXDec(unsigned char c){
	unsigned char b;
	
	b = c;
	if(b/100){
		TXChar(b/100+'0');
	}
	b = b - (b/100) * 100;
	if(b/10){
		TXChar(b/10+'0');
	}
	b = b - (b/10) * 10;
	TXChar(b + '0');
}

void TXDec_Int(unsigned int c){
	unsigned int b;
	
	b = c;
	TXChar(b/10000+'0');
	b = b - (b/10000) * 10000;
	TXChar(b/1000+'0');
	b = b - (b/1000) * 1000;		
	TXChar(b/100+'0');
	b = b - (b/100) * 100;
	TXChar(b/10+'0');
	b = b - (b/10) * 10;
	TXChar(b + '0');
}
	
const unsigned char HEXASCII[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void TXHex(unsigned char c){
	unsigned int b;
	
	TXString("0x");
	b = (c >> 4) & 0x0F;
	TXChar(HEXASCII[b]);
	b = c & 0x0F;
	TXChar(HEXASCII[b]);
}

void TXHex_Int(unsigned int c){
	unsigned int b;
	
	TXString("0x");
	b = (c >> 12) & 0x0F;
	TXChar(HEXASCII[b]);
	b = (c >> 8) & 0x0F;
	TXChar(HEXASCII[b]);
	b = (c >> 4) & 0x0F;
	TXChar(HEXASCII[b]);
	b = c & 0x0F;
	TXChar(HEXASCII[b]);
}
