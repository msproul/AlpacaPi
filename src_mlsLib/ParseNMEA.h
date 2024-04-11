//**************************************************************************************
//*	PODS 2.1 GPS processing code
//*
//*	(C) Picatinny Aresenal
//*	Written by Mark L. Sproul
//*
//**************************************************************************************
//*	Edit History
//**************************************************************************************
//*	Apr 30,	2012	<MLS> Structures created to hold gps data for parsing
//*	Sep 11,	2012	<MLS> Added validDate to NMEA structure
//*	Jan  2,	2013	<MLS> Added LatLonType to TYPE_NMEAInfoStruct
//*	Oct  3,	2016	<MLS> Added TYPE_timeHHMMSS
//*	Oct  3,	2016	<MLS> Added _ENABLE_SATELLITE_ALMANAC_ to save on memory if not needed
//*	Oct  6,	2016	<MLS> Added _ENABLE_WAYPOINT_PARSING_ to save on memory if not needed
//*	Oct  6,	2016	<MLS> Added _ENABLE_PROPRIETARY_PARSING_ to save on memory if not needed
//*	Apr 20,	2017	<MLS> Changing sat snr data from char strings to shorts (int)
//*	Apr 20,	2017	<MLS> Increased sat count from 32 to 96
//*	Apr 21,	2017	<MLS> Added _ENABLE_PDOP_TRACKING_
//*	Apr 24,	2017	<MLS> Added _ENABLE_NMEA_SENTANCE_TRACKING_
//*	May  9,	2017	<MLS> Added gPGRME_exists to indicate that a PGRME sentance had been received
//*	Sep  5,	2023	<MLS> Added	currSatsInUse & currSatMode
//*	Apr  9,	2024	<MLS> Added double values for Lat/Lon to TYPE_NMEAInfoStruct
//*	Apr 10,	2024	<MLS> Added _ENABLE_GPS_AVERAGE_
//**************************************************************************************
//*	memory used with _ENABLE_SATELLITE_ALMANAC_ enabled
//*		sizeof(TYPE_NMEAInfoStruct)     =1677
//*
//*	memory used with _ENABLE_SATELLITE_ALMANAC_ disabled
//*		sizeof(TYPE_NMEAInfoStruct)     =653
//**************************************************************************************

//#include	"ParseNMEA.h"

#ifndef _PARSE_NMEA_H_
#define	_PARSE_NMEA_H_


//**************************************************************************************
//*	configuration options, comment out to save memory
//#define	_ENABLE_ALTITUDE_TRACKING_
#define	_ENABLE_GPS_AVERAGE_
//#define	_ENABLE_LAT_LON_TRACKING_
//#define	_ENABLE_NMEA_SENTANCE_TRACKING_
//#define	_ENABLE_NMEA_POSITION_ERROR_TRACKING_
//#define	_ENABLE_PROPRIETARY_PARSING_
#define	_ENABLE_SATELLITE_ALMANAC_
//#define	_ENABLE_WAYPOINT_PARSING_
//**************************************************************************************



#ifndef __cplusplus
	#include	<stdbool.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#define		kSatDataLen			8
#define		kMaxNumOfSatallites	96
#define		kNMEAnameLen		16
#define		kGPSmodelNameLen	64
#define		kMaxSNRvalue		101

	#define	kMaxLonValueConst	(64800000L)
	#define	kMaxLatValueConst	(32400000L)

	#define	kMaxLonValueWorld	(129600000L)
	#define	kMaxLatValueWorld	(64800000L)



	#define	kOneDegree			(360000L)
	#define	kOneDegreeFloat		(360000.0)
	#define	kTwoDegrees			(2 * 360000L)
	#define	kFiveDegrees		(5 * 360000L)
	#define	kTenDegrees			(10 * 360000L)
	#define	kUnitsPerMinute		6000
	#define	kUnitsPerSecond		100

	//	11/27/97	.Ten is now actually HUNDREDTH
	#define	kUniterPerTenth		1			//*	Feb 2, 1999  MLS and KAS confirmed this is supposed to be value of 1

	#define	k600Integer			6000

	//*	Keith's fudge factors for lat vs lon for central US
	#define	k691		6910
	#define	k691float	6910.0
	#define	k530		5300
	#define	k593float	5930.0

	//	360 * 60 * 60 * 100 / (2 * pi)
	#define	ktSecs2Rad	(20626480.62471)

	#define	kTimesTenFactor	10

//*****************************************************************************
typedef	struct
{
	long	Deg;
	long	Min;
	long	Sec;
	long	Ten;
	char	Ch;
}	LatLonType;

#ifdef _ENABLE_SATELLITE_ALMANAC_
//*****************************************************************************
//*	sat position data
typedef	struct
{
	unsigned long	reportCnt;			//*	number of times it has been reported
	short			satellitePRN;		//*	SV PRN number	(Pseudo Random Noise)
	short			elvevation;			//*	Elevation in degrees, 90 maximum
	short			azimuth;			//*	Azimuth, degrees from true north, 000 to 359
	short			signal2Noise;		//*	Signal to Noise Ratio, 00-99 dB (null when not tracking)
	short			maxSNR;				//*	max SNR seen on this sat

	char			prn[kSatDataLen];	//*	SV PRN number	(Pseudo Random Noise)
} TYPE_SatStatsStruct;
#endif

