//******************************************************************************
//*	Name:			servo_motion_cfg.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Read/parses the servo motion config file for attributes
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
//*	Jun 27,	2022	<RNS> Initial version of Motion read config routines
//*	Jun 30,	2022	<RNS> Renamed local globals prefix to 'gs'
//*	Jul  2,	2022,	<RNS> Renamed .adj field to .guide, removed .freq field
//*	Jul  2,	2022,	<RNS> Updated motionMotor fields to print
//*	Jul  2,	2022,	<RNS> Changed ra/dec to motor0/1 and moved addr into motor0/1
//*	Jul  2,	2022,	<RNS> Removed dummy and corrected RC address range
//*	Jul  3,	2022	<RNS> moved memset for motion config to _read_cfg routine
//*	Jul  3,	2022,	<RNS> Added enum of config files
//*	Jul  4,	2022,	<RNS> renamed .position field to .state
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
#include "servo_motion_cfg.h"

//******************************************************************
enum
{
	MC_ADDR,
	BAUD,
	COMM_PORT,
	RA_KP_CON,
	RA_KI_CON,
	RA_KD_CON,
	RA_IL_CON,
	DEC_KP_CON,
	DEC_KI_CON,
	DEC_KD_CON,
	DEC_IL_CON,

	SERVO_CFG_LAST
};	// of enum


//******************************************************************
static TYPE_CFG_ITEM gsMotionConfigArray[] =
{
	{"MC_ADDR:",			MC_ADDR,		false},
	{"BAUD:",				BAUD,			false},
	{"COMM_PORT:",			COMM_PORT,		false},
	{"RA_KP_CON:",			RA_KP_CON,		false},
	{"RA_KI_CON:",			RA_KI_CON,		false},
	{"RA_KD_CON:",			RA_KD_CON,		false},
	{"RA_IL_CON:",			RA_IL_CON,		false},
	{"DEC_KP_CON:",			DEC_KP_CON,		false},
	{"DEC_KI_CON:",			DEC_KI_CON,		false},
	{"DEC_KD_CON:",			DEC_KD_CON,		false},
	{"DEC_IL_CON:",			DEC_IL_CON,		false},
	{"",					-1,				false}
};


