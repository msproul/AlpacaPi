//******************************************************************************
//*	Name:			servo_scope.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Astronomy motion functionality for a servo telescope mount
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
//*	Apr 26,	2022	<RNS> Initial port of the ServoStar code
//*	Apr 27,	2022	<RNS> still porting to the new RC and MC libs
//*	Apr 28,	2022	<RNS> Removed history cornercase code from 1990s
//*	Apr 28,	2022	<RNS> Unifield path motion with calc_optimal_path
//*	Apr 30,	2022	<RNS> rewrote flip routine to use only hour angles in calcs
//*	May  6,	2022	<RNS> Merged updates and ready for RC testing
//*	May  8,	2022	<RNS> renamed RC* functions from _read_ to _get_
//*	May  8,	2022	<RNS> edit move_by_pos to _posva since default_acc is broken
//*	May  8,	2022	<RNS> Added basic _TEST_ support to check build
//*	May 11,	2022	<MLS> Added Servo_stop_tracking()
//*	May 11,	2022	<MLS> Added return codes to several routines
//*	May 12,	2022	<MLS> Added "const" to filename arguments (to make compiler happy)
//*	May 12,	2022	<MLS> Converted all function parameters from D/HMS to deci hour/degs
//*	May 14,	2022	<RNS> Fixed a spelling typo in error message
//*	May 14,	2022	<RNS> Changed all _step routines to signed variables
//*	May 15,	2022	<RNS> Codereview and updated comments and potential RA range issue
//*	May 16,	2022	<RNS> Fixed lat/log access with location config global now hidden
//*	May 16,	2022	<RNS> Added Servo_pos_to_step() and removed dup code
//*	May 16,	2022	<RNS> Added a SYNCHRONOUS routine to test RC's independent moves for alt-azi
//*****************************************************************************
// Notes: M1 *MUST BE* connected to RA or Azimuth axis, M2 to Dec or Altitude
//*****************************************************************************
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include "ConsoleDebug.h"

#include "servo_std_defs.h"
#include "servo_mc_core.h"
#include "servo_time.h"
#include "servo_rc_utils.h"
#include "servo_scope_cfg.h"
#include "servo_scope.h"

#ifdef _ALPHA_OUT_
#define UNDER_TARGET 5.0
//#define RA_SENSOR_CHANGE 1
//#define DEC_SENSOR_CHANGE 2
//#define BOTH_SENSOR_CHANGE 3
#endif	// _ALPHA_OUT_

// Globals for the telescope axis for real-world floating point variables
static axis gServoRa	=	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
static axis gServoDec	=	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

// Danger, local globals ahead
static double	gSysDepend	=	1.000000;
static char		gMount;
static int		gIgnoreHorizon	=	false;
static int8_t	gAddr;
static int8_t	gSide			=	kEAST;
static double	gMeridianWindow;
static double	gOffTargetTolerance;

static TYPE_SCOPE_CONFIG gScopeConfig;

//*****************************************************************************
// Initializes the serial comm port using the identifying string and baud rates.
// Port setting will always be set to parity 'none' and stop bits equal 1.
//*****************************************************************************
int	Servo_set_comm_port(char com[], int baud)
{
int	status;

	status	=	MC_init_comm(com, baud);

	return (status);
}	// of ss_set_comm_port()

//*****************************************************************************
// Returns the park position found in the scope configuration file.
// This lockdown position is used for statically parking the scope for
// a power-off shutdown.
//*****************************************************************************
void Servo_get_park_coordins(double *ha, double *dec)
{
	*ha		=	gServoRa.park;
	*dec	=	gServoDec.park;
}

//*****************************************************************************
// Returns the standby position found in the scope configuration file.
// This standby position is used for statically parking the scope until
// another slew is needed.
//*****************************************************************************
void Servo_get_standby_coordins(double *ha, double *dec)
{
	*ha	=	gServoRa.standby;
	*dec	=	gServoDec.standby;
}

//*****************************************************************************
// Return the RA & Dec valueboth axis opto interrupters.
//*****************************************************************************
void Servo_get_sync_coordins(double *ra, double *dec)
{
	*ra	=	gServoRa.sync;
	*dec	=	gServoDec.sync;
}

//*	<MLS> these do not appear to be used anyplace -> <RNS> not yet :)
//*****************************************************************************
// This sets an approximate time ratio between a warm host computer and likely
// a cold motor controller. Modern oscillators are quite stable so do not change
// this value without good reason.
// A ration < 1.0 means the MC is running faster than its time standard
//*****************************************************************************
void Servo_set_time_ratio(double ratio)
{
	gSysDepend	=	ratio;
}

//*****************************************************************************
// Returns the current setting the time ratio
//*****************************************************************************
double Servo_get_time_ratio(void)
{
	return (gSysDepend);
}

//*****************************************************************************
// Routine scales down the Ra and Dec acceleration profiles to the passed-in
// percent of max acceleration found in the servo scope config file
//*****************************************************************************
int	Servo_scale_acc(int32_t percentRa, int32_t percentDec)
{
	// Check the range for 0 <= % <= 100
	if ((percentRa < 0) || (percentRa > 100) || (percentDec < 0) || (percentDec > 100))
	{
		return(kERROR);
	}
	// Set the percent acceleration of the max value
	gServoRa.acc	=	(gServoRa.maxAcc * percentRa) / 100;
	gServoDec.acc	=	(gServoDec.maxAcc * percentDec) / 100;
	return (kSTATUS_OK);
}

