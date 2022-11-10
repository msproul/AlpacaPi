//******************************************************************************
//*	Name:			servo_motion.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: header defs for the abstracted Motion commands
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
//*	Jul  2,	2022	<RNS> Initial version of this file via cproto
//*	Jul  3,	2022	<RNS> Updated via cproto
//*	Jul  5,	2022	<RNS> Updated via cproto
//*	Oct 28,	2022	<RNS> Updated via cproto
//*	Nov  1,	2022	<RNS> Updated via cproto
//*	Nov  9,	2022	<RNS> Updated via cproto
//****************************************************************************
//#include	"servo_motion.h"

#ifndef _SERVO_MOTION_H_
#define _SERVO_MOTION_H_

#ifndef	_INCLUDED_SERVO_STD_DEFS_
	#include	"servo_std_defs.h"
#endif


#ifndef	_SERVO_MOTION_CFG_H_
	#include	"servo_motion_cfg.h"
#endif


#ifdef __cplusplus
	extern "C" {
#endif


TYPE_MOTION_MOTOR *Motion_get_motor_ptr(uint8_t motor);
int Motion_set_axis_acc(uint8_t axis, uint32_t acc);
int Motion_get_axis_acc(uint8_t axis, uint32_t *acc);
int Motion_set_axis_vel(uint8_t axis, uint32_t vel);
int Motion_get_axis_vel(uint8_t axis, uint32_t *vel);
int Motion_set_axis_trackRate(uint8_t axis, int32_t track);
int Motion_get_axis_trackRate(uint8_t axis, int32_t *track);
int Motion_set_axis_absZero(uint8_t axis, int32_t count);
int Motion_get_axis_absZero(uint8_t axis, int32_t *zero);
int Motion_get_axis_curr_step(uint8_t axis, int32_t *step);
int Motion_get_axis_curr_vel(uint8_t axis, int32_t *vel);
int Motion_set_axis_buffer(uint8_t axis, _Bool state);
TYPE_MOTION_STATE Motion_get_axis_state(uint8_t axis);
int Motion_get_pending_cmds(uint8_t *raState, uint8_t *decState);
int Motion_set_axis_profile(uint8_t axis);
double Motion_calc_axis_move_time(uint8_t axis, int32_t start, int32_t end);
int Motion_wait_axis_buffer_clear(uint8_t axis);
int Motion_move_axis_by_step(uint8_t axis, int32_t step);
int Motion_move_axis_by_vel(uint8_t axis, int32_t vel);
int Motion_move_axis_by_time(uint8_t axis, int32_t vel, double seconds);
int Motion_stop_axis(uint8_t axis);
int Motion_set_axis_zero(uint8_t axis);
int Motion_reset_axis(uint8_t axis);
int Motion_init(const char *motionCfgFile);


#ifdef __cplusplus
}
#endif

#endif // _SERVO_MOTION_H_
