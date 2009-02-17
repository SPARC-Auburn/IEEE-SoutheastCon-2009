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

#define TX_BUFFER_SIZE 125
#define RX_BUFFER_SIZE 125
#pragma udata QUEUE
volatile unsigned char tx_buffer[TX_BUFFER_SIZE];
volatile unsigned char rx_buffer[RX_BUFFER_SIZE];

#pragma code
volatile unsigned char tx_count = 0;
volatile unsigned char tx_read_ptr = 0;
volatile unsigned char tx_write_ptr = 0;

volatile unsigned char rx_count = 0;
volatile unsigned char rx_read_ptr = 0;
volatile unsigned char rx_write_ptr = 1;

volatile struct status QueueStatus;

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
}

void pushRXQueue(unsigned char c)
{
	if(QueueStatus.RxBufferFull)
	{
		return;	
	}	
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
}

unsigned char popTXQueue(unsigned char *c)
{
	if(QueueStatus.TxBufferEmpty)
	{
		return 0;
	}
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
	return 1;			
}

unsigned char popRXQueue(unsigned char *c)
{
	if(QueueStatus.RxBufferEmpty)
	{
		return 0;
	}
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
	return 1;			
}
