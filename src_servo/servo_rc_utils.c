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
//*	May 21,	2022	<RNS> added addr arg to RC_ MC cmds for multi-RC support
//*	May 22,	2022	<RNS> Fixed some unsigned to signed compares for abs and fabs()
//*	May 22,	2022	<RNS> Fixed some arg type inputs
//*	May 22,	2022	<RNS> Changed main() back to be void, _TEST_ should never take args
//*	May 28,	2022	<RNS> Fixed CRC bug in _check_queue due to ptr swap
//*	May 28,	2022	<RNS> Enhanced unit _TEST_ to support both motors
//*	May 29,	2022	<RNS> Enhanced unit _TEST_ to have better test coverage for debug
//*	May 29,	2022	<RNS> Created Workaround to RC buffer lazy bit7 issue
//*	May 30,	2022	<RNS> Workaround to RC _vela where neg pos & vel became positive
//*	Jun 06,	2022	<RNS> Fixed a bug in get_velocity where status sign was ignored
//*	Jun 06,	2022	<RNS> Added _set_pos_pid and _get_pos_pid functions
//*	Jun 06,	2022	<RNS> Added _read_settings & _write_settings commands
//*	Jun 07,	2022	<RNS> New workaround to read/write settingz
//*	Jun 08,	2022	<RNS> Workaround for set_default_acc, needed extra acc field
//*	Jun 09,	2022	<RNS> Fixed bug in _set_pos_pid, PID order actually DPI in docs
//*	Jun 12,	2022	<RNS> Masked 'set but not used' warning for a returned RC state
//*	Jun 12,	2022	<RNS> Added _set_vel_pid and _get_vel_pid functions
//*	Jun 13	2022	<RNS> Converted all PID function to use float for PID args
//*	Jun 19	2022	<RNS> Fixed an 'unused' compiler warning
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

