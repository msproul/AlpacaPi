//******************************************************************************
//*	Name:			servo_mount.c
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
//*	May 19,	2022	<RNS> convert .home field to .zero to avoid confusion with ASCOM Home
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> Connect flipWin & offTarget to private globals in this file
//*	May 20,	2022	<RNS> Changed gIgnoreHorizon from int to bool
//*	May 20,	2022	<RNS> Added park state functions plus support in Servo_state()
//*	May 21,	2022	<RNS> added addr arg to RC_ MC cmds for multi-RC support
//*	May 21,	2022	<RNS> added _move_to_park() and mod'd Servo_state to set the Park flag
//*	May 21,	2022	<RNS> added Marked the internal routines static and commented out from .h
//*	May 22,	2022	<RNS> Changed main() back to be void, _TEST_ should never take args
//*	May 22,	2022	<RNS> Changed routines using steps as arg to _step_ and marked internal
//*	May 22,	2022	<RNS> added _axis_ to functions that worked a single axis
//*	May 22,	2022	<RNS> added move_axis_by_vel for the ASCOM guiding functionality
//*	May 22,	2022	<RNS> General cleanup to get data/control flow aligned with ASCOM
//*	May 27,	2022	<MLS> Deleted gServoRa & gServoDec, now part of gMountConfig
//*	May 27,	2022	<MLS> Updated all configuration reference to use gMountConfig
//*	May 29,	2022	<RNS> Fixed bug ind _unpark(), gMountAction must be set to stopped
//*	May 30,	2022	<RNS> Tracked down misc ra & dec reference that were copy/paste errors
//*	May 30,	2022	<RNS> Fixed a conversion error in _calc_opt_path
//*	May 30,	2022	<RNS> Fixed bug in servo_state that returned moving to often
//*	May 31,	2022	<RNS> Fixed a mask in servo_state
//*	May 31,	2022	<RNS> Add debug tags for servo_state and cal_opt_path for  multi returns
//*	May 31,	2022	<RNS> Fixed the RA rel direction sign, was returning HA vector, not RA
//*	May 31,	2022	<RNS> Added a forgotten _set_pos after doing a flip
//*	Jun  1,	2022	<RNS> Rewrote _calc_optimal_path(), added _calc_short_vector()
//*	Jun  2,	2022	<RNS> Merged code and fix a sign bug in _set_static_pos()
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
#include "servo_mount_cfg.h"
#include "servo_mount.h"
//#include	"mini_alpaca_defs.h"

#ifdef _ALPHA_OUT_
#define UNDER_TARGET 5.0
//#define	RA_SENSOR_CHANGE	1
//#define	DEC_SENSOR_CHANGE	2
//#define	BOTH_SENSOR_CHANGE	3
#endif // _ALPHA_OUT_

// Globals for the telescope mount axis for real-world floating point variables
TYPE_MOUNT_CONFIG gMountConfig;

// Danger, local globals ahead
static double gSysDepend = 1.000000;
static bool gIgnoreHorizon = false;
static bool gParkState = true;
static TYPE_MOVE gMountAction;
static char gDebugInfoSS[] = "00";
static char gDebugInfoCOP[] = "00000";

//*****************************************************************************
// Initializes the serial comm port using the identifying string and baud rates.
// Port setting will always be set to parity 'none' and stop bits equal 1.
//*****************************************************************************
int Servo_set_comm_port(char com[], int baud)
{
	int status;

	status = MC_init_comm(com, baud);

	return (status);
} // of ss_set_comm_port()

//*****************************************************************************
// Returns the park position found in the mount configuration file.
// This lockdown position is used for statically parking the mount for
// a power-off shutdown.
//*****************************************************************************
void Servo_get_park_coordins(double *ha, double *dec)
{
	*ha = gMountConfig.ra.park;
	*dec = gMountConfig.dec.park;
}

//*****************************************************************************
// Returns the standby position found in the mount configuration file.
// This standby position is used for statically parking the mount until
// another slew is needed.
//*****************************************************************************
void Servo_get_standby_coordins(double *ha, double *dec)
{
	*ha = gMountConfig.ra.standby;
	*dec = gMountConfig.dec.standby;
}

//*****************************************************************************
// Return the RA & Dec valueboth axis opto interrupters.
//*****************************************************************************
void Servo_get_sync_coordins(double *ra, double *dec)
{
	*ra = gMountConfig.ra.sync;
	*dec = gMountConfig.dec.sync;
}

//*****************************************************************************
// This sets an approximate time ratio between a warm host computer and likely
// a cold motor controller. Modern oscillators are quite stable so do not change
// this value without good reason.
// A ration < 1.0 means the MC is running faster than its time standard
//*****************************************************************************
void Servo_set_time_ratio(double ratio)
{
	gSysDepend = ratio;
}

//*****************************************************************************
// Returns the current setting the time ratio
//*****************************************************************************
double Servo_get_time_ratio(void)
{
	return (gSysDepend);
}

//*****************************************************************************
// Releases the mount "parking brake" and allow movement. I order for an unpark
// to happen, the mount must be parked or stopped (allows for error recovery)
// It return bool of true is successful, false if mount could not be unparked
// is PARKING and not completed the move.  Note: the only way to set the park
// state to PARKED is via Servo_move_to_park() and a call to Servo_state
//*****************************************************************************
bool Servo_unpark(void)
{
	int state;

	state = Servo_state();
	if (state == PARKED || state == STOPPED)
	{
		gParkState = false;
		gMountAction = STOPPED;
	}

	return (!gParkState);
}

//*****************************************************************************
// Returns the mount's current Park state, and will return false if the mount
// is PARKING and not completed the move.  Note: the only way to set the park
// state to PARKED is via Servo_move_to_park() and a call to Servo_state
//*****************************************************************************
bool Servo_get_park_state(void)
{
	int state;

	state = Servo_state();

	return (state == PARKED ? true : false);
}

//*****************************************************************************
// Returns the mount's current side of pier, using ASCOM nonsensical definition
// of pier state.
//*****************************************************************************
int8_t Servo_get_pier_side(void)
{
	return gMountConfig.side;
}
//*****************************************************************************
// Routine scales down the Ra and Dec acceleration profiles to the passed-in
// percent of max acceleration found in the servo mount config file
//*****************************************************************************
int Servo_scale_acc(int32_t percentRa, int32_t percentDec)
{
	// Check the range for 0 <= % <= 100
	if ((percentRa < 0) || (percentRa > 100) || (percentDec < 0) || (percentDec > 100))
	{
		return (kERROR);
	}
	// Set the percent acceleration of the max value
	gMountConfig.ra.acc = (gMountConfig.ra.maxAcc * percentRa) / 100;
	gMountConfig.dec.acc = (gMountConfig.dec.maxAcc * percentDec) / 100;
	return (kSTATUS_OK);
}

//*****************************************************************************
// Routine scales down the Ra and Dec velocity profiles to the passed-in
// percent of max acceleration found in the servo mount config file
// Return Values: kSTATUS_OK or kERROR = outside of range 0 <= x <= 100
//*****************************************************************************
int Servo_scale_vel(int32_t percentRa, int32_t percentDec)
{
	// Check the range for 0 <= % <= 100
	if ((percentRa < 0) || (percentRa > 100) || (percentDec < 0) || (percentDec > 100))
	{
		return (kERROR);
	}
	// Set the percent velocity of the max value
	gMountConfig.ra.vel = (gMountConfig.ra.maxVel * percentRa) / 100;
	gMountConfig.dec.vel = (gMountConfig.dec.maxVel * percentDec) / 100;
	return (kSTATUS_OK);
}

//*****************************************************************************
// Allows the mount to move to object below the horizon, normally only
//	used for statically parking the mount. Expects a TRUE | FALSE
//	Returns the old horizon value for the curious
//*****************************************************************************
bool Servo_ignore_horizon(bool state)
{
	bool oldValue = gIgnoreHorizon;
	gIgnoreHorizon = state;
	return (oldValue);
}

