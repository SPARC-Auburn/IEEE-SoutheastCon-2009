#ifndef MAIN_H
#define MAIN_H

#define SLAVE

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