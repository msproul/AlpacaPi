//******************************************************************************
//*	Name:			servo_scope_cfg.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Read/parses the telescope mount config file for attributes
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
#include "servo_scope_cfg.h"

static double RolloverRegion = 0.0;
static double OffTargetTolerance;

//******************************************************************
enum
{
	MC_FREQ = 0,
	MC_ADDR,
	BAUD,
	COMM_PORT,
	MOUNT,
	PARK_SIDE,
	RA_MOTOR_MAX_RPM,
	RA_MOTOR_GEAR,
	RA_MAIN_GEAR,
	RA_ENCODER,
	RA_MAX_VEL,
	RA_MAX_ACC,
	RA_ADJ_VEL,
	RA_SI_CON,
	RA_KP_CON,
	RA_KI_CON,
	RA_KD_CON,
	RA_IL_CON,
	DEC_MOTOR_MAX_RPM,
	DEC_MOTOR_GEAR,
	DEC_MAIN_GEAR,
	DEC_ENCODER,
	DEC_MAX_VEL,
	DEC_MAX_ACC,
	DEC_ADJ_VEL,
	DEC_SI_CON,
	DEC_KP_CON,
	DEC_KI_CON,
	DEC_KD_CON,
	DEC_IL_CON,
	RA_CONFIG,
	DEC_CONFIG,
	RA_GEAR_LASH,
	DEC_GEAR_LASH,
	DEC_PARK,
	RA_SLEW_VEL,
	DEC_SLEW_VEL,
	RA_PARK,
	ROLLOVER_WIN,
	RA_PRECESSION,
	DEC_PRECESSION,
	RA_SENSOR,
	DEC_SENSOR,
	RA_PARK_SENSOR,
	DEC_PARK_SENSOR,
	OFF_TARGET_TOL
}; // of enum

//******************************************************************
typedef struct
{
	char parameter[24];
	short enumValue;
	bool found;
} TYPE_CFG_ITEM;

//} cfgItem, cfgItemPtr;

//******************************************************************
static TYPE_CFG_ITEM gScopeConfigArray[] =
	{
		{"MC_FREQ:", MC_FREQ, false},
		{"MC_ADDR:", MC_ADDR, false},
		{"BAUD:", BAUD, false},
		{"COMM_PORT:", COMM_PORT, false},
		{"MOUNT:", MOUNT, false},
		{"PARK_SIDE:", PARK_SIDE, false},
		{"RA_MOTOR_MAX_RPM:", RA_MOTOR_MAX_RPM, false},
		{"RA_MOTOR_GEAR:", RA_MOTOR_GEAR, false},
		{"RA_MAIN_GEAR:", RA_MAIN_GEAR, false},
		{"RA_ENCODER:", RA_ENCODER, false},
		{"RA_MAX_VEL:", RA_MAX_VEL, false},
		{"RA_MAX_ACC:", RA_MAX_ACC, false},
		{"RA_ADJ_VEL:", RA_ADJ_VEL, false},
		{"RA_SI_CON:", RA_SI_CON, false},
		{"RA_KP_CON:", RA_KP_CON, false},
		{"RA_KI_CON:", RA_KI_CON, false},
		{"RA_KD_CON:", RA_KD_CON, false},
		{"RA_IL_CON:", RA_IL_CON, false},
		{"DEC_MOTOR_MAX_RPM:", DEC_MOTOR_MAX_RPM, false},
		{"DEC_MOTOR_GEAR:", DEC_MOTOR_GEAR, false},
		{"DEC_MAIN_GEAR:", DEC_MAIN_GEAR, false},
		{"DEC_ENCODER:", DEC_ENCODER, false},
		{"DEC_MAX_VEL:", DEC_MAX_VEL, false},
		{"DEC_MAX_ACC:", DEC_MAX_ACC, false},
		{"DEC_ADJ_VEL:", DEC_ADJ_VEL, false},
		{"DEC_SI_CON:", DEC_SI_CON, false},
		{"DEC_KP_CON:", DEC_KP_CON, false},
		{"DEC_KI_CON:", DEC_KI_CON, false},
		{"DEC_KD_CON:", DEC_KD_CON, false},
		{"DEC_IL_CON:", DEC_IL_CON, false},
		{"RA_CONFIG:", RA_CONFIG, false},
		{"DEC_CONFIG:", DEC_CONFIG, false},
		{"RA_GEAR_LASH:", RA_GEAR_LASH, false},
		{"DEC_GEAR_LASH:", DEC_GEAR_LASH, false},
		{"DEC_PARK:", DEC_PARK, false},
		{"RA_SLEW_VEL:", RA_SLEW_VEL, false},
		{"DEC_SLEW_VEL:", DEC_SLEW_VEL, false},
		{"RA_PARK:", RA_PARK, false},
		{"ROLLOVER_WIN:", ROLLOVER_WIN, false},
		{"RA_PRECESSION:", RA_PRECESSION, false},
		{"DEC_PRECESSION:", DEC_PRECESSION, false},
		{"RA_SENSOR:", RA_SENSOR, false},
		{"DEC_SENSOR:", DEC_SENSOR, false},
		{"RA_PARK_SENSOR:", RA_PARK_SENSOR, false},
		{"DEC_PARK_SENSOR:", DEC_PARK_SENSOR, false},
		{"OFF_TARGET_TOL:", OFF_TARGET_TOL, false},
		{"", -1, false}};

