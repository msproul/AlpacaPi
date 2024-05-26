//**************************************************************************
//*	Name:			cameradriver_gps.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	GPS data handling for cameras with built in GPS
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Aug 31,	2023	<MLS> Created cameradriver_gps.cpp
//*	Aug 31,	2023	<MLS> Added WriteFITS_QHY_GPSinfo()
//*	Sep  1,	2023	<MLS> Added lots of GPS data to FITS header
//*	Sep  5,	2023	<MLS> Working on matching SharpCap FITS header
//*	Apr 10,	2024	<MLS> Added WriteFITS_GPSinfo()
//*	Apr 10,	2024	<MLS> Added WriteFITS_Global_GPSinfo()
//*****************************************************************************
//*	data from SharpCap FITS header
//+GPS_W		1936			Width
//+GPS_H		1024			Height
//GPS_LAT		41.1711			Latitude
//GPS_LONG		-73.3275		Longitude
//+GPS_SFLG		51				StartFlag
//GPS_ST		2023-09-02T09:00:41.0000000Z		StartShutterTime
//GPS_SU		44771.2			StartShutterMicroSeconds
//+GPS_EFLG		51				EndFlag
//GPS_ET		2023-09-02T09:00:49.0000000Z		EndShutterTime
//GPS_NT		2023-09-02T09:00:49.0000000Z		NowShutterTime
//+GPS_NFLG		51				NowFlag
//+GPS_TMP		6				TempSequenceNum
//GPS_NU		44773.8			NowShutterMicroSeconds
//GPS_PPSC		9999888			PPSCounter
//GPSSTAT		Locked			GPS	Status
//GPS_EXPU		8000002.6		Exposure (microseconds)
//GPS_DSYS		0.0083211		System clock - GPS clock offset (s)
//+GPS_DSTB		680				Time offset	stable for (s)
//GPS_ALT		0				Altitude (m)
//GPS_EU		44773.8			EndShutterMicroSeconds
//GPS_SEQ		81528			Sequence Number
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"cameradriver.h"

#ifdef _PARSE_NMEA_H_
	#warning	"_PARSE_NMEA_H_ already included"
#endif

#ifdef _ENABLE_GLOBAL_GPS_
	#ifndef _PARSE_NMEA_H_
		#include	"ParseNMEA.h"
	#endif
#endif // _ENABLE_GLOBAL_GPS_

#ifdef _ENABLE_QHY_
	#include	<qhyccd.h>
#endif

#ifdef _ENABLE_FITS_
#define	ISLOCKED(myBoolvalue)	(char *)(myBoolvalue ? "Locked" : "invalid")

//*****************************************************************************
//*	does not write anything if no GPS present
//*	QHY GPS takes priority over global GPS
//*****************************************************************************
void	CameraDriver::WriteFITS_GPSinfo(fitsfile *fitsFilePtr)
{
	if (cGPS.Present)
	{
		WriteFITS_QHY_GPSinfo(fitsFilePtr);
	}
#ifdef _ENABLE_GLOBAL_GPS_
	else if (gNMEAdata.SequenceNumber > 0)
	{
		WriteFITS_Global_GPSinfo(fitsFilePtr);
	}
#endif
}


#if !defined(_ENABLE_GLOBAL_GPS_) && !defined(_ENABLE_QHY_)
//*****************************************************************************
void	GetGPSmodeString(char gpsMode1, char gpsMode2, char *modeString)
{
	modeString[0]	=	0;
	switch (gpsMode1)
	{
		case 'A':	strcpy(modeString, "Automatic: ");	break;
		case 'M':	strcpy(modeString, "Manual: ");	 	break;
		default:	strcpy(modeString, "Unknown: ");	break;
	}

	switch (gpsMode2)
	{
		case '1':	strcat(modeString, "Fix not available");	break;
		case '2':	strcat(modeString, "2D Fix");	 			break;
		case '3':	strcat(modeString, "3D Fix");	 			break;
		default:	strcat(modeString, "Unknown: ");			break;
	}
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
#endif // _ENABLE_GLOBAL_GPS_


#ifdef _ENABLE_GLOBAL_GPS_

//*****************************************************************************
void	CameraDriver::WriteFITS_Global_GPSinfo(fitsfile *fitsFilePtr)
{
int		fitsStatus;
char	latString[64];
char	lonString[64];
char	tempstring[100];

	WriteFITS_Seperator(fitsFilePtr, "GPS Info");
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											(char *)"Data from GPS via Serial Port",
											NULL, &fitsStatus);
#ifdef _ENABLE_GPS_AVERAGE_
	fitsStatus	=	0;
	sprintf(tempstring, "GPS lat/lon/alt values are averaged over %d minutes", (kLatLonAvgCnt / 60));
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);
#endif // _ENABLE_GPS_AVERAGE_

	//-------------------------------------------------------------
	GetGPSmodeString(gNMEAdata.currSatMode1, gNMEAdata.currSatMode2, tempstring);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_MODE",
											tempstring,
											"GPS mode", &fitsStatus);

	//-------------------------------------------------------------
	//*	GPS status
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_STAT",
											ISLOCKED(gNMEAdata.validData),
											"GPS Status", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_DATE",
											ISLOCKED(gNMEAdata.validDate),
											"Date Status", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_TIME",
											ISLOCKED(gNMEAdata.validTime),
											"Time Status", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_LALO",
											ISLOCKED(gNMEAdata.validLatLon),
											"Lat/Lon Status", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_ALTS",
											ISLOCKED(gNMEAdata.validAlt),
											"Altitude Status", &fitsStatus);

	//--------------------------------------------------------------
	//*	Sequence number
	fitsStatus	=	0;
