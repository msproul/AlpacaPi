//*****************************************************************************
//*		skytravel_main.cpp		(c) 2021 by Mark Sproul
//*
//*		This is the main routine for creating controller windows
//*
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
//*	Jan  9,	2021	<MLS> Created skytravel_main.cpp
//*	Feb 12,	2021	<MLS> Added CloseAllExceptFirst()
//*	Sep 28,	2021	<MLS> Added -v = gVerbose command line option
//*	Jan  8,	2022	<MLS> Added support for remote Gaia SQL access
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	<fitsio.h>

#include	"discovery_lib.h"
#include	"cpu_stats.h"
#include	"helper_functions.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"
#include	"AsteroidData.h"

#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"observatory_settings.h"

#include	"controller.h"
#include	"controller_skytravel.h"
#include	"OpenNGC.h"

#ifdef _ENABLE_REMOTE_GAIA_
	#include	"GaiaSQL.h"
#endif

bool	gKeepRunning;
char	gFullVersionString[128];
char	gFirstArgString[256];
bool	gVerbose		=	false;

//-----------------------------------------------------------
//*	star information
TYPE_CelestData		*gStarDataPtr		=	NULL;
long				gStarCount			=	0;

TYPE_CelestData		*gYaleStarDataPtr	=	NULL;
long				gYaleStarCount		=	0;

TYPE_Constelation	*gConstelations		=	NULL;
short				gConstelationCount	=	0;

TYPE_CelestData		*gConstStarPtr		=	NULL;
long				gConstStarCount		=	0;

TYPE_CelestData		*gNGCobjectPtr		=	NULL;
long				gNGCobjectCount		=	0;

TYPE_OpenNGCoutline	*gOpenNGC_outlines	=	NULL;
long				gOpenNGC_outlineCnt	=	0;

TYPE_CelestData		*gSAOobjectPtr		=	NULL;
long				gSAOobjectCount		=	0;

//*	Hipparcos
TYPE_CelestData		*gHipObjectPtr		=	NULL;
long				gHipObjectCount		=	0;

//*	Messier
TYPE_CelestData		*gMessierObjectPtr	=	NULL;
long				gMessierObjectCount	=	0;

//*	HYG
TYPE_CelestData		*gHYGObjectPtr		=	NULL;
long				gHYGObjectCount		=	0;

//*	Henry Draper
TYPE_CelestData		*gDraperObjectPtr	=	NULL;
long				gDraperObjectCount	=	0;

//*	special objects for local use.
TYPE_CelestData		*gSpecialObjectPtr	=	NULL;
long				gSpecialObjectCount	=	0;

//*	Polar Alignment center points
TYPE_CelestData		*gPolarAlignObjectPtr	=	NULL;
long				gPolarAlignObjectCount	=	0;

//*	AAVSO alert list
TYPE_CelestData		*gAAVSOalertsPtr	=	NULL;
long				gAAVSOalertsCnt		=	0;

//*	Gaia star catalog
TYPE_CelestData		*gGaiaObjectPtr		=	NULL;
long				gGaiaObjectCnt		=	0;


//*	Lowell asteroid database
TYPE_Asteroid		*gAsteroidPtr			=	NULL;
long				gAsteroidCnt			=	0;
char				gAsteroidDatbase[32]	=	"";


char				gNGCDatbase[32]			=	"";


//*****************************************************************************
static void	ProcessCmdLineArgs(int argc, char **argv)
{
int		ii;
char	theChar;

	strcpy(gFirstArgString, "");

//	CONSOLE_DEBUG(__FUNCTION__);
	ii	=	1;
	while (ii<argc)
	{
		if (argv[ii][0] == '-')
		{
			theChar	=	argv[ii][1];
			switch(theChar)
			{
				//*	-c	force color
				//*	-cr		means red
				case 'c':
					if (isalpha(argv[ii][2]))
					{
						gColorOverRide	=	argv[ii][2];
					}
					break;

				//*	-i ip address
				case 'i':
					break;

				//	"-q" means quiet
				case 'q':
					gVerbose	=	false;
					break;

				//	"-v" means verbose
				case 'v':
					gVerbose	=	true;
					break;
			}
		}
		else
		{
			strcpy(gFirstArgString, argv[ii]);
		}
		ii++;
	}
}


TYPE_CONTROLER_TIMING_INFO	gControllerTime[kMaxControllers];

