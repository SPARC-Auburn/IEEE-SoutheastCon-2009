#define 	RESET_OP				0x00
#define 	EEPROM_WR_OP			0x01
#define		EEPROM_RD_OP			0x02
#define		CAL_ANT_OP				0x10
#define		LINE_FOLLOW_OP			0x11
#define		CORNER_DETECTION_OP		0x12
#define		LINE_DETECTION_OP		0x13

#define		INT_CAL_ANT_DONE		0x10	// Antenna Calibration Complete

#define		INT_LINE_LEFT			0x11
#define		INT_LINE_RIGHT			0x12
#define		INT_LINE_CENTER			0x13
#define		INT_LINE_ERROR			0x19

#define		INT_CORNER_DETECT		0x14
#define		INT_CORNER_NO_DETECT	0x15

#define		INT_LINE_DETECT_1		0x16
#define		INT_LINE_DETECT_2		0x17
#define		INT_LINE_DETECT_3		0x18