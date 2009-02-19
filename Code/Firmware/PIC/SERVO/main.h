#ifndef MAIN_H
#define MAIN_H

#define SLAVE

#define SERVO0	0xFE
#define SERVO1	0xFD
#define SERVO2	0xFB
#define SERVO3	0xF7

union Servo
{
	unsigned int lt;
	unsigned char bt[2];
};	


int		init 			(void);
void	Init_Oscillator		(void);
void	Init_Interrupts		(void);

void	high_isr 		(void);
void	high_vec 		(void);
void 	low_vec  		(void);
void 	low_isr  		(void);


#endif