//*****************************************************************************
//	By having this stuff in a structure, it allows a single memset to clear the entire set
typedef	struct
{
	char	Lat[kNMEAnameLen];
	char	LatC[kNMEAnameLen];			//*	latitude
	char	Lon[kNMEAnameLen];
	char	LonC[kNMEAnameLen];			//*	longitude
	char	Alt[kNMEAnameLen];			//*	Altitude (in meters)
	char	AltC[kNMEAnameLen];			//*	Unit Character M or F
	char	Sat[kNMEAnameLen];			//*	Satilite Flag
	char	NumSats[kNMEAnameLen];		//*	Number of Satalites
	char	Date[kNMEAnameLen];			//*	date
	char	Time[kNMEAnameLen];			//*	time

	char	HDOP[kNMEAnameLen];
	char	VDOP[kNMEAnameLen];
	char	PDOP[kNMEAnameLen];
	char	TDOP[kNMEAnameLen];
	char	GDOP[kNMEAnameLen];

#ifdef _ENABLE_WAYPOINT_PARSING_
	char	WayPtLat[kNMEAnameLen];
	char	WayPtLatC[kNMEAnameLen];
	char	WayPtLon[kNMEAnameLen];
	char	WayPtLonC[kNMEAnameLen];
	char	WayPtBerT[kNMEAnameLen];
	char	WayPtBerTC[kNMEAnameLen];
	char	WayPtBerM[kNMEAnameLen];
	char	WayPtBerMC[kNMEAnameLen];
	char	WayPtDist[kNMEAnameLen];
	char	WayPtDistC[kNMEAnameLen];

	char	XTE[kNMEAnameLen];
	char	DirToSteer[kNMEAnameLen];
	char	OrigWP[kNMEAnameLen];
	char	DestWP[kNMEAnameLen];
#endif


	char	SpdOvrGrnd[kNMEAnameLen];					//	Speed Over Ground
	char	SpdOvrGrndC[kNMEAnameLen];
	char	TMG[kNMEAnameLen];
	char	MagVar[kNMEAnameLen];
	char	MagVarC[kNMEAnameLen];

#ifdef _ENABLE_SATELLITE_ALMANAC_
	char	Mode[kNMEAnameLen];
#endif

//?	char	Temp[kNMEAnameLen];
}	TYPE_NMEAnameStruct;

//**************************************************************************
typedef	struct
{
	short	hours;
	short	minutes;
	short	seconds;

} TYPE_timeHHMMSS;

//*	15 minute interval
#define	kLoggingIntervalMinutes	10

#ifdef _ENABLE_PDOP_TRACKING_
	#define	kPDOPtacking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kPDOPtacking_ArraySize	((24 * 60 * 60)	/ kPDOPtacking_deltaTime)
#endif


#ifdef _ENABLE_LAT_LON_TRACKING_
	#define	kLatLonTacking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kLatLonTacking_ArraySize	((24 * 60 * 60)	/ kLatLonTacking_deltaTime)
#endif

#ifdef _ENABLE_ALTITUDE_TRACKING_
	#define	kAltTacking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kAltTacking_ArraySize	((24 * 60 * 60)	/ kAltTacking_deltaTime)
#endif

#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
	#define	kPosErrTacking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kPosErrTacking_ArraySize	((24 * 60 * 60)	/ kPosErrTacking_deltaTime)
#endif

#ifdef _ENABLE_SATELLITE_ALMANAC_
	#define	kSatsInUseTracking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kSatsInUseTracking_ArraySize	((24 * 60 * 60)	/ kSatsInUseTracking_deltaTime)
#endif

#ifdef _ENABLE_MAGNETIC_VARIATION_TRACKING_
	#define	kMagVariationTracking_deltaTime	(kLoggingIntervalMinutes * 60)
	#define	kMagVariationTracking_ArraySize	((24 * 60 * 60)	/ kMagVariationTracking_deltaTime)
#endif

#define	kLatLonAvgCnt	500

