//*****************************************************************************
//*		controller_filterwheel.cpp		(c) 2021 by Mark Sproul
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
//*	May 26,	2021	<MLS> Created controller_filterwheel.cpp
//*	May 29,	2021	<MLS> FilterWheel controller working
//*	May 29,	2021	<MLS> Double Click on a filter circle moves to that filter
//*	Jun 18,	2021	<MLS> Added UpdateSupportedActions() to filterwheel
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jun 23,	2023	<MLS> Removed RunBackgroundTasks(), using default in parent class
//*	Jun 25,	2023	<MLS> Added AlpacaProcessReadAll() to filterwheel controller
//*	Jun 30,	2023	<MLS> Added DeviceState window to filterwheel controller
//*****************************************************************************
#ifdef _ENABLE_FILTERWHEEL_CONTROLLER_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include	"discovery_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define	kWindowWidth	450
#define	kWindowHeight	750

#include	"alpaca_defs.h"
#include	"windowtab_filterwheel.h"
#include	"windowtab_DeviceState.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"
#include	"helper_functions.h"
#include	"sendrequest_lib.h"

#include	"controller.h"
#include	"controller_filterwheel.h"
#include	"filterwheel_AlpacaCmds.h"
#include	"filterwheel_AlpacaCmds.cpp"

//**************************************************************************************
enum
{
	kTab_FilterWheel	=	1,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerFilterWheel::ControllerFilterWheel(	const char			*argWindowName,
												TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, true, alpacaDevice)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cDriverInfoTabNum		=	kTab_DriverInfo;
	cFilterWheelTabObjPtr	=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	memset(&cFilterWheelProp, 0, sizeof(TYPE_FilterWheelProperties));
	strcpy(cAlpacaDeviceTypeStr,	"filterwheel");
	cPositionCount			=	9;

	SetCommandLookupTable(gFilterwheelCmdTable);

	//*	copy the device address info
	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);
		strcpy(cAlpacaVersionString,	alpacaDevice->versionString);

		CheckConnectedState();		//*	check connected and connect if not already connected
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

	SetupWindowControls();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFilterWheel::~ControllerFilterWheel(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cFilterWheelTabObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerFilterWheel::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_FilterWheel,	"FilterWheel");
	SetTabText(kTab_DeviceState,	"Dev State");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");

	//--------------------------------------------
	cFilterWheelTabObjPtr	=	new WindowTabFilterWheel(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cFilterWheelTabObjPtr != NULL)
	{
		SetTabWindow(kTab_FilterWheel,	cFilterWheelTabObjPtr);
		cFilterWheelTabObjPtr->SetParentObjectPtr(this);

		CONSOLE_DEBUG("Calling SetPositonCount()");
		cFilterWheelTabObjPtr->SetPositonCount(cPositionCount);
		cFilterWheelTabObjPtr->SetFilterWheelPropPtr(&cFilterWheelProp);
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


	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	ipString[32];
	char	lineBuff[64];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//*****************************************************************************
void	ControllerFilterWheel::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
//	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_FilterWheel, kFilterWheel_AlpacaErrorMsg, errorMsgString);
}

//*****************************************************************************
bool	ControllerFilterWheel::AlpacaGetStartupData(void)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);

	validData	=	AlpacaGetFilterWheelStatus();
	AlpacaGetStartupData_OneAAT();

	return(validData);
}

//*****************************************************************************
bool	ControllerFilterWheel::AlpacaGetStartupData_OneAAT(void)
{
bool	validData;

	CONSOLE_DEBUG("Calling AlpacaGetFilterWheelStartup()");
	validData	=	AlpacaGetFilterWheelStartup();
	if (cFilterWheelTabObjPtr != NULL)
	{
		CONSOLE_DEBUG("Calling SetPositonCount()");
		cFilterWheelTabObjPtr->SetPositonCount(cPositionCount);
		CONSOLE_DEBUG("Returned from SetPositonCount()");
	}
	return(validData);
}

//**************************************************************************************
void	ControllerFilterWheel::UpdateConnectedStatusIndicator(void)
{
	UpdateConnectedIndicator(kTab_FilterWheel,		kFilterWheel_Connected);
}

