//******************************************************************************
//*	Name:			servo_time.c
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Servo support file for time, coordinates and location cfg file
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
//*	Apr 20,	2022	<RNS> Created servo_time.c from a port of ephemeris functions
//*	Apr 21,	2022	<RNS> Pulled needed function for UTC systime,jd, sid, lst
//*	Apr 22,	2022	<RNS> Added alt/azi functions and field rotation
//*	Apr 25,	2022	<RNS> Added improved refraction based on Bennett & NASA vector
//*	Apr 25,	2022	<RNS> Added _TEST_ support for unit test build
//*	May  6,	2022	<RNS> Added Time_str_to_upper()
//*	May  8,	2022	<RNS> Fixed a warning do to long double cast
//*****************************************************************************
// Notes: For RoboClaw M1 *MUST BE* connected to RA or Azimuth axis, M2 to Dec or Altitude
//*****************************************************************************
#include	<sys/time.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>
#include	<stdbool.h>
#include	<ctype.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"servo_std_defs.h"
#include	"servo_time.h"


TYPE_LOCAL_CFG	gServoLocalCfg;

////*****************************************************************************
//void Time_deci_days_to_hours(double *day)
//{
//	*day	=	(*day * 24.0);
//}

//*****************************************************************************
void Time_deci_hours_to_deg(double *hours)
{
	*hours	=	(*hours * 15.0);
}

//*****************************************************************************
void Time_deci_deg_to_hours(double *deg)
{
	*deg	=	(*deg / 15.0);
}

//*****************************************************************************
void Time_str_to_upper(char *in)
{
int16_t	index;

	if (in != NULL)
	{
		for (index = 0; in[index] != '\0'; index++)
		{
			in[index]	=	toupper(in[index]);
		}
	}
}	// of str_to_upper()

//*****************************************************************************
// Returns Unix system time in  decimal seconds.microseconds from Epoch 1970.0
// Note: 946713600 Unix system time for J2000.0
//*****************************************************************************
long double Time_get_systime(void)
{
struct timeval current;

	gettimeofday(&current, NULL);

	return (current.tv_sec + (current.tv_usec / 1000000.0));
} // of Time_get_systime

//*****************************************************************************
// Divide by the number of seconds in a day and add JD offset for epoch 1970.0
//*****************************************************************************
long double Time_systime_to_jd(void)
{
	// Divide by the number of seconds in a day and add JD offset for epoch 1970.0
	return ((Time_get_systime() / 86400.0) + 2440587.5);

} // of Time_systime_to_jd

//*****************************************************************************
// Input a julian date and returns the cooresponding sidereal time in decimal hours
//*****************************************************************************
long double Time_jd_to_sid(long double jd)
{
long double	jCent;
long double	sid;

	// Convert the jd to julian centuries
	jCent	=	(jd - 2415020.0) / 36525.0;
	sid		=	280.46061837 + 360.98564736629 * (jd - 2451545.0);
	sid		+=	(0.000387933 * jCent * jCent) - (jCent * jCent * jCent / 38710000.0);

	// convert to deci hours
	sid *= 24.0;

	// make sure sid is postive and < 24.0, interate if needed
	while (sid < 0.0)
	{
		sid += 24.0;
	}
	while (sid >= 24.0)
	{
		sid -= 24.0;
	}

	return (sid);
}

//*****************************************************************************
// Time_sid_to_lst takes in the Grenich sidereal time in decimal
//   hours and longitude in decimal degrees and returns the result
//   (local sidereal time) in decimal hours normalized 0 - 23.99
//*****************************************************************************
long double Time_sid_to_lst(long double sid, double lon)
{
	sid	-=	(lon / 15.0);
	if (sid < 0.0)
	{
		sid	+=	24.0;
	}
	while (sid >= 24.0)
	{
		sid	-=	24.0;
	}
	return(sid);
}