//*****************************************************************************
int main(int argc, char *argv[])
{
int					iii;
int					objectsCreated;
int					activeObjCnt;
int					keyPressed;
float				fitsVersionRet;
float				fitsVersionVal;
unsigned int		currentMillis;
unsigned int		lastDebugMillis;
unsigned int		deltaSecs;
unsigned long		startNanoSecs;
unsigned long		endNanoSecs;
unsigned long		deltaNanoSecs;
//unsigned long		lastTimingMillis;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("sizeof(TYPE_CelestData)", sizeof(TYPE_CelestData));

	lastDebugMillis		=	millis();
//	lastTimingMillis	=	millis();


	fitsVersionRet		=	ffvers(&fitsVersionVal);
#ifdef CFITSIO_MAJOR
	#if (CFITSIO_MAJOR >= 4)
		printf("cfitsio version %d.%d.%d\r\n", CFITSIO_MAJOR, CFITSIO_MINOR, CFITSIO_MICRO);
	#else
		printf("cfitsio version %3.2f\r\n", CFITSIO_VERSION);
	#endif
#else
	printf("cfitsio version %3.2f\r\n", fitsVersionVal);
#endif
	if (fitsVersionRet != fitsVersionVal)
	{
		printf("cfitsio version %3.2f\r\n", fitsVersionRet);
	}

	memset(&gST_DispOptions,	0, sizeof(SkyTravelDispOptions));

	objectsCreated	=	0;
	gColorOverRide	=	0;


	//*	deal with any options from the command line
	ProcessCmdLineArgs(argc, argv);

	sprintf(gFullVersionString, "%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);


	CPUstats_ReadOSreleaseVersion();
	CPUstats_ReadInfo();

	//*	read in lat/lon values for this location
	ObservatorySettings_Init();
	ObservatorySettings_ReadFile();

#ifdef _ENABLE_REMOTE_GAIA_
	//*	returns true if valid config file
	gST_DispOptions.RemoteGAIAenabled		=	GaiaSQLinit();
	gST_DispOptions.GaiaRequestMode			=	kGaiaRequestMode_3x1;
#else
	gST_DispOptions.RemoteGAIAenabled			=	false;
#endif // _ENABLE_REMOTE_GAIA_
//	CONSOLE_DEBUG_W_STR("RemoteGAIAenabled is", (gST_DispOptions.RemoteGAIAenabled ? "enabled" : "disabled"));

#ifdef _USE_OPENCV_CPP_
	new ControllerSkytravel("SkyTravel++");
#else
	new ControllerSkytravel("SkyTravel");
#endif
	objectsCreated++;

	//*	set the time to all zeros
	for (iii=0; iii<kMaxControllers; iii++)
	{
		gControllerTime[iii].Count				=	0;
		gControllerTime[iii].RecentNanoSecons	=	0;
		gControllerTime[iii].TotalNanoSecons	=	0;
		gControllerTime[iii].AverageNanoSecons	=	0;
	}

	StartDiscoveryQuerryThread();


	gKeepRunning	=	true;
	activeObjCnt	=	objectsCreated;

	while (gKeepRunning && (activeObjCnt > 0))
	{
		activeObjCnt	=	0;
		for (iii=0; iii<kMaxControllers; iii++)
		{
			if (gControllerList[iii] != NULL)
			{
				activeObjCnt++;
				startNanoSecs	=	MSecTimer_getNanoSecs();
				gControllerList[iii]->HandleWindow();
				endNanoSecs		=	MSecTimer_getNanoSecs();
				deltaNanoSecs	=	endNanoSecs - startNanoSecs;

				gControllerTime[iii].Count++;
				gControllerTime[iii].RecentNanoSecons	=	deltaNanoSecs;
				gControllerTime[iii].TotalNanoSecons	+=	deltaNanoSecs;
				gControllerTime[iii].AverageNanoSecons	=	gControllerTime[iii].TotalNanoSecons / gControllerTime[iii].Count;

				keyPressed	=	cv::waitKeyEx(5);

				if (keyPressed > 0)
				{
					Controller_HandleKeyDown(keyPressed);
				}

				if (gControllerList[iii]->cKeepRunning == false)
				{
				//	CONSOLE_DEBUG_W_NUM("Deleting control #", iii);
				//	CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
					delete gControllerList[iii];
					if (gControllerList[iii] != NULL)
					{
						CONSOLE_DEBUG_W_STR("Delete had a problem", gControllerList[iii]->cWindowName);
					}
				}
			}
		}
		if (gVerbose)
		{
			currentMillis	=	millis();
			deltaSecs		=	(currentMillis - lastDebugMillis) / 1000;
			if (deltaSecs > 15)
			{
				DumpControllerBackGroundTaskStatus();
				lastDebugMillis	=	millis();
			}
		}
	}
	CONSOLE_DEBUG("Closing all windows");
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
			delete gControllerList[iii];
			cv::waitKey(10);
		//	sleep(2);
		}
	}
	CONSOLE_DEBUG("Clean exit");
}

//*****************************************************************************
void	CloseAllExceptFirst(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=1; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Closing ", gControllerList[iii]->cWindowName);
			gControllerList[iii]->cKeepRunning	=	false;
		}
	}
}

