//******************************************************************************
//*	Name:			servo_observ_cfg.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Read/parses the servo observatory config file for attributes
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
//*	Jun 28,	2022	<RNS> Initial version of this file
//*	Jun 28,	2022	<RNS> Fixed a bug parsing SITE file from cfg file
//*	Jun 30,	2022	<RNS> renamed local globals prefix to 'gs'
//*	Jul  3,	2022	<RNS> renamed default file to servo_observatory.cfg
//*	Jul  3,	2022	<RNS> moved memset for observ config to _read_cfg 
//*	Jul  3,	2022,	<RNS> Added enum of config files
//*	Jul  3,	2022,	<RNS> removed config parameter from _read_ routinte
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
#include "servo_observ_cfg.h"


// Local global for sharing data
static TYPE_OBSERV_CFG gsServoObservCfg;

//*****************************************************************************
// list of defined tokens in config file and array to keep track of discovery
enum
{
	EPOCH	=	0,
	EPOCH_JD,
	LATITUDE,
	LONGITUDE,
	ELEVATION,
	TEMPERATURE,
	PRESSURE,
	SITE,

	SERVO_CFG_LAST
};	// of enum

//******************************************************************
static TYPE_CFG_ITEM gsObservConfigArray[] =
	{
		{"EPOCH:", EPOCH, false},
		{"EPOCH_JD:", EPOCH_JD, false},
		{"LATITUDE:", LATITUDE, false},
		{"LONGITUDE:", LONGITUDE, false},
		{"ELEVATION:", ELEVATION, false},
		{"TEMPERATURE:", TEMPERATURE, false},
		{"PRESSURE:", PRESSURE, false},
		{"SITE:", SITE, false},
		{"", -1, false}};

//*****************************************************************************
// Get/Set the latitude field from the local global location file struct
//*****************************************************************************
double Servo_get_lat(void)
{
	return(gsServoObservCfg.lat);
}
//*****************************************************************************
void Servo_set_lat(double newLatValue)
{
	gsServoObservCfg.lat	=	newLatValue;
}

//*****************************************************************************
// Get/Set the longitude field from the local global location file struct
//*****************************************************************************
double Servo_get_lon(void)
{
	return(gsServoObservCfg.lon);
}
//*****************************************************************************
void Servo_set_lon(double newLonValue)
{
	gsServoObservCfg.lon	=	newLonValue;
}

//*****************************************************************************
// Get/Set the elevation field from the local global location file struct
//*****************************************************************************
double Servo_get_elev(void)
{
	return(gsServoObservCfg.elev);
}
//*****************************************************************************
void Servo_set_elev(double newElevValue)
{
	gsServoObservCfg.elev	=	newElevValue;
}

//*****************************************************************************
// Get/Set the temperature field from the local global location file struct
//*****************************************************************************
double Servo_get_temperature(void)
{
	return(gsServoObservCfg.temp);
}
//*****************************************************************************
double Servo_get_pressure(void)
{
	return(gsServoObservCfg.press);
}

//*****************************************************************************
// Get the site name field from the local global location file struct
//*****************************************************************************
const char *Servo_get_site(void)
{
	return(gsServoObservCfg.site);
}

//******************************************************************
static int FindKeyWordEnum(const char *keyword)
{
	int iii;
	int enumValue;

	iii = 0;
	enumValue = -1;
	while ((enumValue < 0) && (gsObservConfigArray[iii].enumValue >= 0))
	{
		if (strcasecmp(keyword, gsObservConfigArray[iii].parameter) == 0)
		{
			enumValue = gsObservConfigArray[iii].enumValue;
			//*	verify that the array is in the right order
			if (enumValue != iii)
			{
				fprintf(stderr, "There is an order problem with the token array.\r\n");
			}
		}
		iii++;
	}
	return (enumValue);
}

