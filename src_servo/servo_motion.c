//******************************************************************************
//*	Name:			servo_motion.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Servo motor motion command that are ported to motor controller
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
//*	Jun  29,	2022	<RNS> Created initial version of this file
//*	Jul   1,	2022	<RNS> changes globals to static, added access routines
//*	Jul   2,	2022	<RNS> Added more and more access routines 
//*	Jul   3,	2022	<RNS> Moved MC_comm initialization to Motion_init()
//*	Jul   4,	2022	<RNS> Implemented Motion_get_axis_state()
//*	Jul   4,	2022	<RNS> Implemented Motion_move_axis_by_time
//*****************************************************************************

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <errno.h>
#include "servo_std_defs.h"
#include "servo_mc_core.h"
#include "servo_rc_utils.h"
#include "servo_motion_cfg.h"
#include "servo_motion.h"

#define _ENABLE_CONSOLE_DEBUG_
#include "ConsoleDebug.h"

static TYPE_MOTION_CONFIG gsMotionConfig; 

//*************************************************************************
// Return the address of the motor struct from the specified axis RA or Dec
// from the local global gsMotionConfig
//*************************************************************************
TYPE_MOTION_MOTOR *Motion_get_motor_ptr(uint8_t motor)
{
	switch (motor)
	{
	case SERVO_RA_AXIS:
		return &gsMotionConfig.motor0; 
		break;
	case SERVO_DEC_AXIS:
		return &gsMotionConfig.motor1; 
		break;
	default:
		return NULL;
	} // of switch
}

//*************************************************************************
// Sets  the specified axis's motor acceleration data field
// Returns error if the axis ID is out of range, otherwise OK
//*************************************************************************
int Motion_set_axis_acc(uint8_t axis, uint32_t acc)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// load the acceleration data field
	motor->acc = acc; 
	return kSTATUS_OK;
 }

//*************************************************************************
// Gets the specified axis's stored motor acceleration data field
// Returns error if the axis ID is out of range, otherwise OK
//*************************************************************************
int Motion_get_axis_acc(uint8_t axis, uint32_t *acc)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set input acc ptr to the acceleration data field
	*acc = motor->acc; 
	return kSTATUS_OK;
}

//*************************************************************************
// Sets the specified axis's velocity data field for the motor
// Returns error if the axis ID is out of range, otherwise OK
//*************************************************************************
int Motion_set_axis_vel(uint8_t axis, uint32_t vel)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set the velocity data field
	motor->vel = vel; 
	return kSTATUS_OK;
}

//*************************************************************************
// Gets the specified axis's stored velocity data field
// Returns error if the axis ID is out of range, otherwise OK
//*************************************************************************
int Motion_get_axis_vel(uint8_t axis, uint32_t *vel)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set input acc ptr to the velocity data field
	*vel = motor->vel; 
	return kSTATUS_OK;
}

//*************************************************************************
// Sets the specified axis's velocity tracking speed field
// Returns error if the axis ID is out of range, otherwise OK
//*************************************************************************
void Motion_set_axis_trackRate(uint8_t axis, int32_t track)
{
	TYPE_MOTION_MOTOR *motor;

	// find the correct motor data
	motor = Motion_get_motor_ptr(axis);

	// set the trackRate data field
	motor->trackRate = track; 
	return;
}

//*****************************************************************************
// Returns the current tracking rate from motor controller axis in steps/sec
//*****************************************************************************
int32_t Motion_get_axis_trackRate(uint8_t axis)
{
	TYPE_MOTION_MOTOR *motor;

	// find the correct motor data
	motor = Motion_get_motor_ptr(axis);

	// return the tracking rate
	return motor->trackRate; 
} // of Motion_get_axis_trackRate()

//*************************************************************************
// Returns the specified axis's position in signed steps
//*************************************************************************
int Motion_get_axis_curr_step(uint8_t axis, int32_t *step)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set pos in steps
	RC_get_curr_pos(motor->addr, axis, step);
	return kSTATUS_OK;
}

//*************************************************************************
// Returns the specified axis's velocity in steps / second
//*************************************************************************
int Motion_get_axis_curr_vel(uint8_t axis, int32_t *vel)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set vel in steps/sec
	RC_get_curr_vel(motor->addr, axis, vel);
	return kSTATUS_OK;	
}

