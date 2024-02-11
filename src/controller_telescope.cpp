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
//*	May 29,	2022	<MLS> Added telescope settings tab
//*	Jun 18,	2023	<MLS> Added UpdateSupportedActions() to Telescope controller
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jun 21,	2023	<MLS> Added DeviceState window to telescope controller
//*	Jun 18,	2023	<MLS> Added UpdateSupportedActions() to Telescope controller
//*	Jul 14,	2023	<MLS> Added UpdateOnlineStatus() to Telescope controller
//*	Jan 15,	2024	<MLS> Added Update_TelescopeSideOfPier()
//*	Jan 16,	2024	<MLS> Changed AlpacaGetStatus() to GetStatus_SubClass()
//*****************************************************************************

#ifdef _ENABLE_CTRL_TELESCOPE_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include	"discovery_lib.h"
#include	"helper_functions.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	700
#define	kWindowHeight	600

#include	"windowtab_telescope.h"
#include	"windowtab_teleSettings.h"
#include	"windowtab_capabilities.h"
#include	"windowtab_DeviceState.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_telescope.h"
#include	"telescope_AlpacaCmds.h"
#include	"telescope_AlpacaCmds.cpp"


//**************************************************************************************
enum
{
	kTab_TelescopCtl	=	1,
	kTab_Settings,
	kTab_Capabilities,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerTelescope::ControllerTelescope(	const char			*argWindowName,
											TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, true, alpacaDevice)
{

	//*	zero out all of the Telescope ASCOM properties
	memset(&cTelescopeProp, 0, sizeof(TYPE_TelescopeProperties));
	strcpy(cAlpacaDeviceTypeStr,	"telescope");

	SetCommandLookupTable(gTelescopeCmdTable);
	SetAlternateLookupTable(gTelescopeExtrasTable);
	cDriverInfoTabNum		=	kTab_DriverInfo;
	cDriverInfoTabNum		=	kTab_DriverInfo;
	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();
	cTelescopeTabObjPtr		=	NULL;
	cTeleSettingsTabObjPtr	=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cLastUpdate_milliSecs	=	0;
	cTelescopeProp.PhysicalSideOfPier	=	kPierSide_NotAvailable;

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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	//--------------------------------------------
	DELETE_OBJ_IF_VALID(cTelescopeTabObjPtr);
	DELETE_OBJ_IF_VALID(cTeleSettingsTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
	DELETE_OBJ_IF_VALID(cCapabilitiesTabObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);

	//*	check to see if we are slewing
	if (cTelescopeProp.Slewing)
	{
	bool	validData;

		CONSOLE_DEBUG("Slewing is active, shutting it down");
		validData		=	AlpacaSendPutCmd(	"telescope", "abortslew",	NULL);
		if (validData == false)
		{
			CONSOLE_DEBUG("abortslew failed!!!!!!!!!!!");
		}
	}
}


//**************************************************************************************
void	ControllerTelescope::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_TelescopCtl,	"Control");
	SetTabText(kTab_Settings,		"Settings");
	SetTabText(kTab_Capabilities,	"Capabilities");
	SetTabText(kTab_DeviceState,	"Dev State");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");


	//--------------------------------------------
	cTelescopeTabObjPtr	=	new WindowTabTelescope(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cTelescopeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_TelescopCtl,	cTelescopeTabObjPtr);
		cTelescopeTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cTeleSettingsTabObjPtr	=	new WindowTabTeleSettings(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cTeleSettingsTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Settings,	cTeleSettingsTabObjPtr);
		cTeleSettingsTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDeviceStateTabObjPtr		=	new WindowTabDeviceState(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDeviceStateTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DeviceState,	cDeviceStateTabObjPtr);
		cDeviceStateTabObjPtr->SetParentObjectPtr(this);
		SetDeviceStateTabInfo(kTab_DeviceState, kDeviceState_FirstBoxName, kDeviceState_FirstBoxValue, kDeviceState_Stats);
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

	SetWidgetFont(kTab_TelescopCtl,	kTelescope_IPaddr, kFont_Medium);

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
//*****************************************************************************
void	ControllerTelescope::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}

//**************************************************************************************
void	ControllerTelescope::UpdateStartupData(void)
{
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);

}

//**************************************************************************************
void	ControllerTelescope::UpdateConnectedStatusIndicator(void)
{
	UpdateConnectedIndicator(kTab_TelescopCtl,	kTelescope_Connected);
}

//**************************************************************************************
void	ControllerTelescope::UpdateStatusData(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	UpdateConnectedIndicator(kTab_TelescopCtl,	kTelescope_Connected);
}

