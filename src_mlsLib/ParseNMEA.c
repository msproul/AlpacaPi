//**************************************************************************************
//*	General purpose GPS processing code
//*
//*	(C) Mark L. Sproul & Keith Sproul
//*	Written by Mark L. Sproul
//*
//**************************************************************************************
//*	Edit History
//**************************************************************************************
//*	Apr 14,	1996	<KAS> Put the boxes for the Heads-Up-Display into a Dialog Box template
//*	Apr 19,	1996	<KAS> Separated NMEA and APRS Parse Routines
//*	Dec 11,	1996	<KAS> Removed UpdateHUDdisplay to a different file
//*	Dec 26,	1996	<KAS> Dave VanHorn dvanhron@cedar.net
//*	Dec 26,	1996	<KAS> PNI Compass, same as Jameco
//*	Dec 26,	1996	<KAS> Talked them into RAW data output
//*	Feb  6,	2001	<MLS> ReturnNMEAAltitude - Modified to use ATOF instead of ATOL
//*	Jan 12,	2002	<KAS> Started adding Magellan Proprietary Sentenses
//*	May 20,	2002	<KAS> Started working on adaptive beaconing and corner pegging
//*	Sep 16,	2004	<MLS> Added support for $GPNVP for special Korean GPS, requested by customer
//*	Nov 17,	2004	<MLS> Started separating out NMEA parsing into smaller more manageable routines
//*	Apr 30,	2012	<MLS> Started on GPS support.
//*	Apr 30,	2012	<MLS> GPS parsing code implemented
//*	Sep 11,	2012	<MLS> Added Get_Actual_LatLon_Strings() for GPS display formating
//*	Nov 21,	2012	<MLS> Added GetCurrentLatLon_Strings() for metadata saving
//*	Dec  5,	2012	<MLS> Check for max snr value from satellite GSV string
//*	Dec  5,	2012	<MLS> Added GetMaxSatSignalStrength()
//*	Jan  2,	2013	<MLS> Added LatLonType argument to ReturnNMEALatLon()
//*	Mar 12,	2013	<MLS> Added support for seperate test program,
//*	Mar 12,	2013	<MLS> Fixed lat/lon bug in ReturnNMEALatLon (bug was introduced 1/2/13)
//*	Oct  3,	2016	<MLS> Updating to be a general purpose GPS parser
//*	Oct  4,	2016	<MLS> Moved checksum routines to helper file
//*	Oct  6,	2016	<MLS> Updated ReturnNMEAAltitude() to check for valid altitude
//*	Oct  6,	2016	<MLS> Updated ReturnNMEASpeed() to check for valid speed
//*	Oct 10,	2016	<MLS> Added ParseNMEA_init()
//*	Apr 17,	2017	<MLS> Moved GPS code to seperate libery directroy. I need this for many projects
//*	Apr 18,	2017	<MLS> Removing MacAPRS specfic code
//*	Apr 20,	2017	<MLS> Changing sat snr data from char strings to shorts (int)
//*	Apr 20,	2017	<MLS> Cleaned up $GPGSV paring code
//*	Apr 23,	2017	<MLS> Added $PRWIZCH, Rockwell channel data
//*	Apr 24,	2017	<MLS> Added checksum error count
//*	Apr 25,	2017	<MLS> Added _ENABLE_PDOP_TRACKING_
//*	May  3,	2017	<MLS> Added _ENABLE_ALTITUDE_TRACKING_
//*	May  4,	2017	<MLS> Added parsing for ($PMGLF), no documentation on what it is
//*	May  4,	2017	<MLS> Added parsing for GPS sensor temp ($PGRMT)
//*	May 10,	2017	<MLS> Altitude values now in double as well as int
//*	May 10,	2017	<MLS> Added lat lon tracking as double values
//*	May 10,	2017	<MLS> Added _ENABLE_LAT_LON_TRACKING_
//*	May 11,	2017	<MLS> Added ParseNMEA_TimeString()
//*	May 12,	2017	<MLS> Setting system time working on Linux
//*	May 12,	2017	<MLS> Added Check_And_Set_System_Time()
//*	May 15,	2017	<MLS> Added Sats in use tracking
//*	May 15,	2017	<MLS> Added Mode tracking (1=no fix,2=2D,3=3D)
//*	May 15,	2017	<MLS> Added Magnetic variation tracking
//*	May 25,	2017	<MLS> Added parsing of $PRWIZCH sat signal data
//*	Sep  5,	2023	<MLS> Removing global gNMEAdata, replacing with pointer
//*	Sep  9,	2023	<MLS> Added GetLatLonDouble()
//*	Sep  9,	2023	<MLS> Added ParseNMEA_FormatLatLonStrings()
//**************************************************************************************

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>

#if defined(__unix__)
	#include	<error.h>
	#include	<errno.h>
	#include	<time.h>
	#include	<sys/time.h>
#endif


//#define	_ENABLE_GPS_DEBUGGING_
//#define	_ENABLE_TIME_DEGBUG_


#include	"ParseNMEA.h"
#include	"NMEA_helper.h"

#ifdef _ENABLE_GPS_DEBUGGING_
	#define	_ENABLE_CONSOLE_DEBUG_
#endif

#define	_SHOW_CHECKSUM_ERRORS_

//*	MLS Libraries
#define	_ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//TYPE_NMEAInfoStruct	gNMEAdata;

#define	kMaxNumNmeaArgs		32
#define	kNMEAargLen			32

#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_
	static void	NMEAtrack_Update(const unsigned long nmeaCode, const char *nmeaID, const char *fullString);
#endif

static int				gMaxSatelliteSNRvalue	=	0;
static unsigned long	gNMEAcheckSumErrCnt		=	0;
static unsigned long	gUnknownNEMAcount		=	0;

#ifdef _ENABLE_TIME_DEGBUG_
	char	gLastNMEAdata[512];
#endif

//**************************************************************************************
//*	this is done as a stucture so it can be easily passed to routines instead of being a global
typedef	struct
{
	char			argString[kNMEAargLen];
}	TYPE_NMEAargs;

//**************************************************************************************
void	ParseNMEA_init(TYPE_NMEAInfoStruct *nmeaData)
{
	nmeaData->theNN.Lat[0]		=	0;
	nmeaData->theNN.LatC[0]		=	0;		//*	latitude
	nmeaData->theNN.Lon[0]		=	0;
	nmeaData->theNN.LonC[0]		=	0;		//*	longitude
	nmeaData->theNN.Alt[0]		=	0;		//*	Altitude (in meters)
	nmeaData->theNN.AltC[0]		=	0;		//*	Unit Character M or F
	nmeaData->theNN.Sat[0]		=	0;		//*	Satilite Flag
	nmeaData->theNN.NumSats[0]	=	0;		//*	Number of Satalites
	nmeaData->theNN.Date[0]		=	0;		//*	date
	nmeaData->theNN.Time[0]		=	0;		//*	time

	nmeaData->theNN.HDOP[0]		=	0;
	nmeaData->theNN.VDOP[0]		=	0;
	nmeaData->theNN.PDOP[0]		=	0;
	nmeaData->theNN.TDOP[0]		=	0;
	nmeaData->theNN.GDOP[0]		=	0;

}

//**************************************************************************************
double	GetLatLonDouble(LatLonType *latLonPtr)
{
double	latLonDbl;

	latLonDbl	=	latLonPtr->Deg;
	latLonDbl	+=	latLonPtr->Min / 60.0;
	latLonDbl	+=	latLonPtr->Sec / 3600.0;
	latLonDbl	+=	latLonPtr->Ten / 36000.0;

	if ((latLonPtr->Ch == 'W') || (latLonPtr->Ch == 'E'))
	{
		latLonDbl	=	-latLonDbl;
	}
//	CONSOLE_DEBUG_W_DBL("latLonDbl\t=", latLonDbl);

	return(latLonDbl);
}

//**************************************************************************************
void	ParseNMEA_FormatLatLonStrings(double latValue, char *latString, double lonValue, char *lonString)
{
int		degrees;
int		minutes;
double	seconds;
char	signChar;
double	myLatLonValue;
int		myLatLonValueMinutes;

	//*	first do the lat
	if (latValue < 0)
	{
		signChar			=	'S';
		myLatLonValue		=	-latValue;
	}
	else
	{
		signChar			=	'N';
		myLatLonValue		=	latValue;
	}
	degrees					=	myLatLonValue;
	myLatLonValueMinutes	=	myLatLonValue * 60;
	minutes					=	myLatLonValueMinutes % 60;
	seconds					=	(myLatLonValue * 3600) - (degrees * 3600) - (minutes * 60);

	sprintf(latString, "%03d:%02d:%05.3f %c", degrees, minutes, seconds, signChar);
//	CONSOLE_DEBUG_W_STR("latString\t=", latString);

	//*	now do the lon
	if (lonValue < 0)
	{
		signChar			=	'W';
		myLatLonValue		=	-lonValue;
	}
	else
	{
		signChar			=	'E';
		myLatLonValue		=	lonValue;
	}
	degrees					=	myLatLonValue;
	myLatLonValueMinutes	=	myLatLonValue * 60;
	minutes					=	myLatLonValueMinutes % 60;
	seconds					=	(myLatLonValue * 3600) - (degrees * 3600) - (minutes * 60);

	sprintf(lonString, "%03d:%02d:%05.3f %c", degrees, minutes, seconds, signChar);
//	CONSOLE_DEBUG_W_STR("lonString\t=", lonString);
}


//#ifdef _ENABLE_GPS_DEBUGGING_
//**************************************************************************************
void	DumpGPSdata(TYPE_NMEAInfoStruct	*theNmeaInfo)
{
	if (theNmeaInfo != NULL)
	{
		printf("Valid            = %s\n",		(theNmeaInfo->validData		? "YES" : "NO"));
		printf("Valid time       = %s\n",		(theNmeaInfo->validTime		? "YES" : "NO"));
		printf("Valid date       = %s\n",		(theNmeaInfo->validDate		? "YES" : "NO"));
		printf("Valid Lat/Lon    = %s\n",		(theNmeaInfo->validLatLon	? "YES" : "NO"));
		printf("Valid alt        = %s\n",		(theNmeaInfo->validAlt		? "YES" : "NO"));
		printf("Valid validCseSpd= %s\n",		(theNmeaInfo->validCseSpd	? "YES" : "NO"));
		printf("Lat              = %s %s\n",	theNmeaInfo->theNN.Lat, theNmeaInfo->theNN.LatC);
		printf("Lon              = %s %s\n",	theNmeaInfo->theNN.Lon, theNmeaInfo->theNN.LonC);
		printf("Alt              = %s %s\n",	theNmeaInfo->theNN.Alt, theNmeaInfo->theNN.AltC);

		printf("Lat (parsed)     = %03ld:%02ld:%02ld.%02ld %c\n",	theNmeaInfo->latitude.Deg,
																	theNmeaInfo->latitude.Min,
																	theNmeaInfo->latitude.Sec,
																	theNmeaInfo->latitude.Ten,
																	theNmeaInfo->latitude.Ch);

		printf("Lon (parsed)     = %03ld:%02ld:%02ld.%02ld %c\n",	theNmeaInfo->longitude.Deg,
																	theNmeaInfo->longitude.Min,
																	theNmeaInfo->longitude.Sec,
																	theNmeaInfo->longitude.Ten,
																	theNmeaInfo->longitude.Ch);


		printf("Sat              = %s\n",		theNmeaInfo->theNN.Sat);
		printf("NumSats          = %s\n",		theNmeaInfo->theNN.NumSats);
		printf("Date             = %s\n",		theNmeaInfo->theNN.Date);
		printf("Time             = %s\n",		theNmeaInfo->theNN.Time);
		printf("HDOP             = %s\n",		theNmeaInfo->theNN.HDOP);
		printf("VDOP             = %s\n",		theNmeaInfo->theNN.VDOP);
		printf("PDOP             = %s\n",		theNmeaInfo->theNN.PDOP);
		printf("TDOP             = %s\n",		theNmeaInfo->theNN.TDOP);
		printf("GDOP             = %s\n",		theNmeaInfo->theNN.GDOP);
	}
}