//*****************************************************************************
// Routine scales down the Ra and Dec velocity profiles to the passed-in
// percent of max acceleration found in the servo scope config file
// Return Values: kSTATUS_OK or kERROR = outside of range 0 <= x <= 100
//*****************************************************************************
int	Servo_scale_vel(int32_t percentRa, int32_t percentDec)
{
	// Check the range for 0 <= % <= 100
	if ((percentRa < 0) || (percentRa > 100) || (percentDec < 0) || (percentRa > 100))
	{
		return(kERROR);
	}
	// Set the percent velocity of the max value
	gServoRa.vel	=	(gServoRa.maxVel * percentRa) / 100;
	gServoDec.vel	=	(gServoDec.maxVel * percentDec) / 100;
	return (kSTATUS_OK);
}

//*****************************************************************************
// Allow the scope to move to object below the horizon, normally only
//	used for statically parking the scope. Expects a TRUE | FALSE
//	Returns the old horizon value for the curious
//*****************************************************************************
int	Servo_ignore_horizon(int state)
{
	int oldValue	=	gIgnoreHorizon;
	gIgnoreHorizon	=	state;
	return (oldValue);
}

//*****************************************************************************
// Resets the motor and initializes with the current motion parameters
// for any needed default values and set the home position.
// Return Values: kSTATUS_OK or kERROR = motor out of range
//*****************************************************************************
int	Servo_reset_motor(uint8_t motor)
{
	switch (motor)
	{
	case SERVO_RA_AXIS:
		// reset and intialize the mc
		RC_stop(motor);
		// This command does not seem to work as documented on RC :(
		// RC_set_default_acc(motor, gServoRa.acc);
		RC_set_home(motor);
		break;

	case SERVO_DEC_AXIS:
		RC_stop(motor);
		// This command does not seem to work as documented on RC :(
		// RC_set_default_acc(motor, gServoDec.acc);
		RC_set_home(motor);
		break;

	default:
		return (kERROR);
		break;
	}	// of switch

	return (kSTATUS_OK);
}	// of Servo_reset_motor()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert the motor controllers internal
// step registers to a 'real' RA and Dec values.
// RA and DEC are returned in deciHours and deciDegs
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec)
{
	double currPos;
	long double time;
	double home;

	// convert RA position from steps to decimal deg
	currPos	=	(gServoRa.direction) * (raStep * (1.0 / gServoRa.step)) / 3600.0;

	// compute the current home position by adding the movement of the earth since
	// saving the home position

	// get the current time and compute the elapsed time
	time	=	Time_get_systime();
	time	=	time - gServoRa.time;

	// compute the current home position with time delta, motor direction
	// should not be needed (I think) since time can't go backwards ;^)
	// home	=	gServoRa.home + (gServoRa.direction) * (gSysDepend * (double)time * ARCSEC_PER_SEC / 3600.0);
	home	=	gServoRa.home + (gSysDepend * (double)time * kARCSEC_PER_SEC / 3600.0);
	*ra		=	home + currPos;

	// convert from decidegs to decihours
	Time_deci_deg_to_hours(ra);

	// convert Dec position from steps to decimal deg
	currPos	=	(gServoDec.direction) * (decStep * (1.0 / gServoDec.step)) / 3600.0;

	// get the 'true' position with home offset in decidegs
	*dec	=	gServoDec.home + currPos;

}	// of Servo_step_to_pos()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert ra and dev values in decihour and decidegs
// to MC step positions. It does zero checks for meridian, flip/TTP, etc
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
void Servo_pos_to_step(double ra, double dec, int32_t *raStep, int32_t *decStep)
{
	double raDelta, decDelta;
	double time;
	double home;

	// Convert RA to decidegs
	Time_deci_hours_to_deg(&ra);

	// if a RA tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gServoRa.track != 0)
	{
		// get the current time and compute the elapsed time
		time	=	Time_get_systime();
		time	=	time - gServoRa.time;

		// compute the current home position with time delta
		home	=	gServoRa.home + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home	=	gServoDec.home;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.home value corresponds to zero step position
	raDelta	=	ra - home;

	// get the actual RA step needed (absolute)
	*raStep	=	(int32_t)((gServoRa.direction) * (raDelta * 3600.0 * gServoRa.step));

	// if a Dec tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gServoDec.track != 0)
	{
		// get the current time and compute the elapsed time
		time	=	Time_get_systime();
		time	=	time - gServoDec.time;

		// compute the current home position with time delta
		home	=	gServoDec.home + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home	=	gServoDec.home;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.home value corresponds to zero step position
	decDelta	=	dec - home;

	// get the actual Dec steps needed (absolute)
	*decStep	=	(int32_t)((gServoDec.direction) * (decDelta * 3600.0 * gServoDec.step));

	return;
}	// of Servo_pos_to_step()

//*****************************************************************************
// Returns the current position of the telescope in RA and Dec values.
// RA and DEC are returned in deci hours and deci degs
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
void Servo_get_pos(double *ra, double *dec)
{
int32_t currStep;
double currPos;
long double time;
double home;
double delta	=	0.0;

	// Get the current ra position
	RC_get_curr_pos(SERVO_RA_AXIS, &currStep);

	// convert RA position to decimal deg
	currPos	=	(gServoRa.direction) * (currStep * (1.0 / gServoRa.step)) / 3600.0;

	// if a Ra tracking rate has been set then add in the time delta
	if (gServoRa.track != 0)
	{
		// compute the current home position by adding the movement of the earth since
		// saving the home position

		// get the current time and compute the elapsed time
		time	=	Time_get_systime();
		time	=	time - gServoRa.time;

		// compute the current home position with time delta
		delta	=	(gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
		home	=	gServoRa.home + delta;
	}
	else
	{
		// no tracking rate then delta is not needed.
		home	=	gServoRa.home;
	}

	// add current pos + home position to get "real" value
	*ra	=	home + currPos;

	// convert to decihours
	Time_deci_deg_to_hours(ra);

	// get the current dec position
	RC_get_curr_pos(SERVO_DEC_AXIS, &currStep);

	// convert position to decimal deg
	currPos	=	(gServoDec.direction) * (currStep * (1.0 / gServoDec.step)) / 3600.0;

	// get the 'true' position with home offset
	*dec	=	gServoDec.home + currPos;

}	// Servo_get_pos()

//*****************************************************************************
// Sets the scope position to supplied RA and Dec values. Input RA and Dec
//	are in deciHour and deciDegs respectively. Assume the scope is
//	tracking more or less correctly. (ie. the scope is not stopped)
//	Both RA/Dec home position are stored in deci degrees and the
//	MC's set_home command always sets the encoder/position value to zero
//*****************************************************************************
void Servo_set_pos(double ra, double dec)
{
	// set home on the RA axis (deci degs)
	gServoRa.time	=	Time_get_systime();
	Time_deci_hours_to_deg(&ra);
	gServoRa.home	=	ra;
	RC_set_home(SERVO_RA_AXIS);

	// set home on the DEC axis (deci degs)
	gServoDec.time	=	Time_get_systime();
	gServoDec.home	=	dec;
	RC_set_home(SERVO_DEC_AXIS);
}	// Servo_set_pos()

//*****************************************************************************
// Sets the position for a stopped scope. Since the scope is not moving,
// Hour Angle is used instead of RA and inputs are deciHours and deciDegs
//*****************************************************************************
void Servo_set_static_pos(double ha, double dec)
{
	long double jd;
	long double lst;
	double ra;

	Servo_reset_motor(SERVO_RA_AXIS);
	Servo_reset_motor(SERVO_DEC_AXIS);

	// get the jd, sid time and convert to LST
	jd	=	Time_systime_to_jd();
	lst	=	Time_jd_to_sid(jd);
	lst	=	Time_sid_to_lst(lst, Time_get_lon());

	// set RA to the LST + HA offset
	ra	=	lst + ha;

	// Set the current position
	Servo_set_pos(ra, dec);
}	// of Servo_set_static_pos()

//*****************************************************************************
// Returns the current tracking rate in motor controller axis
//	in steps/sec
//*****************************************************************************
int32_t Servo_get_tracking(uint8_t motor)
{
int32_t curTrackingRate;

	curTrackingRate	=	0;
	switch (motor)
	{
	case SERVO_RA_AXIS:
		curTrackingRate	=	((int32_t)(-gServoRa.direction / gSysDepend * gServoRa.track));
		break;

	case SERVO_DEC_AXIS:
		curTrackingRate	=	((int32_t)(-gServoDec.direction / gSysDepend * gServoDec.track));
		break;

	default:
		// do nothing
		break;
	}	// of switch

	return (curTrackingRate);
}	// of Servo_get_tracking()

//*****************************************************************************
// Sets the tracking rate in motor controller axis
//	in steps/sec	period but does not start tracking
//*****************************************************************************
int	Servo_set_tracking(uint8_t motor, int32_t tracking)
{
double	ra, dec;
int		status	=	kSTATUS_OK;

	// Set the current pos with the current tracking rate and set it to now
	// but only is the step sizes for RA and Dec have been set during
	// Servo_init()
	if ((gServoRa.step != 0.0) && (gServoDec.step != 0.0))
	{
		// Get and set the position to set begin timestamp for new tracking rate
		Servo_get_pos(&ra, &dec);
		Servo_set_pos(ra, dec);

		switch (motor)
		{
		case SERVO_RA_AXIS:
			gServoRa.track	=	(int32_t)(-gServoRa.direction * gSysDepend * tracking);
			break;

		case SERVO_DEC_AXIS:
			gServoDec.track	=	(int32_t)(-gServoDec.direction * gSysDepend * tracking);
			break;

		default:
			// do nothing
			status	=	kERROR;
			break;
		}	// of switch
	}
	else
	{
		// Mount was not initialized or failed Servo_init()
		status	=	kERROR;
	}

	return (status);
}	// of Servo_set_tracking()

//*****************************************************************************
// Start the motor tracking if a tracking rate has been set in the
//	axisPtr data struct.
//*	returns kSTATUS_OK on success, kERROR otherwise
//*****************************************************************************
int	Servo_start_tracking(uint8_t motor)
{
int	status;

	status	=	-1;
	switch (motor)
	{
	case SERVO_RA_AXIS:
		// Start velocity move with the axis tracking rate
		status	=	RC_move_by_vela(SERVO_RA_AXIS, gServoRa.track, gServoRa.acc);
		break;

	case SERVO_DEC_AXIS:
		// Start velocity move with the axis tracking rate
		status	=	RC_move_by_vela(SERVO_DEC_AXIS, gServoDec.track, gServoDec.acc);
		break;

	default:
		// do nothing
		status	=	-1;
		break;
	}	// of switch

	return (status);
}	// Servo_start_tracking()

//*****************************************************************************
// Stop the motor tracking
//	axisPtr data struct.
//*	returns kSTATUS_OK on succes, -1 otherwise
//*****************************************************************************
int	Servo_stop_tracking(uint8_t motor)
{
	int	status;

	CONSOLE_DEBUG(__FUNCTION__);
	status	=	-1;
	switch (motor)
	{
	case SERVO_RA_AXIS:
		// Start velocity move with the axis tracking rate
		status	=	RC_stop(SERVO_RA_AXIS);
		break;

	case SERVO_DEC_AXIS:
		// Start velocity move with the axis tracking rate
		status	=	RC_stop(SERVO_DEC_AXIS);
		break;

	default:
		CONSOLE_DEBUG("Invalid motor specified!!!!!!!!!!!!!!!!");
		// do nothing
		status	=	-1;
		break;
	}	// of switch
	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return (status);
}	// Servo_start_tracking()

//*****************************************************************************
// This is global initialization routine for the Servo system. It read the both
// configuration files, initializes communication with the MC board and sets all
// the motion parameters. Then it initials the scope's current position to Park
// values from config fields, set the encoder home position and starts keeping track
// of any tracking rate positions changes but does not start tracking motion.
// Note: calling this routine will reset the scaling on both the RA and
// Dec axes to 100%, scale_val() and scale_acc()
// Return Values: kSTATUS_OK or kERROR
//*****************************************************************************
int	Servo_init(const char *scopeCfgFile, const char *localCfgFile)
{
int	status	=	kSTATUS_OK;
int	baud;
char port[kMAX_STR_LEN];

	CONSOLE_DEBUG(__FUNCTION__);

	// Read the location config file for the scope location and check status
	status	=	Time_read_local_cfg(localCfgFile);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open scope location file '%s'.\n", localCfgFile);
		return (kERROR);
	}

	// Read the telescope config file for the scope physical characteristics
	memset((void *)&gScopeConfig, 0, sizeof(TYPE_SCOPE_CONFIG));
	status		=	Servo_Read_Scope_Cfg(scopeCfgFile, &gScopeConfig);
	gServoRa	=	gScopeConfig.ra;
	gServoDec	=	gScopeConfig.dec;
	gAddr		=	gScopeConfig.addr;
	gMount		=	gScopeConfig.mount;
	gSide		=	gScopeConfig.side;
	baud		=	gScopeConfig.baud;
	strcpy(port, gScopeConfig.port);

	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open scope configuration file '%s'.\n", scopeCfgFile);
		return (kERROR);
	}

	// Initialize the port from the scope config file for communication
	status	=	MC_init_comm(port, baud);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not initialize comm port '%s'.\n", port);
		return (kERROR);
	}

	// convert drive precession from arcsec/deg to percents only for friction drives - this term likely will be zero
	gServoRa.prec	=	1.0 - (gServoRa.prec / 3600.0);

	// calc encoder steps per arcsec
	gServoRa.step	=	(gServoRa.motorGear * gServoRa.mainGear * gServoRa.encoder * gServoRa.prec) / 1296000.0;

	// calc the max encoder speed in counts per sec (max CPS) based on max motor RPM and encoder resolution
	gServoRa.encoderMaxSpeed	=	gServoRa.motorMaxRPM * gServoRa.encoder / 60.0;

	// Max accel is the acc in arcsec/sec^2 from the config file
	gServoRa.maxAcc	=	(uint32_t)gServoRa.realAcc * gServoRa.step;
	gServoRa.acc	=	gServoRa.maxAcc;

	//*	Initialize the encoder count to zero
	gServoRa.pos	=	0;

	// Max velocity in arcsec/sec from the config file and converted to step/sec
	gServoRa.maxVel	=	(uint32_t)gServoRa.realVel * gServoRa.step;
	gServoRa.vel	=	gServoRa.maxVel;

	// Calc the adjust and slew speeds by multiplying by the mount's steps / arcsec ratio
	gServoRa.adj	=	(uint32_t)gServoRa.realAdj * gServoRa.step;
	gServoRa.slew	=	(uint32_t)gServoRa.realSlew * gServoRa.step;

	gServoRa.status		=	0;
	gServoRa.direction	=	gServoRa.config;
	Servo_set_tracking(SERVO_RA_AXIS, (kARCSEC_PER_SEC * (double)gServoRa.step));
	gServoRa.home		=	0.0;
	gServoRa.time		=	Time_get_systime();

	// Duplicate of what's already done for RA, comments above
	gServoDec.prec				=	1.0 - (gServoDec.prec / 3600.0);
	gServoDec.step				=	(gServoDec.motorGear * gServoDec.mainGear * gServoDec.encoder * gServoDec.prec) / 1296000.0;
	gServoDec.encoderMaxSpeed	=	gServoDec.motorMaxRPM * gServoDec.encoder / 60.0;
	gServoDec.maxAcc			=	(uint32_t)gServoDec.realAcc * gServoDec.step;
	gServoDec.acc				=	gServoDec.maxAcc;
	gServoDec.pos				=	0;
	gServoDec.maxVel			=	(uint32_t)gServoDec.realVel * gServoDec.step;
	gServoDec.vel				=	gServoDec.maxVel;
	gServoDec.adj				=	(uint32_t)gServoDec.realAdj * gServoDec.step;
	gServoDec.slew				=	(uint32_t)gServoDec.realSlew * gServoDec.step;
	gServoDec.status			=	0;
	gServoDec.direction			=	gServoDec.config;

	Servo_set_tracking(SERVO_DEC_AXIS, (kARCSEC_PER_SEC * (double)gServoDec.step));
	gServoDec.home	=	0.0;
	gServoDec.time	=	Time_get_systime();

	// Make sure the "gear lash" is at least 40*axis->step arcseconds for the PID filter resolution
	if (gServoRa.gearLash < (gServoRa.step * 40.0))
	{
		gServoRa.gearLash	=	gServoRa.step * 40.0;
	}
	if (gServoDec.gearLash < (gServoDec.step * 40.0))
	{
		gServoDec.gearLash	=	gServoDec.step * 40.0;
	}

	// Set the position of the stationary mount to the park position from config file
	Servo_set_static_pos(gServoRa.park, gServoDec.park);

#ifdef _ALPHA_OUT_
	// commented out due to not sensor support for Alpha
	// Find out the initial sensor status for RA and DEC
	ss__read_sensors(&raSensor, &decSensor);

	// Check the current sensor status for RA and Dec
	if (gServoRa.syncValue == raSensor)
	{
		gServoRa.syncError	=	false;
	}
	elsefor Roboclaw, need to set the "zero" position to middle of the unsigned position range
	{

		printf("(ss__init) RA sensor value on initialization does not\n");
		printf("match RA_HOME_FLAG value in the scope configuration file\n");
		gServoRa.syncError	=	TRUE;
		printf("< Hit 'RETURN' to continue... >\n");
		ch	=	getch();
	}

	statout("Made it 4\n");

	if (gServoDec.syncValue == decSensor)
	{
		gServoDec.syncError	=	false;
	}
	else
	{
		printf("(ss__init) Dec sensor value on initialization does not\n");
		printf("match DEC_HOME_FLAG value in the scope configuration file\n");
		gServoDec.syncError	=	TRUE;
		printf("< Hit 'RETURN' to continue... >\n");
		ch	=	getch();
	}
#endif	// _ALPHA_OUT_

	// Convert the global off target tolerance from arcseconds to deci degrees
	gOffTargetTolerance	/=	3600.0;

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}	// of ss__init()

