//**************************************************************************
//*	Name:			alpacadriver_gps.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2023
//*					msproul@skychariot.com
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul msproul@skychariot.com
//*****************************************************************************
//*	Apr 15,	2024	<MLS> Created alpacadriver_gps.cpp
//*	Apr 15,	2024	<MLS> Added SendHtml_GPS()
//*	Apr 28,	2024	<MLS> Added PrintLatLonStatsTable()
//*	Apr 29,	2024	<MLS> Added PrintNMEA_SentanceTable()
//*****************************************************************************


#include	<sys/stat.h>


#define	_ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#ifndef _REQUESTDATA_H_
	#include	"RequestData.h"
#endif


#include	"alpacadriver.h"
#include	"alpacadriver_gps.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"

#include	"gps_data.h"
#include	"ParseNMEA.h"
#include	"GPS_graph.h"

#ifdef _ENABLE_GLOBAL_GPS_

bool		gEnableGlobalGPS	=	true;
char		gGlobalGPSbaudrate	=	'9';
char		gGlobalGPSpath[64]	=	"/dev/ttyS0";

//*****************************************************************************
static void	PrintHTMLtableEntry(int mySocketFD, const char *string1, const char *string2)
{
char	lineBuffer[256];

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer, "<TD>%s</TD><TD>%s</TD>", string1, string2);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");
}

//*****************************************************************************
static void	PrintHTMLtableEntryINT(int mySocketFD, const char *string1, const int value)
{
char	string2[256];

	sprintf(string2, "%d", value);
	PrintHTMLtableEntry(mySocketFD, string1, string2);
}

//*****************************************************************************
static void	PrintHTMLtableEntryDBL(int mySocketFD, const char *string1, const double value)
{
char	string2[256];

	sprintf(string2, "%10.9f", value);
	PrintHTMLtableEntry(mySocketFD, string1, string2);
}

//*****************************************************************************
static void	PrintHTMLgraphTableEntry(	int			mySocketFD,
										const char	*titleString,
										const char	*directoryString,
										const char	*fileNameString)
{
char	lineBuffer[256];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("titleString    \t=", titleString);
//	CONSOLE_DEBUG_W_STR("fileNameString \t=", fileNameString);
//	CONSOLE_DEBUG_W_STR("directoryString\t=", directoryString);


	sprintf(lineBuffer,	"<TR><TH>%s</TH></TR>\r\n", titleString);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer, "<TD><IMG SRC=%s/%s></TD>\r\n", directoryString, fileNameString);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");
}

//**************************************************************************************
void	PrintHTMLtableCell(int mySocketFD, const char *cellText)
{
char	lineBuffer[256];

	sprintf(lineBuffer, "\t\t<TD>%s</TD>\r\n\n", cellText);
	SocketWriteData(mySocketFD,	lineBuffer);
}

//**************************************************************************************
//*	from HTMLoutput.h
enum
{
		kFormat_6_0	=	0,
		kFormat_6_1,
		kFormat_6_2,
		kFormat_6_3,
		kFormat_6_4,
		kFormat_6_5,

};
//**************************************************************************************
enum
{
		kUnits_none	=	0,
		kUnits_degrees,
		kUnits_feet
};

//*****************************************************************************
void	PrintHTMLtableCellDouble(int mySocketFD, double argValue, int numFormat, int units)
{
char	lineBuffer[256];
char	numString[32];

	switch(numFormat)
	{
		case kFormat_6_0:			sprintf(numString,	"%6.0f",	argValue);	break;
		case kFormat_6_1:			sprintf(numString,	"%6.1f",	argValue);	break;
		case kFormat_6_2:			sprintf(numString,	"%6.2f",	argValue);	break;
		case kFormat_6_3:			sprintf(numString,	"%6.3f",	argValue);	break;
		case kFormat_6_4:			sprintf(numString,	"%6.4f",	argValue);	break;
		case kFormat_6_5:			sprintf(numString,	"%6.4f",	argValue);	break;
	}

	switch(units)
	{
		case kUnits_none:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%s</TD>\r\n",		numString);
			break;
		case kUnits_degrees:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%s&deg;</TD>\r\n", numString);
			break;
		case kUnits_feet:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%s<sub>(ft)</sub></TD>\r\n", numString);
			break;

	}
	SocketWriteData(mySocketFD,	lineBuffer);
}

//*****************************************************************************
void	PrintHTMLtableCell_INT(int mySocketFD, int argValue, int numFormat, int units)
{
char	lineBuffer[256];

	switch(units)
	{
		case kUnits_none:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%d</TD>\r\n", argValue);
			break;
		case kUnits_degrees:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%d&deg;</TD>\r\n", argValue);
			break;
		case kUnits_feet:
			sprintf(lineBuffer, "\t\t<TD><CENTER>%d<sub>(ft)</sub></TD>\r\n", argValue);
			break;

	}
	SocketWriteData(mySocketFD,	lineBuffer);
}