////**************************************************************************************
//static void	DumpNMEAargs(TYPE_NMEAargs *nmeaArgs)
//{
//int	ii;
//
//	for (ii=0; ii<kMaxNumNmeaArgs; ii++)
//	{
//		printf("arg[%2d]         = %s\n",		ii, nmeaArgs[ii].argString);
//	}
//}


//#endif

#ifdef _ENABLE_SATELLITE_ALMANAC_
//**************************************************************************************
int	GetSatSignalStrength(TYPE_NMEAInfoStruct *nmeaData, int satNumber)
{
int	snrValue;

	snrValue	=	0;
	if ((satNumber >= 0) && (satNumber < kMaxNumOfSatallites))
	{
		snrValue	=	nmeaData->theSats[satNumber].signal2Noise;
	}
	else
	{
	#ifdef _ENABLE_GPS_DEBUGGING_
		CONSOLE_DEBUG("Sat number out of range");
	#endif
	}
	return(snrValue);
}
#endif

//**************************************************************************************
int	GetMaxSatSignalStrength(void)
{
	return(gMaxSatelliteSNRvalue);
}




//**************************************************************************************
//*	returns number of args
//**************************************************************************************
static short	SeparateNMEAline(char *theLine, TYPE_NMEAargs	*nmeaArgs)
{
short			ii,jj,kk,theLen;

	//*	clear out all of the old values
	for (ii=0; ii<kMaxNumNmeaArgs; ii++)
	{
		memset(nmeaArgs[ii].argString, 0, kNMEAargLen);
	}

	ii	=	0;
	jj	=	0;
	theLen	=	strlen(theLine);
	while ((ii < theLen) && (jj<kMaxNumNmeaArgs))
	{
		kk	=	0;
		while  ((theLine[ii] != ',') &&
				(theLine[ii] != '*') &&
				(theLine[ii] != 0x0d) &&
				(theLine[ii] != 0x0a) &&
				(theLine[ii] != 0x00) && (kk < kNMEAargLen))
		{
			nmeaArgs[jj].argString[kk++]	=	theLine[ii++];
		}
		ii++;	//	Skip delimiter
		jj++;	//	Next argument
	}
	return(jj);
}

//**************************************************************************************
int	Ascii2charsToInt(const char *charPtr)
{
int	myValue;

	//*	convert 2 chars to int
	myValue	=	(charPtr[0] & 0x0f) * 10;
	myValue	+=	(charPtr[1] & 0x0f);

	return(myValue);
}


//**************************************************************************************
//*	parse the NMEA time into seconds
static bool	ReturnNMEATime(char *theTimeString, unsigned long *rTime, TYPE_timeHHMMSS *timeHHMMSS)
{
bool			localValidFlag;
unsigned long	mySecondsValue;

	localValidFlag	=	false;

//	CONSOLE_DEBUG_W_STR("theTimeString=", theTimeString);
	//*	check to make sure the string is the right length and the first char is a digit
	if ((strlen(theTimeString) > 5) && (isdigit(theTimeString[0])))
	{
		timeHHMMSS->hours	=	Ascii2charsToInt(theTimeString);
		timeHHMMSS->minutes	=	Ascii2charsToInt(&theTimeString[2]);
		timeHHMMSS->seconds	=	Ascii2charsToInt(&theTimeString[4]);


		mySecondsValue	=	timeHHMMSS->hours * 3600;
		mySecondsValue	+=	timeHHMMSS->minutes * 60;
		mySecondsValue	+=	timeHHMMSS->seconds;

		localValidFlag	=	true;
	}
	else
	{
//		CONSOLE_DEBUG("Setting time invalid");
		//*	not valid
		mySecondsValue	=	0;
		localValidFlag	=	false;
	}
	if (rTime != NULL)
	{
		*rTime			=	mySecondsValue;
	}
#ifdef _ENABLE_TIME_DEGBUG_
	if (localValidFlag == false)
	{
		CONSOLE_DEBUG_W_STR("Time invalid", gLastNMEAdata);
	}
#endif

	return(localValidFlag);
}

//*****************************************************************************	10K
void	FixLonSeconds(LatLonType *theLon, unsigned long *longLon)
{
	if ((theLon->Ch == 'W') || (theLon->Ch == 'w'))
	{
		*longLon=	kMaxLonValueConst	- (	(theLon->Deg * kOneDegree) +
											(theLon->Min * k600Integer) +
											(theLon->Sec * kUnitsPerSecond) +
											(theLon->Ten * kUniterPerTenth));
	}
	else if ((theLon->Ch == 'E') || (theLon->Ch == 'e'))
	{
		*longLon=	kMaxLonValueConst	+ (	(theLon->Deg * kOneDegree) +
											(theLon->Min * k600Integer) +
											(theLon->Sec * kUnitsPerSecond) +
											(theLon->Ten * kUniterPerTenth));
	}
}

//*****************************************************************************	10K
void	FixLatSeconds(LatLonType *theLat, unsigned long *longLat)
{
	if ((theLat->Ch == 'N') || (theLat->Ch == 'n'))
	{
		*longLat=	kMaxLatValueConst	- (	theLat->Deg * kOneDegree +
											theLat->Min * k600Integer +
											theLat->Sec * kUnitsPerSecond +
											theLat->Ten * kUniterPerTenth);
	}
	else if ((theLat->Ch == 'S') || (theLat->Ch == 's'))
	{
		*longLat=	kMaxLatValueConst	+ (	theLat->Deg * kOneDegree +
											theLat->Min * k600Integer +
											theLat->Sec * kUnitsPerSecond +
											theLat->Ten * kUniterPerTenth);
	}
}

//**************************************************************************************
//*	Jan  2,	2013	<MLS> Added LatLonType argument to ReturnNMEALatLon()
//**************************************************************************************
static bool	ReturnNMEALatLon(	char		*latStr,
								char		*latCh,
								char		*lonStr,
								char		*lonCh,
								long		*xxLon,
								long		*yyLat,
								LatLonType	*lat,
								LatLonType	*lon)
{
unsigned long	localxxLon, localyyLat;
long			tSecs;
bool			localValidFlag;

#ifdef _ENABLE_GPS_DEBUGGING_LATLON
	CONSOLE_DEBUG("ReturnNMEALatLon");
	CONSOLE_DEBUG_W_STR("latStr=", latStr);
	CONSOLE_DEBUG_W_STR("lonStr=", lonStr);
#endif

	if ((lonStr[5] == '.') && (latStr[4] == '.') && (isdigit(lonStr[0])) && (isdigit(latStr[0])))
	{
		memset(lon, 0, sizeof(LatLonType));
		memset(lat, 0, sizeof(LatLonType));

		lon->Deg	=	((lonStr[0] & 0x0f) * 100) + ((lonStr[1] & 0x0f) * 10) + (lonStr[2] & 0x0f);
		lon->Min	=	((lonStr[3] & 0x0f) * 10) + (lonStr[4] & 0x0f);
		tSecs		=	((lonStr[6] & 0x0f) * 100) + ((lonStr[7] & 0x0f) * 10) + (lonStr[8] & 0x0f);
		tSecs		=	tSecs * 60;
		lon->Sec	=	tSecs / 1000;
//		lon->Ten	=	(tSecs - (lon->Sec * 1000))/100;
		lon->Ten	=	(tSecs - (lon->Sec * 1000))/10;
		lon->Ch		=	lonCh[0];

		lat->Deg	=	((latStr[0] & 0x0f) * 10) + (latStr[1] & 0x0f);
		lat->Min	=	((latStr[2] & 0x0f) * 10) + (latStr[3] & 0x0f);
		tSecs		=	((latStr[5] & 0x0f) * 100) + ((latStr[6] & 0x0f) * 10) + (latStr[7] & 0x0f);
		tSecs		=	tSecs * 60;
		lat->Sec	=	tSecs / 1000;
//		lat->Ten	=	(tSecs - (lat->Sec * 1000))/100;
		lat->Ten	=	(tSecs - (lat->Sec * 1000))/10;
		lat->Ch		=	latCh[0];

#ifdef _ENABLE_GPS_DEBUGGING_LATLON
		CONSOLE_DEBUG_W_NUM("lat->Deg=", lat->Deg);
		CONSOLE_DEBUG_W_NUM("lat->Min=", lat->Min);
		CONSOLE_DEBUG_W_NUM("lat->Sec=", lat->Sec);
		CONSOLE_DEBUG_W_NUM("lat->Ten=", lat->Ten);

		CONSOLE_DEBUG_W_NUM("lon->Deg=", lon->Deg);
		CONSOLE_DEBUG_W_NUM("lon->Min=", lon->Min);
		CONSOLE_DEBUG_W_NUM("lon->Sec=", lon->Sec);
		CONSOLE_DEBUG_W_NUM("lon->Ten=", lon->Ten);

#endif
		FixLonSeconds(lon, &localxxLon);
		FixLatSeconds(lat, &localyyLat);
		localValidFlag	=	true;
	}
	else
	{
#ifdef _ENABLE_GPS_DEBUGGING_
		CONSOLE_DEBUG("Invalid");
#endif
		localxxLon		=	0;
		localyyLat		=	0;
		localValidFlag	=	false;
	}
	if ((localxxLon == kMaxLonValueConst) && (localyyLat == kMaxLatValueConst))
	{
		localValidFlag	=	false;
	}

	*xxLon	=	localxxLon;
	*yyLat	=	localyyLat;

	return(localValidFlag);
}

#define	kKMeterToMiles	(0.62137119223733)
#define	kKnotsToMiles	(1.1507771827)		/*	6080 ft in Knot	*/
#define	kMetersToFeet	(3.2808398950131)

