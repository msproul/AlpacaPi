//******************************************************************************
//*	Name:			servo_rc_utils.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Roboclaw basic utilities for servo telescope mount control
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
//*	Apr 27,	2022	<RNS> Created servo_rc_utils.h using cproto
//*	Apr 27,	2022	<RNS> add defines for RC return status
//*	Apr 28,	2022	<RNS> add support for buffered moves & return status masks
//*	Apr 28,	2022	<RNS> added vel_raw and vel_buffered (now default)
//*	Apr 28,	2022	<RNS> added calc_move_time
//*	May  6,	2022	<RNS> fixed includes with servo_*c_* file name changes
//*	May  8,	2022	<RNS> fadd a & va suffixes to the move_by functions
//*	May 21,	2022	<RNS> added addr arg to RC_ MC cmds for multi-RC support
//*	May 22,	2022	<RNS> corrected some signness for pos and vel args
//*	May 22,	2022	<RNS> added buffered capability to move_by_vela() 
//*	Jun 12	2022	<RNS> Updated functions list using cproto
//*	Jun 13	2022	<RNS> Converted all PID function to use float for PID args
//****************************************************************************
//#include "servo_rc_utils.h"


#ifndef _SERVO_RC_UTILS_H_
#define _SERVO_RC_UTILS_H_

// Offset for Roboclaw since its 32bit position is unsigned and don't want deal with rollover
//#define kRC_ENCODER_OFFSET 0x80000000
#define	kRC_ENCODER_OFFSET	0x0
// Return states
#define	kRC_ENCODER_ERROR	0xFFFFFFFE
#define	kRC_STATUS_ERROR	0xFFFFFFFF
#define	kRC_CMD_QUEUE_EMPTY	0x80
#define	kRC_OK				0xFF		//*	RC's successful one byte return status

// RC_get_status() return masks below:
// Normal 0x00000000
// E-Stop 0x0000001
// Temperature Error 0x0000002
// Temperature 2 Error 0x0000004
// Main Voltage High Error 0x0000008
// Logic Voltage High Error 0x0000010
// Logic Voltage Low Error 0x0000020
// M1 Driver Fault Error 0x0000040
// M2 Driver Fault Error 0x0000080
// M1 Speed Error 0x0000100
// M2 Speed Error 0x0000200
// M1 Position Error 0x0000400
// M2 Position Error 0x0000800
// M1 Current Error 0x0001000
// M2 Current Error 0x0002000
// M1 Over Current Warning 0x0010000
// M2 Over Current Warning 0x0020000
// ain Voltage High Warning 0x0040000
// Main Voltage Low Warning 0x0080000
// Temperature Warning 0x00100000
// Temperature 2 Warning 0x00200000
// S4 Signal Triggered 0x-00400000
// S5 Signal Triggered 0x00800000
// Speed Error Limit Warning 0x01000000
// Position Error Limit Warning 0x02000000

int RC_converse(uint8_t *cmdBuf, size_t cmdLen, uint8_t *retBuf, size_t retLen);
int RC_get_curr_pos(uint8_t addr, uint8_t motor, int32_t *pos);
int RC_get_curr_velocity(uint8_t addr, uint8_t motor, int32_t *vel);
int RC_set_home(uint8_t addr, uint8_t motor);
int RC_get_status(uint8_t addr, uint32_t *rcStatus);
int RC_check_queue(uint8_t addr, uint8_t *raDepth, uint8_t *decDepth);
int RC_set_default_acc(uint8_t addr, uint8_t motor, uint32_t acc);
int RC_write_settings(uint8_t addr);
int RC_read_settings(uint8_t addr, uint32_t *rcStatus);
int RC_get_pos_pid(uint8_t addr, uint8_t motor, double *propo, double *integ, double *deriv, uint32_t *iMax, uint32_t *deadZ, int32_t *minP, int32_t *maxP);
int RC_set_pos_pid(uint8_t addr, uint8_t motor, double propo, double integ, double deriv, uint32_t iMax, uint32_t deadZ, int32_t minP, int32_t maxP);
int RC_get_vel_pid(uint8_t addr, uint8_t motor, double *propo, double *integ, double *deriv, uint32_t *qpps);
int RC_set_vel_pid(uint8_t addr, uint8_t motor, double propo, double integ, double deriv, uint32_t qpps);
int RC_restore_defaults(uint8_t addr);
int RC_stop(uint8_t addr, uint8_t motor);
double RC_calc_move_time(int32_t pos0, int32_t pos1, uint32_t vel, uint32_t acc);
int RC_move_by_posv(uint8_t addr, uint8_t motor, int32_t pos, uint32_t vel, bool buffered);
int RC_move_by_posva(uint8_t addr, uint8_t motor, int32_t pos, uint32_t vel, uint32_t acc, bool buffered);
int RC_move_by_vela(uint8_t addr, uint8_t motor, int32_t vel, uint32_t acc, bool buffered);
int RC_move_by_vel_raw(uint8_t addr, uint8_t motor, int32_t vel);

#endif // _SERVO_RC_UTILS_H_