//*****************************************************************************
// Time_check_hms - Checks the range of a HMS variable and
//     and truncates it to XXX.595999 wrapping up.
// Note: Truncation will occur for all min and sec values >= 60.0
//*****************************************************************************
void Time_check_hms(double *hms)
{
double		fractSec;
int32_t		hours;
int32_t		minutes;
int32_t		seconds;
double		sign;
double		value;

	CONSOLE_DEBUG(__FUNCTION__);

	// Save the sign since the algorithm only works on pos numbers
	if (*hms < 0.0)
	{
		value	=	-(*hms);
		sign	=	-1;
	}
	else
	{
		value	=	*hms;
		sign	=	1;
	}
	// decode the hms fields to individual
//	#warning "<MLS> I dont think this the right way to do conversion"
	hours		=	(int32_t)value;
	minutes		=	(int32_t)((value - (double)hours) * 100.0);
	seconds		=	(int32_t)((value - (double)hours) * 10000.0 - (double)minutes * 100.0);
	fractSec	=	((value - (double)hours) * 10000.0 - (double)minutes * 100.0 - (double)seconds);

//	CONSOLE_DEBUG_W_NUM("hours  \t=",	hours);
//	CONSOLE_DEBUG_W_NUM("minutes\t=",	minutes);
//	CONSOLE_DEBUG_W_NUM("seconds\t=",	seconds);
//
//	if (1)
//	{
//		hours		=	value;
//		minutes		=	((value - hours) * 100.0);
//		seconds		=	((value - hours) * 10000.0 - minutes * 100.0);
//
//		CONSOLE_DEBUG("----------------");
//		CONSOLE_DEBUG_W_NUM("hours  \t=",	hours);
//		CONSOLE_DEBUG_W_NUM("minutes\t=",	minutes);
//		CONSOLE_DEBUG_W_NUM("seconds\t=",	seconds);
//		CONSOLE_ABORT(__FUNCTION__);
//	}


	// make sure the range is ok for sec otherwise truncate it 0.0
	if (seconds > 59)
	{
		seconds		=	0;
		fractSec	=	0.0;
		minutes++;
	}
	// make sure the range is ok for min otherwise truncate it to 0
	if (minutes > 59)
	{
		minutes	=	0;
		hours++;
	}
	fractSec	+=	((double)minutes * 100.0) + (double)seconds;
	value		=	(double)hours + (fractSec / 10000.0);
	*hms		=	sign * value;
}

//*****************************************************************************
// deci_hours_to_hms - reads in a double of the form HH.hh... and
//   converts it to HH.MMSSss. Also converts DDD.ddd to DDD.MMSSss
// Note: Truncation will occur for all min and sec values >= 60.0
//*****************************************************************************
void Time_deci_hours_to_hms(double *value)
{
double fract;
double fractSec;
int32_t hours;
int32_t minutes;
int32_t seconds;

	hours		=	(int32_t)*value;
	fract		=	((*value - (double)hours) * 3600.0);
	minutes		=	((int32_t)fract) / 60;
	seconds		=	((int32_t)fract) - minutes * 60;
	fractSec	=	fract - (double)((int32_t)fract);

	// make sure the range is ok for sec otherwise truncate it 0.0
	if (seconds > 59)
	{
		seconds		=	0;
		fractSec	=	0.0;
		minutes++;
	}

	// make sure the range is ok for min otherwise truncate it to 0
	if (minutes > 59)
	{

		minutes	=	0;
		hours++;
	}

	fractSec	+=	((double)minutes * 100.0) + (double)seconds;
	*value		=	(double)hours + (fractSec / 10000.0);
}

//*****************************************************************************
// hms_hours_to_deci - reads in a double of the form HH.MMSS...
//   and converts it to HH.hhh...  Also works for translating
//   DDD.MMSSss to DDD.ddd...
// Note: Truncation will occur for all min and sec values >= 60.0
//*****************************************************************************
void Time_hms_hours_to_deci(double *value)
{
double fractSec;
int32_t hours;
int32_t minutes;
int32_t seconds;

	hours		=	(int32_t)*value;
	minutes		=	(int32_t)((*value - (double)hours) * 100.0);
	seconds		=	(int32_t)((*value - (double)hours) * 10000.0 - (double)minutes * 100.0);
	fractSec	=	((*value - (double)hours) * 10000.0 - (double)minutes * 100.0 - (double)seconds);

	// make sure the range is ok for sec otherwise truncate it 0.0
	if (seconds > 59)
	{
		seconds		=	0;
		fractSec	=	0.0;
		minutes++;
	}

	// make sure the range is ok for min otherwise truncate it to 0
	if (minutes > 59)
	{

		minutes	=	0;
		hours++;
	}

	fractSec	+=	((double)minutes * 60.0) + (double)seconds;
	*value		=	(double)hours + (fractSec / 3600.0);
}