//**************************************************************************
typedef	struct
{
	bool				validData;
	bool				validTime;
	bool				validDate;
	bool				validLatLon;
	bool				validAlt;
	bool				validCseSpd;
	bool				validWayPointLatLon;
//	short				validCourse;		//	Deleted May 20, 2002. Do the checking in ParseRDF
	unsigned long		gpsTime;			//	Time in seconds since midnight (GMT)
	TYPE_timeHHMMSS		gpsTimeHHMMSS;		//*	Time in HH:MM:SS - added Oct 3, 2016
	unsigned long		deltaGPSTime;
	LatLonType			latitude;
	LatLonType			longitude;
	double				lat_double;
	double				lon_double;
	double				altitudeFeet;
	double				altitudeMeters;
#ifdef _ENABLE_GPS_AVERAGE_
	double				lat_AverageArray[kLatLonAvgCnt];
	double				lon_AverageArray[kLatLonAvgCnt];
	int					latLonAvgIndex;
	int					latLonAvgCount;
#endif

	short				cse;				//	Degrees
	short				spd;				//	MPH
//?	short				hdg;				//*	Heading
	short				dgps;				//*	Differential GPS indicator
//?	short				vSpd;
	char				gpsModel[kGPSmodelNameLen];
	char				mapDatum[kGPSmodelNameLen];
	int					gpsSensorTemp;		//*	contained in $PGRMT
	TYPE_NMEAnameStruct	theNN;

	char				currSatMode1;
	char				currSatMode2;
	int					currSatsInUse;
	int					numSats;

#ifdef _ENABLE_SATELLITE_ALMANAC_
	long				nmeaGPGSVcount;
	TYPE_SatStatsStruct	theSats[kMaxNumOfSatallites];
	short				PRWIZCHval[kMaxNumOfSatallites];
	long				snrDistribution[kMaxSNRvalue];
	short				satsInUse[kSatsInUseTracking_ArraySize];
	short				satMode[kSatsInUseTracking_ArraySize];
#endif
#ifdef _ENABLE_PDOP_TRACKING_
	double				pdopHistory[kPDOPtacking_ArraySize];
	double				vdopHistory[kPDOPtacking_ArraySize];
	double				hdopHistory[kPDOPtacking_ArraySize];
#endif

#ifdef _ENABLE_LAT_LON_TRACKING_
	double				latitudeHistory[kLatLonTacking_ArraySize];
	double				longitudeHistory[kLatLonTacking_ArraySize];
#endif

#ifdef _ENABLE_ALTITUDE_TRACKING_
	double				altitudeHistory[kAltTacking_ArraySize];
#endif

	double				horizontalPosErr;
	double				verticalPosErr;
	double				sphericalPosErr;
	double				magneticVariation;

#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
	bool				gPGRME_exists;
	double				horzPosErrArry[kPosErrTacking_ArraySize];
	double				vertPosErrArry[kPosErrTacking_ArraySize];
	double				sphrPosErrArry[kPosErrTacking_ArraySize];
#endif

#ifdef _ENABLE_MAGNETIC_VARIATION_TRACKING_
	double				magVariationArray[kMagVariationTracking_ArraySize];
#endif


	//*	for MacAPRS
	long				xxLon;				//	Internal Units (100'ts of an arc second)
	long				yyLat;				//	Internal Units (100'ts of an arc second)
	long				zzAlt;				//	Feet
//-	short				whichIcon;

}	TYPE_NMEAInfoStruct;

extern TYPE_NMEAInfoStruct	gNMEAdata;


#ifdef _ENABLE_SATELLITE_ALMANAC_
//	int	GetSatSignalStrength(int satNumber);
	int	GetSatSignalStrength(TYPE_NMEAInfoStruct *nmeaData, int satNumber);
#endif


void	ParseNMEA_init(TYPE_NMEAInfoStruct *nmeaData);
bool	ParseNMEAstring(TYPE_NMEAInfoStruct *nmeaData, char *theNMEAstring);
//bool	ParseNMEA_TimeString(char *theNMEAstring, bool setSystemTime);
bool	ParseNMEA_TimeString(TYPE_NMEAInfoStruct *nmeaData, char *theNMEAstring, bool setSystemTime);

void	Get_Actual_LatLon_Strings(long theLon, long theLat, char *theLonStr,char *theLatStr, bool includeLabel);
int		Ascii2charsToInt(const char *charPtr);
//void	GetCurrentLatLon_Strings(char *theLonStr,char *theLatStr, bool includeLabel);
void	GetCurrentLatLon_Strings(TYPE_NMEAInfoStruct *nmeaData, char *theLonStr,char *theLatStr, bool includeLabel);
int		GetMaxSatSignalStrength(void);
void	DumpGPSdata(TYPE_NMEAInfoStruct	*theNmeaInfo);
double	GetLatLonDouble(LatLonType *latLonPtr);
void	ParseNMEA_FormatLatLonStrings(double latValue, char *latString, double lonValue, char *lonString);



#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_

#define	kNMEAstringLenMax	128
//*****************************************************************************
//*	this allows us to keep track of which NMEA statements have been seen and how many times
//*****************************************************************************
typedef	struct
{
	unsigned long	nmea4LetterCode;	//*	used as quick searching, skips the first char
	unsigned long	count;
	char			nmeaID[16];
	char			lastData[kNMEAstringLenMax];
} TYPE_NMEAsentance;

#define	kMaxNMEAsentances	50
extern	TYPE_NMEAsentance	gNMEAsentances[kMaxNMEAsentances];

#endif



#ifdef __cplusplus
}
#endif


#endif	//	_PARSE_NMEA_H_
