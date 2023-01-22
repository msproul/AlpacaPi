//*****************************************************************************
//*		controller_main.cpp		(c) 2020-21 by Mark Sproul
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
//*	Feb 27,	2020	<MLS> Created controller_main.cpp
//*	Mar  9,	2020	<MLS> Added ability to quit
//*	Mar 24,	2020	<MLS> Added command line options, ProcessCmdLineArgs()
//*	Apr 18,	2020	<MLS> Added CheckForDome()
//*	Dec  4,	2020	<MLS> Added _ENABLE_ALPACA_QUERY_ (for nettest controller)
//*	Jan 16,	2021	<MLS> Added CheckForOpenWindowByName()
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#ifndef _ENABLE_NET_TEST_
	#define	_ENABLE_ALPACA_QUERY_
#endif


#include	"discovery_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"cpu_stats.h"

#include	"controller.h"
#include	"controller_camera.h"
#include	"controller_cam_normal.h"
#include	"controller_dome.h"
#include	"controller_focus.h"
#include	"controller_focus_generic.h"
#include	"controller_focus_ml_nc.h"
#include	"controller_focus_ml_hr.h"
#include	"controller_switch.h"
#include	"controller_image.h"

#ifdef _ENABLE_NET_TEST_
	#include	"controller_nettest.h"
#endif // _ENABLE_NET_TEST_




#ifdef _ENABLE_USB_FOCUSERS_
	#include	"controller_usb.h"
#endif // _ENABLE_USB_FOCUSERS_


bool	gKeepRunning;
char	gFullVersionString[128];
char	gFirstArgString[256];
bool	gVerbose					=	false;
char	gUserAgentAlpacaPiStr[80]	=	"";



#ifdef _ENABLE_CTRL_SWITCHES_
//*****************************************************************************
static void	GenerateSwitchWindowName(TYPE_REMOTE_DEV *device, int switchNum, char *windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	figure out a window name
	if (strlen(device->hostName) > 0)
	{
		//*	if we have a host name, use it
		if (device->alpacaDeviceNum > 0)
		{
			//*	if there are more than one device on this host, we need to make the window name unique
			sprintf(windowName, "Switch-%s-%d",
								device->hostName,
								device->alpacaDeviceNum);
		}
		else
		{
			sprintf(windowName, "Switch-%s",
								device->hostName);
		}
	}
	else
	{
		sprintf(windowName, "Switch-%d", switchNum);
	}
}
#endif // _ENABLE_CTRL_SWITCHES_

#ifdef _ENABLE_CTRL_CAMERA_
//*****************************************************************************
static void	GenerateCameraWindowName(TYPE_REMOTE_DEV *device, int switchNum, char *windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	figure out a window name
	if (strlen(device->hostName) > 0)
	{
		//*	if we have a host name, use it
		if (device->alpacaDeviceNum > 0)
		{
			//*	if there are more than one device on this host, we need to make the window name unique
			sprintf(windowName, "Camera-%s-%d",
								device->hostName,
								device->alpacaDeviceNum);
		}
		else
		{
			sprintf(windowName, "Camera-%s",
								device->hostName);
		}
	}
	else
	{
		sprintf(windowName, "Camera-%d", switchNum);
	}
}
#endif // _ENABLE_CTRL_CAMERA_



#ifdef _ENABLE_USB_FOCUSERS_
//*****************************************************************************
//*	return the number of objects created
//*****************************************************************************
static 	int OpenUSBFocusers(void)
{
int					moonliteUSBcnt;
TYPE_MOONLITECOM	moonLiteDevice;
int					objectsCreated;
int					iii;
ControllerUSB		*myController;

	//*	check for usb connected focusers
	objectsCreated	=	0;
	moonliteUSBcnt	=	MoonLite_CountFocusers();
	if (moonliteUSBcnt > 0)
	{
		for (iii=0; iii<moonliteUSBcnt; iii++)
		{
			//*	init the structure and get the device path
			MoonLite_InitCom(&moonLiteDevice, iii);
			CONSOLE_DEBUG_W_STR("Usb device=", moonLiteDevice.usbPortPath);

			myController	=	new ControllerUSB(	moonLiteDevice.usbPortPath,
													moonLiteDevice.usbPortPath);
			if (myController != NULL)
			{
				//*	force window update
				myController->HandleWindowUpdate();
				cv::waitKey(100);
			}
			objectsCreated++;
		}
	}
	return(objectsCreated);
}
#endif // _ENABLE_USB_FOCUSERS_


#ifdef _ENABLE_CTRL_SWITCHES_
int		gSwitchNum		=	1;
//*****************************************************************************
static int	CheckForSwitch(TYPE_REMOTE_DEV *remoteDevice)
{
int			objectsCreated;
char		windowName[128]	=	"Switch";
Controller	*myController;

	objectsCreated	=	0;
	if (strcasecmp(remoteDevice->deviceTypeStr, "switch") == 0)
	{
		GenerateSwitchWindowName(remoteDevice, gSwitchNum, windowName);
		myController	=	new ControllerSwitch(windowName,
												&remoteDevice->deviceAddress,
												remoteDevice->port);
		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cv::waitKey(100);
		}

		gSwitchNum++;
		objectsCreated++;
	}
	return(objectsCreated);
}
#endif // _ENABLE_CTRL_SWITCHES_


