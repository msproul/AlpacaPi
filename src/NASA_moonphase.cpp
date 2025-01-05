//*****************************************************************************
//*		NASA_moonphase.cpp		(c) 2024 by Mark Sproul
//*
//*
//*	https://svs.gsfc.nasa.gov/gallery/moonphase/
//*	https://svs.gsfc.nasa.gov/5187		2024 data
//*	https://svs.gsfc.nasa.gov/5415/		2025 data
//*	https://svs.gsfc.nasa.gov/help/#apis-dialamoon
//*	https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/mooninfo_2024.txt
//*	https://svs.gsfc.nasa.gov/vis/a000000/a005400/a005415/mooninfo_2025.txt

//*		NASA Official: Mark SubbaRao	mark.u.subbarao@nasa.gov
//*		SVS Contact: Alex Kekesi		alex.kekesi@nasa.gov
//*		Site Curator: Ella Kaplan		ella.kaplan@nasa.gov
//*		Web Page Design: Ella Kaplan	ella.kaplan@nasa.gov
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 25,	2024	<MLS> Created NASA_moonphase.cpp
//*	Mar 26,	2024	<MLS> Added minutes & seconds to argument list so we can do interpolation
//*	Mar 26,	2024	<MLS> Now doing interpolation for Age, Diam, Dist & Phase
//*	Mar 27,	2024	<MLS> Verified interpolation
//*	Mar 27,	2024	<MLS> Added _ENABLE_INTERPOLATION_
//*	Mar 27,	2024	<MLS> Added NASA_GetDayOfYear()
//*	Mar 28,	2024	<MLS> Added TestInteroplation()
//*	Mar 31,	2024	<MLS> Made internal polar axis angle +/- 180 to fix interpolation
//*	Apr  1,	2024	<MLS> Added NASA_GetMoonImageCount()
//*	Apr  4,	2024	<MLS> Added NASA_DownloadMoonPhaseData()
//*	Apr  5,	2024	<MLS> Added NASA_StartMoonImageDownloadThread()
//*	Apr  6,	2024	<MLS> Fixed moon image number bug (was off by 1)
//*	May 11,	2024	<MLS> Added CalculateDeclinationAvg()
//*	Dec 11,	2024	<MLS> Added NASA_DownloadOneMoonPhaseFile()
//*	Dec 11,	2024	<MLS> Updated downloads for 2025
//*****************************************************************************
//    # https://skyandtelescope.org/astronomy-resources/native-american-full-moon-names/
//    JAN = "wolf"
//    FEB = "snow"
//    MAR = "worm"
//    APR = "pink"
//    MAY = "flower"
//    JUN = "strawberry"
//    JUL = "buck"
//    AUG = "sturgeon"
//    SEP = "corn"
//    OCT = "hunters"
//    NOV = "beaver"
//    DEC = "cold"
//*****************************************************************************
//*	Link of interest on moon movement
//*	https://astronomy.stackexchange.com/questions/29932/how-to-calculate-declination-of-moon
//*****************************************************************************

#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<math.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<pthread.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"

#include	"NASA_moonphase.h"

#define	_ENABLE_INTERPOLATION_

#define	kNASAmoonPhaseDir	"NASA_MoonInfo/"

TYPE_MoonPhase	gMoonPhaseInfo[kMoonPhaseRecCnt];
int				gMoonPhaseCnt	=	0;

TYPE_MoonInfoFile	gMoonInfoFileList[kMoonFileMax];
int					gMoonInfoFileCnt	=	0;

//*****************************************************************************
int	NASA_GetDayOfYear(int year, int month, int day)
{
//								 Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
int		daysInTheMonth[]	=	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int		dayOfTheYear;
int		iii;


	dayOfTheYear	=	0;
	iii				=	0;
	//*	add up the days for each month
	while ((iii < (month -1)) && (iii < 12))
	{
		dayOfTheYear	+=	daysInTheMonth[iii];
		iii++;
	}
	//*	check for leap year
	if ((month > 2) && ((year % 4) == 0))
	{
		dayOfTheYear	+=	1;
	}
	dayOfTheYear	+=	day;
	dayOfTheYear	-=	1;
	return(dayOfTheYear);
}

//*****************************************************************************
int	NASA_GetPhaseIndex(int year, int month, int day, int hour)
{
int		phaseIndex			=	-1;
int		dayOfTheYear;

	if (gMoonPhaseCnt > 0)
	{
		dayOfTheYear	=	NASA_GetDayOfYear(year, month, day);
		phaseIndex		=	dayOfTheYear * 24;
		phaseIndex		+=	hour;
//		phaseIndex		+=	1;	//*	there is no image 0
	}

	return(phaseIndex);
}