//*****************************************************************************
// Handles the messaging between the host the MC. Expect it to be modified to different
//   MC if they require more than a simple send message and return value pair
//*****************************************************************************
int RC_converse(uint8_t *cmdBuf, size_t cmdLen, uint8_t *retBuf, size_t retLen)
{
size_t	len;
int		status;
int		writeStatus;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_LONG("cmdLen\t=", cmdLen);
//	CONSOLE_DEBUG_W_LONG("retLen\t=", retLen);
	writeStatus	=	MC_write_comm(cmdBuf, cmdLen);
//	CONSOLE_DEBUG_W_NUM("MC_write_comm -> writeStatus\t=", writeStatus);
	if (writeStatus == kSTATUS_OK)
	{
		len	=	MC_read_comm(retBuf, retLen);
//		CONSOLE_DEBUG_W_NUM("MC_read_comm -> len\t=", (int)len);
//		CONSOLE_DEBUG_W_LONG("retLen \t\t=", retLen);

		if (len == retLen)
		{
			status	=	kSTATUS_OK;
		}
		else
		{
			printf("RC_converse: got error - len = %ld   retLen = %ld\n", len, retLen);
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
int RC_get_curr_pos(uint8_t addr, uint8_t motor, int32_t *pos)
{
uint8_t		*ptrA, *ptrB;
uint16_t	crc, receiptCrc;
uint32_t	count;
//uint32_t	ret;
int			cmd;
int			len;
int			retState;
[[maybe_unused]] uint8_t status;

//	CONSOLE_DEBUG(__FUNCTION__);
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
	//printf("RC_get_curr_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Creat the note for the comms and converse
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the encoder count 32bit from the receipt buf and status byte
	count	 	=	Receipt_get_dword(gReceiptBuf, &ptrA);
	status		=	Receipt_get_byte(ptrA, &ptrB);

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&addr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);
	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrB, &ptrA);

	//printf("RC_get_curr_pos() crc = %d  receiptCrc = %d\n", crc, receiptCrc);

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
// Status indicates the direction (0 – forward, 1 - backward)
//*****************************************************************************
int RC_get_curr_velocity(uint8_t addr, uint8_t motor, int32_t *vel)
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

//	CONSOLE_DEBUG(__FUNCTION__);
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
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Get the encoder count 32bit from the receipt buf and status byte
	count	=	Receipt_get_dword(gReceiptBuf, &ptrA);
	status	=	Receipt_get_byte(ptrA, &ptrB);

	// Calc length from the receipt buf pointer distance and calc CRC
	len			=	(int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc			=	MC_calc_crc16(&addr, 1, kCLEAR_CRC);
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
	*vel	=	(status == 0) ? count : - count; 
	return(kSTATUS_OK);
} // of RC_get_curr_velocity()

//*****************************************************************************
// Basic MC write position to a single axis
// Write Encoder Count M1 protocol format:
// Send: [Address, 22, Value(4 bytes), CRC(2 bytes)] = 8
// Receive: [0xFF] = 1
//*****************************************************************************
int RC_set_home(uint8_t addr, uint8_t motor)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
//uint16_t	receiptCrc;
int			cmd;
int			len;
int			retState;

//	CONSOLE_DEBUG(__FUNCTION__);
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

	//printf("RC_set_home: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);
	// Create the note for the comms and set endcode to the offset
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	Note_add_dword(ptrA, kRC_ENCODER_OFFSET, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	//printf("RC_set_home: cmd = %d gRC[cmd].in = %d gRC[cmd].out = %d \n", cmd, gRC[cmd].in, gRC[cmd].out);

	retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Check the return status for the happy 0xFF
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);

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
int RC_get_status(uint8_t addr, uint32_t *rcStatus)
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

//	CONSOLE_DEBUG(__FUNCTION__);
	//printf("RC_get_status: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Creat the note for the comms and read status a short cmd
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);

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
	crc			=	MC_calc_crc16(&addr, 1, kCLEAR_CRC);
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
int RC_check_queue(uint8_t addr, uint8_t *raDepth, uint8_t *decDepth)
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

//	CONSOLE_DEBUG(__FUNCTION__);
	// Create the note for the comms and read status a short cmd
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);

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
	crc			=	MC_calc_crc16(&addr, 1, kCLEAR_CRC);
	crc			=	MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc			=	MC_calc_crc16(gReceiptBuf, len, crc);

	// Get CRC from the receipt message
	receiptCrc	=	Receipt_get_word(ptrB, &ptrA);

	//printf("RC_check_queue() crc = %d  receiptCrc = %d\n", crc, receiptCrc);

	// Check SRC and return the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// set RA and Dec buffers to 0xFF, which is out-of-range
		*raDepth	=	0xFF;
		*decDepth	=	0xFF;
		// Return Eerror
		return(kERROR);
	}
	else
	{
		// WORKAROUND! to RC having a lazy bit 7 bit set issue
		// Cmd 47 is never supposed to return a number > 0x80
		// So if it happends, subtract 0x80 from it
		*raDepth = (*raDepth > 0x80) ? *raDepth - 0x80 : *raDepth;
		*decDepth = (*decDepth > 0x80) ? *decDepth - 0x80 : *decDepth;
	}


	// Everything OK, return status
	return(kSTATUS_OK);
} // of RC_check_queue()