//*****************************************************************************
// Takes in ra, dec, let in deci degs and sidereal in deci hours and returns alt/azi in radians
// Azi is done returned in the form of North = 0 and NESW sweep (E = 90, S = 180, W = 270)
//*****************************************************************************
void Time_ra_dec_to_alt_azi(double ra, double dec, long double sid, double lat, double *alt, double *azi)
{
double hourAngle;
double raRad;
double decRad;
double latRad;
double temp;

	// convert ra, dec and lat to radians
	raRad	=	ra * M_PI / 180.0;
	decRad	=	dec * M_PI / 180.0;
	latRad	=	lat * M_PI / 180.0;

	// compute the local hour angle and return it in radians
	hourAngle	=	((sid * M_PI / 12.0) - raRad);

	// if hour angle is negative
	if (hourAngle < 0.0)
	{
		hourAngle	+=	2.0 * M_PI;
	}

	//("hourangle	=	%lf\n", hourAngle);

	// compute the altitude of the object
	*alt	=	asin(sin(latRad) * sin(decRad) + cos(latRad) * cos(decRad) * cos(hourAngle));

	// compute the azimuth angle measured from the south

	//*	Jeen Meeus  - "Astronomical Algorithms"
	// Meeus method using south as zero
	//_azi	=	atan( sin(hourAngle) / (cos(hourAngle) * sin(lat) - tan(dec) * cos(lat)) );

	// compute azi using north as zero
	// *azi	=	acos( (sin(dec) - (sin(lat) * sin(*alt))) / (cos(lat) * cos(*alt)) );
	temp	=	(sin(decRad) - (sin(latRad) * sin(*alt))) / (cos(latRad) * cos(*alt));
	printf("temp = %lf\n", temp);

	*azi	=	acos(temp);

	printf("azi = %lf\n", *azi);

	// convert to correct quadrant
	if (sin(hourAngle) < 0.0)
	{
		*azi	=	2.0 * M_PI - *azi;
	}
} // Time_ra_dec_to_alt_azi()

//******************************************************************************
// Inputs alt/azi in rads, lat in decidegs and returns field rotation degs/hour
// formula: K × cos(az) / cos(alt) , where K = sid_degs/hour * cos(lat)
// and sid_degs/hour = 15.04106858
//******************************************************************************
double Time_calc_field_rotation(double alt, double azi, double lat)
{
double latRad;
double rotRate;

	// convert lat to radians and calc rotation rate
	latRad	=	degsToRads(lat);
	rotRate	=	kARCSEC_PER_SEC * cos(latRad) * cos(azi) / cos(alt);

	return(rotRate);
} // calc_field_rotation()

//******************************************************************************
// takes obj apparent altitude in rads, temp in F, pressure inches and
//   calcs the atmospheric fraction using Bennett (NASA's choice)  and
//   returns it in radians
//******************************************************************************
double Time_calc_refraction(double alt, double temp, double press)
{
double	altDegs;
double	tempC;
double	pressMb;
double	term;
double	refraction;

	altDegs		=	radsToDegs(alt);
	tempC		=	(temp - 32.0) / 1.8;
	pressMb		=	press * 33.8637526; // Conversion factor for Hg.in -> millibars

	// Convert term to rads, calc cotangent to get refraction in arcmins
	term		=	altDegs + (7.31 / (altDegs + 4.4));
	term		=	degsToRads(term);
	refraction	=	(0.28 * pressMb / (tempC + 273)) / tan(term);

	// Convert arcmins to rads multiply by 0.000290888
	refraction *= 0.000290888;

	return(refraction);
}