//*****************************************************************************
static void	PrintLatLonStatsTable(int mySocketFD)
{
double	delataMiles;
double	delataFeet;

//	CONSOLE_DEBUG(__FUNCTION__);

//	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"<BR>\n");
	SocketWriteData(mySocketFD,	"<CENTER>\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\n");
	SocketWriteData(mySocketFD,	"		<TR>\n");
		PrintHTMLtableCell(mySocketFD, "");
		PrintHTMLtableCell(mySocketFD, "<CENTER>Avg");
		PrintHTMLtableCell(mySocketFD, "<CENTER>Std Dev");
		PrintHTMLtableCell(mySocketFD, "<CENTER>&sigma; Miles");
		PrintHTMLtableCell(mySocketFD, "<CENTER>&sigma; Feet");
	SocketWriteData(mySocketFD,	"		</TR>\n");


	delataMiles	=	(gNMEAdata.latitude_std / 360.0) * 24901.0;
	delataFeet	=	delataMiles * 5280;

	SocketWriteData(mySocketFD,	"		<TR>\n");
		PrintHTMLtableCell(mySocketFD, "Latitude");
		PrintHTMLtableCellDouble(mySocketFD,	gNMEAdata.latitude_avg,	kFormat_6_5, kUnits_degrees);
		PrintHTMLtableCellDouble(mySocketFD,	gNMEAdata.latitude_std,	kFormat_6_5, kUnits_degrees);
		PrintHTMLtableCellDouble(mySocketFD,	delataMiles,			kFormat_6_4, kUnits_none);
		PrintHTMLtableCellDouble(mySocketFD,	delataFeet,				kFormat_6_1, kUnits_feet);
	SocketWriteData(mySocketFD,	"		</TR>\n");

	delataMiles	=	(gNMEAdata.longitude_std / 360.0) * 24901.0;
	delataFeet	=	delataMiles * 5280;

	SocketWriteData(mySocketFD,	"		<TR>\n");
		PrintHTMLtableCell(mySocketFD, "Longitude");
		PrintHTMLtableCellDouble(mySocketFD,	gNMEAdata.longitude_avg,	kFormat_6_5, kUnits_degrees);
		PrintHTMLtableCellDouble(mySocketFD,	gNMEAdata. longitude_std,	kFormat_6_5, kUnits_degrees);
		PrintHTMLtableCellDouble(mySocketFD,	delataMiles,				kFormat_6_4, kUnits_none);
		PrintHTMLtableCellDouble(mySocketFD,	delataFeet,					kFormat_6_1, kUnits_feet);
	SocketWriteData(mySocketFD,	"		</TR>\n");


	SocketWriteData(mySocketFD,	"</TABLE>\n");
	SocketWriteData(mySocketFD,	"</CENTER>\n");
	SocketWriteData(mySocketFD,	"<P>\n");
}
#endif // _ENABLE_GLOBAL_GPS_


#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_
//*****************************************************************************
static void	PrintNMEA_SentanceTable(int mySocketFD)
{
int		totalSentanceCnt;
int		iii;

	SocketWriteData(mySocketFD, "<CENTER><TABLE BORDER=1>\n");
	SocketWriteData(mySocketFD, "		<TR>\n");
		PrintHTMLtableCell(mySocketFD, "<CENTER>NMEA ID");
		PrintHTMLtableCell(mySocketFD, "<CENTER>Count");
		PrintHTMLtableCell(mySocketFD, "<CENTER>Last data");
	SocketWriteData(mySocketFD, "		</TR>\n");

	totalSentanceCnt	=	0;
	for (iii=0; iii<kMaxNMEAsentances; iii++)
	{
		if (gNMEAsentances[iii].count > 0)
		{
			totalSentanceCnt	+=	gNMEAsentances[iii].count;
			SocketWriteData(mySocketFD, "		<TR>\n");
				PrintHTMLtableCell(mySocketFD,		gNMEAsentances[iii].nmeaID);
				PrintHTMLtableCell_INT(mySocketFD,	gNMEAsentances[iii].count,	kFormat_6_0, kUnits_none);
				PrintHTMLtableCell(mySocketFD,		gNMEAsentances[iii].lastData);
			SocketWriteData(mySocketFD, "		</TR>\n");
		}
	}

	SocketWriteData(mySocketFD, "		<TR>\n");
		PrintHTMLtableCell(mySocketFD,		"<CENTER>total");
		PrintHTMLtableCell_INT(mySocketFD,	totalSentanceCnt,	kFormat_6_0, kUnits_none);
	SocketWriteData(mySocketFD, "		</TR>\n");

//		SocketWriteData(mySocketFD, "		<TR>\n");
//			PrintHTMLtableCell(mySocketFD,		"<CENTER>total");
//			PrintHTMLtableCell_INT(mySocketFD,	gNMEAdataRecorded,	kFormat_6_0, kUnits_none);
//		SocketWriteData(mySocketFD, "		</TR>\n");

	SocketWriteData(mySocketFD, "</TABLE></CENTER>\n");
}
#endif