//**************************************************************************************
static bool	ReturnNMEASpeed(char *theSpeed, const char *theSpeedUnit,short *ssSpd)
{
short	returnMPH;
double	localDouble;
double	localDSpeed;
bool	validSpdFlag;

	if (strlen(theSpeed) > 0)
	{
		validSpdFlag	=	true;

		localDSpeed	=	atof(theSpeed);
		switch (theSpeedUnit[0])
		{
			case 'K':
			case 'k':	localDouble		=	localDSpeed * kKMeterToMiles;
						returnMPH		=	localDouble;
						break;

			case 'N':
			case 'n':	localDouble		=	localDSpeed * kKnotsToMiles;
						returnMPH		=	localDouble;
						break;

			case 'S':
			case 's':	returnMPH		=	localDSpeed;
						break;

			default:	returnMPH		=	0;
						validSpdFlag	=	false;
						break;
		}
		*ssSpd		=	returnMPH;
	}
	else
	{
		validSpdFlag	=	false;
	}

	return(validSpdFlag);
}

//**************************************************************************************
static bool	ReturnNMEAAltitude(char *theAltitude, char *theAltitudeUnit, long *zzAlt)
{
double	localAlt;
double	localDouble;
long	returnFeet;
bool	validALtFlag;

//	CONSOLE_DEBUG_W_STR("theAltitude=", theAltitude);
//	CONSOLE_DEBUG_W_STR("theAltitudeUnit=", theAltitudeUnit);

	if (strlen(theAltitude) > 0)
	{
		localAlt		=	atof(theAltitude);
		validALtFlag	=	false;
		switch (theAltitudeUnit[0])
		{
			case 'M':
			case 'm':	localDouble		=	localAlt * kMetersToFeet;
						returnFeet		=	localDouble;
						validALtFlag	=	true;
						break;

			case 'F':
			case 'f':	returnFeet		=	localAlt;
						validALtFlag	=	true;
						break;

			default:	returnFeet		=	0;
						validALtFlag	=	false;
						break;
		}

		*zzAlt		=	returnFeet;
	}
	else
	{
		validALtFlag	=	false;
	}
	return(validALtFlag);
}

//**************************************************************************************
static void	TrackLatLonValues(TYPE_NMEAInfoStruct *theNmeaInfo)
{
#ifdef _ENABLE_LAT_LON_TRACKING_

int	latlonIndex;
double	myLatitude;
double	mylongitude;

	if (theNmeaInfo->validLatLon)
	{
		myLatitude	=	theNmeaInfo->latitude.Deg
						+ (theNmeaInfo->latitude.Min / 60.0)
						+ (theNmeaInfo->latitude.Sec / 3600.0)
						+ (theNmeaInfo->latitude.Ten / 36000.0);

		mylongitude	=	theNmeaInfo->longitude.Deg
						+ (theNmeaInfo->longitude.Min / 60.0)
						+ (theNmeaInfo->longitude.Sec / 3600.0)
						+ (theNmeaInfo->longitude.Ten / 36000.0);

		//*	we have a valid time, so we can save the lat lon value
		latlonIndex	=	theNmeaInfo->gpsTime / kLatLonTacking_deltaTime;
		if (latlonIndex < kLatLonTacking_ArraySize)
		{
			theNmeaInfo->latitudeHistory[latlonIndex]	=	myLatitude;
			theNmeaInfo->longitudeHistory[latlonIndex]	=	mylongitude;
		}
	}
#endif
}

#ifdef _ENABLE_ALTITUDE_TRACKING_
//**************************************************************************************
static void	TrackAltitudeValue(TYPE_NMEAInfoStruct	*theNmeaInfo, const double altitude_feet)
{
int	altIndex;

	//*	we have a valid time, so we can save the altitude value
	altIndex	=	theNmeaInfo->gpsTime / kAltTacking_deltaTime;
	if (altIndex < kAltTacking_ArraySize)
	{
		theNmeaInfo->altitudeHistory[altIndex]	=	altitude_feet;
	}
}
#endif	//	_ENABLE_ALTITUDE_TRACKING_



