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
//*	Jan 26,	2021	<MLS> Added SetDomeIPaddress() & SetTelescopeIPaddress()
//*	Jan 26,	2021	<MLS> SkyTravel can choose which dome/telescope to sync with
//*	Feb  4,	2021	<MLS> Added MOON window tab for phase of the moon info
//*	Mar 13,	2021	<MLS> Added AlpacaGetStartupData_Camera()
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


#define	kWindowWidth	1200
#define	kWindowHeight	820

#include	"alpaca_defs.h"
#include	"windowtab_about.h"
#include	"windowtab_alpacalist.h"
#include	"windowtab_deviceselect.h"
#include	"windowtab_dome.h"
#include	"windowtab_fov.h"
#include	"windowtab_iplist.h"
#include	"windowtab_moon.h"
#include	"windowtab_skytravel.h"


#include	"controller.h"
#include	"controller_skytravel.h"


extern char	gFullVersionString[];


double	gTelescopeRA_Hours		=	0.0;
double	gTelescopeRA_Radians	=	0.0;
double	gTelescopeDecl_Degrees	=	0.0;
double	gTelescopeDecl_Radians	=	0.0;


//**************************************************************************************
ControllerSkytravel::ControllerSkytravel(	const char *argWindowName)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{

	CONSOLE_DEBUG(__FUNCTION__);

	cSkyTravelTabOjbPtr		=	NULL;
	cSkySettingsTabObjPtr	=	NULL;
	cDomeTabObjPtr			=	NULL;
	cDeviceSelectObjPtr		=	NULL;
	cAlpacaListObjPtr		=	NULL;
	cIPaddrListObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cFOVTabObjPtr			=	NULL;

	cDomeAddressValid		=	false;
	cTelescopeAddressValid	=	false;
	cUpdateDelta			=	kDefaultUpdateDelta;

	//*	dome specific stuff
	//*	clear out all of the dome properties data
	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));
	cDomeProp.ShutterStatus	=	-1;


	//*	clear all of the telescope specific properties
	memset(&cTelescopeProp, 0, sizeof(TYPE_TelescopeProperties));

	SetupWindowControls();

}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerSkytravel::~ControllerSkytravel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	DELETE_OBJ_IF_VALID(cSkyTravelTabOjbPtr);
	DELETE_OBJ_IF_VALID(cSkySettingsTabObjPtr);
	DELETE_OBJ_IF_VALID(cDomeTabObjPtr);
	DELETE_OBJ_IF_VALID(cAlpacaListObjPtr);
	DELETE_OBJ_IF_VALID(cIPaddrListObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceSelectObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
	DELETE_OBJ_IF_VALID(cFOVTabObjPtr);
}


