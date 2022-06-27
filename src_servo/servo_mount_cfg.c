//******************************************************************************
//*	Name:			servo_mount_cfg.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Read/parses the servo mount config file for attributes
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
//*	Apr 18,	2022	<RNS> revamped the token field to use enums in prep for future enhancements
//*	Apr 20,	2022	<RNS> added config file support for MC_ADDR needed for RC MC
//*	Apr 21,	2022	<RNS> added support for default config filename and cleaned up tokens
//*	Apr 25,	2022	<RNS> added _TEST_ support for an unit build test and print_axis
//*	Apr 26,	2022	<RNS> added new field PARK_SIDE for GEM 'side' when parked
//*	May  1,	2022	<RNS> removed support for split and alt-azi mounts
//*	May  8,	2022	<RNS> updated #includes with servo_time.c for  str_to_upper()
//*	May  8,	2022	<RNS> added support for 38400 baud
//*	May 10,	2022	<MLS> Changed the keyword table
//*	May 10,	2022	<MLS> Added FindKeyWordEnum()
//*	May 14,	2022	<RNS> Added Time_ascii_maybe_HMS_tof for coordinate tokens park, sync, etc
//*	May 16,	2022	<RNS> Corrected nesting of error messages to handle invalid field in cfgfile
//*	May 16,	2022	<RNS> Adopted _TYPE_SCOPE_CONFIG_
//*	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
//*	May 20,	2022	<RNS> converted last 2 globals to use TYPE_MOUNT_CONFIG struct
//*	May 22,	2022	<RNS> Changed main() back to be void, _TEST_ should never take args
//*	May 22,	2022	<RNS> Added dummy var to check for addr field for range outside of uint8_t
//*	May 23,	2022	<MLS> Added ProcessConfigToken() to reduce cyclomatic complexity
//*	May 23,	2022	<MLS> Modified config processing so that error strings are returned
//*	May 26,	2022	<MLS> Added PrintMountConfiguration();
//*	May 26,	2022	<MLS> Updated test code to print out config
//*	Jun 12,	2022	<RNS> Added RC PID support and moved PIDL fields to float
//*	Jun 26,	2022	<RNS> Added support for TTP (thru-the-pole) config field
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define _ENABLE_CONSOLE_DEBUG_
#include "ConsoleDebug.h"

#include "servo_std_defs.h"
#include "servo_time.h"
#include "servo_mount_cfg.h"


//******************************************************************
TYPE_CFG_ITEM gMountConfigArray[] =
{
	{"MC_FREQ:",			MC_FREQ,		false},
	{"MC_ADDR:",			MC_ADDR,		false},
	{"BAUD:",				BAUD,			false},
	{"COMM_PORT:",			COMM_PORT,		false},
	{"MOUNT:",				MOUNT,			false},
	{"TTP:",				TTP,			false},
	{"PARK_SIDE:",			PARK_SIDE,		false},
	{"RA_MOTOR_MAX_RPM:",	RA_MOTOR_MAX_RPM,false},
	{"RA_MOTOR_GEAR:",		RA_MOTOR_GEAR,	false},
	{"RA_MAIN_GEAR:",		RA_MAIN_GEAR,	false},
	{"RA_ENCODER:",			RA_ENCODER,		false},
	{"RA_MAX_VEL:",			RA_MAX_VEL,		false},
	{"RA_MAX_ACC:",			RA_MAX_ACC,		false},
	{"RA_ADJ_VEL:",			RA_ADJ_VEL,		false},
	{"RA_SI_CON:",			RA_SI_CON,		false},
	{"RA_KP_CON:",			RA_KP_CON,		false},
	{"RA_KI_CON:",			RA_KI_CON,		false},
	{"RA_KD_CON:",			RA_KD_CON,		false},
	{"RA_IL_CON:",			RA_IL_CON,		false},
	{"DEC_MOTOR_MAX_RPM:",	DEC_MOTOR_MAX_RPM,false},
	{"DEC_MOTOR_GEAR:",		DEC_MOTOR_GEAR,	false},
	{"DEC_MAIN_GEAR:",		DEC_MAIN_GEAR,	false},
	{"DEC_ENCODER:",		DEC_ENCODER,	false},
	{"DEC_MAX_VEL:",		DEC_MAX_VEL,	false},
	{"DEC_MAX_ACC:",		DEC_MAX_ACC,	false},
	{"DEC_ADJ_VEL:",		DEC_ADJ_VEL,	false},
	{"DEC_SI_CON:",			DEC_SI_CON,		false},
	{"DEC_KP_CON:",			DEC_KP_CON,		false},
	{"DEC_KI_CON:",			DEC_KI_CON,		false},
	{"DEC_KD_CON:",			DEC_KD_CON,		false},
	{"DEC_IL_CON:",			DEC_IL_CON,		false},
	{"RA_CONFIG:",			RA_CONFIG,		false},
	{"DEC_CONFIG:",			DEC_CONFIG,		false},
	{"RA_GEAR_LASH:",		RA_GEAR_LASH,	false},
	{"DEC_GEAR_LASH:",		DEC_GEAR_LASH,	false},
	{"DEC_PARK:",			DEC_PARK,		false},
	{"RA_SLEW_VEL:",		RA_SLEW_VEL,	false},
	{"DEC_SLEW_VEL:",		DEC_SLEW_VEL,	false},
	{"RA_PARK:",			RA_PARK,		false},
	{"ROLLOVER_WIN:",		ROLLOVER_WIN,	false},
	{"RA_PRECESSION:",		RA_PRECESSION,	false},
	{"DEC_PRECESSION:",		DEC_PRECESSION,	false},
	{"RA_SENSOR:",			RA_SENSOR,		false},
	{"DEC_SENSOR:",			DEC_SENSOR,		false},
	{"RA_PARK_SENSOR:",		RA_PARK_SENSOR,	false},
	{"DEC_PARK_SENSOR:",	DEC_PARK_SENSOR,false},
	{"OFF_TARGET_TOL:",		OFF_TARGET_TOL,	false},
	{"",					-1,				false}
};

