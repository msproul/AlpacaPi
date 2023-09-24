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
//*	Aug 31,	2023	<MLS> Added WriteFITS_GPSinfo()
//*	Sep  1,	2023	<MLS> Added lots of GPS data to FITS header
//*	Sep  5,	2023	<MLS> Working on matching SharpCap FITS header
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
#include	"ParseNMEA.h"

#ifdef _ENABLE_QHY_
	#include	<qhyccd.h>
#endif

#ifdef _ENABLE_FITS_
static char	gGPScomment1[]	=	"Data from Camera GPS (QHY174-GPS)";
//*****************************************************************************
void	CameraDriver::WriteFITS_GPSinfo(fitsfile *fitsFilePtr)
{
int		iii;
int		nnn;
int		fitsStatus;
char	tempstring[100];
char	latString[64];
char	lonString[64];

	WriteFITS_Seperator(fitsFilePtr, "GPS Info");
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											gGPScomment1,
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


	switch (cGPS.SatMode1)
	{
		case 'A':	strcpy(tempstring, "Automatic: ");	break;
		case 'M':	strcpy(tempstring, "Manual: ");	 	break;
		default:	strcpy(tempstring, "Unknown: ");	break;
	}

	switch (cGPS.SatMode2)
	{
		case '1':	strcat(tempstring, "Fix not available");	break;
		case '2':	strcat(tempstring, "2D Fix");	 			break;
		case '3':	strcat(tempstring, "3D Fix");	 			break;
		default:	strcat(tempstring, "Unknown: ");			break;
	}
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"GPS_MODE",
											tempstring,
											"GPS mode", &fitsStatus);
#define	ISLOCKED(myBoolvalue)	(char *)(myBoolvalue ? "Locked" : "invalid")
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

	//*	satellites in view
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TUINT,		"GPS_SVEW",
											&cGPS.SatsInView,
											"Satellites in view", &fitsStatus);
	//-------------------------------------------------------------
	CONSOLE_DEBUG_W_DBL("cGPS.Lat\t=", cGPS.Lat);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"GPS_LAT",
											&cGPS.Lat,
											"Latitude from GPS", &fitsStatus);

	CONSOLE_DEBUG_W_DBL("cGPS.Long\t=", cGPS.Long);
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