//*****************************************************************************
// Returns whether the mount is STOPPED, MOVING, TRACKING used the typedef TYPE_MOVE
// with the above fields. For stopped both axis must be stopped. If either access is
// moving, then MOVING is returned.  If the mount is tracking on either or both axis,
// TRACKING is returned.  Tracking is checked against the axis.track value and if close
// TODO: Need to add a check for parked, but is park not implemented yet
//*****************************************************************************
TYPE_MOVE Servo_state(void)
{
	uint8_t raState, decState;
	int32_t raVel, decVel;
	uint32_t absVel, absTrack;
	float velFloat, trackFloat;
	// bool		raStopped, decStopped;

	CONSOLE_DEBUG(__FUNCTION__);

	// read the command queue for RA and Dec
	RC_check_queue(&raState, &decState);
	// Go for the easy win, just see if both cmds queues are empty
	if ((raState & decState) == kRC_CMD_QUEUE_EMPTY)
	{
		return (STOPPED);
	}

	// if cmd queues have more than 1 cmd queued in either buffer (0-based), then definitely moving
	if ((raState & decState) > 0)
	{
		return (MOVING);
	}
	CONSOLE_DEBUG(__FUNCTION__);

	// one or both axis have single cmd in queue, determining moving vs tracking

	// 2nd easy case, dec is executing a command and dec is not supposed to be tracking
	// which is very likely true for any mount other than alt-azi
	if ((decState != kRC_CMD_QUEUE_EMPTY) && gServoDec.track == 0)
	{
		// Therefore dec is moving
		return (MOVING);
	}
	CONSOLE_DEBUG("If you get to here, it means the dec is not moving, but could be tracking");

	// If you get to here, it means the dec is not moving, but could be tracking

	// But check RA first, highly likely .track != 0, if any axis is tracking, then the mount
	// is tracking.  So read the current mount velocity for RA to compare to tracking rate
	RC_get_curr_velocity(SERVO_RA_AXIS, &raVel);

	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel		=	abs(raVel);
	absTrack	=	abs(gServoRa.track);

	// convert to floats
	velFloat	=	(float)absVel;
	trackFloat	=	(float)absTrack;

	// if the RA axis velocity is +/- 10% of the track velocity, then assume RA is tracking
	if (velFloat > (trackFloat * 0.9) && velFloat < (trackFloat * 1.1))
	{
		if (gServoRa.track != 0)
		{
			return (TRACKING);
		}
	}

	// Check Dec for tracking (could be an alt-azi, but do not rely on that)
	RC_get_curr_velocity(SERVO_DEC_AXIS, &decVel);
	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel		=	abs(decVel);
	absTrack	=	abs(gServoDec.track);
	// convert to floats
	velFloat	=	(float)absVel;
	trackFloat	=	(float)absTrack;

	//	CONSOLE_DEBUG_W_NUM("absVel\t=",	absVel);
	//	CONSOLE_DEBUG_W_NUM("absTrack\t=",	absTrack);
	//
	//	CONSOLE_DEBUG_W_DBL("velFloat\t=",		velFloat);
	//	CONSOLE_DEBUG_W_DBL("trackFloat\t=",	trackFloat);
	//
	//	velFloat	=	absVel;
	//	trackFloat	=	absTrack;
	//
	//	CONSOLE_DEBUG_W_DBL("velFloat\t=",		velFloat);
	//	CONSOLE_DEBUG_W_DBL("trackFloat\t=",	trackFloat);
	//
	//	CONSOLE_ABORT(__FUNCTION__);

	// We are now in alt-azi territory, in the rare case RA tracking is ~zero (near pole)
	// if the dec axis velocity is +/- 10% of the track velocity, then Dec is tracking
	if (velFloat > (trackFloat * 0.9) && velFloat < (trackFloat * 1.1))
	{
		if (gServoDec.track != 0)
		{
			return (TRACKING);
		}
	}
	// Not tracking on either axis but since cmd queue is not empty on at least
	// one axis the mount must be moving
	return (MOVING);
}	// of Servo_state()

