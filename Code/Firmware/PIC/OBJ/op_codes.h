#define 	RESET_OP				0x30
#define 	EEPROM_WR_OP			0x31
#define		EEPROM_RD_OP			0x32

#define		POLL_SONAR_OP			0x60		// `
#define		START_BUTTON_OP			0x61

#define		SONAR_NO_OBJECT			0x71		// q
#define		SONAR_PLASTIC			0x74		// t
#define		SONAR_GLASS				0x73
#define		SONAR_ALUMINUM			0x72
#define		SONAR_ERROR				0x75
#define		START_BUTTON_TRIGGERED  0x76


//#define		LINE_FOLLOW_OP			0x11
//#define		CORNER_DETECTION_OP		0x12
//#define		LINE_DETECTION_OP		0x13
//
//#define		INT_CAL_ANT_DONE		0x10	// Antenna Calibration Complete
//
//#define		INT_LINE_LEFT			0x11
//#define		INT_LINE_RIGHT			0x12
//#define		INT_LINE_CENTER			0x13
//#define		INT_LINE_ERROR			0x19
//
//#define		INT_CORNER_DETECT		0x14
//#define		INT_CORNER_NO_DETECT	0x15
//
//#define		INT_LINE_DETECT_1		0x16
//#define		INT_LINE_DETECT_2		0x17
//#define		INT_LINE_DETECT_3		0x18
