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
//*	May 14,	2022	<RNS> Swapped degsToRads/radsToDegs -> RADIANS/DEGREES
//*	May 14,	2022	<RNS> Moved all conversions to use RADIANS/DEGREES
//*	May 14,	2022	<RNS> Added Time_ascii_maybe_hms_tof()
//*	May 15,	2022	<RNS> added Time_normalize functions
//*	May 15,	2022	<RNS> added alt-azi tracking rates function, added to _TEST_
//*	May 15,	2022	<RNS> added Time_alt_azi_to_ra_dec() function
//*	May 16,	2022	<RNS> privatized the gServoLocalCfg global, added get_ field calls
//*	May 16,	2022	<RNS> Added _maybe_hms_ vs atof for lat & lon config field
//*	May 26,	2022	<MLS> Moved enum defs to .h file for AlpacaPi
//*	May 27,	2022	<RNS> Removed printf statements from Time_read_local_cfg()
//*	May 28,	2022	<RNS> Corrected Time_read_local_cfg() call in _TEST_
//*	May 28,	2022	<RNS> Rewrote _jd_to_sid() into _js_to_gmst() with better math
//*	May 29,	2022	<RNS> Renamed _sid_to_lst() to _gmst_to_lst() for consistency
//*	May 29,	2022	<MLS> Fixed couple of lat/lon reporting errors
//*	May 29,	2022	<MLS> Added Time_get_elev()
//*	May 30,	2022	<MLS> Added Time_set_lat(), Time_set_lon(), Time_set_elev()
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

////*****************************************************************************
TYPE_LOCAL_CFG gServoLocalCfg;

////*****************************************************************************
TYPE_CfgItem gLocationArray[] =
{
	{"EPOCH:",		0},
	{"EPOCH_JD:",	0},
	{"LATITUDE:",	0},
	{"LONGITUDE:",	0},
	{"ELEVATION:",	0},
	{"TEMPERATURE:",0},
	{"PRESSURE:",	0},
	{"SITE:",		0},
	{NULL,			0}
};

////*****************************************************************************
// void Time_deci_days_to_hours(double *day)
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
void Time_normalize_HA(double *ha)
{
	*ha	=	(*ha < -12.0) ? *ha + 24.0 : *ha;
	*ha	=	(*ha > 12.0) ? *ha - 24.0 : *ha;
}

//*****************************************************************************
void Time_normalize_hours(long double *hours)
{
	// make sure hours is positive and < 24.0, iterate if needed
	while (*hours < 0.0)
	{
		*hours += 24.0;
	}
	while (*hours >= 24.0)
	{
		*hours -= 24.0;
	}
}

//*****************************************************************************
void Time_normalize_RA(double *ra)
{
long double temp;

	temp	=	(long double)*ra;
	Time_normalize_hours(&temp);

	*ra	=	(double)temp;
}

//*****************************************************************************
void Time_str_to_upper(char *in)
{
	int16_t index;

	if (in != NULL)
	{
		for (index = 0; in[index] != '\0'; index++)
		{
			in[index] = toupper(in[index]);
		}
	}
} // of Time_str_to_upper()

//****************************************************************
// Check to see if the input str is generic floating point format
// of if it's in HMS format denoted by a colon and not a decimal.
// May convert from HMS to deci, return token is in decimal format
//****************************************************************
double Time_ascii_maybe_HMS_tof(char *token)
{
char	*ptr;
double	ret;

	ptr	=	token;

	// look for the HMS colon in the input str until end-of-str
	while (*ptr != ':' && *ptr != '\0')
	{
		ptr++;
	}
	// if we stopped on a colon, input str is in HMS
	if (*ptr == ':')
	{
		// Convert the string to floating format
		*ptr	=	'.';
		// convert to a double and from HMS to deci
		ret	=	atof(token);
		Time_hms_hours_to_deci(&ret);
	}
	else
	{
		// input format is a floating point
		ret	=	atof(token);
	}

	return(ret);
} // of Time_ascii_maybe_HMS_to_float()

//*****************************************************************************
// Returns Unix system time in  decimal seconds.microseconds from Epoch 1970.0
// Note: 946713600 Unix system time for J2000.0
//*****************************************************************************
long double Time_get_systime(void)
{
struct timeval current;

	gettimeofday(&current, NULL);

	return(current.tv_sec + (current.tv_usec / 1000000.0));
} // of Time_get_systime()