//******************************************************************************
//*	return error strings so they can be logged or displayed elsewhere
//******************************************************************************
static bool ProcessConfigToken(char *token,
							   char *argument,
							   TYPE_OBSERV_CFG *observConfig,
							   char *cfgErrorString1,
							   char *cfgErrorString2,
							   char *cfgErrorString3)
{
	int tokenEnumValue;
	bool configLineOK;

	//	CONSOLE_DEBUG(__FUNCTION__);

	cfgErrorString1[0] = 0;
	cfgErrorString2[0] = 0;
	cfgErrorString3[0] = 0;

	tokenEnumValue = FindKeyWordEnum(token);
	configLineOK = true;
	switch (tokenEnumValue)
	{
	case EPOCH:
		gsObservConfigArray[EPOCH].found = true;
		observConfig->baseEpoch = atof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->baseEpoch);

		if (observConfig->baseEpoch < 1999.99 || observConfig->baseEpoch > 2050.01)
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->baseEpoch);
			sprintf(cfgErrorString2, "Must be between 2000.0 and 2050.0");
			configLineOK = false;
		}
		break;

	case EPOCH_JD:
		gsObservConfigArray[EPOCH_JD].found = true;
		observConfig->baseJd = atof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->baseJd);

		if ((observConfig->baseJd < 2451544.9) || (observConfig->baseJd > 2469808.1))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->baseJd);
			sprintf(cfgErrorString2, "Must be between 2451545.0 and 2469808.0");
			configLineOK = false;
		}
		break;

	case LATITUDE:
		gsObservConfigArray[LATITUDE].found = true;
		observConfig->lat = Time_ascii_maybe_HMS_tof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->lat);

		if ((observConfig->lat < -90.1) || (observConfig->lat > 90.1))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->lat);
			sprintf(cfgErrorString2, "Must be between -90.0 and +90.0");
			configLineOK = false;
		}
		break;

	case LONGITUDE:
		gsObservConfigArray[LONGITUDE].found = true;
		observConfig->lon = Time_ascii_maybe_HMS_tof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->lon);

		if ((observConfig->lon < -180.1) || (observConfig->lon > 180.1))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->lon);
			sprintf(cfgErrorString2, "Must be between -180.0 and +180.0");
			configLineOK = false;
		}
		break;

	case ELEVATION:
		gsObservConfigArray[ELEVATION].found = true;
		observConfig->elev = atof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->elev);

		if ((observConfig->elev < -10.0) || (observConfig->elev > 15000.1))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->elev);
			sprintf(cfgErrorString2, "Must be between -10.0 and 15000.0");
			configLineOK = false;
		}
		break;

	case TEMPERATURE:
		gsObservConfigArray[TEMPERATURE].found = true;
		observConfig->temp = atof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->temp);

		if ((observConfig->temp < -80.0) || (observConfig->temp > 140.0))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->temp);
			sprintf(cfgErrorString2, "Must be between -80.0 and 140.0");
			configLineOK = false;
		}
		break;

	case PRESSURE:
		gsObservConfigArray[PRESSURE].found = true;
		observConfig->press = atof(argument);
		// parameter is ok, print it out
		//	printf("%-15.15s = %-15.4f\n", token, observConfig->press);

		if ((observConfig->press < -10.0) || (observConfig->press > 15000.1))
		{
			sprintf(cfgErrorString1, "Invalid %s field '%f'", token, observConfig->press);
			sprintf(cfgErrorString2, "Must be between -10.0 and 15000.0");
			configLineOK = false;
		}
		break;

	case SITE:
		gsObservConfigArray[SITE].found = true;
		strcpy(observConfig->site, argument);
		//	printf("%-15.15s = %-15.15s  \n", token, observConfig->site);
		break;

	default:
		sprintf(cfgErrorString1, "Unrecognized %s field", token);
		configLineOK = false;
		break;
	}
	return (configLineOK);
}