//**************************************************************************************
//*	returns TRUE if line sentance was found and processed
//**************************************************************************************
static bool	ParseNMEAstringsNormal(	const char			*theLine,
									TYPE_NMEAInfoStruct	*theNmeaInfo,
									TYPE_NMEAnameStruct	*theNNptr,
									TYPE_NMEAargs		*nmeaArgs)
{
unsigned long	messageType;
bool			processedOK;

//	CONSOLE_DEBUG(theLine);

	messageType	=	((long)(theLine[2]) << 24) +
					((long)(theLine[3]) << 16) +
					((long)(theLine[4]) <<  8) +
					((long)(theLine[5]));


#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_
	NMEAtrack_Update(messageType, nmeaArgs[0].argString, theLine);
#endif

	processedOK	=	true;
	switch (messageType)
	{

		//---------------------------------------------------------------------
		//	GEOGRAPHIC FIXES OF POSITON
		case	0x50474446:	//='PGDF':			//	Fixes, Dead Reckoning	Predicted
		case	0x50474450:	//='PGDP':			//							Present
		case	0x50474441:	//='PGDA':			//							Past

		case	0x50474C46:	//='PGLF':			//	Fixes Loran C			Predicted
		case	0x50474C50:	//='PGLP':			//							Present
		case	0x50474C41:	//='PGLA':			//							Past

		case	0x50474F46:	//='PGOF':			//	Fixes Omega				Predicted
		case	0x50474F50:	//='PGOP':			//							Present
		case	0x50474F41:	//='PGOA':			//							Past

		case	0x50475846:	//='PGXF':			//	Transit Positions		Predicted
		case	0x50475850:	//='PGXP':			//							Present
		case	0x50475841:	//='PGXA':			//							Past
		{
			strcpy(theNNptr->Time,			nmeaArgs[1].argString);	//	Time of Position FIX
			strcpy(theNNptr->Lat,			nmeaArgs[2].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[3].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[4].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[5].argString);

			theNmeaInfo->validTime		=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(	theNNptr->Lat,
																theNNptr->LatC,
																theNNptr->Lon,
																theNNptr->LonC,
																&theNmeaInfo->xxLon,
																&theNmeaInfo->yyLat,
																&theNmeaInfo->latitude,
																&theNmeaInfo->longitude);
		}	break;



		//---------------------------------------------------------------------
		//*	$GPNVP,040916,122244,3510.1140,N,12903.0885,E,1,06,001,-013,M,317.9,T,000.4,K,*21
		case	'PNVP':
		{
			strcpy(theNNptr->Time,			nmeaArgs[1].argString);		//	Time of Position FIX
			strcpy(theNNptr->Lat,			nmeaArgs[3].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[4].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[5].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[6].argString);
			strcpy(theNNptr->Sat,			nmeaArgs[7].argString);		//	Satilite Flag
			strcpy(theNNptr->NumSats,		nmeaArgs[8].argString);		//	Number of Satalites
			strcpy(theNNptr->HDOP,			nmeaArgs[9].argString);		//	Horizontal Dilution of Precison
			strcpy(theNNptr->Alt,			nmeaArgs[10].argString);	//	Altitude (in meters)
			strcpy(theNNptr->AltC,			nmeaArgs[11].argString);	//	Unit Character M or F
			strcpy(theNNptr->SpdOvrGrnd,	nmeaArgs[14].argString);	//	Speed over ground
			strcpy(theNNptr->SpdOvrGrndC,	nmeaArgs[15].argString);	//	Speed over ground unit (should be K)

			theNmeaInfo->validTime		=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(	theNNptr->Lat,
																theNNptr->LatC,
																theNNptr->Lon,
																theNNptr->LonC,
																&theNmeaInfo->xxLon,
																&theNmeaInfo->yyLat,
																&theNmeaInfo->latitude,
																&theNmeaInfo->longitude);

			theNmeaInfo->validCseSpd	=	ReturnNMEASpeed(theNNptr->SpdOvrGrnd, "N", &theNmeaInfo->spd);	//	Knots, Convert to MPH
			theNmeaInfo->cse			=	atoi(nmeaArgs[12].argString);

		}	break;


		//---------------------------------------------------------------------
		//*	$GPGGA,005604,4027.001,N,07428.737,W,1,05,2.4,10.3,M,-34.0,M,,*46
		//case	0x50474741:	//='PGGA':			//	GPS		GPS Position-Past
		case	'PGGA':		//='PGGA':			//	GPS		GPS Position-Past
		{
//			CONSOLE_DEBUG("PGGA");
//			DumpNMEAargs(nmeaArgs);

			strcpy(theNNptr->Time,			nmeaArgs[1].argString);
			strcpy(theNNptr->Lat,			nmeaArgs[2].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[3].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[4].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[5].argString);
			strcpy(theNNptr->Sat,			nmeaArgs[6].argString);		//	Satellite Flag
			strcpy(theNNptr->NumSats,		nmeaArgs[7].argString);		//	Number of Satellites
			strcpy(theNNptr->HDOP,			nmeaArgs[8].argString);		//	Horizontal Dilution of Precision
			strcpy(theNNptr->Alt,			nmeaArgs[9].argString);		//	Altitude (in meters)
			strcpy(theNNptr->AltC,			nmeaArgs[10].argString);	//	Unit Character M or F

			theNmeaInfo->validTime		=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);

			theNmeaInfo->validAlt		=	ReturnNMEAAltitude(theNNptr->Alt, theNNptr->AltC, &theNmeaInfo->zzAlt);	//	Turn this off for MAGELLEN Ballon Flights
			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(theNNptr->Lat,
															theNNptr->LatC,
															theNNptr->Lon,
															theNNptr->LonC,
															&theNmeaInfo->xxLon,
															&theNmeaInfo->yyLat,
															&theNmeaInfo->latitude,
															&theNmeaInfo->longitude);
			TrackLatLonValues(theNmeaInfo);
		#ifdef _ENABLE_SATELLITE_ALMANAC_
			if (theNmeaInfo->gpsTime > 0)
			{
			int	numSatIndex;

				//*	we have a valid time, so we can save the PDOP value
				numSatIndex	=	theNmeaInfo->gpsTime / kSatsInUseTracking_deltaTime;
				if (numSatIndex < kSatsInUseTracking_ArraySize)
				{
					theNmeaInfo->satsInUse[numSatIndex]	=	atoi(theNNptr->NumSats);
					theNmeaInfo->satMode[numSatIndex]	=	atoi(theNNptr->Mode);
				}
			}
		#endif

			if (strlen(theNNptr->Alt) > 0)
			{
			//	CONSOLE_DEBUG_W_STR("Alt", theNNptr->Alt);
			//	CONSOLE_DEBUG_W_STR("AltC", theNNptr->AltC);
				switch (theNNptr->AltC[0])
				{
					case 'm':
					case 'M':
						theNmeaInfo->altitudeMeters	=	atof(theNNptr->Alt);
						theNmeaInfo->altitudeFeet	=	theNmeaInfo->altitudeMeters * kMetersToFeet;
						break;

					case 'f':
					case 'F':
						theNmeaInfo->altitudeFeet	=	atof(theNNptr->Alt);
						break;
				}
			}

	#ifdef _ENABLE_ALTITUDE_TRACKING_
			if ((theNmeaInfo->validAlt) && (theNmeaInfo->gpsTime > 0))
			{
				TrackAltitudeValue(theNmeaInfo, theNmeaInfo->altitudeFeet);
			}
	#endif

			switch (theNNptr->Sat[0])
			{
				case '2':
					theNmeaInfo->validTime		=	true;
					theNmeaInfo->validLatLon	=	true;
					theNmeaInfo->dgps			=	2;
					break;

				case '1':
					theNmeaInfo->validTime		=	true;
					theNmeaInfo->validLatLon	=	true;
					theNmeaInfo->dgps			=	1;
					break;

				#warning Keith, why is this time invalid
				case '0':
				default:
//					CONSOLE_DEBUG("PGGA - Setting time invalid");
					theNmeaInfo->validTime		=	false;
					theNmeaInfo->validLatLon	=	false;
					theNmeaInfo->dgps			=	0;
					break;
			}
		}	break;

		//---------------------------------------------------------------------
		//	$GPGLL,4027.001,N,07428.738,W,005605,A*3B
//		case	0x50474C4C:	//='PGLL':			//	GEOGRAPHICAL		GLL,XXXX.XX,N,XXXXX.XX,W
		case	'PGLL':		//='PGLL':			//	GEOGRAPHICAL		GLL,XXXX.XX,N,XXXXX.XX,W
		{
			strcpy(theNNptr->Lat,			nmeaArgs[1].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[2].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[3].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[4].argString);
			if (strlen(nmeaArgs[5].argString) > 2)				//	Make sure it is NOT the checksum (EAGLE does this)
			{
				strcpy(theNNptr->Time,		nmeaArgs[5].argString);	//	May not always be present
				theNmeaInfo->validTime	=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			}
			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(	theNNptr->Lat,
																theNNptr->LatC,
																theNNptr->Lon,
																theNNptr->LonC,
																&theNmeaInfo->xxLon,
																&theNmeaInfo->yyLat,
																&theNmeaInfo->latitude,
																&theNmeaInfo->longitude);
//--			gTotalPositCount++;
			if ((nmeaArgs[6].argString[0] == 'A') || (nmeaArgs[6].argString[0] == 0))
			{
//--				theNmeaInfo->whichIcon	=	kGpsIconGLLGood;
//--				gTotalGoodPosit++;
			}
			else
			{
//--				theNmeaInfo->whichIcon	=	kGpsIconGLLBad;
			}
		}	break;

		//---------------------------------------------------------------------
		//*	$GPRMA
		case	'PRMA':			//		Recommended Minimum	Specific Loran-C Data
		{
			strcpy(theNNptr->Lat,			nmeaArgs[2].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[3].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[4].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[5].argString);

			strcpy(theNNptr->SpdOvrGrnd,	nmeaArgs[8].argString);
			strcpy(theNNptr->TMG,			nmeaArgs[9].argString);

			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(	theNNptr->Lat,
																theNNptr->LatC,
																theNNptr->Lon,
																theNNptr->LonC,
																&theNmeaInfo->xxLon,
																&theNmeaInfo->yyLat,
																&theNmeaInfo->latitude,
																&theNmeaInfo->longitude);
		//	Check this against NMEA Specs	05/03/96
			theNmeaInfo->spd	=	atoi(nmeaArgs[8].argString);
			theNmeaInfo->cse	=	atoi(nmeaArgs[9].argString);
		//	if ((ssSpd != 0) || (ccCse != 0))
			if ((strlen(nmeaArgs[8].argString) > 0) && (strlen(nmeaArgs[9].argString) > 0))		//	VAC96
			{
				theNmeaInfo->validCseSpd	=	true;
			}
		}	break;

		//---------------------------------------------------------------------
		//	$GPRMB,A,,,,,,,,,,,,V*71
		//	$GPRMB,A,0.23,R,,CLAY,3922.830,N,09707.314,W,064.5,288.3,038.1,V*2E
		case	'PRMB':			//		Recommended Minimum	Navigation Information
		{
		#ifdef _ENABLE_WAYPOINT_PARSING_
			strcpy(theNNptr->XTE,			nmeaArgs[2].argString);
			strcpy(theNNptr->DirToSteer,	nmeaArgs[3].argString);
			strcpy(theNNptr->OrigWP,		nmeaArgs[4].argString);
			strcpy(theNNptr->DestWP,		nmeaArgs[5].argString);

			strcpy(theNNptr->WayPtLat,		nmeaArgs[6].argString);
			strcpy(theNNptr->WayPtLatC,		nmeaArgs[7].argString);
			strcpy(theNNptr->WayPtLon,		nmeaArgs[8].argString);
			strcpy(theNNptr->WayPtLonC,		nmeaArgs[9].argString);

			strcpy(theNNptr->WayPtDist,		nmeaArgs[10].argString);
			strcpy(theNNptr->WayPtBerT,		nmeaArgs[11].argString);
		#endif
		}	break;

		//---------------------------------------------------------------------
		//			1     2 3       4 5        6 7     8     9
		//	$gprmc,220535,a,2658.37,n,08008.26,w,000.8,117.5,1?
		//	$GPRMC,005604,A,4027.001,N,07428.737,W,000.9,039.1,281197,012.7,W*79
//		case	0x50524D43:	//='PRMC':					//		Recommended Minimum	Specific GPS/Transit Data
		case	'PRMC':		//='PRMC':					//		Recommended Minimum	Specific GPS/Transit Data
		{
		int	magVarSign;

		#ifdef _ENABLE_GPS_DEBUGGING_
			CONSOLE_DEBUG_W_STR("$GPRMC", theLine);
		#endif

			strcpy(theNNptr->Time,			nmeaArgs[1].argString);	//	Is it Current Time? of time of fix?
			strcpy(theNNptr->Lat,			nmeaArgs[3].argString);
			strcpy(theNNptr->LatC,			nmeaArgs[4].argString);
			strcpy(theNNptr->Lon,			nmeaArgs[5].argString);
			strcpy(theNNptr->LonC,			nmeaArgs[6].argString);

			strcpy(theNNptr->SpdOvrGrnd,	nmeaArgs[7].argString);		//	Speed Over Ground	KNOTS
			strcpy(theNNptr->TMG,			nmeaArgs[8].argString);
			strcpy(theNNptr->Date,			nmeaArgs[9].argString);
			strcpy(theNNptr->MagVar,		nmeaArgs[10].argString);
			strcpy(theNNptr->MagVarC,		nmeaArgs[11].argString);

			theNmeaInfo->validTime		=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			theNmeaInfo->validLatLon	=	ReturnNMEALatLon(	theNNptr->Lat,
																theNNptr->LatC,
																theNNptr->Lon,
																theNNptr->LonC,
																&theNmeaInfo->xxLon,
																&theNmeaInfo->yyLat,
																&theNmeaInfo->latitude,
																&theNmeaInfo->longitude);
			if (theNNptr->MagVarC[0] == 'E')
			{
				magVarSign	=	-1;
			}
			else
			{
				magVarSign	=	1;
			}
			theNmeaInfo->magneticVariation	=	magVarSign * atof(theNNptr->MagVar);

	#ifdef _ENABLE_MAGNETIC_VARIATION_TRACKING_
			if (theNmeaInfo->gpsTime > 0)
			{
			int	magVarIndex;

				//*	we have a valid time, so we can save the PDOP value
				magVarIndex	=	theNmeaInfo->gpsTime / kMagVariationTracking_deltaTime;
				if (magVarIndex < kMagVariationTracking_ArraySize)
				{
					theNmeaInfo->magVariationArray[magVarIndex]	=	theNmeaInfo->magneticVariation;
				}
			}
	#endif

	#ifdef _ENABLE_GPS_DEBUGGING_
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->latitude.Deg=", theNmeaInfo->latitude.Deg);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->latitude.Min=", theNmeaInfo->latitude.Min);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->latitude.Sec=", theNmeaInfo->latitude.Sec);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->latitude.Ten=", theNmeaInfo->latitude.Ten);

			CONSOLE_DEBUG_W_NUM("theNmeaInfo->longitude.Deg=", theNmeaInfo->longitude.Deg);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->longitude.Min=", theNmeaInfo->longitude.Min);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->longitude.Sec=", theNmeaInfo->longitude.Sec);
			CONSOLE_DEBUG_W_NUM("theNmeaInfo->longitude.Ten=", theNmeaInfo->longitude.Ten);

	#endif
			theNmeaInfo->validCseSpd	=	ReturnNMEASpeed(theNNptr->SpdOvrGrnd,"N", &theNmeaInfo->spd);	//	Knots, Convert to MPH
			theNmeaInfo->cse			=	atoi(nmeaArgs[8].argString);
			//*	verify the date is valid
			if ((strlen(theNNptr->Date) == 6)	&&
				 isdigit(theNNptr->Date[0])		&&
				 isdigit(theNNptr->Date[1])		&&
				 isdigit(theNNptr->Date[2])		&&
				 isdigit(theNNptr->Date[3])		&&
				 isdigit(theNNptr->Date[4])		&&
				 isdigit(theNNptr->Date[5]))
			{
			//	CONSOLE_DEBUG("Date OK");
				theNmeaInfo->validDate	=	true;
			}
//--			gTotalPositCount++;
			if (nmeaArgs[2].argString[0] == 'A')
			{
//--				theNmeaInfo->whichIcon	=	kGpsIconRMCGood;
//--				gTotalGoodPosit++;
			}
			else
			{
//--				theNmeaInfo->whichIcon	=	kGpsIconRMCBad;
			}
		#ifdef _ENABLE_GPS_DEBUGGING_