//*****************************************************************************
//*	Month	1->12
//*	Day		1->31
//*	Hour	0->23
//*	Minute	0->59
//*	Second	0->59
//*****************************************************************************
bool	NASA_GetMoonPhaseInfo(	int		year,
								int		month,
								int		day,
								int		hour,
								int		minute,
								int		second,
								TYPE_MoonPhase *moonPhaseInfo)
{
int		phaseIndex;
bool	validFlag	=	false;
double	deltaValue;

//	CONSOLE_DEBUG_W_NUM("years  \t=", years);
//	CONSOLE_DEBUG_W_NUM("months \t=", months);
//	CONSOLE_DEBUG_W_NUM("days   \t=", days);
//	CONSOLE_DEBUG_W_NUM("hours  \t=", hours);
//	CONSOLE_DEBUG_W_NUM("minutes\t=", minutes);

	phaseIndex		=	NASA_GetPhaseIndex(year, month, day, hour);
//	CONSOLE_DEBUG_W_NUM("phaseIndex\t=", phaseIndex);
	if (phaseIndex >= 0)
	{
		*moonPhaseInfo				=	gMoonPhaseInfo[phaseIndex];
		moonPhaseInfo->Time_Minute	=	minute;
		moonPhaseInfo->Time_Second	=	second;

	#ifdef _ENABLE_INTERPOLATION_
		if ((minute > 0) || (second > 0))
		{
		int		nextValueIndex;
		double	nextValueAge;
		double	nextValueDiam;
		double	nextValueDist;
		double	nextValuePhase;
		double	nextValueAngle;
		double	nextValueELat;
		double	nextValueELon;
		double	myMinute;

			nextValueIndex	=	phaseIndex + 1;
			if (nextValueIndex < gMoonPhaseCnt)
			{
				myMinute				=	minute + (second / 60.0);
//				CONSOLE_DEBUG_W_DBL("myMinute\t=", myMinute);
				//*	lets interpolate
				nextValuePhase			=	gMoonPhaseInfo[nextValueIndex].Phase;
				nextValueAge			=	gMoonPhaseInfo[nextValueIndex].Age;
				nextValueDiam			=	gMoonPhaseInfo[nextValueIndex].Diam;
				nextValueDist			=	gMoonPhaseInfo[nextValueIndex].Dist;
				nextValueAngle			=	gMoonPhaseInfo[nextValueIndex].AxisA;
				nextValueELat			=	gMoonPhaseInfo[nextValueIndex].ELat;
				nextValueELon			=	gMoonPhaseInfo[nextValueIndex].ELon;

				//*	phase
				deltaValue				=	nextValuePhase - moonPhaseInfo->Phase;
				moonPhaseInfo->Phase	+=	myMinute * (deltaValue / 60.0);

				//*	age
				deltaValue				=	nextValueAge - moonPhaseInfo->Age;
				moonPhaseInfo->Age		+=	myMinute * (deltaValue / 60.0);

				//*	Diameter
				deltaValue				=	nextValueDiam - moonPhaseInfo->Diam;
				moonPhaseInfo->Diam		+=	myMinute * (deltaValue / 60.0);

				//*	Distance
				deltaValue				=	nextValueDist - moonPhaseInfo->Dist;
				moonPhaseInfo->Dist		+=	myMinute * (deltaValue / 60.0);

				//*	Polar Angle
				deltaValue				=	nextValueAngle - moonPhaseInfo->AxisA;
				moonPhaseInfo->AxisA	+=	myMinute * (deltaValue / 60.0);

				//*	ELat
				deltaValue				=	nextValueELat - moonPhaseInfo->ELat;
				moonPhaseInfo->ELat		+=	myMinute * (deltaValue / 60.0);

				//*	ELon
				deltaValue				=	nextValueELon - moonPhaseInfo->ELon;
				moonPhaseInfo->ELon		+=	myMinute * (deltaValue / 60.0);
			}
		}
	#endif // _ENABLE_INTERPOLATION_
		validFlag		=	true;
	}
	else
	{
		memset(moonPhaseInfo, 0, sizeof(TYPE_MoonPhase));
		validFlag		=	false;

	}
	return(validFlag);
}

//*****************************************************************************
static int	GetMonthFromString(char *month3Letter)
{
int	monthNumber	=	0;

	if (strcasecmp(month3Letter, "Jan") == 0)
	{
		monthNumber	=	1;
	}
	else if (strcasecmp(month3Letter, "Feb") == 0)
	{
		monthNumber	=	2;
	}
	else if (strcasecmp(month3Letter, "Mar") == 0)
	{
		monthNumber	=	3;
	}
	else if (strcasecmp(month3Letter, "Apr") == 0)
	{
		monthNumber	=	4;
	}
	else if (strcasecmp(month3Letter, "May") == 0)
	{
		monthNumber	=	5;
	}
	else if (strcasecmp(month3Letter, "Jun") == 0)
	{
		monthNumber	=	6;
	}
	else if (strcasecmp(month3Letter, "Jul") == 0)
	{
		monthNumber	=	7;
	}
	else if (strcasecmp(month3Letter, "Aug") == 0)
	{
		monthNumber	=	8;
	}
	else if (strcasecmp(month3Letter, "Sep") == 0)
	{
		monthNumber	=	9;
	}
	else if (strcasecmp(month3Letter, "Oct") == 0)
	{
		monthNumber	=	10;
	}
	else if (strcasecmp(month3Letter, "Nov") == 0)
	{
		monthNumber	=	11;
	}
	else if (strcasecmp(month3Letter, "Dec") == 0)
	{
		monthNumber	=	12;
	}

	return(monthNumber);
}

//*****************************************************************************
//   Date       Time    Phase    Age    Diam    Dist     RA        Dec      Slon      Slat     Elon     Elat   AxisA
//01 Jan 2024 00:00 UT  78.03  19.019  1771.3  404634  10.5867   12.7508   -55.867   -1.554   0.041   -4.685   20.699
//01 Jan 2024 01:00 UT  77.71  19.061  1771.2  404664  10.6172   12.5390   -56.372   -1.554  -0.015   -4.644   20.759
//*****************************************************************************
static void	ProcessMoonPhaseArg(int argEnum, char *argBuff, TYPE_MoonPhase *moonPhaseInfo)
{
double	angle;

//	printf("%d\t%s\r\n", argEnum, argBuff);

	switch(argEnum)
	{
		case kMoonPhase_Date_DOM:
			moonPhaseInfo->Date_DOM	=	atoi(argBuff);
			break;

		case kMoonPhase_Date_Month:
			strncpy(moonPhaseInfo->MonthName, argBuff, 3);
			moonPhaseInfo->MonthName[3]	=	0;
			moonPhaseInfo->Date_Month	=	GetMonthFromString(moonPhaseInfo->MonthName);
			break;

		case kMoonPhase_Date_Year:
			moonPhaseInfo->Date_Year	=	atoi(argBuff);
			break;

		case kMoonPhase_Time_Time:
			moonPhaseInfo->Time_Hour	=	atoi(argBuff);
			break;

		case kMoonPhase_UTC:
			//*	do nothing
			break;

		case kMoonPhase_Phase:
			moonPhaseInfo->Phase	=	atof(argBuff);
			break;

		case kMoonPhase_Age:
			moonPhaseInfo->Age		=	atof(argBuff);
			break;

		case kMoonPhase_Diam:
			moonPhaseInfo->Diam		=	atof(argBuff);
			break;

		case kMoonPhase_Dist:
			moonPhaseInfo->Dist		=	atoi(argBuff);
			break;

		case kMoonPhase_RA:
			moonPhaseInfo->RA		=	atof(argBuff);
			break;

		case kMoonPhase_Dec:
			moonPhaseInfo->Dec		=	atof(argBuff);
			break;

		case kMoonPhase_Slon:
			moonPhaseInfo->SLon		=	atof(argBuff);
			break;

		case kMoonPhase_Slat:
			moonPhaseInfo->SLat		=	atof(argBuff);
			break;

		case kMoonPhase_Elon:
			moonPhaseInfo->ELon		=	atof(argBuff);
			break;

		case kMoonPhase_Elat:
			moonPhaseInfo->ELat		=	atof(argBuff);
			break;

		case kMoonPhase_AxisA:
			angle	=	atof(argBuff);
			if (angle > 180.0)
			{
				angle	=	angle - 360.0;
			}
			moonPhaseInfo->AxisA	=	angle;
			break;

	}
}

