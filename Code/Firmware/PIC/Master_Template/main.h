#ifndef MAIN_H
#define MAIN_H

int		init 				(void);
void	Init_Oscillator		(void);
void	Init_Interrupts		(void);
void	Init_I2C			(void);
void 	Init_USART			(void);

void	high_isr 		(void);
void	high_vec 		(void);
void 	low_vec  		(void);
void 	low_isr  		(void);

#endif
