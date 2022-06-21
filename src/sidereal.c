//**************************************************************************
//*	Name:			sidereal.c
//*
//*	Author:			Mark Sproul (C) 2019, 2020
//*
//*	Description:	library for sidereal time
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jul 15,	2019	<MLS> Started on sidereal time
//*	Nov  8,	2019	<MLS> Made sidereal into a library
//*	Dec  6,	2020	<MLS> Added CalcSiderealTime_dbl()
//*	Jun 20,	2022	<RNS> Reimplemented CalcSiderealTime with Meeus eq. 12.4
//*	Jun 20,	2022	<RNS> fixed some logic bug and a pointer bug
//*****************************************************************************
#ifdef _INCLUDE_SIDEREAL_MAIN_
	#include	<stdlib.h>
	#include	<string.h>
	#include	<strings.h>
	#include	<unistd.h>
	#include	<stdbool.h>
	#include	<stdio.h>
	#include	<math.h>
#endif // _INCLUDE_SIDEREAL_MAIN_

#include	<time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"sidereal.h"

//#define	LONGITUDE		-2.247926			//	Longitude for Manchester UK
//#define	LATITUDE		53.53				//	Latitude for Manchester UK

#define	LONGITUDE		-(74.0 + (58.0 / 60.0) + (49.0 / 3600.0)) 	//	Longitude for Shohola NJ

//**************************************************************************
// Calculate the number of days since Jan 1
static int	CalcNumDays(int year, int month, int day)
{
int		daysSinceJan1;
int		leapyear;

//	CONSOLE_DEBUG_W_NUM("year\t=",	year);
//	CONSOLE_DEBUG_W_NUM("month\t=",	month);
//	CONSOLE_DEBUG_W_NUM("day\t=",	day);

	leapyear		=	((year - 2000) % 4 == 0)? 1 : 0;
	daysSinceJan1	=	0;
	switch(month)
	{
		case 12:	daysSinceJan1	+=	30;				// Dec
		case 11:	daysSinceJan1	+=	31;				// Nov
		case 10:	daysSinceJan1	+=	30;				// Oct
		case 9: 	daysSinceJan1	+=	31;				// Sep
		case 8: 	daysSinceJan1	+=	31;				// Aug
		case 7: 	daysSinceJan1	+=	30;				// Jul
		case 6: 	daysSinceJan1	+=	31;				// Jun
		case 5: 	daysSinceJan1	+=	30;				// May
		case 4: 	daysSinceJan1	+=	31;				// Apr
		case 3: 	daysSinceJan1	+=	28 + leapyear;	// Mar (if year is leapyear, add extra day after February)
		case 2: 	daysSinceJan1	+=	31; break;		// Feb
	}
	return daysSinceJan1 + day;			// days from Jan 1 of given year
}