//******************************************************************
static int FindKeyWordEnum(const char *keyword)
{
int iii;
int enumValue;

	iii			=	0;
	enumValue	=	-1;
	while ((enumValue < 0) && (gMountConfigArray[iii].enumValue >= 0))
	{
		if (strcasecmp(keyword, gMountConfigArray[iii].parameter) == 0)
		{
			enumValue	=	gMountConfigArray[iii].enumValue;
			//*	verify that the array is in the right order
			if (enumValue != iii)
			{
				fprintf(stderr, "There is an order problem with the token array!!!!!!!!!!!!\r\n");
			}
		}
		iii++;
	}
	return (enumValue);
}

//******************************************************************************
//*	return error strings so they can be logged or displayed elsewhere
//******************************************************************************
static bool	ProcessConfigToken(	char				*token,
								char				*argument,
								TYPE_MOUNT_CONFIG	*mountConfig,
								char				*cfgErrorString1,
								char				*cfgErrorString2,
								char				*cfgErrorString3)
{
int				tokenEnumValue;
TYPE_MountAxis	*ra;
TYPE_MountAxis	*dec;
bool			configLineOK;
int				dummy;

//	CONSOLE_DEBUG(__FUNCTION__);

	cfgErrorString1[0]	=	0;
	cfgErrorString2[0]	=	0;
	cfgErrorString3[0]	=	0;

	ra	=	&mountConfig->ra;
	dec	=	&mountConfig->dec;

	tokenEnumValue	=	FindKeyWordEnum(token);
	configLineOK	=	true;
	switch (tokenEnumValue)
	{
		case MC_FREQ:
			gMountConfigArray[MC_FREQ].found	=	true;
			mountConfig->freq					=	atof(argument);
			// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, mountConfig->freq);

			if (mountConfig->freq == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, mountConfig->freq);
				sprintf(cfgErrorString2,	"Must be '1000000.0'");
				configLineOK	=	false;
			}
			break;

		case MC_ADDR:
			gMountConfigArray[MC_ADDR].found	=	true;
			// save to dummy to see if arg is out of range since addr is only 8bit
			dummy								=	atoi(argument);
			mountConfig->addr					=	atoi(argument);
		//	printf("%-15.15s = %-15d  \n", token, mountConfig->addr);

			if (dummy < 0 || dummy > 255)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%d'", token, dummy);
				sprintf(cfgErrorString2,	"Usage:  %s  128", token);
				configLineOK	=	false;
			}
			break;

		case BAUD:
			gMountConfigArray[BAUD].found	=	true;
			mountConfig->baud				=	atoi(argument);
		//	printf("%-15.15s = %-15d  \n", token, mountConfig->baud);

			if ((mountConfig->baud != 9600) && (mountConfig->baud != 19200) && (mountConfig->baud != 38400))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%d'", token, mountConfig->baud);
				sprintf(cfgErrorString2,	"Usage:  %s  9600", token);
				configLineOK	=	false;
			}
			break;

		case COMM_PORT:
			gMountConfigArray[COMM_PORT].found	=	true;
			strcpy(mountConfig->port, argument);
		//	printf("%-15.15s = %-15.15s  \n", token, mountConfig->port);
			break;

		case MOUNT:
			gMountConfigArray[MOUNT].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);
			if (!strcmp(argument, "FORK"))
			{
				mountConfig->mount	=	kFORK;
			}
			else if (!strcmp(argument, "GERMAN"))
			{
				mountConfig->mount	=	kGERMAN;

			}
			//		else if (!strcmp(argument, "SPLIT"))
			//		{
			//			mountConfig->mount	=	kSPLIT;
			//		}
			//		else if (!strcmp(argument, "ALTAZI"))
			//		{
			//			mountConfig->mount	=	kALTAZI;
			//		}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'FORK' or'GERMAN'");
				sprintf(cfgErrorString3,	"Usage:  %s  FORK", token);
				configLineOK	=	false;
			}
			break;

		case TTP:
			gMountConfigArray[TTP].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);

			if (!strcmp(argument, "YES"))
			{
				mountConfig->ttp	=	true;
			}
			else if (!strcmp(argument, "NO"))
			{
				mountConfig->ttp	=	false;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'YES' or'NO'");
				sprintf(cfgErrorString3,	"Usage:  %s  YES", token);
				configLineOK	=	false;
			}
			break;

		case PARK_SIDE:
			gMountConfigArray[PARK_SIDE].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);

			if (!strcmp(argument, "WEST"))
			{
				mountConfig->side			=	kWEST;
				mountConfig->ra.parkInfo	=	kWEST;
			}
			else if (!strcmp(argument, "EAST"))
			{
				mountConfig->side			=	kEAST;
				mountConfig->ra.parkInfo	=	kEAST;
			}
			else if (!strcmp(argument, "NONE"))
			{
				mountConfig->side			=	kNONE;
				mountConfig->ra.parkInfo	=	kNONE;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'EAST', 'WEST', 'NONE'");
				sprintf(cfgErrorString3,	"Usage:  %s  EAST", token);
				configLineOK	=	false;
			}
			break;

		case RA_MOTOR_MAX_RPM:
			gMountConfigArray[RA_MOTOR_MAX_RPM].found	=	true;
			ra->motorMaxRPM								=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->motorMaxRPM);

			if (ra->motorMaxRPM <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'",	token, ra->motorMaxRPM);
				sprintf(cfgErrorString2,	"Usage:  %s  7200.0",		token);
				configLineOK	=	false;
			}
			break;

		case RA_MOTOR_GEAR:
			gMountConfigArray[RA_MOTOR_GEAR].found	=	true;
			ra->motorGear							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->motorGear);

			if (ra->motorGear <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->motorGear);
				sprintf(cfgErrorString2,	"Usage:  %s  18.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_MAIN_GEAR:
			gMountConfigArray[RA_MAIN_GEAR].found	=	true;
			ra->mainGear							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->mainGear);

			if (ra->mainGear <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->mainGear);
				sprintf(cfgErrorString2,	"Usage:  %s  359.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_ENCODER:
			gMountConfigArray[RA_ENCODER].found	=	true;
			ra->encoder							=	atof(argument);
		//	printf("%-15.15s = %-15.4lf  \n", token, ra->encoder);

			if (ra->encoder == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->encoder);
				sprintf(cfgErrorString2,	"Usage:  %s  2000.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_MAX_VEL:
			gMountConfigArray[RA_MAX_VEL].found	=	true;
			ra->realVel							=	atof(argument);
		//	printf("%-15.15s = %-15.4lf  \n", token, ra->realVel);

			if (ra->realVel == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->realVel);
				sprintf(cfgErrorString2,	"Usage:  %s  15000.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_MAX_ACC:
			gMountConfigArray[RA_MAX_ACC].found	=	true;
			ra->realAcc							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->realAcc);

			if (ra->realAcc == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->realAcc);
				sprintf(cfgErrorString2,	"Usage:  %s  10.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_ADJ_VEL:
			gMountConfigArray[RA_ADJ_VEL].found	=	true;
			ra->realAdj							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->realAdj);

			if (ra->realAdj == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->realAdj);
				sprintf(cfgErrorString2,	"Usage:  %s  900.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_SI_CON:
			gMountConfigArray[RA_SI_CON].found	=	true;
			ra->si								=	(uint8_t)atoi(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->si);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%d'", token, (int)ra->si);
				sprintf(cfgErrorString2,	"Usage:  %s  0", token);
				configLineOK	=	false;
				configLineOK	=	false;
			}
			break;

		case RA_KP_CON:
			gMountConfigArray[RA_KP_CON].found	=	true;
			ra->kp								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->kp);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->kp);
				sprintf(cfgErrorString2,	"Usage:  %s  60.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_KI_CON:
			gMountConfigArray[RA_KI_CON].found	=	true;
			ra->ki								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->ki);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->ki);
				sprintf(cfgErrorString2,	"Usage:  %s  30.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_KD_CON:
			gMountConfigArray[RA_KD_CON].found	=	true;
			ra->kd								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->kd);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->kd);
				sprintf(cfgErrorString2,	"Usage:  %s  300.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_IL_CON:
			gMountConfigArray[RA_IL_CON].found	=	true;
			ra->il								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->il);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->il);
				sprintf(cfgErrorString2,	"Usage:  %s  130.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_MOTOR_MAX_RPM:
			gMountConfigArray[DEC_MOTOR_MAX_RPM].found	=	true;
			dec->motorMaxRPM							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->motorMaxRPM);

			if (ra->motorMaxRPM <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->motorMaxRPM);
				sprintf(cfgErrorString2,	"Usage:  %s  7200.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_MOTOR_GEAR:
			gMountConfigArray[DEC_MOTOR_GEAR].found	=	true;
			dec->motorGear							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->motorGear);

			if (dec->motorGear == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->motorGear);
				sprintf(cfgErrorString2,	"Usage:  %s  18.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_MAIN_GEAR:
			gMountConfigArray[DEC_MAIN_GEAR].found	=	true;
			dec->mainGear							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->mainGear);

			if (dec->mainGear == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->mainGear);
				sprintf(cfgErrorString2,	"Usage:  %s  359.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_ENCODER:
			gMountConfigArray[DEC_ENCODER].found	=	true;
			dec->encoder							=	atof(argument);
		//	printf("%-15.15s = %-15.4lf  \n", token, dec->encoder);

			if (dec->encoder == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->encoder);
				sprintf(cfgErrorString2,	"Usage:  %s  2000.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_MAX_VEL:
			gMountConfigArray[DEC_MAX_VEL].found	=	true;
			dec->realVel							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->realVel);

			if (dec->realVel == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->realVel);
				sprintf(cfgErrorString2,	"Usage:  %s  15000.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_MAX_ACC:
			gMountConfigArray[DEC_MAX_ACC].found	=	true;
			dec->realAcc							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->realAcc);

			if (dec->realAcc == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->realAcc);
				sprintf(cfgErrorString2,	"Usage:  %s  10.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_ADJ_VEL:
			gMountConfigArray[DEC_ADJ_VEL].found	=	true;
			dec->realAdj							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->realAdj);

			if (dec->realAdj == 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->realAdj);
				sprintf(cfgErrorString2,	"Usage:  %s  900.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_SI_CON:
			gMountConfigArray[DEC_SI_CON].found	=	true;
			dec->si								=	(uint8_t)atoi(argument);
		//	printf("%-15.15s = %-15d  \n", token, dec->si);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%d'", token, dec->si);
				sprintf(cfgErrorString2,	"Usage:  %s  0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_KP_CON:
			gMountConfigArray[DEC_KP_CON].found	=	true;
			dec->kp								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, dec->kp);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->kp);
				sprintf(cfgErrorString2,	"Usage:  %s  60.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_KI_CON:
			gMountConfigArray[DEC_KI_CON].found	=	true;
			dec->ki								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, dec->ki);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->ki);
				sprintf(cfgErrorString2,	"Usage:  %s  60.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_KD_CON:
			gMountConfigArray[DEC_KD_CON].found	=	true;
			dec->kd								=	atof(argument);

		//	printf("%-15.15s = %-15d  \n", token, dec->kd);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->kd);
				sprintf(cfgErrorString2,	"Usage:  %s  300.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_IL_CON:
			gMountConfigArray[DEC_IL_CON].found	=	true;
			dec->il								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, dec->il);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->il);
				sprintf(cfgErrorString2,	"Usage:  %s  130.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_CONFIG:
			gMountConfigArray[RA_CONFIG].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);

			if (!strcmp(argument, "FORWARD"))
			{
				ra->config	=	kFORWARD;
			}
			else if (!strcmp(argument, "REVERSE"))
			{
				ra->config	=	kREVERSE;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'FORWARD' or 'REVERSE'");
				sprintf(cfgErrorString3,	"Usage:  %s  FORWARD", token);
				configLineOK	=	false;
			}
			break;

		case DEC_CONFIG:
			gMountConfigArray[DEC_CONFIG].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);

			if (!strcmp(argument, "FORWARD"))
			{
				dec->config		=	kFORWARD;
				dec->parkInfo	=	kFORWARD;
			}
			else if (!strcmp(argument, "REVERSE"))
			{
				dec->config		=	kREVERSE;
				dec->parkInfo	=	kREVERSE;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'FORWARD' or 'REVERSE'");
				sprintf(cfgErrorString3,	"Usage:  %s  FORWARD", token);
				configLineOK	=	false;
			}
			//	printf("dec->parkInfo = 0x%X\n", dec->parkInfo);
			break;

		case RA_GEAR_LASH:
			gMountConfigArray[RA_GEAR_LASH].found	=	true;
			ra->gearLash							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->gearLash);

			if (ra->gearLash < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->gearLash);
				sprintf(cfgErrorString2,	"Usage:  %s  1500.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_GEAR_LASH:
			gMountConfigArray[DEC_GEAR_LASH].found	=	true;
			dec->gearLash							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->gearLash);

			if (dec->gearLash < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->gearLash);
				sprintf(cfgErrorString2,	"Usage:  %s  1500.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_PARK:
			gMountConfigArray[DEC_PARK].found	=	true;
			dec->park							=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->park);

			if (dec->park < -91.0 || dec->park > 91.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->park);
				sprintf(cfgErrorString2,	"Usage:  %s  0.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_SLEW_VEL:
			gMountConfigArray[RA_SLEW_VEL].found	=	true;
			ra->realSlew							=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->realSlew);

			if (ra->realSlew <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->realSlew);
				sprintf(cfgErrorString2,	"Usage:  %s  1000.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_SLEW_VEL:
			gMountConfigArray[DEC_SLEW_VEL].found	=	true;
			dec->realSlew							=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->realSlew);

			if (dec->realSlew <= 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->realSlew);
				sprintf(cfgErrorString2,	"Usage:  %s  1000.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_PARK:
			gMountConfigArray[RA_PARK].found	=	true;
			ra->park							=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->park);

			if (ra->park < -7.0 || ra->park > 7.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->park);
				sprintf(cfgErrorString2,	"Usage:  %s  0.0", token);
				configLineOK	=	false;
			}
			break;

		case ROLLOVER_WIN:
			gMountConfigArray[ROLLOVER_WIN].found	=	true;
			mountConfig->flipWin					=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, mountConfig->flipWin);

			if (mountConfig->flipWin < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, mountConfig->flipWin);
				sprintf(cfgErrorString2,	"Usage:  %s  1.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_PRECESSION:
			gMountConfigArray[RA_PRECESSION].found	=	true;
			ra->prec								=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->prec);

			if (ra->prec < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->prec);
				sprintf(cfgErrorString2,	"Usage:  %s  0.32", token);
				configLineOK	=	false;
			}
			break;

		case DEC_PRECESSION:
			gMountConfigArray[DEC_PRECESSION].found	=	true;
			dec->prec								=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->prec);

			if (dec->prec < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->prec);
				sprintf(cfgErrorString2,	"Usage:  %s  0.60", token);
				configLineOK	=	false;
			}
			break;

		case RA_SENSOR:
			gMountConfigArray[RA_SENSOR].found	=	true;
			ra->sync							=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, ra->sync);

			if (ra->sync < -7.0 || ra->sync > 7.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->sync);
				sprintf(cfgErrorString2,	"Usage:  %s  0.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_SENSOR:
			gMountConfigArray[DEC_SENSOR].found	=	true;
			dec->sync							=	Time_ascii_maybe_HMS_tof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, dec->sync);

			if (dec->sync < -91.0 || dec->sync > 91.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->sync);
				sprintf(cfgErrorString2,	"Usage:  %s  0.0", token);
				configLineOK	=	false;
			}
			break;

		case RA_PARK_SENSOR:
			gMountConfigArray[RA_PARK_SENSOR].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);

			if (strcmp(argument, "ON") == 0)
			{
				ra->syncValue	=	true;
			}
			else if (strcmp(argument, "OFF") == 0)
			{
				ra->syncValue	=	false;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'ON' or 'OFF'");
				sprintf(cfgErrorString3,	"Usage:  %s  ON", token);
				configLineOK	=	false;
			}
			break;

		case DEC_PARK_SENSOR:
			gMountConfigArray[DEC_PARK_SENSOR].found	=	true;
		//	printf("%-15.15s = %-15.15s  \n", token, argument);
			if (strcmp(argument, "ON") == 0)
			{
				dec->syncValue	=	true;
			}
			else if (strcmp(argument, "OFF") == 0)
			{
				dec->syncValue	=	false;
			}
			else
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Must be 'ON' or 'OFF'");
				sprintf(cfgErrorString3,	"Usage:  %s  ON", token);
				configLineOK	=	false;
			}
			break;

		case OFF_TARGET_TOL:
			gMountConfigArray[OFF_TARGET_TOL].found	=	true;
			mountConfig->offTarget					=	atof(argument);
		//	printf("%-15.15s = %-15.4f  \n", token, mountConfig->offTarget);
			if (mountConfig->offTarget < 0.0)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, mountConfig->offTarget);
				sprintf(cfgErrorString2,	"Usage:  %s  1.0", token);
				configLineOK	=	false;
			}
			break;

		default:
			sprintf(cfgErrorString1,	"Unrecognized %s field", token);
			configLineOK	=	false;
			break;
	}
	return(configLineOK);
}