//*************************************************************************
// Set the max acceleration from the config file to the RC default speed
// Send: [Address, 68, Accel(4 bytes), CRC(2 bytes)]
// Receive: [0xFF]
// WARNING:  I don't think this routine works, it returns no errors
//*************************************************************************
int RC_set_default_acc(uint8_t addr, uint8_t motor, uint32_t acc)
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
	//printf("RC_set_default_acc: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Create the note for the comms and set endcode to the offset
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	// add acceleration value
	Note_add_dword(ptrA, acc, &ptrB);
	// WORKAROUND: apparently you need a decel value as well
	Note_add_dword(ptrB, acc, &ptrA);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrA - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	//printf("RC_set_default_acc: crcLength = %d  crc = %d\n", len, crc);
	Note_add_word(ptrA, crc, &ptrB);

	//printf("RC_set_default_acc: cmd = %d gRC[cmd].in = %d gRC[cmd].out = %d \n", cmd, gRC[cmd].in, gRC[cmd].out);
	// Added 4 bytes to the input lenght to allow for the extra acc parameter
	RC_converse(gNoteBuf, gRC[cmd].in + 4, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_set_default_acc()


//******************************************************************
// Write the current settings to the Roboclaw EEPROM
// Send: [Address, 94] -> is WRONG
// Receive: [0xFF]
// WORKAROUND! Add 4byte value and CRC - thus +6 in RC_converse
//******************************************************************
int RC_write_settings(uint8_t addr)
{
uint8_t		*ptrA, *ptrB; 
uint32_t	status;
uint16_t	crc;
int			cmd	=	WRITENVM;
int			len; 
int			retState;

//	CONSOLE_DEBUG(__FUNCTION__);
	//printf("RC_get_status: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Creat the note for the comms and read status a short cmd
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	// WORKAROUND: Add dummy 32bit value 
	//Note_add_dword(ptrA, 0xE22EAB7A, &ptrB);
	Note_add_dword(ptrA, 0x0, &ptrB);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrB - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrB, crc, &ptrA);

	retState	=	RC_converse(gNoteBuf, gRC[cmd].in + 6, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_write_settings()

//******************************************************************
// Read the current settings to the Roboclaw EEPROM
// Send: [Address, 95]
// Receive: [Enc1Mode, Enc2Mode, CRC(2 bytes)]
//******************************************************************
int RC_read_settings(uint8_t addr, [[maybe_unused]] uint32_t *rcStatus)
{
//uint8_t		readMsg[kSMALL_STR_LEN];	// data str to be sent to mc
//uint8_t		writeMsg[kSMALL_STR_LEN];	// data str to be read from mc
//uint8_t		rtnData[kSMALL_STR_LEN];	// str to hold the rtn data from rtn msg
//int			sendLen;					// length of message to send to mc
//int			dataLen;					// length of the return data from read mc
uint8_t		*ptrA, *ptrB;
uint16_t	crc;
uint32_t	status;
//uint32_t	ret;
int			cmd	=	READNVM;
int			len;
int			retState;

//	CONSOLE_DEBUG(__FUNCTION__);
	//printf("RC_get_status: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);


	// Creat the note for the comms and read status a short cmd
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrA - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrA, crc, &ptrB);

	//retState	=	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	retState	=	RC_converse(gNoteBuf, gRC[cmd].in +2, gReceiptBuf, 1);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return(kERROR);
	}


	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_read_settings()

//******************************************************************
// Returns the *position* PID values and motion fields for an axis
// Returns kSTATUS_OK or kERROR, PID values need to be divided by 1024
// Send: [Address, 63]
// Receive: [P(4 bytes), I(4 bytes), D(4 bytes), MaxI(4 bytes)s, Deadzone(4 bytes),
//			MinPos(4 byte), MaxPos(4 byte), CRC(2 bytes)]
//******************************************************************
int RC_get_pos_pid(uint8_t addr, uint8_t motor, double *propo, double *integ, double *deriv, uint32_t *iMax, uint32_t *deadZ, int32_t *minP, int32_t *maxP)
{
uint8_t		*ptrA, *ptrB;
uint16_t	crc;
uint16_t	receiptCrc;
//uint32_t	ret;
int			cmd;
int			len;
int			retState;

//	CONSOLE_DEBUG(__FUNCTION__);
	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	GETM1POSPID;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	GETM2POSPID;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	// Creat the note for the comms and converse
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	retState = RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return (kERROR);
	}

	// Get the PID value and the rest of the parameters and scale interl PID back to float
	*propo	= (double) Receipt_get_dword(gReceiptBuf, &ptrA) / 1024.0;
	*integ	= (double) Receipt_get_dword(ptrA, &ptrB) / 1024.0;
	*deriv	= (double) Receipt_get_dword(ptrB, &ptrA) / 1024.0;
	*iMax	= Receipt_get_dword(ptrA, &ptrB);
	*deadZ	= Receipt_get_dword(ptrB, &ptrA);
	*minP	= Receipt_get_dword(ptrA, &ptrB);
	*maxP	= Receipt_get_dword(ptrB, &ptrA);

	// Calc length from the receipt buf pointer distance and calc CRC
	len = (int)(ptrA - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc = MC_calc_crc16(&addr, 1, kCLEAR_CRC);
	crc = MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc = MC_calc_crc16(gReceiptBuf, len, crc);
	// Get CRC from the receipt message
	receiptCrc = Receipt_get_word(ptrA, &ptrB);

	// Check CRC and retun the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// Return Error and all zeros
		*propo	= 	0.0; 
		*integ	=	0.0;
		*deriv	=	0.0;
		*iMax	=	0;
		*deadZ	=	0;
		*minP	=	0;
		*maxP	=	0;
				return (kERROR);
	}
	// Everything OK, return new value
	return (kSTATUS_OK);
} // of RC_get_pos_pid()

