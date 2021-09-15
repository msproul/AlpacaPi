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
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"



#include	"discovery_lib.h"
#include	"cpu_stats.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"

#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"observatory_settings.h"

#include	"controller.h"
#include	"controller_skytravel.h"


bool	gKeepRunning;
char	gFullVersionString[128];
char	gFirstArgString[256];

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

//*	Hipparcos
TYPE_CelestData		*gHipObjectPtr		=	NULL;
long				gHipObjectCount		=	0;

//*	Messier
TYPE_CelestData		*gMessierOjbectPtr	=	NULL;
long				gMessierOjbectCount	=	0;

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


//*****************************************************************************
static void	ProcessCmdLineArgs(int argc, char **argv)
{
int		ii;
char	theChar;

	strcpy(gFirstArgString, "");

	CONSOLE_DEBUG(__FUNCTION__);
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

			}
		}
		else
		{
			strcpy(gFirstArgString, argv[ii]);
		}
		ii++;
	}
}

#include <fitsio.h>

//*****************************************************************************
int main(int argc, char *argv[])
{
int					iii;
int					objectsCreated;
int					activeObjCnt;
int					keyPressed;
float				fitsVersionRet;
float				fitsVersionVal;

	fitsVersionRet	=	ffvers(&fitsVersionVal);
	printf("cfitsio version %3.2f\r\n", CFITSIO_VERSION);
	printf("cfitsio version %3.2f\r\n", fitsVersionRet);
	printf("cfitsio version %3.2f\r\n", fitsVersionVal);

	objectsCreated	=	0;
	gColorOverRide	=	0;

	CONSOLE_DEBUG(__FUNCTION__);

	ProcessCmdLineArgs(argc, argv);

	sprintf(gFullVersionString, "%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);


	CPUstats_ReadOSreleaseVersion();
	CPUstats_ReadInfo();

	//*	read in lat/lon values for this location
	ObservatorySettings_Init();
	ObservatorySettings_ReadFile();


	new ControllerSkytravel("SkyTravel");
	objectsCreated++;

	StartDiscoveryQuerryThread();


	if (objectsCreated > 0)
	{
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
					gControllerList[iii]->HandleWindow();
			//		usleep(100);
					keyPressed	=	cvWaitKey(50);
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
		}
		CONSOLE_DEBUG("Closing all windows");
		for (iii=0; iii<kMaxControllers; iii++)
		{
			if (gControllerList[iii] != NULL)
			{
				CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
				delete gControllerList[iii];
				cvWaitKey(10);
			//	sleep(2);
			}
		}
		CONSOLE_DEBUG("Clean exit");
	}
	else
	{
		CONSOLE_DEBUG("No devices found");
	}
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

//*****************************************************************************
//*	this steps through the Controller Object List to see if there is a window by this name
//*****************************************************************************
bool	CheckForOpenWindowByName(const char *windowName)
{
int		iii;
bool	windowExists;

	windowExists	=	false;
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			if (strcmp(gControllerList[iii]->cWindowName, windowName) == 0)
			{
				windowExists	=	true;
				break;
			}
		}
	}
	return(windowExists);
}