//			DumpNMEAargs(nmeaArgs);
		#endif
		}	break;

		//---------------------------------------------------------------------
	//	case	'PAAM':		//	Waypoint Arrival Alarm				AAM,A,A,XXX.,N,CCCC
	//	case	'PALM':		//	GPS Almanc Data

		case	'PAPA':		//-	Auto Pilot Format-A	Loran
		case	'PAPB':		//	Autopilot Sentence "B"
		#ifdef _ENABLE_WAYPOINT_PARSING_
			strcpy(theNNptr->DestWP,		nmeaArgs[10].argString);
		#endif
			break;

		//---------------------------------------------------------------------
	//	case	'PASD':		//*	Autopilot System Data

		case	'PBEC':		//	Bearing & Distance to Waypoint, Dead Reckoning
			strcpy(theNNptr->Time,		nmeaArgs[1].argString);
			theNmeaInfo->validTime	=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			break;

		//---------------------------------------------------------------------
	//	case	'PBER':		//-?	$GPBER
	//	case	'PBOD':		//		$GPBOD,,T,,M,,*47	Bearing, Origin to Destination				BOD,XXX.,T,XXX.,M,CCCC,CCCC
	//	case	'PBPI':		//-?	$GPBPI

		//		  1        2        3 4         5 6    7 8     9 10    1 12
		//$GPBWC,134859.00,4123.600,N,07327.250,W,98.7,T,111.5,M,37.42,N,DANBURY
		case	0x50415743:	//='PBWC':		//		$GPBWC	Bearing & Distance to Waypoint
		case	0x50415752:	//='PBWR':		//		$GPBWR	Bearing & Distance to Waypoint, Rhumb Line
		{
		#ifdef _ENABLE_WAYPOINT_PARSING_
			strcpy(theNNptr->Time,			nmeaArgs[1].argString);
			strcpy(theNNptr->WayPtLat,		nmeaArgs[2].argString);
			strcpy(theNNptr->WayPtLatC,		nmeaArgs[3].argString);
			strcpy(theNNptr->WayPtLon,		nmeaArgs[4].argString);
			strcpy(theNNptr->WayPtLonC,		nmeaArgs[5].argString);
			strcpy(theNNptr->WayPtBerT,		nmeaArgs[6].argString);
			strcpy(theNNptr->WayPtBerTC,	nmeaArgs[7].argString);
			strcpy(theNNptr->WayPtBerM,		nmeaArgs[8].argString);
			strcpy(theNNptr->WayPtBerMC,	nmeaArgs[9].argString);
			strcpy(theNNptr->WayPtDist,		nmeaArgs[10].argString);
			strcpy(theNNptr->WayPtDistC,	nmeaArgs[11].argString);
			strcpy(theNNptr->DestWP,		nmeaArgs[12].argString);
			theNmeaInfo->validTime	=		ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
		#endif
		}	break;

		//---------------------------------------------------------------------
	//	case	'PBWW':		//	Bearing, Waypoint to Waypoint
	//	case	'PDBK':		//-	Depth of Water		DBK,XXXX.X,f,XXXX.X,M,XXX.X,F	Depth Below Keel
	//	case	'PDBS':		//-	Depth of Water		DBS,XXXX.X,f,XXXX.X,M,XXX.X,F	Depth Below Surface
	//	case	'PDBT':		//	Depth of Water		DBT,XXXX.X,f,XXXX.X,M,XXX.X,F	Depth Below Transducer
	//	case	'PDCN':		//	Decca Postion
	//	case	'PDPT':		//*	Depth
	//	case	'PDRU':		//-?
	//	case	'PFSI':		//*	Frequency Set Information
	//	case	'PGLC':		//	Geographic Position, Loran-C
	//	//		'PGLL		//	Geographic Position, Latitude.Longitude



		//---------------------------------------------------------------------
		//	$GPGSA,A,3,,03,15,18,19,27,31,,,,,,2.8,2.4,1.0*3E
	//	case	0x50475341:	//='PGSA':		//	GPS DOP and Active Satellites
		case	'PGSA':		//='PGSA':		//	GPS DOP and Active Satellites
			//*	arg	1	=	Mode, M= Manual, A=Automatic
			//*	arg 2	=	Mode,	1=Fix not available
			//*						2=2D
			//*						3=3D
			//*	arg 3->14,	IDs of SV used in position fix (null for unused fields)
			//*	arg 15,	=	PDOP
			//*	arg 16,	=	HDOP
			//*	arg 17,	=	VDOP
			{
				theNmeaInfo->currSatMode1	=	nmeaArgs[1].argString[0];
				theNmeaInfo->currSatMode2	=	nmeaArgs[2].argString[0];
				theNmeaInfo->currSatsInUse	=	atoi(nmeaArgs[2].argString);
//				CONSOLE_DEBUG_W_HEX("currSatMode1\t=",	theNmeaInfo->currSatMode1);
//				CONSOLE_DEBUG_W_HEX("currSatMode2\t=",	theNmeaInfo->currSatMode2);
//				CONSOLE_DEBUG_W_NUM("currSatsInUse\t=",	theNmeaInfo->currSatsInUse);
			#ifdef _ENABLE_SATELLITE_ALMANAC_
//				CONSOLE_DEBUG_W_STR("$GPGSA", theLine);
				//*	Oct  3,	2016	TODO: Compare this with MacAPRS code
				strcpy(theNNptr->Mode,					nmeaArgs[2].argString);

				strcpy(theNmeaInfo->theSats[ 0].prn,	nmeaArgs[3].argString);
				strcpy(theNmeaInfo->theSats[ 1].prn,	nmeaArgs[4].argString);
				strcpy(theNmeaInfo->theSats[ 2].prn,	nmeaArgs[5].argString);
				strcpy(theNmeaInfo->theSats[ 3].prn,	nmeaArgs[6].argString);
				strcpy(theNmeaInfo->theSats[ 4].prn,	nmeaArgs[7].argString);
				strcpy(theNmeaInfo->theSats[ 5].prn,	nmeaArgs[8].argString);
				strcpy(theNmeaInfo->theSats[ 6].prn,	nmeaArgs[9].argString);
				strcpy(theNmeaInfo->theSats[ 7].prn,	nmeaArgs[10].argString);
				strcpy(theNmeaInfo->theSats[ 8].prn,	nmeaArgs[11].argString);
				strcpy(theNmeaInfo->theSats[ 9].prn,	nmeaArgs[12].argString);
				strcpy(theNmeaInfo->theSats[10].prn,	nmeaArgs[13].argString);
				strcpy(theNmeaInfo->theSats[11].prn,	nmeaArgs[14].argString);
			#endif
				strcpy(theNNptr->PDOP,					nmeaArgs[15].argString);
				strcpy(theNNptr->HDOP,					nmeaArgs[16].argString);
				strcpy(theNNptr->VDOP,					nmeaArgs[17].argString);

			#ifdef _ENABLE_PDOP_TRACKING_
				if (theNmeaInfo->gpsTime > 0)
				{
				int	pdopIndex;

					//*	we have a valid time, so we can save the PDOP value
					pdopIndex	=	theNmeaInfo->gpsTime / kPDOPtacking_deltaTime;
					if (pdopIndex < kPDOPtacking_ArraySize)
					{
						theNmeaInfo->pdopHistory[pdopIndex]	=	atof(theNNptr->PDOP);
						theNmeaInfo->vdopHistory[pdopIndex]	=	atof(theNNptr->VDOP);
						theNmeaInfo->hdopHistory[pdopIndex]	=	atof(theNNptr->HDOP);
					}
				}
			#endif
			//	CONSOLE_DEBUG_W_STR("$GPRMC",	theLine);
			//	CONSOLE_DEBUG_W_STR("PDOP",		theNNptr->PDOP);
			//	CONSOLE_DEBUG_W_STR("HDOP",		theNNptr->HDOP);
			//	CONSOLE_DEBUG_W_STR("VDOP",		theNNptr->VDOP);
			}
			break;

		//---------------------------------------------------------------------
		//	$GPGSV
		//case	0x50475356:	//='PGSV':		//	GPS Satellites in View
		case	'PGSV':		//	GPS Satellites in View
		{
	#ifdef _ENABLE_SATELLITE_ALMANAC_
		int 	ii;
		int		satArgIndex;
		short	satIndex;
		short	snrDistIndex;

			//**************************************************************************************
			//	$GPGSV,2,1,07,02,18,287,30,03,22,050,30,15,30,149,30,18,12,209,31*76
			//
			//	GPS Satellites in view
			//
			//	eg. $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
			//	    $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
			//	    $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D
			//
			//
			//	    $GPGSV,1,1,13,02,02,213,,03,-3,000,,11,00,121,,14,13,172,05*62
			//
			//
			//	1    = Total number of messages of this type in this cycle
			//	2    = Message number
			//	3    = Total number of SVs in view
			//	4    = SV PRN number
			//	5    = Elevation in degrees, 90 maximum
			//	6    = Azimuth, degrees from true north, 000 to 359
			//	7    = SNR, 00-99 dB (null when not tracking)
			//	8-11 = Information about second SV, same as field 4-7
			//	12-15= Information about third SV, same as field 4-7
			//	16-19= Information about fourth SV, same as field 4-7
			//**************************************************************************************
			theNmeaInfo->nmeaGPGSVcount++;
			//*	Sep  5,	2023	<MLS> Added NumSats from GPGSV
			strcpy(theNmeaInfo->theNN.NumSats,		nmeaArgs[3].argString);		//	Number of Satalites

			//*	step through the 4 possible data sets
			for (satArgIndex = 4; satArgIndex <= 16; satArgIndex += 4)
			{
				satIndex	=	atoi(nmeaArgs[satArgIndex].argString);

				if ((satIndex >= 0) && (satIndex < kMaxNumOfSatallites))
				{
					theNmeaInfo->theSats[satIndex].reportCnt	+=	1;
					theNmeaInfo->theSats[satIndex].satellitePRN	=	atoi(nmeaArgs[satArgIndex + 0].argString);
					theNmeaInfo->theSats[satIndex].elvevation	=	atoi(nmeaArgs[satArgIndex + 1].argString);
					theNmeaInfo->theSats[satIndex].azimuth		=	atoi(nmeaArgs[satArgIndex + 2].argString);
					theNmeaInfo->theSats[satIndex].signal2Noise	=	atoi(nmeaArgs[satArgIndex + 3].argString);

					if (theNmeaInfo->theSats[satIndex].signal2Noise > theNmeaInfo->theSats[satIndex].maxSNR)
					{
						theNmeaInfo->theSats[satIndex].maxSNR	=	theNmeaInfo->theSats[satIndex].signal2Noise;
					}

					//*	keep track of the SNR distribution
					snrDistIndex	=	theNmeaInfo->theSats[satIndex].signal2Noise;
					if ((snrDistIndex > 0) && (snrDistIndex < kMaxSNRvalue))
					{
						theNmeaInfo->snrDistribution[snrDistIndex]	+=	1;
					}

				}
				else
				{
					CONSOLE_DEBUG_W_NUM("$GPGSV: satIndex out of bounds", satIndex);
					CONSOLE_DEBUG_W_STR("Data=", theLine);
				}
				if (satIndex >= 32)
				{
			//		CONSOLE_DEBUG_W_STR("$GPGSV", theLine);
			//		CONSOLE_DEBUG_W_NUM("$GPGSV: satIndex >= 32", satIndex);
				}
			}


			//*	Dec  5,	2012	<MLS> Check for max snr value from satellite GSV string
			for (ii=0; ii<kMaxNumOfSatallites; ii++)
			{
			int	mySNRvalue;

				mySNRvalue	=	theNmeaInfo->theSats[ii].signal2Noise;
				if (mySNRvalue > gMaxSatelliteSNRvalue)
				{
					gMaxSatelliteSNRvalue	=	mySNRvalue;
				#ifdef _ENABLE_GPS_DEBUGGING_
					CONSOLE_DEBUG_W_NUM("$GPGSV New Max GPS signal strength=", gMaxSatelliteSNRvalue);
				#endif
				}
			}
	#endif
		}

		break;

		//---------------------------------------------------------------------
	//			'GXA		//	Trnsit Position
	//	case	'PGTD':		//					GTD,XXXXX.X,XXXXX.X,XXXXX.X,XXXXX.X				Loran Apelco
	//	case	'PHCC':		//-?	HEADING
	//	case	'PHCD':		//	Magnetic Deviation
	//	case	'PHDG':		//*	Heading, Deviation & Variation
		case	'PHDM':		//-	HEADING							HDT,XXX.,T						Heading, Magnetic, Present
		{
			theNmeaInfo->cse			=	atoi(nmeaArgs[1].argString);
			theNmeaInfo->validCseSpd	=	true;
		}	break;

		//---------------------------------------------------------------------
	//	case	'PHDT':		//*	Heading, True					HDT,XXX.,T						Heading, True, Present
	//	case	'PHSC':		//*	Heading	Steering Command		HDT,XXX.,T,XXX.,M				Steering Heading Command
	//	case	'PHVD':		//?	Magnetic Variation Derived
	//	case	'PHVM':		//?	Magnetic Variation Manual
	//	case	'PIMA':		//?	Vessel Identification
	//	case	'PLCD':		//	Loran-C Signal data
	//	case	'PMDA':		//?	METEROLOGICAL, COMPOSITE
	//	case	'PMHU':		//?
	//	case	'PMMB':		//?	METEROLOGICAL PRIMITIVE
	//	case	'PMTA':
	//	case	'PMTW':		//	Water Temperature
	//	case	'PMWD':
	//	case	'PMWH':
	//	case	'PMWS':
	//	case	'PMWV':		//*	Wind Speed & Angle
	//	case	'POLN':		//	Omega Lane Numbers
	//	case	'POLW':
	//	case	'POMP':
	//	case	'PONZ':
	//	case	'POSD':		//*	Own Ship data
	//			'PRMA':
	//			'PRMB':
	//			'PRMC':
	//	case	'PROT':		//*	Rate of Turn
	//	case	'PRPM':		//*	Revolutions
	//	case	'PRSA':		//*	Rudder Sensor Angle
	//	case	'PRSD':		//*	Radar System Data

		//	$GPRTE,0,1,c,*36
		//	$GPRTE,1,1,c,,CLAYCENTER*07
		case	'PRTE':		//	$GPRTE	Routes
			break;

		//---------------------------------------------------------------------
	//	case	'PSBK':
	//	case	'PSCD':
	//	case	'PSCY':
	//	case	'PSDB':
	//	case	'PSFI':		//*	Scanning Frequency Information
	//	case	'PSGD':
	//	case	'PSGR':
	//	case	'PSIU':
	//	case	'PSLC':
	//	case	'PSNC':
	//	case	'PSNU':
	//	case	'PSPD':
	//	case	'PSSF':
	//	case	'PSTC':
	//	case	'PSTN':		//	Multiple Data ID
	//	case	'PSTR':
	//	case	'PSUF':
	//	case	'PSUS':
	//	case	'PTEC':
	//	case	'PTEP':
	//	case	'PTGA':
	//	case	'PTIF':
	//	case	'PTRF':		//	Transit Fix Data
	//	case	'PTRP':
	//	case	'PTRS':
	//	case	'PTTM':		//*	Tracked Target Message
	//	case	'PVBW':		//*	Dual Ground/Water Speed
	//	case	'PVCD':
	//	case	'PVDR':		//	Set and Drift
	//	case	'PVHW':
	//	case	'PVLW':		//	Distance Travled through the Water
	//	case	'PVPE':
	//	case	'PVPW':		//	Speed, Measured Parallel to Wind
	//	case	'PVTA':

		//	$GPVTG,,T,,M,,N,,K*4E
		//	$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
		case	'PVTG':		//	Track Made Good and Ground Speed	//	Check This one
		{
			strcpy(theNNptr->TMG,			nmeaArgs[1].argString);
			strcpy(theNNptr->SpdOvrGrnd,	nmeaArgs[5].argString);	//	Knots
			strcpy(theNNptr->SpdOvrGrndC,	nmeaArgs[6].argString);	//	Should be 'N'

		//	strcpy(theNNptr->SpdOvrGrnd,	nmeaArgs[7].argString);	//	KM/HR
		//	strcpy(theNNptr->SpdOvrGrndC,	nmeaArgs[8].argString);	//	Should be 'K'

		//	ssSpd			=	atoi(nmeaArgs[5].argString);
			theNmeaInfo->validCseSpd	=	ReturnNMEASpeed(theNNptr->SpdOvrGrnd, theNNptr->SpdOvrGrndC, &theNmeaInfo->spd);
			theNmeaInfo->cse			=	atoi(nmeaArgs[1].argString);
		}	break;

		//---------------------------------------------------------------------
	//	case	'PVTI':
	//	case	'PVWE':
	//	case	'PVWR':
	//	case	'PVWT':
	//	case	'PWCV':		//	Waypoint Closure Velocity
	//	case	'PWDC':
	//	case	'PWDR':
	//	case	'PWFM':
	//	case	'PWNC':		//	Distance, Waypoint to Waypoint
	//	case	'PWNR':

		//	$GPWPL,3922.830,N,09707.314,W,CLAY*40
		case	0x5057504C:	//='PWPL':		//	Waypoint Location
		#ifdef _ENABLE_WAYPOINT_PARSING_
			strcpy(theNNptr->WayPtLat,		nmeaArgs[1].argString);
			strcpy(theNNptr->WayPtLatC,		nmeaArgs[2].argString);
			strcpy(theNNptr->WayPtLon,		nmeaArgs[3].argString);
			strcpy(theNNptr->WayPtLonC,		nmeaArgs[4].argString);
			strcpy(theNNptr->DestWP,		nmeaArgs[5].argString);
			theNmeaInfo->validWayPointLatLon	=	true;
		#endif
			break;

		//---------------------------------------------------------------------
	//	case	'PXDR':		//	Transducer Measurements
	//	case	'PXTE':		//	Cross-Track Error, Measured
	//	case	'PXTR':		//	Cross-Track Error, Dead Reckoning
	//	case	'PYWP':		//	Water Propagation
	//	case	'PYWS':		//	Water Salinity
	//	case	'PZCD':

		//*	$GPZDA
		case	0x505A4441:	//='PZDA':		//	Time & Date
			{
				strcpy(theNNptr->Time,		nmeaArgs[1].argString);
			#ifdef __MAC__
				sprintf(theNNptr->Date,"%s/%s/%s",nmeaArgs[2].argString, nmeaArgs[3].argString, nmeaArgs[4].argString);
			#endif
				theNmeaInfo->validTime	=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
			}
			break;

		//---------------------------------------------------------------------
	//	case	'PZEV':
	//	case	'PZFI':
	//	case	'PZFO':		//	UTC & Time from Origin Waypoint
	//	case	'PZLZ':
	//	case	'PZPI':
	//	case	'PZTA':
	//	case	'PZTE':
	//	case	'PZTG':		//	UTC & Time to Destination Waypoint
	//	case	'PZTI':
	//	case	'PZWP':
	//	case	'PZZU':



		//---------------------------------------------------------------------
		default:
			gUnknownNEMAcount++;
			processedOK	=	false;
			break;
	}

	return(processedOK);
}

