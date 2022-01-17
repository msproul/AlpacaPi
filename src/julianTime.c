//*****************************************************************************
//*	julianTime.c
//*		(C) 2020 by Mark Sproul
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 28,	2019	<MLS> Added CalcMJD() Modified Julian Date
//*	Dec  1,	2020	<MLS> Created julianTime.c, moved CalcMJD() to this file
//*	Nov 30,	2020	<MLS> Added moon rise and moon set to FITS data
//*	Dec 30,	2021	<MLS> Added Julian_GetUTCfromJulian()
//*	Dec 31,	2021	<MLS> Added Julian_CalcMJDFromDate()
//*****************************************************************************

#include	<stdlib.h>
#include	<time.h>
#include	<sys/time.h>
#include	<string.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"julianTime.h"

//**************************************************************************************
//*	2400000.5 	the difference between the Julian date and the Modified Julian Date
//*	40587		the number of days between the MJD epoch (1858-11-17) and Unix epoch (1970-01-01)
//*	86400		the number of seconds in one day (24 hours)
//**************************************************************************************

#define	kJulian_MJD_Delta	(2400000.5)
#define	kSecondsPerDay		(86400.0)

//**************************************************************************************
double	Julian_ConvertFromMJD(const double mjdDate)
{
double	julianDateTime;

	julianDateTime	=	mjdDate + kJulian_MJD_Delta;
	return(julianDateTime);
}

//**************************************************************************************
//*	2400000.5 	the difference between the Julian date and the Modified Julian Date
//*	40587		the number of days between the MJD epoch (1858-11-17) and Unix epoch (1970-01-01)
//*	86400		the number of seconds in one day (24 hours)
//**************************************************************************************
double Julian_CalcDate(struct timeval *timeStruct)
{
double	julianDateTime;
double	aditionalSeconds;


	julianDateTime		=	40587 + (timeStruct->tv_sec / kSecondsPerDay);	//*	86400 number of seconds in a day
	julianDateTime		+=	2400000.5;	//*	difference between the Julian date and the Modified Julian Date
	aditionalSeconds	=	(timeStruct->tv_usec / 1000000.0) / kSecondsPerDay;
	julianDateTime		+=	aditionalSeconds;
	return(julianDateTime);
}


//**************************************************************************************
double Julian_GetCurrentDate(void)
{
double			julianDateTime;
struct timeval	timeStruct;

	gettimeofday(&timeStruct, NULL);

	julianDateTime	=	Julian_CalcDate(&timeStruct);
	return(julianDateTime);
}

//**************************************************************************************
double Julian_CalcFromDate(const int month, const int day, const int year)
{
struct timeval	timeStruct;
struct tm		linuxTime;
double			julianDateTime;

//	printf("%2d/%2d/%4d\t--\t", month, day, year);
	memset(&linuxTime, 0, sizeof(linuxTime));
	linuxTime.tm_year	=	year - 1900;
	linuxTime.tm_mon	=	month - 1;
	linuxTime.tm_mday	=	day;
	linuxTime.tm_hour	=	0;
	linuxTime.tm_min	=	0;
	linuxTime.tm_sec	=	0;
	linuxTime.tm_isdst	=	0;


	timeStruct.tv_sec	=	mktime(&linuxTime);

	//*	adjust for time zone
//	timeStruct.tv_sec	-=	(5 * 60 * 60);
//	CONSOLE_DEBUG_W_NUM("tm_year\t=",	linuxTime.tm_year);
//	CONSOLE_DEBUG_W_NUM("tm_mon\t=",	linuxTime.tm_mon);
//	CONSOLE_DEBUG_W_NUM("tm_mday\t=",	linuxTime.tm_mday);
//	CONSOLE_DEBUG_W_NUM("tm_hour\t=",	linuxTime.tm_hour);
//	CONSOLE_DEBUG_W_NUM("tm_min\t=",	linuxTime.tm_min);
//	CONSOLE_DEBUG_W_NUM("tm_sec\t=",	linuxTime.tm_sec);
//	CONSOLE_DEBUG_W_NUM("tm_isdst\t=",	linuxTime.tm_isdst);

//	CONSOLE_DEBUG_W_NUM("tm_wday\t=",	linuxTime.tm_wday);
//	CONSOLE_DEBUG_W_NUM("tm_yday\t=",	linuxTime.tm_yday);

//	CONSOLE_DEBUG_W_LONG("tv_sec\t=",	timeStruct.tv_sec);

	timeStruct.tv_usec	=	0;

	julianDateTime	=	Julian_CalcDate(&timeStruct);
	return(julianDateTime);
}


//**************************************************************************************
//*	2400000.5 	the difference between the Julian date and the Modified Julian Date
//*	40587		the number of days between the MJD epoch (1858-11-17) and Unix epoch (1970-01-01)
//*	86400		the number of seconds in one day (24 hours)
//**************************************************************************************
void Julian_GetUTCfromJulian(const double	julianDateTime, struct timeval *timeStruct)
{
double	myMJDdateTime;

	myMJDdateTime		=	julianDateTime - 2400000.5;	//*	difference between the Julian date and the Modified Julian Date
	timeStruct->tv_sec	=	(myMJDdateTime - 40587) * kSecondsPerDay;
	timeStruct->tv_usec	=	0;
}

//**************************************************************************************
//*	The value 40587 is the number of days between the MJD epoch (1858-11-17)
//*	and the Unix epoch (1970-01-01), and 86400 is the number of seconds in a day.
//*	This calculation was versified against the AAVSO Julian time calculator
//*	https://www.aavso.org/cgi-bin/jdclock.pl
//**************************************************************************************
double Julian_CalcMJD(struct timeval *timeStruct)
{
double	mjdTime;
double	aditionalSeconds;

	mjdTime				=	40587 + (timeStruct->tv_sec / kSecondsPerDay);
	aditionalSeconds	=	(timeStruct->tv_usec / 1000000.0) / kSecondsPerDay;
	mjdTime				+=	aditionalSeconds;
	return(mjdTime);
}

//**************************************************************************************
double Julian_CalcMJDFromDate(const int month, const int day, const int year)
{
struct timeval	timeStruct;
struct tm		linuxTime;
double			mjdTime;

	memset(&linuxTime, 0, sizeof(linuxTime));
	linuxTime.tm_year	=	year - 1900;
	linuxTime.tm_mon	=	month - 1;
	linuxTime.tm_mday	=	day;
	linuxTime.tm_hour	=	0;
	linuxTime.tm_min	=	0;
	linuxTime.tm_sec	=	0;
	linuxTime.tm_isdst	=	0;

	timeStruct.tv_sec	=	mktime(&linuxTime);

	//*	adjust for time zone
//	timeStruct.tv_sec	-=	(5 * 60 * 60);
	timeStruct.tv_usec	=	0;

	mjdTime	=	Julian_CalcMJD(&timeStruct);
	return(mjdTime);
}


//**************************************************************************************
void Julian_GetUTCfromMJD(const double	mjdDateTime, struct timeval *timeStruct)
{
	timeStruct->tv_sec	=	(mjdDateTime - 40587) * kSecondsPerDay;
	timeStruct->tv_usec	=	0;
}


