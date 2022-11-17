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
//*	Redistribution of this source code must retain this copyright notice.
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
//*	May 11,	2022	<MLS> Added Servo_stop_axes_tracking()
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
//*	Jun 12,	2022	<RNS> updated Servo_init() to read PID setting from config file
//*	Jun 13,	2022	<RNS> Converted all PID function to use float for PID args
//*	Jun 17,	2022	<RNS> Renamed Servo_stop_axes_axis_tracking to Servo_stop_axes and can do both
//*	Jun 17,	2022	<RNS> Deleted Servo_stop_axes_all(), obsolete with Servo_stop_axes doing both
//*	Jun 17,	2022	<RNS> Added and renamed start_axis_tracking to support both axes
//*	Jun 19,	2022	<RNS> Fixed a toggle gParkState bug in _unpark() and return to int
//*	Jun 21,	2022	<RNS> Fixed incorret variable ref and conversion in move_to_static()
//*	Jun 24,	2022	<RNS> Added routines _is_TTP(), _get_lst(), _get_HA()
//*	Jun 26,	2022	<RNS> Added _COP_type() and simplified _calc_COP routines
//*	Ju1  1,	2022	<RNS> Moved low-level motor routines to Motion_ lib
//*	Ju1  2,	2022	<RNS> Ported Mount_ code to new Motion_ lib routines
//*	Jul  3,	2022	<RNS> Changed .time/.zero to .zeroTS/.zeroPos for clarity
//*	Jul  3,	2022	<RNS> Relocated memset to *_cfg.h source files
//*	Jul  3,	2022	<RNS> Rewrote both _step_to_pos and _pos_to_step
//*	Jul  3,	2022	<RNS> ported servo_init() to use Motion and Observ fields
//*	Jul  3,	2022	<RNS> renamed Servo_stop to Servo_stop_axes for consistancy
//*	Jul  4,	2022	<RNS> renamed gMountConfig to gsMountConfig to ID static
//*	Jul  4,	2022	<RNS> Removed dup code in move_static to use common _pos_to_step()
//*	Jul  4,	2022	<RNS> removed old code, now committed to new routines
//*	Jul  5,	2022	<RNS> Fixed not setting acc/vel down at the Motion level
//*	Jul  8,	2022	<RNS> Added missing support for .encoderMaxRate
//*	Jul  8,	2022	<RNS> Fixed _step_to_pos, dec had / swapped for *
//*	Jul 12,	2022	<RNS> Fixed a ton of initialization in Servo_init()
//*	Jul 20,	2022	<RNS> Fixed sign error in _move_to_static for relative RA direction
//*	Jul 20,	2022	<RNS> Fixed if-else braces error in COP for Fork mount
//*	Jul 22,	2022	<RNS> Fixed incorrect direction sign in flip_coordins 
//*	Oct 28,	2022	<RNS> Added support to initilize motor absZero field
//*	Nov 04,	2022	<RNS> Rewrote move_to_park to use absZero encoder values
//*	Nov 11,	2022	<RNS> Fixed/toggled RA vs. HA relative direction in COP_type()
//*	Nov 11,	2022	<RNS> Fixed comments, simplified code in _calc_short_vector
//*	Nov 13,	2022	<RNS> Added Servo_check_RA_axis_for_wrap routine
//*	Nov 13,	2022	<RNS> Modified _optimal_path to check for RA wrap on FORK mounts
//*	Nov 15,	2022	<RNS> Rewrote _calc_optimal_path to separate path calc from mount type
//*	Nov 16,	2022	<RNS> Added Servo_check_german_for_upside_down() + COP check for it;
//*****************************************************************************

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
#include "servo_observ_cfg.h"
#include "servo_motion_cfg.h"
#include "servo_mount_cfg.h"
#include "servo_motion.h"
#include "servo_mount.h"

#ifdef _ALPHA_OUT_
#define UNDER_TARGET 5.0
//#define	RA_SENSOR_CHANGE	1
//#define	DEC_SENSOR_CHANGE	2
//#define	BOTH_SENSOR_CHANGE	3
#endif // _ALPHA_OUT_

// Globals for the telescope mount, motion and location data structs
TYPE_MOUNT_CONFIG 	gMountConfig;

// Danger, local globals ahead
static double		gSysDepend		=	1.000000;
static bool			gIgnoreHorizon	=	false;
static bool			gParkState		=	true;
static TYPE_MOVE	gMountAction;
static char			gDebugInfoSS[]	=	"00";
static char			gDebugInfoCOP[]	=	"UNDEF";

//*****************************************************************************
// Return values for the COP move type
enum
{
	EAST_TO_EAST	=	1,
	EAST_TO_WEST,
	WEST_TO_WEST,
	WEST_TO_EAST
};

//*****************************************************************************
// Returns the park position found in the mount configuration file.
// This lockdown position is used for statically parking the mount for
// a power-off shutdown.
//*****************************************************************************
void Servo_get_park_coordins(double *ha, double *dec)
{
	*ha		=	gMountConfig.ra.park;
	*dec	=	gMountConfig.dec.park;
}

//*****************************************************************************
// Returns the standby position found in the mount configuration file.
// This standby position is used for statically parking the mount until
// another slew is needed.
//*****************************************************************************
void Servo_get_standby_coordins(double *ha, double *dec)
{
	*ha		=	gMountConfig.ra.standby;
	*dec	=	gMountConfig.dec.standby;
}

//*****************************************************************************
// Return the RA & Dec valueboth axis opto interrupters.
//*****************************************************************************
void Servo_get_sync_coordins(double *ra, double *dec)
{
	*ra		=	gMountConfig.ra.sync;
	*dec	=	gMountConfig.dec.sync;
}

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
double	Servo_get_time_ratio(void)
{
	return (gSysDepend);
}

//*****************************************************************************
// Releases the mount "parking brake" and allow movement. In order for an unpark
// to happen, the mount must be parked or stopped (allows for error recovery)
// It returns bool of true is successful, false if mount could not be unparked
// Note: the only way to set the park state to PARKED is via Servo_move_to_park()
// and a call to Servo_state after the PARKING move is complete
//*****************************************************************************
int Servo_unpark(void)
{
int state;

	state	=	Servo_state();
	printf("@@@ Servo_unpark() state = %d\n", state);
	if (state == PARKED || state == STOPPED)
	{
		gParkState		=	false;
		gMountAction	=	STOPPED;
		return(kSTATUS_OK);
	}
	else
	{
		return (kERROR);
	}
}