#ifdef _ENABLE_GPS_AVERAGE_
	fits_write_key(fitsFilePtr, TINT,		"GPS_SEQ",
											&gNMEAdata.latLonAvgCount,
											"Sequence Number", &fitsStatus);
#else
	fits_write_key(fitsFilePtr, TINT,		"GPS_SEQ",
											&gNMEAdata.SequenceNumber,
											"Sequence Number", &fitsStatus);
#endif // _ENABLE_GPS_AVERAGE_

	//-------------------------------------------------------------
	//*	satellites in view
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TINT,		"GPS_SVEW",
											&gNMEAdata.numSats,
											"Satellites in view", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_LAT",
											&gNMEAdata.lat_double,
											"Latitude from GPS", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_LONG",
											&gNMEAdata.lon_double,
											"Longitude from GPS", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_ALT",
							#ifdef _ENABLE_GPS_AVERAGE_
											&gNMEAdata.alt_average,
							#else
											&gNMEAdata.altitudeMeters,
							#endif
											"Altitude from GPS (meters)", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_ALTF",
											&gNMEAdata.altitudeFeet,
											"Altitude from GPS (feet)", &fitsStatus);

	//-------------------------------------------------------------
	ParseNMEA_FormatLatLonStrings(	gNMEAdata.lat_average,
									latString,
									gNMEAdata.lon_average,
									lonString);
	strcpy(tempstring, latString);
	strcat(tempstring, " / ");
	strcat(tempstring, lonString);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_TEXT",
											tempstring,
											NULL, &fitsStatus);

	//-------------------------------------------------------------
	FormatTimeStringISO8601_tm(&gNMEAdata.linuxTime, tempstring);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_DTTM",
											tempstring,
											"Date/Time", &fitsStatus);
}

