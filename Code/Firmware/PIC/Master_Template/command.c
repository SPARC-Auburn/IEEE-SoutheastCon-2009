#include "command.h"
#include "hardware.h"
#include "init.h"
#include "i2c.h"
#include "serial.h"

#define I2C 1

#define HEX 1
#define DEC 2
#define BIN 3
#define RAW 4

static unsigned char busMode=1, displayFormat=1;

unsigned char checkCommand(unsigned char c){
	static unsigned char i;
	switch(c){	
		case 'o':
		case 'O': // Set output format
			TXString(" 1. HEX\x0D\x0A 2. DEC\x0D\x0A 3. BIN\x0D\x0A 4. RAW\x0D\x0A OUTPUT MODE>");
			displayFormat=getMenuReply(4);
			TXString("903 OUTPUT MODE SET\x0D\x0A");
			break;
		default:
			return 0;
	}	
	return 1;
}

void processCommandString(unsigned int nb, unsigned char *commandArr){
	unsigned int i,err=0,b=0;
	static unsigned char numberArr[12];
	
	for(i=0;i<nb;i++){
		switch(commandArr[i]){
			case '{':
			case '[':
				busStartWrite();
				break;
			case '}':
			case ']':
				busStopWrite();
				break;
			case 'r':
			case 'R':
				busReadByte();
				break;
			case 0x20:
			case ',':
			case 0x0a:
			case 0x0d:
				break;
			default:
				if(commandArr[i]>0x29 && commandArr[i]<0x40){ //First Digit Valid ASCII?
					b=0;
					numberArr[1]=0;
					while(1){
						if(i==nb){	// Break at end of data
							i--;
							break;
						}
						// Is it numeric?
						if(((commandArr[i]>0x2F && commandArr[i]<0x40) || (commandArr[i]>0x40 && commandArr[i]<0x47) ||(commandArr[i]>0x60 && commandArr[i]<0x67) || commandArr[i]=='x' || commandArr[i]=='h' || commandArr[i]=='^' || commandArr[i]=='&' || commandArr[i]=='r') ){
							numberArr[b]=commandArr[i];
							b++;
							i++;
						} else { // non-number
							i--;
							break;
						}
					}
					ASCII2BYTE(b, numberArr);
					b=0;			
				} else {
					b=0;
					err++;
					break;
				}
		}
		if(err) break;
	}
	if(err) TXString("000 SYNTAX ERROR\x0D\0A");
}

void ASCII2BYTE(unsigned char b, unsigned char *numberArr){
	unsigned char c=0,j;
	
	switch (numberArr[1]){
		case 'x':
		case 'h':
			for(j=2; j<b; j++){
				c*=0x10;
				if(numberArr[j]<0x40){
					c+=(numberArr[j]-0x30);
				}else if (numberArr[j]<0x47){
					c+=(numberArr[j]-0x37);
				}else{
					c+=(numberArr[j]-0x57);
				}
			}	
			break;
		case 'b':
			for(j=2;j<b;j++){
				c<<=1;	
				if(numberArr[j]=='1')c|=1;
			}
			break;
		default:
			for(j=0;j<b;j++){
				c*=10;
				c+=(numberArr[j]-0x30);
			}
			break;
	}
	busWriteByte(c);
}	

unsigned char getMenuReply(unsigned char i){
	unsigned char c;
	while(1){
		while(!RXReady());
		c=(RXChar())-0x31;	//Correct from ASCII
		TXString("\x0D\x0A");
		if(c<i)break;
		TXString("001 SYNTAX ERROR, TRY AGAIN>");
	}
	return c+1;
}			

void echoByteValue(unsigned char c){
	switch(displayFormat){
		case HEX:
			TXHex(c);
			break;
		case DEC:
			TXDec(c);
			break;
		case BIN:
			TXBin(c);
			break;
		case RAW:
			TXChar(c);
			break;
	}
}

void setBusMode(){
	switch(busMode){
		case I2C:
			Init_I2C();
			TXString("202 I2C Ready\x0D\x0A");
			break;
		default:
			break;
	}			
}		

void busStartWrite(void){
	switch(busMode){
		case I2C:
			StartI2C();
			TXString("210 I2C START CONDITION\x0D\x0A");
			break;
	}
}

void busStopWrite(void){
	switch(busMode){
		case I2C:
			StopI2C();
			TXString("240 I2C STOP CONDITION\x0D\x0A");
			break;
	}
}

void busWriteByte(unsigned char c){
	switch(busMode){
		case I2C:
			TXString("220 I2C WRITE:");
			echoByteValue(c);
			if(WriteI2C(c)==0){
				TXString(" GOT ACK: YES\x0D\x0A");
			} else {
				TXString(" GOT ACK: NO\x0D\x0A");
			}
			break;
	}
}				

void busReadByte(void){
	unsigned char c;
	switch(busMode){
		case I2C:
			TXString("230 I2C READ: ");
			c = ReadI2C();
			echoByteValue(c);
			TXString("\x0D\x0A");
			break;
	}
}

void busReadBulk(unsigned char c){
	unsigned char i,j;
	switch(busMode){
		case I2C:
			TXString("231 I2C BULK READ, ");
			echoByteValue(c);
			TXString(" BYTES:\x0D\x0A");
			for(i=0; i<c; i++){
				j = ReadI2C();
				echoByteValue(j);
			}
			TXString("\x0D\x0A");
	}
}