#ifdef _ENABLE_PROPRIETARY_PARSING_
//**************************************************************************************
//*	returns TRUE if line sentance was found and processed
//**************************************************************************************
static bool	ParseNMEAstringsProprietary(char				*theLine,
										TYPE_NMEAInfoStruct	*theNmeaInfo,
										TYPE_NMEAnameStruct	*theNNptr,
										TYPE_NMEAargs		*nmeaArgs)
{
long		propiteryType;
bool		processedOK;
char		myChar;
short		tShort;
short		altitudeSource;
int			ii;

//	CONSOLE_DEBUG_W_STR("Proprietary", theLine);
	propiteryType	=	((long)(theLine[2]) << 24) +
						((long)(theLine[3]) << 16) +
						((long)(theLine[4]) <<  8) +
						((long)(theLine[5]));

#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_
	NMEAtrack_Update(propiteryType, nmeaArgs[0].argString, theLine);
#endif

//	CONSOLE_DEBUG_W_HEX("propiteryType", propiteryType);
//	CONSOLE_DEBUG_W_STR("propiteryType", &theLine[2]);

	processedOK	=	true;
	switch (propiteryType)
	{
		//*	ORIGINgps ORG14XX

		//*	$PSRF150	OK to send data to the module
		//*	$PSRF156,23,1,0*09
		case 'SRF1':
			//*	SiRF protocol stuff
			break;

		//*	 $PGRME,3.7,M,4.9,M,6.1,M*20
		//*	Estimated Position Error
		case 'GRME':
			theNmeaInfo->horizontalPosErr	=	atof(nmeaArgs[1].argString);
			theNmeaInfo->verticalPosErr		=	atof(nmeaArgs[3].argString);
			theNmeaInfo->sphericalPosErr	=	atof(nmeaArgs[5].argString);
		#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
			theNmeaInfo->gPGRME_exists		=	true;
				if (theNmeaInfo->gpsTime > 0)
				{
				int	trackIndex;


					//*	we have a valid time, so we can save the altitude value
					trackIndex	=	theNmeaInfo->gpsTime / kPosErrTacking_deltaTime;
					if (trackIndex < kPosErrTacking_ArraySize)
					{
						theNmeaInfo->horzPosErrArry[trackIndex]	=	theNmeaInfo->horizontalPosErr;
						theNmeaInfo->vertPosErrArry[trackIndex]	=	theNmeaInfo->verticalPosErr;
						theNmeaInfo->sphrPosErrArry[trackIndex]	=	theNmeaInfo->sphericalPosErr;
					}
				}
		#endif
			break;

		//*	$PGRMV,0.0,0.0,-0.1*70
		//*	3D Velocity Information
		case 'GRMV':
			break;

		//*	$PGRMM,WGS 84*06
		//*	Map Datum
		case 'GRMM':
			strcpy(theNmeaInfo->mapDatum, nmeaArgs[1].argString);
			break;

		//*	$PGRMT,GPS 15-H Ver. 2.02,P,P,R,R,P,,36,R*03
		//*	Sensor Status Information
		case 'GRMT':
//			CONSOLE_DEBUG_W_STR("GPS Model=\t", nmeaArgs[1].argString);
			strcpy(theNmeaInfo->gpsModel, nmeaArgs[1].argString);
			theNmeaInfo->gpsSensorTemp	=	atoi(nmeaArgs[8].argString);
			break;

		//*	 $PGRMZ,4998,f,3*27
		//*	Alutitude Information
		case 'GRMZ':
			altitudeSource	=	atoi(nmeaArgs[3].argString);
			if (altitudeSource == 3)
			{
				myChar		=	tolower(nmeaArgs[2].argString[0]);
				if (strlen(nmeaArgs[1].argString) > 0)
				{
					theNmeaInfo->validAlt	=	true;
					if (myChar == 'f')
					{
						theNmeaInfo->altitudeFeet	=	atof(nmeaArgs[1].argString);
						theNmeaInfo->zzAlt			=	atoi(nmeaArgs[1].argString);
					//	CONSOLE_DEBUG_W_STR("alt", nmeaArgs[1].argString);
					}
					else if (myChar == 'm')
					{
						theNmeaInfo->altitudeMeters	=	atof(nmeaArgs[1].argString);
						theNmeaInfo->altitudeFeet	=	theNmeaInfo->altitudeMeters * kMetersToFeet;
					}
			#ifdef _ENABLE_ALTITUDE_TRACKING_
					if ((theNmeaInfo->validAlt) && (theNmeaInfo->gpsTime > 0))
					{
						TrackAltitudeValue(theNmeaInfo, theNmeaInfo->altitudeFeet);
					}
			#endif
				}
			}
//			CONSOLE_DEBUG_W_NUM("GRMZ", theNmeaInfo->altitudeFeet);
			break;


		//*	$PRWIZCH,22,0,22,0,22,0,21,0,22,0,22,0,22,0,22,0,22,0,22,0,22,0,22,0*4E
		//*	$PRWIZCH,22,7,31,7,32,7,26,7,00,0,14,7,16,7,25,7,23,7,03,2,29,7,00,0*43
		//*	Rockwell propritary
		//*	Conexant Proprietary Zodiac Channel Status Message
		//*	$PRWIZCH identifies it as a Rockwell (ehr Connexant) Zodiac chipset based GPS receiver.
		//*	Field contains 12 pairs of a satellite PRN followed by signal quality number, 0-7
		case 'RWIZ':
		#ifdef _ENABLE_SATELLITE_ALMANAC_
			for (ii=0; ii<12; ii++)
			{
			int	satIdx;
			int	satValue;

				satIdx		=	atoi(nmeaArgs[(ii * 2) + 1].argString);
				satValue	=	atoi(nmeaArgs[(ii * 2) + 2].argString);
				if ((satIdx >= 0) && (satIdx < kMaxNumOfSatallites))
				theNmeaInfo->PRWIZCHval[satIdx]	=	satValue;
			}
		#endif
			break;

		//*	Jan 12,	2002	<KAS> Started adding Magellan Proprietary Sentenses
		//*	May  3,	2017	<MLS> Copying Magellan code from MacAPRS
		//	Magellan Propriatery Sentenses
		//	http://www.magellangps.com/PDFs/v10.pdf
		//	http://www.magellangps.com/PDFs/maggpsc.pdf
		case 'MGNT':
			CONSOLE_DEBUG_W_STR("Magellan", theLine);
			break;

		//	$PMGLB,03,165003,16,N,356342,4641097,1,3,006,000772,f,-112,f*26
		case 'MGLB':
			CONSOLE_DEBUG_W_STR("$PMGLB", theLine);
			tShort	=	atoi(nmeaArgs[1].argString);
			switch (tShort)
			{
				case 2:
					{
						strcpy(theNNptr->Alt,		nmeaArgs[2].argString);	//	Altitude
						strcpy(theNNptr->AltC,		nmeaArgs[3].argString);	//	Altitude Unit

						theNmeaInfo->validAlt	=	ReturnNMEAAltitude(theNNptr->Alt,theNNptr->AltC, &theNmeaInfo->zzAlt);
					}
					break;

				//	$PMGLB,03,165003,16,N,356342,4641097,1,3,006,000772,f,-112,f*26
				case 3:
					{
						strcpy(theNNptr->Time,		nmeaArgs[2].argString);

					//	strcpy(theNNptr->LonC,		nmeaArgs[3]);	//	Zone Number
					//	strcpy(theNNptr->LatC,		nmeaArgs[4]);	//	N or S Hemispher
					//	strcpy(theNNptr->Lat,		nmeaArgs[5]);	//	UTM Lat?
					//	strcpy(theNNptr->Lon,		nmeaArgs[6]);	//	UTM Lon?

						strcpy(theNNptr->Alt,		nmeaArgs[10].argString);	//	Altitude
						strcpy(theNNptr->AltC,		nmeaArgs[11].argString);	//	Altitude Unit

						theNmeaInfo->validAlt	=	ReturnNMEAAltitude(	theNNptr->Alt,
																		theNNptr->AltC,
																		&theNmeaInfo->zzAlt);
						theNmeaInfo->validTime	=	ReturnNMEATime(theNNptr->Time, &theNmeaInfo->gpsTime, &theNmeaInfo->gpsTimeHHMMSS);
					}
					break;
				default:
					break;
			}
			break;

		//*	no documentation as to what this is (5/4/2017)
		//$PMGLF,02,24,0,S,1,13,1,S,0,14,1,A,1,04,1,A,1,12,1,S,1,6*31
		//$PMGLF,02,24,1,9,1,16,1,9,1,07,0,9,1,09,1,9,1,29,1,8,1,6*59
		//$PMGLF,02,04,1,S,1,16,0,S,1,13,1,S,0,05,1,S,1,24,1,S,1,5*36
		case 'MGLF':
			break;


		default:
			CONSOLE_DEBUG_W_STR("Not handled", theLine);
			CONSOLE_DEBUG_W_STR("propiteryType", &theLine[2]);
			processedOK	=	false;
			break;

	}
	return(processedOK);
}
#endif