//******************************************************************************
int Time_read_local_cfg(TYPE_LOCAL_CFG *local, const char *localCfgFile)
{
int		retStatus;
char	filename[kMAX_STR_LEN];
FILE	*inFile;
char	inString[kMAX_STR_LEN];
int		line	=	1;
int		okFlag	=	true;
int		column	=	0;
int		loop	=	0;
char	delimiters[]	=	" \t\r\n\v\f";	// POSIX whitespace chars
char	*token;
char	*argument;
char	*rest	=	NULL;

	CONSOLE_DEBUG(__FUNCTION__);

// list of defined tokens in config file and array to keep track of discovery
enum
{
	EPOCH,
	EPOCH_JD,
	LATITUDE,
	LONGITUDE,
	ELEVATION,
	TEMPERATURE,
	PRESSURE,
	SITE
};

cfgItem initLocalArray[] =
	{
		{	"EPOCH:",		0	},
		{	"EPOCH_JD:",	0	},
		{	"LATITUDE:",	0	},
		{	"LONGITUDE:",	0	},
		{	"ELEVATION:",	0	},
		{	"TEMPERATURE:",	0	},
		{	"PRESSURE:",	0	},
		{	"SITE:",		0	},
		{	NULL,			0	}
	};

	// If not filename provided, use default name
	if (localCfgFile == NULL)
	{
		strcpy(filename, kLOCAL_CFG_FILE);
	}
	else
	{
		strcpy(filename, localCfgFile);
	}

	// open the scope configuration file
//	if ((inFile = fopen(filename, "r")) == NULL)
	inFile	=	fopen(filename, "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error: could not open cfg file %s\n", filename);
		return (-1);
	}

	// get all of the lines in the file
	while (fgets(inString, kMAX_STR_LEN, inFile) != NULL)
	{
		// get first token of the line read from the file
		token	=	strtok_r(inString, delimiters, &rest);

		// If non-comment token found on line
		if (token != NULL && token[0] != '#')
		{
			// Get corresponding argument for the token
			argument	=	strtok_r(NULL, delimiters, &rest);
			if (argument == NULL)
			{
				fprintf(stderr, "Error: (read_local_cfg) on line %d of file '%s'\n", line, filename);
				fprintf(stderr, "       Invalid syntax: field %s is missing argument", token);
			}

			// Token and argument exist, now determine if they are valid
			Time_str_to_upper(token);

			if (strcmp(token, initLocalArray[EPOCH].parameter) == 0)
			{
				initLocalArray[EPOCH].found	=	true;
				local->baseEpoch			=	atof(argument);
				// parameter is ok, print it out
				printf("%-15.15s = %-15.4f  ", token, local->baseEpoch);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->baseEpoch < 1999.99 || local->baseEpoch > 2050.01)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->baseEpoch);
					fprintf(stderr, "       Must be between 2000.0 and 2050.0'\n");
					fprintf(stderr, "       Usage:  %s  2000.0\n", token);
				}
			}

			else if (strcmp(token, initLocalArray[EPOCH_JD].parameter) == 0)
			{
				initLocalArray[EPOCH_JD].found	=	true;
				local->baseJd					=	atof(argument);
				printf("%-15.15s = %-15f  ", token, local->baseJd);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->baseJd < 2451544.9 || local->baseJd > 2469808.1)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->baseJd);
					fprintf(stderr, "       Usage:  %s 2451545.0 \n", token);
				}
			}
			else if (strcmp(token, initLocalArray[LATITUDE].parameter) == 0)
			{
				initLocalArray[LATITUDE].found	=	true;
				local->lat						=	atof(argument);
				printf("%-15.15s = %-15f  ", token, local->lat);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->lat < -90.1 || local->lat > 90.1)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->lat);
					fprintf(stderr, "       Usage:  %s  45.0\n", token);
				}
			}
			else if (strcmp(token, initLocalArray[LONGITUDE].parameter) == 0)
			{
				initLocalArray[LONGITUDE].found	=	true;
				local->lon						=	atof(argument);

				printf("%-15.15s = %-15.15f  ", token, local->lon);
				if (column++ % 2)
				{
					printf("\n");
				}
				if (local->elev < -180.1 || local->lon > 180.1)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->elev);
					fprintf(stderr, "       Must be between -180.0 and 180.0'\n");
					fprintf(stderr, "       Usage:  %s  120.0\n", token);
				}
			}
			else if (strcmp(token, initLocalArray[ELEVATION].parameter) == 0)
			{
				initLocalArray[ELEVATION].found	=	true;
				local->elev						=	atof(argument);
				// parameter is ok, print it out
				printf("%-15.15s = %-15.4f  ", token, local->elev);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->elev < -10.0 || local->elev > 15000.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->elev);
					fprintf(stderr, "       Must be between 0.0 and 15000.0'\n");
					fprintf(stderr, "       Usage:  %s  500.0\n", token);
				}
			}

			else if (strcmp(token, initLocalArray[TEMPERATURE].parameter) == 0)
			{
				initLocalArray[TEMPERATURE].found	=	true;
				local->temp							=	atof(argument);
				printf("%-15.15s = %-15f  ", token, local->temp);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->temp < -80.0 || local->temp > 140.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->temp);
					fprintf(stderr, "       Usage:  %s  60.0\n", token);
				}
			}
			else if (strcmp(token, initLocalArray[PRESSURE].parameter) == 0)
			{
				initLocalArray[PRESSURE].found	=	true;
				local->press					=	atof(argument);
				printf("%-15.15s = %-15f  ", token, local->press);
				if (column++ % 2)
				{
					printf("\n");
				}

				if (local->lat < 28.0 || local->press > 32.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->press);
					fprintf(stderr, "       Usage:  %s  30.0\n", token);
				}
			}
			else if (strcmp(token, initLocalArray[SITE].parameter) == 0)
			{
				initLocalArray[SITE].found	=	true;
				strcpy(local->site, argument);
				printf("%-15.15s = %-15.15s  ", token, local->site);
				if (column++ % 2)
				{
					printf("\n");
				}
			}
			else
			{
				fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
				fprintf(stderr, "       Unrecognized %s field\n", token);
			}
		} // of if not a comment line
		// increment the line counter
		line++;
	} // of while

	// Even out the columns before any missing args error statements
	printf("\n");

	while (initLocalArray[loop].parameter != NULL)
	{
		if (initLocalArray[loop].found == false)
		{
			fprintf(stderr, "Error: (validate_local_cfg) Configuation variable:\n");
			fprintf(stderr, "       '%s' was not found or of improper format.\n", initLocalArray[loop].parameter);
			fprintf(stderr, "       from file '%s'\n", filename);
			okFlag	=	false;
		}
		loop++;
	}

	// configuration file had a syntax error
	if (okFlag == false)
	{
		fprintf(stderr, "Error: (validate_local_cfg) Error found in configuration:\n");
		fprintf(stderr, "       from file '%s'\n", filename);
		fflush(stderr);
		retStatus	=	-1;
	}
	else
	{
		// No Errors in configuration file
		retStatus	=	0;
	}

	return(retStatus);
} // time_read_local_cfg

