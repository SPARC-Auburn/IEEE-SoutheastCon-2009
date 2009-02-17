#ifndef MAIN_H
#define MAIN_H

#define SLAVE

void	high_isr 		(void);
void	high_vec 		(void);
void 	low_vec  		(void);
void 	low_isr  		(void);

void	read_antennas	(void);
void	read_ars		(void);
void	interrupt		(char);
void	cal_ars			(void);
void	cal_ant			(void);
void	line_follow		(void);
void	corner_detection(void);
void	line_detection	(void);

struct proc_status {
	unsigned ProcessInProgress			:1;
	unsigned cal_ant_enabled			:1;
	unsigned line_follow_enabled		:1;
	unsigned corner_detection_enabled	:1;
	unsigned line_detection_enabled		:1;
	
	unsigned line_follow_int			:1;
	unsigned corner_detect_int			:1;
	unsigned line_detect_int			:1; 
};

union int_byte
{
	unsigned int lt;
	unsigned char bt[2];
};	

unsigned char current_proc;
unsigned char current_parameters[32];
unsigned char parameter_count;
