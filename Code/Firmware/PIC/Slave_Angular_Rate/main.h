#ifndef MAIN_H
#define MAIN_H

#define SLAVE

int		init 			(void);
void	Init_Oscillator		(void);
void	Init_Interrupts		(void);

void	high_isr 		(void);
void	high_vec 		(void);
void 	low_vec  		(void);
void 	low_isr  		(void);

union int_byte {
	unsigned int lt;
	unsigned char bt[2];
};	