//******************************************************************
// Sets all the *position* PID values and motion fields for an axis
// Returns kSTATUS_OK or kERROR, PID values need to be scaled by 1024x
// Send: [Address, 61, D(4 bytes), P(4 bytes), I(4 bytes), MaxI(4 bytes),
// Deadzone(4 bytes), MinPos(4 bytes), MaxPos(4 bytes), CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_set_pos_pid(uint8_t addr, uint8_t motor, double propo, double integ, double deriv, uint32_t iMax, uint32_t deadZ, int32_t minP, int32_t maxP)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
int			cmd;
int			len;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;

//	CONSOLE_DEBUG(__FUNCTION__);

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	SETM1POSPID;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	SETM2POSPID;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	//printf("RC_stop: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Convert from float to internal RC integer format by multiply by 1024
	propo	*= 1024.0;
	deriv	*= 1024.0;
	integ	*= 1024.0; 

	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	// Note: the set command order is not PID---- but DPI---- 
	Note_add_dword(ptrA, (uint32_t) deriv, &ptrB);
	Note_add_dword(ptrB, (uint32_t) propo, &ptrA);
	Note_add_dword(ptrA, (uint32_t) integ, &ptrB);
	Note_add_dword(ptrB, iMax, &ptrA);
	Note_add_dword(ptrA, deadZ, &ptrB);
	Note_add_dword(ptrB, minP, &ptrA);
	Note_add_dword(ptrA, maxP, &ptrB);

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
} // of RC_set_pos_pid()