//*****************************************************************************
//*	this routine gets called one time to get the info on the telescope that does not change
//*****************************************************************************
bool	ControllerTelescope::AlpacaGetStartupData(void)
{
bool	validData;
//char	returnString[256];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("telescope", cAlpacaDevNum);
	if (validData)
	{
		CONSOLE_DEBUG_W_STR("Valid supported actions:", cWindowName);
		UpdateSupportedActions();
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
//		//========================================================
//		validData	=	AlpacaGetStringValue(	"telescope", "description",	NULL,	returnString);
	}
	validData	=	AlpacaGetStartupData_TelescopeOneAAT();
	if (cTelescopeTabObjPtr != NULL)
	{
		cTelescopeTabObjPtr->UpdateTelescopeInfo(&cTelescopeProp, true);
	}
	if (cTeleSettingsTabObjPtr != NULL)
	{
		cTeleSettingsTabObjPtr->UpdateTelescopeInfo(&cTelescopeProp, true);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

	return(validData);
}

//**************************************************************************************
void	ControllerTelescope::UpdateOnlineStatus(void)
{
cv::Scalar	bgColor;
cv::Scalar	txtColor;

	bgColor		=	cOnLine ? CV_RGB(0,		0,	0)	: CV_RGB(255,	0,	0);
	txtColor	=	cOnLine ? CV_RGB(255,	0,	0)	: CV_RGB(0,		0,	0);

	SetWidgetBGColor(	kTab_TelescopCtl,	kTelescope_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_TelescopCtl,	kTelescope_IPaddr,		txtColor);

	SetWidgetBGColor(	kTab_Settings,		kTeleSettings_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_Settings,		kTeleSettings_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_Capabilities,	kCapabilities_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_Capabilities,	kCapabilities_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DeviceState,	kDeviceState_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_DeviceState,	kDeviceState_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		txtColor);
}

//*****************************************************************************
void	ControllerTelescope::AlpacaGetCapabilities(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	ReadOneTelescopeCapability("canfindhome",			"CanFindHome",			&cTelescopeProp.CanFindHome);
	ReadOneTelescopeCapability("canmoveaxis?Axis=0",	"CanMoveAxis-RA",		&cTelescopeProp.CanMoveAxis[0]);
	ReadOneTelescopeCapability("canmoveaxis?Axis=1",	"CanMoveAxis-DEC",		&cTelescopeProp.CanMoveAxis[1]);
	ReadOneTelescopeCapability("canmoveaxis?Axis=2",	"CanMoveAxis-3",		&cTelescopeProp.CanMoveAxis[2]);
	ReadOneTelescopeCapability("canpark",				"CanPark",				&cTelescopeProp.CanPark);
	ReadOneTelescopeCapability("canpulseguide",			"CanPulseGuide",		&cTelescopeProp.CanPulseGuide);
	ReadOneTelescopeCapability("cansetdeclinationrate",	"CanSetDeclinationRate",
																				&cTelescopeProp.CanSetDeclinationRate);
	ReadOneTelescopeCapability("cansetguiderates",		"CanSetGuideRates",		&cTelescopeProp.CanSetGuideRates);
	ReadOneTelescopeCapability("cansetpark",			"CanSetPark",			&cTelescopeProp.CanSetPark);

	ReadOneTelescopeCapability("cansetpierside",		"CanSetPierSide",		&cTelescopeProp.CanSetPierSide);
	ReadOneTelescopeCapability("cansetrightascensionrate",
														"CanSetRightAscensionRate",
																				&cTelescopeProp.CanSetRightAscensionRate);
	ReadOneTelescopeCapability("cansettracking",		"CanSetTracking",		&cTelescopeProp.CanSetTracking);
	ReadOneTelescopeCapability("canslew",				"CanSlew",				&cTelescopeProp.CanSlew);
	ReadOneTelescopeCapability("canslewaltaz",			"CanSlewAltAz",			&cTelescopeProp.CanSlewAltAz);
	ReadOneTelescopeCapability("canslewaltazasync",		"CanSlewAltAzAsync",	&cTelescopeProp.CanSlewAltAzAsync);
	ReadOneTelescopeCapability("canslewasync",			"CanSlewAsync",			&cTelescopeProp.CanSlewAsync);
	ReadOneTelescopeCapability("cansync",				"CanSync",				&cTelescopeProp.CanSync);
	ReadOneTelescopeCapability("cansyncaltaz",			"CanSyncAltAz",			&cTelescopeProp.CanSyncAltAz);
	ReadOneTelescopeCapability("canunpark",				"CanUnpark",			&cTelescopeProp.CanUnpark);
	ReadOneTelescopeCapability("doesrefraction",		"DoesRefraction",		&cTelescopeProp.DoesRefraction);
}

//*****************************************************************************
void	ControllerTelescope::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_TelescopCtl,	kTelescope_AlpacaErrorMsg,		errorMsgString);
	SetWidgetText(kTab_Settings,	kTeleSettings_AlpacaErrorMsg,	errorMsgString);
}

