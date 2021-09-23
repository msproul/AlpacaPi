//*****************************************************************************
//*		controller_telescope.cpp		(c) 2021 by Mark Sproul
//*
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
//*	Jan 22,	2021	<MLS> Created controller_telescope.cpp
//*	Jan 23,	2021	<MLS> Added Update_TelescopeRtAscension()
//*	Jan 23,	2021	<MLS> Added Update_TelescopeDeclination()
//*	Jan 24,	2021	<MLS> Converted TelescopeControler to use properties struct
//*****************************************************************************


#ifdef _ENABLE_CTRL_TELESCOPE_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"helper_functions.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	700
#define	kWindowHeight	550

#include	"windowtab_telescope.h"
#include	"windowtab_capabilities.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_telescope.h"


//**************************************************************************************
enum
{
	kTab_Telescope	=	1,
	kTab_Capabilities,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerTelescope::ControllerTelescope(	const char			*argWindowName,
											struct sockaddr_in	*deviceAddress,
											const int			port,
											const int			deviceNum)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, kNoBackgroundTask)
{

	//*	zero out all of the Telescope ASCOM properties
	memset(&cTelescopeProp, 0, sizeof(TYPE_TelescopeProperties));


	cAlpacaDevNum			=	deviceNum;
	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();

	cTelescopeTabObjPtr		=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;

		AlpacaSetConnected("telescope", true);
	}
	cLastUpdate_milliSecs	=	0;
	SetupWindowControls();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerTelescope::~ControllerTelescope(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//--------------------------------------------
	DELETE_OBJ_IF_VALID(cTelescopeTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);

}


//**************************************************************************************
void	ControllerTelescope::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Telescope,		"Telescope");
	SetTabText(kTab_Capabilities,	"Capabilities");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");


	//--------------------------------------------
	cTelescopeTabObjPtr	=	new WindowTabTelescope(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cTelescopeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Telescope,	cTelescopeTabObjPtr);
		cTelescopeTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	SetWidgetFont(kTab_Telescope,	kTelescope_IPaddr, kFont_Medium);

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	lineBuff[128];
	char	ipString[32];

		inet_ntop(AF_INET, &cDeviceAddress.sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

//		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//**************************************************************************************
void	ControllerTelescope::RunBackgroundTasks(bool enableDebug)
{
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;
bool		validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT("telescope");

		SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
		SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
		SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
		SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
		SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

		cReadStartup	=	false;
	}

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdate_milliSecs;
	if (deltaMilliSecs > 2000)
	{
		validData	=	AlpacaGetStatus();
		if (validData == false)
		{
			CONSOLE_DEBUG("AlpacaGetStatus() fialed")
		}
		cLastUpdate_milliSecs	=	millis();

		UpdateConnectedIndicator(kTab_Telescope,		kTelescope_Connected);
	}
}

//*****************************************************************************
//*	this routine gets called one time to get the info on the telescope that does not change
//*****************************************************************************
bool	ControllerTelescope::AlpacaGetStartupData(void)
{
bool	validData;
char	returnString[256];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("telescope", cAlpacaDevNum);
	if (validData)
	{
		CONSOLE_DEBUG_W_STR("Valid supported actions:", cWindowName);
//		UpdateSupportedActions();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("telescope", cAlpacaDevNum);
	}
	else
	{
		//========================================================
		validData	=	AlpacaGetStringValue(	"telescope", "description",	NULL,	returnString);
		if (validData)
		{
			SetWidgetText(kTab_Telescope, kTelescope_AlpacaDrvrVersion, returnString);
		}
	}

	validData	=	AlpacaGetStartupData_TelescopeOneAAT();


	SetWidgetValid(kTab_Telescope,	kTelescope_Readall,		cHas_readall);
	return(validData);
}

//*****************************************************************************
void	ControllerTelescope::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_Telescope, kTelescope_ErrorMsg, errorMsgString);
}

//*****************************************************************************
void	ControllerTelescope::AlpacaProcessReadAll(	const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char *valueString)
{
bool	dataWasHandled;

//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	if (strcasecmp(deviceTypeStr, "Telescope") == 0)
	{
		dataWasHandled	=	AlpacaProcessReadAll_Telescope(deviceNum, keywordString, valueString);
		if (dataWasHandled)
		{
			//*	we are done, skip the rest
		}
		else
		{
			AlpacaProcessReadAll_Common(	deviceTypeStr,
											deviceNum,
											keywordString,
											valueString);

		}
//		else if (strcasecmp(keywordString, "name") == 0)
//		{
//			SetWidgetText(kTab_DriverInfo, kDriverInfo_Name, valueString);
//		}
//		else if (strcasecmp(keywordString, "driverinfo") == 0)
//		{
//			SetWidgetText(kTab_DriverInfo, kDriverInfo_DriverInfo, valueString);
//		}
//		else if (strcasecmp(keywordString, "description") == 0)
//		{
//			SetWidgetText(kTab_DriverInfo, kDriverInfo_Description, valueString);
//		}
//		else if (strcasecmp(keywordString, "driverversion") == 0)
//		{
//			SetWidgetText(kTab_DriverInfo, kDriverInfo_DriverVersion, valueString);
//		}
//		else if (strcasecmp(keywordString, "interfaceversion") == 0)
//		{
//			SetWidgetText(kTab_DriverInfo, kDriverInfo_InterfaceVersion, valueString);
//		}

	}
#ifdef _ENABLE_SKYTRAVEL_
	else if (strcasecmp(deviceTypeStr, "somthingelse") == 0)
	{
		//*	you get the idea
	}
#endif // _ENABLE_SKYTRAVEL_
}


