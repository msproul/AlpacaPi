//*****************************************************************************
//*		skytravel_radec.cpp		(c) 2022 by Mark Sproul
//*
//*	Description:
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  4,	2022	<MLS> Created skytravel_radec.cpp
//*****************************************************************************


//*****************************************************************************
//*	http://jonvoisey.net/blog/2018/07/data-converting-alt-az-to-ra-dec-example/
//*	convert az/el to RA dec
//*	Inputs
//*		altitude (a),
//*		azimuth (A),
//*		sidereal time (ST),
//*		latitude (phi).
//*	Outputs
//*		right ascension (alpha)
//*		declination (delta)
//*****************************************************************************

#include	<math.h>
#include	<stdio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))
//#define	DEGREES(radians)	((radians) * 180.0 / M_PI)


//*****************************************************************************
void	GetRADEC_fromAzEl(	double	azimuth_rad,
							double	altitude_rad,
							double	latitude_rad,
							double	siderealTime,
							double	*ret_rightAscension,
							double	*ret_Declination
							)
{
double	declination;
double	sin_Declination;
double	cos_HourAngle;
double	hourAngle;
double	rightAscension;


//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("altitude\t\t=",	DEGREES(altitude_rad));
	CONSOLE_DEBUG_W_DBL("azimuth\t\t=",		DEGREES(azimuth_rad));
//	CONSOLE_DEBUG_W_DBL("siderealTime\t=",	DEGREES(siderealTime));
//	CONSOLE_DEBUG_W_DBL("latitude\t\t=",	DEGREES(latitude_rad));
//
//	CONSOLE_DEBUG("");
//	CONSOLE_DEBUG_W_DBL("altitude_rad\t=",	(altitude_rad));
//	CONSOLE_DEBUG_W_DBL("azimuth_rad\t=",	(azimuth_rad));
//	CONSOLE_DEBUG_W_DBL("siderealTime\t=",	(siderealTime));
//	CONSOLE_DEBUG_W_DBL("latitude_rad\t=",	(latitude_rad));
//	CONSOLE_DEBUG("");


	//	We’ll start by finding the declination as we’ll need it for the next step.
	sin_Declination	=	(sin(latitude_rad) * sin(altitude_rad))
						+ (cos(latitude_rad) * cos(altitude_rad) * cos(azimuth_rad));

//	CONSOLE_DEBUG_W_DBL("latitude\t\t=",	DEGREES(latitude_rad));
//	CONSOLE_DEBUG_W_DBL("altitude\t\t=",	DEGREES(altitude_rad));
//	CONSOLE_DEBUG("");
//	CONSOLE_DEBUG_W_DBL("latitude\t\t=",	DEGREES(latitude_rad));
//	CONSOLE_DEBUG_W_DBL("altitude\t\t=",	DEGREES(altitude_rad));
//	CONSOLE_DEBUG_W_DBL("azimuth\t\t=",		DEGREES(azimuth_rad));
//
//	CONSOLE_DEBUG("");
//
//	CONSOLE_DEBUG_W_DBL("sin_Declination\t=", sin_Declination);


	declination		=	asin(sin_Declination);
//	CONSOLE_DEBUG_W_DBL("declination_R\t=", (declination));
//	CONSOLE_DEBUG_W_DBL("declination_D\t=", DEGREES(declination));

	//*	Next up, we’ll use our second equation to get the hour angle:
	cos_HourAngle	=	(sin(altitude_rad) - (sin(latitude_rad) * sin(declination)))
						/ ((cos(latitude_rad) * cos(declination)));

	hourAngle		=	acos(cos_HourAngle);
//	CONSOLE_DEBUG_W_DBL("cos_HourAngle\t=", (cos_HourAngle));
//	CONSOLE_DEBUG_W_DBL("hourAngle_rad\t=", (hourAngle));
//	CONSOLE_DEBUG_W_DBL("hourAngle_deg\t=", DEGREES(hourAngle));


	//*	Lastly, we can get the right ascension from the simple formula:
	rightAscension	=	siderealTime - hourAngle;
//	CONSOLE_DEBUG_W_DBL("rightAscension_R\t=", (rightAscension));
//	CONSOLE_DEBUG_W_DBL("rightAscension_D\t=", DEGREES(rightAscension));

	*ret_rightAscension	=	rightAscension;
	*ret_Declination	=	declination;

}



#ifdef _INCLUDE_RADEC_MAIN_



//*****************************************************************************
int main(int argc, char *argv[])
{
double	azimuth_A_rad;
double	altitude_a_rad;
double	latitude_rad;
double	siderealTime;
double	rightAscension;
double	declination;

	CONSOLE_DEBUG("RA/DEC test program");

	azimuth_A_rad	=	RADIANS(180);
	altitude_a_rad	=	RADIANS(60.34);
	latitude_rad	=	RADIANS(38.59);
	siderealTime	=	RADIANS(297.93);

	GetRADEC_fromAzEl(	azimuth_A_rad,
						altitude_a_rad,
						latitude_rad,
						siderealTime,
						&rightAscension,
						&declination);



}

#endif	//	_INCLUDE_RADEC_MAIN_