//*****************************************************************************
// Resets the motor and initializes with the current motion parameters
// for any needed default values and set the home position.
// Return Values: kSTATUS_OK or kERROR = motor out of range
//*****************************************************************************
int Servo_reset_motor(uint8_t motor)
{
	switch (motor)
	{
	case SERVO_RA_AXIS:
		// reset and intialize the mc
		RC_stop(gMountConfig.addr, motor);
		// This command does not seem to work as documented on RC :(
		// RC_set_default_acc(gMountConfig.addr, motor, gMountConfig.ra.acc);
		RC_set_home(gMountConfig.addr, motor);
		break;

	case SERVO_DEC_AXIS:
		RC_stop(gMountConfig.addr, motor);
		// This command does not seem to work as documented on RC :(
		// RC_set_default_acc(gMountConfig.addr, motor, gMountConfig.dec.acc);
		RC_set_home(gMountConfig.addr, motor);
		break;

	default:
		return (kERROR);
		break;
	} // of switch

	return (kSTATUS_OK);
} // of Servo_reset_motor()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert the motor controllers internal
// step registers to a 'real' RA and Dec values.
// RA and DEC are returned in deciHours and deciDegs
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
// <RNS> Commented out due to -Wall whining about a static function not being used
// static void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec)
void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec)
{
	double currPos;
	long double time;
	double home;

	// convert RA position from steps to decimal deg
	currPos = (gMountConfig.ra.direction) * (raStep * (1.0 / gMountConfig.ra.step)) / 3600.0;

	// compute the current home position by adding the movement of the earth since
	// saving the home position

	// get the current time and compute the elapsed time
	time = Time_get_systime();
	time = time - gMountConfig.ra.time;

	// compute the current home position with time delta, motor direction
	// should not be needed (I think) since time can't go backwards ;^)
	// home	=	gMountConfig.ra.zero + (gMountConfig.ra.direction) * (gSysDepend * (double)time * ARCSEC_PER_SEC / 3600.0);
	home = gMountConfig.ra.zero + (gSysDepend * (double)time * kARCSEC_PER_SEC / 3600.0);
	*ra = home + currPos;

	// convert from decidegs to decihours
	Time_deci_deg_to_hours(ra);

	// convert Dec position from steps to decimal deg
	currPos = (gMountConfig.dec.direction) * (decStep * (1.0 / gMountConfig.dec.step)) / 3600.0;

	// get the 'true' position with home offset in decidegs
	*dec = gMountConfig.dec.zero + currPos;

} // of Servo_step_to_pos()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert ra and dev values in decihour and decidegs
// to MC step positions. It does zero checks for meridian, flip/TTP, etc
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
static void Servo_pos_to_step(double ra, double dec, int32_t *raStep, int32_t *decStep)
{
	double raDelta, decDelta;
	double time;
	double home;

	// Convert RA to decidegs
	Time_deci_hours_to_deg(&ra);

	// if a RA tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since last Servo_set_pos() call
	if (gMountConfig.ra.track != 0)
	{
		// get the current time and compute the elapsed time
		time = Time_get_systime();
		time = time - gMountConfig.ra.time;

		// compute the current home position with time delta
		home = gMountConfig.ra.zero + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home = gMountConfig.ra.zero;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.zero value corresponds to zero step position
	// TODO: Potential error due to 0/24 rollover comparison, need to convert to HA?
	raDelta = ra - home;

	// Convert from decidegs to arcsecs
	raDelta *= 3600.0;

	// get the actual RA steps needed in absolute steps, not relative offset
	*raStep = (int32_t)((gMountConfig.ra.direction) * (raDelta * gMountConfig.ra.step));

	// if a Dec tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since last Servo_set_pos() call
	if (gMountConfig.dec.track != 0)
	{
		// get the current time and compute the elapsed time
		time = Time_get_systime();
		time = time - gMountConfig.dec.time;

		// compute the current home position with time delta
		home = gMountConfig.dec.zero + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home = gMountConfig.dec.zero;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.zero value corresponds to zero step position
	decDelta = dec - home;

	// Convert from decidegs to arcsecs
	decDelta *= 3600.0;

	// get the actual Dec steps needed in absolute steps, not relative offset
	*decStep = (int32_t)((gMountConfig.dec.direction) * (decDelta * gMountConfig.dec.step));

	return;
} // of Servo_pos_to_step()

//*****************************************************************************
// Returns the current position of the mount in RA and Dec values.
// RA and DEC are returned in deci hours and deci degs
// TODO: Will need major mind-numbing changes to support alt-azi
// TODO: will need to add tracking delta for Dec for alt-azi
//*****************************************************************************
void Servo_get_pos(double *ra, double *dec)
{
	int32_t currStep;
	double currPos;
	long double time;
	double home;
	double delta = 0.0;

	// Get the current RA position in steps
	RC_get_curr_pos(gMountConfig.addr, SERVO_RA_AXIS, &currStep);

	// convert RA step position to arcsec
	currPos = (gMountConfig.ra.direction) * (currStep / gMountConfig.ra.step);
	// Convert from arcsecs to decidegs
	currPos /= 3600.0;

	// if a Ra tracking rate has been set then add in the time delta
	if (gMountConfig.ra.track != 0)
	{
		// compute the current home position by adding the movement of the earth since
		// saving the home position

		// get the current time and compute the elapsed time
		time = Time_get_systime();
		time -= gMountConfig.ra.time;

		// compute the current home position with time delta
		// direction not needed, time only flows one way
		// TODO: need to add actual tracking rate and not always sidereal
		delta = (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
		home = gMountConfig.ra.zero + delta;
	}
	else
	{
		// no tracking rate then delta is not needed.
		home = gMountConfig.ra.zero;
	}

	// add current pos + home position to get "real" value
	*ra = home + currPos;

	// convert to decihours
	// TODO: RA could be out-of-range 0/24
	Time_deci_deg_to_hours(ra);

	// TODO:  Missing check for Dec tracking needed for Alt-azl
	// get the current dec position in steps
	RC_get_curr_pos(gMountConfig.addr, SERVO_DEC_AXIS, &currStep);

	// convert position from steps to arcsecs
	currPos = (gMountConfig.dec.direction) * (currStep / gMountConfig.dec.step);
	// Convert from arcsecs to decidegs
	currPos /= 3600.0;

	// get the "real" position with home offset
	*dec = gMountConfig.dec.zero + currPos;

	return;
} // Servo_get_pos()

//*****************************************************************************
// Sets the mount position to supplied RA and Dec values. Input RA and Dec
//	are in deciHour and deciDegs respectively. Assume the mount is
//	tracking more or less correctly. (ie. the mount is not stopped)
//	Both RA/Dec home position are stored in deci degrees and the
//	MC's set_home command always sets the encoder/position value to zero
//*****************************************************************************
void Servo_set_pos(double ra, double dec)
{
	// set home on the RA axis (deci degs)
	gMountConfig.ra.time = Time_get_systime();
	Time_deci_hours_to_deg(&ra);
	gMountConfig.ra.zero = ra;
	RC_set_home(gMountConfig.addr, SERVO_RA_AXIS);

	// set home on the DEC axis (deci degs)
	gMountConfig.dec.time = Time_get_systime();
	gMountConfig.dec.zero = dec;
	RC_set_home(gMountConfig.addr, SERVO_DEC_AXIS);
} // Servo_set_pos()

//*****************************************************************************
// Sets the position for a stopped mount. Since the mount is not moving,
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
	jd = Time_systime_to_jd();
	lst = Time_jd_to_gmst(jd);
	lst = Time_gmst_to_lst(lst, Time_get_lon());

	// set RA to the LST + HA offset
	ra = lst - ha;

	// Set the current position
	Servo_set_pos(ra, dec);
} // of Servo_set_static_pos()

//*****************************************************************************
// INTERNAL ROUTINE: Returns the current tracking rate in motor controller axis
//	in steps/sec
//*****************************************************************************
int32_t Servo_get_axis_step_track(uint8_t motor)
{
	int32_t curTrackingRate;

	curTrackingRate = 0;
	switch (motor)
	{
	case SERVO_RA_AXIS:
		curTrackingRate = gMountConfig.ra.track;
		break;

	case SERVO_DEC_AXIS:
		curTrackingRate = gMountConfig.dec.track;
		break;

	default:
		// do nothing
		break;
	} // of switch

	return (curTrackingRate);
} // of Servo_get_step_track()

//*****************************************************************************
// INTERNAL ROUTINE: Sets the tracking rate in motor controller axis
//	in steps/sec but does not start tracking
// TODO: the minus sign in the equate is likely wrong
//*****************************************************************************
int Servo_set_axis_step_track(uint8_t motor, int32_t tracking)
{
	double ra, dec;
	int status = kSTATUS_OK;

	// Set the current pos with the current tracking rate but only if the step
	// sizes for RA and Dec have been set during Servo_init
	if ((gMountConfig.ra.step != 0.0) && (gMountConfig.dec.step != 0.0))
	{
		// Get and set the position to set begin timestamp for new tracking rate
		Servo_get_pos(&ra, &dec);
		Servo_set_pos(ra, dec);

		switch (motor)
		{
		case SERVO_RA_AXIS:
			gMountConfig.ra.track = (int32_t)(-gMountConfig.ra.direction * gSysDepend * tracking);
			break;

		case SERVO_DEC_AXIS:
			gMountConfig.dec.track = (int32_t)(-gMountConfig.dec.direction * gSysDepend * tracking);
			break;

		default:
			// do nothing
			status = kERROR;
			break;
		} // of switch
	}
	else
	{
		// Mount was not initialized or failed Servo_init
		status = kERROR;
	}

	return (status);
} // of Servo_set_step_track()

//*****************************************************************************
// Start the motor tracking if a tracking rate has been set in the
// axisPtr data struct. this is an unbuffered command and will overwrite any
// pending actions on the axis
//*	returns kSTATUS_OK on success, kERROR otherwise
//*****************************************************************************
int Servo_start_axis_tracking(uint8_t motor)
{
	int status;

	CONSOLE_DEBUG(__FUNCTION__);
	status = kERROR;

	switch (motor)
	{
	case SERVO_RA_AXIS:
		// Start velocity move with the axis tracking rate if not zero
		if (gMountConfig.ra.track != 0)
		{
			status = RC_move_by_vela(gMountConfig.addr,
									 SERVO_RA_AXIS,
									 gMountConfig.ra.track,
									 gMountConfig.ra.acc,
									 false);
		}
		break;

	case SERVO_DEC_AXIS:
		// Start velocity move with the axis tracking rate if not zero
		if (gMountConfig.dec.track != 0)
		{
			status = RC_move_by_vela(gMountConfig.addr,
									 SERVO_DEC_AXIS,
									 gMountConfig.dec.track,
									 gMountConfig.dec.acc,
									 false);
		}
		break;

	default:
		// do nothing
		status = kERROR;
		break;
	} // of switch

	return (status);
} // of Servo_start_tracking()

//*****************************************************************************
// Stops the motor. Existing tracking rate value, if set, is not affected
// returns kSTATUS_OK on success, or kERROR otherwise
//*****************************************************************************
int Servo_stop_axis_tracking(uint8_t motor)
{
	int status;

	CONSOLE_DEBUG(__FUNCTION__);
	status = kERROR;
	switch (motor)
	{
	case SERVO_RA_AXIS:
		// Stop the RA motor
		status = RC_stop(gMountConfig.addr, SERVO_RA_AXIS);
		break;

	case SERVO_DEC_AXIS:
		// Stop the Dec motor
		status = RC_stop(gMountConfig.addr, SERVO_DEC_AXIS);
		break;

	default:
		CONSOLE_DEBUG("Invalid motor specified!!!!!!!!!!!!!!!!");
		// do nothing
		status = kERROR;
		break;
	} // of switch
	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return (status);
} // Servo_start_tracking()

//*****************************************************************************
// Move the axis indefinately at a rate in degs/sec specfied by the argument.
// Rate is limited by the axis .slew field which is the max hand control speed
//*	returns kSTATUS_OK on success, or kERROR otherwise
//*****************************************************************************
int Servo_move_axis_by_vel(uint8_t motor, double vel)
{
	int status;
	int32_t stepVel;
	TYPE_MountAxis *currAxis = NULL;

	CONSOLE_DEBUG(__FUNCTION__);
	status = kSTATUS_OK;
	// figure out which axis we are working with and assign to currAxis
	switch (motor)
	{
	case SERVO_RA_AXIS:
		currAxis = &gMountConfig.ra;
		break;

	case SERVO_DEC_AXIS:
		currAxis = &gMountConfig.dec;
		break;

	default:
		CONSOLE_DEBUG("Invalid motor specified!!!!!!!!!!!!!!!!");
		// do nothing
		status = kERROR;
		break;
	} // of switch

	if (status != kERROR)
	{
		// Convert the input vel in degs/sec rate to step vel measured in steps/sec
		vel *= 3600.0; // first convert input vel into arcsecs/sec
		// Set velocity direction based on axis cfg, divide by the axis->steps to get final vel in steps
		stepVel = (int32_t)(vel * currAxis->direction / currAxis->step);
		status = RC_move_by_vela(gMountConfig.addr, motor, stepVel, currAxis->acc, false);
	}

	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return (status);
} // Servo_move_by_vel()

//*****************************************************************************
// This is global initialization routine for the Servo system. It read the both
// configuration files, initializes communication with the MC board and sets all
// the motion parameters. Then it initializes the mount's current position to Park
// values from config fields, set the encoder home position and starts keeping track
// of any tracking rate positions changes but does not start tracking motion.
// Note: calling this routine will reset the scaling on both the RA and
// Dec axes to 100%, scale_val() and scale_acc()
// It assumes the mount is at it's PARK state and will set gParkState to true
// Return Values: kSTATUS_OK or kERROR
//*****************************************************************************
int Servo_init(const char *mountCfgFile, const char *localCfgFile)
{
	int status = kSTATUS_OK;
	double currRa, currDec; 

	CONSOLE_DEBUG(__FUNCTION__);

	// Read the location config file for the mount location and check status
	status = Time_read_local_cfg(localCfgFile);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open mount location file '%s'.\n", localCfgFile);
		return (kERROR);
	}

	// Read the mount config file for the mount physical characteristics
	memset((void *)&gMountConfig, 0, sizeof(TYPE_MOUNT_CONFIG));
	status = Servo_read_mount_cfg(mountCfgFile, &gMountConfig);

	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open mount configuration file '%s'.\n", mountCfgFile);
		return (kERROR);
	}

	// Initialize the port from the mount config file for communication
	status = MC_init_comm(gMountConfig.port, gMountConfig.baud);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not initialize comm port '%s'.\n", gMountConfig.port);
		return (kERROR);
	}

	// convert drive precession for *friction drives* from arcsec/deg to percents only - this term will likely just be 1.0
	gMountConfig.ra.prec = 1.0 - (gMountConfig.ra.prec / 3600.0);

	// calc encoder steps per arcsec
	gMountConfig.ra.step = (gMountConfig.ra.motorGear * gMountConfig.ra.mainGear * gMountConfig.ra.encoder * gMountConfig.ra.prec) / 1296000.0;

	// calc the max encoder speed in counts per sec (max CPS) based on max motor RPM and encoder resolution
	gMountConfig.ra.encoderMaxSpeed = gMountConfig.ra.motorMaxRPM * gMountConfig.ra.encoder / 60.0;

	// Max accel is the acc in arcsec/sec^2 from the config file
	gMountConfig.ra.maxAcc = (uint32_t)gMountConfig.ra.realAcc * gMountConfig.ra.step;
	gMountConfig.ra.acc = gMountConfig.ra.maxAcc;

	//*	Initialize the encoder count to zero
	gMountConfig.ra.pos = 0;

	// Max velocity in arcsec/sec from the config file and converted to step/sec
	gMountConfig.ra.maxVel = (uint32_t)gMountConfig.ra.realVel * gMountConfig.ra.step;
	gMountConfig.ra.vel = gMountConfig.ra.maxVel;

	// Calc the adjust and slew speeds by multiplying by the mount's steps / arcsec ratio
	gMountConfig.ra.adj = (uint32_t)gMountConfig.ra.realAdj * gMountConfig.ra.step;
	gMountConfig.ra.slew = (uint32_t)gMountConfig.ra.realSlew * gMountConfig.ra.step;

	gMountConfig.ra.status = 0;
	gMountConfig.ra.direction = gMountConfig.ra.config;

	// TODO: BUG!  routine set the position for both ra & dec, but dec is not yet initialized, see below
	gMountConfig.ra.track = -kARCSEC_PER_SEC * gMountConfig.ra.direction * gSysDepend * gMountConfig.ra.step;

	// Servo_set_axis_step_track(SERVO_RA_AXIS, (kARCSEC_PER_SEC * (double)gMountConfig.ra.step));
	gMountConfig.ra.zero = 0.0;
	gMountConfig.ra.time = Time_get_systime();

	// Duplicate of what's already done for RA, comments above
	gMountConfig.dec.prec = 1.0 - (gMountConfig.dec.prec / 3600.0);
	gMountConfig.dec.step = (gMountConfig.dec.motorGear * gMountConfig.dec.mainGear * gMountConfig.dec.encoder * gMountConfig.dec.prec) / 1296000.0;
	gMountConfig.dec.encoderMaxSpeed = gMountConfig.dec.motorMaxRPM * gMountConfig.dec.encoder / 60.0;
	gMountConfig.dec.maxAcc = (uint32_t)gMountConfig.dec.realAcc * gMountConfig.dec.step;
	gMountConfig.dec.acc = gMountConfig.dec.maxAcc;
	gMountConfig.dec.pos = 0;
	gMountConfig.dec.maxVel = (uint32_t)gMountConfig.dec.realVel * gMountConfig.dec.step;
	gMountConfig.dec.vel = gMountConfig.dec.maxVel;
	gMountConfig.dec.adj = (uint32_t)gMountConfig.dec.realAdj * gMountConfig.dec.step;
	gMountConfig.dec.slew = (uint32_t)gMountConfig.dec.realSlew * gMountConfig.dec.step;
	gMountConfig.dec.status = 0;
	gMountConfig.dec.direction = gMountConfig.dec.config;

	gMountConfig.dec.track = 0;
	gMountConfig.dec.zero = 0.0;
	gMountConfig.dec.time = Time_get_systime();

	// Make sure the "gear lash" is at least 40*axis->step arcseconds for the PID filter resolution
	if (gMountConfig.ra.gearLash < (gMountConfig.ra.step * 40.0))
	{
		gMountConfig.ra.gearLash = gMountConfig.ra.step * 40.0;
	}
	if (gMountConfig.dec.gearLash < (gMountConfig.dec.step * 40.0))
	{
		gMountConfig.dec.gearLash = gMountConfig.dec.step * 40.0;
	}

	// Convert the global off target tolerance from arcseconds to deci degrees
	gMountConfig.offTarget /= 3600.0;

	// Set the position of the stationary mount to the park position from config file
	printf("ra.park = %lf	dec.park = %lf\n",gMountConfig.ra.park, gMountConfig.dec.park);
	Servo_set_static_pos(gMountConfig.ra.park, gMountConfig.dec.park);
	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	// Set the mount state to Park
	gParkState = true;