//*****************************************************************************
static void	ParseNASAmoonPhaseLine(const char *lineBuff, TYPE_MoonPhase *moonPhaseInfo)
{
int		iii;
int		ccc;
int		sLen;
int		argEnum;
char	theChar;
char	argBuff[32];

//	printf("%s\r\n", lineBuff);
	sLen	=	strlen(lineBuff);
	argEnum	=	0;
	ccc		=	0;
	iii		=	0;
	while (iii < sLen)
	{
		theChar	=	lineBuff[iii];
		if (theChar <= 0x20)
		{
			argBuff[ccc]	=	0;
			ProcessMoonPhaseArg(argEnum, argBuff, moonPhaseInfo);
			ccc	=	0;
			while (lineBuff[iii + 1] == 0x20)
			{
				iii++;
			}
			argEnum++;
		}
		else
		{
			argBuff[ccc]	=	theChar;
			ccc++;
		}
		iii++;
	}
}

//*****************************************************************************
typedef struct
{
	int		indexNum;
	double	phaseValue;
} TYPE_ValuesOfInterest;

#define	kMaxEvents	15
#define	kMaxVOIcnt	50
//*****************************************************************************
//*	this determines the phase and assigns all hours of THAT day to that value
//*	this is much more complicated than it seems.
//*	There can be one or more entries at the lowest point, same for highest point
//*****************************************************************************
static void	CalculatePhaseNamesWholeDay(void)
{
int		iii;
int		newMoonCnt;
int		fullMoonCnt;
int		currentHour;
int		jjj;
int		hhh;
double	lowestPhaseValue;
int		indexOfLowestPhaseValue;
double	highestPhaseValue;
int		indexOfHighestPhaseValue;
int		voiValueCnt;
TYPE_ValuesOfInterest	valuesOfInterest[kMaxVOIcnt];


	//--------------------------------------------------------
	//*	compute delta values
	iii	=	1;
	while (iii<gMoonPhaseCnt)
	{
		gMoonPhaseInfo[iii].PhaseDelta	=	gMoonPhaseInfo[iii].Phase - gMoonPhaseInfo[iii - 1].Phase;
		iii++;
	}

	//--------------------------------------------------------
	//*	now set the intermediate values
	//*	many of these will get over-written later
	for (iii=0; iii<gMoonPhaseCnt; iii++)
	{
		if (gMoonPhaseInfo[iii].PhaseDelta < 0.0)
		{
			strcpy(gMoonPhaseInfo[iii].PhaseName, "Waning");
		}
		else
		{
			strcpy(gMoonPhaseInfo[iii].PhaseName, "Waxing");
		}
		if (gMoonPhaseInfo[iii].Phase < 50.0)
		{
			strcat(gMoonPhaseInfo[iii].PhaseName, " crescent");
		}
		else
		{
			strcat(gMoonPhaseInfo[iii].PhaseName, " gibbous");
		}
	}

	newMoonCnt	=	0;
	fullMoonCnt	=	0;
	//------------------------------------------------------
	//*	fist go through and find all of the NEW MOON events
	//*	A new moon event is the lowest point,
	//*	detected when the values are going down and then back up.
//	CONSOLE_DEBUG_W_NUM("gMoonPhaseCnt\t=", gMoonPhaseCnt);
	iii	=	0;
	while (iii<gMoonPhaseCnt)
	{
		if (gMoonPhaseInfo[iii].Phase < 0.4)
		{
			//*	we are now going to collet the values below this
			jjj	=	0;
			while (gMoonPhaseInfo[iii].Phase < 0.4)
			{
				if (jjj < kMaxVOIcnt)
				{
					valuesOfInterest[jjj].indexNum		=	iii;
					valuesOfInterest[jjj].phaseValue	=	gMoonPhaseInfo[iii].Phase;
					jjj++;
				}
				else
				{
					CONSOLE_ABORT(__FUNCTION__);
				}
				iii++;
			}
			//*	go through the values and find the index of the lowest one
			lowestPhaseValue		=	100;
			indexOfLowestPhaseValue	=	-1;
			voiValueCnt				=	jjj;
//			CONSOLE_DEBUG_W_NUM("New moon voiValueCnt\t=", voiValueCnt);
			for (jjj=0; jjj<voiValueCnt; jjj++)
			{
				if (valuesOfInterest[jjj].phaseValue < lowestPhaseValue)
				{
					lowestPhaseValue		=	valuesOfInterest[jjj].phaseValue;
					indexOfLowestPhaseValue	=	valuesOfInterest[jjj].indexNum;
				}
			}
			if ((indexOfLowestPhaseValue >= 0) && (indexOfLowestPhaseValue < gMoonPhaseCnt))
			{
				gMoonPhaseInfo[indexOfLowestPhaseValue].IsNewMoon	=	true;
				strcpy(gMoonPhaseInfo[indexOfLowestPhaseValue].PhaseName, "New Moon");
				newMoonCnt++;
//				printf("%5d\tNew  Moon on %s %2d at %2d:00 Illumination=%5.2f\r\n", indexOfLowestPhaseValue,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].MonthName,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Date_DOM,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Time_Hour,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Phase);

				//*	now we are going to go back and set all of the hours on this day to New Moon
				currentHour	=	gMoonPhaseInfo[indexOfLowestPhaseValue].Time_Hour;
				jjj			=	indexOfLowestPhaseValue - currentHour;
				for (hhh=0; hhh<24; hhh++)
				{
					strcpy(gMoonPhaseInfo[jjj].PhaseName, "New Moon");
					jjj++;
				}
			}
		}
		iii++;
	}

	//===================================================================================
	//*	now look for FULL MOON
	iii	=	0;
	while (iii<gMoonPhaseCnt)
	{
		if (gMoonPhaseInfo[iii].Phase > 99.6)
		{
			//*	we are now going to collet the values below this
			jjj	=	0;
			while (gMoonPhaseInfo[iii].Phase > 99.6)
			{
				if (jjj < kMaxVOIcnt)
				{
					valuesOfInterest[jjj].indexNum		=	iii;
					valuesOfInterest[jjj].phaseValue	=	gMoonPhaseInfo[iii].Phase;
					jjj++;
				}
				else
				{
					CONSOLE_ABORT(__FUNCTION__);
				}
				iii++;
			}
			//*	go through the values and find the index of the highest one
			highestPhaseValue			=	0;
			indexOfHighestPhaseValue	=	-1;
			voiValueCnt					=	jjj;
//			CONSOLE_DEBUG_W_NUM("Full moon voiValueCnt\t=", voiValueCnt);
			for (jjj=0; jjj<voiValueCnt; jjj++)
			{
				if (valuesOfInterest[jjj].phaseValue > highestPhaseValue)
				{
					highestPhaseValue			=	valuesOfInterest[jjj].phaseValue;
					indexOfHighestPhaseValue	=	valuesOfInterest[jjj].indexNum;
				}
			}
			if ((indexOfHighestPhaseValue >= 0) && (indexOfHighestPhaseValue < gMoonPhaseCnt))
			{
				gMoonPhaseInfo[indexOfHighestPhaseValue].IsFullMoon	=	true;
				strcpy(gMoonPhaseInfo[indexOfHighestPhaseValue].PhaseName, "Full Moon");
				fullMoonCnt++;
//				printf("%5d\tFull Moon on %s %2d at %2d:00 Illumination=%5.2f\r\n", indexOfHighestPhaseValue,
//												gMoonPhaseInfo[indexOfHighestPhaseValue].MonthName,
//												gMoonPhaseInfo[indexOfHighestPhaseValue].Date_DOM,
//												gMoonPhaseInfo[indexOfHighestPhaseValue].Time_Hour,
//												gMoonPhaseInfo[indexOfHighestPhaseValue].Phase);

				//*	now we are going to go back and set all of the hours on this day to New Moon
				currentHour	=	gMoonPhaseInfo[indexOfHighestPhaseValue].Time_Hour;
				jjj			=	indexOfHighestPhaseValue - currentHour;
				for (hhh=0; hhh<24; hhh++)
				{
					strcpy(gMoonPhaseInfo[jjj].PhaseName, "Full Moon");
					jjj++;
				}
			}
		}
		iii++;
	}


	//===================================================================================
	//*	now look for Quarters
	iii	=	0;
	while (iii < gMoonPhaseCnt)
	{
		if ((gMoonPhaseInfo[iii].Phase > 48.0) && (gMoonPhaseInfo[iii].Phase < 52.0))
		{
			//*	we are now going to collet the values below this
			jjj	=	0;
			while ((gMoonPhaseInfo[iii].Phase > 48.0) && (gMoonPhaseInfo[iii].Phase < 52.0))
			{
				if (jjj < kMaxVOIcnt)
				{
					valuesOfInterest[jjj].indexNum		=	iii;
					valuesOfInterest[jjj].phaseValue	=	fabs(50.0 - gMoonPhaseInfo[iii].Phase);
					jjj++;
				}
				else
				{
					CONSOLE_ABORT(__FUNCTION__);
				}
				iii++;
			}
			//*	go through the values and find the index of the lowest one
			lowestPhaseValue			=	999.0;
			indexOfLowestPhaseValue		=	-1;
			voiValueCnt					=	jjj;
//			CONSOLE_DEBUG_W_NUM("Quarter moon voiValueCnt\t=", voiValueCnt);
			for (jjj=0; jjj<voiValueCnt; jjj++)
			{
				if (valuesOfInterest[jjj].phaseValue < lowestPhaseValue)
				{
					lowestPhaseValue		=	valuesOfInterest[jjj].phaseValue;
					indexOfLowestPhaseValue	=	valuesOfInterest[jjj].indexNum;
				}
			}
			if ((indexOfLowestPhaseValue >= 0) && (indexOfLowestPhaseValue < gMoonPhaseCnt))
			{
			char	quarterString[32];

				if (gMoonPhaseInfo[indexOfLowestPhaseValue].PhaseDelta > 0.0)
				{
					gMoonPhaseInfo[indexOfLowestPhaseValue].IsFirstQuater	=	true;
					strcpy(quarterString, "First Quarter");
				}
				else
				{
					gMoonPhaseInfo[indexOfLowestPhaseValue].IsThirdQuater	=	true;
					strcpy(quarterString, "Third Quarter");
				}
				strcpy(gMoonPhaseInfo[indexOfLowestPhaseValue].PhaseName, quarterString);
//				printf("%5d\tQuarter Moon on %s %2d at %2d:00 Illumination=%5.2f\r\n", indexOfLowestPhaseValue,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].MonthName,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Date_DOM,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Time_Hour,
//												gMoonPhaseInfo[indexOfLowestPhaseValue].Phase);

				//*	now we are going to go back and set all of the hours on this day to New Moon
				currentHour	=	gMoonPhaseInfo[indexOfLowestPhaseValue].Time_Hour;
				jjj			=	indexOfLowestPhaseValue - currentHour;
				for (hhh=0; hhh<24; hhh++)
				{
					strcpy(gMoonPhaseInfo[jjj].PhaseName, quarterString);
					jjj++;
				}
			}
			else
			{
				CONSOLE_DEBUG("Something is wrong!!!!!!!!!!!!!!!");
			}
		}
		iii++;
	}

	//--------------------------------------------------------------------------------------
	//*	this is not quit correct, but any phase that is above 99% I am going to label as full
	for (iii=0; iii < gMoonPhaseCnt; iii++)
	{
		if ((gMoonPhaseInfo[iii].Phase >= 99.0) && (gMoonPhaseInfo[iii].IsFullMoon == false))
		{
			if (strncmp(gMoonPhaseInfo[iii].PhaseName, "Full", 4) != 0)
			{
				strcpy(gMoonPhaseInfo[iii].PhaseName, "Full Moon~");
			}
		}
	}

//	CONSOLE_DEBUG_W_NUM("newMoonCnt \t=",	newMoonCnt);
//	CONSOLE_DEBUG_W_NUM("fullMoonCnt\t=",	fullMoonCnt);
}