//*****************************************************************************
// Returns the mount's current Park state, and will return false if the mount
// is PARKING and not completed the move.  Note: the only way to set the park
// state to PARKED to call Servo_move_to_park() and after move is complete,
//  a call to Servo_state.  Servo_state will set the PARKED flag
//*****************************************************************************
bool Servo_get_park_state(void)
{
int state;

	state	=	Servo_state();

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
// Return a true if the mount is thru-the-pole (FORK) or flipped (GEM)
//*****************************************************************************
bool Servo_is_TTP(void)
{
	// If the current side is different the config file starting orientation
	return (gMountConfig.side != gMountConfig.ra.parkInfo ? true : false);
}

//*****************************************************************************
// Returns the current LST at the mount's longitude
//*****************************************************************************
double	Servo_get_lst(void)
{
double	jd, lst;

	// get the jd, GM Sid Time and convert to LST
	jd	=	Time_systime_to_jd();
	lst	=	Time_jd_to_gmst(jd);
	lst	=	Time_gmst_to_lst(lst, Servo_get_lon());

	return(lst);
}

//*****************************************************************************
// Returns the Hour Angle of the mount's current position
//*****************************************************************************
double	Servo_get_HA(void)
{
double	ra, dec;
double	lst;

	// get the LST
	lst	=	Servo_get_lst();

	Servo_get_pos(&ra, &dec);
	// HA = LST - RA and reuse lst for HA
	lst	-=	ra;
	Time_normalize_HA(&lst);

	return (lst);
}

//*****************************************************************************
// Allows the mount to move to object below the horizon, normally only
//	used for statically parking the mount. Expects a TRUE | FALSE
//	Returns the old horizon value for the curious
//*****************************************************************************
bool Servo_ignore_horizon(bool state)
{
bool oldValue	=	gIgnoreHorizon;

	gIgnoreHorizon	=	state;
	return oldValue;
}

//*****************************************************************************
// Resets the motor and initializes with the current motion parameters
// for any needed default values and set the home position.
// Return Values: kSTATUS_OK or kERROR = motor out of range
//*****************************************************************************
int Servo_reset_motor(uint8_t motor)
{
int status;

	status	=	Motion_reset_axis(motor);

	return status;
} // of Servo_reset_motor()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert the motor controllers internal
// step registers to a 'real' RA and Dec values.
// RA and DEC are returned in deciHours and deciDegs
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
static void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec)
// void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec)
{
double		zeroDegs, raDegs;
long double	currTime, elapsedTime, elapsedDegs;

	// get the current time and compute the elapsed time since zeroPos set
	currTime	=	Time_get_systime();
	elapsedTime	=	currTime - gMountConfig.ra.zeroTS;

	// Calc the elasped arcsecs since last zeroPos, based on Sidereal rate
	elapsedDegs	=	-kSIDER_RATE_ARCSECS * gMountConfig.ra.direction * gSysDepend * elapsedTime;
	elapsedDegs	/=	3600.0; 	// Convert to degrees

	// Calc the updated zeroPos by adding the movement of the earth since saving the zero position
	zeroDegs	=	gMountConfig.ra.zeroPos + elapsedDegs;

	// convert input RA position from steps to decimal deg
	raDegs	=	(gMountConfig.ra.direction) * (raStep / gMountConfig.ra.step) / 3600.0;

	// Calculated RA in degs is just zeroDegs + raDegs because of zero steps based
	*ra		=	zeroDegs + raDegs;
	// convert from decidegs to decihours for ra return value
	Time_deci_deg_to_hours(ra);
	Time_normalize_RA(ra);

	// convert Dec position from steps to decimal deg
	zeroDegs	=	(gMountConfig.dec.direction) * (decStep / gMountConfig.dec.step) / 3600.0;

	// get the 'true' position with home offset in decidegs, works due to steps is zero based
	// printf("### Servo_step_to_pos: zeroDegs:%.3f dec.zeroPos:%.3f\n", zeroDegs, gMountConfig.dec.zeroPos);
	*dec		=	gMountConfig.dec.zeroPos + zeroDegs;
	return;
} // of Servo_step_to_pos()

//*****************************************************************************
// INTERNAL ROUTINE: used to convert ra and dev values in decihour and decidegs
// to MC step positions. It does no checks for meridian, flip/TTP, etc
// TODO: Will need major mind-numbing changes to support alt-azi
//*****************************************************************************
static void Servo_pos_to_step(double ra, double dec, int32_t *raStep, int32_t *decStep)
// void Servo_pos_to_step(double ra, double dec, int32_t *raStep, int32_t *decStep)
{
double		zeroDegs;
long double	currTime, elapsedTime, elapsedDegs;
double		raDelta, decDelta;

	// Convert RA to decidegs
	Time_deci_hours_to_deg(&ra);

	// get the current time and compute the elapsed time since zeroPos set
	currTime	=	Time_get_systime();
	elapsedTime	=	currTime - gMountConfig.ra.zeroTS;

	// Calc the elasped arcsecs since last zeroPos, based on Sidereal rate
	elapsedDegs	=	-kSIDER_RATE_ARCSECS * gMountConfig.ra.direction * gSysDepend * elapsedTime;
	elapsedDegs	/=	3600.0; 	// Convert to degrees

	// Calc the updated zeroPos by adding the movement of the earth since saving the zero position
	zeroDegs	=	gMountConfig.ra.zeroPos + elapsedDegs;

	// Determine the degrees difference from target coordins and zero position
	// this math only works since axis.zeroPos value corresponds to zero step position
	// TODO: Rare potential 24/0 rolloever issue if zero position is set more than 12 hours ago
	raDelta	=	Servo_calc_short_vector(zeroDegs, ra, 360.0);
	raDelta	*=	3600.0; 	// Convert from decidegs to arcsecs

	// get the actual RA steps needed in absolute steps, not relative offset
	*raStep	=	(int32_t)((gMountConfig.ra.direction) * (raDelta * gMountConfig.ra.step));

	// Determine the Dec degrees difference from target coordins and home position
	// this math only works since axis.zeroPos value corresponds to zero step position
	decDelta	=	dec - gMountConfig.dec.zeroPos;
	decDelta	*=	3600.0;	// Convert from decidegs to arcsecs

	// get the actual Dec steps needed in absolute steps, not relative offset
	*decStep	=	(int32_t)((gMountConfig.dec.direction) * (decDelta * gMountConfig.dec.step));
} // of Servo_pos_to_step()

//*****************************************************************************
// Returns the current position of the mount in RA and Dec values.
// RA and DEC are returned in deci hours and deci degs
//*****************************************************************************
void Servo_get_pos(double *ra, double *dec)
{
int32_t raStep, decStep;

	// Get the current RA/Dec position in steps
	Motion_get_axis_curr_step(SERVO_RA_AXIS, &raStep);
	Motion_get_axis_curr_step(SERVO_DEC_AXIS, &decStep);

	// Convert from steps to RA/Dec
	Servo_step_to_pos(raStep, decStep, ra, dec);
} // Servo_get_pos()

//*****************************************************************************
// Sets the mount position to supplied RA and Dec values. Input RA and Dec
// are in deciHour and deciDegs respectively. Assume the mount is
// tracking more or less correctly. (ie. the mount is not stopped)
// Both RA/Dec home position are stored in deci degrees and the
// MC's set_home command always sets the encoder/position value to zero
//*****************************************************************************
void Servo_set_pos(double ra, double dec)
{
	// set home on the RA axis (deci degs)
	gMountConfig.ra.zeroTS	=	Time_get_systime();
	Time_deci_hours_to_deg(&ra);
	gMountConfig.ra.zeroPos	=	ra;
	Motion_set_axis_zero(SERVO_RA_AXIS);

	// set home on the DEC axis (deci degs)
	gMountConfig.dec.zeroTS		=	Time_get_systime();
	gMountConfig.dec.zeroPos	=	dec;
	Motion_set_axis_zero(SERVO_DEC_AXIS);
} // Servo_set_pos()

//*****************************************************************************
// Sets the position for a stopped mount. Since the mount is not moving,
// Hour Angle is used instead of RA and inputs are deciHours and deciDegs
//*****************************************************************************
void Servo_set_static_pos(double ha, double dec)
{
long double	lst;
double		ra;

	Servo_reset_motor(SERVO_RA_AXIS);
	Servo_reset_motor(SERVO_DEC_AXIS);

	// get the LST
	lst	=	Servo_get_lst();

	// set RA to the LST + HA offset
	ra	=	lst - ha;

	// Set the current position
	Servo_set_pos(ra, dec);
} // of Servo_set_static_pos()

//*****************************************************************************
// INTERNAL ROUTINE: Sets the tracking rate in motor controller axis
//	in steps/sec but does not start tracking
//*****************************************************************************
static int Servo_set_axis_tracking(uint8_t motor, int32_t tracking)
{
int status	=	kSTATUS_OK;

	switch (motor)
	{
		case SERVO_RA_AXIS:
			gMountConfig.ra.trackRate	=	(int32_t)tracking;
			break;

		case SERVO_DEC_AXIS:
			gMountConfig.dec.trackRate	=	(int32_t)tracking;
			break;

		default:
			// do nothing
			status	=	kERROR;
			break;
	} // of switch

	// save a local copy for Motion routines
	Motion_set_axis_trackRate(motor, tracking);

	return (status);
} // of Servo_set_axis_tracking()

//*****************************************************************************
// INTERNAL ROUTINE: Returns the current tracking rate in motor controller axis
//	in steps/sec
//*****************************************************************************
static int32_t Servo_get_axis_trackRate(uint8_t motor)
{
int32_t currTrackingRate;

	currTrackingRate	=	0;
	switch (motor)
	{
		case SERVO_RA_AXIS:
			currTrackingRate	=	gMountConfig.ra.trackRate;
			break;

		case SERVO_DEC_AXIS:
			currTrackingRate	=	gMountConfig.dec.trackRate;
			break;

		default:
			// do nothing
			break;
	} // of switch

	return (currTrackingRate);
} // of Servo_get_axis_trackRate()