//******************************************************************
static int FindKeyWordEnum(const char *keyword)
{
	int iii;
	int enumValue;
	iii = 0;
	enumValue = -1;
	while ((enumValue < 0) && (gScopeConfigArray[iii].enumValue >= 0))
	{
		if (strcasecmp(keyword, gScopeConfigArray[iii].parameter) == 0)
		{
			enumValue = gScopeConfigArray[iii].enumValue;
			//*	verify that the array is in the right order
			if (enumValue != iii)
			{
				printf("There is an order problem with the token array!!!!!!!!!!!!\r\n");
			}
		}
		iii++;
	}
	return (enumValue);
}


//******************************************************************
int Servo_Read_Scope_Cfg(const char *scopeCfgFile, TYPE_SCOPE_CONFIG *scopeConfig)
{
	char filename[kMAX_STR_LEN];
	char inString[kMAX_STR_LEN];
	FILE *inFile;
	uint16_t line = 1;
	bool okFlag = true;
	// int			column			=	0;
	char delimiters[] = " \t\r\n\v\f"; // POSIX whitespace chars
	char *token;
	char *argument;
	char *rest = NULL;
	int tokenEnumValue;
	int iii;
	int retStatus;
	axisPtr ra;
	axisPtr dec;

	ra = &scopeConfig->ra;
	dec = &scopeConfig->dec;

	CONSOLE_DEBUG(__FUNCTION__);

	retStatus = -1;
	//*	first, go through and invalidate all of the parameters
	iii = 0;
	while (strlen(gScopeConfigArray[iii].parameter) > 0)
	{
		gScopeConfigArray[iii].found = false;
		iii++;
	}

	//	printf("\r\n%s ************************************************\r\n", __FUNCTION__);
	// If no filename specified, use the default
	if (scopeCfgFile == NULL)
	{
		strcpy(filename, kSCOPE_CFG_FILE);
	}
	else
	{
		strcpy(filename, scopeCfgFile);
	}
	// open the scope configuration file
	inFile = fopen(filename, "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error: could not open cfg file %s\n", filename);
		return (-1);
	}

	// get all of the lines in the file
	while (fgets(inString, kMAX_STR_LEN, inFile) != NULL)
	{
		// get first token of the line read from the file
		token = strtok_r(inString, delimiters, &rest);

		// If not a blank line and non-comment token found on line 
		if ((token != NULL) && (token[0] != '#'))
		{
			tokenEnumValue = FindKeyWordEnum(token);
			// <RNS> need to check the if the token is valid before printing otherwise error message is wacky
			if (tokenEnumValue == -1)
			{
				// Not a valid token
				fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
				fprintf(stderr, "       Invalid field: %s\n", token);
			}
			else
			{
				// token valid, so print it
				printf("Token:%-20s = %d\t", token, tokenEnumValue);

				// Get corresponding argument for the token
				argument = strtok_r(NULL, delimiters, &rest);
				if (argument != NULL)
				{
					// Token and argument exist, now determine if they are valid
					switch (tokenEnumValue)
					{
					case MC_FREQ:
						gScopeConfigArray[MC_FREQ].found = true;
						scopeConfig->freq = atof(argument);
						// parameter is ok, print it out
						printf("%-15.15s = %-15.4f  ", token, scopeConfig->freq);

						if (scopeConfig->freq == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, scopeConfig->freq);
							fprintf(stderr, "       Must be '1000000.0'\n");
							fprintf(stderr, "       Usage:  %s  1000000.0\n", token);
						}
						break;

					case MC_ADDR:
						gScopeConfigArray[MC_ADDR].found = true;
						scopeConfig->addr = atoi(argument);
						printf("%-15.15s = %-15d  ", token, scopeConfig->addr);

						if (scopeConfig->addr < 0 || scopeConfig->addr > 255)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, scopeConfig->addr);
							fprintf(stderr, "       Usage:  %s  128\n", token);
						}

						break;

					case BAUD:
						gScopeConfigArray[BAUD].found = true;
						scopeConfig->baud = atoi(argument);
						printf("%-15.15s = %-15d  ", token, scopeConfig->baud);

						if ((scopeConfig->baud != 9600) && (scopeConfig->baud != 19200) && (scopeConfig->baud != 38400))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, scopeConfig->baud);
							fprintf(stderr, "       Usage:  %s  9600\n", token);
						}
						break;

					case COMM_PORT:
						gScopeConfigArray[COMM_PORT].found = true;
						strcpy(scopeConfig->port, argument);
						printf("%-15.15s = %-15.15s  ", token, scopeConfig->port);
						break;

					case MOUNT:
						gScopeConfigArray[MOUNT].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);
						if (!strcmp(argument, "FORK"))
						{
							scopeConfig->mount = kFORK;
						}
						else if (!strcmp(argument, "GERMAN"))
						{
							scopeConfig->mount = kGERMAN;

						}
						//				else if (!strcmp(argument, "SPLIT"))
						//				{
						//					*mount	=	kSPLIT;
						//				}
						//				else if (!strcmp(argument, "ALTAZI"))
						//				{
						//					*mount	=	kALTAZI;
						//				}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'FORK' or'GERMAN'\n");
							fprintf(stderr, "       Usage:  %s  FORK\n", token);
						}
						break;

					case PARK_SIDE:
						gScopeConfigArray[PARK_SIDE].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);

						if (!strcmp(argument, "WEST"))
						{
							scopeConfig->side = kWEST;
							scopeConfig->ra.parkInfo = kWEST;
						}
						else if (!strcmp(argument, "EAST"))
						{
							scopeConfig->side = kEAST;
							scopeConfig->ra.parkInfo = kEAST;
						}
						else if (!strcmp(argument, "NONE"))
						{
							scopeConfig->side = kNONE;
							scopeConfig->ra.parkInfo = kNONE;
						}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'EAST', 'WEST', 'NONE'\n");
							fprintf(stderr, "       Usage:  %s  EAST\n", token);
						}
						break;

					case RA_MOTOR_MAX_RPM:
						gScopeConfigArray[RA_MOTOR_MAX_RPM].found = true;
						ra->motorMaxRPM = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->motorMaxRPM);

						if (ra->motorMaxRPM <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->motorMaxRPM);
							fprintf(stderr, "       Usage:  %s  7200.0\n", token);
						}
						break;

					case RA_MOTOR_GEAR:
						gScopeConfigArray[RA_MOTOR_GEAR].found = true;
						ra->motorGear = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->motorGear);

						if (ra->motorGear <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->motorGear);
							fprintf(stderr, "       Usage:  %s  18.0\n", token);
						}
						break;

					case RA_MAIN_GEAR:
						gScopeConfigArray[RA_MAIN_GEAR].found = true;
						ra->mainGear = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->mainGear);

						if (ra->mainGear <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->mainGear);
							fprintf(stderr, "       Usage:  %s  359.0\n", token);
						}
						break;

					case RA_ENCODER:
						gScopeConfigArray[RA_ENCODER].found = true;
						ra->encoder = atof(argument);
						printf("%-15.15s = %-15.4lf  ", token, ra->encoder);

						if (ra->encoder == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->encoder);
							fprintf(stderr, "       Usage:  %s  2000.0\n", token);
						}
						break;

					case RA_MAX_VEL:
						gScopeConfigArray[RA_MAX_VEL].found = true;
						ra->realVel = atof(argument);
						printf("%-15.15s = %-15.4lf  ", token, ra->realVel);

						if (ra->realVel == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->realVel);
							fprintf(stderr, "       Usage:  %s  15000.0\n", token);
						}
						break;

					case RA_MAX_ACC:
						gScopeConfigArray[RA_MAX_ACC].found = true;
						ra->realAcc = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->realAcc);

						if (ra->realAcc == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->realAcc);
							fprintf(stderr, "       Usage:  %s  10.0\n", token);
						}
						break;

					case RA_ADJ_VEL:
						gScopeConfigArray[RA_ADJ_VEL].found = true;
						ra->realAdj = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->realAdj);

						if (ra->realAdj == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->realAdj);
							fprintf(stderr, "       Usage:  %s  900.0\n", token);
						}
						break;

					case RA_SI_CON:
						gScopeConfigArray[RA_SI_CON].found = true;
						ra->si = (uint8_t)atoi(argument);
						printf("%-15.15s = %-15d  ", token, ra->si);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, (int)ra->si);
							fprintf(stderr, "       Usage:  %s  0\n", token);
						}
						break;

					case RA_KP_CON:
						gScopeConfigArray[RA_KP_CON].found = true;
						ra->kp = atoi(argument);
						printf("%-15.15s = %-15d  ", token, ra->kp);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, ra->kp);
							fprintf(stderr, "       Usage:  %s  60\n", token);
						}
						break;

					case RA_KI_CON:
						gScopeConfigArray[RA_KI_CON].found = true;
						ra->ki = atoi(argument);
						printf("%-15.15s = %-15d  ", token, ra->ki);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, ra->ki);
							fprintf(stderr, "       Usage:  %s  30\n", token);
						}
						break;

					case RA_KD_CON:
						gScopeConfigArray[RA_KD_CON].found = true;
						ra->kd = atoi(argument);
						printf("%-15.15s = %-15d  ", token, ra->kd);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, ra->kd);
							fprintf(stderr, "       Usage:  %s  300\n", token);
						}
						break;

					case RA_IL_CON:
						gScopeConfigArray[RA_IL_CON].found = true;
						ra->il = atoi(argument);
						printf("%-15.15s = %-15d  ", token, ra->il);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, ra->il);
							fprintf(stderr, "       Usage:  %s  130\n", token);
						}
						break;

					case DEC_MOTOR_MAX_RPM:
						gScopeConfigArray[DEC_MOTOR_MAX_RPM].found = true;
						dec->motorMaxRPM = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->motorMaxRPM);

						if (ra->motorMaxRPM <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->motorMaxRPM);
							fprintf(stderr, "       Usage:  %s  7200.0\n", token);
						}
						break;

					case DEC_MOTOR_GEAR:
						gScopeConfigArray[DEC_MOTOR_GEAR].found = true;
						dec->motorGear = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->motorGear);

						if (dec->motorGear == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->motorGear);
							fprintf(stderr, "       Usage:  %s  18.0\n", token);
						}
						break;

					case DEC_MAIN_GEAR:
						gScopeConfigArray[DEC_MAIN_GEAR].found = true;
						dec->mainGear = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->mainGear);

						if (dec->mainGear == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->mainGear);
							fprintf(stderr, "       Usage:  %s  359.0\n", token);
						}
						break;

					case DEC_ENCODER:
						gScopeConfigArray[DEC_ENCODER].found = true;
						dec->encoder = atof(argument);
						printf("%-15.15s = %-15.4lf  ", token, dec->encoder);

						if (dec->encoder == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->encoder);
							fprintf(stderr, "       Usage:  %s  2000.0\n", token);
						}
						break;

					case DEC_MAX_VEL:
						gScopeConfigArray[DEC_MAX_VEL].found = true;
						dec->realVel = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->realVel);

						if (dec->realVel == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->realVel);
							fprintf(stderr, "       Usage:  %s  15000.0\n", token);
						}
						break;

					case DEC_MAX_ACC:
						gScopeConfigArray[DEC_MAX_ACC].found = true;
						dec->realAcc = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->realAcc);

						if (dec->realAcc == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->realAcc);
							fprintf(stderr, "       Usage:  %s  10.0\n", token);
						}
						break;

					case DEC_ADJ_VEL:
						gScopeConfigArray[DEC_ADJ_VEL].found = true;
						dec->realAdj = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->realAdj);

						if (dec->realAdj == 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->realAdj);
							fprintf(stderr, "       Usage:  %s  900.0\n", token);
						}
						break;

					case DEC_SI_CON:
						gScopeConfigArray[DEC_SI_CON].found = true;
						dec->si = (uint8_t)atoi(argument);
						printf("%-15.15s = %-15d  ", token, dec->si);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, dec->si);
							fprintf(stderr, "       Usage:  %s  0\n", token);
						}
						break;

					case DEC_KP_CON:
						gScopeConfigArray[DEC_KP_CON].found = true;
						dec->kp = atoi(argument);
						printf("%-15.15s = %-15d  ", token, dec->kp);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, dec->kp);
							fprintf(stderr, "       Usage:  %s  60\n", token);
						}
						break;

					case DEC_KI_CON:
						gScopeConfigArray[DEC_KI_CON].found = true;
						dec->ki = atoi(argument);
						printf("%-15.15s = %-15d  ", token, dec->ki);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, dec->ki);
							fprintf(stderr, "       Usage:  %s  60\n", token);
						}
						break;

					case DEC_KD_CON:
						gScopeConfigArray[DEC_KD_CON].found = true;
						dec->kd = atoi(argument);

						printf("%-15.15s = %-15d  ", token, dec->kd);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, dec->kd);
							fprintf(stderr, "       Usage:  %s  300\n", token);
						}
						break;

					case DEC_IL_CON:
						gScopeConfigArray[DEC_IL_CON].found = true;
						dec->il = atoi(argument);
						printf("%-15.15s = %-15d  ", token, dec->il);

						if (!isdigit(argument[0]))
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%d'\n", token, dec->il);
							fprintf(stderr, "       Usage:  %s  130\n", token);
						}
						break;

					case RA_CONFIG:
						gScopeConfigArray[RA_CONFIG].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);

						if (!strcmp(argument, "FORWARD"))
						{
							ra->config = kFORWARD;
						}
						else if (!strcmp(argument, "REVERSE"))
						{
							ra->config = kREVERSE;
						}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'FORWARD' or 'REVERSE'\n");
							fprintf(stderr, "       Usage:  %s  FORWARD\n", token);
						}
						break;

					case DEC_CONFIG:
						gScopeConfigArray[DEC_CONFIG].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);

						if (!strcmp(argument, "FORWARD"))
						{
							dec->config = kFORWARD;
							dec->parkInfo = kFORWARD;
						}
						else if (!strcmp(argument, "REVERSE"))
						{
							dec->config = kREVERSE;
							dec->parkInfo = kREVERSE;
						}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'FORWARD' or 'REVERSE'\n");
							fprintf(stderr, "       Usage:  %s  FORWARD\n", token);
						}
						//	printf("dec->parkInfo = 0x%X", dec->parkInfo);
						break;

					case RA_GEAR_LASH:
						gScopeConfigArray[RA_GEAR_LASH].found = true;
						ra->gearLash = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->gearLash);

						if (ra->gearLash < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->gearLash);
							fprintf(stderr, "       Usage:  %s  1500.0\n", token);
						}
						break;

					case DEC_GEAR_LASH:
						gScopeConfigArray[DEC_GEAR_LASH].found = true;
						dec->gearLash = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->gearLash);

						if (dec->gearLash < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->gearLash);
							fprintf(stderr, "       Usage:  %s  1500.0\n", token);
						}
						break;

					case DEC_PARK:
						gScopeConfigArray[DEC_PARK].found = true;
						dec->park = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->park);

						if (dec->park < -91.0 || dec->park > 91.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->park);
							fprintf(stderr, "       Usage:  %s  0.0\n", token);
						}
						break;

					case RA_SLEW_VEL:
						gScopeConfigArray[RA_SLEW_VEL].found = true;
						ra->realSlew = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->realSlew);

						if (ra->realSlew <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->realSlew);
							fprintf(stderr, "       Usage:  %s  1000.0\n", token);
						}
						break;

					case DEC_SLEW_VEL:
						gScopeConfigArray[DEC_SLEW_VEL].found = true;
						dec->realSlew = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->realSlew);

						if (dec->realSlew <= 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->realSlew);
							fprintf(stderr, "       Usage:  %s  1000.0\n", token);
						}
						break;

					case RA_PARK:
						gScopeConfigArray[RA_PARK].found = true;
						ra->park = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->park);

						if (ra->park < -7.0 || ra->park > 7.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->park);
							fprintf(stderr, "       Usage:  %s  0.0\n", token);
						}
						break;

					case ROLLOVER_WIN:
						gScopeConfigArray[ROLLOVER_WIN].found = true;
						RolloverRegion = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, RolloverRegion);

						if (RolloverRegion < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, RolloverRegion);
							fprintf(stderr, "       Usage:  %s  1.0\n", token);
						}
						break;

					case RA_PRECESSION:
						gScopeConfigArray[RA_PRECESSION].found = true;
						ra->prec = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->prec);

						if (ra->prec < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->prec);
							fprintf(stderr, "       Usage:  %s  0.32\n", token);
						}
						break;

					case DEC_PRECESSION:
						gScopeConfigArray[DEC_PRECESSION].found = true;
						dec->prec = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->prec);

						if (dec->prec < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->prec);
							fprintf(stderr, "       Usage:  %s  0.60\n", token);
						}
						break;

					case RA_SENSOR:
						gScopeConfigArray[RA_SENSOR].found = true;
						ra->sync = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, ra->sync);

						if (ra->sync < -7.0 || ra->sync > 7.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, ra->sync);
							fprintf(stderr, "       Usage:  %s  0.0\n", token);
						}
						break;

					case DEC_SENSOR:
						gScopeConfigArray[DEC_SENSOR].found = true;
						dec->sync = Time_ascii_maybe_HMS_tof(argument);
						printf("%-15.15s = %-15.4f  ", token, dec->sync);

						if (dec->sync < -91.0 || dec->sync > 91.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, dec->sync);
							fprintf(stderr, "       Usage:  %s  0.0\n", token);
						}
						break;

					case RA_PARK_SENSOR:
						gScopeConfigArray[RA_PARK_SENSOR].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);

						if (strcmp(argument, "ON") == 0)
						{
							ra->syncValue = true;
						}
						else if (strcmp(argument, "OFF") == 0)
						{
							ra->syncValue = false;
						}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'ON' or 'OFF'\n");
							fprintf(stderr, "       Usage:  %s  ON\n", token);
						}
						break;

					case DEC_PARK_SENSOR:
						gScopeConfigArray[DEC_PARK_SENSOR].found = true;
						printf("%-15.15s = %-15.15s  ", token, argument);
						if (strcmp(argument, "ON") == 0)
						{
							dec->syncValue = true;
						}
						else if (strcmp(argument, "OFF") == 0)
						{
							dec->syncValue = false;
						}
						else
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%s'\n", token, argument);
							fprintf(stderr, "       Must be 'ON' or 'OFF'\n");
							fprintf(stderr, "       Usage:  %s  ON\n", token);
						}
						break;

					case OFF_TARGET_TOL:
						gScopeConfigArray[OFF_TARGET_TOL].found = true;
						OffTargetTolerance = atof(argument);
						printf("%-15.15s = %-15.4f  ", token, OffTargetTolerance);
						if (OffTargetTolerance < 0.0)
						{
							fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
							fprintf(stderr, "       Invalid %s field '%f'\n", token, OffTargetTolerance);
							fprintf(stderr, "       Usage:  %s  1.0\n", token);
						}
						break;

					default:
						fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
						fprintf(stderr, "       Unrecognized %s field\n", token);
						break;
					}
				}
				else
				{
					fprintf(stderr, "Error: (init_scope_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid syntax: field %s is missing argument\n", token);
				}
			} // of if-else token not valid
			printf("\n");
		} // of if not a comment line

		// increment the line counter
		line++;

	} // of while

	fclose(inFile);

	// Even out the columns before any missing args error statements
	printf("\n");

	iii = 0;
	while (strlen(gScopeConfigArray[iii].parameter) > 0)
	{
		if (gScopeConfigArray[iii].found == false)
		{
			fprintf(stderr, "Error: (validate_scope_cfg) Configuation variable:\n");
			fprintf(stderr, "       '%s' was not found or of improper format.\n",
					gScopeConfigArray[iii].parameter);
			fprintf(stderr, "       from file '%s'\n", filename);
			okFlag = false;
		}
		iii++;
	}

	// configuration file had a syntax error
	if (okFlag == false)
	{
		fprintf(stderr, "Error: (validate_scope_cfg) Error found in configuration:\n");
		fprintf(stderr, "       from file '%s'\n", filename);
		fflush(stderr);
		retStatus = -1;
	}
	else
	{
		// No Errors in configuration file

		// If mount is FORK, then set the side to EAST due to ASCOM default definitions
		if (scopeConfig->mount == kFORK)
		{
			scopeConfig->side = kEAST;
		}

		retStatus = 0;
	}

	return (retStatus);
} // Servo_Read_Scope_Cfg

//#define _TEST_SERVO_SCOPE_CFG_
#ifdef _TEST_SERVO_SCOPE_CFG_
//********************************************************************************************
//* Dump the axisPtr data structure to standard out */
//********************************************************************************************
void Test_print_axis(axisPtr ax)
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

//********************************************************************************************
int main(int argc, char **argv)
{
	TYPE_SCOPE_CONFIG scopeConfig;
	char configFile[] = "servo_scope.cfg";

	printf("file name = %s\n", configFile);

	Servo_Read_Scope_Cfg(configFile, &scopeConfig);

	return (0);
}
#endif //	_TEST_SERVO_SCOPE_CFG_
