//******************************************************************************
//*	Name:			servo_mount_cfg.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Reads/parses the telescope mount config files
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
//*	May  6,	2022	<RNS> Created servo_scope_cfg.h using cproto
//*	May 10,	2022	<MLS> Added structure TYPE_SCOPE_CONFIG
//*	May 16,	2022	<RNS> Adopted _TYPE_SCOPE_CONFIG
//*	May 19,	2022	<RNS> convert .home field to .zero to avoid confusion with ASCOM Home
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> added to flipWin and offTarget to TYPE_MOUNT_CONFIG
//*	May 22,	2022	<RNS> Changed .pos from usigned to signed
//*	May 25,	2022	<MLS> Moved config token defs to .h for access from other routines
//****************************************************************************

#ifndef _SERVO_MOUNT_CFG_H_
#define _SERVO_MOUNT_CFG_H_

// Default name for the telescope mount config file
#define kSCOPE_CFG_FILE "servo_mount.cfg"


//******************************************************************
enum
{
	MC_FREQ	=	0,
	MC_ADDR,
	BAUD,
	COMM_PORT,
	MOUNT,
	PARK_SIDE,
	RA_MOTOR_MAX_RPM,
	RA_MOTOR_GEAR,
	RA_MAIN_GEAR,
	RA_ENCODER,
	RA_MAX_VEL,
	RA_MAX_ACC,
	RA_ADJ_VEL,
	RA_SI_CON,
	RA_KP_CON,
	RA_KI_CON,
	RA_KD_CON,
	RA_IL_CON,
	DEC_MOTOR_MAX_RPM,
	DEC_MOTOR_GEAR,
	DEC_MAIN_GEAR,
	DEC_ENCODER,
	DEC_MAX_VEL,
	DEC_MAX_ACC,
	DEC_ADJ_VEL,
	DEC_SI_CON,
	DEC_KP_CON,
	DEC_KI_CON,
	DEC_KD_CON,
	DEC_IL_CON,
	RA_CONFIG,
	DEC_CONFIG,
	RA_GEAR_LASH,
	DEC_GEAR_LASH,
	DEC_PARK,
	RA_SLEW_VEL,
	DEC_SLEW_VEL,
	RA_PARK,
	ROLLOVER_WIN,
	RA_PRECESSION,
	DEC_PRECESSION,
	RA_SENSOR,
	DEC_SENSOR,
	RA_PARK_SENSOR,
	DEC_PARK_SENSOR,
	OFF_TARGET_TOL
};	// of enum

//******************************************************************
typedef struct
{
	char parameter[24];
	short enumValue;
	bool found;
} TYPE_CFG_ITEM;

extern	TYPE_CFG_ITEM gMountConfigArray[];

//******************************************************************
typedef struct axis_t
{
	double		motorGear;			// here and below, real-world float values for mount physical characteristics
	double		mainGear;
	double		motorMaxRPM;
	double		encoder;
	double		realAcc;
	double		realVel;
	double		realAdj;
	double		realSlew;
	double		config;			 	//	arbitrary value based on physical motor mounting and wiring
	double		step;				//	the all important calculated steps per arcsecond value
	double		prec;				//	defines the amount of slippage between drive system, used for friction drives
	uint16_t	encoderMaxSpeed;	//	here and below are the int value for the MC, calc'd from the real-world values above
	int32_t		pos;
	uint32_t	maxAcc;
	uint32_t	acc;
	uint32_t	maxVel;
	uint32_t	vel;
	uint32_t	adj;
	uint32_t	slew;
	uint8_t		si;
	uint16_t	kp;
	uint16_t	ki;
	uint16_t	kd;
	uint16_t	il;
	uint16_t	status;
	uint8_t		cmdQueue;
	int32_t		track;
	double		direction;
	double		park;
	int8_t		parkInfo;		// on RA holds the original park_side of mount, on dec it hold the original direction before any flip
	long double	time;
	double		gearLash;
	// double	lockDown;
	double		zero;			// Axis real value at zero steps in decimal degrees, including RA
	double		standby;
	double		sync;
	uint16_t	syncValue;
	uint16_t	syncError;
} axis, *axisPtr;

//******************************************************************
typedef struct
{
	axis		ra;
	axis		dec;
	double		freq;
	uint8_t		addr;
	char		mount;
	int8_t		side;
	double 		flipWin;
	double 		offTarget;
	char		port[48];
	int			baud;
} TYPE_MOUNT_CONFIG;

#ifdef __cplusplus
	extern "C" {
#endif

extern	TYPE_MOUNT_CONFIG gScopeConfig;

int	Servo_read_mount_cfg(const char *mountCfgFile, TYPE_MOUNT_CONFIG *mountConfig);

#ifdef __cplusplus
}
#endif

#endif // _SERVO_MOUNT_CFG_H_