//******************************************************************
int Servo_read_mount_cfg(const char *mountCfgFile, TYPE_MOUNT_CONFIG *mountConfig)
{
char		filename[kMAX_STR_LEN];
char		inString[kMAX_STR_LEN];
FILE		*inFile;
uint16_t	line	=	1;
bool		okFlag	=	true;
char		delimiters[]	=	" \t\r\n\v\f";	// POSIX whitespace chars
char		*token;
char		*argument;
char		*rest	=	NULL;
int			iii;
int			retStatus;
bool		configLineOK;
char		cfgErrorString1[80];
char		cfgErrorString2[80];
char		cfgErrorString3[80];

	CONSOLE_DEBUG(__FUNCTION__);

	retStatus	=	-1;
	//*	first, go through and invalidate all of the parameters
	iii	=	0;
	while (strlen(gMountConfigArray[iii].parameter) > 0)
	{
		gMountConfigArray[iii++].found	=	false;
	}

	// If no filename specified, use the default
	if (mountCfgFile == NULL)
	{
		strcpy(filename, kSCOPE_CFG_FILE);
	}
	else
	{
		strcpy(filename, mountCfgFile);
	}
	// open the mount configuration file
	inFile	=	fopen(filename, "r");
	if (inFile != NULL)
	{
		// get all of the lines in the file
		while (fgets(inString, kMAX_STR_LEN, inFile) != NULL)
		{
			// get first token of the line read from the file
			token	=	strtok_r(inString, delimiters, &rest);

			// If not a blank line and non-comment token found on line
			if ((token != NULL) && (token[0] != '#'))
			{
				// <RNS> need to check the if the token is valid before printing otherwise error message is wacky
				// token valid, so print it
	//				printf("Token:%-20s\n", token);

				// Get corresponding argument for the token
				argument	=	strtok_r(NULL, delimiters, &rest);
				if (argument != NULL)
				{
					// Token and argument exist, now determine if they are valid
					configLineOK	=	ProcessConfigToken(	token,
															argument,
															mountConfig,
															cfgErrorString1,
															cfgErrorString2,
															cfgErrorString3);
					if (configLineOK == false)
					{
						fprintf(stderr, "Error: (init_mount_cfg) on line %d of file '%s'\n", line, filename);
						fprintf(stderr, "       %s\n", cfgErrorString1);
						fprintf(stderr, "       %s\n", cfgErrorString2);
						fprintf(stderr, "       %s\n", cfgErrorString3);
					}
				}
				else
				{
					fprintf(stderr, "Error: (init_mount_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid syntax: field %s is missing argument\n", token);
				}
			}	// of if not a comment line
			line++;		// increment the line counter
		}	// of while
		fclose(inFile);

		// Even out the columns before any missing args error statements
		//printf("\n");
//		PrintMountConfiguration();

		iii	=	0;
		while (strlen(gMountConfigArray[iii].parameter) > 0)
		{
			if (gMountConfigArray[iii].found == false)
			{
				fprintf(stderr, "Error: (validate_mount_cfg) Configuation variable:\n");
				fprintf(stderr, "       '%s' was not found or of improper format.\n",
											gMountConfigArray[iii].parameter);
				fprintf(stderr, "       from file '%s'\n", filename);
				okFlag	=	false;
			}
			iii++;
		}

		// configuration file had a syntax error
		if (okFlag == false)
		{
			fprintf(stderr, "Error: (validate_mount_cfg) Error found in configuration:\n");
			fprintf(stderr, "       from file '%s'\n", filename);
			fflush(stderr);
			retStatus	=	-1;
		}
		else
		{
			// No Errors in configuration file

			// If mount is FORK, then set the side to EAST due to ASCOM default definitions
			if (mountConfig->mount == kFORK)
			{
				mountConfig->side	=	kEAST;
			}

			retStatus	=	0;
		}
	}
	else		//*	file open
	{
		fprintf(stderr, "Error: could not open cfg file %s\n", filename);
		retStatus	=	-1;
	}	//*	of fopen
	return (retStatus);
}	// of Servo_read_mount_cfg()


