//*****************************************************************************
//*		controller_ml_single.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 24,	2020	<MLS> Created controller_ml_single.cpp
//*	Apr 24,	2020	<MLS> Started on controller for Moonlite single focuser
//*	Apr 24,	2020	<MLS> Moonlite single focuser working as subclass
//*	Apr 25,	2020	<MLS> Cleaned up Moonlite single as remote device
//*	Jun 15,	2020	<MLS> Got ML Hi-Res Stepper and Mini Controller V2 from Cloudy Nights
//*	Jun 19,	2020	<MLS> Tested with Mini Controller V2
//*	Jul 16,	2020	<MLS> Moonlite focuser tested on 64bit Raspberry Pi OS
//*****************************************************************************


#if defined(_ENABLE_USB_FOCUSERS_) || defined(_ENABLE_CTRL_FOCUSERS_)

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700

#include	"windowtab_ml_single.h"
#include	"windowtab_config.h"
#include	"windowtab_about.h"
#include	"focuser_common.h"

#include	"controller.h"
#include	"controller_focus.h"
#include	"controller_focus_ml_hr.h"

//**************************************************************************************
enum
{
	kTab_MLsingle	=	1,
	kTab_Config,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};


//**************************************************************************************
ControllerMLsingle::ControllerMLsingle(	const char			*argWindowName,
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum)
	:ControllerFocus(	argWindowName,
						deviceAddress,
						port,
						deviceNum,
						kFocuserType_MoonliteSingle)
{


	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_Alpaca, kFocuserType_MoonliteSingle);


	cAlpacaDevNum	=	deviceNum;
	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

	CreateWindowTabs();

}


//**************************************************************************************
ControllerMLsingle::ControllerMLsingle(	const char			*argWindowName,
										const char			*usbPortPath)
	:ControllerFocus(argWindowName, usbPortPath,  kFocuserType_MoonliteSingle)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_USB, kFocuserType_MoonliteSingle);

	if (usbPortPath != NULL)
	{
		strcpy(cUSBpath, usbPortPath);
		cValidUSB	=	true;
	}
	CreateWindowTabs();

	SetWindowIPaddrInfo(cUSBpath, true);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerMLsingle::~ControllerMLsingle(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	delete the windowtab objects
	DELETE_OBJ_IF_VALID(cMLsingleTabObjPtr);
	DELETE_OBJ_IF_VALID(cConfigTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerMLsingle::CreateWindowTabs(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);

	SetTabText(kTab_MLsingle,	"Focuser");
	SetTabText(kTab_Config,		"Config");
	SetTabText(kTab_DriverInfo,	"Driver Info");
	SetTabText(kTab_About,		"About");

	//================================================================
	cMLsingleTabObjPtr		=	new WindowTabMLsingle(	cWidth,
														cHeight,
														cBackGrndColor,
														cCommMode,
														kFocuserType_MoonliteSingle,
														cWindowName);
	if (cMLsingleTabObjPtr != NULL)
	{
		SetTabWindow(kTab_MLsingle,	cMLsingleTabObjPtr);
		cMLsingleTabObjPtr->SetParentObjectPtr(this);
	}

	//================================================================
	cConfigTabObjPtr	=	new WindowTabConfig(	cWidth, cHeight, cBackGrndColor);
	if (cConfigTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Config,	cConfigTabObjPtr);
		cConfigTabObjPtr->SetParentObjectPtr(this);
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

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//*****************************************************************************
void	ControllerMLsingle::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_MLsingle, kMLsingle_AlpacaErrorMsg, errorMsgString);
}

//*****************************************************************************
void	ControllerMLsingle::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

	SetWidgetText(kTab_MLsingle,		kMLsingle_Model,				cCommonProp.Description);

	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
}

//**************************************************************************************
void	ControllerMLsingle::UpdateFocuserPosition(const int newFocuserPosition)
{
	cFocuserPosition	=	newFocuserPosition;

	SetWidgetNumber(kTab_MLsingle,	kMLsingle_focValue,		cFocuserPosition);

}

//**************************************************************************************
void	ControllerMLsingle::UpdateTemperature(const double newTemperature)
{
char	lineBuff[64];

	cTemperature_DegC	=	newTemperature;
	sprintf(lineBuff, "%1.1f C / %1.1f F", cTemperature_DegC, ((cTemperature_DegC * (9.0/5.0)) + 32));
	SetWidgetText(kTab_MLsingle, kMLsingle_Temperature, lineBuff);

//-	if (cGraphTabObjPtr != NULL)
//-	{
//-		cGraphTabObjPtr->LogTemperature(cTemperature_DegC);
//-		if (cCurrentTabNum == kTab_Graphs)
//-		{
//-			cUpdateWindow	=	true;
//-		}
//-	}

}


//*****************************************************************************
void	ControllerMLsingle::UpdateWindowTabs_Everything(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cFocuserPosition\t=",		cFocuserPosition);
//	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos\t=",	cFocuserDesiredPos);

	SetWidgetNumber(kTab_MLsingle,	kMLsingle_focValue,		cFocuserPosition);
	SetWidgetNumber(kTab_MLsingle,	kMLsingle_focDesired,	cFocuserDesiredPos);

	UpdateConnectedIndicator(kTab_MLsingle,		kMLsingle_Connected);

}

//*****************************************************************************
void	ControllerMLsingle::UpdateWindowTabs_ReadAll(bool hasReadAll)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_MLsingle,	kMLsingle_Readall,		cHas_readall);
	SetWidgetValid(kTab_Config,		kCongfigBox_Readall,	cHas_readall);
}

//*****************************************************************************
void	ControllerMLsingle::UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetNumber(kTab_MLsingle, kMLsingle_focDesired, cFocuserDesiredPos);
}

//*****************************************************************************
void	ControllerMLsingle::UpdateWindowTabs_ConnectState(bool connectedState)
{
	if (connectedState)
	{
		SetWidgetText(kTab_MLsingle, kMLsingle_Connect, "Disconnect");
	}
	else
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		SetWidgetText(kTab_MLsingle, kMLsingle_Connect,		"Connect");
		SetWidgetText(kTab_MLsingle, kMLsingle_Temperature,	"NO DATA");
		SetWidgetText(kTab_MLsingle, kMLsingle_focValue,	"NO DATA");
		SetWidgetText(kTab_MLsingle, kMLsingle_focDesired,	"-----");
	}
}

#endif // defined(_ENABLE_USB_FOCUSERS_) || defined(_ENABLE_CTRL_FOCUSERS_)
