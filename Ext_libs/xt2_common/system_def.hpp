#ifndef APP_SYSTEM_DEFS_HPP_
#define APP_SYSTEM_DEFS_HPP_

enum DIRECTION
{
	UNDEF 	= 0,
	FWD 	= 1,
	REV 	= 2,
};

enum
{
	BRAKETIMEOUT = 150,
	TIMEOUT_INFINITE = -1
};

enum
{
	DEFAULT_DURATION = 300,
	DURATION_INFINITE = -1
};

enum BUTTONS
{
	 P1_bm =   0x0001		//CL left
	,P2_bm =   0x0002		//Column rotation release brake
	,P3_bm =   0x0004		//tube rotation release brake & rotate tube movement enabling in command mode
	,P4_bm =   0x0008		//arm vertical release brake
	,P5_bm =   0x0010		//Arm transversal brake release
	,P6_bm =   0x0020 		//CL right
	,P7_bm =   0x0040		//release all column brakes
	,P8_bm =   0x0080		//but 8
	,P4n2_bm = 0x0100  		//arm vertical up
	,P4n3_bm = 0x0200  		//arm vertical down
};

#define CXstepsMult 51//51 //381.65; //steps per mm
#define TXstepsMult 99.41//99.41;
#define HRstepsMult 8.955 //steps per tdeg


#endif //APP_SYSTEM_DEFS_HPP_
