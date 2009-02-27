#define 	RESET_OP				0x30
#define 	EEPROM_WR_OP			0x01
#define		EEPROM_RD_OP			0x02
#define		CAL_ANT_OP				0x40
#define		LINE_FOLLOW_OP			0x41
#define		CORNER_DETECTION_OP		0x42
#define		LINE_DETECTION_OP		0x43

#define 	GET_ANGLE_OP			0x60
#define		ZERO_ANGLE_OP			0x62
#define		MONITOR_ANGLE_OP		0x61

#define		INT_ANGLE				0X60


#define		INT_CAL_ANT_DONE		0x40	// Antenna Calibration Complete

#define		INT_LINE_LEFT			0x41
#define		INT_LINE_RIGHT			0x42
#define		INT_LINE_CENTER			0x43
#define		INT_LINE_ERROR			0x49


#define		INT_CORNER_DETECT		0x44
#define		INT_CORNER_NO_DETECT	0x45

#define		INT_LINE_DETECT_LEFT	0x46
#define		INT_LINE_DETECT_RIGHT	0x47
#define		INT_LINE_DETECT_FRONT	0x48

#define		INT_EXCEED_LEFT			0x61
#define		INT_EXCEED_RIGHT		0X62