//*************************************************************************
// Enables or Disables buffered moves on axis
//*************************************************************************
int Motion_set_axis_buffer(uint8_t axis, bool state)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// set the axix buffered state to arg value
	motor->buffered = state; 
	return kSTATUS_OK;	
}

//*************************************************************************
// Returns the specified axis's state of motion and the 3 return types 
// are: STOPPED, MOVING_BY_POS, MOVING_BY_VEL
//*************************************************************************
TYPE_MOTION_STATE Motion_get_axis_state(uint8_t axis)
{
	uint8_t raState, decState, cmdState;
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// Get the command state for the correct motor
	Motion_get_pending_cmds(&raState, &decState);
	cmdState = (axis == SERVO_RA_AXIS) ? raState : decState; 

	if (cmdState == kRC_CMD_QUEUE_EMPTY)
	{
		motor->state = COMPLETED; 
	}
	// Otherwise return the current state set by _move_ routines
	return motor->state;	
 }

//*************************************************************************
// Set the MC specific data structures needed for operations, includes PID
// and other required values specific to the motion controller 
//*************************************************************************
int Motion_set_axis_profile(uint8_t axis)
{
	TYPE_MOTION_MOTOR *motor;

	// Get the ptr to the specified motor's data structure
	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		// If ptr is NULL, return error
		return kERROR; 
	}

	// Set the PID value in MC with the config file data
	RC_set_pos_pid(	motor->addr, 
					axis,
					motor->kp,
					motor->ki,
					motor->kd,
					(uint32_t) motor->il,
					0, // DeadZone
					(int32_t) -kSTEP_FOREVER,  
					(int32_t) kSTEP_FOREVER ); 

	// Need to set QPPS or the MC will not move, the only way is using vel PID
	RC_set_vel_pid(	motor->addr, 
					axis,
					motor->kp / 40.0, 	// convert to internal data format (divide pos value by 40)
					motor->ki / 40.0, 	// convert to internal data format (divide pos value by 40)
					0.0, 									// it's really a PI cmd so zero D as it's not needed							
					(uint32_t) motor->encoderMaxSpeed );	// calc'd max step rate from above

	// TODO: temporary
	// RC_write_settings(axis);
	return kSTATUS_OK;
} // of Motion_set_axis_profile()

//*************************************************************************
// Calculates and return the duration time floating seconds for the axis 
// move in steps based on the stored acceleration and velocity parameters 
//*************************************************************************
double Motion_calc_axis_move_time(uint8_t axis, int32_t start, int32_t end)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	return RC_calc_move_time(start, end, motor->vel, motor->acc);
}

//*************************************************************************
// Moves the specified axis by a signed number of steps, can be buffered 
// or unbuffered command depending the latest axis_dis/enable_buffer() call 
//*************************************************************************
int Motion_move_axis_by_step(uint8_t axis, int32_t step)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// Update the motor state and send move cmd
	motor->state = MOVING_BY_POS; 
	RC_move_by_posva(motor->addr, axis, step, motor->vel, motor->acc, motor->buffered);
	printf("!!! motor-addr:%d axis:%d step:%d, motor->vel:%d motor->acc:%d motor->buffered:%d\n", motor->addr, axis, step, motor->vel, motor->acc, motor->buffered);
	return kSTATUS_OK;	
}

//*************************************************************************
// Start the specified axis moving at a constant velocity after the acceleration
// ramp and can be buffered or unbuffered command depending the latest 
// axis_dis/enable_buffer() call 
//*************************************************************************
int Motion_move_axis_by_vel(uint8_t axis, int32_t vel)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// Update the motor state and send move cmd
	motor->state = MOVING_BY_VEL; 
	RC_move_by_vela(motor->addr, axis, vel, motor->acc, motor->buffered);
	return kSTATUS_OK;	
}

