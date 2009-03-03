#ifndef MAIN_H
#define MAIN_H

#define SLAVE

void	high_isr 		(void);
void	high_vec 		(void);
void 	low_vec  		(void);
void 	low_isr  		(void);

void	Refresh_EEPROM	(void);
unsigned char	poll_sonar		(void);


union int_byte {
	unsigned int lt;
	unsigned char bt[2];
};	

struct proc_status {
	unsigned ProcessInProgress			:1;
	unsigned sonar_poll_enabled			:1;	
	unsigned start_button_enabled  		:1;
};
