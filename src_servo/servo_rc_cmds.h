//*****************************************************************************
//*	Name:			servo_rc_cmds.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description:    Command array for Roboclaw MC used by RC_utils
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
//*	Apr 19,	2022	<RNS> Initially created by spreadsheet
//*	Apr 26,	2022	<RNS> Update spreadsheet to pick up MxDUTY cmns for stop()
//*	Apr 27,	2022	<RNS> Update spreadsheet to add yet more commands
//*	May  6,	2022	<RNS> changed filename to servo_rc_cmds, fixed #defines
//*	May  6,	2022	<RNS> Fixed corrupted spreadsheet data
//*	Jun  6,	2022	<RNS> Fixed naming convetion from READ to GET for PID cmds
//*****************************************************************************
#ifndef _SERVO_RC_CMDS_H_
#define _SERVO_RC_CMDS_H_
// This file needs to be auto generated from the Roboclaw command
// details spreadsheet for selected and supported commands required
// by the servo system

typedef struct cmdTable_t
{
	uint8_t	cmd;	//	RC command number
	uint8_t	in;		//	length of data in to RC
	uint8_t	out;	// length of data out from RC
} cmdTable, *cmdTablePtr;

//*****************************************************************************
#define RC_NUM_CMDS 72
enum
{
	SETSERIALTO				=	0,
	GETSERIALTO				=	1,
	GETM1ENCVALUE			=	2,
	GETM2ENCVALUE			=	3,
	GETM1ENCSPEED			=	4,
	GETM2ENCSPEED			=	5,
	SETM1ENCCOUNT			=	6,
	SETM2ENCCOUNT			=	7,
	GETMAINVOLTAGE			=	8,
	GETLOGICvOLTAGE			=	9,
	SETM1VELPID				=	10,
	SETM2VELPID				=	11,
	M1DUTY					=	12,
	M2DUTY					=	13,
	MIXEDDUTY				=	14,
	M1SPEED					=	15,
	M2SPEED					=	16,
	MIXEDSPEED				=	17,
	GETBUFFERS				=	18,
	GETPWMS					=	19,
	GETCURRENTS				=	20,
	GETM1VELPID				=	21,
	GETM2VELPID				=	22,
	SETMAINVOLTAGES			=	23,
	SETLOGICVOLTAGES		=	24,
	GETMAINVOLTSETTING		=	25,
	GETLOGICVOLTSETTING		=	26,
	SETM1POSPID				=	27,
	SETM2POSPID				=	28,
	GETM1POSPID				=	29,
	GETM2POSPID				=	30,
	M1SPEEDACCELDECELPOS	=	31,
	M2SPEEDACCELDECELPOS	=	32,
	MIXEDSPEEDACCELDECELPOS	=	33,
	SETM1DEFAULTACCEL		=	34,
	SETM2DEFAULTACCEL		=	35,
	SETM1DEFAULTSPEED		=	36,
	SETM2DEFAULTSPEED		=	37,
	GETDEFAULTSPEED			=	38,
	SETPINFUNCTIONS			=	39,
	GETPINFUNCTIONS			=	40,
	GETENCODERVALUES		=	41,
	GETDEFAULTACCEL			=	42,
	GETTEMP					=	43,
	GETSTATUS				=	44,
	GETENCODERMODE			=	45,
	SETM1ENCODERMODE		=	46,
	SETM2ENCODERMODE		=	47,
	WRITENVM				=	48,
	READNVM					=	49,
	SETCONFIG				=	50,
	GETCONFIG				=	51,
	SETSPEEDERRLIMITS		=	52,
	GETSPEEDERRLIMITS		=	53,
	GETSPEEDERRORS			=	54,
	SETPOSERRLIMITS			=	55,
	GETPOSERRLIMITS			=	56,
	GETPOSERRORS			=	57,
	M1POS					=	58,
	M2POS					=	59,
	MIXEDPOS				=	60,
	M1SPEEDPOS				=	61,
	M2SPEEDPOS				=	62,
	MIXEDSPEEDPOS			=	63,
	SETM1MAXCURRENT			=	64,
	SETM2MAXCURRENT			=	65,
	GETM1MAXCURRENT			=	66,
	GETM2MAXCURRENT			=	67,
	SETPWMMODE				=	68,
	GETPWMMODE				=	69,
	SETNVM					=	70,
	GETNVM					=	71
};

// CmdID, bytes writen, bytes received
cmdTable gRC[RC_NUM_CMDS]	=
{
	{	14	,	5	,	1	},
	{	15	,	2	,	3	},
	{	16	,	2	,	7	},
	{	17	,	2	,	7	},
	{	18	,	2	,	7	},
	{	19	,	2	,	7	},
	{	22	,	8	,	1	},
	{	23	,	8	,	1	},
	{	24	,	2	,	4	},
	{	25	,	2	,	4	},
	{	28	,	20	,	1	},
	{	29	,	20	,	1	},
	{	32	,	6	,	1	},
	{	33	,	6	,	1	},
	{	34	,	8	,	1	},
	{	35	,	8	,	1	},
	{	36	,	8	,	1	},
	{	37	,	12	,	1	},
	{	47	,	2	,	4	},
	{	48	,	2	,	6	},
	{	49	,	2	,	6	},
	{	55	,	2	,	18	},
	{	56	,	2	,	18	},
	{	57	,	8	,	1	},
	{	58	,	8	,	1	},
	{	59	,	2	,	6	},
	{	60	,	2	,	6	},
	{	61	,	32	,	1	},
	{	62	,	32	,	1	},
	{	63	,	2	,	30	},
	{	64	,	2	,	30	},
	{	65	,	21	,	1	},
	{	66	,	21	,	1	},
	{	67	,	37	,	1	},
	{	68	,	8	,	1	},
	{	69	,	8	,	1	},
	{	70	,	6	,	1	},
	{	71	,	6	,	1	},
	{	72	,	2	,	6	},
	{	74	,	7	,	1	},
	{	75	,	2	,	5	},
	{	78	,	2	,	10	},
	{	81	,	2	,	10	},
	{	82	,	2	,	4	},
	{	90	,	2	,	6	},
	{	91	,	2	,	4	},
	{	92	,	5	,	1	},
	{	93	,	5	,	1	},
	{	94	,	2	,	1	},
	{	95	,	2	,	4	},
	{	98	,	6	,	1	},
	{	99	,	2	,	4	},
	{	109	,	12	,	1	},
	{	110	,	2	,	10	},
	{	111	,	2	,	10	},
	{	112	,	12	,	1	},
	{	113	,	2	,	10	},
	{	114	,	2	,	10	},
	{	119	,	9	,	1	},
	{	120	,	9	,	1	},
	{	121	,	13	,	1	},
	{	122	,	13	,	1	},
	{	123	,	13	,	1	},
	{	124	,	21	,	1	},
	{	133	,	12	,	1	},
	{	134	,	12	,	1	},
	{	135	,	2	,	10	},
	{	136	,	2	,	10	},
	{	148	,	5	,	1	},
	{	149	,	2	,	3	},
	{	252	,	7	,	1	},
	{	253	,	3	,	4	}
};
#endif // of _SERVO_RC_CMDS_H_