//*****************************************************************************
bool	ControllerTelescope::AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
															const int	deviceNum,
															const int	keywordEnum,
															const char	*valueString)
{
bool		dataWasHandled;

//	CONSOLE_DEBUG(__FUNCTION__);
	dataWasHandled	=	AlpacaProcessReadAll_TelescopeIdx(deviceNum, keywordEnum, valueString);

	return(dataWasHandled);
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
void	ControllerTelescope::GetStatus_SubClass(void)
{
bool	validData;
bool	previousOnLineState;

	CONSOLE_DEBUG(__FUNCTION__);

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

		//*	update the window tabs with everything
		if (cTelescopeTabObjPtr != NULL)
		{
			cTelescopeTabObjPtr->UpdateTelescopeInfo(&cTelescopeProp, false);
		}
		if (cTeleSettingsTabObjPtr != NULL)
		{
			//*	false means only update the dynamic properties
			cTeleSettingsTabObjPtr->UpdateTelescopeInfo(&cTelescopeProp, false);
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

	//*	does this device have "DeviceState"
	if (cOnLine && cHas_DeviceState)
	{
	//	AlpacaGetStatus_DeviceState();
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
//	return(validData);
}

//**************************************************************************************
void	ControllerTelescope::Update_TelescopeRtAscension(void)
{
char	hhmmssString[64];

	FormatHHMMSS(cTelescopeProp.RightAscension, hhmmssString, false);
	SetWidgetText(kTab_TelescopCtl,	kTelescope_RA_value,		hhmmssString);
}

//**************************************************************************************
void	ControllerTelescope::Update_TelescopeDeclination(void)
{
char	hhmmssString[64];

	FormatHHMMSS(cTelescopeProp.Declination, hhmmssString, true);
	SetWidgetText(kTab_TelescopCtl,	kTelescope_DEC_value,		hhmmssString);
}

//**************************************************************************************
void	ControllerTelescope::Update_TelescopeSideOfPier(void)
{
char	dataString[64];

	//*	side of pier - logical
	GetSideOfPierString(cTelescopeProp.SideOfPier, dataString);
	SetWidgetText(kTab_TelescopCtl,	kTelescope_SideOfPier_value,	dataString);

	//*	side of pier - physical
	GetSideOfPierString(cTelescopeProp.PhysicalSideOfPier, dataString);
	SetWidgetText(kTab_TelescopCtl,	kTelescope_PhysSideOfPier_value,	dataString);
}



//*****************************************************************************
void	ControllerTelescope::UpdateSupportedActions(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_TelescopCtl,	kTelescope_Readall,			cHas_readall);
	SetWidgetValid(kTab_TelescopCtl,	kTelescope_DeviceState,		cHas_DeviceState);

	SetWidgetValid(kTab_Settings,		kTeleSettings_Readall,		cHas_readall);
	SetWidgetValid(kTab_Settings,		kTeleSettings_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_Capabilities,	kCapabilities_Readall,		cHas_readall);
	SetWidgetValid(kTab_Capabilities,	kCapabilities_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DeviceState,	kDeviceState_Readall,		cHas_readall);
	SetWidgetValid(kTab_DeviceState,	kDeviceState_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_DeviceState,	cHas_DeviceState);

	if (cHas_DeviceState == false)
	{
		cDeviceStateTabObjPtr->SetDeviceStateNotSupported();
	}
}

//**************************************************************************************
void	ControllerTelescope::UpdateCapabilityList(void)
{
	UpdateCapabilityListID(kTab_Capabilities, kCapabilities_TextBox1, kCapabilities_TextBoxN);
}

//*****************************************************************************
void	GetSideOfPierString(TYPE_PierSide sideOfPier, char *sideOfPierString)
{
	switch(sideOfPier)
	{
		case kPierSide_NotAvailable:
			strcpy(sideOfPierString, "Not available");
			break;

		//*	Normal pointing state - Mount on the East side of pier (looking West)
		case kPierSide_pierEast:
			strcpy(sideOfPierString, "East");
			break;

		//*	Through the pole pointing state - Mount on the West side of pier (looking East)
		case kPierSide_pierWest:
			strcpy(sideOfPierString, "West");
			break;

		case kPierSide_pierUnknown:
			strcpy(sideOfPierString, "unkown");
			break;

		default:
			strcpy(sideOfPierString, "error");
			break;
	}
}

//#define	_PARENT_IS_TELESCOPE_
//#ifdef _ENABLE_SKYTRAVEL_
//	#undef _ENABLE_SKYTRAVEL_
//	#define	PARENT_CLASS	ControllerTelescope
//	#include	"controller_tscope_common.cpp"
//#endif


#define	_PARENT_IS_TELESCOPE_
#define	PARENT_CLASS	ControllerTelescope
#include	"controller_tscope_common.cpp"

#endif // _ENABLE_CTRL_TELESCOPE_