//*****************************************************************************
// INTERNAL ROUTINE: Moves to the desired RA and Dec step position plus an addition
// distance to compensate for slew time if tracking is set on the axis. This does
// not wait or check for completion. This is a RC buffered command and will clear
// any current and commands pending. This RC command is buffered so
// completion can be checked with check_queue() where 0x80 equals complete
// or the higher level function Servo_state() looking for STOPPED or TRACKING
// If the track parameter (velocity coefficient) is non-zero for either axis
// the routine will start tracking on that axis(axes).
//*****************************************************************************
static int	Servo_move_step_track(int32_t raStep, int32_t decStep)
{
double	duration;
int32_t	pos;
int		status	=	kSTATUS_OK;

	// If RA axis has a tracking value
	if (gServoRa.track != 0)
	{
		// figure out there we are now
		status		-=	RC_get_curr_pos(SERVO_RA_AXIS, &pos);
		// calculate the duration of the axis move and convert to step and add to target
		duration	=	RC_calc_move_time(pos, raStep, gServoRa.vel, gServoRa.acc);
		raStep		+=	(uint32_t)(duration * (double)gServoRa.track);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)
		status	-=	RC_move_by_posva(SERVO_RA_AXIS, raStep, gServoRa.vel, gServoRa.acc, false);
		// This velocity command will start when the above pos cmd completes
		status	-=	RC_move_by_vela(SERVO_RA_AXIS, gServoRa.track, gServoRa.acc);
	}
	else
	{
		status	-=	RC_move_by_posva(SERVO_RA_AXIS, raStep, gServoRa.vel, gServoRa.acc, false);
	}

	// If Dec axis has a tracking value
	if (gServoDec.track != 0)
	{
		// figure out there we are now
		status	-=	RC_get_curr_pos(SERVO_DEC_AXIS, &pos);
		// calculate the duration of the axis move and convert to step and add to target
		duration	=	RC_calc_move_time(pos, decStep, gServoDec.vel, gServoRa.acc);
		raStep		+=	(uint32_t)(duration * (double)gServoDec.track);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)
		status	-=	RC_move_by_posva(SERVO_DEC_AXIS, raStep, gServoDec.vel, gServoDec.acc, false);
		// This velocity command will start when the above pos cmd completes
		status	-=	RC_move_by_vela(SERVO_DEC_AXIS, gServoDec.track, gServoDec.acc);
	}
	else
	{
		status	-=	RC_move_by_posva(SERVO_DEC_AXIS, raStep, gServoDec.vel, gServoDec.acc, false);
	}

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}	// of Servo_move_step_track()

