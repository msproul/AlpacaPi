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
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"


#ifndef _ENABLE_NET_TEST_
	#define	_ENABLE_ALPACA_QUERY_
#endif


#include	"discovery_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_camera.h"
#include	"controller_cam_normal.h"
#include	"controller_dome.h"
#include	"controller_focus.h"
#include	"controller_focus_generic.h"
#include	"controller_ml_nc.h"
#include	"controller_ml_single.h"
#include	"controller_switch.h"
#include	"controller_image.h"

#ifdef _ENABLE_NET_TEST_
	#include	"controller_nettest.h"
#endif // _ENABLE_NET_TEST_




#ifdef _ENABLE_USB_FOCUSERS_
	#include	"controller_usb.h"
#endif // _ENABLE_USB_FOCUSERS_


bool	gKeepRunning;
char	gColorOverRide	=	0;
char	gFullVersionString[128];
char	gFirstArgString[256];

#ifdef _ENABLE_CTRL_FOCUSERS_
//*****************************************************************************
//*	returns focuser type as per enum
//*	figure out a window name
//*****************************************************************************
static int	GenerateFocuserWindowName(TYPE_REMOTE_DEV *device, int focuserNum, char *windowName)
{
int	myFocuserTYpe;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(device->deviceName);

	if (strncasecmp(device->deviceName, "NiteCrawler", 11) == 0)
	{
		myFocuserTYpe	=	kFocuserType_NiteCrawler;
		if (strlen(device->hostName) > 0)
		{
			//*	if we have a host name, use it
			if (device->alpacaDeviceNum > 0)
			{
				//*	if there are more than one device on this host, we need to make the window name unique
				sprintf(windowName, "NiteCrawler-%s-%d",
									device->hostName,
									device->alpacaDeviceNum);
			}
			else
			{
				sprintf(windowName, "NiteCrawler-%s",
									device->hostName);
			}
		}
		else
		{
			sprintf(windowName, "NiteCrawler -%d", focuserNum);
		}

	}
	else if (strncasecmp(device->deviceName, "Moonlite", 8) == 0)
	{
		myFocuserTYpe	=	kFocuserType_MoonliteSingle;
		sprintf(windowName, "Moonlite -%d", focuserNum);
		if (strlen(device->hostName) > 0)
		{
			//*	if we have a host name, use it
			if (device->alpacaDeviceNum > 0)
			{
				//*	if there are more than one device on this host, we need to make the window name unique
				sprintf(windowName, "Moonlite-%s-%d",
									device->hostName,
									device->alpacaDeviceNum);
			}
			else
			{
				sprintf(windowName, "Moonlite-%s",
									device->hostName);
			}
		}
		else
		{
			sprintf(windowName, "Moonlite -%d", focuserNum);
		}
	}
	else
	{
		myFocuserTYpe	=	kFocuserType_Other;
		sprintf(windowName, "Focuser -%d", focuserNum);

	}
	CONSOLE_DEBUG(windowName);
	return(myFocuserTYpe);
}
#endif // _ENABLE_CTRL_FOCUSERS_


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
				cvWaitKey(100);
			}

			objectsCreated++;
		}
	}
	return(objectsCreated);
}
#endif // _ENABLE_USB_FOCUSERS_

#ifdef _ENABLE_CTRL_FOCUSERS_
int		gFocuserNum		=	1;

//*****************************************************************************
static int	CheckForFocuser(TYPE_REMOTE_DEV *remoteDevice)
{
Controller		*myController;
char			windowName[128]	=	"Moonlite NiteCrawler";
int				objectsCreated;
int				myFocuserType;

	objectsCreated	=	0;
	//*	is it a focuser?
	if (strcasecmp(remoteDevice->deviceType, "focuser") == 0)
	{
		//*	figure out a window name
		myFocuserType	=	GenerateFocuserWindowName(remoteDevice, gFocuserNum, windowName);

		//*	create the controller window object
		if (myFocuserType == kFocuserType_NiteCrawler)
		{
			myController	=	new ControllerNiteCrawler(	windowName,
															&remoteDevice->deviceAddress,
															remoteDevice->port,
															remoteDevice->alpacaDeviceNum);

		}
		else if (myFocuserType == kFocuserType_MoonliteSingle)
		{
			myController	=	new ControllerMLsingle(		windowName,
															&remoteDevice->deviceAddress,
															remoteDevice->port,
															remoteDevice->alpacaDeviceNum);
		}
		else
		{
			myController	=	new ControllerFocusGeneric(	windowName,
															&remoteDevice->deviceAddress,
															remoteDevice->port,
															remoteDevice->alpacaDeviceNum);

		}
		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cvWaitKey(100);
		}

		gFocuserNum++;
		objectsCreated++;
	}
	return(objectsCreated);
}
#endif // _ENABLE_CTRL_FOCUSERS_

