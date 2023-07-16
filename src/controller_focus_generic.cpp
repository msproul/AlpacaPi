//*****************************************************************************
//*		controller_focus_generic.cpp		(c) 2021 by Mark Sproul
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
//*	Jan 14,	2021	<MLS> Created controller_focus_generic.cpp
//*	Jan 14,	2021	<MLS> Focuser controller working with ASCOM/Remote
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jul  9,	2023	<MLS> Added device-state window to Focuser-generic
//*	Jul 14,	2023	<MLS> Added UpdateOnlineStatus() to Focuser-generic controller
//*****************************************************************************

#if defined(_ENABLE_CTRL_FOCUSERS_)

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_ml_single.h"
#include	"windowtab_config.h"
#include	"windowtab_about.h"
#include	"focuser_common.h"

#include	"controller.h"
#include	"controller_focus.h"
#include	"controller_focus_generic.h"

//**************************************************************************************
enum
{
	kTab_Focuser	=	1,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerFocusGeneric::ControllerFocusGeneric(	const char			*argWindowName,
												TYPE_REMOTE_DEV		*alpacaDevice,
												const int			focuserType)
	:ControllerFocus(	argWindowName,
						alpacaDevice,
						kFocuserType_MoonliteSingle)
{
	CONSOLE_DEBUG(__FUNCTION__);


	cDriverInfoTabNum		=	kTab_DriverInfo;

	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_Alpaca, kFocuserType_MoonliteSingle);

	CreateWindowTabs();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFocusGeneric::~ControllerFocusGeneric(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	//*	delete the windowtab objects
	DELETE_OBJ_IF_VALID(cMLsingleTabObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerFocusGeneric::CreateWindowTabs(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);

	SetTabText(kTab_Focuser,		"Focuser");
	SetTabText(kTab_DeviceState,	"Dev State");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");

	//================================================================
	cMLsingleTabObjPtr		=	new WindowTabMLsingle(	cWidth,
														cHeight,
														cBackGrndColor,
														cCommMode,
														kFocuserType_Other,
														cWindowName);
	if (cMLsingleTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Focuser,	cMLsingleTabObjPtr);
		cMLsingleTabObjPtr->SetParentObjectPtr(this);
	}

	//================================================================
	cDeviceStateTabObjPtr		=	new WindowTabDeviceState(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDeviceStateTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DeviceState,	cDeviceStateTabObjPtr);
		cDeviceStateTabObjPtr->SetParentObjectPtr(this);
		SetDeviceStateTabInfo(kTab_DeviceState, kDeviceState_FirstBoxName, kDeviceState_FirstBoxValue, kDeviceState_Stats);
	}

	//================================================================
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}


	//================================================================
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

//		CONSOLE_DEBUG(__FUNCTION__);
		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateCommonProperties(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

	SetWidgetText(kTab_Focuser,			kMLsingle_Model,				cCommonProp.Description);


	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//**************************************************************************************
void	ControllerFocusGeneric::UpdateStartupData(void)
{
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
}

//**************************************************************************************
void	ControllerFocusGeneric::UpdateStatusData(void)
{
char	lineBuff[64];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cFocuserProp.Position\t=",	cFocuserProp.Position);
	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos   \t=",	cFocuserDesiredPos);
	UpdateConnectedIndicator(	kTab_Focuser,	kMLsingle_Connected);
	SetWidgetNumber(			kTab_Focuser,	kMLsingle_focValue,		cFocuserProp.Position);
	SetWidgetNumber(			kTab_Focuser,	kMLsingle_focDesired,	cFocuserDesiredPos);

	sprintf(lineBuff, "%1.1f C / %1.1f F", cFocuserProp.Temperature_DegC, ((cFocuserProp.Temperature_DegC * (9.0/5.0)) + 32));
	SetWidgetText(kTab_Focuser, kMLsingle_Temperature, lineBuff);
}

//**************************************************************************************
void	ControllerFocusGeneric::UpdateOnlineStatus(void)
{
cv::Scalar	bgColor;
cv::Scalar	txtColor;

	bgColor		=	cOnLine ? CV_RGB(0,		0,	0)	: CV_RGB(255,	0,	0);
	txtColor	=	cOnLine ? CV_RGB(255,	0,	0)	: CV_RGB(0,		0,	0);

	SetWidgetBGColor(	kTab_Focuser,		kMLsingle_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_Focuser,		kMLsingle_IPaddr,		txtColor);

	SetWidgetBGColor(	kTab_DeviceState,	kDeviceState_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_DeviceState,	kDeviceState_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		txtColor);
}

//**************************************************************************************
void	ControllerFocusGeneric::UpdateFocuserPosition(void)
{
	SetWidgetNumber(			kTab_Focuser,	kMLsingle_focValue,		cFocuserProp.Position);
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateSupportedActions(void)
{
	SetWidgetValid(kTab_Focuser,		kMLsingle_Readall,			cHas_readall);
	SetWidgetValid(kTab_Focuser,		kMLsingle_DeviceState,		cHas_DeviceState);

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
void	ControllerFocusGeneric::UpdateWindowTabs_Everything(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cFocuserProp.Position\t=",	cFocuserProp.Position);
//	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos   \t=",	cFocuserDesiredPos);

	SetWidgetNumber(kTab_Focuser,	kMLsingle_focValue,		cFocuserProp.Position);
	SetWidgetNumber(kTab_Focuser,	kMLsingle_focDesired,	cFocuserDesiredPos);

	UpdateConnectedIndicator(kTab_Focuser,		kMLsingle_Connected);
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetNumber(kTab_Focuser, kMLsingle_focDesired, cFocuserDesiredPos);
}

//*****************************************************************************
void	ControllerFocusGeneric::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_Focuser, kMLsingle_AlpacaErrorMsg, errorMsgString);
}


//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_ConnectState(bool connectedState)
{
	if (connectedState)
	{
		SetWidgetText(kTab_Focuser, kMLsingle_Connect, "Disconnect");
	}
	else
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		SetWidgetText(kTab_Focuser, kMLsingle_Connect,		"Connect");
		SetWidgetText(kTab_Focuser, kMLsingle_Temperature,	"NO DATA");
		SetWidgetText(kTab_Focuser, kMLsingle_focValue,	"NO DATA");
		SetWidgetText(kTab_Focuser, kMLsingle_focDesired,	"-----");
	}
}

#endif // defined(_ENABLE_CTRL_FOCUSERS_)
