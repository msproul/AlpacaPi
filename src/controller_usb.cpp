//*****************************************************************************
//*		controller_usb.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 23,	2020	<MLS> Created controller_usb.cpp
//*****************************************************************************


#ifdef _ENABLE_USB_FOCUSERS_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700

#include	"windowtab_usb.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_usb.h"

//**************************************************************************************
enum
{
	kTab_USB	=	1,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerUSB::ControllerUSB(	const char			*argWindowName,
								const char			*usbPath)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
	strcpy(cUSBpath, usbPath);

	SetupWindowControls();

	SetWidgetText(kTab_USB,		kUSBselect_AlpacaDrvrVersion,		gFullVersionString);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerUSB::~ControllerUSB(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cUSBTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerUSB::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_USB,		"USB");
	SetTabText(kTab_About,		"About");


	cUSBTabObjPtr	=	new WindowTabUSB(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cUSBTabObjPtr != NULL)
	{
		SetTabWindow(kTab_USB,	cUSBTabObjPtr);
		cUSBTabObjPtr->SetParentObjectPtr(this);
	}

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	SetWidgetFont(kTab_USB,		kUSBselect_IPaddr, kFont_Medium);

	SetWidgetText(kTab_USB,		kUSBselect_IPaddr,	cUSBpath);
}

//**************************************************************************************
void	ControllerUSB::RunBackgroundTasks(bool enableDebug)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

}


#endif // _ENABLE_USB_FOCUSERS_
