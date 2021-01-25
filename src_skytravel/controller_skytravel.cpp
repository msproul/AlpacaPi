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


#define	kWindowWidth	1200
#define	kWindowHeight	820

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
	//*	clear out all of the dome properties data
	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));
	cDomeProp.ShutterStatus			=	-1;


	//*	clear all of the telescope specific properties
	memset(&cTelescopeProp, 0, sizeof(TYPE_TelescopeProperties));


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
	cDomeTabObjPtr	=	new WindowTabDome(cWidth, cHeight, cBackGrndColor, cWindowName, true);
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

				SetWidgetBGColor(kTab_SkyTravel,	kSkyTravel_DomeIndicator,		CV_RGB(64,	255,	64));
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

				cReadStartup_Telescope	=	true;
				cTelescopeHas_readall	=	false;

				SetWidgetBGColor(kTab_SkyTravel,	kSkyTravel_TelescopeIndicator,	CV_RGB(64,	255,	64));
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

	if (foundSomething || needToUpdate)
	{
		cAlpacaListObjPtr->UpdateRemoteDeviceList();
	}

	if (needToUpdate)
	{
		//*	here is where we query the devices for information

		//----------------------------------------------------
		//*	is the DOME IP address valid
		if (cDomeAddressValid)
		{
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
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cDomeHas_readall);
		if (cDomeHas_readall == false)
		{
			cDeviceAddress	=	cDomeIpAddress;
			cPort			=	cDomeIpPort;
			cAlpacaDevNum	=	cDomeAlpacaDeviceNum;
			validData	=	AlpacaGetStringValue(	"dome", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
				SetWidgetText(kTab_Dome,		kDomeBox_AlpacaDrvrVersion,		cAlpacaVersionString);
			}
		}

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
//SJP_Parser_t	jsonParser;
bool			validData;
char			returnString[128];
char			ipAddrStr[128];

//	CONSOLE_DEBUG(__FUNCTION__);

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
//		cUpdateDelta	=	kDefaultUpdateDelta;
		if (cDomeProp.Slewing)
		{
			//*	if we slewing, we want to update more often
//			cUpdateDelta	=	2;
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Slewing");
		}
		else if (cDomeProp.AtHome)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Home");
		}
		else if (cDomeProp.AtPark)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Park");
		}
		else
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "---");
		}


		//*	if we shutter is opening or closing, we want to update more often
		if ((cDomeProp.ShutterStatus == kShutterStatus_Opening) ||
				(cDomeProp.ShutterStatus == kShutterStatus_Closing))
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

//*****************************************************************************
bool	ControllerSkytravel::AlpacaGetTelescopeStatus(void)
{
bool	validData;
bool	previousOnLineState;
char	raString[64];
char	decString[64];
char	textBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	validData			=	false;
	previousOnLineState	=	cOnLine;
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


		SetWidgetText(kTab_SkyTravel, kSkyTravel_LX200_RA_DEC, textBuff);
	}
	else
	{
		CONSOLE_DEBUG("Failed to get telescope status!!!!!!!!!!");
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
//		CONSOLE_DEBUG_W_2STR(__FUNCTION__, deviceType, keywordString);
		AlpacaProcessReadAll_Telescope(deviceNum, keywordString, valueString);
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

//	SetWidgetText(kSkyTravel_LX200_RA_DEC, ra_dec_string);

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