//******************************************************************
int Servo_read_observ_cfg(const char *observCfgFile)
{
	char filename[kMAX_STR_LEN];
	char inString[kMAX_STR_LEN];
	FILE *inFile;
	uint16_t line = 1;
	bool okFlag = true;
	char delimiters[] = " \t\r\n\v\f"; // POSIX whitespace chars
	char *token;
	char *argument;
	char *rest = NULL;
	int iii;
	int retStatus;
	bool configLineOK;
	char cfgErrorString1[80];
	char cfgErrorString2[80];
	char cfgErrorString3[80];
	TYPE_OBSERV_CFG *observConfig;

	CONSOLE_DEBUG(__FUNCTION__);

	// Initialize local observConfig ptr to static global
	observConfig = &gsServoObservCfg;

	retStatus = -1;
	//*	first, go through and invalidate all of the parameters
	iii = 0;
	while (strlen(gsObservConfigArray[iii].parameter) > 0)
	{
		gsObservConfigArray[iii++].found = false;
	}
	// Zero out the supplied observ config data structure
	memset((void *)observConfig, 0, sizeof(TYPE_OBSERV_CFG));

	// If no filename specified, use the default
	if (observCfgFile == NULL)
	{
		strcpy(filename, kOBSERV_CFG_FILE);
	}
	else
	{
		strcpy(filename, observCfgFile);
	}
	// open the observ configuration file
	inFile = fopen(filename, "r");
	if (inFile != NULL)
	{
		// get all of the lines in the file
		while (fgets(inString, kMAX_STR_LEN, inFile) != NULL)
		{
			// get first token of the line read from the file
			token = strtok_r(inString, delimiters, &rest);

			// If not a blank line and non-comment token found on line
			if ((token != NULL) && (token[0] != '#'))
			{
				// Get corresponding argument for the token
				argument = strtok_r(NULL, delimiters, &rest);
				if (argument != NULL)
				{
					// Token and argument exist, now determine if they are valid
					// special case for SITE since it can be multi-token
					if (FindKeyWordEnum(token) == SITE)
					{
						// concatonate the rest of the line using filename string as buf
						strcpy(filename, argument);
						argument = strtok_r(NULL, delimiters, &rest);
						while (argument != NULL)
						{
							// put a space betten the return tokens
							strcat(filename, " ");
							strcat(filename, argument);
							argument = strtok_r(NULL, delimiters, &rest);
						}
						argument = filename;
					}

					configLineOK = ProcessConfigToken(token,
													  argument,
													  observConfig,
													  cfgErrorString1,
													  cfgErrorString2,
													  cfgErrorString3);
					if (configLineOK == false)
					{
						fprintf(stderr, "Error: (observ_read_cfg) on line %d of file '%s'\n", line, filename);
						fprintf(stderr, "       %s\n", cfgErrorString1);
						fprintf(stderr, "       %s\n", cfgErrorString2);
						fprintf(stderr, "       %s\n", cfgErrorString3);
					}
				}
				else
				{
					fprintf(stderr, "Error: (observ_read_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid syntax: field %s is missing argument\n", token);
				}
			}		// of if not a comment line
			line++; // increment the line counter
		}			// of while
		fclose(inFile);

		// Even out the columns before any missing args error statements
		// printf("\n");
		//		Print_observ_configuration();

		iii = 0;
		while (strlen(gsObservConfigArray[iii].parameter) > 0)
		{
			if (gsObservConfigArray[iii].found == false)
			{
				fprintf(stderr, "Error: (observ_read_cfg) Configuation variable:\n");
				fprintf(stderr, "       '%s' was not found or of improper format.\n",
						gsObservConfigArray[iii].parameter);
				fprintf(stderr, "       from file '%s'\n", filename);
				okFlag = false;
			}
			iii++;
		}

		// configuration file had a syntax error
		if (okFlag == false)
		{
			fprintf(stderr, "Error: (observ_read_cfg) Error found in configuration:\n");
			fprintf(stderr, "       from file '%s'\n", filename);
			fflush(stderr);
			retStatus = -1;
		}
		else
		{
			// No Errors in configuration file
			retStatus = 0;
		}
	}
	else //*	file open
	{
		fprintf(stderr, "Error: could not open cfg file %s\n", filename);
		retStatus = -1;
	} //*	of fopen
	return (retStatus);
} // of Servo_read_observ_cfg()

#ifdef _TEST_SERVO_OBSERV_CFG_
//**************************************************************************************
static void PrintConfigParam_Dbl(const int cfgEnum, const double value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15.4f  \n", gsObservConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void PrintConfigParam_Str(const int cfgEnum, const char *value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %s  \n", gsObservConfigArray[cfgEnum].parameter, value);
	}
}

//**************************************************************************************
static void PrintConfigParam_Int(const int cfgEnum, const int value)
{
	if ((cfgEnum >= 0))
	{
		printf("%-15.15s = %-15d  \n", gsObservConfigArray[cfgEnum].parameter, value);
	}
}

//******************************************************************
void Print_observ_configuration(void)
{
	// char	lineBuff[64];

	PrintConfigParam_Dbl(EPOCH, gsServoObservCfg.baseEpoch);
	PrintConfigParam_Dbl(EPOCH_JD, gsServoObservCfg.baseJd);
	PrintConfigParam_Dbl(LATITUDE, gsServoObservCfg.lat);
	PrintConfigParam_Dbl(LONGITUDE, gsServoObservCfg.lon);
	PrintConfigParam_Dbl(ELEVATION, gsServoObservCfg.elev);
	PrintConfigParam_Dbl(TEMPERATURE, gsServoObservCfg.temp);
	PrintConfigParam_Dbl(PRESSURE, gsServoObservCfg.press);
	PrintConfigParam_Str(SITE, gsServoObservCfg.site);
}

//********************************************************************************************
//* Dump the axisPtr data structure to standard out */
//********************************************************************************************
void Test_print_axis(TYPE_OBSERV_CFG *observ)
{
	printf("observ->baseEpoch = %f\n", observ->baseEpoch);
	printf("observ->baseJd = %f\n", observ->baseJd);
	printf("observ->lat = %f\n", observ->lat);
	printf("observ->lon = %f\n", observ->lon);
	printf("observ->elev = %f\n", observ->elev);
	printf("observ->temp = %f\n", observ->temp);
	printf("observ->press = %f\n", observ->press);
	printf("observ->site = %s\n", observ->site);
}

//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
int main(void)
{
	char configFile[] = "servo_observatory.cfg";

	printf("file name = %s\n", configFile);

	Servo_read_observ_cfg(configFile, &gsServoObservCfg);

	Print_observ_configuration();

	return (0);
}
#endif //	_TEST_SERVO_OBSERV_CFG_
