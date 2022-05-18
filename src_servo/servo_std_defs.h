//*****************************************************************************
//*	Name:			servo_std_defs.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description:            LM628/629 programming library in C
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
//*	Mar 31,	2022	<RNS> Created original version via port from servostar
//*	Apr 22,	2022	<RNS> Pulled in cfg_item typedef
//*	Apr 25,	2022	<RNS> Pulled in some simple defines for a common location
//*	Apr 25,	2022	<RNS> Removed legacy int/char typedefs, add str_to_upper()
//*	Apr 29,	2022	<RNS> Moved in common defines used in multiple files
//*	Apr 29,	2022	<RNS> Updated arcsec/sec
//*	May  6,	2022	<RNS> Moved str_to_upper() to servo_time.c
//*	May 11,	2022	<MLS> Changed constant RA to SERVO_RA_AXIS
//*	May 11,	2022	<MLS> Changed constant DEC to SERVO_DEC_AXIS
//*	May 14,	2022	<MLS> Removed degToRads/radToDegs
//*****************************************************************************

#ifndef	_INCLUDED_SERVO_STD_DEFS_
#define	_INCLUDED_SERVO_STD_DEFS_

// standard defines for c types
#define	kFOUND			1
#define	kNOT_FOUND		0
#define	kSTATUS_OK		0
#define	kERROR			-1
#define	kBELOW_HORIZON	-1

#define	kFORWARD		(1.0)
#define	kREVERSE		(-1.0)

#define	kMAX_STR_LEN	256
#define	kSMALL_STR_LEN	32

#define	kALTAZI			'a'
#define	kFORK			'f'
#define	kGERMAN			'g'
#define	kSPLIT			's'
#define	kTEST			't'

#define	kEAST			'e'
#define	kWEST			'w'
#define	kNONE			'n'

#define	kARCSEC_PER_SEC	15.04106864

#define	SERVO_RA_AXIS	(uint8_t)0
#define	SERVO_DEC_AXIS	(uint8_t)1

#define	DEGREES(radians)	((radians) * (180.0 / M_PI))
#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

//*****************************************************************************
typedef struct cfgItem_t
{
	char *parameter;
	int found;
} cfgItem, cfgItemPtr;

#endif
