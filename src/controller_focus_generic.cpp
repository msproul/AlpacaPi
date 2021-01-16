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
//*****************************************************************************


#if defined(_ENABLE_CTRL_FOCUSERS_)

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/version.hpp"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700

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
	kTab_About,

	kTab_Count

};

#define	kFocuserBoxWidth	302
#define	kFocuserBoxHeight	715


//**************************************************************************************
ControllerFocusGeneric::ControllerFocusGeneric(	const char			*argWindowName,
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
	}

	CreateWindowTabs();

}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFocusGeneric::~ControllerFocusGeneric(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	delete the windowtab objects
	if (cMLsingleTabObjPtr != NULL)
	{
		delete cMLsingleTabObjPtr;
		cMLsingleTabObjPtr	=	NULL;
	}

	if (cAboutBoxTabObjPtr != NULL)
	{
		delete cAboutBoxTabObjPtr;
		cAboutBoxTabObjPtr	=	NULL;
	}
}

//**************************************************************************************
void	ControllerFocusGeneric::CreateWindowTabs(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);

	SetTabText(kTab_Focuser,	"Focuser");
	SetTabText(kTab_About,		"About");

	//================================================================
	cMLsingleTabObjPtr		=	new WindowTabMLsingle(	cWidth,
														cHeight,
														cBackGrndColor,
														cCommMode,
														cWindowName);
	if (cMLsingleTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Focuser,	cMLsingleTabObjPtr);
		cMLsingleTabObjPtr->SetParentObjectPtr(this);
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

		CONSOLE_DEBUG(__FUNCTION__);
		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);
	}
}


//**************************************************************************************
void	ControllerFocusGeneric::UpdateFocuserPostion(const int newFocuserPostion)
{
	cFocuserPosition	=	newFocuserPostion;

	SetWidgetNumber(kTab_Focuser,	kMLsingle_focValue,		cFocuserPosition);

}

//**************************************************************************************
void	ControllerFocusGeneric::UpdateTemperature(const double newTemperature)
{
char	lineBuff[64];

	cTemperature_DegC	=	newTemperature;
	sprintf(lineBuff, "%1.1f C / %1.1f F", cTemperature_DegC, ((cTemperature_DegC * (9.0/5.0)) + 32));
	SetWidgetText(kTab_Focuser, kMLsingle_Temperature, lineBuff);

}


//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_Everything(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cFocuserPosition\t=",		cFocuserPosition);
//	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos\t=",	cFocuserDesiredPos);

	SetWidgetNumber(kTab_Focuser,	kMLsingle_focValue,		cFocuserPosition);
	SetWidgetNumber(kTab_Focuser,	kMLsingle_focDesired,	cFocuserDesiredPos);
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_ReadAll(bool hasReadAll)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_Focuser,	kMLsingle_Readall,		cHas_readall);
	SetWidgetValid(kTab_About,		kAboutBox_Readall,		cHas_readall);
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_Version(const char *versionString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, versionString);
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cAlpacaVersionString);

	SetWidgetText(kTab_Focuser,	kMLsingle_AlpacaDrvrVersion,	cAlpacaVersionString);
	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,	cAlpacaVersionString);
}

//*****************************************************************************
void	ControllerFocusGeneric::UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetNumber(kTab_Focuser, kMLsingle_focDesired, cFocuserDesiredPos);
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
