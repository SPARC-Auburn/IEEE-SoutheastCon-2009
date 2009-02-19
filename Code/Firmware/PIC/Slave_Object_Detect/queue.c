/*
 *  queue.c
 *  
 *
 *  Created by William Woodall on 1/17/09.
 *  Copyright 2009 Auburn University. All rights reserved.
 *
 */

#include "queue.h"
#include "hardware.h"

#define TX_BUFFER_SIZE 120
#define RX_BUFFER_SIZE 120
#pragma udata QUEUE
unsigned char tx_buffer[TX_BUFFER_SIZE];
unsigned char rx_buffer[RX_BUFFER_SIZE];

#pragma code
unsigned char tx_count = 0;
unsigned char tx_read_ptr = 0;
unsigned char tx_write_ptr = 0;

unsigned char rx_count = 0;
unsigned char rx_read_ptr = 0;
unsigned char rx_write_ptr = 0;

struct status QueueStatus = {0,1,0,1,0,0};

void initQueue(void)
{
	tx_count = 0;
	tx_read_ptr = 0;
	tx_write_ptr = 0;
	
	rx_count = 0;
	rx_read_ptr = 0;
	rx_write_ptr = 0;

	QueueStatus.TxBufferFull = 0;
	QueueStatus.TxBufferEmpty = 1;
	QueueStatus.RxBufferFull = 0;
	QueueStatus.RxBufferEmpty = 1;
	QueueStatus.RxOverFlow = 0;
	QueueStatus.RxError = 0;
}

void SerialISR(void)
{
	unsigned char chTemp;
	if(PIR1bits.TXIF & PIE1bits.TXIE)
	{		
		if(!QueueStatus.TxBufferEmpty)
		{			
			TXREG = tx_buffer[tx_read_ptr];									
			if(QueueStatus.TxBufferFull)
			{
				QueueStatus.TxBufferFull = 0;				
			}
			tx_count--;
			if(tx_count == 0)
			{
				QueueStatus.TxBufferEmpty = 1;				
			}
			tx_read_ptr++;
			if(tx_read_ptr == TX_BUFFER_SIZE)
			{
				tx_read_ptr = 0;				
			}
		}
		else
		{   
			PIE1bits.TXIE = 0;				
		}
	}
	else if( PIR1bits.RCIF & PIE1bits.RCIE)
	{	
		if(RCSTAbits.FERR)   /* FERR error condition */
		{ 
			chTemp = RCREG;				
		}
		else if (RCSTAbits.OERR) /* OERR error condition */
		{					
			RCSTAbits.CREN = 0;
			RCSTAbits.CREN = 1;
			chTemp = RCREG;															
		}
		else if (QueueStatus.RxBufferFull) 
		{ 
			chTemp = RCREG;
		}		 
		else if(!QueueStatus.RxBufferFull)
		{									
			QueueStatus.RxBufferEmpty = 0;
			rx_buffer[rx_write_ptr] = RCREG;
			rx_count++;
			if(rx_count == RX_BUFFER_SIZE)
			{
			 	QueueStatus.RxBufferFull = 1;
			}
			rx_write_ptr++;
			if(rx_write_ptr == RX_BUFFER_SIZE)
			{
				rx_write_ptr = 0; 
			}	
		}		
}
}	

unsigned char isTXEmpty(void) 
{
	return QueueStatus.TxBufferEmpty;
}

unsigned char isRXEmpty(void)
{
	return QueueStatus.RxBufferEmpty;
}	

unsigned char isTXFull(void)
{
	return QueueStatus.TxBufferFull;
}

unsigned char isRXFull(void)
{
	return QueueStatus.RxBufferFull;
}

void pushTXQueue(unsigned char c)
{
	if(QueueStatus.TxBufferFull)
	{
		return;	
	}	
	PIE1bits.TXIE = 0; 		// Disable Interrupts for safety
	tx_buffer[tx_write_ptr] = c;
	tx_count++;
	QueueStatus.TxBufferEmpty = 0;
	if(tx_count == TX_BUFFER_SIZE)
	{
		QueueStatus.TxBufferFull = 1;
	}
	tx_write_ptr++;
	if(tx_write_ptr == TX_BUFFER_SIZE)
	{
		tx_write_ptr = 0;	
	}
	PIE1bits.TXIE = 1;		// (Re)Enable interrupts for transmit			
}
void pushRXQueue(unsigned char c)
{
	if(QueueStatus.RxBufferFull)
	{
		return;	
	}	
	PIE1bits.RCIE = 0; 		// Disable Interrupts for safety
	rx_buffer[rx_write_ptr] = c;
	rx_count++;
	QueueStatus.RxBufferEmpty = 0;
	if(rx_count == RX_BUFFER_SIZE)
	{
		QueueStatus.RxBufferFull = 1;
	}
	rx_write_ptr++;
	if(rx_write_ptr == RX_BUFFER_SIZE)
	{
		rx_write_ptr = 0;	
	}
	PIE1bits.RCIE = 1;		// (Re)Enable interrupts for receive			
}
unsigned char popTXQueue(unsigned char *c)
{
	if(QueueStatus.TxBufferEmpty)
	{
		return 0;
	}
	PIE1bits.TXIE = 0;	// Disable Interrupts for safety
	QueueStatus.TxBufferFull = 0;
	*c = tx_buffer[tx_read_ptr];
	tx_count--;
	if(tx_count == 0)
	{
		QueueStatus.TxBufferEmpty = 1;	
	}	
	tx_read_ptr++;
	if(tx_read_ptr == TX_BUFFER_SIZE)
	{
		tx_read_ptr = 0;	
	}
	PIE1bits.TXIE = 1; // (Re)Enable interrupts for transmit
	return 1;			
}
unsigned char popRXQueue(unsigned char *c)
{
	if(QueueStatus.RxBufferEmpty)
	{
		return 0;
	}
	PIE1bits.RCIE = 0;	// Disable Interrupts for safety
	QueueStatus.RxBufferFull = 0;
	*c = rx_buffer[rx_read_ptr];
	rx_count--;
	if(rx_count == 0)
	{
		QueueStatus.RxBufferEmpty = 1;	
	}	
	rx_read_ptr++;
	if(rx_read_ptr == RX_BUFFER_SIZE)
	{
		rx_read_ptr = 0;	
	}
	PIE1bits.RCIE = 1; // (Re)Enable interrupts for transmit
	return 1;			
}