//**************************************************************************************
//*	returns true if valid string
//**************************************************************************************
bool	ParseNMEAstring(TYPE_NMEAInfoStruct *nmeaData, char *theNMEAstring)
{
bool			validString;
bool			checkSumOK;
short			nmeaChecksum;
short			calculatedChecksum;
int				lineLen;
TYPE_NMEAargs	nmeaArgs[kMaxNumNmeaArgs];
bool			processedOK;


#ifdef _ENABLE_TIME_DEGBUG_
	strcpy(gLastNMEAdata, theNMEAstring);
#endif

//	CONSOLE_DEBUG("");
//	CONSOLE_DEBUG_W_NUM("sizeof(bool)=", sizeof(bool));

	validString	=	false;
	checkSumOK	=	false;
	//*	make sure the line starts with a "$"
	if (theNMEAstring[0] == '$')
	{
		//*	evaluate the checksum
		lineLen	=	strlen(theNMEAstring);

		//*	some GPS's do not transmit checksums
		if (theNMEAstring[lineLen - 3] == '*')
		{
			calculatedChecksum	=	CalculateNMEACheckSum(theNMEAstring);
			nmeaChecksum		=	ExtractChecksumFromNMEAline(theNMEAstring);
			if (calculatedChecksum == nmeaChecksum)
			{
				checkSumOK	=	true;
			}
			else
			{
				checkSumOK	=	false;
			#if defined(_ENABLE_GPS_DEBUGGING_) || defined(_SHOW_CHECKSUM_ERRORS_)
				CONSOLE_DEBUG_W_STR("Checksum error:", theNMEAstring);
				CONSOLE_DEBUG_W_HEX("calculatedChecksum=", calculatedChecksum);
			#endif
				gNMEAcheckSumErrCnt++;
			}
		}
		else
		{
			//*	since it did NOT have a checksum, assume its OK
		//	checkSumOK	=	true;

			checkSumOK	=	false;
		#if defined(_ENABLE_GPS_DEBUGGING_) || defined(_SHOW_CHECKSUM_ERRORS_)
//			CONSOLE_DEBUG_W_STR("Checksum MISSING:", theNMEAstring);
		#endif
		}


		if (checkSumOK)
		{
			//*	now we have a known good NMEA string, lets tear it appart
			SeparateNMEAline(theNMEAstring, nmeaArgs);
		//	for (ii=0; ii<kMaxNumNmeaArgs; ii++)
		//	{
		//		if (strlen(nmeaArgs[ii].argString) > 0)
		//		{
		//			printf("arg %2d =%s\n", ii, nmeaArgs[ii].argString);
		//		}
		//	}

			processedOK	=	false;
			if (theNMEAstring[1] == 'G')
			{
				//======================================================================
				//*	$Gxxxx 		==> Normal sentences
				processedOK	=	ParseNMEAstringsNormal(	theNMEAstring,
														nmeaData,
														&nmeaData->theNN,
														nmeaArgs);

			}
		#ifdef _ENABLE_PROPRIETARY_PARSING_
			else if (theNMEAstring[1] == 'P')
			{
				//======================================================================
				//*	$Pxxx		==>	Propritary data
				processedOK	=	ParseNMEAstringsProprietary(theNMEAstring,
															nmeaData->,
															&nmeaData->theNN,
															nmeaArgs);

			}
		#endif

			if (processedOK)
			{
				nmeaData->validData	=	true;
				validString			=	true;
			#ifdef _ENABLE_GPS_DEBUGGING_
				printf("*******************************\n");
				printf("NMEA=%s\n", theNMEAstring);
			//	DumpGPSdata(&gNMEAdata);
			#endif
			}
			else
			{
		#ifdef _ENABLE_GPS_DEBUGGING_
				printf("GPS sentance not processed=%s\n", theNMEAstring);
		#endif
			}
		}
		else
		{
		#ifdef _ENABLE_GPS_DEBUGGING_
			printf("Checksum error =%s\n", theNMEAstring);
		#endif
		}
	}
	return(validString);
}