//**************************************************************************************
void	ControllerSkytravel::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_ST_Count);

	//=============================================================
	SetTabText(kTab_SkyTravel,	"SkyTravel");
	cSkyTravelTabOjbPtr		=	new WindowTabSkyTravel(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSkyTravelTabOjbPtr != NULL)
	{
		SetTabWindow(kTab_SkyTravel,	cSkyTravelTabOjbPtr);
		cSkyTravelTabOjbPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_ST_Settings,	"Settings");
	cSkySettingsTabObjPtr		=	new WindowTabSTsettings(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSkySettingsTabObjPtr != NULL)
	{
		SetTabWindow(kTab_ST_Settings,	cSkySettingsTabObjPtr);
		cSkySettingsTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_ST_FOV,	"F.O.V.");
	cFOVTabObjPtr		=	new WindowTabFOV(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cFOVTabObjPtr != NULL)
	{
	TYPE_CameraFOV	*myFOVptr;

		SetTabWindow(kTab_ST_FOV,	cFOVTabObjPtr);
		cFOVTabObjPtr->SetParentObjectPtr(this);


		//*	we want the SKYTRAVEL window to know about the CAMERA FOV array

		myFOVptr	=	cFOVTabObjPtr->GetCameraFOVptr();
		if (cSkyTravelTabOjbPtr != NULL)
		{
			cSkyTravelTabOjbPtr->SetCameraFOVptr(myFOVptr);
		}
	}



	//=============================================================
	SetTabText(kTab_Moon,	"Moon");
	cMoonTabObjPtr		=	new WindowTabMoon(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cMoonTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Moon,	cMoonTabObjPtr);
		cMoonTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_ST_Dome,		"Dome");
	cDomeTabObjPtr	=	new WindowTabDome(cWidth, cHeight, cBackGrndColor, cWindowName, true);
	if (cDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_ST_Dome,	cDomeTabObjPtr);
		cDomeTabObjPtr->SetParentObjectPtr(this);
		cDomeTabObjPtr->SetDomePropertiesPtr(&cDomeProp);
	}

	//=============================================================
	SetTabText(kTab_AlpacaList,		"Alpaca List");
	cAlpacaListObjPtr	=	new WindowTabAlpacaList(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAlpacaListObjPtr != NULL)
	{
		SetTabWindow(kTab_AlpacaList,	cAlpacaListObjPtr);
		cAlpacaListObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_IPList,		"IP List");
	cIPaddrListObjPtr	=	new WindowTabIPList(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cIPaddrListObjPtr != NULL)
	{
		SetTabWindow(kTab_IPList,	cIPaddrListObjPtr);
		cIPaddrListObjPtr->SetParentObjectPtr(this);
	}


	//=============================================================
	SetTabText(kTab_DeviceList,		"Device Selection");
	cDeviceSelectObjPtr	=	new WindowTabDeviceSelect(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDeviceSelectObjPtr != NULL)
	{
		SetTabWindow(kTab_DeviceList,	cDeviceSelectObjPtr);
		cDeviceSelectObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_ST_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_ST_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//**************************************************************************************
//*	returns true if found something new
bool	ControllerSkytravel::LookForIPaddress(void)
{
int		iii;
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
				SetDomeIPaddress(&gRemoteList[iii]);
				foundSomething			=	true;
			}
		}

		if (cTelescopeAddressValid == false)
		{
			if (strcasecmp("Telescope", gRemoteList[iii].deviceTypeStr) == 0)
			{
				CONSOLE_DEBUG_W_STR("We found a Telescope", gRemoteList[iii].deviceNameStr);
				SetTelescopeIPaddress(&gRemoteList[iii]);
				foundSomething			=	true;
			}
		}
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(foundSomething);
}

//**************************************************************************************
void	ControllerSkytravel::SetDomeIPaddress(TYPE_REMOTE_DEV *remoteDomeDevice)
{
char	ipAddrStr[32];
char	lineBuff[64];


	//============================================
	//*	disable all of the extra commands until we know if they exist
	if (cDomeTabObjPtr != NULL)
	{
		cDomeTabObjPtr->ResetKnownCommands();
	}

	cDomeIpAddress			=	remoteDomeDevice->deviceAddress;
	cDomeIpPort				=	remoteDomeDevice->port;
	cDomeAlpacaDeviceNum	=	remoteDomeDevice->alpacaDeviceNum;
	cDomeAddressValid		=	true;

	cReadStartup_Dome		=	true;
	cDomeHas_readall		=	false;

	PrintIPaddressToString(cDomeIpAddress.sin_addr.s_addr, ipAddrStr);
	sprintf(lineBuff, "%s:%d/%d", ipAddrStr, cDomeIpPort, cDomeAlpacaDeviceNum);

	SetWindowIPaddrInfo(lineBuff, true);

	SetWidgetBGColor(kTab_SkyTravel,	kSkyTravel_DomeIndicator,		CV_RGB(64,	255,	64));

	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);
}