//******************************************************************
static int FindKeyWordEnum(const char *keyword)
{
int iii;
int enumValue;

	iii			=	0;
	enumValue	=	-1;
	while ((enumValue < 0) && (gsMotionConfigArray[iii].enumValue >= 0))
	{
		if (strcasecmp(keyword, gsMotionConfigArray[iii].parameter) == 0)
		{
			enumValue	=	gsMotionConfigArray[iii].enumValue;
			//*	verify that the array is in the right order
			if (enumValue != iii)
			{
				fprintf(stderr, "There is an order problem with the motion token array.\r\n");
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
								TYPE_MOTION_CONFIG	*motionConfig,
								char				*cfgErrorString1,
								char				*cfgErrorString2,
								char				*cfgErrorString3)
{
int				tokenEnumValue;
TYPE_MOTION_MOTOR	*ra;
TYPE_MOTION_MOTOR	*dec;
bool				configLineOK;

//	CONSOLE_DEBUG(__FUNCTION__);

	cfgErrorString1[0]	=	0;
	cfgErrorString2[0]	=	0;
	cfgErrorString3[0]	=	0;

	ra	=	&motionConfig->motor0;
	dec	=	&motionConfig->motor1;

	tokenEnumValue	=	FindKeyWordEnum(token);
	configLineOK	=	true;
	switch (tokenEnumValue)
	{
		case MC_ADDR:
			gsMotionConfigArray[MC_ADDR].found	=	true;
			// Convert to in, Roboclaw only supports 8 addr 0x80 to 0x87, so AND mask with 0x87
			motionConfig->motor0.addr			=	atoi(argument) & 0x87;
			motionConfig->motor1.addr			=	motionConfig->motor0.addr;

		//	printf("%-15.15s = %-15d  \n", token, motionConfig->addr);

			if (motionConfig->motor0.addr < 0x80 || motionConfig->motor0.addr > 0x87)
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%s'", token, argument);
				sprintf(cfgErrorString2,	"Usage:  %s  128", token);
				configLineOK	=	false;
				// default to the standard RC address 128, best chance for something to work
				motionConfig->motor0.addr = 128; 
				motionConfig->motor1.addr = 128; 
			}
			break;

		case BAUD:
			gsMotionConfigArray[BAUD].found	=	true;
			motionConfig->baud				=	atoi(argument);
		//	printf("%-15.15s = %-15d  \n", token, motionConfig->baud);

			if ((motionConfig->baud != 9600) && (motionConfig->baud != 19200) && (motionConfig->baud != 38400))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%d'", token, motionConfig->baud);
				sprintf(cfgErrorString2,	"Usage:  %s  9600", token);
				configLineOK	=	false;
			}
			break;

		case COMM_PORT:
			gsMotionConfigArray[COMM_PORT].found	=	true;
			strcpy(motionConfig->port, argument);
		//	printf("%-15.15s = %-15.15s  \n", token, motionConfig->port);
			break;

		case RA_KP_CON:
			gsMotionConfigArray[RA_KP_CON].found	=	true;
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
			gsMotionConfigArray[RA_KI_CON].found	=	true;
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
			gsMotionConfigArray[RA_KD_CON].found	=	true;
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
			gsMotionConfigArray[RA_IL_CON].found	=	true;
			ra->il								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, ra->il);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, ra->il);
				sprintf(cfgErrorString2,	"Usage:  %s  130.0", token);
				configLineOK	=	false;
			}
			break;

		case DEC_KP_CON:
			gsMotionConfigArray[DEC_KP_CON].found	=	true;
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
			gsMotionConfigArray[DEC_KI_CON].found	=	true;
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
			gsMotionConfigArray[DEC_KD_CON].found	=	true;
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
			gsMotionConfigArray[DEC_IL_CON].found	=	true;
			dec->il								=	atof(argument);
		//	printf("%-15.15s = %-15d  \n", token, dec->il);

			if (!isdigit(argument[0]))
			{
				sprintf(cfgErrorString1,	"Invalid %s field '%f'", token, dec->il);
				sprintf(cfgErrorString2,	"Usage:  %s  130.0", token);
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
int Servo_read_motion_cfg(const char *motionCfgFile, TYPE_MOTION_CONFIG *motionConfig)
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
	while (strlen(gsMotionConfigArray[iii].parameter) > 0)
	{
		gsMotionConfigArray[iii++].found	=	false;
	}
	// Zero out the motion config data structure
	memset((void *)motionConfig, 0, sizeof(TYPE_MOTION_CONFIG));

	// If no filename specified, use the default
	if (motionCfgFile == NULL)
	{
		strcpy(filename, kMOTION_CFG_FILE);
	}
	else
	{
		strcpy(filename, motionCfgFile);
	}
	// open the motion configuration file
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
															motionConfig,
															cfgErrorString1,
															cfgErrorString2,
															cfgErrorString3);
					if (configLineOK == false)
					{
						fprintf(stderr, "Error: (init_motion_cfg) on line %d of file '%s'\n", line, filename);
						fprintf(stderr, "       %s\n", cfgErrorString1);
						fprintf(stderr, "       %s\n", cfgErrorString2);
						fprintf(stderr, "       %s\n", cfgErrorString3);
					}
				}
				else
				{
					fprintf(stderr, "Error: (init_motion_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid syntax: field %s is missing argument\n", token);
				}
			}	// of if not a comment line
			line++;		// increment the line counter
		}	// of while
		fclose(inFile);

		// Even out the columns before any missing args error statements
		//printf("\n");
