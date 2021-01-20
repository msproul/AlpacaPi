//*****************************************************************************
//*		controller_skytravel.cpp		(c) 2020 by Mark Sproul
//*
//*
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  9,	2021	<MLS> Created controller_skytravel.cpp
//*	Jan  9,	2021	<MLS> Added LookForIPaddress()
//*	Jan  9,	2021	<MLS> Added a bunch of stuff from controller_dome
//*	Jan  9,	2021	<MLS> SkyTravel is now talking to the dome controller
//*****************************************************************************


#ifdef _ENABLE_SKYTRAVEL_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

//#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"discoverythread.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	1100
#define	kWindowHeight	800

#include	"alpaca_defs.h"
#include	"windowtab_skytravel.h"
#include	"windowtab_dome.h"
#include	"windowtab_about.h"


#include	"controller.h"
#include	"controller_skytravel.h"
#include	"lx200_com.h"


extern char	gFullVersionString[];




//**************************************************************************************
ControllerSkytravel::ControllerSkytravel(	const char *argWindowName)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{

	CONSOLE_DEBUG(__FUNCTION__);

	cSkyTravelTabOjbPtr		=	NULL;
	cDomeTabObjPtr			=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	cDomeAddressValid		=	false;
	cTelescopeAddressValid	=	false;

	//*	dome specific stuff
	cCanFindHome			=	false;
	cCanPark				=	false;
	cCanSetAltitude			=	false;
	cCanSetAzimuth			=	false;
	cCanSetPark				=	false;
	cCanSetShutter			=	false;
	cCanSlave				=	false;
	cCanSyncAzimuth			=	false;
	cCanPark				=	false;
	cAtHome					=	false;
	cAtPark					=	false;
	cSlewing				=	false;
	cAzimuth_Dbl			=	0.0;
	cAltitude_Dbl			=	0.0;
	cShutterStatus			=	-1;
//	cShutterInfoValid		=	false;
//	cSlitTrackerInfoValid	=	false;
//	cLogSlitData			=	false;
//	cSlitDataLogFilePtr		=	NULL;
//	cValidGravity			=	false;



	SetupWindowControls();

	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,		gFullVersionString);

}



//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerSkytravel::~ControllerSkytravel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//=============================================================
	if (cSkyTravelTabOjbPtr != NULL)
	{
		delete cSkyTravelTabOjbPtr;
		cSkyTravelTabOjbPtr	=	NULL;
	}

	//=============================================================
	if (cDomeTabObjPtr != NULL)
	{
		delete cDomeTabObjPtr;
		cDomeTabObjPtr	=	NULL;
	}

	//=============================================================
	if (cAlpacaListObjPtr != NULL)
	{
		delete cAlpacaListObjPtr;
		cAlpacaListObjPtr	=	NULL;
	}

	//=============================================================
	if (cAboutBoxTabObjPtr != NULL)
	{
		delete cAboutBoxTabObjPtr;
		cAboutBoxTabObjPtr	=	NULL;
	}
}


//**************************************************************************************
void	ControllerSkytravel::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);

	SetTabText(kTab_SkyTravel,	"SkyTravel");
	cSkyTravelTabOjbPtr		=	new WindowTabSkyTravel(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSkyTravelTabOjbPtr != NULL)
	{
		SetTabWindow(kTab_SkyTravel,	cSkyTravelTabOjbPtr);
		cSkyTravelTabOjbPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_Dome,		"Dome");
	cDomeTabObjPtr	=	new WindowTabDome(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Dome,	cDomeTabObjPtr);
		cDomeTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_AlpacaList,		"Alpaca List");
	cAlpacaListObjPtr	=	new WindowTabAlpacaList(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_AlpacaList,	cAlpacaListObjPtr);
		cAlpacaListObjPtr->SetParentObjectPtr(this);
	}


	//=============================================================
	SetTabText(kTab_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

//	SetWidgetFont(kTab_Video,	kUSBselect_IPaddr, kFont_Medium);

//	SetWidgetText(kTab_Video,	kUSBselect_IPaddr,	cUSBpath);
//	SetWidgetText(kTab_About,	kAboutBox_IPaddr,	cUSBpath);
}