//*****************************************************************************
// INTERNAL ROUTINE: Moves to the desired RA and Dec step position and does
// not wait or check for completion. This is a RC buffered command and will clear
// any current and commands pending. This RC command is buffered so
// completion can be checked with check_queue() where 0x80 equals complete
// or the higher level function Servo_state() looking for STOPPED
// This routine *WILL NOT* start tracking even if the track parameters are
// non-zero for either axis
//*****************************************************************************
int	Servo_move_step(int32_t raStep, int32_t decStep, bool buffered)
{
int	status;
	// Start the Dec move
	status	=	RC_move_by_posva(SERVO_DEC_AXIS, decStep, gServoDec.vel, gServoDec.acc, buffered);

	// Start the RA move and decrement status if error returned
	status	-=	RC_move_by_posva(SERVO_RA_AXIS, raStep, gServoRa.vel, gServoRa.acc, buffered);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}	// of Servo_move_step()

//*****************************************************************************
// INTERNAL ROUTINE: This routine translates the current position before
// "rolling over" for flipping the scope in overwrite input values.
// Input are in deciHours and deciDegs
// It *DOES NOT* move the mount, but just calcs and returns the flip values
//  Used on German and Split Ring mounts or on fork mounts to 'go thru the pole'
// TODO: Need to check the comment-out section to determine if it safe for from optimal_path and zero change
//*****************************************************************************
void Servo_calc_flip_coordins(double *ra, double *dec, double *direction, int8_t *side)
{
	// check to see which side the scope is on
	if (*side == kEAST)
	{
		*ra		+=	12.0;
		*side	=	kWEST;
	}
	else
	{
		*ra		-=	12.0;
		*side	=	kEAST;
	}
	// check range
	Time_normalize_RA(ra);

	// complement Dec and reverse direction
	*dec	=	180.0 - *dec;

	// toggle dec motion direction
	*direction	=	(*direction < 0) ? kREVERSE : kFORWARD;
}