//******************************************************************
// Returns the *velocity* PID values and QPPS for an axis
// Returns kSTATUS_OK or kERROR, PID values need to be divided by 65536
// Send: [Address, 55]
// Receive: [P(4 bytes), I(4 bytes), D(4 bytes), QPPS(4 byte), CRC(2 bytes)]
//******************************************************************
int RC_get_vel_pid(uint8_t addr, uint8_t motor, double *propo, double *integ, double *deriv, uint32_t *qpps)
{
uint8_t		*ptrA, *ptrB;
uint16_t	crc;
uint16_t	receiptCrc;
//uint32_t	ret;
int			cmd;
int			len;
int			retState;

//	CONSOLE_DEBUG(__FUNCTION__);
	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	GETM1VELPID;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	GETM2VELPID;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	// Creat the note for the comms and converse
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	retState = RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	if (retState != kSTATUS_OK)
	{
		CONSOLE_DEBUG("MC_converse() returned error");
		return (kERROR);
	}

	// Get the PID value and the rest of the parameters
	*propo	= (double) Receipt_get_dword(gReceiptBuf, &ptrA) / 65536.0;
	*integ	= (double) Receipt_get_dword(ptrA, &ptrB) / 65536.0;
	*deriv	= (double) Receipt_get_dword(ptrB, &ptrA) / 65536.0;
	*qpps	= Receipt_get_dword(ptrA, &ptrB);

	// Calc length from the receipt buf pointer distance and calc CRC
	len = (int)(ptrB - gReceiptBuf);
	// Calc CRC including addr, cmd and entire receipt buffer, not the current ptr
	crc = MC_calc_crc16(&addr, 1, kCLEAR_CRC);
	crc = MC_calc_crc16(&gRC[cmd].cmd, 1, crc);
	crc = MC_calc_crc16(gReceiptBuf, len, crc);
	// Get CRC from the receipt message
	receiptCrc = Receipt_get_word(ptrB, &ptrA);

	// Check CRC and retun the encoder count with corrected offset
	if (receiptCrc != crc)
	{
		// Return Error and all zeros
		*propo	= 	0; 
		*integ	=	0;
		*deriv	=	0;
		*qpps	=	0;
		return (kERROR);
	}
	// Everything OK, return new value
	return (kSTATUS_OK);
} // of RC_get_vel_pid()

//******************************************************************
// Sets the *velocity* PID values and QPPS values for an axis  
// Returns kSTATUS_OK or kERROR, PID values need to be scaled by 65536x
// Send: [Address, 28, D(4 bytes), P(4 bytes), I(4 bytes), QPPS(4 byte), CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_set_vel_pid(uint8_t addr, uint8_t motor, double propo, double integ, double deriv, uint32_t qpps)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
int			cmd;
int			len;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;

//	CONSOLE_DEBUG(__FUNCTION__);

	// Select motor:  RA (0) means M1 and DEC (1) means M2
	switch (motor)
	{
		case SERVO_RA_AXIS:
			cmd	=	SETM1VELPID;
			break;

		case SERVO_DEC_AXIS:
			cmd	=	SETM2VELPID;
			break;

		default:
			// Neither RA or DEC selected, return error
			return(kERROR);
			break;
	}

	//printf("RC_stop: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Convert from float to internal RC integer format by multiply by 65536
	propo	*= 65536.0;
	deriv	*= 65536.0;
	integ	*= 65536.0;

	// Create the note for the comms and set end code to the offset
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
	// Note: the set command order is not PID---- but DPI---- 
	Note_add_dword(ptrA, deriv, &ptrB);
	Note_add_dword(ptrB, propo, &ptrA);
	Note_add_dword(ptrA, integ, &ptrB);
	Note_add_dword(ptrB, qpps, &ptrA);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrA - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrA, crc, &ptrB);

	RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_set_vel_pid()

//******************************************************************
// Restore the roboclaw to the factory settings
// Returns kSTATUS_OK or kERROR
// Send: [Address, 80, CRC(2 bytes)]
// Receive: [0xFF]
// TODO:  Add cmd 80 to the RC_cmd.h file and get rid of the hard coded lens
//******************************************************************
int RC_restore_defaults(uint8_t addr)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
//int			cmd;
int			len;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;