#if 0
//*****************************************************************************
static void	CalculatePhaseNames(void)
{
int		iii;
double	previousPhase;

	for (iii=0; iii<gMoonPhaseCnt; iii++)
	{
		//*	figure out the phase name from the phase value
//		if (gMoonPhaseInfo[iii].Phase > 99.70)
		if (gMoonPhaseInfo[iii].Phase > 99.00)
		{
			strcpy(gMoonPhaseInfo[iii].PhaseName, "Full Moon");
		}
		else if (gMoonPhaseInfo[iii].Phase < 0.60)
		{
			strcpy(gMoonPhaseInfo[iii].PhaseName, "New Moon");
		}
		else if ((gMoonPhaseInfo[iii].Phase > 48.0) && (gMoonPhaseInfo[iii].Phase < 52.0))
		{
			if (gMoonPhaseInfo[iii].Phase > previousPhase)
			{
				strcpy(gMoonPhaseInfo[iii].PhaseName, "First Quarter");
			}
			else
			{
				strcpy(gMoonPhaseInfo[iii].PhaseName, "Third Quarter");
			}
		}
		else if (gMoonPhaseInfo[iii].Phase > 50.0)
		{
			if (gMoonPhaseInfo[iii].Phase > previousPhase)
			{
				strcpy(gMoonPhaseInfo[iii].PhaseName, "Waxing gibbous");
			}
			else
			{
				strcpy(gMoonPhaseInfo[iii].PhaseName, "Waning gibbous");
			}
		}
		previousPhase	=	gMoonPhaseInfo[iii].Phase;
	}
}
#endif // 0