//*****************************************************************************
// Stops both motors
//*****************************************************************************
void Servo_stop_all(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	// stop the motors
	RC_stop(SERVO_RA_AXIS);
	RC_stop(SERVO_DEC_AXIS);
}

//*****************************************************************************
// This is try-before-you-buy routine. It calcs the best path for the mount to from start
// to end and returns the *relative direction* needed to move the scope from the start
// position but makes *no changes*.  If you don't like the path chosen, don't use it and no
// values will have changed. It assumes that end coordins are above the horizon.
// It also will return whether a flip is needed, but *does not* make the flip coordins
// changes to the start position.  If a flip is returned, you *MUST* flip the start coordins
// for the returned path in raDirection & decDirection to be accurate.
// All inputs in decimal Hours/degs format and returns boolean if a mount flip is required
//*****************************************************************************
bool Servo_calc_optimal_path(double startRa, double startDec, double lst, double endRa, double endDec, double *raDirection, double *decDirection)
{
double	startRaHA, endRaHA;
double	startRaFlip, startDecFlip, startRaHAFlip;
double	raPath, decPath, raPathFlip, decPathFlip;
double	direction;	// not used
int8_t	side;		// not used

	// calc HA for all coordins
	startRaHA	=	lst - startRa;
	endRaHA		=	lst - endRa;

	// Converting the coordins to hour angle and normalizing solves the RA 0/24 wrap issue
	// normalize both RA HA to -12.0 < ra < 12.0
	Time_normalize_HA(&startRaHA);
	Time_normalize_HA(&endRaHA);

	// RA path distance is just  endHA - startHA
	// Note: if pathRa is neg, it means a move Eastward, a positve pathRa means Westward in North Hemisphere
	raPath	=	endRaHA - startRaHA;

	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decPath	=	endDec - startDec;
	// printf("decPath(%lf) = endDec(%lf) - startDec(%lf)\n", decPath, endDec, startDec);

	// look for the simple move where RA does not cross the meridian
	if (fabs(raPath) < fabs(startRaHA))
	{
		// got lucky, it's a simple path return the directions in HA and degs
		printf("NM---> ");
		// get the current position and convert to deci hours and degs
		*raDirection	=	raPath;
		*decDirection	=	decPath;
		return (false);
	}

	// Now on to complex moves, we are crossing the meridian
	// and first determine if we *really* need to cross the meridian

	// do a what-if calc if we flipped the start position
	startRaFlip		=	startRa;
	startDecFlip	=	startDec;
	Servo_calc_flip_coordins(&startRaFlip, &startDecFlip, &direction, &side);

	// COPIED from above - only calc new flipped start HA for all coordins
	// The end coorindates remain the same
	startRaHAFlip	=	lst - startRaFlip;

	// Converting the flipped start coordins to hour angle and normalizing solves the RA 0/24 wrap issue
	// normalize HA to -12.0 < ra < 12.0
	Time_normalize_HA(&startRaHAFlip);

	// RA flipped path distance is just endHA - startHA
	raPathFlip	=	endRaHA - startRaHAFlip;
	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decPathFlip	=	endDec - startDecFlip;
	// Check to see if the flipped coordins are now a simple move, where RA does not cross the meridian
	if (fabs(raPathFlip) < fabs(startRaHA))
	{
		// got lucky with the flip, it's a simple path return the directions in HA and degs
		printf("FTP--> ");
		*raDirection	=	raPathFlip;
		*decDirection	=	decPathFlip;
		return (true);
	}

	// OK, now we need to cross the meridian, do not pass 'go', do not collect $200 ;^)

	// Check to see of the RA axis moves at least 10% less on the RA flip path
	if (fabs(raPathFlip) < (fabs(raPathFlip) * 0.9))
	{
		printf("SFCM-> ");

		*raDirection	=	raPathFlip;
		*decDirection	=	decPathFlip;
		return (true);
	}

	// Check to see if a GEM move goes past the meridian flip window
	if (gMount == kGERMAN)
	{
		// Check to see if a GEM mount must flip due to the RA path exceed the meridian window
		if ((fabs(raPath) - fabs(startRaHA)) > gMeridianWindow)
		{
			printf("FGCM->");

			*raDirection	=	raPathFlip;
			*decDirection	=	decPathFlip;
			return (true);
		}
	}

	// Basically all easy and special cases for mount type are complete
	// so for a FORK, it just a normal move across the meridian
	// If GEM mount got here, it means it crosses the meridian but still
	// within the meridian flip window, so no flip required and stick
	// with the original path calculation
	printf("CM---> ");

	*raDirection	=	raPath;
	*decDirection	=	decPath;
	return (false);
}