//	CONSOLE_DEBUG(__FUNCTION__);

	//printf("RC_stop: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Create the note for the comms and set end code to the offset
	Note_init(gNoteBuf, addr, 80, &ptrA);

	// Get length and calc CRC then add it the note
	len	=	(int)(ptrA - gNoteBuf);
	crc	=	MC_calc_crc16(gNoteBuf, len, kCLEAR_CRC);
	Note_add_word(ptrA, crc, &ptrB);

	// RC_converse(gNoteBuf, gRC[cmd].in, gReceiptBuf, gRC[cmd].out);
	RC_converse(gNoteBuf, 4, gReceiptBuf, 1);

	// Check the one byte return status
	status	=	Receipt_get_byte(gReceiptBuf, &ptrA);
	if (status != kRC_OK)
	{
		return(kERROR);
	}

	return(kSTATUS_OK);
} // of RC_restore_defaults()
//*************************************************************************
// Stop the motor by setting the duty cycle to zero
// Send: [Address, 32, Duty(2 Bytes), CRC(2 bytes)]
// Receive: [0xFF]
//*************************************************************************
int RC_stop(uint8_t addr, uint8_t motor)
{
uint8_t		*ptrA, *ptrB, status;
uint16_t	crc;
int			cmd;
int			len;
int			zero	=	0;
//uint16_t	receiptCrc;
//uint32_t	count;
//uint32_t	ret;

//	CONSOLE_DEBUG(__FUNCTION__);

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

	//printf("RC_stop: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// Create the note for the comms and set endcode to the offset
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
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
double RC_calc_move_time(int32_t pos0, int32_t pos1, uint32_t vel, uint32_t acc)
{
int32_t dist	=	abs(pos1 - pos0);
double distF, velF, accF, vCalc, time;

	// Cast the uints to doubles
	distF	=	(double)dist;
	velF	=	(double)vel;
	accF	=	(double)acc;

	// our accel = decel, does the profile have time to reach max vel, from physics
	// v = sqrt(2 * acc * displacement ) so use half the dist for displacement (2's cancel)
	// convert to floating absolute to avoid any ugliness from a sqrt of neg num
	vCalc	=	sqrt(fabs(accF * distF));

	// check to see if the move is long enough to get to max vel
	if (vCalc > velF)
	{
		// move reaches max velocity so standard trapezoidal print32_tofile
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
int RC_move_by_posv(uint8_t addr, uint8_t motor, int32_t pos, uint32_t vel, bool buffered)
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
	//printf("RC_move_by_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// If requesting a buffered command (eg. TRUE) set variable now to 0
	// otherwise, setting it to 1 means stop any running cmds and execute it now
	now	=	(buffered == true) ? 0 : 1;

	// Create the note for the comms and add vel, pos
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
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
// buffer the command if 'true'.   If 'false' it overwrites any executing cmds
// Moves the axis by pos steps with velocity and accel parameters
// Send: [Address, 65, Accel(4 bytes), Speed(4 Bytes), Decel(4 bytes), Position(4 Bytes), Buffer, CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_move_by_posva(uint8_t addr, uint8_t motor, int32_t pos, uint32_t vel, uint32_t acc, bool buffered)
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
	//printf("RC_move_by_pos: addr = %X cmd = %d gRC[cmd].cmd = %d\n", addr, cmd, gRC[cmd].cmd);

	// If requesting a buffered command (eg. TRUE) set variable now to 0
	// otherwise, setting it to 1 means stop any running cmds and execute it now
	now	=	(buffered == true) ? 0 : 1;

	// Create the note for the comms and add vel, pos
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
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
// Added workaround to negative position move with a negative velocity become a
// positive position move.  That is soooooo not documented :(
//******************************************************************
int RC_move_by_vela(uint8_t addr, uint8_t motor, int32_t vel, uint32_t acc, bool buffered)
{
int32_t	pos;
int		status;

	// if vel is negative, the subtract the faraway position from current
	status	=	kSTATUS_OK;
	switch (motor)
	{
		case SERVO_RA_AXIS:
			RC_get_curr_pos(addr, SERVO_RA_AXIS, &pos);
			if (vel < 0)
			{
				pos	=	- POS_FOREVER;
			}
			else
			{
				pos	=	POS_FOREVER;
			}
			RC_move_by_posva(addr, SERVO_RA_AXIS, pos, abs(vel), acc, buffered);
			break;

		case SERVO_DEC_AXIS:
			RC_get_curr_pos(addr, SERVO_DEC_AXIS, &pos);
			if (vel < 0)
			{
				pos	=	- POS_FOREVER;
			}
			else
			{
				pos	=	POS_FOREVER;
			}
			RC_move_by_posva(addr, SERVO_DEC_AXIS, pos, abs(vel), acc, buffered);
			break;

		default:
			// Neither RA or DEC selected, return error
			status	=	kERROR;
			break;
	}

	return(status);
} // of RC_move_by_vela()

//******************************************************************
// Moves the axis by signed velocity, this is an UNBUFFERED command
// and will clear our any current of pending actions in the command buffer
// Send: [Address, 35, Speed(4 Bytes), CRC(2 bytes)]
// Receive: [0xFF]
//******************************************************************
int RC_move_by_vel_raw(uint8_t addr, uint8_t motor, int32_t vel)
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
	Note_init(gNoteBuf, addr, gRC[cmd].cmd, &ptrA);
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
#include <time.h>
int	main(void)
{
uint8_t raDepth, decDepth; 
char buf[256];
int32_t pos	=	0;
uint32_t status = 0;
//uint32_t settings;
uint8_t addr = 0x80; // Default addr for RC MC
double 	propo, integ, deriv;
uint32_t iMax, deadZ;
int32_t minP, maxP;


	// Mark, Ignore this one line if format, need to see more statements on one screen, it's just for testing

	if (MC_init_comm("/dev/ttyACM0", 38400) != 0)
	{
		printf("Error: mc_init_comm() failed\n");
		return kERROR;
	}
	printf("Initializin RA and Dec Motors with unbuffered commands\n");

	if (RC_set_home(addr, SERVO_RA_AXIS) == kERROR)			printf("RC_set_home returned error\n");
	if (RC_get_curr_pos(addr, SERVO_RA_AXIS, &pos) == kERROR)		printf("RC_current_pos returned error\n");
	printf("RA pos = %X		", pos);

	if (RC_set_home(addr, SERVO_DEC_AXIS) == kERROR)			printf("RC_set_home returned error\n");
	if (RC_get_curr_pos(addr, SERVO_DEC_AXIS, &pos) == kERROR)		printf("RC_current_pos returned error\n");
	printf("Dec pos = %X\n", pos);
	if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	printf("\nTesting PID commands\n");
	// Get the current settings
	// RC_get_pos_pid(addr, SERVO_RA_AXIS,  &propo, &integ, &deriv, &iMax, &deadZ, &minP, &maxP);
	// printf("P:%d I:%d D:%d iMax:%d: Dz%d Min:%d Max:%d\n", propo, integ, deriv, iMax, deadZ, minP, maxP);

	// printf("Incrementing all values by 1\n");
	// propo++;
	// integ++;
	// deriv++;
	// iMax++;
	// deadZ++;
	// minP -= 16;
	// maxP -= 16; 
	// printf("P:%d I:%d D:%d iMax:%d: Dz%d Min:%d Max:%d\n", propo, integ, deriv, iMax, deadZ, minP, maxP);

	// printf("Setting the PID and then reading back\n");
	// RC_set_pos_pid(addr, SERVO_RA_AXIS,  propo, integ, deriv, iMax, deadZ, minP, maxP);
	RC_get_pos_pid(addr, SERVO_RA_AXIS,  &propo, &integ, &deriv, &iMax, &deadZ, &minP, &maxP);
	printf("POS P:%.2f I:%.2f D:%.2f iMax:%d: Dz:%d Min:%d Max:%d\n", propo, integ, deriv, iMax, deadZ, minP, maxP);

	RC_get_vel_pid(addr, SERVO_RA_AXIS,  &propo, &integ, &deriv, &iMax); 
	printf("VEL P:%.2f I:%.2f D:%.2f QPPS:%d\n", propo, integ, deriv, iMax);

	
	// printf("Writing new values to EEPROM\n");
	// RC_write_settings(addr); 
	// printf("Reading values from EEPROM\n");
	// RC_read_settings(addr, &settings); 

	// printf("Setting default acc\n");
	// RC_set_default_acc(addr, SERVO_RA_AXIS, 4000); 

	// printf("\nStarting RA and Dec Motors with unbuffered commands\n");
	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");

	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 10000, 20000, 5000, false) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 50000, 20000, 5000, false) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 50000, 20000, 5000, false) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// printf("\nOutput of buffers from 4 unbuffered moves:\n");
	// while ((raDepth & decDepth) != 0x80)
	// {
	// 	sleep(1);
	// 	RC_check_queue(addr, &raDepth, &decDepth);
	// 	printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// }

	// printf("\nOutput of buffers from 1 buffered move:\n");
	// RC_check_queue(addr, &raDepth, &decDepth);
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 10000, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 10000, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// printf("hit any key to reverse motors \n");
	// fgets(buf, 256, stdin);
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");

	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 0, 20000, 5000, false) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 0, 20000, 5000, false) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, raDepth, decDepth);

	// printf("hit any key to stop motors\n");
	// fgets(buf, 256, stdin);
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// RC_stop(addr, SERVO_RA_AXIS);
	// RC_stop(addr, SERVO_DEC_AXIS);
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// printf("\nStarting RA and Dec Motors with 4 buffered commands\n");
	// printf("hit any key to begin\n");
	// fgets(buf, 256, stdin);

	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, -10000, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 10000, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 0, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 0, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// if (RC_move_by_posva(addr, SERVO_RA_AXIS, 10000, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, -10000, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// 	if (RC_move_by_posva(addr, SERVO_RA_AXIS, 0, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_pos returned error\n");
	// if (RC_move_by_posva(addr, SERVO_DEC_AXIS, 0, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_pos returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// if (RC_check_queue(addr, &raDepth, &decDepth) == kERROR)		printf("RC_check_queue returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);

	// while ((raDepth & decDepth) != 0x80)
	// {
	// 	sleep(1);
	// 	RC_check_queue(addr, &raDepth, &decDepth);
	// 	printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);
	// }


	// printf("hit any key to stop motors\n");
	// fgets(buf, 256, stdin);

	// RC_stop(addr, SERVO_RA_AXIS);
	// RC_stop(addr, SERVO_DEC_AXIS);

	// printf("Now starting test for move by velocity\n");
	// if (RC_move_by_vela(addr, SERVO_RA_AXIS, 20000, 5000, true) == kERROR)		printf("RA RC_move_by_vela returned error\n");
	// if (RC_move_by_vela(addr, SERVO_DEC_AXIS, 20000, 5000, true) == kERROR)		printf("DEC RC_move_by_vela returned error\n");
	// if (RC_get_status(addr, &status) == kERROR)		printf("RC_get_status returned error\n");
	// //printf("Sleeping for 5 secs\n");
	// //sleep(5);

	// printf("hit any key to reverse velocity\n");
	// fgets(buf, 256, stdin);

	// RC_stop(addr, SERVO_RA_AXIS);
	// RC_stop(addr, SERVO_DEC_AXIS);
	
	// printf("Now testing reverse for velocity\n");

	// if (RC_move_by_vela(addr, SERVO_RA_AXIS, -20000, 5000, false) == kERROR)		printf("RA RC_move_by_vela returned error\n");
	// if (RC_move_by_vela(addr, SERVO_DEC_AXIS, -20000, 5000, false) == kERROR)		printf("DEC RC_move_by_vela returned error\n");
	// printf("*** status = %X  RA queue = %X  Dec queue = %X\n", status, (uint8_t) raDepth, (uint8_t) decDepth);


	printf("hit any key to stop motors\n");
	fgets(buf, 256, stdin);

	RC_stop(addr, SERVO_RA_AXIS);
	RC_stop(addr, SERVO_DEC_AXIS);

	MC_shutdown();

	return(kSTATUS_OK);
}
#endif
