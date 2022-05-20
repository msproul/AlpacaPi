//******************************************************************************
//*	Name:			servo_rc_utils.c
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
//*	Apr 11,	2022	<RNS> Created RC_utils.c from LMx_utils.c
//*	Apr 20,	2022	<RNS> Cleaned up globals to reduce variable's scope
//*	Apr 25,	2022	<RNS> Ported stop, reset and max_acc routines
//*	Apr 26,	2022	<RNS> Created move by pos and acc routines
//*	Apr 27,	2022	<RNS> Add check_queue function find end-of move_by_pos
//*	Apr 28,	2022	<RNS> added vel_raw and vel_buffered (now default)
//*	Apr 28,	2022	<RNS> added calc_move_time
//*	May  6,	2022	<RNS> fixed includes with servo_*c_* file name changes
//*	May  7,	2022	<RNS> Fixed ordering bug with rc_cmds.h
//*	May  7,	2022	<RNS> Fixed a bug with RC CRC16 calcs missing addr & cmd
//*	May  8,	2022	<RNS> renamed RC* functions from _read_ to _get_
//*	May  8,	2022	<RNS> edit move_by_pos to _posva since default_acc is broken
//*	May 19,	2022	<RNS> Cleaned up unit _TEST_ for warnings and printfs
//*****************************************************************************
// Notes:   M1 *MUST BE* connected to RA or Azimuth axis, M2 to Dec or Altitude
//*****************************************************************************
#include	<stdio.h>
#include	<stddef.h>
#include	<string.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"servo_std_defs.h"
#include	"servo_mc_core.h"
#include	"servo_rc_utils.h"
#include	"servo_rc_cmds.h"

// This is value used to make a buffered move_by_vel command
#define POS_FOREVER 0x3000000

// Global buffers for Notes and Receipts
static uint8_t gNoteBuf[64];
static uint8_t gReceiptBuf[64];

// Roboclaw most popular values
static uint8_t gAddr	=	0x80; // default address for RC

//*****************************************************************************
// Handles the messaging between the host the MC. Expect it to be modified to different
//   MC if they require more than a simple send message and return value pair
//*****************************************************************************
int RC_converse(uint8_t *cmdBuf, size_t cmdLen, uint8_t *retBuf, size_t retLen)
{
int		len;
int		status;
int		writeStatus;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_LONG("cmdLen\t=", cmdLen);
	CONSOLE_DEBUG_W_LONG("retLen\t=", retLen);
	writeStatus	=	MC_write_comm(cmdBuf, cmdLen);
	CONSOLE_DEBUG_W_NUM("MC_write_comm -> writeStatus\t=", writeStatus);
	if (writeStatus == kSTATUS_OK)
	{
		len	=	MC_read_comm(retBuf, retLen);
		CONSOLE_DEBUG_W_NUM("MC_read_comm -> len\t=", len);
		CONSOLE_DEBUG_W_LONG("retLen \t\t=", retLen);

		if (len == retLen)
		{
			status	=	kSTATUS_OK;
		}
		else
		{
			printf("RC_converse: got error - len = %d   retLen = %d\n", len, (int) retLen);
			status	=	kERROR;
		}
	}
	else
	{
		CONSOLE_DEBUG("MC_write_comm() failed");
		status	=	kERROR;
	}
	// If we got here, everything worked
	return(status);
} // of RC_converse()