#ifdef _ALPHA_OUT_
//*****************************************************************************
// TODO: WIP functions the may or may not be needed with new system
//*****************************************************************************
//*	May 10,	2022	<MLS> Fixed Servo_calc_move_to_remove_gearlash()
void Servo_calc_move_to_remove_gearlash(void)
{
	uint32_t currRaStep, currDecStep;

	if (ss__get_tracking(SERVO_RA_AXIS) >= 0)
	{
		// tracking is in the positive direction come in on the negative side of the target
		currRaStep	+=	(uint32_t)(gServoRa.direction * UNDER_TARGET * gServoRa.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currRaStep	-=	(uint32_t)(gServoRa.direction * UNDER_TARGET * gServoRa.step);
	}
	// Take care of the Dec axis
	if (ss__get_tracking(SERVO_DEC_AXIS) >= 0)
	{
		// tracking is in the positive direction come in on the negative side of the target
		currDecStep	+=	(uint32_t)(gServoDec.direction * UNDER_TARGET * gServoDec.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currDecStep	-=	(uint32_t)(gServoDec.direction * UNDER_TARGET * gServoDec.step);
	}
}
#endif

//*****************************************************************************
// This is the a simple spiral move routine and is a SYNCRONOUS CALL. It is
// merely for testing right now and is a prototype for the moves needed to support
// alt-azi mount. It will spiral out 1 to 3 times depending on the arg count
// The routine take in RA deci-hours and Dec and is error checking free very
// primitive routine, so should be used with caution.  It will not check for
// flip, TTP or anything else.  It's like a joystick, don't get overconfident.
//*****************************************************************************
void Servo_spiral_move(double raMove, double decMove, int loop)
{
double	ra, dec;
int		count, pos;
double	currRaMove	=	0.0;
double	currDecMove	=	0.0;
int32_t	raStep, decStep;

	typedef struct spiral_matrix_t
	{
		short ra;
		short dec;
	} TYPE_SPIRAL_MOVE;

	// below is the matrix, moves from position x to x +1, intial position is 0
	//	2	3	4
	//	1	0	5
	//	8	7	6

	TYPE_SPIRAL_MOVE spiralMat[9] =
	{
		{-1, 0},
		{0, 1},
		{1, 0},
		{1, 0},
		{0, -1},
		{0, -1},
		{-1, 0},
		{-1, 0},
		{0, 1}
	};

	// Make sure the max spiral loop is <= 3
	loop	=	(loop > 3) ? 3 : loop;

	// get current position
	Servo_get_pos(&ra, &dec);

	// Scale the spiral to 50% first loop, 100% second and 150% third
	for (count = 0; count < loop; count++)
	{
		currRaMove	+=	raMove * 0.5;
		currDecMove	+=	decMove * 0.5;

		// Move the matrix of positions, first "left" then CW
		for (pos = 0; pos < 9; pos++)
		{
			ra	+=	(spiralMat[pos].ra) * currRaMove;
			dec	+=	(spiralMat[pos].dec) * currDecMove;

			// calc the step location from the updated ra/dec
			Servo_pos_to_step(ra, dec, &raStep, &decStep);
			// use buffered moves
			Servo_move_step(raStep, decStep, true);
		}

		// TODO:  Make Async  SYNCRONOUS CALL
		while (Servo_state() == MOVING)
		{
			sleep(1);
		}
	}
}

//*****************************************************************************
// This is the main motion routine.  The routine take in RA deci-hours and
// Dec, latitude and longitude in deci-degs.  This routine calls calc_optimal_path()
// that will determine the 'best' path and perform a flip/thru-the-pole if needed
//*****************************************************************************
int	Servo_move_to_coordins(double gotoRa, double gotoDec, double lat, double lon)
{
int32_t	targetRaStep, targetDecStep;
double	targetRa, targetDec;
double	raRelDir, decRelDir;
double	currRa, currDec;
double	direction;
double	jd, lst;
double	alt, azi;
int		status	=	kSTATUS_OK;
bool	flip	=	false;

	// convert ra hours to decidegs
	Time_deci_hours_to_deg(&gotoRa);

	// Calc time jd, sid, lst to get alt-azi
	jd	=	Time_systime_to_jd();
	lst	=	Time_jd_to_sid(jd);
	lst	=	Time_sid_to_lst(lst, lon);
	// alt-azi is returned in radians
	Time_ra_dec_to_alt_azi(gotoRa, gotoDec, lst, lat, &alt, &azi);

	// if object is below the horizon
	if (gIgnoreHorizon == false && alt < 0.0)
	{
		// this is astronomy not geology, so return
		return(kBELOW_HORIZON);
	}

	// get the current position in deci hours and degs
	Servo_get_pos(&currRa, &currDec);

	// Determine the 'best' path form curr position to the goto position
	flip	=	Servo_calc_optimal_path(currRa, currDec, lst, gotoRa, gotoDec, &raRelDir, &decRelDir);

	// If flip is return, You must flip the mount otherwise the rel dir for RA/Dec will be incorrect
	if (flip == true)
	{
		// Do the actual flip on the current coordins
		// Current coordins are guaranteed to change after this call
		Servo_calc_flip_coordins(&currRa, &currDec, &direction, &gSide);

		// Compare to the orignal direction values for Dec axix from the scope config file
		if (gSide != gServoRa.parkInfo)
		{
			// set the direction to the opposite from config file
			gServoDec.direction	=	(gServoDec.parkInfo > 0.0) ? kREVERSE : kFORWARD;
		}
		else
		{
			// side equals the value in the config file, so set direction to config file value
			gServoDec.direction	=	(double)gServoDec.parkInfo;
		}
	}

	// determine the new target based on the  RA/Dec relative direction
	targetRa	=	currRa + raRelDir;
	targetDec	=	currDec + decRelDir;

	// convert the target RA & Dec to target steps
	Servo_pos_to_step(targetRa, targetDec, &targetRaStep, &targetDecStep);

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:  add back in gearlash,  KLUDGE!
	/////////////////////////////////////////////////////////////////////////////////////

	status	-=	Servo_move_step_track(targetRaStep, targetDecStep);

	// If status is < zero, return error
	return(status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}	// of Servo_move_to_coordins()

//*****************************************************************************
// This routine is for stopping the scope for extended periods of time and
// uses and Hour Angle and not RA for an accurate static position.
// Inputs are in deciHour and deciDeg and routine is asynchronous so you need to
// to poll with Servo_state() and wait for STOPPED state
//*****************************************************************************
int	Servo_move_to_static(double parkHA, double parkDec)
{
int32_t	targetRaStep, targetDecStep;
double	currRaDelta, currDecDelta;
double	targetRa, targetDec;
double	currRa, currDec;
double	dummy	=	0;
double	currHA, targetHA, raRelDir;
double	time, jd, lst;
double	home;
int		status	=	kSTATUS_OK;

	targetHA	=	parkHA;
	targetDec	=	parkDec;

	// convert ra deciHours to deciDegs
	Time_deci_hours_to_deg(&targetHA);

	// if the mount is currently thru-the-pole or flipped from config file setting
	if (gSide != gServoRa.parkInfo)
	{
		// Get the current position and flip it
		Servo_get_pos(&currRa, &currDec);
		// Unflip the mount
		Servo_calc_flip_coordins(&currRa, &currDec, &dummy, &gSide);

		// Since side now equals the value in the config file, so set direction from config file value
		gServoDec.direction	=	(double)gServoDec.parkInfo;
	}

	// Calc time jd, sid to get LST
	jd	=	Time_systime_to_jd();
	lst	=	Time_jd_to_sid(jd);
	lst	=	Time_sid_to_lst(lst, Time_get_lon());

	// calc HA of the current RA position
	currHA	=	lst - currRa;

	// relative change needed for RA by subtracting current pos HA from the input park RA
	raRelDir	=	targetHA - currHA;

	// determine the new target based on the  RA/Dec relative direction
	targetRa	=	currRa + raRelDir;
	// Dec relative calc is not needed since targetDec is just the parkDec

	// Prepare Dec axis for move first (Dec is typically less time sensitive than RA)
	// However, this sensitivity will change when adding alt-azi support

	// if a Dec tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gServoDec.track != 0)
	{
		// get the current time and compute the elapsed time
		time	=	Time_get_systime();
		time	=	time - gServoDec.time;

		// compute the current home position with time delta
		home	=	gServoDec.home + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home	=	gServoDec.home;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.home value corresponds to zero step position
	currDecDelta	=	targetDec - home;

	// get the actual Dec steps needed (absolute)
	targetDecStep	=	(uint32_t)((gServoDec.direction) * (currDecDelta * 3600.0 * gServoDec.step));

	// Prepare RA axis for move last (less change from the systime call)

	// if a RA tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gServoRa.track != 0)
	{
		// get the current time and compute the elapsed time
		time	=	Time_get_systime();
		time	=	time - gServoRa.time;

		// compute the current home position with time delta
		home	=	gServoRa.home + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home	=	gServoDec.home;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.home value corresponds to zero step position
	currRaDelta	=	targetRa - home;

	// get the actual RA step needed  (absolute)
	targetRaStep	=	(uint32_t)((gServoRa.direction) * (currRaDelta * 3600.0 * gServoRa.step));

	// Check the status and if not zero, decrement
	status	-=	Servo_move_step(targetRaStep, targetDecStep, false);

	// If status is < zero, return error
	return(status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}	// of Servo_move_to_static()

//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
#ifdef _TEST_SERVO_SCOPE_
int	main(int argc, char **argv)
{
	Servo_init(NULL, NULL);
	return (0);
}
#endif	// _TEST_SERVO_SCOPE_
