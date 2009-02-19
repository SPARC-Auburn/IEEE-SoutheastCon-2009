/*
 *  queue.h
 *  
 *
 *  Created by William Woodall on 1/17/09.
 *  Copyright 2009 Auburn University. All rights reserved.
 *
 */

struct status
{
	unsigned TxBufferFull  :1;
	unsigned TxBufferEmpty :1;
	unsigned RxBufferFull  :1;
	unsigned RxBufferEmpty :1;
	unsigned RxOverFlow :1;
	unsigned RxError:1;				
};

void			initQueue		(void);
void			SerialISR		(void);

unsigned char 	isTXEmpty		(void);
unsigned char	isRXEmpty		(void);
unsigned char	isTXFull		(void);
unsigned char	isRXFull		(void);
void			pushTXQueue		(unsigned char c);
void			pushRXQueue		(unsigned char c);
unsigned char	popTXQueue		(unsigned char *c);
unsigned char	popRXQueue		(unsigned char *c);
