//*****************************************************************************
//*		windowtab_usb.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:	C++ Client to talk to Alpaca devices
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 23,	2020	<MLS> Created windowtab_usb.cpp
//*	Apr 21,	2021	<MLS> Fixed bug in button setup
//*	Jan 22,	2023	<MLS> Updated moonlite include files
//*****************************************************************************

#ifdef _ENABLE_USB_FOCUSERS_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_usb.h"
#include	"controller.h"

#include	"controller_usb.h"

#include	"controller_focus.h"
#include	"controller_focus_ml_nc.h"
#include	"controller_focus_ml_hr.h"

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabUSB::WindowTabUSB(	const int	xSize,
							const int	ySize,
							cv::Scalar	backGrndColor,
							const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabUSB::~WindowTabUSB(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabUSB::SetupWindowControls(void)
{
int		yLoc;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kUSBselect_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kUSBselect_Title, "USB port selection");
	SetBGcolorFromWindowName(kUSBselect_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	yLoc			+=	100;

	SetWidget(			kUSBselect_Instructions,	15,			yLoc,		cWidth-30,		cTitleHeight);
	SetWidgetText(		kUSBselect_Instructions,	"Select focuser type");
	SetWidgetBorder(	kUSBselect_Instructions,	false);
	yLoc			+=	cTitleHeight;
	yLoc			+=	20;

	//*	create the buttons for each USB focuser type
	for (iii=kUSBselect_BtnNiteCrawler; iii<=kUSBselect_LastOption; iii++)
	{
		SetWidget(			iii,	15,			yLoc,		cWidth-30,		cTitleHeight);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetBGColor(	iii,	CV_RGB(255, 255, 255));

		yLoc			+=	cTitleHeight;
		yLoc			+=	8;
	}

	SetWidgetText(kUSBselect_BtnNiteCrawler,	"Moonlite NiteCrawler");
	SetWidgetText(kUSBselect_BtnSingle,			"Moonlite Single Focuser");
//	SetWidgetText(kUSBselect_BtnCrawler,		"Moonlite Crawler");

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kUSBselect_IPaddr,
							-1,	//	kUSBselect_Readall,
							-1,	//	kUSBselect_AlpacaErrorMsg,
							-1,	//	kUSBselect_LastCmdString,
							kUSBselect_AlpacaLogo,
							-1);

}

//*****************************************************************************
void	WindowTabUSB::ProcessButtonClick(const int buttonIdx, const int flags)
{
ControllerUSB	*parentController;
char			usbPath[45];

	CONSOLE_DEBUG(__FUNCTION__);
	switch(buttonIdx)
	{
		case kUSBselect_BtnNiteCrawler:
			if (cParentObjPtr != NULL)
			{
				parentController	=	(ControllerUSB *)cParentObjPtr;
				strcpy(usbPath, parentController->cUSBpath);

				new ControllerNiteCrawler("NiteCrawler", usbPath);

				parentController->cKeepRunning	=	false;
			}
			else
			{
				CONSOLE_DEBUG("parent ptr is NULL");
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;

		case kUSBselect_BtnSingle:
			if (cParentObjPtr != NULL)
			{
				parentController	=	(ControllerUSB *)cParentObjPtr;
				strcpy(usbPath, parentController->cUSBpath);

				new ControllerMLsingle("Moonlite Single Focuser", usbPath);

				parentController->cKeepRunning	=	false;
			}
			else
			{
				CONSOLE_DEBUG("parent ptr is NULL");
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;


		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;

	}
	DisplayLastAlpacaCommand();
}

#endif // _ENABLE_USB_FOCUSERS_