//**************************************************************************************
void	ControllerSkytravel::SetTelescopeIPaddress(TYPE_REMOTE_DEV *remoteDomeDevice)
{
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	cTelescopeIpAddress			=	remoteDomeDevice->deviceAddress;
	cTelescopeIpPort			=	remoteDomeDevice->port;
	cTelescopeAlpacaDeviceNum	=	remoteDomeDevice->alpacaDeviceNum;
	cTelescopeAddressValid		=	true;

	cReadStartup_Telescope		=	true;
	cTelescopeHas_readall		=	false;

	SetWidgetBGColor(kTab_SkyTravel,	kSkyTravel_TelescopeIndicator,	CV_RGB(64,	255,	64));

	SetWidgetText(kTab_SkyTravel,	kSkyTravel_Telescope_RA_DEC,	"-----------");

	inet_ntop(AF_INET, &(cTelescopeIpAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

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
		AlpacaGetStartupData_Dome();
		cReadStartup_Dome	=	false;
	}

	if (cTelescopeAddressValid && cReadStartup_Telescope)
	{
		AlpacaGetStartupData_Telescope();

		cReadStartup_Telescope	=	false;
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

	if (cFirstDataRead || (deltaSeconds >= cUpdateDelta))
	{
		needToUpdate	=	true;
	}

	if (foundSomething || needToUpdate)
	{
		cAlpacaListObjPtr->UpdateRemoteDeviceList();
		cDeviceSelectObjPtr->UpdateRemoteDeviceList();
		cFOVTabObjPtr->UpdateRemoteDeviceList();
		cIPaddrListObjPtr->UpdateIPaddrList();
	}

	if (cDomeProp.Slewing && (deltaSeconds >= 1))
	{
		needToUpdate	=	true;

	}

	if ((cDomeProp.ShutterStatus == kShutterStatus_Opening) ||
				(cDomeProp.ShutterStatus == kShutterStatus_Closing))
	{
		if (deltaSeconds >= 1)
		{
			needToUpdate	=	true;
		}
	}



	if (needToUpdate)
	{
//		CONSOLE_DEBUG_W_NUM("Need to update", cDebugCounter++);
		//*	here is where we query the devices for information

		//----------------------------------------------------
		//*	is the DOME IP address valid
		if (cDomeAddressValid)
		{
//			CONSOLE_DEBUG("Updating dome status");
			validData	=	AlpacaGetDomeStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("AlpacaGetDomeStatus failed");
			}
		}
		//----------------------------------------------------
		//*	is the TELESCOPE IP address valid
		if (cTelescopeAddressValid)
		{
			validData	=	AlpacaGetTelescopeStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("AlpacaGetTelescopeStatus failed");
			}
		}

		cLastUpdate_milliSecs	=	millis();
	}


	if (cSkyTravelTabOjbPtr != NULL)
	{
		cSkyTravelTabOjbPtr->RunBackgroundTasks();
	}

	if (cMoonTabObjPtr != NULL)
	{
		cMoonTabObjPtr->RunBackgroundTasks();
	}

	if (cFOVTabObjPtr != NULL)
	{
		cFOVTabObjPtr->RunBackgroundTasks();
	}

}

//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetStartupData_Camera(	TYPE_REMOTE_DEV			*remoteDevice,
															TYPE_CameraProperties	*cameraProp)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	AlpacaGetDoubleValue(	remoteDevice->deviceAddress,
											remoteDevice->port,
											remoteDevice->alpacaDeviceNum,
											"camera",
											"pixelsizex",
											NULL,
											&cameraProp->PixelSizeX);

	validData	=	AlpacaGetDoubleValue(	remoteDevice->deviceAddress,
											remoteDevice->port,
											remoteDevice->alpacaDeviceNum,
											"camera",
											"pixelsizey",
											NULL,
											&cameraProp->PixelSizeY);

	validData	=	AlpacaGetIntegerValue(	remoteDevice->deviceAddress,
											remoteDevice->port,
											remoteDevice->alpacaDeviceNum,
											"camera",
											"cameraxsize",
											NULL,
											&cameraProp->CameraXsize);


	validData	=	AlpacaGetIntegerValue(	remoteDevice->deviceAddress,
											remoteDevice->port,
											remoteDevice->alpacaDeviceNum,
											"camera",
											"cameraysize",
											NULL,
											&cameraProp->CameraYsize);

//	validData	=	AlpacaGetIntegerValue("camera", "cameraxsize",	NULL,	&cCameraProp.CameraXsize);
//	validData	=	AlpacaGetIntegerValue("camera", "cameraysize",	NULL,	&cCameraProp.CameraYsize);


	return(validData);
}