//*****************************************************************************
// Basic MC read position from a single axis and corrects for the encoder offset
// Read Encoder Count M1 protocol format:
// Send: [Address, 16] = 2
// Receive: [Enc1(4 bytes), Status, CRC(2 bytes)] = 7
//*****************************************************************************
int RC_get_curr_pos(u_int8_t motor, int32_t *pos)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc, receiptCrc;
uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
int			retState;

	CONSOLE_DEBUG(__FUNCTION__);
	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	GETM1ENCVALUE;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	GETM2ENCVALUE;
			break;

		default:
			// Neither RA or DEC selected, return error
			*pos	=	kRC_ENCODER_ERROR;
			return(kERROR);
			break;
	}
	printf("RC_get_curr_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// Creat the note for the comms and converse
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the encoder count 32bit from the receipt buf and status byte
	count	=	Receipt_get_dword(gReceiptBuf, &ptrA);
	status	=	Receipt_get_byte(ptrA, &ptrB);
	if (status != kSTATUS_OK)
	{
		CONSOLE_DEBUG("Receipt_get_byte() returned error");
	}

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&gAddr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);
	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrB, &ptrA);

	printf("RC_get_curr_pos() crc = %d  receiptCrc = %d\n", crc, receiptCrc);

	// Check CRC and return the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// Return Error
		*pos	=	kRC_ENCODER_ERROR;
		return(kERROR);
	}
	// Everything OK, return new value
	*pos	=	count - kRC_ENCODER_OFFSET;
	return(kSTATUS_OK);
} // of RC_get_curr_pos()

//*****************************************************************************
// Basic MC read velocity from a single axis in encoder counts / sec
// Read Encoder Speed M1 protocol format:
// Send: [Address, 18] = 2
// Receive: [Speed(4 bytes), Status, CRC(2 bytes)] = 7
//*****************************************************************************
int RC_get_curr_velocity(u_int8_t motor, int32_t *vel)
{
uint8_t		*ptrA, *ptrB;
uint8_t		status;
uint16_t	crc;
uint16_t	receiptCrc;
uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
int			retState;

	CONSOLE_DEBUG(__FUNCTION__);
	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	GETM1ENCSPEED;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	GETM2ENCSPEED;
			break;

		default:
			// Neither RA or DEC selected, return error
			*vel	=	kRC_ENCODER_ERROR;
			return(kERROR);
			break;
	}

	// Creat the note for the comms and converse
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the encoder count 32bit from the receipt buf and status byte
	count	=	Receipt_get_dword(gReceiptBuf, &ptrA);
	status	=	Receipt_get_byte(ptrA, &ptrB);
	if (status != kSTATUS_OK)
	{
		CONSOLE_DEBUG("Receipt_get_byte() returned error");
	}

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&gAddr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);
	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrB, &ptrA);

	// Check CRC and retun the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// Return Eerror
		*vel	=	kRC_ENCODER_ERROR;
		return(kERROR);
	}
	// Everything OK, return new value
	*vel	=	count - kRC_ENCODER_OFFSET;
	return(kSTATUS_OK);
} // of RC_get_curr_velocity()

//*****************************************************************************
// Basic MC write position to a single axis
// Write Encoder Count M1 protocol format:
// Send: [Address, 22, Value(4 bytes), CRC(2 bytes)] = 8
// Receive: [0xFF] = 1
//*****************************************************************************
int RC_set_home(uint8_t motor)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
//uint16_t	receiptCrc;
int			cmd;
int			len;
int			retState;

	CONSOLE_DEBUG(__FUNCTION__);
	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	SETM1ENCCOUNT;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	SETM2ENCCOUNT;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	// Create the note for the comms and set endcode to the offset
	printf("RC_set_home: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, kRC_ENCODER_OFFSET, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	printf("RC_set_home: cmd = %d gRC[cmd].in = %d gRC[cmd].out = %d \n", cmd, gRC[cmd].in, gRC[cmd].out);

	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Check the return status for the happy 0xFF
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	printf("RC_set_home() : return status = %x\n", (int)status);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // RC_set_home()

//******************************************************************
// Read the current unit status, both motors
// Send: [Address, 90] = 2
// Receive: [Status (32bits?), CRC(2 bytes)] -> needs to be 6
// Return masks are documented and decoded in RC_utils.h
//******************************************************************
int RC_get_status(uint32_t *rcStatus)
{
//uint8_t		readMsg[kSMALL_STR_LEN];	// data str to be sent to mc
//uint8_t		writeMsg[kSMALL_STR_LEN];	// data str to be read from mc
//uint8_t		rtnData[kSMALL_STR_LEN];	// str to hold the rtn data from rtn msg
//int			sendLen;					// length of message to send to mc
//int			dataLen;					// length of the return data from read mc
uint8_t		*ptrA, *ptrB;
uint16_t	crc;
uint16_t	receiptCrc;
uint32_t	status;
//uint32_t	ret;
int			cmd	=	GETSTATUS;
int			len;
int			retState;

	CONSOLE_DEBUG(__FUNCTION__);
	printf("RC_get_status: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// Creat the note for the comms and read status a short cmd
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the status (32bit?)
	status	=	Receipt_get_dword(gReceiptBuf, &ptrA);

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrA - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&gAddr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);

	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrA, &ptrB);

	// Check SRC and return the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// Return Eerror
		*rcStatus	=	kRC_STATUS_ERROR;
		return(kERROR);
	}
	// Everything OK, return status
	*rcStatus	=	status;
	return(kSTATUS_OK);
} // of RC_get_status()

