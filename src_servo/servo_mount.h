//******************************************************************************
//*	Name:			servo_mount.h
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
//*	May  6,	2022	<RNS> Created servo_scope.h using cproto
//*	May 14,	2022	<RNS> Changed all _step routines to signed variables
//*	May 16,	2022	<RNS> Added Servo_pos_to_step()
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> Changed gIgnoreHorizon from int to bool
//*	May 20,	2022	<RNS> Added park state functions plus support in Servo_state()
//*	May 21,	2022	<RNS> added Marked the internal routines static and commented out from .h
//*	May 22,	2022	<RNS> Changed routines using steps as arg to _step_ and marked internal
//*	May 22,	2022	<RNS> added _axis_ to functions that worked a single axis
//*	May 22,	2022	<RNS> added move_axis_by_vel for the ASCOM guiding functionality
//*	May 22,	2022	<RNS> General cleanup to get data/control flow aligned with ASCOM
//* Jun  6,	2022	<RNS> regenerated *all* function declarations with cproto
//* Jun 17,	2022	<RNS> regenerated *all* function declarations with cproto
//* Jun 19,	2022	<RNS> changed Servo_park() return type from bool to int
//****************************************************************************

#ifndef _SERVO_MOUNT_H_
#define _SERVO_MOUNT_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

//****************************************************************************
typedef enum
{
	PARKED 	=	0,
	PARKING,
//	HOMED,
//	HOMING,
	STOPPED,
	MOVING,
	TRACKING
}	TYPE_MOVE;

#ifdef __cplusplus
	extern "C" {
#endif

// servo_mount.c 
int Servo_set_comm_port(char com[], int baud);
void Servo_get_park_coordins(double *ha, double *dec);
void Servo_get_standby_coordins(double *ha, double *dec);
void Servo_get_sync_coordins(double *ra, double *dec);
void Servo_set_time_ratio(double ratio);
double Servo_get_time_ratio(void);
int Servo_unpark(void);
bool Servo_get_park_state(void);
int8_t Servo_get_pier_side(void);
int Servo_scale_acc(int32_t percentRa, int32_t percentDec);
int Servo_scale_vel(int32_t percentRa, int32_t percentDec);
bool Servo_ignore_horizon(bool state);
int Servo_reset_motor(uint8_t motor);
void Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec);
void Servo_get_pos(double *ra, double *dec);
void Servo_set_pos(double ra, double dec);
void Servo_set_static_pos(double ha, double dec);
int32_t Servo_get_axis_step_track(uint8_t motor);
int Servo_set_axis_step_track(uint8_t motor, int32_t tracking);
int Servo_start_tracking(uint8_t motor);
int Servo_stop(uint8_t motor);
int Servo_move_axis_by_vel(uint8_t motor, double vel);
int Servo_init(const char *mountCfgFile, const char *localCfgFile);
TYPE_MOVE Servo_state(void);
double Servo_calc_short_vector(double begin, double end, double max);
bool Servo_calc_optimal_path(double startRa, double startDec, double lst, double endRa, double endDec, double *raDirection, double *decDirection);
int Servo_move_spiral(double raMove, double decMove, int loop);
int Servo_move_to_coordins(double gotoRa, double gotoDec, double lat, double lon);
int Servo_move_to_static(double parkHA, double parkDec);
int Servo_move_to_park(void);

#ifdef __cplusplus
}
#endif

#endif // of _SERVO_MOUNT_H_