//*****************************************************************************
// Start the one or both motors tracking if a tracking rate has been set in the
// axisPtr data struct. this is an unbuffered command and will overwrite any
// pending actions on the axis
//*	returns kSTATUS_OK on success, kERROR otherwise
//*****************************************************************************
int Servo_start_axes_tracking(uint8_t motor)
{
double	ra, dec;
int32_t	trackingRate;
int		status;

	CONSOLE_DEBUG(__FUNCTION__);
	status	=	kERROR;

	// Set the current pos with the current tracking rate but only if the step
	// sizes for RA and Dec have been set during Servo_init
	if ((gMountConfig.ra.step > 0.0) && (gMountConfig.dec.step > 0.0))
	{
		// Get and set the position to set begin timestamp for new tracking rate session
		Servo_get_pos(&ra, &dec);
		Servo_set_pos(ra, dec);

		switch (motor)
		{
			case SERVO_BOTH_AXES:
				// for both axes, do RA first and fall thru the case switch to get Dec too
				// Start RA velocity move with the axis tracking rate if not zero
				trackingRate	=	Servo_get_axis_trackRate(SERVO_RA_AXIS);
				if (trackingRate != 0)
				{
					// Non-buffered move by tracking velocity
					Motion_set_axis_buffer(SERVO_RA_AXIS, false);
					status	=	Motion_move_axis_by_vel(SERVO_RA_AXIS, trackingRate);
				}
				[[fallthrough]];
				// NOTE: NO BREAK, BOTH needs to fall thru to check Dec as well

			case SERVO_DEC_AXIS:
				// Start DEC velocity move with the axis tracking rate if not zero
				trackingRate	=	Servo_get_axis_trackRate(SERVO_DEC_AXIS);
				if (trackingRate != 0)
				{
					// Non-buffered move by tracking velocity
					Motion_set_axis_buffer(SERVO_DEC_AXIS, false);
					status	=	Motion_move_axis_by_vel(SERVO_DEC_AXIS, trackingRate);
				}
				break;

			case SERVO_RA_AXIS:
				// Start RA velocity move with the axis tracking rate if not zero
				trackingRate	=	Servo_get_axis_trackRate(SERVO_RA_AXIS);
				if (trackingRate != 0)
				{
					// Non-buffered move by tracking velocity
					Motion_set_axis_buffer(SERVO_RA_AXIS, false);
					status	=	Motion_move_axis_by_vel(SERVO_RA_AXIS, trackingRate);
				}
				break;

			default:
				// do nothing
				status	=	kERROR;
				break;
		} // of switch
	}
	else
	{
		// Mount was not initialized or failed Servo_init
		status	=	kERROR;
	} // of if-else steps vars initialized

	return (status);
} // of Servo_start_axes_tracking()

//*****************************************************************************
// Stops a single motor or both motors. Existing tracking rate value, if set,
// is not affected. Returns kSTATUS_OK on success, or kERROR otherwise
//*****************************************************************************
int Servo_stop_axes(uint8_t motor)
{
int		status;

	CONSOLE_DEBUG(__FUNCTION__);
	status	=	kERROR;
	switch (motor)
	{
		case SERVO_BOTH_AXES:
			// for both axes, do RA first and fall thru the case switch to get Dec too
			// Stop the RA motor
			status	=	Motion_stop_axis(SERVO_RA_AXIS);
			[[fallthrough]];
			// NO BREAK

		case SERVO_DEC_AXIS:
			// Stop the Dec motor
			status	=	Motion_stop_axis(SERVO_DEC_AXIS);
			break;

		case SERVO_RA_AXIS:
			// Stop the RA motor
			status	=	Motion_stop_axis(SERVO_RA_AXIS);
			break;

		default:
			CONSOLE_DEBUG("Invalid motor specified!!!!!!!!!!!!!!!!");
			// do nothing
			status	=	kERROR;
			break;
	} // of switch
	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return status;
} // Servo_stop_axes()

