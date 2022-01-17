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
//*****************************************************************************

#ifdef _INCLUDE_SIDERIAL_MAIN_
	#include	<stdlib.h>
	#include	<string.h>
	#include	<strings.h>
	#include	<unistd.h>
	#include	<stdbool.h>
	#include	<stdio.h>
	#include	<math.h>
#endif // _INCLUDE_SIDERIAL_MAIN_

#include	<time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"sidereal.h"

// sidereal calculation constants
#define	kDC				0.0657098244
#define	kTC				1.00273791
#define	kGC				6.648605
#define	kG2000			6.5988098
#define	kLC				0.0497958000000001
#define	kNC				-0.0159140999999998
#define	kFUDGE			-0.013922				//	fudge factor (unnecessary?
//#define	LONGITUDE		-2.247926			//	Longitude for Manchester UK
//#define	LATITUDE		53.53					//	Latitude for Manchester UK

#define	LONGITUDE		-(74.0 + (58.0 / 60.0) + (49.0 / 3600.0)) 	//	Longitude for Shohola NJ

//#define	kSiderealday	23.9344699
#define	kSiderealday	(23.0 + (56.0 / 60.0) + (4.0905 / 3600.0))	//	length of sidereal day (23:56:04.0905)


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



// ********************************************************************************** //
//						GREENWICH & LOCAL SIDEREAL TIME CALCULATIONS
// ********************************************************************************** //
// based on "ASTR 310 - Observational Astronomy: Formula for Greenwich Sidereal Time (GST)"
// see http://www.astro.umd.edu/~jph/GST_eqn.pdf formulas

//**************************************************************************
void	CalcSiderealTime(	struct tm	*utcTime,
							struct tm	*siderealTimePtr,
							double		argLongitude)
{
double	utc;
int		fullYear;
int		g;
int		leap;		// number of leap years since 2000
int		nonleap;	// number of non-leap years since 2000
double	G;			// number of days
int		numDays;
double	GST;		// Greenwich Sidereal Time
double	LST;		// Local apparent Sidereal Time
int		dh,dm,ds;	// local sidereal time

	// decimal form
	utc	=	utcTime->tm_hour +
			utcTime->tm_min / 60.0 +
			utcTime->tm_sec / 3600.0;


	fullYear	=	1900 + utcTime->tm_year;
	// calculate numDays
	numDays		=	CalcNumDays(fullYear, (utcTime->tm_mon + 1), utcTime->tm_mday);


	// calculate G (based on extrapolation)
	g		=	(fullYear - 2000);
	leap	=	(g + 1.0) / 4.0;							// number of leap years since 2000
	nonleap	=	g - leap;									// number of non-leap years since 2000
	G		=	kG2000 + (leap * kLC) + (nonleap * kNC);	// number of days

//	CONSOLE_DEBUG_W_DBL("G\t\t=",	G);

	// calculate GST and Local Sidereal Time (LST)
	GST		=	G + (kDC * numDays) + (kTC * utc) + kFUDGE;			// Greenwich Sidereal Time
	LST		=	GST + 24.0 + (argLongitude / 360.0 * kSiderealday);	// adjust for longitude (longitude portion of siderail day
	while (LST > 24.0)
	{
		LST	-=	24.0;
	}											// adjust to bring into 0-24 hours
	dh	=	LST;								// translate into hours, ...
	dm	=	(LST - dh) * 60.0;					//... mins and ...
	ds	=	(LST - dh - (dm / 60.0))*3600.0;	//... seconds


//	printf("\r\n");
//	CONSOLE_DEBUG_W_NUM("fullYear\t=", fullYear);
//	CONSOLE_DEBUG_W_NUM("g\t\t=", g);
//	CONSOLE_DEBUG_W_NUM("leap\t=", leap);
//	CONSOLE_DEBUG_W_NUM("nonleap\t=", nonleap);

//	CONSOLE_DEBUG_W_DBL("LST\t=", LST);
//	CONSOLE_DEBUG_W_NUM("dh\t\t=", dh);
//	CONSOLE_DEBUG_W_NUM("dm\t\t=", dm);
//	CONSOLE_DEBUG_W_NUM("ds\t\t=", ds);

	//*	copy all of the UTC stuff over, then change what needs to be changed
	*siderealTimePtr			=	*utcTime;
	siderealTimePtr->tm_year	=	fullYear - 1900;
	siderealTimePtr->tm_hour	=	dh;
	siderealTimePtr->tm_min		=	dm;
	siderealTimePtr->tm_sec		=	ds;

	//**********************************************
	//*	Added by MLS, Nov 8, 2019
	//*	check to see if we went over a day boundary
//	if (siderealTimePtr->tm_hour == 0)
//	{
//		//*	we are on the next day
//		CONSOLE_DEBUG("We have crossed a day boundry!!!!!!!!!!!!!!!!!!!!!");
//		CONSOLE_DEBUG_W_NUM("utcTime->tm_mon\t=",	(utcTime->tm_mon + 1));
//		CONSOLE_DEBUG_W_NUM("utcTime->tm_mday\t=",	utcTime->tm_mday);
//		CONSOLE_DEBUG_W_NUM("utcTime->tm_hour\t=",	utcTime->tm_hour);
//		CONSOLE_DEBUG_W_NUM("sidTime->tm_hour\t=",	siderealTimePtr->tm_hour);
//		exit(0);
//	}

//	CONSOLE_DEBUG_W_NUM("tm_year\t\t=",	siderealTimePtr->tm_year);
//	CONSOLE_DEBUG_W_NUM("tm_hour\t\t=",	siderealTimePtr->tm_hour);
//	CONSOLE_DEBUG_W_NUM("tm_min\t\t=",	siderealTimePtr->tm_min);
//	CONSOLE_DEBUG_W_NUM("tm_sec\t\t=",	siderealTimePtr->tm_sec);

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

#ifdef _INCLUDE_SIDERIAL_MAIN_
//**************************************************************************
int main(int argc, char **argv)
{
struct tm	*linuxTime;
struct tm	utcTime;
struct tm	siderealTime;
time_t		currentTime;
int			daysSinceJan1;

	printf("Sidereal day =%3.10f\r\n",	kSiderealday);
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