//*****************************************************************************
bool	ControllerFilterWheel::AlpacaGetStatus(void)
{
bool	validData;

	validData	=	AlpacaGetFilterWheelStatus();
	cLastUpdate_milliSecs	=	millis();
	UpdateConnectedIndicator(kTab_FilterWheel,		kFilterWheel_Connected);
	return(validData);
}

//*****************************************************************************
void	ControllerFilterWheel::AlpacaGetCapabilities(void)
{
	//*	filter wheel does not have any "Capabilities"
	//*	required for base class
}

//*****************************************************************************
bool	ControllerFilterWheel::AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString)
{
int		argValueInt;
bool	dataWasHandled	=	true;

	CONSOLE_DEBUG("New version of AlpacaProcessReadAllIdx(), with enum");
	CONSOLE_DEBUG_W_NUM("keywordEnum\t=", keywordEnum);
	CONSOLE_DEBUG_W_STR("valueString\t=", valueString);
	argValueInt	=	atoi(valueString);
	switch(keywordEnum)
	{
		case kCmd_Filterwheel_focusoffsets:	//*	Filter focus offsets
		case kCmd_Filterwheel_names:		//*	Filter wheel filter names
			break;

		case kCmd_Filterwheel_position:		//*	GET-Returns the current filter wheel position
			CONSOLE_DEBUG_W_NUM("kCmd_Filterwheel_position\t=", argValueInt);
			//*	alpaca/ascom uses filter wheel positions from 0 -> N-1
			if ((argValueInt >= 0) && (argValueInt < kMaxFiltersPerWheel))
			{
				cFilterWheelProp.Position	=	argValueInt;
				cFilterWheelProp.IsMoving	=	false;
			}
			else if (argValueInt == -1)
			{
				cFilterWheelProp.IsMoving	=	true;
			}
			UpdateFilterWheelPosition();
			break;

		case kCmd_Filterwheel_Extras:
		case kCmd_Filterwheel_readall:
			break;

		default:
			dataWasHandled	=	false;
			break;
	}

//	CONSOLE_ABORT(cWindowName);
	return(dataWasHandled);
}

//**************************************************************************************
void	ControllerFilterWheel::UpdateStartupData(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
	CONSOLE_DEBUG_W_BOOL("cHas_readall       \t=",	cHas_readall);
	CONSOLE_DEBUG_W_BOOL("cHas_DeviceState   \t=",	cHas_DeviceState);
	CONSOLE_DEBUG_W_BOOL("cHas_temperaturelog\t=",	cHas_temperaturelog);

	UpdateFilterWheelInfo();
}

//**************************************************************************************
void	ControllerFilterWheel::UpdateStatusData(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	UpdateConnectedIndicator(kTab_FilterWheel,		kFilterWheel_Connected);
}

//*****************************************************************************
void	ControllerFilterWheel::UpdateSupportedActions(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_FilterWheel,	kFilterWheel_Readall,		cHas_readall);
	SetWidgetValid(kTab_FilterWheel,	kFilterWheel_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DeviceState,	kDeviceState_Readall,		cHas_readall);
	SetWidgetValid(kTab_DeviceState,	kDeviceState_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_DeviceState,	cHas_DeviceState);

	if (cHas_DeviceState == false)
	{
		cDeviceStateTabObjPtr->SetDeviceStateNotSupported();
	}
}

//*****************************************************************************
void	ControllerFilterWheel::UpdateFilterWheelInfo(void)
{

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	CONSOLE_DEBUG_W_STR("cFilterWheelName\t=", cFilterWheelName);
	SetWidgetText(	kTab_FilterWheel,	kFilterWheel_Name,	cFilterWheelName);
	cUpdateWindow	=	true;
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFilterWheel::UpdateFilterWheelPosition(void)
{

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//+	SetWidgetText(	kTab_Camera,	kCameraBox_FilterWheelName,	cFilterWheelName);
	cUpdateWindow	=	true;
}

#define	PARENT_CLASS	ControllerFilterWheel
#define	_PARENT_IS_FILTERWHEEL_
#include "controller_fw_common.cpp"

#endif // _ENABLE_FILTERWHEEL_CONTROLLER_