//*****************************************************************************
// Move the axis at a rate in degs/sec for a
// Rate is limited by the axis .slew field which is the max hand control speed
//*	returns kSTATUS_OK on success, or kERROR otherwise
// TODO: Need to update this for guidin with time direction field, 0 is forever
//*****************************************************************************
int Servo_move_axis_by_vel(uint8_t motor, double vel)
{
int status;
int32_t velStep;

	CONSOLE_DEBUG(__FUNCTION__);

	// Convert the input vel in degs/sec rate to step vel measured in steps/sec
	// first convert input vel into arcsecs/sec
	vel	*=	3600.0;
	switch (motor)
	{
		case SERVO_RA_AXIS:
			// Set velocity direction based on axis cfg, divide by the axis->steps to get final vel in steps
			velStep	=	(int32_t)(vel * gMountConfig.ra.direction / gMountConfig.ra.step);
			break;

		case SERVO_DEC_AXIS:
			// Set velocity direction based on axis cfg, divide by the axis->steps to get final vel in steps
			velStep	=	(int32_t)(vel * gMountConfig.dec.direction / gMountConfig.dec.step);
			break;

		default:
			// do nothing
			break;
	} // of switch

	// Set the move to be unbuffered
	Motion_set_axis_buffer(motor, false);
	status	=	Motion_move_axis_by_vel(motor, velStep);
	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return status;
} // Servo_move_axis_by_vel()
//*****************************************************************************
// Move the axis at a rate in degs/sec for a
// Rate is limited by the axis .slew field which is the max hand control speed
//*	returns kSTATUS_OK on success, or kERROR otherwise
// TODO: Need to update this for guidin with time direction field, 0 is forever
//*****************************************************************************
int Servo_move_axis_by_vel_time(uint8_t motor, double vel, double seconds)
{
int status;
int32_t velStep;

	CONSOLE_DEBUG(__FUNCTION__);

	// Convert the input vel in degs/sec rate to step vel measured in steps/sec
	// first convert input vel into arcsecs/sec
	vel	*=	3600.0;
	switch (motor)
	{
		case SERVO_RA_AXIS:
			// Set velocity direction based on axis cfg, divide by the axis->steps to get final vel in steps
			velStep	=	(int32_t)(vel * gMountConfig.ra.direction / gMountConfig.ra.step);
			break;

		case SERVO_DEC_AXIS:
			// Set velocity direction based on axis cfg, divide by the axis->steps to get final vel in steps
			velStep	=	(int32_t)(vel * gMountConfig.dec.direction / gMountConfig.dec.step);
			break;

		default:
			// do nothing
			break;
	} // of switch

	// Set the move to be unbuffered
	Motion_set_axis_buffer(motor, false);
	status	=	Motion_move_axis_by_time(motor, velStep, seconds);
	CONSOLE_DEBUG_W_NUM("status\t=", status);

	return status;
} // Servo_move_axis_by_vel()

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
int Servo_init(const char *observCfgFile, const char *mountCfgFile, const char *motionCfgFile)
{
int		status;
int		returnStatus	=	kSTATUS_OK;
double	currRa, currDec;

	CONSOLE_DEBUG(__FUNCTION__);

	// Read the location config file for the mount location and check status
	status	=	Servo_read_observ_cfg(observCfgFile);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open observatory location file '%s'.\n", observCfgFile);
		returnStatus	=	kERROR;
	}

	// initialize motion interface for the motor controller
	status = Motion_init(motionCfgFile);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open intialize the motor controller\n");
		returnStatus = kERROR;
	}

	// Read the mount config file for the mount physical characteristics
	status = Servo_read_mount_cfg(mountCfgFile, &gMountConfig);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not open mount configuration file '%s'.\n", mountCfgFile);
		returnStatus = kERROR;
	}

	// If all three config files were read sucessfully
	if (returnStatus == kSTATUS_OK)
	{
		// convert drive precession for *friction drives* from arcsec/deg to percents only - this term will likely just be 1.0
		gMountConfig.ra.prec = 1.0 - (gMountConfig.ra.prec / 3600.0);

		// calc encoder steps per arcsec
		gMountConfig.ra.step = (gMountConfig.ra.motorGear * gMountConfig.ra.mainGear * gMountConfig.ra.encoder * gMountConfig.ra.prec) / 1296000.0;

		// Max accel is the acc in arcsec/sec^2 from the mount config file
		gMountConfig.ra.maxAcc = (uint32_t)gMountConfig.ra.realAcc * gMountConfig.ra.step;
		gMountConfig.ra.acc = gMountConfig.ra.maxAcc;
		Motion_set_axis_acc(SERVO_RA_AXIS, gMountConfig.ra.acc);

		// Max velocity in arcsec/sec from the config file and converted to step/sec
		gMountConfig.ra.maxVel = (uint32_t)gMountConfig.ra.realVel * gMountConfig.ra.step;
		gMountConfig.ra.vel = gMountConfig.ra.maxVel;
		Motion_set_axis_vel(SERVO_RA_AXIS, gMountConfig.ra.vel);

		// Calc the adjust and slew speeds by multiplying by the mount's steps / arcsec ratio
		gMountConfig.ra.guideRate = (uint32_t)gMountConfig.ra.realAdj * gMountConfig.ra.step;
		gMountConfig.ra.manSlewRate = (uint32_t)gMountConfig.ra.realSlew * gMountConfig.ra.step;

		gMountConfig.ra.direction = gMountConfig.ra.config;

		// Set the PID value in MC with the profile from motion config file data
		Motion_set_axis_profile(SERVO_RA_AXIS);

		// Set the  RC tracking rate to sidereal as default and copy to active tracking rate
		gMountConfig.ra.defaultRate = -kSIDER_RATE_ARCSECS * gMountConfig.ra.direction * gSysDepend * gMountConfig.ra.step;
		Servo_set_axis_tracking(SERVO_RA_AXIS, gMountConfig.ra.defaultRate);

		// Initialize all the zero position fields and set timestamp
		Motion_set_axis_absZero(SERVO_RA_AXIS, 0); 
		gMountConfig.ra.zeroPos = 0.0;
		gMountConfig.ra.zeroTS = Time_get_systime();

		// Duplicate of what's already done for RA, comments above
		gMountConfig.dec.prec = 1.0 - (gMountConfig.dec.prec / 3600.0);
		gMountConfig.dec.step = (gMountConfig.dec.motorGear * gMountConfig.dec.mainGear * gMountConfig.dec.encoder * gMountConfig.dec.prec) / 1296000.0;
		gMountConfig.dec.maxAcc = (uint32_t)gMountConfig.dec.realAcc * gMountConfig.dec.step;
		gMountConfig.dec.acc = gMountConfig.dec.maxAcc;
		Motion_set_axis_acc(SERVO_DEC_AXIS, gMountConfig.dec.acc);
		gMountConfig.dec.maxVel = (uint32_t)gMountConfig.dec.realVel * gMountConfig.dec.step;
		gMountConfig.dec.vel = gMountConfig.dec.maxVel;
		Motion_set_axis_vel(SERVO_DEC_AXIS, gMountConfig.dec.vel);
		gMountConfig.dec.guideRate = (uint32_t)gMountConfig.dec.realAdj * gMountConfig.dec.step;
		gMountConfig.dec.manSlewRate = (uint32_t)gMountConfig.dec.realSlew * gMountConfig.dec.step;
		gMountConfig.dec.direction = gMountConfig.dec.config;
		Motion_set_axis_profile(SERVO_DEC_AXIS);
		gMountConfig.dec.defaultRate = 0;
		Servo_set_axis_tracking(SERVO_DEC_AXIS, gMountConfig.dec.defaultRate);
		Motion_set_axis_absZero(SERVO_DEC_AXIS, 0); 
		gMountConfig.dec.zeroPos = 0.0;
		gMountConfig.dec.zeroTS = Time_get_systime();

		// // Make sure the "gear lash" is at least 40*axis->step arcseconds for the PID filter resolution
		// if (gMountConfig.ra.gearLash < (gMountConfig.ra.step * 40.0))
		// {
		// 	gMountConfig.ra.gearLash	=	gMountConfig.ra.step * 40.0;
		// }
		// if (gMountConfig.dec.gearLash < (gMountConfig.dec.step * 40.0))
		// {
		// 	gMountConfig.dec.gearLash	=	gMountConfig.dec.step * 40.0;
		// }

		// Convert the global off target tolerance from arcseconds to deci degrees
		gMountConfig.offTarget /= 3600.0;

		// Set the position of the stationary mount to the park position from config file
		printf("ra.park = %lf	dec.park = %lf\n", gMountConfig.ra.park, gMountConfig.dec.park);
		// Set the position based on the park *HA* and Dec
		Servo_set_static_pos(gMountConfig.ra.park, gMountConfig.dec.park);
		// This returns the current position in *RA* and Dec
		Servo_get_pos(&currRa, &currDec);
		printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
		// Set the mount state to Park
		gParkState = true;
	} // of if kSTATUS_OK

	return (returnStatus);
} // of Servo__init()