//*************************************************************************
// Start the specified axis moving at a constant velocity after the acceleration
// ramp and can be buffered or unbuffered command depending the latest 
// axis_dis/enable_buffer() call 
//*************************************************************************
int Motion_move_axis_by_time(uint8_t axis, int32_t vel, double seconds)
{
	// double 	propo, integ, deriv;
	// uint32_t iMax, deadZ;
	// int32_t minP, maxP;
	int32_t		dist, startVel;
	uint32_t 	acc; 
	int			status; 
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// Update the motor state and send move cmd
	motor->state = MOVING_BY_TIME; 
	startVel = Motion_get_axis_trackRate(axis);
	Motion_get_axis_acc(axis, &acc);

	// Calc the move distance for the time period assuming a rapid decel 
	dist = RC_calc_move_distance(startVel, vel, acc, seconds);
	
	// // This nows get funky, increasing the deadband to be equal to velocity so save PID settings
	// RC_get_pos_pid(motor->addr, axis,  &propo, &integ, &deriv, &iMax, &deadZ, &minP, &maxP);
	// // This should allow the move cmd to complete without a decel stage
	// RC_set_pos_pid(motor->addr, axis,  propo, integ, deriv, iMax, vel, minP, maxP);

	// Move by position for the time duration with really fast decel
	status = RC_move_by_posvad(motor->addr, axis, dist, motor->vel, motor->acc, 0, false);

	// Do a 2nd buffered move by velocity if there was tracking rate
	if (startVel != 0)
	{
		status -= RC_move_by_vela(motor->addr, axis, vel, motor->acc, true);
	}

	return (status == kSTATUS_OK) ? kSTATUS_OK : kERROR;
}

//*************************************************************************
// Stop the motor on the specified axis
//*************************************************************************
int Motion_stop_axis(uint8_t axis)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}
	// Update the motor state and stop the motor
	motor->state = COMPLETED; 
	RC_stop(motor->addr, axis);
	return kSTATUS_OK;
}
//*************************************************************************
// Resets the specified axis to a known good state
//*************************************************************************
int Motion_reset_axis(uint8_t axis)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}	
	Motion_stop_axis(axis); 
	Motion_set_axis_profile(axis);
	Motion_set_axis_buffer(axis, false); 
	RC_set_home(motor->addr, axis);
	return kSTATUS_OK;
}

//*************************************************************************
// Sets the axis zero position, formerly known as 'home'
//*************************************************************************
int Motion_set_axis_zero(uint8_t axis)
{
	TYPE_MOTION_MOTOR *motor;

	motor = Motion_get_motor_ptr(axis);
	if (motor == NULL)
	{
		return kERROR; 
	}	
	RC_set_home(motor->addr, axis);
	return kSTATUS_OK;
}

//*************************************************************************
// Returns the pending commands using RC notation... for now
// TODO: Convert the RC notation to simple zero based numbering
// TOTO: But that will break Servo_state()... so caution for now
//*************************************************************************
int Motion_get_pending_cmds(uint8_t *raState, uint8_t *decState)
{
	// just a wrapper for the RC call, use the RA or dec addr field, they are the same
	return RC_check_queue(gsMotionConfig.motor0.addr, raState, decState);
}

//*************************************************************************
int Motion_init(const char *motionCfgFile)
{
	int status; 

	status = Servo_read_motion_cfg(motionCfgFile, &gsMotionConfig);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (Motion_init) Could not open motion configuration file '%s'.\n", motionCfgFile);
		return (kERROR);
	}

	// Initialize the port from the mount config file for communication
	// Used the port and baud info from the motion config file
	status = MC_init_comm(gsMotionConfig.port, gsMotionConfig.baud);
	if (status != kSTATUS_OK)
	{
		printf("FATAL: (servo_init) Could not initialize comm port '%s'.\n", gsMotionConfig.port);
		return (kERROR);
	}

	// Initialized the motors with a reset and then reload profiles
	Motion_reset_axis(SERVO_RA_AXIS); 
	Motion_reset_axis(SERVO_DEC_AXIS); 
	
	return (kSTATUS_OK);
}

//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
#ifdef _TEST_SERVO_MOTION_
int main(void)
{
	int status;
	uint8_t raState, decState;

	status = Motion_init("servo_motion.cfg");
	if (status != kSTATUS_OK)
	{

		Motion_get_pending_cmds(&raState, &decState);

		printf("raState:%d  decState:%d\n", raState, decState);
	}
	return 0;
}
#endif // _TEST_SERVO_MOTION_