#ifdef _ALPHA_OUT_
	// commented out due to not sensor support for Alpha
	// Find out the initial sensor status for RA and DEC
	ss__read_sensors(&raSensor, &decSensor);

	// Check the current sensor status for RA and Dec
	if (gMountConfig.ra.syncValue == raSensor)
	{
		gMountConfig.ra.syncError = false;
	}
	elsefor Roboclaw, need to set the "zero" position to middle of the unsigned position range
	{

		printf("(ss__init) RA sensor value on initialization does not\n");
		printf("match RA_HOME_FLAG value in the mount configuration file\n");
		gMountConfig.ra.syncError = TRUE;
		printf("< Hit 'RETURN' to continue... >\n");
		ch = getch();
	}

	statout("Made it 4\n");

	if (gMountConfig.dec.syncValue == decSensor)
	{
		gMountConfig.dec.syncError = false;
	}
	else
	{
		printf("(ss__init) Dec sensor value on initialization does not\n");
		printf("match DEC_HOME_FLAG value in the mount configuration file\n");
		gMountConfig.dec.syncError = TRUE;
		printf("< Hit 'RETURN' to continue... >\n");
		ch = getch();
	}
#endif // _ALPHA_OUT_

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of ss__init()

//*****************************************************************************
// Returns whether the mount is PARKED, STOPPED, MOVING or TRACKING using typedef TYPE_MOVE
// with the above fields. For STOPPED both axis must be stopped and if it's stopped,
// it may be parked if the global state gParkState is set to true. If either axis is
// moving, then MOVING is returned and has a high precedence than TRACKING.
// If the mount is tracking on either or both axes, TRACKING is returned.
// Tracking is checked against the axis.track value must be withiin 10%
//*****************************************************************************
TYPE_MOVE Servo_state(void)
{
	uint8_t raState, decState;
	int32_t raVel, decVel;
	uint32_t absVel, absTrack;
	float velFloat, trackFloat;
	int status;
	// bool		raStopped, decStopped;

	// CONSOLE_DEBUG(__FUNCTION__);

	// read the command queue for RA and Dec
	status = RC_check_queue(gMountConfig.addr, &raState, &decState);
	if (status != kSTATUS_OK)
	{
		CONSOLE_DEBUG("ERROR:  RC_check_queue returned error!");
	}
	// Go for the easy win, just see if both cmds queues are empty
	// printf("\n *** ra state = %x	dec state = %x ***\n", raState, decState);

	if ((raState & decState) == kRC_CMD_QUEUE_EMPTY)
	{
		// Check to see is the mount is PARKED or just STOPPED
		if (gParkState == true)
		{
			// printf("### Mount is parked\n");
			strcpy(gDebugInfoSS, "P1");
			return (PARKED);
		}
		else
		{
			// IMPORTANT: This conditional moves the mount to the PARKED state
			if (gMountAction == PARKING)
			{
				// Mount action is PARKING, but the mount is now STOPPED
				// therefore it just completed PARKING and need to return PARKED
				gParkState = true;
				// reset mount action to PARKED as well
				gMountAction = PARKED;
				strcpy(gDebugInfoSS, "P2");
				return (PARKED);
			}
			else
			{
				// Otherwise, it's just stopped
				strcpy(gDebugInfoSS, "S1");
				return (STOPPED);
			}
		}
	}

	// if cmd queues have more than 1 cmd queued in either buffer (0-based), then definitely moving
	// This mask/compare is tricky and only works due to the above 'if' eliminated double  0x80 case
	if (((raState | decState) & 0x3F) != 0x0)
	{
		// printf("\n *** ra state = %x	dec state = %x ***\n", raState, decState);
		strcpy(gDebugInfoSS, "M1");
		return (MOVING);
	}
	// CONSOLE_DEBUG(__FUNCTION__);

	// one or both axis have single cmd in queue, determining moving vs tracking

	// 2nd easy case, dec is executing a command and dec is not supposed to be tracking
	// which is very likely true for any mount other than alt-azi
	if ((decState != kRC_CMD_QUEUE_EMPTY) && gMountConfig.dec.track == 0)
	{
		// Therefore dec is moving
		strcpy(gDebugInfoSS, "M2");
		return (MOVING);
	}
	//	CONSOLE_DEBUG("If you get to here, it means the dec is not moving, but could be tracking");

	// If you get to here, it means the dec is not moving, but could be tracking

	// But check RA first, highly likely .track != 0, if any axis is tracking, then the mount
	// is tracking.  So read the current mount velocity for RA to compare to tracking rate
	RC_get_curr_velocity(gMountConfig.addr, SERVO_RA_AXIS, &raVel);

	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel = abs(raVel);
	absTrack = abs(gMountConfig.ra.track);
	// printf("^^^ absVel = %d	absTrack = %d\n", absVel, absTrack);

	// convert to floats
	velFloat = (float)absVel;
	trackFloat = (float)absTrack;

	// if the RA axis velocity is +/- 20% of the track velocity, then assume RA is tracking
	// TODO: KLUDGE!  Tighten down the PID to get a better tolerance than 40%
	if (velFloat > (trackFloat * 0.6) && velFloat < (trackFloat * 1.4))
	{
		if (gMountConfig.ra.track != 0)
		{
			strcpy(gDebugInfoSS, "T1");
			return (TRACKING);
		}
	}

	// Check Dec for tracking (could be an alt-azi, but do not rely on that)
	RC_get_curr_velocity(gMountConfig.addr, SERVO_DEC_AXIS, &decVel);
	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel = abs(decVel);
	absTrack = abs(gMountConfig.dec.track);
	// convert to floats
	velFloat = (float)absVel;
	trackFloat = (float)absTrack;

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
		if (gMountConfig.dec.track != 0)
		{
			strcpy(gDebugInfoSS, "T2");
			return (TRACKING);
		}
	}
	// Not tracking on either axis but since cmd queue is not empty on at least
	// one axis the mount must be moving
	strcpy(gDebugInfoSS, "M3");
	return (MOVING);
} // of Servo_state()

