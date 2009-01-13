#include <p18cxxx.h>
#include "serial.h"
#include "usart.h"

void TXBin(unsigned char c){
	unsigned char i,j;
	j=0b10000000;

	putrsUSART("0b");

	for(i=0;i<8;i++){
		if(c&j){
			while(BusyUSART());
			WriteUSART('1');
		}else{
			while(BusyUSART());
			WriteUSART('0');
		}
		j>>=1;
	}

}

void TXDec(unsigned char c){
	unsigned char b;
	
	b = c;
	if(b/100){
		while(BusyUSART());
		WriteUSART(b/100+'0');
	}
	b = b - (b/100) * 100;
	if(b/10){
		while(BusyUSART());
		WriteUSART(b/10+'0');
	}
	b = b - (b/10) * 10;
	WriteUSART(b + '0');
}

const unsigned char HEXASCII[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void TXHex(unsigned int c){
	unsigned int b;
	
	putrsUSART("0x");
	b = (c >> 4) & 0x0F;
	while(BusyUSART());
	WriteUSART(HEXASCII[b]);
	b = c & 0x0F;
	while(BusyUSART());
	WriteUSART(HEXASCII[b]);
}