//*****************************************************************************
// Divide by the number of seconds in a day and add JD offset for epoch 1970.0
// https://aa.usno.navy.mil/data/JulianDate for a trustworthy calculator
//*****************************************************************************
long double Time_systime_to_jd(void)
{
	// Divide by the number of seconds in a day and add JD offset for epoch 1970.0
	return((Time_get_systime() / 86400.0) + 2440587.5);

} // of Time_systime_to_jd

//*****************************************************************************
// Input a Julian date and returns the corresponding Greenwich mean sidereal time i
// in decimal hours.  Uses Jean Meeus formula 12.4 from his books
// https://aa.usno.navy.mil/data/siderealtime for a trustworthy calculator
//*****************************************************************************
long double Time_jd_to_gmst(long double jd)
{
long double		jCent;
long double		gmstDegs;
long double		deltaJd;

	// Get the Julian centuries and the delta against J2000
	deltaJd	=	jd - 2451545.0L;
	jCent	=	deltaJd / 36525.0L;

	// Using Meeus formula 12.4, which does not need whole Julian days
	gmstDegs	=	280.46061837L + (360.98564736629L * deltaJd);
	jCent		*=	jCent;		//  Need the square for the next term
	gmstDegs	+=	0.000387933L * jCent;
	jCent		*=	jCent;		//  Need the cube for the next term
	gmstDegs	-=	jCent / 38710000.0L;

	// convert from degrees to hours and normalize
	gmstDegs	/=	15.0L;
	Time_normalize_hours(&gmstDegs);

	return(gmstDegs);
}

//*****************************************************************************
// Time_gmst_to_lst takes in the Greenwich sidereal time in decimal
//   hours and longitude in decimal degrees and returns the result
//   (local sidereal time) in decimal hours normalized 0 - 23.99
//*****************************************************************************
long double Time_gmst_to_lst(long double sid, double lon)
{
	sid	-=	(lon / 15.0);

	// make sure sid is postive and < 24.0
	Time_normalize_hours(&sid);

	return(sid);
}

