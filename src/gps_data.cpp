//**************************************************************************
//*	Name:			gps_data.cpp
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr  9,	2024	<MLS> Created gps_data.cpp
//*	Apr 26,	2024	<MLS> Started working on gps graph support
//*	Apr 27,	2024	<MLS> GPS graph working from alpacapi driver
//*****************************************************************************

//#define _ENABLE_GLOBAL_GPS_

#ifdef _ENABLE_GLOBAL_GPS_

#include	<errno.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pthread.h>
#include	<unistd.h>
#include	<sys/stat.h>
//#include <sys/types.h>


#include	<fcntl.h>
#include	<termios.h>
#include	<sys/ioctl.h>	//*	ioctl() call definitions

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"

#include	"ParseNMEA.h"
#include	"NMEA_helper.h"

#include	"serialport.h"
#include	"gps_data.h"

#ifdef _ENABLE_GPS_GRAPHS_
	#include	"GPS_graph.h"
	static void	CreateGPSgrapicsDirectory(void);
#endif

//===========================================================================
//*	GPS info
TYPE_NMEAInfoStruct		gNMEAdata;		//*	from ParseNMEA.h

static pthread_t		gGPSdataThreadID;
static char				gSerialPortPath[64];
static char				gSerialPortSpeed	=	'9';	//*	9 for 9600, 4 for 4800
#define	kBuffSize	100

//**************************************************************************************
static void	*GPS_Thread(void *arg)
{
char		buf[kBuffSize + 1];
int			readCnt;
int			ccc;
int			iii;
int			serialFD;
int			nmeaSentenceCnt;
char		theChar;
char		nmeaLineBuff[256];
struct stat	fileStatus;
int			returnCode;
//uint32_t	lastGrapicsSave_ms;
//uint32_t	current_ms;
//uint32_t	delta_ms;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(gSerialPortPath);
	CONSOLE_DEBUG(__FUNCTION__);

	ParseNMEA_init(&gNMEAdata);
//	lastGrapicsSave_ms	=	0;
	//---------------------------------------------------
	//*	check to make sure the devices is present
	returnCode	=	stat(gSerialPortPath, &fileStatus);		//*	fstat - check for existence of file
	if (returnCode == 0)
	{
		CONSOLE_DEBUG_W_STR("Device is present", gSerialPortPath);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Device NOT FOUND!!!!!", gSerialPortPath);
		CONSOLE_DEBUG(gSerialPortPath);
		CONSOLE_DEBUG("Thread exit!!!!!!!");
		return(NULL);
	}

//	serialFD	=	open(gSerialPortPath, O_RDWR | O_NOCTTY | O_SYNC);
	serialFD	=	open(gSerialPortPath, O_RDONLY | O_NOCTTY | O_SYNC);
	if (serialFD < 0)
	{
		CONSOLE_DEBUG_W_STR("ERROR on open()", gSerialPortPath);
		CONSOLE_DEBUG_W_NUM("ERROR number\t=", errno);
		CONSOLE_DEBUG_W_STR("ERROR string\t=", strerror(errno));
		CONSOLE_DEBUG_W_NUM("serialFD    \t=", serialFD);
//		fprintf(stderr, "error %d opening %s: %s", errno, (char *)arg, strerror(errno));
		CONSOLE_DEBUG("Thread exit!!!!!!!");
		return(NULL);
	}

	if (gSerialPortSpeed == '4')
	{
		CONSOLE_DEBUG("Setting baud rate for GPS to B4800");
		Serial_Set_Attribs(serialFD, B4800, 0);  // set speed to 4800 bps, 8n1 (no parity)
	}
	else
	{
		//*	default is 9600 because Raspberry Pi GPS board is 9600
		CONSOLE_DEBUG("Setting baud rate for GPS to B9600");
		Serial_Set_Attribs(serialFD, B9600, 0);  // set speed to 4800 bps, 8n1 (no parity)
	}
	Serial_Set_Blocking(serialFD, true);
	nmeaSentenceCnt	=	0;
	ccc				=	0;
	while (1)				// receive 25:  approx 100 uS per char transmit
	{
		readCnt			=	read(serialFD, buf, kBuffSize);  // read up to 100 characters if ready to read
		if (readCnt > 0)
		{
			buf[readCnt]	=	0;
			buf[kBuffSize]	=	0;

			for (iii=0; iii<readCnt; iii++)
			{
				theChar	=	buf[iii];
				if (theChar < 0x20)
				{
					nmeaLineBuff[ccc]	=	0;
					if (ccc > 5)
					{
					#ifdef _INCLUDE_GPSTEST_MAIN_
						printf("%s\r\n", nmeaLineBuff);
					#endif
						//	true means set system time
						if (nmeaLineBuff[0] == '$')
						{
							ParseNMEA_TimeString(&gNMEAdata, nmeaLineBuff, false);
							ParseNMEAstring(&gNMEAdata, nmeaLineBuff);
						}
					}
					ccc	=	0;
					//-------------------------------------------
					nmeaSentenceCnt++;
//					if ((nmeaSentenceCnt % 100) == 0)
//					{
//						CONSOLE_DEBUG_W_NUM("nmeaSentenceCnt\t=", nmeaSentenceCnt);
//						DumpGPSdata(&gNMEAdata);
//					}
				}
				else
				{
					nmeaLineBuff[ccc++]	=	theChar;
				}
			}
		}
//*	this was moved to alpacadriver_gps.cpp so that the images are only created when needed
//	#ifdef _ENABLE_GPS_GRAPHS_
//		current_ms	=	millis();
//		delta_ms	=	current_ms - lastGrapicsSave_ms;
//		if (delta_ms > (1 * 60 * 1000))
////		if (delta_ms > (10 * 1000))
//		{
//			CreateGPSgraphics();
//			lastGrapicsSave_ms	=	current_ms;
//		}
//
//	#endif // _ENABLE_GPS_GRAPHS_
	}
}