//*****************************************************************************
// INTERNAL ROUTINE: Moves to the desired RA and Dec step position plus an addition
// distance to compensate for slew time if tracking is set on the axis. This does
// not wait or check for completion. This is a RC buffered command and will clear
// any current and commands pending. This RC command is buffered so
// completion can be checked with check_queue() where 0x80 equals complete
// or the higher level function Servo_state() looking for STOPPED or TRACKING
// If the track parameter (velocity coefficient) is non-zero for either axis
// the routine will start tracking on that axis(axes).
// TODO: May need to pull the calc_move_time() out of this routine for alt-azi
//*****************************************************************************
static int Servo_move_step_track(int32_t raStep, int32_t decStep)
{
	double duration;
	int32_t pos;
	int status = kSTATUS_OK;

	// If RA axis has a tracking value
	if (gMountConfig.ra.track != 0)
	{
		// figure out where we are now
		status -= RC_get_curr_pos(gMountConfig.addr, SERVO_RA_AXIS, &pos);
		// calculate the duration of the axis move and convert to step and add to target
		duration = RC_calc_move_time(pos, raStep, gMountConfig.ra.vel, gMountConfig.ra.acc);
		raStep += (uint32_t)(duration * (double)gMountConfig.ra.track);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)
		status -= RC_move_by_posva(gMountConfig.addr, SERVO_RA_AXIS, raStep, gMountConfig.ra.vel, gMountConfig.ra.acc, false);
		// printf("!!! raStep = %d  vel = %d  acc = %d\n", raStep, gMountConfig.ra.vel, gMountConfig.ra.acc);
		//  This velocity command will start when the above pos cmd completes
		status -= RC_move_by_vela(gMountConfig.addr, SERVO_RA_AXIS, gMountConfig.ra.track, gMountConfig.ra.acc, true);
	}
	else
	{
		status -= RC_move_by_posva(gMountConfig.addr, SERVO_RA_AXIS, raStep, gMountConfig.ra.vel, gMountConfig.ra.acc, false);
	}

	// If Dec axis has a tracking value
	if (gMountConfig.dec.track != 0)
	{
		// figure out where we are now
		status -= RC_get_curr_pos(gMountConfig.addr, SERVO_DEC_AXIS, &pos);
		// calculate the duration of the axis move and convert to step and add to target
		duration = RC_calc_move_time(pos, decStep, gMountConfig.dec.vel, gMountConfig.dec.acc);
		decStep += (uint32_t)(duration * (double)gMountConfig.dec.track);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)
		status -= RC_move_by_posva(gMountConfig.addr, SERVO_DEC_AXIS, decStep, gMountConfig.dec.vel, gMountConfig.dec.acc, false);
		// This velocity command will start when the above pos cmd completes
		status -= RC_move_by_vela(gMountConfig.addr, SERVO_DEC_AXIS, gMountConfig.dec.track, gMountConfig.dec.acc, true);
	}
	else
	{
		status -= RC_move_by_posva(gMountConfig.addr, SERVO_DEC_AXIS, decStep, gMountConfig.dec.vel, gMountConfig.dec.acc, false);
	}

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_step_track()