//*****************************************************************************
void	ControllerTelescope::AlpacaProcessSupportedActions(	const char	*deviceType,
															const int	deviveNum,
															const char	*valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, valueString);

	AlpacaProcessSupportedActions_Telescope(deviveNum, valueString);
}


//*****************************************************************************
bool	ControllerTelescope::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG(__FUNCTION__);

	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("telescope", cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_TelescopeOneAAT();
		validData	=	AlpacaGetCommonConnectedState("telescope");
	}

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
		cOnLine	=	true;

		//*	update telescope tracking
		if (cTelescopeProp.Tracking)
		{
			SetWidgetText(kTab_Telescope,		kTelescope_TrackingStatus, "Tracking is On");
			SetWidgetBGColor(kTab_Telescope,	kTelescope_TrackingStatus,	CV_RGB(0,	255,	0));
			SetWidgetTextColor(kTab_Telescope,	kTelescope_TrackingStatus,	CV_RGB(0,	0,	0));
		}
		else
		{
			SetWidgetText(kTab_Telescope,		kTelescope_TrackingStatus, "Tracking is Off");
			SetWidgetBGColor(kTab_Telescope,	kTelescope_TrackingStatus,	CV_RGB(0,	0,	0));
			SetWidgetTextColor(kTab_Telescope,	kTelescope_TrackingStatus,	CV_RGB(255,	0,	0));
		}

		//*	update slewing
		if (cTelescopeProp.Slewing)
		{
			SetWidgetText(kTab_Telescope,		kTelescope_SlewingStatus,	"Slewing is On");
			SetWidgetBGColor(kTab_Telescope,	kTelescope_SlewingStatus,	CV_RGB(0,	255,	0));
			SetWidgetTextColor(kTab_Telescope,	kTelescope_SlewingStatus,	CV_RGB(0,	0,	0));
		}
		else
		{
			SetWidgetText(kTab_Telescope,		kTelescope_SlewingStatus, "Slewing is Off");
			SetWidgetBGColor(kTab_Telescope,	kTelescope_SlewingStatus,	CV_RGB(0,	0,	0));
			SetWidgetTextColor(kTab_Telescope,	kTelescope_SlewingStatus,	CV_RGB(255,	0,	0));
		}

		SetWidgetChecked(kTab_Telescope,	kTelescope_TrackingRate_Sidereal,
												(cTelescopeProp.TrackingRate == kDriveRate_driveSidereal));

		SetWidgetChecked(kTab_Telescope,	kTelescope_TrackingRate_Lunar,
												(cTelescopeProp.TrackingRate == kDriveRate_driveLunar));

		SetWidgetChecked(kTab_Telescope,	kTelescope_TrackingRate_Solar,
												(cTelescopeProp.TrackingRate == kDriveRate_driveSolar));

		SetWidgetChecked(kTab_Telescope,	kTelescope_TrackingRate_King,
												(cTelescopeProp.TrackingRate == kDriveRate_driveKing));
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
void	ControllerTelescope::Update_TelescopeRtAscension(void)
{
char	hhmmssString[64];

	FormatHHMMSS(cTelescopeProp.RightAscension, hhmmssString, false);
	SetWidgetText(kTab_Telescope,	kTelescope_RA_value,		hhmmssString);
}

//**************************************************************************************
void	ControllerTelescope::Update_TelescopeDeclination(void)
{
char	hhmmssString[64];

	FormatHHMMSS(cTelescopeProp.Declination, hhmmssString, true);
	SetWidgetText(kTab_Telescope,	kTelescope_DEC_value,		hhmmssString);
}


//**************************************************************************************
void	ControllerTelescope::UpdateCapabilityList(void)
{
int		boxID;
int		iii;
char	textString[80];

//	CONSOLE_DEBUG(__FUNCTION__);

	iii	=	0;
	while (cCapabilitiesList[iii].capabilityName[0] != 0)
	{
		boxID	=	kCapabilities_TextBox1 + iii;
		strcpy(textString,	cCapabilitiesList[iii].capabilityName);
		strcat(textString,	":\t");
		strcat(textString,	cCapabilitiesList[iii].capabilityValue);

//		CONSOLE_DEBUG(textString);

		if (boxID <= kCapabilities_TextBoxN)
		{
			SetWidgetText(kTab_Capabilities, boxID, textString);
		}

		iii++;
	}
}



#define	_PARENT_IS_TELESCOPE_
#define	PARENT_CLASS	ControllerTelescope
#include	"controller_tscope_common.cpp"

#endif // _ENABLE_CTRL_TELESCOPE_