//*****************************************************************************
// Time_check_hms - Checks the range of a HMS variable and
//     and truncates it to XXX.595999 wrapping up.
// Note: Truncation will occur for all min and sec values >= 60.0
//*****************************************************************************
void Time_check_hms(double *hms)
{
double	fractSec;
int32_t	hours;
int32_t	minutes;
int32_t	seconds;
double	sign;
double	value;

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
double	fract;
double	fractSec;
int32_t	hours;
int32_t	minutes;
int32_t	seconds;

	hours		=	(int32_t)*value;
	fract		=	((*value - (double)hours) * 3600.0);
	minutes		=	((int32_t)fract) / 60;
	seconds		=	((int32_t)fract) - minutes * 60;
	fractSec	=	fract - (double)((int32_t)fract);

	// make sure the range is ok for sec otherwise truncate it 0.0
	if (seconds > 59)
	{
		seconds	=	0;
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
double		fractSec;
int32_t		hours;
int32_t		minutes;
int32_t		seconds;

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
// Returns the latitude field from the global location file struct
//*****************************************************************************
double Time_get_lat(void)
{
	return(gServoLocalCfg.lat);
}

//*****************************************************************************
void Time_set_lat(double newLatValue)
{
	gServoLocalCfg.lat	=	newLatValue;
}

//*****************************************************************************
// Returns the longitude field from the global location file struct
//*****************************************************************************
double Time_get_lon(void)
{
	return(gServoLocalCfg.lon);
}

//*****************************************************************************
void Time_set_lon(double newLonValue)
{
	gServoLocalCfg.lon	=	newLonValue;
}

//*****************************************************************************
// Returns the elevation field from the global location file struct
//*****************************************************************************
double Time_get_elev(void)
{
	return(gServoLocalCfg.elev);
}

//*****************************************************************************
void Time_set_elev(double newElevValue)
{
	gServoLocalCfg.elev	=	newElevValue;
}

//*****************************************************************************
double Time_get_temperature(void)
{
	return(gServoLocalCfg.temp);
}

//*****************************************************************************
double Time_get_pressure(void)
{
	return(gServoLocalCfg.press);
}


//*****************************************************************************
// Returns the site name field from the global location file struct
//*****************************************************************************
const char *Time_get_site(void)
{
	return(gServoLocalCfg.site);
}

//*****************************************************************************
// Takes in ra, dec, let in deci degs and sidereal in deci hours and returns alt/azi in radians
// Azi is done returned in the form of North = 0 and NESW sweep (E = 90, S = 180, W = 270)
//*****************************************************************************
void Time_ra_dec_to_alt_azi(double ra, double dec, long double lst, double lat, double *alt, double *azi)
{
double	hourAngle;
double	raRad;
double	decRad;
double	latRad;
double	temp;

	// convert ra, dec and lat to radians
	raRad	=	RADIANS(ra);
	decRad	=	RADIANS(dec);
	latRad	=	RADIANS(lat);

	// compute the local hour angle and return it in radians
	hourAngle	=	((lst * M_PI / 12.0) - raRad);

	// if hour angle is negative
	if (hourAngle < 0.0)
	{
		hourAngle += 2.0 * M_PI;
	}

	//("hourangle	=	%lf\n", hourAngle);

	// compute the altitude of the object
	*alt	=	asin(sin(latRad) * sin(decRad) + cos(latRad) * cos(decRad) * cos(hourAngle));

	// compute the azimuth angle measured from the south
	// from Jean Meeus  - "Astronomical Algorithms"
	// Meeus method using south as zero
	//_azi	=	atan( sin(hourAngle) / (cos(hourAngle) * sin(lat) - tan(dec) * cos(lat)) );

	// compute azi using north as zero
	// *azi	=	acos( (sin(dec) - (sin(lat) * sin(*alt))) / (cos(lat) * cos(*alt)) );
	temp	=	(sin(decRad) - (sin(latRad) * sin(*alt))) / (cos(latRad) * cos(*alt));
	*azi	=	acos(temp);

	// convert to correct quadrant
	if (sin(hourAngle) < 0.0)
	{
		*azi	=	2.0 * M_PI - *azi;
	}
} // Time_ra_dec_to_alt_azi()

//******************************************************************************
// Convert from alt-azi to ra/dec.  Inputs are alt-azi in radians, lst in decihours
// lat is decidegs and the routine returns RA and dec in decihours and decidegs
//******************************************************************************
void Time_alt_azi_to_ra_dec(double alt, double azi, long double lst, double lat, double *ra, double *dec)
{
double hourAngle;
double raRad;
double decRad;
double latRad	=	RADIANS(lat);

	// Compute the easy axis, it's always dec ;^)
	decRad	=	asin((sin(alt) * sin(latRad)) + (cos(alt) * cos(latRad) * cos(azi)));

	// compute hour angle in radians
	hourAngle	=	acos((sin(alt) - (sin(latRad) * sin(decRad))) / (cos(latRad) * cos(decRad)));

	// convert to correct quadrant, Note: everything is in radians
	if (sin(azi) > 0.0)
	{
		hourAngle	=	2.0 * M_PI - hourAngle;
	}

	// Convert lst to radians and use hour angle to calc ra position
	raRad	=	((lst * M_PI / 12.0) - hourAngle);

	/* make sure ra is positive */
	raRad	=	(raRad < 0.0) ? raRad + 2.0 * M_PI : raRad;

	/* convert to degrees */
	*ra		=	DEGREES(raRad);
	*dec	=	DEGREES(decRad);

} // of Time_alt_azi_to_ra_dec()

//******************************************************************************
// Inputs alt/azi in rads, lat in decidegs and returns the alt-azi tracking
// rates for that position in degs/min
// https://www.ing.iac.es//~docs/tcs/software/TCS_PAPER_RL.pdf
//******************************************************************************
int Time_calc_alt_azi_tracking(double alt, double azi, double lat, double *rateAlt, double *rateAzi)
{
double zenith	=	(M_PI / 2.0) - alt; // in rads
double latRad	=	RADIANS(lat);

	// chekc for a reasonable value for zenith to avoid sin() divisor -> zero
	if (zenith > (0.99 * M_PI) || zenith < 0.01 * M_PI)
	{
		return(kERROR);
	}
	// the magic rate formulas for both axes in arcsec/sec
	*rateAlt	=	cos(latRad) * sin(azi) * kARCSEC_PER_SEC;
	*rateAzi	=	(sin(latRad) * sin(zenith) - (cos(latRad) * cos(zenith) * cos(azi))) / sin(zenith) * kARCSEC_PER_SEC;

	// TODO: integrate the forumla for blind spot region where azi rate exceeds axis max vel
	// this should be an return error condition

	return(kSTATUS_OK);
} // of Time_calc_alt_azi_tracking()

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
	latRad	=	RADIANS(lat);
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
double altDegs;
double tempC;
double pressMb;
double term;
double refraction;

	altDegs		=	DEGREES(alt);
	tempC		=	(temp - 32.0) / 1.8;
	pressMb		=	press * 33.8637526; // Conversion factor for Hg.in -> millibars

	// Convert term to rads, calc cotangent to get refraction in arcmins
	term		=	altDegs + (7.31 / (altDegs + 4.4));
	term		=	RADIANS(term);
	refraction	=	(0.28 * pressMb / (tempC + 273)) / tan(term);

	// Convert arcmins to rads multiply by 0.000290888
	refraction	*=	0.000290888;

	return(refraction);
}

//******************************************************************************
int Time_read_local_cfg(const char *localCfgFile)
{
TYPE_LOCAL_CFG	*local;
int				retStatus;
char			filename[kMAX_STR_LEN];
FILE			*inFile;
char			inString[kMAX_STR_LEN];
int				line	=	1;
int				okFlag	=	true;
int				loop	=	0;
char			delimiters[]	=	" \t\r\n\v\f";	//	POSIX whitespace chars
char			*token;
char			*argument;
char			*rest	=	NULL;

	CONSOLE_DEBUG(__FUNCTION__);

	// Map the local pointer to the address of private local global
	local	=	&gServoLocalCfg;

	// If not filename provided, use default name
	if (localCfgFile == NULL)
	{
		strcpy(filename, kLOCAL_CFG_FILE);
	}
	else
	{
		strcpy(filename, localCfgFile);
	}

	// open the mount locations configuration file
	//	if ((inFile = fopen(filename, "r")) == NULL)
	inFile	=	fopen(filename, "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error: could not open location cfg file %s\n", filename);
		return(-1);
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

			if (strcmp(token, gLocationArray[EPOCH].parameter) == 0)
			{
				gLocationArray[EPOCH].found	=	true;
				local->baseEpoch	=	atof(argument);

				if (local->baseEpoch < 1999.99 || local->baseEpoch > 2050.01)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->baseEpoch);
					fprintf(stderr, "       Must be between 2000.0 and 2050.0'\n");
					fprintf(stderr, "       Usage:  %s  2000.0\n", token);
				}
			}
			else if (strcmp(token, gLocationArray[EPOCH_JD].parameter) == 0)
			{
				gLocationArray[EPOCH_JD].found	=	true;
				local->baseJd					=	atof(argument);

				if ((local->baseJd < 2451544.9) || (local->baseJd > 2469808.1))
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->baseJd);
					fprintf(stderr, "       Usage:  %s 2451545.0 \n", token);
				}
			}
			else if (strcmp(token, gLocationArray[LATITUDE].parameter) == 0)
			{
				gLocationArray[LATITUDE].found	=	true;
				local->lat						=	Time_ascii_maybe_HMS_tof(argument);

				if ((local->lat < -90.1) || (local->lat > 90.1))
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->lat);
					fprintf(stderr, "       Usage:  %s  45.0\n", token);
				}
			}
			else if (strcmp(token, gLocationArray[LONGITUDE].parameter) == 0)
			{
				gLocationArray[LONGITUDE].found	=	true;
				local->lon						=	Time_ascii_maybe_HMS_tof(argument);

				if ((local->lon < -180.1) || (local->lon > 180.1))
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->elev);
					fprintf(stderr, "       Must be between -180.0 and 180.0'\n");
					fprintf(stderr, "       Usage:  %s  120.0\n", token);
				}
			}
			else if (strcmp(token, gLocationArray[ELEVATION].parameter) == 0)
			{
				gLocationArray[ELEVATION].found	=	true;
				local->elev						=	atof(argument);

				if (local->elev < -10.0 || local->elev > 15000.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->elev);
					fprintf(stderr, "       Must be between 0.0 and 15000.0'\n");
					fprintf(stderr, "       Usage:  %s  500.0\n", token);
				}
			}

			else if (strcmp(token, gLocationArray[TEMPERATURE].parameter) == 0)
			{
				gLocationArray[TEMPERATURE].found	=	true;
				local->temp							=	atof(argument);

				if (local->temp < -80.0 || local->temp > 140.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->temp);
					fprintf(stderr, "       Usage:  %s  60.0\n", token);
				}
			}
			else if (strcmp(token, gLocationArray[PRESSURE].parameter) == 0)
			{
				gLocationArray[PRESSURE].found	=	true;
				local->press					=	atof(argument);

				if (local->lat < 28.0 || local->press > 32.0)
				{
					fprintf(stderr, "Error: (init_local_cfg) on line %d of file '%s'\n", line, filename);
					fprintf(stderr, "       Invalid %s field '%f'\n", token, local->press);
					fprintf(stderr, "       Usage:  %s  30.0\n", token);
				}
			}
			else if (strcmp(token, gLocationArray[SITE].parameter) == 0)
			{
				gLocationArray[SITE].found	=	true;
				strcpy(local->site, argument);
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

	while (gLocationArray[loop].parameter != NULL)
	{
		if (gLocationArray[loop].found == false)
		{
			fprintf(stderr, "Error: (validate_local_cfg) Configuation variable:\n");
			fprintf(stderr, "       '%s' was not found or of improper format.\n", gLocationArray[loop].parameter);
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
#include <time.h>
int main(void)
{
double		lon	=	-120.0;
long double	jd;
long double	sid;
long double	lst;
double		ra, dec, lat, alt, azi, rate;
double		temp	=	50.0;
double		press	=	29.83;	// 29.9214;
double		v, w, x, y, z;
double		rateAlt, rateAzi;
int			status;
char		inStr[256];
time_t		current;
struct tm	*utcTime;

	// Read int the location config file
	Time_read_local_cfg("servo_location.cfg");

	// wait for a second rollover
	current	=	time(NULL);
	while (current == time(NULL))
		;

	// Now at the beginning of a new second
	current	=	time(NULL);
	utcTime	=	gmtime(&current);
	printf("UTC Time: %2d:%02d:%02d\n", (utcTime->tm_hour) % 24, utcTime->tm_min, utcTime->tm_sec);
	printf("Time since epoch 1970.0 in seconds : %.8Lf\n", Time_get_systime());
	jd	=	Time_systime_to_jd();
	printf("JD time now is : %.8Lf\n", jd);
	sid	=	Time_jd_to_gmst(jd);
	printf("SID  is %.8Lf for JD = %.8Lf\n", sid, jd);
	sid	=	Time_jd_to_gmst(jd);
	printf("GMST is %.8Lf for JD = %.8Lf\n", sid, jd);
	lst	=	Time_sid_to_lst(sid, lon);
	printf("LST is  at %Lf longitude : %lf\n\n", lst, lon);

	// Calc alt/azi
	ra	=	(double)lst;
	dec	=	60.0;
	lat	=	45.0;

	Time_ra_dec_to_alt_azi(ra, dec, lst, lat, &alt, &azi);
	printf("ra = %lf  dec = %lf  alt = %lf  azi = %lf\n", ra, dec, DEGREES(alt), DEGREES(azi));

	// Calc field rotation rate for alt-azi
	rate	=	Time_calc_field_rotation(alt, azi, lat);

	printf("Field rotation rate = %lf degs/se\n", rate);

	// test refraction
	for (alt = 5.0; alt < 90.0; alt += 5.0)
	{

		v	=	Time_calc_refraction(RADIANS(alt), temp, press);
		w	=	Time_calc_refraction(RADIANS(alt), temp + 10.0, press);
		x	=	Time_calc_refraction(RADIANS(alt), temp - 10.0, press);
		y	=	Time_calc_refraction(RADIANS(alt), temp, press + 0.5);
		z	=	Time_calc_refraction(RADIANS(alt), temp, press - 0.5);

		v	=	DEGREES(v) * 60.0;
		w	=	DEGREES(w) * 60.0;
		x	=	DEGREES(x) * 60.0;
		y	=	DEGREES(y) * 60.0;
		z	=	DEGREES(z) * 60.0;

		printf("Alt degs= %lf  Refract arcmin= %lf  Temp + 10= %lf  Temp - 10= %lf Press + 0.5= %lf Press - 0.5= %lf\n", alt, v, w, x, y, z);
	}

	while (1)
	{
		printf("\nTesting the alt-azi tracking rates\n\n");

		printf("\nEnter alt azi lat ->  ");

		fgets(inStr, 256, stdin);
		sscanf(inStr, "%lf %lf %lf", &alt, &azi, &lat);
		alt	=	RADIANS(alt);
		azi	=	RADIANS(azi);

		if (inStr[0] == 'q' || inStr[0] == 'Q')
		{
			return(-1);
		}
		status	=	Time_calc_alt_azi_tracking(alt, azi, lat, &rateAlt, &rateAzi);

		if (status != kSTATUS_OK)
		{
			printf("Routne returned error\n\n");
		}
		else
		{
			printf("Alt = %f    Azi = %f    Lat = %f    rAlt = %f   rAzi %f\n", alt, azi, lat, rateAlt, rateAzi);
		}
	}
	return(0);
} // of main()
#endif // of _TEST_SERVO_TIME_