//*****************************************************************************
static void	CalculateDeclinationAvg(void)
{
int		iii;
double	decDelta;
double	decDetlta_Min;
double	decDetlta_Max;
//double	decDetlta_Avg;
double	decDeltaTotal;

	decDetlta_Min	=	99.0;
	decDetlta_Max	=	0.0;
	decDeltaTotal	=	0.0;

	for (iii=1; iii<gMoonPhaseCnt; iii++)
	{
		decDelta	=	fabs(gMoonPhaseInfo[iii].Dec - gMoonPhaseInfo[iii-1].Dec);
		if (decDelta > decDetlta_Max)
		{
			decDetlta_Max	=	decDelta;
		}
		if (decDelta < decDetlta_Min)
		{
			decDetlta_Min	=	decDelta;
		}
		decDeltaTotal	+=	decDelta;
	}
//	decDetlta_Avg	=	decDeltaTotal / (gMoonPhaseCnt -1);
//	CONSOLE_DEBUG_W_DBL("decDetlta_Min\t=",	decDetlta_Min);
//	CONSOLE_DEBUG_W_DBL("decDetlta_Max\t=",	decDetlta_Max);
//	CONSOLE_DEBUG_W_DBL("decDetlta_Avg\t=",	decDetlta_Avg);
}


//*****************************************************************************
int	NASA_ReadMoonPhaseData(void)
{
FILE		*filePointer;
char		fileName[128];
char		filePath[128];
char		lineBuff[256];
int			recordCount;
int			ignoredCount;
int			currentYear;

//	CONSOLE_DEBUG(__FUNCTION__);

	memset(gMoonPhaseInfo, 0, (sizeof(TYPE_MoonPhase) * kMoonPhaseRecCnt));
	currentYear	=	GetCurrentYear();

	sprintf(fileName, "mooninfo_%4d.txt", currentYear);
	CONSOLE_DEBUG_W_NUM("currentYear\t=", currentYear);
	CONSOLE_DEBUG_W_STR("fileName   \t=", fileName);
//	CONSOLE_ABORT(__FUNCTION__);

	strcpy(filePath, kNASAmoonPhaseDir);
//	strcat(filePath, "mooninfo_2024.txt");
	strcat(filePath, fileName);

	recordCount		=	0;
	ignoredCount	=	0;
	filePointer		=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer) && (recordCount < 9000))
		{
			if (isdigit(lineBuff[0]) && isdigit(lineBuff[1]) && (lineBuff[2] == 0x20))
			{
				ParseNASAmoonPhaseLine(lineBuff, &gMoonPhaseInfo[recordCount]);
				recordCount++;
			}
			else
			{
//				printf("%s\r\n", lineBuff);
				ignoredCount++;
			}
		}
//		CONSOLE_DEBUG_W_NUM("recordCount \t=",	recordCount);
//		CONSOLE_DEBUG_W_NUM("ignoredCount\t=",	ignoredCount);

		gMoonPhaseCnt	=	recordCount;
//		CalculatePhaseNames();
		CalculatePhaseNamesWholeDay();
		CalculateDeclinationAvg();
	}
	else
	{
		CONSOLE_DEBUG_W_STR("NASA Moon Phase info not found, looking for:", filePath);
	}
	return(gMoonPhaseCnt);
}

//*****************************************************************************
static int	FileNameQSort(const void *e1, const void* e2)
{
int					retValue;
TYPE_MoonInfoFile	*entry1;
TYPE_MoonInfoFile	*entry2;

	entry1		=	(TYPE_MoonInfoFile *)e1;
	entry2		=	(TYPE_MoonInfoFile *)e2;
	retValue	=	strcmp(entry1->FileName, entry2->FileName);
//	if (retValue == 0)
//	{
//		CONSOLE_DEBUG_W_2STR("duplicate:", entry1->FileName, entry2->FileName);
//		CONSOLE_ABORT(__FUNCTION__);
//	}
	return(retValue);
}

//*****************************************************************************
int	NASA_ReadMoonPhaseDirectory(void)
{
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
char			curFileName[128];
int				fileIndex;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, kNASAmoonPhaseDir);
	memset(gMoonInfoFileList, 0, (sizeof(TYPE_MoonInfoFile) * kMoonFileMax));

	fileIndex	=	0;
	directory	=	opendir(kNASAmoonPhaseDir);
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
//			CONSOLE_DEBUG("----------------------------------");
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				strcpy(curFileName, dir->d_name);
				if (strncmp(curFileName, "mooninfo", 8) == 0)
				{
//					CONSOLE_DEBUG(curFileName);
					if (fileIndex < kMoonFileMax)
					{
						strcpy(gMoonInfoFileList[fileIndex].FileName,	curFileName);
						fileIndex++;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Ran out of room, cnt\t=",	fileIndex);
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		closedir(directory);
		//-----------------------------------------------------------------
		//*	if there are more than one, sort them so there is consistency.
		//*	accessing the file path does not always guarantee the same order
		if  (fileIndex > 1)
		{
			qsort(gMoonInfoFileList, fileIndex, sizeof(TYPE_MoonInfoFile), FileNameQSort);
		}
//		CONSOLE_DEBUG("----------------------------------");
//		for (iii=0; iii<fileIndex; iii++)
//		{
//			CONSOLE_DEBUG(gMoonInfoFileList[iii].FileName);
//		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open directory\t=",	kNASAmoonPhaseDir);
	}
	return(fileIndex);

}

//*****************************************************************************
int	NASA_GetMoonImageCount(int year)
{
int				imageCount;
char			imageDirPath[64];
int				myYear;
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;

//	CONSOLE_DEBUG(__FUNCTION__);

	imageCount	=	0;
	if (year < 2011)
	{
		myYear	=	2024;
	}
	sprintf(imageDirPath, "%smoon%04d/", kNASAmoonPhaseDir, myYear);

//	CONSOLE_DEBUG(imageDirPath);

	directory	=	opendir(imageDirPath);
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				if (strncmp(dir->d_name, "moon.", 5) == 0)
				{
					imageCount++;
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		closedir(directory);
	}
//	CONSOLE_DEBUG_W_NUM("imageCount\t=", imageCount);

	return(imageCount);
}