//*****************************************************************************
#ifdef _TEST_SERVO_TIME_
//*****************************************************************************
int main(void)
{
localCfg	test;
double		lon		=	-120.0;
long double	jd;
long double	sid;
long double	lst;
double		ra, dec, lat, alt, azi, rate;
double		temp	=	50.0;
double		press	=	29.83;	// 29.9214;
double		v, w, x, y, z;

	// Read int the location config file
	Time_read_local_cfg(&test, NULL);

	printf("Time since epoch 1970.0 in seconds : %Lf\n", Time_get_systime());
	jd	=	Time_systime_to_jd();
	printf("Today the JD time is : %Lf\n", jd);
	sid	=	Time_jd_to_sid(jd);
	printf("Sidereal time at 0 longitude : %Lf\n", sid);
	lst	=	Time_sid_to_lst(sid, lon);
	printf("LST is  at %lf longitude : %Lf\n", lon, lst);

	// Calc alt/azi
	ra	=	(double)lst;
	dec	=	60.0;
	lat	=	45.0;

	Time_ra_dec_to_alt_azi(ra, dec, lst, lat, &alt, &azi);
	printf("ra = %lf  dec = %lf  alt = %lf  azi = %lf\n", ra, dec, alt / M_PI * 180.0, azi / M_PI * 180.0);

	// Calc field rotation rate for alt-azi
	rate	=	Time_calc_field_rotation(alt, azi, lat);

	printf("Field rotation rate = %lf degs/se\n", rate);

	// test refraction
	for (alt = 5.0; alt < 90.0; alt += 5.0)
	{

		v	=	Time_calc_refraction(degsToRads(alt), temp, press);
		w	=	Time_calc_refraction(degsToRads(alt), temp + 10.0, press);
		x	=	Time_calc_refraction(degsToRads(alt), temp - 10.0, press);
		y	=	Time_calc_refraction(degsToRads(alt), temp, press + 0.5);
		z	=	Time_calc_refraction(degsToRads(alt), temp, press - 0.5);

		v	=	radsToDegs(v) * 60.0;
		w	=	radsToDegs(w) * 60.0;
		x	=	radsToDegs(x) * 60.0;
		y	=	radsToDegs(y) * 60.0;
		z	=	radsToDegs(z) * 60.0;

		printf("Alt degs= %lf  Refract arcmin= %lf  Temp + 10= %lf  Temp - 10= %lf Press + 0.5= %lf Press - 0.5= %lf\n", alt, v, w, x, y, z);
	}

	return(0);
} // of main()
#endif // of _TEST_SERVO_TIME_