// **********************************************************************************
//						GREENWICH & LOCAL SIDEREAL TIME CALCULATIONS
// **********************************************************************************
// based on "ASTR 310 - Observational Astronomy: Formula for Greenwich Sidereal Time (GST)"
// see http://www.astro.umd.edu/~jph/GST_eqn.pdf formulas
// **********************************************************************************
void	CalcSiderealTime(	struct tm	*utcTime,
							struct tm	*siderealTimePtr,
							double		argLongitude)
{
time_t		utcSysTime; 
int			sidSecTime; 
double 		jd, jCent, deltaJd; 
double		gmstDegs;

	// Convert the itemized UTC time struct back to timeval format with only second accuracy
	utcSysTime	=	timegm(utcTime);

	// Calc the std JD 
	jd = ((double) utcSysTime / 86400.0) + 2440587.5;

	// Get the Julian centuries and the delta against J2000
	deltaJd	=	jd - 2451545.0L;
	jCent	=	deltaJd / 36525.0L;

	// Using Meeus formula 12.4, which does not need whole Julian days
	gmstDegs	=	280.46061837 + (360.98564736629 * deltaJd);
	jCent		*=	jCent;		//  Need the square for the next term
	gmstDegs	+=	0.000387933 * jCent;
	jCent		*=	jCent;		//  Need the cube for the next term
	gmstDegs	-=	jCent / 38710000.0;

	// add the offset for longitude in degrees
	gmstDegs 	+=	argLongitude;

	// convert from degrees to hours and normalize
	gmstDegs	/=	15.0;

	while (gmstDegs < 0.0)
	{
		gmstDegs += 24.0;
	}
	while (gmstDegs >= 24.0)
	{
		gmstDegs -= 24.0;
	}
	// Convert gmst hours to seconds
	gmstDegs *= 3600.0;
	sidSecTime	= gmstDegs;

	// Convert float to int for fractional compare
	sidSecTime	= gmstDegs;
	//Round up by one sec if the leftover fraction is >= half
	if ((gmstDegs - (double) sidSecTime) >= 0.5)
	{
		sidSecTime += 1;
	}

	// Convert to broken down time format - not the day may be off by one and it never used
	memcpy(siderealTimePtr, utcTime, sizeof(struct tm));

	// Load the actual sidereal time int the fields
	siderealTimePtr->tm_hour	= sidSecTime / 3600;
	sidSecTime = sidSecTime % 3600; 
	siderealTimePtr->tm_min		= sidSecTime / 60; 
	sidSecTime = sidSecTime % 60; 
	siderealTimePtr->tm_sec		= sidSecTime;

	// CONSOLE_DEBUG_W_NUM("tm_year\t\t=",	siderealTimePtr->tm_year);
	// CONSOLE_DEBUG_W_NUM("tm_hour\t\t=",	siderealTimePtr->tm_hour);
	// CONSOLE_DEBUG_W_NUM("tm_min\t\t=",	siderealTimePtr->tm_min);
	// CONSOLE_DEBUG_W_NUM("tm_sec\t\t=",	siderealTimePtr->tm_sec);

	return;
}

//**************************************************************************
double	CalcSiderealTime_dbl(struct tm	*utcTime, double argLongitude)
{
struct tm	myUtcTime;
struct tm	mySiderTime;
double		siderTime_Dbl;
time_t		currentTime;

	if (utcTime == NULL)
	{
		currentTime	=	time(NULL);
		myUtcTime	=	*gmtime(&currentTime);
		CalcSiderealTime(&myUtcTime, &mySiderTime, argLongitude);
	}
	else
	{
		CalcSiderealTime(utcTime, &mySiderTime, argLongitude);
	}

	siderTime_Dbl	=	mySiderTime.tm_hour +
						mySiderTime.tm_min / 60.0 +
						mySiderTime.tm_sec / 3600.0;

	return(siderTime_Dbl);
}

#ifdef _INCLUDE_SIDEREAL_MAIN_
//**************************************************************************
int main(int argc, char **argv)
{
struct tm	*linuxTime;
struct tm	utcTime;
struct tm	siderealTime;
time_t		currentTime;
int			daysSinceJan1;

	printf("Sidereal day =%3.10f\r\n",	23.0 + (56.0 / 60.0) + (4.0905 / 3600.0));

	currentTime	=	time(NULL);
	linuxTime	=	localtime(&currentTime);

	daysSinceJan1	=	CalcNumDays((1900 + linuxTime->tm_year),
									(1 + linuxTime->tm_mon),
									linuxTime->tm_mday);

	printf("daysSinceJan1 day =%d\r\n",	daysSinceJan1);

	while (true)
	{
		currentTime	=	time(NULL);
		linuxTime	=	localtime(&currentTime);

		printf("%d/%d/%d %02d:%02d:%02d",
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								(1900 + linuxTime->tm_year),
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);
		printf("\t");

		utcTime		=	*gmtime(&currentTime);


		printf("UTC=%d/%d/%d %02d:%02d:%02d",
								(1 + utcTime.tm_mon),
								utcTime.tm_mday,
								(1900 + utcTime.tm_year),
								utcTime.tm_hour,
								utcTime.tm_min,
								utcTime.tm_sec);
		fflush(stdout);

		CalcSiderealTime(&utcTime, &siderealTime, LONGITUDE);

		printf("\t");
		printf("SID=%d/%d/%d %02d:%02d:%02d",
								(1 + siderealTime.tm_mon),
								siderealTime.tm_mday,
								(1900 + siderealTime.tm_year),
								siderealTime.tm_hour,
								siderealTime.tm_min,
								siderealTime.tm_sec);

		printf("\r\n");
		sleep(1);
	}
}

#endif

