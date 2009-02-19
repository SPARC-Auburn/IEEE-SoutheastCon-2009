#define 	RESET_OP				0x30
#define 	EEPROM_WR_OP			0x31
#define		EEPROM_RD_OP			0x32
#define		CAL_ANT_OP				0x40
#define		LINE_FOLLOW_OP			0x41
#define		CORNER_DETECTION_OP		0x42
#define		LINE_DETECTION_OP		0x43

#define		INT_CAL_ANT_DONE		0x40	// Antenna Calibration Complete

#define		INT_LINE_LEFT			0x41
#define		INT_LINE_RIGHT			0x42
#define		INT_LINE_CENTER			0x43
#define		INT_LINE_ERROR			0x49

#define		INT_CORNER_DETECT		0x44
#define		INT_CORNER_NO_DETECT	0x45

#define		INT_LINE_DETECT_1		0x46
#define		INT_LINE_DETECT_2		0x47
#define		INT_LINE_DETECT_3		0x48