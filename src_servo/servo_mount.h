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
//****************************************************************************
//#include	"servo_mount.h"


#ifndef _SERVO_MOUNT_H_
#define _SERVO_MOUNT_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

//****************************************************************************
typedef enum
{
	STOPPED	=	0,
	MOVING,
	TRACKING
}	TYPE_MOVE;

#ifdef __cplusplus
	extern "C" {
#endif

int			Servo_set_comm_port(char com[], int baud);
void		Servo_get_park_coordins(double *ha, double *dec);
void		Servo_get_standby_coordins(double *ha, double *dec);
void		Servo_get_sync_coordins(double *ra, double *dec);
void		Servo_set_time_ratio(double ratio);
double		Servo_get_time_ratio(void);
int			Servo_scale_acc(int32_t percentRa, int32_t percentDec);
int			Servo_scale_vel(int32_t percentRa, int32_t percentDec);
int			Servo_ignore_horizon(int state);
int			Servo_reset_motor(uint8_t motor);
void		Servo_step_to_pos(int32_t raStep, int32_t decStep, double *ra, double *dec);
void		Servo_pos_to_step(double ra, double dec, int32_t *raStep, int32_t *decStep);
void		Servo_get_pos(double *ra, double *dec);
void		Servo_set_pos(double ra, double dec);
void		Servo_set_static_pos(double ha, double dec);
int32_t		Servo_get_tracking(uint8_t motor);
int			Servo_set_tracking(uint8_t motor, int32_t tracking);
int			Servo_start_tracking(uint8_t motor);
int			Servo_stop_tracking(uint8_t motor);
int			Servo_init(const char *mountCfgFile, const char *localCfgFile);
TYPE_MOVE	Servo_state(void);
//int       	Servo_move_step_track(int32_t raStep, int32_t decStep);
//int			Servo_move_step(int32_t raStep, int32_t decStep, bool buffered);
void		Servo_calc_flip_coordins(double *ra, double *dec, double *direction, int8_t *side);
void		Servo_stop_all(void);
bool		Servo_calc_optimal_path(double startRa, double startDec, double lst, double endRa, double endDec, double *raDirection, double *decDirection);
void 		Servo_spiral_move(double raMove, double decMove, int loop);
int			Servo_move_to_coordins(double gotoRa, double gotoDec, double lat, double lon);
int			Servo_move_to_static(double parkHA, double parkDec);

#ifdef __cplusplus
}
#endif

#endif // of _SERVO_MOUNT_H_