//*****************************************************************************
// INTERNAL ROUTINE: Moves to the desired RA and Dec step position and does
// not wait or check for completion. This is a RC buffered command and will clear
// any current and commands pending. This RC command is buffered so
// completion can be checked with check_queue() where 0x80 equals complete
// or the higher level function Servo_state() looking for STOPPED
// This routine *WILL NOT* start tracking even if the track parameters are
// non-zero for either axis
//*****************************************************************************
static int Servo_move_step(int32_t raStep, int32_t decStep, bool buffered)
{
	int status;
	// Start the Dec move
	status = RC_move_by_posva(gMountConfig.addr, SERVO_DEC_AXIS, decStep, gMountConfig.dec.vel, gMountConfig.dec.acc, buffered);

	// Start the RA move and decrement status if error returned
	status -= RC_move_by_posva(gMountConfig.addr, SERVO_RA_AXIS, raStep, gMountConfig.ra.vel, gMountConfig.ra.acc, buffered);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_step()

//*****************************************************************************
// INTERNAL ROUTINE: This routine translates the current position before
// "rolling over" for flipping the mount in overwrite input values.
// Input are in deciHours and deciDegs
// It *DOES NOT* move the mount, but just calcs and returns the flip values
//  Used on German and Split Ring mounts or on fork mounts to 'go thru the pole'
// TODO: Need to check the comment-out section to determine if it safe for from optimal_path and zero change
//*****************************************************************************
static void Servo_calc_flip_coordins(double *ra, double *dec, double *direction, int8_t *side)
{
	// check to see which side the mount is on
	if (*side == kEAST)
	{
		*ra += 12.0;
		*side = kWEST;
	}
	else
	{
		*ra -= 12.0;
		*side = kEAST;
	}
	// check range
	Time_normalize_RA(ra);

	// complement Dec and reverse direction
	*dec = 180.0 - *dec;

	// toggle dec motion direction
	*direction = (*direction < 0) ? kREVERSE : kFORWARD;
}

//*****************************************************************************
// Stops both motors
//*****************************************************************************
void Servo_stop_all(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	// stop the motors
	RC_stop(gMountConfig.addr, SERVO_RA_AXIS);
	RC_stop(gMountConfig.addr, SERVO_DEC_AXIS);
}

//*****************************************************************************
// This routine will find the shortest path between two points on a circle
// Circle size is supplied by the 'max' arg, and for RA it is 24.0
// It is only used the RA axis (Dec doesn't need it) and will return the
// vector needed to move from the start position in hour.  This was a PITA
//*****************************************************************************
static double Servo_calc_short_vector(double begin, double end, double max)
{
	double sDiff;
	double rawDiff;
	double modDiff;

	sDiff = 0.0;
	rawDiff = (begin > end) ? begin - end : end - begin;
	// set the rollover value from the args
	modDiff = fmod(rawDiff, max);

	// Split the circle and pick the shorter length
	if (modDiff > (max / 2.0))
	{
		// There is a shorter path in opposite direction
		sDiff = (max - modDiff);
		// if end larger toggle the sign
		if (end > begin)
		{
			sDiff = sDiff * -1.0;
		}
	}
	else
	{
		sDiff = modDiff;
		// if begin larger toggle the sign
		if (begin > end)
		{
			sDiff = sDiff * -1.0;
		}
	}

	return sDiff;
}

//*****************************************************************************
// INTERNAL ROUNTINE:  This is try-before-you-buy routine. It calcs the best path
// for the mount to from start to end and returns the *relative direction* needed
// to move the mount from the start position but makes *no changes*.  If you don't
// like the path chosen, don't use it and no values will have changed. It assumes
// that end coordins are above the horizon. It also will return whether a flip
// is needed, but *does not* make the flip coordins changes to the start position.
// If a flip is returned, you *MUST* flip the start coordins for the returned path
// in raDirection & decDirection to be accurate. Note: raPath(Flip) vector is
// measured in HA which is the opposite of the RA direction, so need to toggle the
// sign on the return for relative RA movement
// All inputs in decimal Hours/degs format and returns boolean if a mount flip is required
//*****************************************************************************
static bool Servo_calc_optimal_path(double startRa, double startDec, double lst, double endRa, double endDec, double *raDirection, double *decDirection)
{
	double startRaHa, endRaHa;
	double startRaFlip, startDecFlip, startRaHaFlip;
	double decStdPath, decFlipPath, raStdPath, raFlipPath;
	double direction; // not used
	int8_t side;	  // not used

	// Converting the input star and end coordins to hour angle
	startRaHa = lst - startRa;
	endRaHa = lst - endRa;
	Time_normalize_HA(&startRaHa);
	Time_normalize_HA(&endRaHa);

	// Do the simple Dec axis first, it just path = end - start;
	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decStdPath = endDec - startDec;
	// printf(> decStdPath(%.2lf)  endDec(%.2lf)  startDec(%.2lf)\n", decStdPath, endDec, startDec);

	// Find the short path for the Ra axis for a std move
	raStdPath = Servo_calc_short_vector(startRa, endRa, 24.0);
	// printf("LST:%.2lf - StartRa = %.2lf  endRa = %.2lf  raStdPath = %.2lf  startRaHa = %.2lf  endRaHa = %.2lf\n", lst, startRa, endRa, raStdPath, startRaHa, endRaHa);

	// if starting on the west side and std path moves east past LST
	if (startRaHa > 0.0 && (startRaHa - raStdPath) < 0.0)
	{
		// this move crosses meridian
	}
	// if starting on the east side and std path moves west past lst
	else if (startRaHa < 0.0 && (startRaHa - raStdPath) > 0.0)
	{
		// this move crosses meridian
	}
	else
	{
		// this move stays on it's side of the meridian
		*raDirection = raStdPath;
		*decDirection = decStdPath;
		strcpy(gDebugInfoCOP, "X--SM");
		return (false);
	}

	// Now that crosses meridian is true, but lets see if we can change that with a flip
	// so do a what-if compare of distance with start position flipped for TTP move
	// TODO:  This move could be merged with the GEM flip routine, but just get it working
	startRaFlip = startRa;
	startDecFlip = startDec;
	Servo_calc_flip_coordins(&startRaFlip, &startDecFlip, &direction, &side);
	// printf("LST:%.2lf - StartRaFlip = %.2lf	endRa = %.2lf	raFlippath = %.2lf startRaHaFlip = %.2lf\n", lst, startRaFlip, endRa, raFlipPath, startRaHaFlip);

	// Calc the the HA of the new flipped start position, endRaHa remain the same
	startRaHaFlip = lst - startRaFlip;
	Time_normalize_HA(&startRaHaFlip);

	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decFlipPath = endDec - startDecFlip;

	// Find the short path of the Ra axis for a flip move, endRa is same as std path
	raFlipPath = Servo_calc_short_vector(startRaFlip, endRa, 24.0);
	// printf("LST:%.2lf - StartRaFlip = %.2lf	endRa = %.2lf	raFlippath = %.2lf startRaHaFlip = %.2lf\n", lst, startRaFlip, endRa, raFlipPath, startRaHaFlip);

	// if starting on the west side and std path moves past lst
	if (startRaHaFlip > 0.0 && (startRaHaFlip - raFlipPath) < 0.0)
	{
		// this move crosses meridian
	}
	// if starting on the east side and std path moves past lst
	else if (startRaHaFlip < 0.0 && (startRaHaFlip - raFlipPath) > 0.0)
	{
		// this move crosses meridian
	}
	else
	{
		// After the flip this move stays on it's side of the meridian
		*raDirection = raFlipPath;
		*decDirection = decFlipPath;
		strcpy(gDebugInfoCOP, "XF-SM");
		return (true);
	}

	// OK, now we need to cross the meridian, do not pass 'go', do not collect $200 ;^)

	// Check to see if a GEM move goes past the meridian flip window
	if (gMountConfig.mount == kGERMAN)
	{
		// Check to see if a GEM mount must flip due to the RA path exceed the meridian window

		// if starting on the west side and std path moves east past LST
		if ((startRaHa > 0.0) && ((startRaHa - raStdPath) < gMountConfig.flipWin))
		{
			// this move crosses meridian from the west and goes past the GEM's meridian flip window, so flip and return
			*raDirection = raFlipPath;
			*decDirection = decFlipPath;
			strcpy(gDebugInfoCOP, "GFWMW");
			return (true);
		}
		// if starting on the east side and std path moves west past LST
		else if ((startRaHa < 0.0) && ((startRaHa - raStdPath) > gMountConfig.flipWin))
		{
			// this move crosses meridian from the east and goes past the GEM's meridian flip window, so flip and return
			*raDirection = raFlipPath;
			*decDirection = decFlipPath;
			strcpy(gDebugInfoCOP, "GFEMW");
			return (true);
		}
		else
		{
			// the move does cross the meridian but stay within the meridian flip window, no flip needed
			*raDirection = raStdPath;
			*decDirection = decStdPath;
			strcpy(gDebugInfoCOP, "GF<MW");
			return (false);
		}
	}
	// If you got here, the *mount is NOT a GEM* and need to find the shortest path for a FORK

	// Check to see of the RA axis moves at least 10% less on the RA flip path vs RA path
	if (fabs(raFlipPath) < (fabs(raStdPath) * 0.9))
	{
		// the flip is shorter than the 'normal' path and will cross the meridian
		*raDirection = raFlipPath;
		*decDirection = decFlipPath;
		strcpy(gDebugInfoCOP, "FT-CM");
		return (true);
	}

	// This is just FORK std path move that crosses the meridian
	*raDirection = raStdPath;
	*decDirection = decStdPath;
	strcpy(gDebugInfoCOP, "F--CM");
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
		currRaStep += (uint32_t)(gMountConfig.ra.direction * UNDER_TARGET * gMountConfig.ra.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currRaStep -= (uint32_t)(gMountConfig.ra.direction * UNDER_TARGET * gMountConfig.ra.step);
	}
	// Take care of the Dec axis
	if (ss__get_tracking(SERVO_DEC_AXIS) >= 0)
	{
		// tracking is in the positive direction come in on the negative side of the target
		currDecStep += (uint32_t)(gMountConfig.dec.direction * UNDER_TARGET * gMountConfig.dec.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currDecStep -= (uint32_t)(gMountConfig.dec.direction * UNDER_TARGET * gMountConfig.dec.step);
	}
}
#endif

//*****************************************************************************
// This is the a simple spiral move routine and is a SYNCRONOUS CALL. It is
// merely for testing right now and is a prototype for the moves needed to support
// alt-azi mount. It will spiral out 1 to 3 times depending on the arg count
// The routine take in RA deci-hours and Dec and is error checking free very
// primitive routine, so should be used with caution. It will not check for
// flip, TTP or anything else. It's like a joystick, so don't get overconfident.
//*****************************************************************************
int Servo_move_spiral(double raMove, double decMove, int loop)
{
	double ra, dec;
	int count, pos;
	double currRaMove = 0.0;
	double currDecMove = 0.0;
	int32_t raStep, decStep;

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
			// position start -> end
			{-1, 0}, // 0 -> 1
			{0, 1},	 // 1 -> 2
			{1, 0},	 // 2 -> 3
			{1, 0},	 // 3 -> 4
			{0, -1}, // 4 -> 5
			{0, -1}, // 5 -> 6
			{-1, 0}, // 6 -> 7
			{-1, 0}, // 7 -> 8
			{0, 1}	 // 8 -> 1
		};

	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// Make sure the max spiral loop is <= 3
	loop = (loop > 3) ? 3 : loop;

	// get current position
	Servo_get_pos(&ra, &dec);

	// Scale the spiral to 50% first loop, 100% second and 150% third
	for (count = 0; count < loop; count++)
	{
		currRaMove += raMove * 0.5;
		currDecMove += decMove * 0.5;

		// Move the matrix of positions, first "left" then CW
		for (pos = 0; pos < 9; pos++)
		{
			ra += (spiralMat[pos].ra) * currRaMove;
			dec += (spiralMat[pos].dec) * currDecMove;

			// calc the step location from the updated ra/dec
			Servo_pos_to_step(ra, dec, &raStep, &decStep);
			// use buffered moves
			Servo_move_step(raStep, decStep, true);
		}

		// TODO:  Convert to Async call from SYNCRONOUS call
		while (Servo_state() == MOVING)
		{
			sleep(1);
		}
	}
	return kSTATUS_OK;
} // of Servo_move_spiral()

//*****************************************************************************
// This is the main motion routine.  The routine take in RA deci-hours and
// Dec, latitude and longitude in deci-degs.  This routine calls calc_optimal_path()
// that will determine the 'best' path and perform a flip/thru-the-pole if needed
//*****************************************************************************
int Servo_move_to_coordins(double gotoRa, double gotoDec, double lat, double lon)
{
	int32_t targetRaStep, targetDecStep;
	double targetRa, targetDec;
	double raRelDir, decRelDir;
	double currRa, currDec;
	double degsRa;
	double direction;
	double jd, lst;
	double alt, azi;
	int status = kSTATUS_OK;
	bool flip = false;

	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// convert ra hours to decidegs for alt-azi call
	degsRa = gotoRa;
	Time_deci_hours_to_deg(&degsRa);

	// Calc time jd, sid, lst to get alt-azi
	jd = Time_systime_to_jd();
	lst = Time_jd_to_gmst(jd);
	lst = Time_gmst_to_lst(lst, lon);
	// alt-azi is returned in radians
	Time_ra_dec_to_alt_azi(degsRa, gotoDec, lst, lat, &alt, &azi);

	// if object is below the horizon
	if (gIgnoreHorizon == false && alt < 0.0)
	{
		// this is astronomy not geology, so return
		return (kBELOW_HORIZON);
	}

	// get the current position in deci hours and degs
	Servo_get_pos(&currRa, &currDec);

	// Determine the 'best' path form curr position to the goto position
	flip = Servo_calc_optimal_path(currRa, currDec, lst, gotoRa, gotoDec, &raRelDir, &decRelDir);
	printf("\n&&& CurrRa = %lf	CurrDec = %lf 	LST = %lf	flip = %d\n", currRa, currDec, lst, (int)flip);
	printf("&&& gotoRa = %lf	gotoDec = %lf 	raRelDir = %lf	decRelDir = %lf\n", gotoRa, gotoDec, raRelDir, decRelDir);

	// If flip is return, You must flip the mount otherwise the rel dir for RA/Dec will be incorrect
	if (flip == true)
	{
		// Do the actual flip on the current coordins
		// Current coordins are guaranteed to change after this call
		Servo_calc_flip_coordins(&currRa, &currDec, &direction, &gMountConfig.side);
		Servo_set_pos(currRa, currDec);
		printf("&&& After flip CurrRa = %lf	CurrDec = %lf\n\n", currRa, currDec);

		// Compare to the orignal direction values for Dec axix from the mount config file
		if (gMountConfig.side != gMountConfig.ra.parkInfo)
		{
			// set the direction to the opposite from config file
			gMountConfig.dec.direction = (gMountConfig.dec.parkInfo > 0.0) ? kREVERSE : kFORWARD;
		}
		else
		{
			// side equals the value in the config file, so set direction to config file value
			gMountConfig.dec.direction = (double)gMountConfig.dec.parkInfo;
		}
	}

	// determine the new target based on the  RA/Dec relative direction
	targetRa = currRa + raRelDir;
	targetDec = currDec + decRelDir;

	// convert the target RA & Dec to target steps
	Servo_pos_to_step(targetRa, targetDec, &targetRaStep, &targetDecStep);
	// printf("&&& targRa = %lf	targDec = %lf 	targRaStep = %d	targDecStep = %d\n\n", targetRa, targetDec, targetRaStep, targetDecStep);

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:  add back in gearlash,  KLUDGE!
	/////////////////////////////////////////////////////////////////////////////////////

	status -= Servo_move_step_track(targetRaStep, targetDecStep);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_to_coordins()

//*****************************************************************************
// This routine is for stopping the mount for extended periods of time and
// uses and Hour Angle and not RA for an accurate static position.
// Inputs are in deciHour and deciDeg and routine is asynchronous so you need to
// to poll w15ith Servo_state() and wait for STOPPED state
//*****************************************************************************
int Servo_move_to_static(double parkHA, double parkDec)
{
	int32_t targetRaStep, targetDecStep;
	double currRaDelta, currDecDelta;
	double targetRa, targetDec;
	double currRa, currDec;
	double dummy = 0;
	double currHA, targetHA, raRelDir;
	double time, jd, lst;
	double home;
	int status = kSTATUS_OK;

	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	targetHA = parkHA;
	targetDec = parkDec;

	// convert ra deciHours to deciDegs
	Time_deci_hours_to_deg(&targetHA);

	// if the mount is currently thru-the-pole or flipped from config file setting
	if (gMountConfig.side != gMountConfig.ra.parkInfo)
	{
		// Get the current position and flip it
		Servo_get_pos(&currRa, &currDec);
		// Unflip the mount
		Servo_calc_flip_coordins(&currRa, &currDec, &dummy, &gMountConfig.side);

		// Since side now equals the value in the config file, so set direction from config file value
		gMountConfig.dec.direction = (double)gMountConfig.dec.parkInfo;
	}

	// Calc time jd, sid to get LST
	jd = Time_systime_to_jd();
	lst = Time_jd_to_gmst(jd);
	lst = Time_gmst_to_lst(lst, Time_get_lon());

	// calc HA of the current RA position
	currHA = lst - currRa;

	// relative change needed for RA by subtracting current pos HA from the input park RA
	raRelDir = targetHA - currHA;

	// determine the new target based on the  RA/Dec relative direction
	targetRa = currRa + raRelDir;
	// Dec relative calc is not needed since targetDec is just the parkDec

	// Prepare Dec axis for move first (Dec is typically less time sensitive than RA)
	// However, this sensitivity will change when adding alt-azi support

	// if a Dec tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gMountConfig.dec.track != 0)
	{
		// get the current time and compute the elapsed time
		time = Time_get_systime();
		time = time - gMountConfig.dec.time;

		// compute the current home position with time delta
		home = gMountConfig.dec.zero + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home = gMountConfig.dec.zero;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.zero value corresponds to zero step position
	currDecDelta = targetDec - home;

	// get the actual Dec steps needed (absolute)
	targetDecStep = (uint32_t)((gMountConfig.dec.direction) * (currDecDelta * 3600.0 * gMountConfig.dec.step));

	// Prepare RA axis for move last (less change from the systime call)

	// if a RA tracking rate is set then compute the time delta for the offset in deci-degs
	// Note: this only works for a constant velocity value since Servo_set_pos() call
	if (gMountConfig.ra.track != 0)
	{
		// get the current time and compute the elapsed time
		time = Time_get_systime();
		time = time - gMountConfig.ra.time;

		// compute the current home position with time delta
		home = gMountConfig.ra.zero + (gSysDepend * time * kARCSEC_PER_SEC / 3600.0);
	}
	else
	{
		// compute the home position without the time delta
		home = gMountConfig.ra.zero;
	}

	// Determine the degrees difference from target coordins and home position
	// this math only works since axis.zero value corresponds to zero step position
	currRaDelta = targetRa - home;

	// get the actual RA step needed  (absolute)
	targetRaStep = (uint32_t)((gMountConfig.ra.direction) * (currRaDelta * 3600.0 * gMountConfig.ra.step));

	// Check the status and if not zero, decrement
	status -= Servo_move_step(targetRaStep, targetDecStep, false);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_to_static()

//******************************************************************************
// This routine is will park the mount asynchronously.  You must check the
// with Servo_state() to confirm completion of the action and to flip the
// mount the park state to true, needed for the ASCOM AtPark property
// If you don't call Servo_state() after the mount completes parking, Park flag
// will *never* be set.  That would be bad... so just do it. ;^)
//*****************************************************************************
int Servo_move_to_park(void)
{
	double ha, dec;
	int status;

	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// Get the park coordins set in mount config file
	Servo_get_park_coordins(&ha, &dec);
	// Allow a move to anywhere and set the action state, then  move to static pos
	Servo_ignore_horizon(true);
	gMountAction = PARKING;
	status = Servo_move_to_static(ha, dec);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_to_park()

//******************************************************************************
//********************************************************************************************
//* Dump the axisPtr data structure to standard out */
//********************************************************************************************
// static void Servo_print_axis(TYPE_MountAxis *ax)
// {
// 	printf("ax->motorGear = %f\n", ax->motorGear);
// 	printf("ax->mainGear = %f\n", ax->mainGear);
// 	printf("ax->motorMaxRPM = %f\n", ax->motorMaxRPM);
// 	printf("ax->encoder = %f\n", ax->encoder);
// 	printf("ax->realAcc = %f\n", ax->realAcc);
// 	printf("ax->realVel = %f\n", ax->realVel);
// 	printf("ax->realAdj = %f\n", ax->realAdj);
// 	printf("ax->config = %f\n", ax->config);
// 	printf("ax->step = %f\n", ax->step);
// 	printf("ax->prec = %f\n", ax->prec);
// 	printf("ax->encoderMaxSpeed = %d\n", ax->encoderMaxSpeed);
// 	printf("ax->pos = %d\n", ax->pos);
// 	printf("ax->maxAcc = %d\n", ax->maxAcc);
// 	printf("ax->acc = %d\n", ax->acc);
// 	printf("ax->maxVel = %d\n", ax->maxVel);
// 	printf("ax->vel = %d\n", ax->vel);
// 	printf("ax->adj = %d\n", ax->adj);
// 	printf("ax->si = %d\n", ax->si);
// 	printf("ax->kp = %d\n", ax->kp);
// 	printf("ax->ki = %d\n", ax->ki);
// 	printf("ax->kd = %d\n", ax->kd);
// 	printf("ax->il = %d\n", ax->il);
// 	printf("ax->status = %x\n", ax->status);
// 	printf("ax->track = %d\n", ax->track);
// 	printf("ax->direction = %f\n", ax->direction);
// 	printf("ax->park = %f\n", ax->park);
// 	printf("ax->parkInfo = %x\n", (int)ax->parkInfo);
// 	printf("ax->time = %Lf\n", ax->time);
// 	printf("ax->gearLash = %f\n", ax->gearLash);
// 	printf("ax->zero = %f\n", ax->zero);
// 	printf("ax->standby = %f\n", ax->standby);
// 	printf("ax->sync = %f\n", ax->sync);
// 	printf("ax->syncValue = %d\n", ax->syncValue);
// 	printf("ax->syncError = %d\n", ax->syncError);
// }
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
#define _TEST_SERVO_MOUNT_
#ifdef _TEST_SERVO_MOUNT_
int main(void)
{
	//	double ra, dec, ha;
	double parkHa, parkDec;
	double jd, gmst, lst;
	double lat, lon;
	double currRa, currDec;
	int32_t currRaStep, currDecStep;

	int state;
	char buf[256];

	printf("\nSERVO_TEST unit test program\n");
	Servo_init("servo_mount.cfg", "servo_location.cfg");

	Servo_stop_all();

	printf("\nhit any key to begin\n");
	fgets(buf, 256, stdin);

	printf("Checking the _pos_to_step and _step_to_pos routines");
	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	Servo_pos_to_step(currRa, currDec, &currRaStep, &currDecStep);
	printf("** Current Step RA = %d   Dec = %d\n", currRaStep, currDecStep);
	printf("Converting back from steps to position\n");
	Servo_step_to_pos(currRaStep, currDecStep, &currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);

	// printf("Dumping RA settings\n\n");
	// Servo_print_axis(&(gMountConfig.ra));

	// printf("\nhit any key to begin\n");
	// fgets(buf, 256, stdin);

	// printf("Dumping Dec settings\\nn");
	// Servo_print_axis(&(gMountConfig.dec));

	// printf("\nhit any key to begin\n");
	// fgets(buf, 256, stdin);

	Servo_get_park_coordins(&parkHa, &parkDec);
	printf("** Current Park HA = %lf   Dec = %lf\n", parkHa, parkDec);

	jd = Time_systime_to_jd();
	gmst = Time_jd_to_gmst(jd);
	lst = Time_gmst_to_lst(gmst, Time_get_lon());

	currRa = lst - parkHa;
	Time_normalize_RA(&currRa);
	currDec = parkDec;
	printf("after park - curr RA = %lf dec = %lf\n", currRa, currDec);
	Servo_set_pos(currRa, currDec);

	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	Servo_set_pos(currRa, currDec);
	printf("** Setting postion to same value\n");
	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);

	Servo_unpark();

	// Make a small change to Ra and dec for testing
	//	currRa += 0.25;
	//	currDec -= 2.0;

	lat = Time_get_lat();
	lon = Time_get_lon();

	printf("********************************************************\n");
	printf("Move 1: Simple -0.5 hr +5 deg, no meridian crossing at LST:%lf\n", lst);
	printf("********************************************************\n");
	// Reset current RA/Dec back to the park position
	currRa = lst - parkHa;
	Time_normalize_RA(&currRa);
	currDec = parkDec;
	Servo_set_pos(currRa, currDec);
	printf("* Current  Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	currRa -= 0.5;
	currDec += 5;
	;
	printf("* Target Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	printf("********************************************************\n");
	Servo_move_to_coordins(currRa, currDec, lat, lon);

	state = Servo_state();
	while (state == MOVING)
	{
		sleep(3);
		Servo_get_pos(&currRa, &currDec);
		state = Servo_state();
		printf("** Current Pos  RA = %lf   Dec = %lf   gDebugInfoSS = %s\n", currRa, currDec, gDebugInfoSS);
	}

	printf("********************************************************\n");
	printf("Move 2: Simple +2.5 hr +40 deg with meridian crossing at LST:%lf\n", lst);
	printf("********************************************************\n");
	// Reset current RA/Dec back to the park position
	currRa = lst - parkHa;
	Time_normalize_RA(&currRa);
	currDec = parkDec;
	Servo_set_pos(currRa, currDec);
	;
	printf("* Current  Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	currRa += 2.5;
	currDec += 40;
	printf("* Target Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	printf("********************************************************\n");

	Servo_move_to_coordins(currRa, currDec, lat, lon);

	state = Servo_state();
	while (state == MOVING)
	{
		sleep(3);
		Servo_get_pos(&currRa, &currDec);
		state = Servo_state();
		printf("** Current Pos  RA = %lf   Dec = %lf   gDebugInfoSS = %s\n", currRa, currDec, gDebugInfoSS);
	}

	printf("STOPPING motors\n");
	Servo_stop_all();

	return (0);
}
#endif // _TEST_SERVO_MOUNT_