#ifdef _TEST_SERVO_MOUNT_CFG_
//**************************************************************************************
static void	PrintConfigParam_Dbl(const int cfgEnum, const double value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15.4f  \n", gMountConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void	PrintConfigParam_Str(const int cfgEnum, const char *value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %s  \n", gMountConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void	PrintConfigParam_Int(const int cfgEnum, const int value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15d  \n", gMountConfigArray[cfgEnum].parameter, value);
	}
}

//******************************************************************
void	PrintMountConfiguration(void)
{
char	lineBuff[64];

	PrintConfigParam_Dbl(MC_FREQ,		gMountConfig.freq);
	PrintConfigParam_Int(MC_ADDR,		gMountConfig.addr);
	PrintConfigParam_Str(COMM_PORT,		gMountConfig.port);
	PrintConfigParam_Int(BAUD,			gMountConfig.baud);
	switch(gMountConfig.mount)
	{
		case kFORK:		strcpy(lineBuff,	"FORK");	break;
		case kGERMAN:	strcpy(lineBuff,	"GERMAN");	break;
		default:		strcpy(lineBuff,	"unknown");	break;
	}
	PrintConfigParam_Str(MOUNT,			lineBuff);

	switch(gMountConfig.side)
	{
		case kEAST:		strcpy(lineBuff,	"EAST");	break;
		case kWEST:		strcpy(lineBuff,	"WEST");	break;
		case kNONE:		strcpy(lineBuff,	"NONE");	break;
		default:		strcpy(lineBuff,	"unknown");	break;
	}
	PrintConfigParam_Str(PARK_SIDE,			lineBuff);
	PrintConfigParam_Int(TTP,				gMountConfig.ttp);
	PrintConfigParam_Dbl(ROLLOVER_WIN,		gMountConfig.flipWin);
	PrintConfigParam_Dbl(OFF_TARGET_TOL,	gMountConfig.offTarget);


	//--------------------------------------------------------------------------------
	PrintConfigParam_Dbl(RA_MOTOR_GEAR,		gMountConfig.ra.motorGear);
	PrintConfigParam_Dbl(RA_MAIN_GEAR,		gMountConfig.ra.mainGear);
	PrintConfigParam_Dbl(RA_MOTOR_MAX_RPM,	gMountConfig.ra.motorMaxRPM);
	PrintConfigParam_Dbl(RA_ENCODER,		gMountConfig.ra.encoder);
	PrintConfigParam_Dbl(RA_MAX_ACC,		gMountConfig.ra.realAcc);
	PrintConfigParam_Dbl(RA_MAX_VEL,		gMountConfig.ra.realVel);
	PrintConfigParam_Dbl(RA_ADJ_VEL,		gMountConfig.ra.realAdj);
	PrintConfigParam_Dbl(RA_SLEW_VEL,		gMountConfig.ra.realSlew);
	PrintConfigParam_Dbl(RA_CONFIG,			gMountConfig.ra.config);
	PrintConfigParam_Dbl(RA_PRECESSION,		gMountConfig.ra.prec);
	PrintConfigParam_Dbl(RA_PARK,			gMountConfig.ra.park);

	PrintConfigParam_Int(RA_SI_CON,			gMountConfig.ra.si);
	PrintConfigParam_Int(RA_KP_CON,			gMountConfig.ra.kp);
	PrintConfigParam_Int(RA_KI_CON,			gMountConfig.ra.ki);
	PrintConfigParam_Int(RA_KD_CON,			gMountConfig.ra.kd);
	PrintConfigParam_Int(RA_IL_CON,			gMountConfig.ra.il);
	PrintConfigParam_Dbl(RA_GEAR_LASH,		gMountConfig.ra.gearLash);
	PrintConfigParam_Dbl(RA_SENSOR,			gMountConfig.ra.sync);
	PrintConfigParam_Int(RA_PARK_SENSOR,	gMountConfig.ra.syncValue);

	//--------------------------------------------------------------------------------
	PrintConfigParam_Dbl(DEC_MOTOR_GEAR,	gMountConfig.dec.motorGear);
	PrintConfigParam_Dbl(DEC_MAIN_GEAR,		gMountConfig.dec.mainGear);
	PrintConfigParam_Dbl(DEC_MOTOR_MAX_RPM,	gMountConfig.dec.motorMaxRPM);
	PrintConfigParam_Dbl(DEC_ENCODER,		gMountConfig.dec.encoder);
	PrintConfigParam_Dbl(DEC_MAX_ACC,		gMountConfig.dec.realAcc);
	PrintConfigParam_Dbl(DEC_MAX_VEL,		gMountConfig.dec.realVel);
	PrintConfigParam_Dbl(DEC_ADJ_VEL,		gMountConfig.dec.realAdj);
	PrintConfigParam_Dbl(DEC_SLEW_VEL,		gMountConfig.dec.realSlew);
	PrintConfigParam_Dbl(DEC_CONFIG,		gMountConfig.dec.config);
	PrintConfigParam_Dbl(DEC_PRECESSION,	gMountConfig.dec.prec);
	PrintConfigParam_Dbl(DEC_PARK,			gMountConfig.dec.park);

	PrintConfigParam_Int(DEC_SI_CON,		gMountConfig.dec.si);
	PrintConfigParam_Int(DEC_KP_CON,		gMountConfig.dec.kp);
	PrintConfigParam_Int(DEC_KI_CON,		gMountConfig.dec.ki);
	PrintConfigParam_Int(DEC_KD_CON,		gMountConfig.dec.kd);
	PrintConfigParam_Int(DEC_IL_CON,		gMountConfig.dec.il);
	PrintConfigParam_Dbl(DEC_GEAR_LASH,		gMountConfig.dec.gearLash);
	PrintConfigParam_Dbl(DEC_SENSOR,		gMountConfig.dec.sync);
	PrintConfigParam_Int(DEC_PARK_SENSOR,	gMountConfig.dec.syncValue);
}