//*****************************************************************************
// Returns whether the mount is PARKED, STOPPED, MOVING or TRACKING using typedef TYPE_MOVE
// with the above fields. For STOPPED both axis must be stopped and if it's stopped,
// it may be parked if the global state gParkState is set to true. If either axis is
// moving, then MOVING is returned and has a high precedence than TRACKING.
// If the mount is tracking on either or both axes, TRACKING is returned.
// Tracking is checked against the axis.trackRate value must be withiin 10%
//*****************************************************************************
TYPE_MOVE Servo_state(void)
{
uint8_t		raState, decState;
int32_t		raVel, decVel;
uint32_t	absVel, absTrack;
float		velFloat, trackFloat;
int			status;
// bool		raStopped, decStopped;

	// CONSOLE_DEBUG(__FUNCTION__);

	// read the command queue for RA and Dec
	status	=	Motion_get_pending_cmds(&raState, &decState);
	if (status != kSTATUS_OK)
	{
		CONSOLE_DEBUG("ERROR:  Motion_get_pending_cmds() returned error!");
	}
	// Go for the easy win, just see if both cmds queues are empty
	// printf("\n *** ra state = %x	dec state = %x ***\n", raState, decState);

	// TODO: need to convert the weird RC numbering system to simple zero based
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
				gParkState		=	true;
				// reset mount action to PARKED as well
				gMountAction	=	PARKED;
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
	if ((decState != kRC_CMD_QUEUE_EMPTY) && gMountConfig.dec.trackRate == 0)
	{
		// Therefore dec is moving
		strcpy(gDebugInfoSS, "M2");
		return (MOVING);
	}
	//	CONSOLE_DEBUG("If you get to here, it means the dec is not moving, but could be tracking");

	// If you get to here, it means the dec is not moving, but could be tracking

	// But check RA first, highly likely .trackRate != 0, if any axis is tracking, then the mount
	// is tracking.  So read the current mount velocity for RA to compare to tracking rate
	Motion_get_axis_curr_vel(SERVO_RA_AXIS, &raVel);

	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel		=	abs(raVel);
	absTrack	=	abs(gMountConfig.ra.trackRate);
	// printf("^^^ absVel = %d	absTrack = %d\n", absVel, absTrack);

	// convert to floats
	velFloat	=	(float)absVel;
	trackFloat	=	(float)absTrack;

	// if the RA axis velocity is +/- 20% of the track velocity, then assume RA is tracking
	// TODO: KLUDGE!  Tighten down the PID to get a better tolerance than 50%
	if (velFloat > (trackFloat * 0.5) && velFloat < (trackFloat * 1.5))
	{
		if (gMountConfig.ra.trackRate != 0)
		{
			strcpy(gDebugInfoSS, "T1");
			return (TRACKING);
		}
	}

	// Check Dec for tracking (could be an alt-azi, but do not rely on that)
	Motion_get_axis_curr_vel(SERVO_DEC_AXIS, &decVel);
	// KLUDGE!  using absolute values until I figure out how RC does neg velocities
	absVel		=	abs(decVel);
	absTrack	=	abs(gMountConfig.dec.trackRate);
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
		if (gMountConfig.dec.trackRate != 0)
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
// TODO: need to check the logic of using tracking&sign to set alt-azi move time
//*****************************************************************************
static int Servo_move_step_track(int32_t raStep, int32_t decStep)
{
double	duration;
int32_t	trackingRate;
int32_t	currStep;
uint8_t raState, decState;
int		status	=	kSTATUS_OK;

	// If RA axis has a tracking value
	trackingRate	=	Servo_get_axis_trackRate(SERVO_RA_AXIS);
	if (trackingRate != 0)
	{
		// figure out where we are now
		status	-=	Motion_get_axis_curr_step(SERVO_RA_AXIS, &currStep);
		// calculate the duration of the axis move and convert to step and add to target
		duration	=	Motion_calc_axis_move_time(SERVO_RA_AXIS, currStep, raStep);
		// additional steps are add/subract based on the sign of the axis tracking rate
		raStep		+=	(uint32_t)(duration * (double)trackingRate);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)

		Motion_get_pending_cmds(&raState, &decState);
		printf("S_mst: Before RA M_mabs Qra:%d Qdec:%d\n", raState, decState);

		Motion_set_axis_buffer(SERVO_RA_AXIS, false);
		status	-=	Motion_move_axis_by_step(SERVO_RA_AXIS, raStep);
		
		Motion_get_pending_cmds(&raState, &decState);
		printf("S_mst: After RA M_mabs Qra:%d Qdec%d\n", raState, decState);
		printf("!!! raStep = %d  vel = %d  acc = %d\n", raStep, gMountConfig.ra.vel, gMountConfig.ra.acc);

		// This velocity command will start when the above pos cmd completes
		// Buffered move by tracking velocity
				
		Motion_get_pending_cmds(&raState, &decState);
		printf("S_mst: Before RA M_mabv Qra:%d Qdec:%d\n", raState, decState);

		Motion_set_axis_buffer(SERVO_RA_AXIS, true);
		status	-=	Motion_move_axis_by_vel(SERVO_RA_AXIS, trackingRate);

		Motion_get_pending_cmds(&raState, &decState);
		printf("S_mst:  After RA M_mabv Qra:%d Qdec:%d\n", raState, decState);
	}
	else
	{
		Motion_set_axis_buffer(SERVO_RA_AXIS, false);
		status	-=	Motion_move_axis_by_step(SERVO_RA_AXIS, raStep);
	}

	// If Dec axis has a tracking value
	trackingRate	=	Servo_get_axis_trackRate(SERVO_DEC_AXIS);
	if (trackingRate != 0)
	{
		// figure out where we are now
		status		-=	Motion_get_axis_curr_step(SERVO_DEC_AXIS, &currStep);
		// calculate the duration of the axis move and convert to step and add to target
		duration	=	Motion_calc_axis_move_time(SERVO_DEC_AXIS, currStep, decStep);
		// additional steps are add/subract based on the sign of the axis tracking rate
		decStep		+=	(uint32_t)(duration * (double)trackingRate);

		// make the first slew to time corrected target and buffer a vel cmd for tracking
		// this first command has to empty any remaining buffered cmds (false)
		Motion_set_axis_buffer(SERVO_DEC_AXIS, false);
		status	-=	Motion_move_axis_by_step(SERVO_DEC_AXIS, decStep);
		printf("!!! decStep = %d  vel = %d  acc = %d\n", raStep, gMountConfig.dec.vel, gMountConfig.dec.acc);

		// This velocity command will start when the above pos cmd completes
		// Buffered move by tracking velocity
		Motion_set_axis_buffer(SERVO_DEC_AXIS, true);
		status	-=	Motion_move_axis_by_vel(SERVO_DEC_AXIS, trackingRate);
	}
	else
	{
		Motion_set_axis_buffer(SERVO_DEC_AXIS, false);
		status	-=	Motion_move_axis_by_step(SERVO_DEC_AXIS, decStep);
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
int		status;

	// Set the desired buffered state for the moves
	Motion_set_axis_buffer(SERVO_RA_AXIS, buffered);
	Motion_set_axis_buffer(SERVO_DEC_AXIS, buffered);

	// Start the RA move and decrement status if error returned
	status	=	Motion_move_axis_by_step(SERVO_RA_AXIS, raStep);

	// Start the Dec move
	status	-=	Motion_move_axis_by_step(SERVO_DEC_AXIS, decStep);

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

	// toggle dec motion direction - basically > 0 means currently "forward"
	*direction	=	(*direction > 0) ? kREVERSE : kFORWARD;
}

//*****************************************************************************
// This routine will find the shortest path between two points on a circle
// Circle size is supplied by the 'max' arg, and for RA it is 24.0, Dec is 360.0
// It will return the vector needed to move from the start position in the 
// selected units.  This was a PITA
//*****************************************************************************
double Servo_calc_short_vector(double begin, double end, double max)
{
double	sDiff;
double	rawDiff;
double	modDiff;

	sDiff	=	0.0;
	rawDiff	=	(begin > end) ? begin - end : end - begin;
	// set the rollover value from the args
	modDiff	=	fmod(rawDiff, max);

	// Split the circle and look for the shorter length
	if (modDiff > (max / 2.0))
	{
		// There is a shorter path in opposite direction
		sDiff	=	(max - modDiff);
		// if end larger toggle the sign
		if (end > begin)
		{
			sDiff	=	-sDiff;
		}
	}
	else
	{
		sDiff	=	modDiff;
		// if begin larger toggle the sign
		if (begin > end)
		{
			sDiff	=	-sDiff;
		}
	}

	return sDiff;
}

//*****************************************************************************
// INTERNAL ROUNTINE: Determines the path/move type for an input, HA, path and
// flipWin.  raPath is a *relative* direction in RA (not HA) from the startRaHA
// position. Flipwin is important for GEM mounts, allows for limited movement
// past LST/meridian/0HA. Since one edge of the flip windows is alway 0.0 LST,
// just need test against other edge. For FORK mount the flipwin should 0.0.
// TODO: Need to flip to static once fully debugged
//*****************************************************************************
int Servo_COP_type(double startRaHa, double raPath, double flipWin)
{
	double haPath; 

	// HA relative direction is reversed from RA relative direction
	haPath = -raPath; 
	// If starting position is on the EAST side of the meridian
	if (startRaHa < 0.0)
	{
		// If the raPath vector is positive enough to move past the west flip window edge of meridian
		if ( (startRaHa + haPath) > flipWin )
		{
			printf("&&& Servo_COP_type() East to West\n");
			return EAST_TO_WEST;
		}
		else // stays on the east side within the meridian window
		{
			printf("&&& Servo_COP_type() East to East\n");
			return EAST_TO_EAST;
		}
	}
	else // start position is WEST of the meridian
	{
		// If the raPath vector is negative enough to move past the east flip window edge of meridian
		if ( (startRaHa + haPath) < -flipWin )
		{
			printf("&&& Servo_COP_type() West to East\n");
			return WEST_TO_EAST;
		}
		else // stays on the west side within the meridian window
		{
			printf("&&& Servo_COP_type() West to West\n");
			return WEST_TO_WEST;
		}
	}
	// It should never get here, but just in case
	return kERROR;
} // of Servo_COP_move_type()

//*****************************************************************************
// INTERNAL ROUNTINE: Checks the RA path to avoid moving the RA axis more than 
// 12H (180degs) from current LST using hour angle math.  We want to keep the 
// mount 'upright' to avoid cable wrap issues. 
// TODO: Need to flip to static once fully debugged
//*****************************************************************************
bool Servo_check_RA_axis_for_wrap(double startRaHa, double raPath)
{
	double haPath; 

	// HA relative direction is reversed from RA relative direction
	haPath = -raPath; 
	// If the end path is outside the region of +/- 12.0 HA
	if (fabs(startRaHa + haPath) > 12.0)
	{
		return true; 
	}
	else 
	{
		return false; 
	}
} // of Servo_check_RA_axis_for_wrap()

//*****************************************************************************
// INTERNAL ROUNTINE: Checks if a GERMAN mount would have the counterweight
// higher then the telescope.  This uses 6H (90degs) from current LST using 
// hour angle math.  We want to keep the mount 'upright' 
// TODO: Need to flip to static once fully debugged
//*****************************************************************************
bool Servo_check_german_for_upside_down(double startRaHa, double raPath, double region)
{
	double haPath; 

	// HA relative direction is reversed from RA relative direction
	haPath = -raPath; 
	// If the end path is beyond +/- 6.0 HA (90 degs from meridian) plus region
	if (fabs(startRaHa + haPath) > (6.0 + region))
	{
		return true; 
	}
	else // start position is WEST of the meridian
	{
		return false; 
	}
} // of Servo_check_german_for_upside_down()
//*****************************************************************************
// INTERNAL ROUNTINE:  This is try-before-you-buy routine. It calcs the best path
// for the mount to from start to end and returns the *relative direction* needed
// to move the mount from the start position but makes *no changes*.  If you don't
// like the path chosen, don't use it and no values will have changed. It assumes
// that end coordins are above the horizon. It also will return whether a flip
// is needed, but *does not* make the flip coordins changes to the start position.
// If a flip is returned, you *MUST* flip the start coordins for the returned path
// in raDirection & decDirection to be accurate.
// All inputs in decimal Hours/degs format and returns boolean if mount flip required
// TODO: Does not currently support TTP mode for GEM mounts, regardless the config file setting
// TODO: Need to flip to static once fully debugged
//*****************************************************************************
bool Servo_calc_optimal_path(double startRa, double startDec, double lst, double endRa, double endDec, double *raDirection, double *decDirection)
{
	double startRaHa, endRaHa;
	double startRaFlip, startDecFlip, startRaHaFlip;
	double decStdPath, decFlipPath, raStdPath, raFlipPath;
	bool raStdPathWraps;
	// bool	raFlipPathWraps;
	bool germanUpsideDown;
	int raStdPathType;
	double direction; // not used
	int8_t side;	  // not used
					  // int		raFlipPathType;

	// *** Do all the possible path calcs upfront for standard and flipped path, even it they may not be used

	// Converting the input start and end coordins to hour angle
	startRaHa = lst - startRa;
	endRaHa = lst - endRa;
	Time_normalize_HA(&startRaHa);
	Time_normalize_HA(&endRaHa);

	// Do the simple Dec axis first, it just path = end - start;
	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decStdPath = endDec - startDec;
	printf("&&& Servo_COP()  decStdPath(%.2lf)  endDec(%.2lf)  startDec(%.2lf)\n", decStdPath, endDec, startDec);

	// Find the short path for the Ra axis for a std move
	raStdPath = Servo_calc_short_vector(startRa, endRa, 24.0);
	printf("&&& Servo_COP() LST:%.2lf - StartRa = %.2lf  endRa = %.2lf  raStdPath = %.2lf  startRaHa = %.2lf  endRaHa = %.2lf\n", lst, startRa, endRa, raStdPath, startRaHa, endRaHa);

	// This first check is for a simple move (no meridian crossing), works for both FORK and GEM mounts
	raStdPathType = Servo_COP_type(startRaHa, raStdPath, gMountConfig.flipWin);

	// Calc the alternate path if we were to flip the mount vs just keep the std path (above)
	startRaFlip = startRa;
	startDecFlip = startDec;
	Servo_calc_flip_coordins(&startRaFlip, &startDecFlip, &direction, &side);

	// Calc the the HA of the new flipped start position, but endRaHa remains the same
	startRaHaFlip = lst - startRaFlip;
	Time_normalize_HA(&startRaHaFlip);
	printf("&&& Servo_COP() LST:%.2lf startRaFlip:%.2lf endRa:%.2lf  startRaHaFlip:%.2lf\n", lst, startRaFlip, endRa, startRaHaFlip);

	// Note: if pathDec is neg, it means a move towards South, positive... a move towards North
	decFlipPath = endDec - startDecFlip;
	printf("&&& Servo_COP() decFlipPath(%.2lf)  endDec(%.2lf)  startDecFlip(%.2lf)\n", decFlipPath, endDec, startDecFlip);

	// Find the short path of the Ra axis for a flip move, endRa is same as std path
	raFlipPath = Servo_calc_short_vector(startRaFlip, endRa, 24.0);
	printf("&&& Servo_COP() LST:%.2lf startRaFlip:%.2lf endRa:%.2lf  raFlipPath:%.2lf startRaHaFlip:%.2lf\n", lst, startRaFlip, endRa, raFlipPath, startRaHaFlip);

	// Not needed right now - Determine the rrap and move type of flipped path
	// check if the std path will cause RA to wrap (12H/180deg past LST) on either side
	// raFlipPathWraps = Servo_check_RA_axis_for_wrap(startRaHaFlip, raFlipPath);
	// raFlipPathType	=	Servo_COP_move_type(startRaHaFlip, raFlipPath, gMountConfig.flipWin);
	// Look for what hopefully should be a simple move (same side of meridian), with a possible exception of checking for FORK RA wrap

	// *** Now determine Mount type and then start applying standard and flip path option, by mount type only, no sharing

	switch (gMountConfig.mount)
	{
	// *** If a FORK mount, forks worry about wrapping the RA axis past +/-180degs from the meridian
	case kFORK:
		// This is a simplified fork move that prioritizes not wrapping the RA axis
		// and currently just optimized the move for RA distance only
		// TODO: Needs to also look for the shortest path for both axes for forks that do TTP
		// check if the std path will cause RA to wrap (12H/180deg past LST) on either side
		raStdPathWraps = Servo_check_RA_axis_for_wrap(startRaHa, raStdPath);
		if (raStdPathWraps == true)
		{
			// The FORK move slews the RA axis past 180degs from LST
			// If the fork mount does not allow thru-the-pole motion
			if (gMountConfig.ttp == false)
			{
				// Need to send the RA axis the opposite directon and thru the meridian
				// Add or sub 24.0H (360degs) from the RA standard path
				*raDirection = (raStdPath > 0.0) ? (-24.0 + raStdPath) : (24 + raStdPath);
				*decDirection = decStdPath;
				strcpy(gDebugInfoCOP, "FW-SS");
				return (false);
			}
			else
			{
				// The fork mount allows TTP motion, so just flip the mount
				*raDirection = raFlipPath;
				*decDirection = decFlipPath;
				strcpy(gDebugInfoCOP, "FWTTP");
				return (true);
			}
		}
		else
		{
			// Check to see if the fork allows TTP movement, *and* RA path is at least 10% shorter with RA flip path vs RA std path
			if ((gMountConfig.ttp == true) && (fabs(raFlipPath) < (fabs(raStdPath) * 0.9)))
			{
				// the flip is shorter than the 'normal' path and will cross the meridian
				*raDirection = raFlipPath;
				*decDirection = decFlipPath;
				strcpy(gDebugInfoCOP, "FFTTP");
				return (true);
			}
			else
			{
				// This is just a boring FORK std path move
				*raDirection = raStdPath;
				*decDirection = decStdPath;
				strcpy(gDebugInfoCOP, "F--SS");
				return (false);
			}
		}
		break; // of switch gMountConfig.mount : case kFORK

	// *** If a GERMAN mount, germans worry about crashing the scope into the mount pier
	// *** A lesser worry is having the counterweigth higher than the scope, but that style points ;^)
	case kGERMAN:
		// check if a german mount would be upside down eg. counterweight higher than scope by 0.5H/7.5degs
		germanUpsideDown = Servo_check_german_for_upside_down(startRaHa, raStdPath, 0.5);

		// Determine the type of path the german mount will make regarding crossing the meridian
		switch (raStdPathType)
		{
		case EAST_TO_EAST:
		case WEST_TO_WEST:
			// this move stays on it's current side of the meridian plus the flip window
			// but check to see if german mount goes "upside down"
			if (germanUpsideDown == true)
			{
				// Mount would end up being upside down so do a flip
				*raDirection = raFlipPath;
				*decDirection = decFlipPath;
				strcpy(gDebugInfoCOP, "GFUSD");
				return (true);
			}
			else
			{
				// The mount stays upright so it's a just a simple slew
				*raDirection = raStdPath;
				*decDirection = decStdPath;
				strcpy(gDebugInfoCOP, "G--SS");
				return (false);
			}
			break;

		// if starting on the west side and std path moves east past LST and flip window
		case WEST_TO_EAST:
			// this move crosses meridian from the west and goes past the GEM's meridian flip window, so flip and return
			*raDirection = raFlipPath;
			*decDirection = decFlipPath;
			strcpy(gDebugInfoCOP, "GFW2E");
			return (true);
			break;

		// if starting on the east side and std path moves west past LST and flip window
		case EAST_TO_WEST:
			// this move crosses meridian from the east and goes past the GEM's meridian flip window, so flip and return
			*raDirection = raFlipPath;
			*decDirection = decFlipPath;
			strcpy(gDebugInfoCOP, "GFE2W");
			return (true);
			break;

		default:
			// Should never get here as it would be handled by the above cases
			// Never should get here, but just-in-case, do the least amount of damage
			*raDirection = 0.0;
			*decDirection = 0.0;
			strcpy(gDebugInfoCOP, "ERROR");
			return (false);
			break;
		}
		break; // of switch gMountConfig.mount : case kGERMAN

		// TODO: case ALTAZI will go here

	default:
		// It should never get here since the config files error handling would have caught unsupported mount
		printf("Error: (Servo_calc_optimal_path) - MOUNT TYPE %d NOT SUPPORTED\n", gMountConfig.mount);
		break;
	} // of switch gMountConfig.mount

	// Never should get here, but just-in-case, do the least amount of damage
	*raDirection = 0.0;
	*decDirection = 0.0;
	strcpy(gDebugInfoCOP, "ERROR");
	return (false);
} // of Servo_calc_optimal_path()

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
		currRaStep	+=	(uint32_t)(gMountConfig.ra.direction * UNDER_TARGET * gMountConfig.ra.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currRaStep	-=	(uint32_t)(gMountConfig.ra.direction * UNDER_TARGET * gMountConfig.ra.step);
	}
	// Take care of the Dec axis
	if (ss__get_tracking(SERVO_DEC_AXIS) >= 0)
	{
		// tracking is in the positive direction come in on the negative side of the target
		currDecStep	+=	(uint32_t)(gMountConfig.dec.direction * UNDER_TARGET * gMountConfig.dec.step);
	}
	else
	{
		// tracking is in the negative direction come in on the positive side of the target
		currDecStep	-=	(uint32_t)(gMountConfig.dec.direction * UNDER_TARGET * gMountConfig.dec.step);
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
int32_t	targetRaStep, targetDecStep;
double	targetRa, targetDec;
double	raRelDir, decRelDir;
double	currRa, currDec;
double	degsRa;
double	direction;
double	lst;
double	alt, azi;
int		status	=	kSTATUS_OK;
bool	flip	=	false;

	printf("&&& Servo_move_to_coordins() RA:%.4f Dec:%.4f Lat:%.4f Lon:%.4f Park:%d\n", gotoRa, gotoDec, lat, lon, gParkState);
	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// convert ra hours to decidegs for alt-azi call
	degsRa	=	gotoRa;
	Time_deci_hours_to_deg(&degsRa);

	// get the LST
	lst	=	Servo_get_lst();
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
	flip	=	Servo_calc_optimal_path(currRa, currDec, lst, gotoRa, gotoDec, &raRelDir, &decRelDir);
	printf("\n&&& Servo_move_to_coordins()  CurrRa = %lf	CurrDec = %lf 	LST = %lf	flip = %d	COP:%s\n", currRa, currDec, lst, (int)flip, gDebugInfoCOP);
	printf("&&& Servo_move_to_coordins()  gotoRa = %lf	gotoDec = %lf 	raRelDir = %lf	decRelDir = %lf\n", gotoRa, gotoDec, raRelDir, decRelDir);

	// If flip is returned, you must flip the mount otherwise the rel dir for RA/Dec will be incorrect
	if (flip == true)
	{
		// Do the actual flip on the current coordins
		// Current coordins are guaranteed to change after this call
		Servo_calc_flip_coordins(&currRa, &currDec, &direction, &gMountConfig.side);
		Servo_set_pos(currRa, currDec);
		printf("&&& Servo_move_to_coordins() After flip CurrRa = %lf	CurrDec = %lf\n\n", currRa, currDec);

		// Compare to the orignal direction values for Dec axis from the mount config file
		if (Servo_is_TTP() == true)
		{
			// set the direction to the opposite from config file
			gMountConfig.dec.direction	=	(gMountConfig.dec.parkInfo > 0.0) ? kREVERSE : kFORWARD;
		}
		else
		{
			// side equals the value in the config file, so set direction to config file value
			gMountConfig.dec.direction	=	(double)gMountConfig.dec.parkInfo;
		}
	}

	// determine the new target based on the  RA/Dec relative direction
	targetRa	=	currRa + raRelDir;  // BUG!  
	targetDec	=	currDec + decRelDir;

	// convert the target RA & Dec to target steps
	Servo_pos_to_step(targetRa, targetDec, &targetRaStep, &targetDecStep);
	printf("&&& Servo_move_to_coordins()  targRa = %lf	targDec = %lf 	targRaStep = %d	targDecStep = %d\n\n", targetRa, targetDec, targetRaStep, targetDecStep);

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:  add back in gearlash,  KLUDGE!
	/////////////////////////////////////////////////////////////////////////////////////

	status	-=	Servo_move_step_track(targetRaStep, targetDecStep);

	// If status is < zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_to_coordins()

//*****************************************************************************
// This routine is for stopping the mount for extended periods of time and
// uses and Hour Angle and not RA for an accurate static position. If the
// mount flipped, it will unflip and then just converts the HA/Dec to
// current RA/Dec and then moves the mount with tracking off.
// Inputs are in deciHour and deciDeg and routine is asynchronous so you need to
// to poll with Servo_state() and wait for STOPPED state
// TODO: need to pull the tracking checks out since not needed for future alt-azi
//*****************************************************************************
int Servo_move_to_static(double parkHA, double parkDec)
{
int32_t	targetRaStep, targetDecStep;
double	targetRa, targetDec;
double	currRa, currDec;
double	dummy	=	0;
double	currHA, raRelDir;
double	lst;
int		status	=	kSTATUS_OK;

	printf("@@@ Servo_move_to_static()  parkHA:%.6f parkDec:%.6f\n", parkHA, parkDec);

	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// Setting the dec target is easy, only updated if mount needs to flip
	targetDec	=	parkDec;

	printf("@@@ Servo_move_to_static()  .side:%d .ra.parkInfo:%d\n", gMountConfig.side, gMountConfig.ra.parkInfo);

	// RA target is harder, so get where the mount is now
	Servo_get_pos(&currRa, &currDec);

	// if the mount is currently thru-the-pole or flipped from config file setting
	if (Servo_is_TTP() == true)
	{
		printf("@@@ Servo_move_to_static()  TTP == true\n");

		// Unflip the mount and update targetDec to reflect flipped value
		Servo_calc_flip_coordins(&currRa, &currDec, &dummy, &gMountConfig.side);
		targetDec	=	currDec;

		// Since side now equals the value in the config file, so set direction from config file value
		gMountConfig.dec.direction	=	(double)gMountConfig.dec.parkInfo;
	}

	// get the LST and calc HA of the current RA position
	lst		=	Servo_get_lst();
	currHA	=	lst - currRa;

	printf("@@@ Servo_move_to_static()  currHA:%.6f lst:%.6f currRa:%.6f\n", currHA, lst, currRa);
	// calc relative hours needed for RA by subtracting current pos HA from the input park HA
	raRelDir	=	currHA - parkHA;		// this calc seems backwards, but it works when modelled 
	printf("@@@ Servo_move_to_static()  parkHA:%.6f currHA:%.6f\n", parkHA, currHA);

	// determine the new RA target in hours based current RA plus relative direction
	targetRa	=	currRa + raRelDir;
	printf("@@@ Servo_move_to_static()  targetRa:%.6f currRa:%.6f raRelDri:%.6f\n", targetRa, currRa, raRelDir);

	// convert the target RA & Dec to target steps
	Servo_pos_to_step(targetRa, targetDec, &targetRaStep, &targetDecStep);
	printf("@@@ Servo_move_to_static() targRa = %lf	targDec = %lf 	targRaStep = %d	targDecStep = %d\n\n", targetRa, targetDec, targetRaStep, targetDecStep);

	// Check the status and if not zero, decrement
	status	-=	Servo_move_step(targetRaStep, targetDecStep, false);

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
int32_t	targetRaStep, targetDecStep;
double 	currRa, currDec; 
int		status;

	// printf("@@@ Servo_move_to_park() gParkState = %d\n", gParkState);
	// Make sure the mount is not Parked, but *if parked* then return error
	if (gParkState == true)
	{
		return kERROR;
	}

	// if the mount is inverted, then unflip the current coordinates
	if (Servo_is_TTP() == true)
	{
		Servo_get_pos(&currRa, &currDec);
		Servo_calc_flip_coordins(&currRa, &currDec, &gMountConfig.dec.direction, &gMountConfig.side);
		Servo_set_pos(currRa, currDec);

		// Now check to see if the mount got messed up some how
		if (gMountConfig.side != gMountConfig.ra.parkInfo)
		{
			printf("ERROR: (Servo_move_to_park) ra.side is different from ra.parkInfo\n");
		}
		if (gMountConfig.dec.direction != gMountConfig.dec.parkInfo)
		{
			printf("ERROR: (Servo_move_to_park) dec.direction is different from dec.parkInfo\n");
		}

		// Set the mount coordins to the flipped position
		Servo_set_pos(currRa, currDec);
	} 
	// Get the absolute zeros which is the original park position for the session
	Motion_get_axis_absZero(SERVO_RA_AXIS, &targetRaStep);
	Motion_get_axis_absZero(SERVO_DEC_AXIS, &targetDecStep);

	// Allow a move to anywhere and set the action state, then  move to static pos
	Servo_ignore_horizon(true);
	gMountAction	=	PARKING;

	// Move to the orignal zeros of both axes and check status
	status	=	Servo_move_step(targetRaStep, targetDecStep, false);

	// Force flags back to original config file state for being parked, just in case
	gMountConfig.side = gMountConfig.ra.parkInfo;
	gMountConfig.dec.direction = gMountConfig.dec.parkInfo;

	// printf("@@@ Servo_move_to_park() status = %d\n", status);
	// If status is not zero, return error
	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
} // of Servo_move_to_park()
//******************************************************************************
// int Servo_move_to_park_old(void)
// {
// double	ha, dec;
// int		status;

// 	// printf("@@@ Servo_move_to_park() gParkState = %d\n", gParkState);
// 	// Make sure the mount is not Parked, but *if parked* then return error
// 	if (gParkState == true)
// 	{
// 		return kERROR;
// 	}

// 	// Get the park coordins set in mount config file
// 	Servo_get_park_coordins(&ha, &dec);
// 	// Allow a move to anywhere and set the action state, then  move to static pos
// 	Servo_ignore_horizon(true);
// 	gMountAction	=	PARKING;
// 	// printf("@@@ Servo_move_to_park() ha:%.6f dec:%.6f\n", ha, dec);

// 	status	=	Servo_move_to_static(ha, dec);

// 	// printf("@@@ Servo_move_to_park() status = %d\n", status);
// 	// If status is < zero, return error
// 	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
// } // of Servo_move_to_park_old()

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
// 	printf("ax->encoderMaxRate = %d\n", ax->encoderMaxRate);
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
// #define _TEST_SERVO_MOUNT_
#ifdef _TEST_SERVO_MOUNT_
int main(void)
{
//	double	ra, dec, ha;
double		parkHa, parkDec;
double		jd, gmst, lst;
double		lat, lon;
double		currRa, currDec;
int32_t		currRaStep, currDecStep;
double		propo, integ, deriv;
uint32_t	iMax, deadZ;
int32_t		minP, maxP;
uint8_t		raState, decState;

uint32_t	rcStatus = 0xFFFFFFFF; 

int state;
char buf[256];

	printf("\nSERVO_TEST unit test program\n");
//-	Servo_init("servo_observatory.cfg", "servo_mount.cfg", "servo_motion.cfg");
	Servo_init(kOBSERV_CFG_FILE, kMOUNT_CFG_FILE, kMOTION_CFG_FILE);

	Servo_stop_axes(SERVO_BOTH_AXES);

	jd		=	Time_systime_to_jd();
	gmst	=	Time_jd_to_gmst(jd);
	lst		=	Time_gmst_to_lst(gmst, Servo_get_lon());
	printf("@@@ current time is jd:%.5lf GMST:%.5lf LST:%.5lf\n", jd, gmst, lst);

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

	RC_get_pos_pid(128, SERVO_RA_AXIS,  &propo, &integ, &deriv, &iMax, &deadZ, &minP, &maxP);
	printf("POS P:%.2f I:%.2f D:%.2f iMax:%d Dz:%d Min:%d Max:%d\n", propo, integ, deriv, iMax, deadZ, minP, maxP);

	RC_get_vel_pid(128, SERVO_RA_AXIS,  &propo, &integ, &deriv, &iMax);
	printf("VEL P:%.2f I:%.2f D:%.2f QPPS:%d\n", propo, integ, deriv, iMax);

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

	jd		=	Time_systime_to_jd();
	gmst	=	Time_jd_to_gmst(jd);
	lst		=	Time_gmst_to_lst(gmst, Servo_get_lon());
	printf("@@@ current time is jd:%.5lf GMST:%.5lf LST:%.5lf\n", jd, gmst, lst);

	currRa	=	lst - parkHa;
	Time_normalize_RA(&currRa);
	currDec	=	parkDec;
	printf("LST:%.5lf After park - curr RA = %lf dec = %lf\n", lst, currRa, currDec);
	Servo_set_pos(currRa, currDec);

	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	Servo_set_pos(currRa, currDec);
	printf("** Setting postion to same value\n");
	Servo_get_pos(&currRa, &currDec);
	printf("** Current Pos  RA = %lf   Dec = %lf\n", currRa, currDec);

	Servo_unpark();

	lat	=	Servo_get_lat();
	lon	=	Servo_get_lon();

	printf("********************************************************\n");
	printf("Move 1: Simple -1.0 hr -15.0 deg relative, at LST:%lf\n", lst);
	printf("********************************************************\n");
	// Reset current RA/Dec back to the park position
	currRa	=	lst - parkHa;
	Time_normalize_RA(&currRa);
	currDec	=	parkDec;
	Servo_set_pos(currRa, currDec);
	printf("* Current  Pos  RA = %lf   Dec = %lf\n", currRa, currDec);

	currRa	+=	-1.0;
	currDec	+=	-15.0;
	
	//Servo_ignore_horizon(true);
	printf("* Target Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	printf("********************************************************\n");
	Servo_move_to_coordins(currRa, currDec, lat, lon);

	state	=	Servo_state();
	while (state == MOVING)
	{
		sleep(3);
		Servo_get_pos(&currRa, &currDec);
		state	=	Servo_state();
		RC_check_queue(128, &raState, &decState);
		//rcStatus = RC_get_status(SERVO_RA_AXIS, &rcStatus);
		printf("### Current Pos  RA = %lf   Dec = %lf   gDebugInfoSS = %s Qra:%d Qdec:%d rcStatus:%x\n", currRa, currDec, gDebugInfoSS, raState, decState, rcStatus);
	}

	printf("\nhit any key to begin next move\n");
	fgets(buf, 256, stdin);

	printf("********************************************************\n");
	printf("Move 2: Simple =-3.0 hr -40 deg relative with meridian crossing at LST:%lf\n", lst);
	printf("********************************************************\n");
	printf("* Current  Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	currRa 	+=	-3.0;
	currDec	+=	-40.0;
	printf("* Target Pos  RA = %lf   Dec = %lf\n", currRa, currDec);
	printf("********************************************************\n");

	sleep(1);

	Servo_move_to_coordins(currRa, currDec, lat, lon);
	RC_check_queue(128, &raState, &decState);
	while (raState == 2)
	{
		printf("raState is 2\n");
		Servo_move_to_coordins(currRa, currDec, lat, lon);
		RC_check_queue(128, &raState, &decState);
	}


	sleep(1);

	state	=	Servo_state();
	while (state == MOVING)
	{
		sleep(3);
		Servo_get_pos(&currRa, &currDec);
		state	=	Servo_state();
		RC_check_queue(128, &raState, &decState);
		printf("** Current Pos  RA = %lf   Dec = %lf   gDebugInfoSS = %s Qra:%d Qdec:%d\n", currRa, currDec, gDebugInfoSS, raState, decState);

	}

	printf("## Target RA tracking %d\n", gMountConfig.ra.trackRate);
	state	=	Servo_state();
	while (state == TRACKING)
	{
		sleep(2);
		RC_get_curr_vel(128, SERVO_RA_AXIS, &currRaStep);
		state	=	Servo_state();
		printf("** Current velocity  RA = %d  gDebugInfoSS = %s\n", currRaStep, gDebugInfoSS);
	}
	printf("STOPPING motors\n");
	Servo_stop_axes(SERVO_BOTH_AXES);

	return (0);
}
#endif // _TEST_SERVO_MOUNT_
