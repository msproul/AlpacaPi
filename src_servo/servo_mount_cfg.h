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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	May  6,	2022	<RNS> Created servo_scope_cfg.h using cproto
//*	May 10,	2022	<MLS> Added structure TYPE_MOUNT_CONFIG
//*	May 16,	2022	<RNS> Adopted TYPE_MOUNT_CONFIG
//*	May 19,	2022	<RNS> convert .home field to .zero to avoid confusion with ASCOM Home
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> added to flipWin and offTarget to TYPE_MOUNT_CONFIG
//*	May 22,	2022	<RNS> Changed .pos from unsigned to signed
//*	Jun 12,	2022	<RNS> Changed PIDL fields from integer to double
//*	Jun 26,	2022	<RNS> Added support for TTP (thru-the-pole) config field
//*	Jun 27,	2022	<RNS> Changed default mount cfg #define to kMOUNT_CFG_FILE
//*	Jul  3,	2022	<RNS> Changed .time/.zero to .zeroTS/.zeroPos for clarity
//*	Jul  3,	2022	<RNS> Moved enum of config files to .c file
//*	Jul  4,	2022	<RNS> Moved tracking rate support back in from Motion_
//****************************************************************************

#ifndef _SERVO_MOUNT_CFG_H_
#define _SERVO_MOUNT_CFG_H_

#ifndef	_INCLUDED_SERVO_STD_DEFS_
	#include	"servo_std_defs.h"
#endif


// Default name for the telescope mount config file
#define kMOUNT_CFG_FILE "servo_mount.cfg"


//******************************************************************
enum
{
	MOUNT,
	TTP,
	PARK_SIDE,
	RA_MOTOR_GEAR,
	RA_MAIN_GEAR,
	RA_ENCODER,
	RA_MAX_VEL,
	RA_MAX_ACC,
	RA_ADJ_VEL,
	DEC_MOTOR_GEAR,
	DEC_MAIN_GEAR,
	DEC_ENCODER,
	DEC_MAX_VEL,
	DEC_MAX_ACC,
	DEC_ADJ_VEL,
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
	OFF_TARGET_TOL,

	MOUNT_CFG_LAST
};	// of enum

//******************************************************************

extern TYPE_CFG_ITEM gMountConfigArray[];

//******************************************************************
typedef struct
{
	double		motorGear;		// here and below, real-world float values for mount physical characteristics
	double		mainGear;
	double		encoder;
	double		realAcc;
	double		realVel;
	double		realAdj;
	double		realSlew;
	double		config;			//	arbitrary value based on physical motor mounting and wiring
	double		step;			//	the all important calculated steps per arcsecond value
	double		prec;			//	defines the amount of slippage between drive system, used for friction drives
	double		direction;
	double		park;

	uint32_t	maxAcc;
	uint32_t	acc;
	uint32_t	maxVel;
	uint32_t	vel;

	int32_t		defaultRate;	// default rate is set to sidereral in signed/steps
	int32_t		trackRate;
	uint32_t	guideRate;
	uint32_t	manSlewRate;

	int8_t		parkInfo;		// on RA holds the original park_side of mount, on dec it hold the original direction before any flip
	double		gearLash;
	long double	zeroTS;			// Timestamp the zero position was set
	double		zeroPos;		// Axis real value at zero steps in decimal degrees, including RA
	double		standby;
	double		sync;
	uint16_t	syncValue;
	uint16_t	syncError;
} TYPE_MountAxis;
//-	*axisPtr;
//******************************************************************
typedef struct
{
	TYPE_MountAxis	ra;
	TYPE_MountAxis	dec;
	char			mount;
	int				ttp;
	int8_t			side;
	double 			flipWin;
	double 			offTarget;
} TYPE_MOUNT_CONFIG;

extern TYPE_MOUNT_CONFIG gMountConfig;

#ifdef __cplusplus
	extern "C" {
#endif

int 	Servo_read_mount_cfg(const char *mountCfgFile, TYPE_MOUNT_CONFIG *mountConfig);
bool	Servo_check_mount_cfg(void);
void	Print_mount_configuration(void);

#ifdef __cplusplus
}
#endif

#endif // _SERVO_MOUNT_CFG_H_