//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetStartupData_Dome(void)
{
//SJP_Parser_t	jsonParser;
bool			validData;
char			returnString[128];
char			ipAddrStr[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	inet_ntop(AF_INET, &(cDomeIpAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG(ipAddrStr);

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions(	&cDomeIpAddress,
												cDomeIpPort,
												"dome",
												cDomeAlpacaDeviceNum);
	if (validData)
	{
		cDeviceAddress	=	cDomeIpAddress;
		cPort			=	cDomeIpPort;
		cAlpacaDevNum	=	cDomeAlpacaDeviceNum;

		SetWidgetValid(kTab_ST_Dome,			kDomeBox_Readall,		cDomeHas_readall);
		if (cDomeHas_readall == false)
		{
			validData	=	AlpacaGetStringValue(	"dome", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
				SetWidgetText(kTab_ST_Dome,		kDomeBox_AlpacaDrvrVersion,		cAlpacaVersionString);
			}
		}
		validData	=	AlpacaGetBooleanValue(	"dome", "cansetazimuth",	NULL,	&cDomeProp.CanSetAzimuth);
		validData	=	AlpacaGetBooleanValue(	"dome", "cansetpark",		NULL,	&cDomeProp.CanSetPark);
		validData	=	AlpacaGetBooleanValue(	"dome", "canfindhome",		NULL,	&cDomeProp.CanFindHome);
		validData	=	AlpacaGetBooleanValue(	"dome", "canslave",			NULL,	&cDomeProp.CanSlave);

//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanSetAzimuth\t=",	cDomeProp.CanSetAzimuth);
//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanSetPark   \t=",	cDomeProp.CanSetPark);
//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanFindHome  \t=",	cDomeProp.CanFindHome);
//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanSlave     \t=",	cDomeProp.CanSlave);

		cDomeTabObjPtr->UpdateControls();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}


	cLastUpdate_milliSecs	=	millis();

	return(validData);
}

//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetStartupData_Telescope(void)
{
bool			validData;
char			ipAddrStr[128];

	CONSOLE_DEBUG(__FUNCTION__);

	inet_ntop(AF_INET, &(cTelescopeIpAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG(ipAddrStr);

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions(	&cTelescopeIpAddress,
												cTelescopeIpPort,
												"telescope",
												cTelescopeAlpacaDeviceNum);
	if (validData)
	{

	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
	validData	=	AlpacaGetStartupData_TelescopeOneAAT();

	cLastUpdate_milliSecs	=	millis();

	return(validData);
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
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, "ReadAll");
		validData	=	AlpacaGetStatus_ReadAll(&cDomeIpAddress, cDomeIpPort, "dome", cDomeAlpacaDeviceNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_DomeOneAAT();
	}

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
		cUpdateDelta	=	kDefaultUpdateDelta;
		if (cDomeProp.Slewing)
		{
			//*	if we slewing, we want to update more often
			cUpdateDelta	=	1;
			SetWidgetText(kTab_ST_Dome, kDomeBox_CurPosition, "Slewing");
			cUpdateWindow	=	true;
		}
		else if (cDomeProp.AtHome)
		{
			SetWidgetText(kTab_ST_Dome, kDomeBox_CurPosition, "Home");
			cUpdateWindow	=	true;
		}
		else if (cDomeProp.AtPark)
		{
			SetWidgetText(kTab_ST_Dome, kDomeBox_CurPosition, "Park");
			cUpdateWindow	=	true;
		}
		else
		{
			SetWidgetText(kTab_ST_Dome, kDomeBox_CurPosition, "---");
			cUpdateWindow	=	true;
		}


		//*	if we shutter is opening or closing, we want to update more often
		if ((cDomeProp.ShutterStatus == kShutterStatus_Opening) ||
				(cDomeProp.ShutterStatus == kShutterStatus_Closing))
		{
			cUpdateDelta	=	1;
		}
		SetWidgetBGColor(	kTab_SkyTravel,	kSkyTravel_DomeIndicator,	CV_RGB(64,	255,	64));
	}
	else
	{
		cOnLine	=	false;
		SetWidgetBGColor(	kTab_SkyTravel,	kSkyTravel_DomeIndicator,	CV_RGB(255,	0,	0));
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetTelescopeStatus(void)
{
bool	validData;
//bool	previousOnLineState;
char	raString[64];
char	decString[64];
char	textBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	validData			=	false;
//	previousOnLineState	=	cOnLine;
	if (cTelescopeHas_readall)
	{
//		CONSOLE_DEBUG("cTelescopeHas_readall");
		validData	=	AlpacaGetStatus_ReadAll(&cTelescopeIpAddress,
												cTelescopeIpPort,
												"telescope",
												cTelescopeAlpacaDeviceNum);
	}
	else
	{
//		CONSOLE_DEBUG("AlpacaGetStatus_TelescopeOneAAT");
		validData	=	AlpacaGetStatus_TelescopeOneAAT();
	}

	//*	do we have new data
	if (validData)
	{

		//*	update the RA/DEC in the SkyTravel window
		FormatHHMMSS(cTelescopeProp.RightAscension,	raString, false);
		FormatHHMMSS(cTelescopeProp.Declination,	decString, true);
		sprintf(textBuff, "%s / %s", raString, decString);


		SetWidgetText(		kTab_SkyTravel, kSkyTravel_Telescope_RA_DEC, textBuff);
		SetWidgetBGColor(	kTab_SkyTravel,	kSkyTravel_TelescopeIndicator,	CV_RGB(64,	255,	64));
	}
	else
	{
		CONSOLE_DEBUG("Failed to get telescope status!!!!!!!!!!");
		SetWidgetText(kTab_SkyTravel, kSkyTravel_Telescope_RA_DEC, "Telescope failed");
		SetWidgetBGColor(	kTab_SkyTravel,	kSkyTravel_TelescopeIndicator,	CV_RGB(255,	0,	0));
	}


	return(validData);
}


//*****************************************************************************
void	ControllerSkytravel::AlpacaProcessReadAll(	const char	*deviceType,
													const int	deviceNum,
													const char	*keywordString,
													const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);
	if (strcasecmp(deviceType, "Dome") == 0)
	{
		AlpacaProcessReadAll_Dome(deviceNum, keywordString, valueString);
	}
	else if (strcasecmp(deviceType, "Telescope") == 0)
	{
		AlpacaProcessReadAll_Telescope(deviceNum, keywordString, valueString);
	}
	else if (strcasecmp(deviceType, "Camera") == 0)
	{
	//	AlpacaProcessReadAll_Camera(deviceNum, keywordString, valueString);
		if (cFOVTabObjPtr != NULL)
		{
			cFOVTabObjPtr->AlpacaProcessReadAll_Camera(deviceNum, keywordString, valueString);
		}
	}
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	ControllerSkytravel::AlpacaProcessSupportedActions(	const char	*deviceType,
															const int	deviveNum,
															const char	*valueString)
{
//	CONSOLE_DEBUG_W_2STR(__FUNCTION__, deviceType, valueString);

	if (strcasecmp(deviceType, "Dome") == 0)
	{
		AlpacaProcessSupportedActions_Dome(deviveNum, valueString);
	}
	else if (strcasecmp(deviceType, "Telescope") == 0)
	{
		AlpacaProcessSupportedActions_Telescope(deviveNum, valueString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}


//**************************************************************************************
void	ControllerSkytravel::RefreshWindow(void)
{
	HandleWindowUpdate();
	cvWaitKey(100);
}

//**************************************************************************************
void	ControllerSkytravel::Update_TelescopeRtAscension(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("cTelescopeProp.RightAscension", cTelescopeProp.RightAscension);
	gTelescopeRA_Radians	=	RADIANS(cTelescopeProp.RightAscension * 15.0);

//	SetWidgetText(kSkyTravel_Telescope_RA_DEC, ra_dec_string);

}

//**************************************************************************************
void	ControllerSkytravel::Update_TelescopeDeclination(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("cTelescopeProp.Declination", cTelescopeProp.Declination);
	gTelescopeDecl_Radians	=	RADIANS(cTelescopeProp.Declination);
}



#define	PARENT_CLASS	ControllerSkytravel
#include	"controller_tscope_common.cpp"

#endif // _ENABLE_SKYTRAVEL_