//******************************************************************
// Read the current buffer depth for each motor, show where there are commands
// in-process or pending. Returns the number of commands outstanding, where a 0
// means the last command is in process and 0x80 mean all cmds completed
// and max depth is 64 (zero-based) so 0x3F or less, Error returns 0xFF in args
// Send: [Address, 47]
// Receive: [BufferM1, BufferM2, CRC(2 bytes)]
//******************************************************************
int RC_check_queue(uint8_t *raDepth, uint8_t *decDepth)
{
//uint8_t		readMsg[kSMALL_STR_LEN];	// data str to be sent to mc
//uint8_t		writeMsg[kSMALL_STR_LEN];	// data str to be read from mc
//uint8_t		rtnData[kSMALL_STR_LEN];	// str to hold the rtn data from rtn msg
//int			sendLen;					// length of message to send to mc
//int			dataLen;					// length of the return data from read mc
uint8_t		*ptrA, *ptrB;
uint16_t	crc;
uint16_t	receiptCrc;
//uint32_t	status;
//uint32_t	ret;
int			cmd	=	GETBUFFERS;
int			len;
int			retState;

	CONSOLE_DEBUG(__FUNCTION__);
	// Creat the note for the comms and read status a short cmd
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the status (32bit?)
	*raDepth	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	*decDepth	=	Receipt_get_byte(ptrA, &ptrB);

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&gAddr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);

	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrA, &ptrB);

	// Check SRC and return the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// set RA and Dec buffers to 0xFF, which is out-of-range
		*raDepth	=	0xFF;
		*decDepth	=	0xFF;
		// Return Eerror
		return(kERROR);
	}
	// Everything OK, return status
	return(kSTATUS_OK);
} // of RC_check_buffer()

//*************************************************************************
// Set the max acceleration from the config file to the RC default speed
// Send: [Address, 68, Accel(4 bytes), CRC(2 bytes)]
// Receive: [0xFF]
//*************************************************************************
int RC_set_default_acc(uint8_t motor, uint32_t acc)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
//int			i;

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
	case SERVO_RA_AXIS:
		cmd	=	SETM1DEFAULTACCEL;
		break;

	case SERVO_DEC_AXIS:
		cmd	=	SETM2DEFAULTACCEL;
		break;

	default:
		// Neither RA or DEC selected, return error
		return(kERROR);
		break;
	}
	printf("RC_set_default_acc: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// Create the note for the comms and set endcode to the offset
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, acc, &ptrB);
	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	printf("RC_set_default_acc: crcLength = %d  crc = %d\n", len, crc);
	Note_add_word(ptrB, crc, &ptrA);

	printf("RC_set_default_acc: cmd = %d gRC[cmd].in = %d gRC[cmd].out = %d \n", cmd, gRC[cmd].in, gRC[cmd].out);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_set_default_acc()