#ifdef _ENABLE_CTRL_CAMERA_
int		gCameraNum	=	1;
//*****************************************************************************
static int	CheckForCamera(TYPE_REMOTE_DEV *remoteDevice)
{
Controller	*myController;
char		windowName[128]	=	"Camera";
int			objectsCreated;

	objectsCreated	=	0;
	if (strcasecmp(remoteDevice->deviceTypeStr, "camera") == 0)
	{
		GenerateCameraWindowName(remoteDevice, gCameraNum, windowName);
		CONSOLE_DEBUG_W_STR("windowName=", windowName);

		myController	=	new ControllerCamNormal(windowName, remoteDevice);

		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cv::waitKey(100);
		}
		gCameraNum++;
		objectsCreated++;
	}
	return(objectsCreated);
}
#endif // _ENABLE_CTRL_CAMERA_




#ifdef _ENABLE_CTRL_DOME_
//*****************************************************************************
static int	CheckForDome(TYPE_REMOTE_DEV *remoteDevice)
{
ControllerDome	*myDomeController;
char			windowName[64];
char			ipAddressStr[32];
int				objectsCreated;
int				iii;

	CONSOLE_DEBUG(__FUNCTION__);
	objectsCreated	=	0;
	if (strcasecmp(remoteDevice->deviceTypeStr, "dome") == 0)
	{
		inet_ntop(AF_INET, &remoteDevice->deviceAddress.sin_addr, ipAddressStr, INET_ADDRSTRLEN);

		strcpy(windowName, "Dome-");
		strcat(windowName, ipAddressStr);
		CONSOLE_DEBUG_W_STR("windowName=", windowName);

		myDomeController	=	new ControllerDome(windowName, remoteDevice);

		if (myDomeController != NULL)
		{
			//*	force window update
			myDomeController->HandleWindowUpdate();
			cv::waitKey(100);

			//*	now lets look thru the lsit and see if there is a shutter
			for (iii=0; iii<gAlpacaDiscoveredCnt; iii++)
			{
				if (strcasecmp(gAlpacaDiscoveredList[iii].deviceTypeStr, "shutter") == 0)
				{
					myDomeController->SetAlpacaShutterInfo(&gAlpacaDiscoveredList[iii]);
				}
			#ifdef _ENABLE_SLIT_TRACKER_
				else if (strcasecmp(gAlpacaDiscoveredList[iii].deviceTypeStr, "SlitTracker") == 0)
				{
					myDomeController->SetAlpacaSlitTrackerInfo(&gAlpacaDiscoveredList[iii]);
				}
			#endif
			}
		}
		objectsCreated++;
	}
	if (objectsCreated > 0)
	{
		CONSOLE_DEBUG_W_NUM("Number of dome objects created\t=", objectsCreated);
	}
	return(objectsCreated);
}
#endif // _ENABLE_CTRL_CAMERA_

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


//*****************************************************************************
int main(int argc, char *argv[])
{
int		iii;
int		objectsCreated;
int		activeObjCnt;

	objectsCreated	=	0;
	gColorOverRide	=	0;

	CONSOLE_DEBUG(__FUNCTION__);

	ProcessCmdLineArgs(argc, argv);

	sprintf(gFullVersionString, "%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);

	CPUstats_ReadOSreleaseVersion();
	CPUstats_ReadInfo();

#ifdef _ENABLE_ALPACA_QUERY_

#ifdef _ENABLE_USB_FOCUSERS_
	objectsCreated	+=	OpenUSBFocusers();
#endif // _ENABLE_USB_FOCUSERS_


	SetupBroadcast();
	SendAlpacaQueryBroadcast();

	CONSOLE_DEBUG_W_NUM("gAlpacaDiscoveredCnt\t=", gAlpacaDiscoveredCnt);

	//*	step through the alpaca devices and see if there are any focusers
	for (iii=0; iii<gAlpacaDiscoveredCnt; iii++)
	{
		#ifdef _ENABLE_CTRL_FOCUSERS_
			objectsCreated	+=	CheckForFocuser(&gAlpacaDiscoveredList[iii]);
		#endif // _ENABLE_CTRL_FOCUSERS_

		#ifdef _ENABLE_CTRL_SWITCHES_
			objectsCreated	+=	CheckForSwitch(&gAlpacaDiscoveredList[iii]);
		#endif // _ENABLE_CTRL_SWITCHES_
		#ifdef _ENABLE_CTRL_CAMERA_
			objectsCreated	+=	CheckForCamera(&gAlpacaDiscoveredList[iii]);
		#endif // _ENABLE_CTRL_CAMERA_

		#ifdef _ENABLE_CTRL_DOME_
			objectsCreated	+=	CheckForDome(&gAlpacaDiscoveredList[iii]);
		#endif // _ENABLE_CTRL_DOME_

	}

#ifdef _ENABLE_USB_FOCUSERS_
//	//*	did not find any, create one for testing
//	if (objectsCreated == 0)
//	{
//		new ControllerFocus(	"Focuser",
//								NULL,
//								0,
//								0,
//								kFocuserType_NiteCrawler);
//		objectsCreated++;
//	}
#endif


#endif // _ENABLE_ALPACA_QUERY_


#ifdef _ENABLE_NET_TEST_
//	CONSOLE_DEBUG("_ENABLE_NET_TEST_");

	new ControllerNetTest(	"Network Test",
							NULL,
							0,
							0);
	objectsCreated++;
#endif // _ENABLE_NET_TEST_

#ifdef _ENABLE_SKYTRAVEL_
	#error "this probably can be deleted"
	new ControllerImage("SkyTravel", NULL);
	objectsCreated++;
#endif // _ENABLE_SKYTRAVEL_

	if (objectsCreated > 0)
	{
		gKeepRunning	=	true;
		activeObjCnt	=	objectsCreated;

		while (gKeepRunning && (activeObjCnt > 0))
		{
			activeObjCnt	=	ProcessControllerWindows();
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
	else
	{
		CONSOLE_DEBUG("No devices found");
	}
}

#if 0
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
#endif // 0