#ifdef _ENABLE_CTRL_SWITCHES_
int		gSwitchNum		=	1;
//*****************************************************************************
static int	CheckForSwitch(TYPE_REMOTE_DEV *remoteDevice)
{
int			objectsCreated;
char		windowName[128]	=	"Switch";
Controller	*myController;

	objectsCreated	=	0;
	if (strcasecmp(remoteDevice->deviceType, "switch") == 0)
	{
		GenerateSwitchWindowName(remoteDevice, gSwitchNum, windowName);
		myController	=	new ControllerSwitch(windowName,
												&remoteDevice->deviceAddress,
												remoteDevice->port);
		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cvWaitKey(100);
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
	if (strcasecmp(remoteDevice->deviceType, "camera") == 0)
	{
		GenerateCameraWindowName(remoteDevice, gCameraNum, windowName);
		CONSOLE_DEBUG_W_STR("windowName=", windowName);

		myController	=	new ControllerCamNormal(windowName, remoteDevice);

		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cvWaitKey(100);
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
	if (strcasecmp(remoteDevice->deviceType, "dome") == 0)
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
			cvWaitKey(100);

			//*	now lets look thru the lsit and see if there is a shutter
			for (iii=0; iii<gAlpacaDeviceCnt; iii++)
			{
				if (strcasecmp(gAlpacaIPaddrList[iii].deviceType, "shutter") == 0)
				{
					myDomeController->SetAlpacaShutterInfo(&gAlpacaIPaddrList[iii]);
				}
				else if (strcasecmp(gAlpacaIPaddrList[iii].deviceType, "SlitTracker") == 0)
				{
					myDomeController->SetAlpacaSlitTrackerInfo(&gAlpacaIPaddrList[iii]);
				}
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
int main(int argc, char *argv[])
{
int					iii;
int					objectsCreated;
int					activeObjCnt;
int					keyPressed;

	objectsCreated	=	0;
	gColorOverRide	=	0;

	CONSOLE_DEBUG(__FUNCTION__);

	ProcessCmdLineArgs(argc, argv);

	sprintf(gFullVersionString, "%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);

#ifdef _ENABLE_ALPACA_QUERY_

#ifdef _ENABLE_USB_FOCUSERS_
	objectsCreated	+=	OpenUSBFocusers();
#endif // _ENABLE_USB_FOCUSERS_


	SetupBroadcast();
	SendAlpacaQueryBroadcast();

	CONSOLE_DEBUG_W_NUM("gAlpacaDeviceCnt\t=", gAlpacaDeviceCnt);

	//*	step through the alpaca devices and see if there are any focusers
	for (iii=0; iii<gAlpacaDeviceCnt; iii++)
	{
		#ifdef _ENABLE_CTRL_FOCUSERS_
			objectsCreated	+=	CheckForFocuser(&gAlpacaIPaddrList[iii]);
		#endif // _ENABLE_CTRL_FOCUSERS_

		#ifdef _ENABLE_CTRL_SWITCHES_
			objectsCreated	+=	CheckForSwitch(&gAlpacaIPaddrList[iii]);
		#endif // _ENABLE_CTRL_SWITCHES_
		#ifdef _ENABLE_CTRL_CAMERA_
			objectsCreated	+=	CheckForCamera(&gAlpacaIPaddrList[iii]);
		#endif // _ENABLE_CTRL_CAMERA_

		#ifdef _ENABLE_CTRL_DOME_
			objectsCreated	+=	CheckForDome(&gAlpacaIPaddrList[iii]);
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
	new ControllerImage("SkyTravel", NULL);
	objectsCreated++;
#endif // _ENABLE_SKYTRAVEL_

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
					keyPressed	=	cvWaitKey(25);
					if (keyPressed > 0)
					{
						Controller_HandleKeyDown(keyPressed);
					}

					if (gControllerList[iii]->cKeepRunning == false)
					{
						CONSOLE_DEBUG_W_NUM("Deleting control #", iii);
						CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
						delete gControllerList[iii];
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