//		Print_motion_configuration();

		iii	=	0;
		while (strlen(gsMotionConfigArray[iii].parameter) > 0)
		{
			if (gsMotionConfigArray[iii].found == false)
			{
				fprintf(stderr, "Error: (motion_read_cfg) Configuation variable:\n");
				fprintf(stderr, "       '%s' was not found or of improper format.\n",
											gsMotionConfigArray[iii].parameter);
				fprintf(stderr, "       from file '%s'\n", filename);
				okFlag	=	false;
			}
			iii++;
		}

		// configuration file had a syntax error
		if (okFlag == false)
		{
			fprintf(stderr, "Error: (motion_read_cfg) Error found in configuration:\n");
			fprintf(stderr, "       from file '%s'\n", filename);
			fflush(stderr);
			retStatus	=	-1;
		}
		else
		{
			// No Errors in configuration file
			retStatus	=	0;
		}
	}
	else		//*	file open
	{
		fprintf(stderr, "Error: could not open cfg file %s\n", filename);
		retStatus	=	-1;
	}	//*	of fopen
	return (retStatus);
}	// of Servo_read_motion_cfg()

//#define _TEST_SERVO_MOTION_CFG_
#ifdef _TEST_SERVO_MOTION_CFG_

// Local global for print* commands
TYPE_MOTION_CONFIG  gsMotionConfig; 


//**************************************************************************************
static void	PrintConfigParam_Dbl(const int cfgEnum, const double value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15.4f  \n", gsMotionConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void	PrintConfigParam_Str(const int cfgEnum, const char *value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %s  \n", gsMotionConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void	PrintConfigParam_Int(const int cfgEnum, const int value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15d  \n", gsMotionConfigArray[cfgEnum].parameter, value);
	}
}

//******************************************************************
void	PrintMotionConfiguration(void)
{
//char	lineBuff[64];

	PrintConfigParam_Int(MC_ADDR,		gsMotionConfig.motor0.addr);
	PrintConfigParam_Str(COMM_PORT,		gsMotionConfig.port);
	PrintConfigParam_Int(BAUD,			gsMotionConfig.baud);

	PrintConfigParam_Dbl(RA_KP_CON,			gsMotionConfig.motor0.kp);
	PrintConfigParam_Dbl(RA_KI_CON,			gsMotionConfig.motor0.ki);
	PrintConfigParam_Dbl(RA_KD_CON,			gsMotionConfig.motor0.kd);
	PrintConfigParam_Dbl(RA_IL_CON,			gsMotionConfig.motor0.il);

	PrintConfigParam_Dbl(DEC_KP_CON,		gsMotionConfig.motor1.kp);
	PrintConfigParam_Dbl(DEC_KI_CON,		gsMotionConfig.motor1.ki);
	PrintConfigParam_Dbl(DEC_KD_CON,		gsMotionConfig.motor1.kd);
	PrintConfigParam_Dbl(DEC_IL_CON,		gsMotionConfig.motor1.il);
}

//********************************************************************************************
//* Dump the axisPtr data structure to standard out */
//********************************************************************************************
void Test_print_motion_motor(TYPE_MOTION_MOTOR *ax)
{
	printf("ax->encoderMaxSpeed = %d\n", ax->encoderMaxSpeed);
	printf("ax->state = %d\n", ax->state);
	printf("ax->maxAcc = %d\n", ax->maxAcc);
	printf("ax->acc = %d\n", ax->acc);
	printf("ax->maxVel = %d\n", ax->maxVel);
	printf("ax->vel = %d\n", ax->vel);
	printf("ax->guideRate = %d\n", ax->guideRate);
	printf("ax->manSlewRate = %d\n", ax->manSlewRate);
	printf("ax->kp = %f\n", ax->kp);
	printf("ax->ki = %f\n", ax->ki);
	printf("ax->kd = %f\n", ax->kd);
	printf("ax->il = %f\n", ax->il);
	printf("ax->status = %x\n", ax->status);
	printf("ax->cmdQueue = %d\n", ax->cmdQueue);
	printf("ax->track = %d\n", ax->trackRate);
	printf("ax->buffered = %d\n", ax->buffered);
}

//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
int main(void)
{
	char configFile[]	=	"servo_motion.cfg";

	printf("file name = %s\n", configFile);

	Servo_read_motion_cfg(configFile, &gsMotionConfig);

	PrintMotionConfiguration();

	return (0);
}
#endif	//	_TEST_SERVO_MOTION_CFG_