//$GPGGA,023420.000,4121.6625,N,07458.8289,W,1,08,0.95,437.3,M,-34.0,M,,*50
//$GPGSA,A,3,29,10,05,18,23,13,24,15,,,,,1.25,0.95,0.80*07
//$GPRMC,023420.000,A,4121.6625,N,07458.8289,W,0.53,2.19,110424,,,A*71
//$GPVTG,2.19,T,,M,0.53,N,0.99,K,A*31
//$GPGGA,023421.000,4121.6626,N,07458.8289,W,1,08,0.97,437.3,M,-34.0,M,,*50
//$GPGSA,A,3,29,10,05,18,23,13,24,15,,,,,1.67,0.97,1.36*0F
//$GPRMC,023421.000,A,4121.6626,N,07458.8289,W,0.61,358.75,110424,,,A*74
//$GPVTG,358.75,T,,M,0.61,N,1.14,K,A*32
//$GPGGA,023422.000,4121.6628,N,07458.8290,W,1,08,0.97,437.3,M,-34.0,M,,*55
//$GPGSA,A,3,29,10,05,18,23,13,24,15,,,,,1.67,0.97,1.36*0F
//$GPGSV,3,1,10,18,77,244,17,15,61,049,26,23,50,305,22,24,44,144,24*7E
//$GPGSV,3,2,10,13,29,048,11,05,22,093,16,10,17,289,16,27,13,313,16*76
//$GPGSV,3,3,10,29,09,200,14,32,02,233,*7C
//$GPRMC,023422.000,A,4121.6628,N,07458.8290,W,0.36,33.21,110424,,,A*4C
//$GPVTG,33.21,T,,M,0.36,N,0.66,K,A*0B
//$GPGGA,023423.000,4121.6630,N,07458.8291,W,1,08,0.95,437.3,M,-34.0,M,,*5E
//$GPGSA,A,3,29,10,05,18,23,13,24,15,,,,,1.25,0.95,0.80*07
//$GPRMC,023423.000,A,4121.6630,N,07458.8291,W,0.24,42.09,110424,,,A*4A
//$GPVTG,42.09,T,,M,0.24,N,0.45,K,A*05
//$GPGGA,023424.000,4121.6631,N,07458.8293,W,1,08,0.95,437.3,M,-34.0,M,,*5A
//$GPGSA,A,3,29,10,05,18,23,13,24,15,,,,,1.25,0.95,0.80*07
//$GPRMC,023424.000,A,4121.6631,N,07458.8293,W,0.16,87.74,110424,,,A*4C
//$GPVTG,87.74,T,,M,0.16,N,0.29,K,A*0D
//$GPGGA,023425.000,4121.6634,N,07458.8295,W,1,09,0.87,437.3,M,-34.0,M,,*5A
//$GPGSA,A,3,29,10,05,18,23,27,13,24,15,,,,1.17,0.87,0.78*07
//$GPRMC,023425.000,A,4121.6634,N,07458.8295,W,0.30,67.18,110424,,,A*4E
//$GPVTG,67.18,T,,M,0.30,N,0.56,K,A*05
//$GPGGA,023426.000,4121.6634,N,07458.8294,W,1,09,0.87,437.3,M,-34.0,M,,*58
//$GPGSA,A,3,29,10,05,18,23,27,13,24,15,,,,1.17,0.87,0.78*07
//$GPRMC,023426.000,A,4121.6634,N,07458.8294,W,0.32,105.16,110424,,,A*75
//$GPVTG,105.16,T,,M,0.32,N,0.59,K,A*33
//$GPGGA,023427.000,4121.6634,N,07458.8293,W,1,09,0.87,437.3,M,-34.0,M,,*5E
//$GPGSA,A,3,29,10,05,18,23,27,13,24,15,,,,1.17,0.87,0.78*07
//$GPGSV,3,1,11,18,77,244,18,15,61,049,26,23,50,305,22,24,44,144,26*72
//$GPGSV,3,2,11,13,29,048,14,05,22,093,16,10,17,289,16,27,13,313,16*72
//$GPGSV,3,3,11,29,09,200,14,32,02,233,14,37,,,*7C
//$GPRMC,023427.000,A,4121.6634,N,07458.8293,W,0.24,70.83,110424,,,A*4B
#endif // _ENABLE_GLOBAL_GPS_



static char	gGPS_QHYcomment1[]	=	"Data from Camera GPS (QHY174-GPS)";
//*****************************************************************************
void	CameraDriver::WriteFITS_QHY_GPSinfo(fitsfile *fitsFilePtr)
{
int		iii;
int		nnn;
int		fitsStatus;
char	tempstring[100];
char	latString[64];
char	lonString[64];

	WriteFITS_Seperator(fitsFilePtr, "QHY GPS Info");
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											gGPS_QHYcomment1,
											NULL, &fitsStatus);
	sprintf(tempstring, "GPS Camera Model: %s", cGPS.CameraName);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);

	sprintf(tempstring, "GPS Camera Library version: %s", cDeviceVersion);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);

	sprintf(tempstring, "GPS Camera Firmware Version: %s", cDeviceFirmwareVersStr);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);

	sprintf(tempstring, "GPS Camera FPGA Version: %s", cGPS.FPGAversion);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);
#ifdef _ENABLE_QHY_
	#ifdef SDK_SVN_REVISION
//	CONSOLE_DEBUG_W_NUM("SDK_SVN_REVISION\t=", SDK_SVN_REVISION);
	sprintf(tempstring, "GPS QHY library SDK_SVN_REVISION: %d", SDK_SVN_REVISION);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											tempstring,
											NULL, &fitsStatus);
	#endif
#endif