//*****************************************************************************
//*	returns true if the file exists
//*****************************************************************************
bool	NASA_GetMoonImageFilePath(int year, int month, int day, int hour, char *imagePath, char *imageFileName)
{
int			phaseIndex;
int			imageNumber;
char		yearString[32];
struct stat	fileStatus;
int			returnCode;
bool		fileExists	=	false;

	phaseIndex	=	NASA_GetPhaseIndex(year, month, day, hour);
//	CONSOLE_DEBUG_W_NUM("phaseIndex\t=", phaseIndex);
	if (phaseIndex >= 0)
	{
		sprintf(yearString, "moon%04d/", year);

		imageNumber	=	phaseIndex + 1;
		sprintf(imageFileName, "moon.%04d.jpg", imageNumber);
//		CONSOLE_DEBUG_W_STR("imageFileName\t=", imageFileName);

		strcpy(imagePath, kNASAmoonPhaseDir);
		strcat(imagePath, yearString);
		strcat(imagePath, imageFileName);
		returnCode	=	stat(imagePath, &fileStatus);
		if (returnCode == 0)
		{
			fileExists	=	true;
		}
	}
	return(fileExists);
}

//**************************************************************************************
static void NASA_CheckDirectories(int year)
{
struct stat	fileStatus;
int			returnCode;
mode_t		dirPermissions	=	0755;
char		subDirString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("year\t=", year);
	//*	check to make sure the directory is there
	returnCode	=	stat(kNASAmoonPhaseDir, &fileStatus);		//*	fstat - check for existence of file
	if (returnCode == 0)
	{
		//*	the directory exists
	}
	else
	{
		returnCode	=	mkdir(kNASAmoonPhaseDir, dirPermissions);
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	if (year > 2000)
	{
//		CONSOLE_DEBUG("Creating sub directory");

		//*	now check for the sub directory for the current year
		sprintf(subDirString, "%smoon%04d", kNASAmoonPhaseDir, year);
		CONSOLE_DEBUG_W_STR("subDirString\t=", subDirString);
		returnCode	=	stat(subDirString, &fileStatus);		//*	fstat - check for existence of file
		if (returnCode == 0)
		{
			//*	the directory exists
		}
		else
		{
			returnCode	=	mkdir(subDirString, dirPermissions);
			if (returnCode == 0)
			{
				CONSOLE_DEBUG_W_STR("Directory created OK\t=", subDirString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Directory failed\t=", subDirString);
				CONSOLE_ABORT(__FUNCTION__);
			}
		}
	}
}


static	pthread_t	gNASAdownload_ThreadID;
static	bool		gNASAdownloadThreadIsRunning	=	false;
static	int			gDownLoadYear					=	2000;
static	char		gLatestDownloadFile[32]			=	"";
//*****************************************************************************
//*	/home/mark/dev-mark/alpaca/NASA_MoonInfo/moon2024/moon.2140.jpg
//*	https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/frames/730x730_1x1_30p/moon.2141.jpg
//*****************************************************************************
static void	*NASA_DownloadImageThread(void *arg)
{
char		imageFileName[64];
char		imagePath[128];
char		yearString[32];
char		commandString[256];
int			imageNumber;
bool		keepGoing;
struct stat	fileStatus;
int			returnCode;
int			systemRetCode;
//char		urlString[]	=	"https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/frames/730x730_1x1_30p/";

char		urlString[]	=	"https://svs.gsfc.nasa.gov/vis/a000000/a005400/a005415/frames/730x730_1x1_30p/";	//*	north up
//char		urlString[]	=	"https://svs.gsfc.nasa.gov/vis/a000000/a005400/a005416/frames/730x730_1x1_30p/";	//*	south up

//	CONSOLE_DEBUG(__FUNCTION__);

	gNASAdownloadThreadIsRunning	=	true;
	NASA_CheckDirectories(gDownLoadYear);

	sprintf(yearString, "moon%04d/", gDownLoadYear);
	imageNumber	=	1;
	keepGoing	=	true;
	while (keepGoing && (imageNumber < (367 * 24)))
	{
		sprintf(imageFileName, "moon.%04d.jpg", imageNumber);
		strcpy(imagePath, kNASAmoonPhaseDir);
		strcat(imagePath, yearString);
		strcat(imagePath, imageFileName);
		returnCode	=	stat(imagePath, &fileStatus);
		if (returnCode == 0)
		{
			CONSOLE_DEBUG_W_STR("File exists ", imagePath);
		}
		else
		{
			//*	the file does not exist
			strcpy(commandString, "cd NASA_MoonInfo/");
			strcat(commandString, yearString);
			strcat(commandString, "/;wget ");
			strcat(commandString, urlString);
			strcat(commandString, imageFileName);
			strcat(commandString, " 2>/dev/null");
//			printf("%s\r\n", commandString);
//			CONSOLE_DEBUG(commandString);
			systemRetCode	=	system(commandString);
			if (systemRetCode == 0)
			{
				CONSOLE_DEBUG_W_STR("Downloaded ", imageFileName);
				strcpy(gLatestDownloadFile, imageFileName);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
				CONSOLE_DEBUG_W_STR("commandString was:", commandString);
				keepGoing	=	false;
			}
			sleep(2);
//			usleep(1 * 1000);
		}
		imageNumber++;
	}
	CONSOLE_DEBUG("NASA Image Download thread exiting!!!!!!!!!!!!");

	gNASAdownloadThreadIsRunning	=	false;
	return(NULL);
}

//*****************************************************************************
//*	returns true if download thread is running
bool	NASA_GetLatestDownLoadImageName(char *imageName)
{
	if (gNASAdownloadThreadIsRunning)
	{
		strcpy(imageName, gLatestDownloadFile);
	}
	return(gNASAdownloadThreadIsRunning);
}

//*****************************************************************************
void	NASA_StartMoonImageDownloadThread(const int year)
{
int		threadErr;

//	CONSOLE_DEBUG(__FUNCTION__);

	gDownLoadYear	=	year;
	if (gNASAdownloadThreadIsRunning == false)
	{

		threadErr	=	pthread_create(	&gNASAdownload_ThreadID,
										NULL,
										&NASA_DownloadImageThread,
										NULL);
		if (threadErr == 0)
		{
			CONSOLE_DEBUG("NASA_DownloadImageThread created successfully");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
		}
	}
	else
	{
		CONSOLE_DEBUG("Thread already running!!!!");
	}
}

//**************************************************************************************
static void	NASA_DownloadOneMoonPhaseFile(const char *moonPhaseURL)
{

int			systemRetCode;
char		commandString[256];
char		fileName[256];
char		filePath[256];
char		*slashPtr;
struct stat	fileStatus;
int			returnCode;

	slashPtr	=	strchr((char *)moonPhaseURL, '/');
	while (slashPtr != NULL)
	{
		slashPtr++;
		strcpy(fileName, slashPtr);
		slashPtr	=	strchr(fileName, '/');
//		CONSOLE_DEBUG(fileName);
	}
	strcpy(filePath, kNASAmoonPhaseDir);
	strcat(filePath, fileName);
	returnCode	=	stat(filePath, &fileStatus);		//*	fstat - check for existence of file
	if (returnCode == 0)
	{
		//*	file already exists
		CONSOLE_DEBUG_W_STR("File already exists:", filePath);
	}
	else
	{

	//	strcpy(commandString, "cd NASA_MoonInfo/;wget ");
		strcpy(commandString, "cd ");
		strcat(commandString, kNASAmoonPhaseDir);
		strcat(commandString, ";wget ");
		strcat(commandString, moonPhaseURL);
		strcat(commandString, " 2>/dev/null");
		CONSOLE_DEBUG_W_STR("commandString:", commandString);
		systemRetCode	=	system(commandString);
		if (systemRetCode == 0)
		{
			CONSOLE_DEBUG_W_STR("Downloaded ", commandString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
			CONSOLE_DEBUG_W_STR("ERROR system() returned", strerror(systemRetCode));
		}
	}
}

//**************************************************************************************
void	NASA_DownloadMoonPhaseData(void)
{

	//*	check to make sure the directory is there
	NASA_CheckDirectories(0);
	NASA_DownloadOneMoonPhaseFile("https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/mooninfo_2024.txt");
	NASA_DownloadOneMoonPhaseFile("https://svs.gsfc.nasa.gov/vis/a000000/a005400/a005415/mooninfo_2025.txt");


	NASA_ReadMoonPhaseData();
//	CONSOLE_ABORT(__FUNCTION__);
}



#ifndef _ALPACA_PI_

//*****************************************************************************
int	NASA_GetMoonImageFiles(int year, int firstIndex, int retrieveCnt)
{
char		imageFileName[64];
char		imagePath[128];
char		yearString[32];
char		commandString[256];
int			imageNumber;
int			imageCount;
bool		keepGoing;
struct stat	fileStatus;
int			returnCode;
int			systemRetCode;

//	/home/mark/dev-mark/alpaca/NASA_MoonInfo/moon2024/moon.2140.jpg
//	https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/frames/730x730_1x1_30p/moon.2141.jpg
	sprintf(yearString, "moon%04d/", year);
	imageNumber	=	firstIndex;
	imageCount	=	0;
	keepGoing	=	true;
	while (keepGoing && (imageCount < retrieveCnt))
	{
		sprintf(imageFileName, "moon.%04d.jpg", imageNumber);
		strcpy(imagePath, kNASAmoonPhaseDir);
		strcat(imagePath, yearString);
		strcat(imagePath, imageFileName);
		returnCode	=	stat(imagePath, &fileStatus);
		if (returnCode == 0)
		{
			CONSOLE_DEBUG_W_STR("File exists ", imagePath);
		}
		else
		{
			//*	the file does not exist
			strcpy(commandString, "cd ");
			strcat(commandString, kNASAmoonPhaseDir);
			strcat(commandString, yearString);
			strcat(commandString, ";wget https://svs.gsfc.nasa.gov/vis/a000000/a005100/a005187/frames/730x730_1x1_30p/");
			strcat(commandString, imageFileName);
			strcat(commandString, " 2>/dev/null");
//			printf("%s\r\n", commandString);
			systemRetCode	=	system(commandString);
			if (systemRetCode == 0)
			{
				CONSOLE_DEBUG_W_STR("Downloaded ", imageFileName);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
				keepGoing	=	false;
			}
//			sleep(1);
			usleep(1 * 1000);
		}
		imageNumber++;
		imageCount++;
	}
	return(imageCount);
}


//*****************************************************************************
static void	DumpMoonPhaseRecord(TYPE_MoonPhase *moonPhaseInfo)
{
	printf("-------------------------\r\n");
	printf("Year    \t=%d\r\n",	moonPhaseInfo->Date_Year);
	printf("Month   \t=%s\r\n",	moonPhaseInfo->MonthName);
	printf("DOM     \t=%d\r\n",	moonPhaseInfo->Date_DOM);
	printf("Hour    \t=%d\r\n",	moonPhaseInfo->Time_Hour);
	printf("Phase   \t=%f\r\n",	moonPhaseInfo->Phase);
	printf("Age     \t=%f\r\n",	moonPhaseInfo->Age);
	printf("Phase   \t=%s\r\n",	moonPhaseInfo->PhaseName);
}

//*****************************************************************************
static void	PrintMoonEvents(void)
{
int		iii;

	for (iii=0; iii<gMoonPhaseCnt; iii++)
	{
//		if (gMoonPhaseInfo[iii].IsNewMoon ||
//			gMoonPhaseInfo[iii].IsFullMoon ||
//			gMoonPhaseInfo[iii].IsFirstQuater ||
//			gMoonPhaseInfo[iii].IsThirdQuater)
		if (gMoonPhaseInfo[iii].IsNewMoon)
		{
			printf("%5d\tOn %s %2d at %2d:00 Illumination=%5.2f %s\r\n",
											iii,
											gMoonPhaseInfo[iii].MonthName,
											gMoonPhaseInfo[iii].Date_DOM,
											gMoonPhaseInfo[iii].Time_Hour,
											gMoonPhaseInfo[iii].Phase,
											gMoonPhaseInfo[iii].PhaseName);
		}
	}
}


//*****************************************************************************
//*	Find Max and Mins
//*****************************************************************************
static void	FindExtreams(void)
{
double	maxIllum;
double	minIllum;
int		iii;

	maxIllum	=	0.0;
	minIllum	=	999.0;
	for (iii=0; iii<gMoonPhaseCnt; iii++)
	{
		if (gMoonPhaseInfo[iii].Phase > maxIllum)
		{
			maxIllum	=	gMoonPhaseInfo[iii].Phase;
		}
		if (gMoonPhaseInfo[iii].Phase < minIllum)
		{
			minIllum	=	gMoonPhaseInfo[iii].Phase;
//			DumpMoonPhaseRecord(&gMoonPhaseInfo[iii]);
		}
	}
	CONSOLE_DEBUG_W_DBL("maxIllum\t=",	maxIllum);
	CONSOLE_DEBUG_W_DBL("minIllum\t=",	minIllum);
}


//*****************************************************************************
void	TestInteroplation(void)
{
int				day;
int				hour;
int				minuute;
TYPE_MoonPhase	moonPhaseInfo;
int				count;
bool			validInfo;

//	CONSOLE_DEBUG(__FUNCTION__);
	count	=	0;
	for (day=1; day<=31; day++)
	{
		for (hour=0; hour<24; hour++)
		{
			for (minuute=0; minuute<60; minuute++)
			{
//				printf("D=%2d, H=%2d, M=%2d\r\n", day, hour, minuute);
				validInfo	=	NASA_GetMoonPhaseInfo(	2024,		//*	year
														1,			//*	month
														day,		//*	day
														hour,		//*	hour
														minuute,	//*	minute
														0,			//*	seconds
														&moonPhaseInfo);
//				CONSOLE_DEBUG_W_BOOL("validInfo\t=", validInfo);
				printf("Y=%4d M=%s DOM=%2d T=%2d:%02d \tPhase=\t%5.4f\tAge=\t%5.4f\r\n",
												moonPhaseInfo.Date_Year,
												moonPhaseInfo.MonthName,
												moonPhaseInfo.Date_DOM,
												moonPhaseInfo.Time_Hour,
												minuute,
												moonPhaseInfo.Phase,
												moonPhaseInfo.Age);
				count++;
			}
		}
	}
	printf("Count=%d\r\n", count);
}
//*****************************************************************************
void	TestDateCalculation(void)
{
int		dayOfTheYear;
int		phaseIndex;
int		year	=	2024;
int		month	=	1;
int		day		=	1;
int		hour	=	0;
char	imageFileName[256];
char	imagePath[256];

	for (day=1; day<=2; day++)
	{
		for (hour=0; hour<23; hour++)
		{
			dayOfTheYear	=	NASA_GetDayOfYear(year, month, day);
			phaseIndex		=	NASA_GetPhaseIndex(year, month, day, hour);
			CONSOLE_DEBUG_W_NUM("dayOfTheYear\t=",	dayOfTheYear);
			CONSOLE_DEBUG_W_NUM("phaseIndex  \t=",	phaseIndex);
			NASA_GetMoonImageFilePath(year, month, day, hour, imagePath, imageFileName);
		}
	}
}

//*****************************************************************************
int main(int argc, char *argv[])
{
int				iii;
double			myPhase;
TYPE_MoonPhase	moonPhaseInfo;

	printf("NASA moon info test\r\n");
	NASA_ReadMoonPhaseData();
//	TestInteroplation();
//	exit(0);
//	iii	=	0;
//	while (iii<(365 * 24))
////	while (strcmp(gMoonPhaseInfo[iii].MonthName, "Jan") == 0)
////	while (iii< (5 * 31 * 24))
//	{
////		if (strcmp(gMoonPhaseInfo[iii].MonthName, "May") == 0)
//		{
//			printf("%5d-%4d %s %2d %2d-%5.2f:",	iii,
//												gMoonPhaseInfo[iii].Date_Year,
//												gMoonPhaseInfo[iii].MonthName,
//												gMoonPhaseInfo[iii].Date_DOM,
//												gMoonPhaseInfo[iii].Time_Hour,
//												gMoonPhaseInfo[iii].Phase);
//			myPhase	=	0;
//			while(myPhase < gMoonPhaseInfo[iii].Phase)
//			{
//				printf(" ");
//				myPhase	+=	1;
//			}
//			printf("* %s\r\n", gMoonPhaseInfo[iii].PhaseName);
//		}
//		iii	+=	1;
//	}
//	exit(0);

//	NASA_GetMoonPhaseInfo(2024, 3, 29, 3, 0, 0, &moonPhaseInfo);
//	exit(0);
//	DumpMoonPhaseRecord(&moonPhaseInfo);
//	for (iii=0; iii<=60; iii++)
//	{
//		NASA_GetMoonPhaseInfo(2024, 3, 25, 10, iii, 0, &moonPhaseInfo);
//		printf("%4d %s %2d %2d\tPhase=\t%5.4f\tAge=\t%5.4f\r\n",
//										moonPhaseInfo.Date_Year,
//										moonPhaseInfo.MonthName,
//										moonPhaseInfo.Date_DOM,
//										moonPhaseInfo.Time_Hour,
//										moonPhaseInfo.Phase,
//										moonPhaseInfo.Age);
//	}
//	FindExtreams();
//	PrintMoonEvents();

	TestDateCalculation();

//	NASA_GetMoonImageFiles(2024, 8500, 1000);
}

#endif // _ALPACA_PI_


//   Date       Time    Phase    Age    Diam    Dist     RA        Dec      Slon      Slat     Elon     Elat   AxisA
//28 Mar 2024 02:00 UT  93.02  17.708  1791.4  400085  14.3426  -16.1964   -35.543   -0.241  -5.025    2.635   17.547
//28 Mar 2024 03:00 UT  92.81  17.750  1791.9  399971  14.3749  -16.4012   -36.050   -0.240  -5.069    2.690   17.435

//28	Mar	2024	02:00	UT	93.02	17.708	1791.4	400085	14.3426	-16.1964	-35.543	-0.241	-5.025	2.635	17.547
//28	Mar	2024	03:00	UT	92.81	17.750	1791.9	399971	14.3749	-16.4012	-36.050	-0.240	-5.069	2.690	17.435


//FILENAME= 'TEST-2024-03-28T02_13_34.976-ZWO-BCF-H.fits' / Orig filename

//COMMENT = '-------------------------------------Moon Info----------------------'
//COMMENT = 'Moon Phase info from NASA'
//COMMENT = 'https://svs.gsfc.nasa.gov/5187'
//MOONPHAS= 'Waning gibbous'     / Current Moon Phase
//MOONILUM=              92.9745 / Percent illumination
//MOONAGE =              17.7171 / Number of days since new moon
//MOONDIAM=     1791.50833333333 / Diameter of the moon (arcseconds)
//MOONDIST=               400060 / Distance to moon (km)
//MOON-RA =              14.3426 / Right Ascension of moon
//MOON-DEC=             -16.1964 / Declination of moon
//MOONAXIS=               17.547 / North Polar Axis Angle
//MOONVIS =                    T / Moon is visible
//COMMENT = 'Note: These times are local time zone NOT UTC'
//COMMENT = 'Previous moon event:'
//MOONPREV= 'Moon rise at Wed Mar 27 21:57:57 2024, Azimuth 112.10'
//COMMENT = 'Next moon event:'
//MOONNEXT= 'Moon set at Thu Mar 28 08:07:08 2024, Azimuth 245.13'