//**************************************************************************************
static void	Check_And_Set_System_Time(const char *theDateString, const char *theTimeString)
{
#if defined(__unix__)
struct timeval	tv;
struct timeval	gps_tv;
struct timezone	tz;
struct tm		myTm;
time_t			gpsSecsValue;
int				timeRetVal;
long			localTimeError;
char			newDateTimeString[64];

	myTm.tm_mday	=	Ascii2charsToInt(theDateString);		//*	day of the month
	myTm.tm_mon		=	Ascii2charsToInt(&theDateString[2]);	//*	month
	myTm.tm_year	=	Ascii2charsToInt(&theDateString[4]);	//*	year

	myTm.tm_year	+=	100;
	myTm.tm_mon		-=	1;


	myTm.tm_sec		=	Ascii2charsToInt(&theTimeString[4]);	//*	seconds
	myTm.tm_min		=	Ascii2charsToInt(&theTimeString[2]);	//*	minutes
	myTm.tm_hour	=	Ascii2charsToInt(theTimeString);		//*	hours


	myTm.tm_wday	=	0;	//*	day of the week
	myTm.tm_yday	=	0;	//*	day in the year
	myTm.tm_isdst	=	1;	//*	daylight saving time

	timeRetVal		=	gettimeofday(&tv, &tz);
	if (timeRetVal == 0)
	{

		myTm.tm_hour	-=	(tz.tz_minuteswest / 60);	//*	adjust for time zone
		myTm.tm_isdst	=	tz.tz_dsttime;


		gpsSecsValue	=	mktime(&myTm);

		localTimeError	=	tv.tv_sec - gpsSecsValue;

		if (localTimeError > 0)
		{
			CONSOLE_DEBUG_W_LONG("CPU clock ahead by =", localTimeError);
		}
		else if (localTimeError < 0)
		{
			CONSOLE_DEBUG_W_LONG("CPU clock behind by =", -localTimeError);
		}

		if (localTimeError != 0)
	//	if (abs(localTimeError) <  5)
		{
			gps_tv.tv_sec	=	gpsSecsValue;
			gps_tv.tv_usec	=	0;
			tz.tz_dsttime	=	1;
	      	timeRetVal		=	settimeofday(&gps_tv, &tz);
			if (timeRetVal == 0)
			{
				sprintf(newDateTimeString, "%02d/%02d/%04d %02d:%02d:%02d",
												(myTm.tm_mon + 1),
												myTm.tm_mday,
												(myTm.tm_year + 1900),
												myTm.tm_hour,
												myTm.tm_min,
												myTm.tm_sec);
				CONSOLE_DEBUG_W_STR("settimeofday OK", newDateTimeString);
			}
			else
			{
			int	saveErrno;

				saveErrno	=	errno;
				CONSOLE_DEBUG_W_NUM(	"settimeofday ERR, errno",	saveErrno);
				CONSOLE_DEBUG_W_LONG(	"gps_tv.tv_sec    \t=",		gps_tv.tv_sec);
				CONSOLE_DEBUG_W_LONG(	"gps_tv.tv_usec   \t=",		gps_tv.tv_usec);
				CONSOLE_DEBUG_W_NUM(	"tz.tz_minuteswest\t=",		tz.tz_minuteswest);
				CONSOLE_DEBUG_W_NUM(	"tz.tz_dsttime    \t=",		tz.tz_dsttime);
			}
		}
		else
		{
			CONSOLE_DEBUG("CPU time is correct");
		}
	//	CONSOLE_DEBUG_W_NUM("tv.tv_sec     =", tv.tv_sec);
	//	CONSOLE_DEBUG_W_NUM("gpsSecsValue  =", gpsSecsValue);
	//	CONSOLE_DEBUG_W_NUM("localTimeError=", localTimeError);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("gettimeofday error",	timeRetVal);
	}

#endif
}

//**************************************************************************************
//*	ParseNMEA_TimeString
//*		this routine parses ONLY time. This is meant to make time setting much faster
//*		Returns true if valid time
//*
//*	It is OK to pass all NMEA data to this routine, it will return false on all strings
//*	that do not contain time
//*
//*	Only parses $GPRMC strings
//**************************************************************************************
bool	ParseNMEA_TimeString(TYPE_NMEAInfoStruct *nmeaData, char *theNMEAstring, bool setSystemTime)
{
short			validTime;
short			calculatedChecksum;
short			nmeaChecksum;
int				lineLen;
int				ii;
char			theDateString[16];
char			theTimeString[16];
bool			allDigitsFlag;
//TYPE_timeHHMMSS	timeHHMMSS;
int				commaCntr;
int				slen;

	validTime	=	false;
	//*	make sure the line starts with a "$"
	if (theNMEAstring[0] == '$')
	{
		//*	evaluate the checksum
		lineLen	=	strlen(theNMEAstring);

		//*	some GPS's do not transmit checksums
		if (theNMEAstring[lineLen - 3] == '*')
		{
			calculatedChecksum	=	CalculateNMEACheckSum(theNMEAstring);
			nmeaChecksum		=	ExtractChecksumFromNMEAline(theNMEAstring);
			if (calculatedChecksum == nmeaChecksum)
			{
				//	$GPRMC,105211,A,4056.1873,N,07434.4805,W,0.015,324.3,120517,14.2,W*42
				if (strncmp(theNMEAstring, "$GPRMC", 6) == 0)
				{
					//*	we have a string that contains time and date
					slen	=	strlen(theNMEAstring);
					//*	go through and find the commas
					commaCntr	=	0;
					for (ii=0; ii<slen; ii++)
					{
						//	$GPRMC,105211,A,4056.1873,N,07434.4805,W,0.015,324.3,120517,14.2,W*42
						if (theNMEAstring[ii] == ',')
						{
							commaCntr++;
							switch(commaCntr)
							{
								case 1:	//*	arg 1 is the time
									strncpy(theTimeString, &theNMEAstring[ii + 1], 6);
									theTimeString[6]	=	0;
									break;

								case 9:	//*	arg 9 is the date
									strncpy(theDateString, &theNMEAstring[ii + 1], 6);
									theDateString[6]	=	0;
									break;
							}
						}
					}
					//*	make sure they are all digits
					allDigitsFlag	=	true;
					for (ii=0; ii<6; ii++)
					{
						if (isdigit(theDateString[ii]) == false)
						{
							allDigitsFlag	=	false;
							break;
						}
						if (isdigit(theTimeString[ii]) == false)
						{
							allDigitsFlag	=	false;
							break;
						}
					}

					//*	do we have all valid data
					if (allDigitsFlag)
					{
						//*	all data is valid, we can proceed
						validTime		=	true;
						if (setSystemTime)
						{
							Check_And_Set_System_Time(theDateString, theTimeString);
						}

						//*	update the global nmea data structures
						nmeaData->validTime	=	true;
						nmeaData->validDate	=	true;
						strcpy(nmeaData->theNN.Date, theDateString);
						strcpy(nmeaData->theNN.Time, theTimeString);
					}
					else
					{
					//	CONSOLE_DEBUG_W_STR("NOT", theNMEAstring);
					}
				}
			}
		}
	}
	return(validTime);
}



//**************************************************************************************
//*	this prints the internal format data as human readable values
void	Get_Actual_LatLon_Strings(long theLon, long theLat, char *theLonStr,char *theLatStr, bool includeLabel)
{
LatLonType	lat,lon;
long		localLat,localLon;

	if (theLon > kMaxLonValueConst)		lon.Ch	=	'E';		else	lon.Ch	=	'W';
	if (theLat > kMaxLatValueConst)		lat.Ch	=	'S';		else	lat.Ch	=	'N';

	localLon	=	labs(kMaxLonValueConst	-	theLon);
	localLat	=	labs(kMaxLatValueConst	-	theLat);

	lon.Deg	=	(localLon) / kOneDegree;
	lon.Min	=	(localLon - (lon.Deg * kOneDegree))/k600Integer;
	lon.Sec	=	(localLon - (lon.Deg * kOneDegree) - (lon.Min * k600Integer)) / kUnitsPerSecond;
	lon.Ten	=	0;

	lat.Deg	=	(localLat)/kOneDegree;
	lat.Min	=	(localLat - (lat.Deg * kOneDegree))/k600Integer;
	lat.Sec	=	(localLat - (lat.Deg * kOneDegree) - (lat.Min * k600Integer)) / kUnitsPerSecond;
	lat.Ten	=	0;

	if (includeLabel)
	{
		sprintf(theLonStr, "Lon %3ld %02ld' %02ld\"%c ", lon.Deg, lon.Min, lon.Sec, lon.Ch);
		sprintf(theLatStr, "Lat %3ld %02ld' %02ld\"%c ", lat.Deg, lat.Min, lat.Sec, lat.Ch);
	}
	else
	{
		sprintf(theLonStr, "%3ld %02ld' %02ld\"%c ", lon.Deg, lon.Min, lon.Sec, lon.Ch);
		sprintf(theLatStr, "%3ld %02ld' %02ld\"%c ", lat.Deg, lat.Min, lat.Sec, lat.Ch);
	}
}


//**************************************************************************************
void	GetCurrentLatLon_Strings(TYPE_NMEAInfoStruct *nmeaData, char *theLonStr,char *theLatStr, bool includeLabel)
{
	Get_Actual_LatLon_Strings(nmeaData->xxLon, nmeaData->yyLat, theLonStr, theLatStr, includeLabel);
}

#pragma mark -
#pragma mark NMEA sentance tracking
#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_

TYPE_NMEAsentance	gNMEAsentances[kMaxNMEAsentances];
int					gNMEAsentanceCnt	=	-1;

//**************************************************************************************
static  int QsortNMEAsentance(const void *e1, const void *e2)
{
int					returnValue;
TYPE_NMEAsentance	*rec1, *rec2;

	rec1		=	(TYPE_NMEAsentance *)e1;
	rec2		=	(TYPE_NMEAsentance *)e2;
	returnValue	=	strcmp(rec1->nmeaID, rec2->nmeaID);
	return(returnValue);
}


//**************************************************************************************
static void	NMEAtrack_Update(const unsigned long nmeaCode, const char *nmeaID, const char *fullString)
{
int		ii;
bool	keepLooking;

//	CONSOLE_DEBUG_W_HEX("nmeaCode", nmeaCode);
//	CONSOLE_DEBUG_W_STR("nmeaID", nmeaID);
//	CONSOLE_DEBUG_W_STR("fullString", fullString);

	//*	if the table is blank, zero it.
	if (gNMEAsentanceCnt < 0)
	{
		for (ii=0; ii<kMaxNMEAsentances; ii++)
		{
			memset(&gNMEAsentances[ii], 0, sizeof(TYPE_NMEAsentance));
		}
		gNMEAsentanceCnt	=	0;
	}

	if ((nmeaCode > 0) && (strlen(nmeaID) > 0))
	{
		//*	now see if we can find it in the table
		ii			=	0;
		keepLooking	=	true;
		while (keepLooking && (ii<gNMEAsentanceCnt))
		{
			if (nmeaCode == gNMEAsentances[ii].nmea4LetterCode)
			{
				//*	this looks promising
				if (strcmp(nmeaID, gNMEAsentances[ii].nmeaID) == 0)
				{
					//*	OK, its a match
					gNMEAsentances[ii].count			+=	1;
					strcpy(gNMEAsentances[ii].lastData,	fullString);

					keepLooking	=	false;
				}
			}
			ii++;
		}

		if (keepLooking)
		{
			if (gNMEAsentanceCnt < kMaxNMEAsentances)
			{
				//*	we didnt find it, put it in the next available slot
				gNMEAsentances[gNMEAsentanceCnt].nmea4LetterCode	=	nmeaCode;
				gNMEAsentances[gNMEAsentanceCnt].count				+=	1;
				strcpy(gNMEAsentances[gNMEAsentanceCnt].nmeaID,			nmeaID);
				strcpy(gNMEAsentances[gNMEAsentanceCnt].lastData,		fullString);

				gNMEAsentanceCnt++;

				if (gNMEAsentanceCnt > 1)
				{
					qsort((void *)gNMEAsentances, gNMEAsentanceCnt, sizeof(TYPE_NMEAsentance), QsortNMEAsentance);
				}
			}
			else
			{
				CONSOLE_DEBUG("Ran out of space in table");
			}
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Invalid data", nmeaID);
	}
}

#endif	//	_ENABLE_NMEA_SENTANCE_TRACKING_
