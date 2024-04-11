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
//*****************************************************************************

#define _ENABLE_GLOBAL_GPS_

#ifdef _ENABLE_GLOBAL_GPS_

#include	<errno.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pthread.h>
#include	<unistd.h>

#include	<fcntl.h>
#include	<termios.h>
#include	<sys/ioctl.h> //ioctl() call definitions

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"ParseNMEA.h"
#include	"NMEA_helper.h"

#include	"serialport.h"
#include	"gps_data.h"


//===========================================================================
//*	GPS info
TYPE_NMEAInfoStruct		gNMEAdata;		//*	from ParseNMEA.h
TYPE_GPSdata			gGPS;
static pthread_t		gGPSdataThreadID;
static char				gSerialPortPath[32];

#define	kBuffSize	100

//**************************************************************************************
static void	*GPS_Thread(void *arg)
{
char				buf[kBuffSize + 1];
int					readCnt;
int					ccc;
int					iii;
int					serialFD;
int					nmeaSentenceCnt;
char				theChar;
char				nmeaLineBuff[256];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(gSerialPortPath);
	CONSOLE_DEBUG(__FUNCTION__);

	ParseNMEA_init(&gNMEAdata);

//	serialFD	=	open(gSerialPortPath, O_RDWR | O_NOCTTY | O_SYNC);
	serialFD	=	open(gSerialPortPath, O_RDONLY | O_NOCTTY | O_SYNC);
	if (serialFD < 0)
	{
		CONSOLE_DEBUG_W_STR("ERROR on open()", gSerialPortPath);
		CONSOLE_DEBUG_W_NUM("ERROR number\t=", errno);
		CONSOLE_DEBUG_W_STR("ERROR string\t=", strerror(errno));
//		fprintf(stderr, "error %d opening %s: %s", errno, (char *)arg, strerror(errno));
		CONSOLE_DEBUG("Thread exit!!!!!!!");
		return NULL;
	}

	Serial_Set_Attribs(serialFD, B9600, 0);  // set speed to 4800 bps, 8n1 (no parity)
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
//					#ifdef _INCLUDE_GPSTEST_MAIN_
						printf("%s\r\n", nmeaLineBuff);
//					#endif

						//	true means set system time
						ParseNMEA_TimeString(&gNMEAdata, nmeaLineBuff, false);
						ParseNMEAstring(&gNMEAdata, nmeaLineBuff);
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
	}
}


//*****************************************************************************
void	GPS_StartThread(const char *serialPortPathArg)
{
int		threadErr;

	if (serialPortPathArg != NULL)
	{
		strcpy(gSerialPortPath, serialPortPathArg);
	}
	else
	{
		strcpy(gSerialPortPath, "/dev/ttyS0");
	}
	threadErr	=	pthread_create(&gGPSdataThreadID, NULL, &GPS_Thread, NULL);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG_W_NUM("pthread_create() returned error#", threadErr);
	}
}

#ifdef _INCLUDE_GPSTEST_MAIN_

//*****************************************************************************
int	main(int argc, char **argv)
{
	printf("Staring gps read data thread %s\r\n", __FUNCTION__);

	GPS_StartThread("/dev/ttyS0");

	while (1)
	{
		sleep(1);
	}
}
#endif // _INCLUDE_GPSTEST_MAIN_

#endif // _ENABLE_GLOBAL_GPS_