//********************************************************************************************
//* Dump the axisPtr data structure to standard out */
//********************************************************************************************
void Test_print_axis(TYPE_MountAxis *ax)
{
	printf("ax->motorGear = %f\n", ax->motorGear);
	printf("ax->mainGear = %f\n", ax->mainGear);
	printf("ax->encoder = %f\n", ax->encoder);
	printf("ax->realAcc = %f\n", ax->realAcc);
	printf("ax->realVel = %f\n", ax->realVel);
	printf("ax->realAdj = %f\n", ax->realAdj);
	printf("ax->config = %f\n", ax->config);
	printf("ax->step = %f\n", ax->step);
	printf("ax->pos = %d\n", ax->pos);
	printf("ax->maxAcc = %d\n", ax->maxAcc);
	printf("ax->acc = %d\n", ax->acc);
	printf("ax->maxVel = %d\n", ax->maxVel);
	printf("ax->vel = %d\n", ax->vel);
	printf("ax->adj = %d\n", ax->adj);
	printf("ax->si = %d\n", ax->si);
	printf("ax->kp = %d\n", ax->kp);
	printf("ax->ki = %d\n", ax->ki);
	printf("ax->kd = %d\n", ax->kd);
	printf("ax->il = %d\n", ax->il);
	printf("ax->status = %x\n", ax->status);
	printf("ax->track = %d\n", ax->track);
	printf("ax->direction = %f\n", ax->direction);
	printf("ax->park = %f\n", ax->park);
	printf("ax->time = %Lf\n", ax->time);
	printf("ax->sync = %f\n", ax->sync);
	printf("ax->syncValue = %d\n", ax->syncValue);
	printf("ax->syncError = %d\n", ax->syncError);
}

TYPE_MOUNT_CONFIG gMountConfig;

//********************************************************************************************
int main(void)
{
char configFile[]	=	"servo_mount.cfg";

	printf("file name = %s\n", configFile);

	Servo_read_mount_cfg(configFile, &gMountConfig);

	PrintMountConfiguration();

	return (0);
}
#endif	//	_TEST_SERVO_MOUNT_CFG_