//	switch (cGPS.SatMode1)
//	{
//		case 'A':	strcpy(tempstring, "Automatic: ");	break;
//		case 'M':	strcpy(tempstring, "Manual: ");	 	break;
//		default:	strcpy(tempstring, "Unknown: ");	break;
//	}
//
//	switch (cGPS.SatMode2)
//	{
//		case '1':	strcat(tempstring, "Fix not available");	break;
//		case '2':	strcat(tempstring, "2D Fix");	 			break;
//		case '3':	strcat(tempstring, "3D Fix");	 			break;
//		default:	strcat(tempstring, "Unknown: ");			break;
//	}
	GetGPSmodeString(cGPS.SatMode1, cGPS.SatMode2, tempstring);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_MODE",
											tempstring,
											"GPS mode", &fitsStatus);
	//-------------------------------------------------------------
	//*	GPS status
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_STAT",
											ISLOCKED(cGPS.Status),
											"GPS Status", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_DATE",
											ISLOCKED(cGPS.DateValid),
											"Date Status", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_TIME",
											ISLOCKED(cGPS.TimeValid),
											"Time Status", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_LALO",
											ISLOCKED(cGPS.LaLoValid),
											"Lat/Lon Status", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_ALTS",
											ISLOCKED(cGPS.AltValid),
											"Altitude Status", &fitsStatus);

	//--------------------------------------------------------------
	//*	QHY Sequence number
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_SEQ",
											&cGPS.SequenceNumber,
											"Sequence Number", &fitsStatus);

	//-------------------------------------------------------------
	//*	satellites in view
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_SVEW",
											&cGPS.SatsInView,
											"Satellites in view", &fitsStatus);
	//-------------------------------------------------------------
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_LAT",
											&cGPS.Lat,
											"Latitude from GPS", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_LONG",
											&cGPS.Long,
											"Longitude from GPS", &fitsStatus);

	//-------------------------------------------------------------
	ParseNMEA_FormatLatLonStrings(cGPS.Lat, latString, cGPS.Long, lonString);
	strcpy(tempstring, latString);
	strcat(tempstring, " / ");
	strcat(tempstring, lonString);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_TEXT",
											tempstring,
											NULL, &fitsStatus);


	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_ALT",
											&cGPS.Altitude,
											"Altitude from GPS (meters)", &fitsStatus);

	//-------------------------------------------------------------
	//*	shutter Start Time
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_ST",
											cGPS.ShutterStartTimeStr,
											"Shutter Start Time", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_SU",
											&cGPS.SU,
											"Start Shutter Micro Seconds", &fitsStatus);
	//*	shutter End Time
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_ET",
											cGPS.ShutterEndTimeStr,
											"Shutter End Time", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_EU",
											&cGPS.EU,
											"End Shutter Micro Seconds", &fitsStatus);
	//*	exposure time
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_EXPU",
											&cGPS.Exposure_us,
											"Exposure (microseconds)", &fitsStatus);
	//----------------------------------------------------
	//*	Now Time
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_NT",
											cGPS.NowTimeStr,
											"Now Time", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_NU",
											&cGPS.NU,
											"Now Shutter Micro Seconds", &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_DSYS",
											&cGPS.ClockDeltaSecs,
											"System clock - GPS clock offset (s)", &fitsStatus);

	//*	Pulse Per Second counter
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_PPSC",
											&cGPS.PPSC,
											"Pulse Per Second counter", &fitsStatus);

	//-----------------------------------------------------------------
	//*	Do we have any NMEA data to print insert into the FITS header
	if (cGPS.NMEAdataIdx > 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"GPS:NM short for \"NMEA\" = National Marine Electronics Association",
												NULL, &fitsStatus);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"NOTE: Some of the GPS/NMEA data maybe truncated due to FITS limits",
												NULL, &fitsStatus);
		nnn	=	1;
		for (iii=0; iii<cGPS.NMEAdataIdx; iii++)
		{
			if (strlen(cGPS.NMEAdata[iii].nmeaString) > 0)
			{
				sprintf(tempstring, "GPS_NM%02d", nnn);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	tempstring,
														cGPS.NMEAdata[iii].nmeaString,
														NULL, &fitsStatus);
				nnn++;
			}
		}
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TUINT,		"GPS_NMGC",
												&cGPS.NMEAdataIdx,
												"NMEA Good sentence Count", &fitsStatus);

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TUINT,		"GPS_NMEC",
												&cGPS.NMEAerrCnt,
												"NMEA checksum Error Count", &fitsStatus);
	}
	else
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"No NMEA data available",
												NULL, &fitsStatus);
	}
}
#endif // _ENABLE_FITS_

//*****************************************************************************
void	CameraDriver::GPS_ResetNMEAbuffer(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<kMaxNMEAstrings; iii++)
	{
		cGPS.NMEAdata[iii].nmeaString[0]	=	0;
	}
	cGPS.NMEAdataIdx	=	0;
	cGPS.NMEAerrCnt		=	0;
}

//*****************************************************************************
void	CameraDriver::GPS_AddNMEAstring(const char *nmeaString)
{
	if (cGPS.NMEAdataIdx < kMaxNMEAstrings)
	{
		if (strlen(nmeaString) < kMaxNMEAlen)
		{
			strcpy(cGPS.NMEAdata[cGPS.NMEAdataIdx].nmeaString, nmeaString);
			cGPS.NMEAdataIdx++;
		}
	}
}