//*************************************************************************
// Stop the motor by setting the duty cycle to zero
// Send: [Address, 32, Duty(2 Bytes), CRC(2 bytes)]
// Receive: [0xFF]
//*************************************************************************
int RC_stop(uint8_t motor)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
int			cmd;
int			len;
int			zero	=	0;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;

	CONSOLE_DEBUG(__FUNCTION__);

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
	case SERVO_RA_AXIS:
		cmd	=	M1DUTY;
		break;

	case SERVO_DEC_AXIS:
		cmd	=	M2DUTY;
		break;

	default:
		// Neither RA or DEC selected, return error
		return(kERROR);
		break;
	}

	printf("RC_stop: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// Create the note for the comms and set endcode to the offset
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_word(ptrA, zero, &ptrB);
	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_stop()

//******************************************************************
// calcs the time it will take to move from pos1 from pos0 with given vel & acc
// returns the value in decimal seconds
//******************************************************************
double RC_calc_move_time(uint32_t pos0, uint32_t pos1, uint32_t vel, uint32_t acc)
{
uint32_t dist	=	abs(pos1 - pos0);
double distF, velF, accF, vCalc, time;

	// Cast the uints to doubles
	distF	=	(double)dist;
	velF	=	(double)vel;
	accF	=	(double)acc;

	// our accel = decel, does the profile have time to reach max vel, from physics
	// v = sqrt(2 * acc * displacement ) so use half the dist for displacement (2's cancel)
	vCalc	=	sqrt(accF * distF);

	// check to see if the move is long enough to get to max vel
	if (vCalc > velF)
	{
		// move reaches max velocity so standard trapezoidal profile
		// time equal dist / max vel + extra time for accel and decel ramps
		time	=	distF / velF + (velF / accF);
	}
	else
	{
		// move is too short to reach max vel, so it's a triangle profile
		time	=	2 * vCalc / accF;
	}

	return(time);
} // RC_calc_move_time()

//******************************************************************
// Move the axis to input position with the input velocity and will buffer the
// command if 'true'.   If 'false' it overwrites any executing cmd
// Moves the axis by pos steps with velocity parameter
// Send; [Address, 122, Speed (4 bytes), Position (4 bytes), Buffer, CRC (2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_move_by_posv(uint8_t motor, uint32_t pos, uint32_t vel, bool buffered)
{
uint8_t		*ptrA, *ptrB, status, now;
uint16_t	crc;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
//int			i;

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	M1SPEEDPOS;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	M2SPEEDPOS;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}
	printf("RC_move_by_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// If requesting a buffered command (eg. TRUE) set variable now to 0
	// otherwise, setting it to 1 means stop any running cmds and execute it now
	now	=	(buffered == true) ? 0 : 1;

	// Create the note for the comms and add vel, pos
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, vel, &ptrB);
	Note_add_dword(ptrB, pos, &ptrA);
	// add buffer arg, 1 = stop any running cmds and execute it now
	Note_add_byte(ptrA, now, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_move_by_posv()

//******************************************************************
// Move the axis to input position with the input velocity and accel and will
// buffer the command if 'true'.   If 'false' it overwrites any executing cmd
// Moves the axis by pos steps with velocity and accel parameters
// Send: [Address, 65, Accel(4 bytes), Speed(4 Bytes), Decel(4 bytes), Position(4 Bytes), Buffer, CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_move_by_posva(uint8_t motor, uint32_t pos, uint32_t vel, uint32_t acc, bool buffered)
{
uint8_t		*ptrA, *ptrB, status, now;
uint16_t	crc;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
//int			i;

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	M1SPEEDACCELDECELPOS;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	M2SPEEDACCELDECELPOS;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}
	printf("RC_move_by_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", gAddr, cmd, gRC[cmd].cmd);

	// If requesting a buffered command (eg. TRUE) set variable now to 0
	// otherwise, setting it to 1 means stop any running cmds and execute it now
	now	=	(buffered == true) ? 0 : 1;

	// Create the note for the comms and add vel, pos
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, acc, &ptrB);
	Note_add_dword(ptrB, vel, &ptrA);
	Note_add_dword(ptrA, acc, &ptrB);
	Note_add_dword(ptrB, pos, &ptrA);

	// add buffer arg, 1 = stop any running cmds and execute it now
	Note_add_byte(ptrA, now, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_move_by_posva()

//******************************************************************
// Calls RC_move_by_pos with supplied velocity and acc then creates an artificial position
// far far away. This allows the RC to effectively have a buffered position
// command that mimics the a velocity move that will last many many hours
//******************************************************************
int RC_move_by_vela(uint8_t motor, uint32_t vel, uint32_t acc)
{
int32_t	pos;
int		status;

	// if vel is negative, the subtract the faraway position from current
	status	=	kSTATUS_OK;
	switch (motor)
	{
		case SERVO_RA_AXIS:
			RC_get_curr_pos(SERVO_RA_AXIS, &pos);
			if (vel < 0)
			{
				pos	-=	POS_FOREVER;
			}
			else
			{
				pos	+=	POS_FOREVER;
			}
			RC_move_by_posva(SERVO_RA_AXIS, pos, vel, acc, true);
			break;

		case SERVO_DEC_AXIS:
			RC_get_curr_pos(SERVO_DEC_AXIS, &pos);
			if (vel < 0)
			{
				pos	-=	POS_FOREVER;
			}
			else
			{
				pos	+=	POS_FOREVER;
			}
			RC_move_by_posva(SERVO_DEC_AXIS, pos, vel, acc, true);
			break;

		default:
			// Neither RA or DEC selected, return error
			status	=	kERROR;
			break;
	}

	return(status);
} // of RC_move_by_vela()

//******************************************************************
// Moves the axis by signed velocity
// Send: [Address, 35, Speed(4 Bytes), CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_move_by_vel_raw(uint8_t motor, uint32_t vel)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
int			cmd;
int			len;
//uint8_t		now	=	1;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;
//int			i;

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	M1SPEED;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	M2SPEED;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	// Create the note for the comms and add vel parameter
	Note_init(gNoteBuf, gAddr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, vel, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_move_by_vel_raw()

//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
#ifdef _TEST_RC_UTILS_
int	main(int argc, char **argv)
{
char buf[256];
int32_t pos	=	0;
uint32_t status = 0; 

	if (MC_init_comm("/dev/ttyACM0", 38400) != 0)
	{
		printf("Error: mc_init_comm() failed\n");
		return kERROR;
	}
	if (RC_set_home(SERVO_RA_AXIS) == kERROR)
	{
		printf("RC_set_home returned error\n");
	}

	if (RC_get_curr_pos(SERVO_RA_AXIS, &pos) == kERROR)
	{
		printf("RC_current_pos returned error\n");
	}
	printf("Pos = %X\n", pos);

	if (RC_move_by_posva(SERVO_RA_AXIS, 10000, 5000, 500, false) == kERROR)
	//   if (RC_move_by_vel_raw(SERVO_RA_AXIS, 1000) == kERROR)
	{
		printf("RC_move_by_pos returned error\n");
	}

	if (RC_get_status(&status) == kERROR)
	{
		printf("RC_get_status returned error\n");
	}
	printf("status = %X\n", status);

	printf("hit any key to reverse motor \n");
	fgets(buf, 256, stdin);

	if (RC_move_by_posva(SERVO_RA_AXIS, 0, 5000, 500, false) == kERROR)
	//   if (RC_move_by_vel_raw(SERVO_RA_AXIS, 1000) == kERROR)
	{
		printf("RC_move_by_pos returned error\n");
	}

	if (RC_get_status(&status) == kERROR)
	{
		printf("RC_get_status returned error\n");
	}
	printf("status = %X\n", status);
	if (RC_get_curr_pos(SERVO_RA_AXIS, &pos) == kERROR)
	{
		printf("RC_current_pos returned error\n");
	}
	printf("Pos = %X\n", pos);

	if (RC_get_status(&status) == kERROR)
	{
		printf("RC_get_status returned error\n");
	}
	printf("status = %X\n", pos);

	printf("hit any key to stop motor\n");
	fgets(buf, 256, stdin);

	RC_stop(SERVO_RA_AXIS);

	MC_shutdown();

	return(kSTATUS_OK);
}
#endif