uint32_t	gLastGrapicsSave_ms	=	0;


//*****************************************************************************
void	SendHtml_GPS(TYPE_GetPutRequestData *reqData)
{
char		lineBuffer[512];
char		urlString[512];
int			mySocketFD;
char		gpsWebTitle[]	=	"GPS Information";

#ifdef _ENABLE_GPS_GRAPHS_
uint32_t	current_ms;
uint32_t	delta_ms;

		//*	check to see if we need to create the images
		//*	dont bother re-creating the images if it was done in the last minute
		current_ms	=	millis();
		delta_ms	=	current_ms - gLastGrapicsSave_ms;
		if ((gLastGrapicsSave_ms == 0) || (delta_ms > (1 * 60 * 1000)))
		{
			CreateGPSgraphics();
			gLastGrapicsSave_ms	=	millis();
		}
#endif // _ENABLE_GPS_GRAPHS_


//	CONSOLE_DEBUG(__FUNCTION__);
	mySocketFD	=	reqData->socket;
//		CONSOLE_DEBUG_W_NUM("mySocketFD\t=", mySocketFD);
	SocketWriteData(mySocketFD,	gHtmlHeader_html);
//		SocketWriteData(mySocketFD,	gHtmlNightMode);
	sprintf(lineBuffer, "<TITLE>%s</TITLE>\r\n", gpsWebTitle);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H1>Alpaca device driver Web server</H1>\r\n");
	sprintf(lineBuffer, "<H3>%s</H3>\r\n", gpsWebTitle);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

	//====================================================
#ifdef _ENABLE_GLOBAL_GPS_
	if (gEnableGlobalGPS)
	{
	char		latString[256];
	char		lonString[256];
	int			returnCode;
	struct stat	fileStatus;

		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

		PrintHTMLtableEntry(mySocketFD,	"Device",		gGlobalGPSpath);

		switch(gGlobalGPSbaudrate)
		{
			case '1':	strcpy(lineBuffer,	"19200");	break;
			case '4':	strcpy(lineBuffer,	"4800");	break;
			case '9':	strcpy(lineBuffer,	"9600");	break;
			default:	strcpy(lineBuffer,	"unknown");	break;
		}
		PrintHTMLtableEntry(mySocketFD,	"Baud rate",	lineBuffer);

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		if (gNMEAdata.SequenceNumber > 0)
		{
			SocketWriteData(mySocketFD,	"<CENTER>\r\n");
			SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

			//----------------------------------------------------------------------------
			GetGPSmodeString(gNMEAdata.currSatMode1, gNMEAdata.currSatMode2, lineBuffer);
			PrintHTMLtableEntry(mySocketFD,	"Mode",		lineBuffer);

			//----------------------------------------------------------------------------
			#define	ISLOCKED(myBoolvalue)	(char *)(myBoolvalue ? "Locked" : "invalid")

			PrintHTMLtableEntry(mySocketFD,	"GPS Status",		ISLOCKED(gNMEAdata.validData));
			PrintHTMLtableEntry(mySocketFD,	"Date Status",		ISLOCKED(gNMEAdata.validDate));
			PrintHTMLtableEntry(mySocketFD,	"Time Status",		ISLOCKED(gNMEAdata.validTime));
			PrintHTMLtableEntry(mySocketFD,	"Lat/Lon Status",	ISLOCKED(gNMEAdata.validLatLon));
			PrintHTMLtableEntry(mySocketFD,	"Altitude Status",	ISLOCKED(gNMEAdata.validAlt));


			//--------------------------------------------------------------
			//*	Sequence number
			PrintHTMLtableEntryINT(mySocketFD,	"Sequence number",			gNMEAdata.SequenceNumber);
			PrintHTMLtableEntryINT(mySocketFD,	"Satellites in view",		gNMEAdata.numSats);
			PrintHTMLtableEntryDBL(mySocketFD,	"Latitude",					gNMEAdata.lat_double);
			PrintHTMLtableEntryDBL(mySocketFD,	"Longitude",				gNMEAdata.lon_double);

		#ifdef _ENABLE_GPS_AVERAGE_
			ParseNMEA_FormatLatLonStrings(	gNMEAdata.lat_average,
											latString,
											gNMEAdata.lon_average,
											lonString);
		#else
			ParseNMEA_FormatLatLonStrings(	gNMEAdata.lat_double,
											latString,
											gNMEAdata.lon_double,
											lonString);
		#endif

			PrintHTMLtableEntry(mySocketFD,	"Latitude",			latString);
			PrintHTMLtableEntry(mySocketFD,	"Longitude",		lonString);

			FormatTimeStringISO8601_tm(&gNMEAdata.linuxTime, lineBuffer);
			PrintHTMLtableEntry(mySocketFD,	"Date/Time",		lineBuffer);

			//*	create a google maps link
		#ifdef _ENABLE_GPS_AVERAGE_
			FormatGoogleMapsRequest(urlString, gNMEAdata.lat_average, gNMEAdata.lon_average);
		#else
			FormatGoogleMapsRequest(urlString, gNMEAdata.lat_double, gNMEAdata.lon_double);
		#endif
			sprintf(lineBuffer,	"<A HREF=%s target=google>Google Maps</A>", urlString);
			PrintHTMLtableEntry(mySocketFD,	"Google Maps",		lineBuffer);
			SocketWriteData(mySocketFD,	"</TABLE>\r\n");
			SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		}
		else
		{
			SocketWriteData(mySocketFD,	"<CENTER>\r\n");
			SocketWriteData(mySocketFD,	"<H1>No data has been received from the GPS</H1>\r\n");
			SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		}
		//--------------------------------------------------------
		//*	output links to images


		returnCode	=	stat(kGPSimageDirectory, &fileStatus);	//*	fstat - check for existence of file
		if (returnCode == 0)
		{
			SocketWriteData(mySocketFD,	"<CENTER>\r\n");
			SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
			//--------------------------------------------------------
		#ifdef _ENABLE_SATELLITE_TRAILS_
			PrintHTMLgraphTableEntry(	mySocketFD,
										"Satellite Tracking",
										kGPSimageDirectory,
										"satelliteTrails.jpg");
			PrintHTMLgraphTableEntry(	mySocketFD,
										"Satellite Elevation",
										kGPSimageDirectory,
										elevationGraphFileName);
		#endif // _ENABLE_SATELLITE_TRAILS_

		#ifdef _ENABLE_LAT_LON_TRACKING_
			PrintHTMLgraphTableEntry(	mySocketFD,
										"Lat Lon Tracking",
										kGPSimageDirectory,
										"latlonGraph.jpg");
			SocketWriteData(mySocketFD,	"<TR><TD><CENTER>\r\n");
			PrintLatLonStatsTable(mySocketFD);
			SocketWriteData(mySocketFD,	"</TD><TR></CENTER>\r\n");
		#endif // _ENABLE_LAT_LON_TRACKING_

		#ifdef _ENABLE_ALTITUDE_TRACKING_
			PrintHTMLgraphTableEntry(	mySocketFD,
										"Altitude Tracking",
										kGPSimageDirectory,
										altGraphFileName);
		#endif // _ENABLE_ALTITUDE_TRACKING_

		#ifdef _ENABLE_PDOP_TRACKING_
			PrintHTMLgraphTableEntry(	mySocketFD,
										"PDOP Tracking",
										kGPSimageDirectory,
										pdopGraphFileName);
		#endif // _ENABLE_PDOP_TRACKING_

		#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
			if (gNMEAdata.gPGRME_exists)
			{
				PrintHTMLgraphTableEntry(	mySocketFD,
											"Position Error Tracking",
											kGPSimageDirectory,
											posErrGraphFileName);
			}
		#endif // _ENABLE_NMEA_POSITION_ERROR_TRACKING_

		#ifdef _ENABLE_SATELLITE_ALMANAC_
			PrintHTMLgraphTableEntry(	mySocketFD,
										"Satellite SNR distribution",
										kGPSimageDirectory,
										snrGraphFileName);

			PrintHTMLgraphTableEntry(	mySocketFD,
										"Satellites in Use",
										kGPSimageDirectory,
										satsInUseGraphFileName);
		#endif // _ENABLE_SATELLITE_ALMANAC_

			SocketWriteData(mySocketFD,	"</TABLE>\r\n");
			SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_
			PrintNMEA_SentanceTable(mySocketFD);
		#endif // _ENABLE_NMEA_SENTANCE_TRACKING_
		}
	}
	else
	{
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H1>GPS input is not enabled</H1>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
#else
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H1>Global GPS support is not enabled on this server</H1>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
#endif
	SocketWriteData(mySocketFD,	"</BODY></HTML>\r\n");
}

