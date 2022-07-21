//******************************************************************************
//*	Name:			servo_motion_cfg.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Reads/parses the telescope motion config files
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++ and led by Mark Sproul
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.
//*	You must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	Jun 27,	2022	<RNS> Initial version of Motion read config routines
//*	Jul  1,	2022	<RNS> Added typedef MOTION_STATE
//*	Jul  2,	2022	<RNS> Renamed .adj field to .guide, removed .freq field
//*	Jul  2,	2022	<RNS> Changed order of typedefs to be more clear
//*	Jul  3,	2022	<RNS> Moved enum of config files to .c file
//*	Jul  6,	2022	<MLS> Moved enums BACK to .h file
//*	Jul  7,	2022	<RNS> Added enum support for max encoder rate
//****************************************************************************
//#include	"servo_motion_cfg.h"

#ifndef _SERVO_MOTION_CFG_H_
#define _SERVO_MOTION_CFG_H_

#ifndef	_INCLUDED_SERVO_STD_DEFS_
	#include	"servo_std_defs.h"
#endif


// Default name for the telescope motion config file
#define kMOTION_CFG_FILE "servo_motion.cfg"


//******************************************************************
enum
{
	MC_ADDR,
	BAUD,
	COMM_PORT,
	RA_KP_CON,
	RA_KI_CON,
	RA_KD_CON,
	RA_IL_CON,
	RA_ENC_MAX,
	DEC_KP_CON,
	DEC_KI_CON,
	DEC_KD_CON,
	DEC_IL_CON,
	DEC_ENC_MAX,

	MOTION_CFG_LAST
};	// of enum

extern TYPE_CFG_ITEM gMotionConfigArray[];


//******************************************************************
typedef enum
{
	COMPLETED = 		0,
	MOVING_BY_POS,
	MOVING_BY_VEL,
	MOVING_BY_TIME
}  TYPE_MOTION_STATE;

//******************************************************************
typedef struct
{
	// Common motor fields for any motion controller
	TYPE_MOTION_STATE	state;
	uint32_t			acc;
	uint32_t			vel;
	int32_t				trackRate;
	// Common PID + intergration limit fields for MCs
	double				kp;
	double				ki;
	double				kd;
	double				il;
	// fields unique to Roboclaw
	uint8_t				addr;
	double				motorMaxRPM;
	uint32_t			encoderMaxRate;
	uint16_t			status;
	uint8_t				cmdQueue;
	bool				buffered;
} TYPE_MOTION_MOTOR;


//******************************************************************
typedef struct
{
	TYPE_MOTION_MOTOR	motor0;
	TYPE_MOTION_MOTOR	motor1;
	TYPE_MOTION_STATE	state;
	char			    port[kMAX_STR_LEN];
	int				    baud;
} TYPE_MOTION_CONFIG;

extern TYPE_MOTION_CONFIG	gMotionConfig;
extern TYPE_CFG_ITEM		gMotionConfigArray[];

#ifdef __cplusplus
	extern "C" {
#endif

int		Servo_read_motion_cfg(const char *motionCfgFile, TYPE_MOTION_CONFIG *motionConfig);
bool	Servo_check_motion_cfg(void);
void	Print_motion_configuration(void);

#ifdef __cplusplus
}
#endif

#endif // _SERVO_MOTION_CFG_H_