//**************************************************************************************
//*	returns true if found something new
bool	ControllerSkytravel::LookForIPaddress(void)
{
int		iii;
char	ipString[32];
char	lineBuff[64];
bool	foundSomething;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
	foundSomething	=	false;
	for (iii=0; iii<gRemoteCnt; iii++)
	{
		if (cDomeAddressValid == false)
		{
			if (strcasecmp("Dome", gRemoteList[iii].deviceTypeStr) == 0)
			{
				CONSOLE_DEBUG_W_STR("We found a dome", gRemoteList[iii].deviceNameStr);
				cDomeIpAddress			=	gRemoteList[iii].deviceAddress;
				cDomeIpPort				=	gRemoteList[iii].port;
				cDomeAlpacaDeviceNum	=	gRemoteList[iii].alpacaDeviceNum;
				cDomeAddressValid		=	true;

				cReadStartup_Dome		=	true;
				cDomeHas_readall		=	false;

				PrintIPaddressToString(cDomeIpAddress.sin_addr.s_addr, ipString);
				sprintf(lineBuff, "%s:%d/%d", ipString, cDomeIpPort, cDomeAlpacaDeviceNum);

				SetWindowIPaddrInfo(lineBuff, true);
				foundSomething			=	true;
			}
		}
		if (cTelescopeAddressValid == false)
		{
			if (strcasecmp("Telescope", gRemoteList[iii].deviceTypeStr) == 0)
			{
				CONSOLE_DEBUG_W_STR("We found a Telescope", gRemoteList[iii].deviceNameStr);
				cTelescopeIpAddress		=	gRemoteList[iii].deviceAddress;
				cTelescopeIpPort		=	gRemoteList[iii].port;
				cTelescopeAddressValid	=	true;
				foundSomething			=	true;
			}
		}
	}
	return(foundSomething);
}

//**************************************************************************************
void	ControllerSkytravel::RunBackgroundTasks(void)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
//uint32_t	secsSinceLastListUpdate;
bool		needToUpdate;
bool		validData;
bool		foundSomething;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	if (cDomeAddressValid && cReadStartup_Dome)
	{
		CONSOLE_DEBUG("Reading supported actions");
		AlpacaGetStartupData();
		cReadStartup_Dome	=	false;
	}

	if (cReadStartup)
	{
//		AlpacaGetStartupData();

		cReadStartup	=	false;
	}

	//*	check for valid IP addresses
	foundSomething	=	false;
	if ((cDomeAddressValid == false) || (cTelescopeAddressValid == false))
	{
		foundSomething	=	LookForIPaddress();
	}



	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;


	if (cForceAlpacaUpdate)
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}
	else if (deltaSeconds >= 4)
	{
		needToUpdate	=	true;
	}

	if (foundSomething || needToUpdate)
	{
		cAlpacaListObjPtr->UpdateList();
	}

	if (needToUpdate)
	{
		//*	here is where we query the devices for information

		//*	is the IP address valid
		if (cDomeAddressValid)
		{
			validData	=	AlpacaGetDomeStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("AlpacaGetDomeStatus failed");
			}
		}


		cLastUpdate_milliSecs	=	millis();
	}

	if (cSkyTravelTabOjbPtr != NULL)
	{
		cSkyTravelTabOjbPtr->RunBackgroundTasks();
	}

}


//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetDomeStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG(__FUNCTION__);
	validData			=	false;
	previousOnLineState	=	cOnLine;
	if (cDomeHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll(&cDomeIpAddress, cDomeIpPort, "dome", cDomeAlpacaDeviceNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();
	}
#if 0
	//=================================================
	if (cShutterInfoValid)
	{
		AlpacaGetShutterReadAll();
	}

	//=================================================
	if (cSlitTrackerInfoValid)
	{
		AlpacaGetSlitTrackerReadAll();
	}
#endif // 0

	if (validData)
	{
//		CONSOLE_DEBUG("We have valid data");
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
		cOnLine	=	true;

		//======================================================================
//		cUpdateDelta	=	kDefaultUpdateDelta;
		if (cSlewing)
		{
			//*	if we slewing, we want to update more often
//			cUpdateDelta	=	2;
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Slewing");
		}
		else if (cAtHome)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Home");
		}
		else if (cAtPark)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Park");
		}
		else
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "---");
		}


		//*	if we shutter is opening or closing, we want to update more often
		if ((cShutterStatus == kShutterStatus_Opening) || (cShutterStatus == kShutterStatus_Closing))
		{
//			cUpdateDelta	=	2;
		}
	}
	else
	{
		cOnLine	=	false;
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}



//**************************************************************************************
void	ControllerSkytravel::RefreshWindow(void)
{
	HandleWindowUpdate();
	cvWaitKey(100);
}


#endif // _ENABLE_SKYTRAVEL_