//*****************************************************************************
void	GPS_StartThread(const char *serialPortPathArg, const char baudRate)
{
int		threadErr;

#ifdef _ENABLE_GPS_GRAPHS_
	CreateGPSgrapicsDirectory();
#endif

	if (serialPortPathArg != NULL)
	{
		strcpy(gSerialPortPath, serialPortPathArg);
	}
	else
	{
		strcpy(gSerialPortPath, "/dev/ttyS0");
	}
	//*	save the baud rate indicator
	gSerialPortSpeed	=	baudRate;
	threadErr	=	pthread_create(&gGPSdataThreadID, NULL, &GPS_Thread, NULL);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG_W_NUM("pthread_create() returned error#", threadErr);
	}
}

#ifdef _ENABLE_GPS_GRAPHS_
//*****************************************************************************
static void	CreateGPSgrapicsDirectory(void)
{
int			returnCode;
int			mkdirErrCode;
struct stat	fileStatus;


	returnCode	=	stat(kGPSimageDirectory, &fileStatus);	//*	fstat - check for existence of file
	if (returnCode == 0)
	{
//		CONSOLE_DEBUG_W_STR("Directory is present", kGPSimageDirectory);
	}
	else
	{
		mkdirErrCode	=	mkdir(kGPSimageDirectory, 0744);
		if (mkdirErrCode == 0)
		{
			CONSOLE_DEBUG_W_NUM("mkdir() failed", mkdirErrCode);
		}
	}
}

static int	gGraphsCreatedCounter	=	0;

//*****************************************************************************
void	CreateGPSgraphics(void)
{
	CONSOLE_DEBUG_W_NUM("gGraphsCreatedCounter\t=", gGraphsCreatedCounter);

	SETUP_TIMING();

	gGraphsCreatedCounter++;
#ifdef _ENABLE_SATELLITE_TRAILS_
	CreateSatelliteTrailsGraph(NULL, kGPSimageDirectory, "satelliteTrails.jpg");
	CreateSatelliteElevationGraph(NULL, kGPSimageDirectory, elevationGraphFileName);
#endif

#ifdef _ENABLE_LAT_LON_TRACKING_
	CreateLatLonHistoryPlot(	NULL, kGPSimageDirectory,		"latlonGraph.jpg");
	CreateLatDetailHistoryPlot(	NULL, kGPSimageDirectory,		"latitudeDetail.jpg");
#endif

#ifdef _ENABLE_ALTITUDE_TRACKING_
	CreateAltitudeHistoryPlot(NULL, kGPSimageDirectory, 	altGraphFileName);
#endif

#ifdef _ENABLE_PDOP_TRACKING_
	CreatePDOPhistoryPlot(NULL, kGPSimageDirectory,			pdopGraphFileName);
#endif

#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
	CreatePositionErrorHistoryPlot(NULL, kGPSimageDirectory, posErrGraphFileName);
#endif

#ifdef _ENABLE_ALTITUDE_TRACKING_
	CreateAltitudeHistoryPlot(NULL, kGPSimageDirectory, altGraphFileName);
#endif // _ENABLE_ALTITUDE_TRACKING_

#ifdef _ENABLE_SATELLITE_ALMANAC_
	CreateSNRdistrbutionPlot(	NULL, kGPSimageDirectory, snrGraphFileName);
	CreateSatsInUseHistoryPlot(	NULL, kGPSimageDirectory, satsInUseGraphFileName);
#endif // _ENABLE_SATELLITE_ALMANAC_

	DEBUG_TIMING("Time to write out image files:");
}
#endif // _ENABLE_GPS_GRAPHS_

#ifdef _INCLUDE_GPSTEST_MAIN_

//*****************************************************************************
int	main(int argc, char **argv)
{
	printf("Staring gps read data thread %s\r\n", __FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kLatLonTacking_ArraySize\t=",	kLatLonTacking_ArraySize);
	CONSOLE_DEBUG_W_NUM("kAltTacking_ArraySize   \t=",	kAltTacking_ArraySize);
	CONSOLE_DEBUG_W_NUM("kPosErrTacking_ArraySize\t=",	kPosErrTacking_ArraySize);
	CONSOLE_DEBUG_W_NUM("kPDOPtacking_ArraySize  \t=",	kPDOPtacking_ArraySize);

	GPS_StartThread("/dev/ttyS0");

	while (1)
	{
		sleep(1);
	}
}
#endif // _INCLUDE_GPSTEST_MAIN_

#endif // _ENABLE_GLOBAL_GPS_
