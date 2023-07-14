//*****************************************************************************
//*		windowtab_DeviceState.cpp		(c) 2023 by Mark Sproul
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 19,	2023	<MLS> Created windowtab_DeviceState.cpp
//*	Jun 24,	2023	<MLS> Added SetDeviceStateNotSupported()
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_DeviceState.h"
#include	"controller.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabDeviceState::WindowTabDeviceState(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabDeviceState::~WindowTabDeviceState(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabDeviceState::SetupWindowControls(void)
{
int		yLoc;
int		boxNumber;
int		nameLeft;
int		nameWidth;
int		valueLeft;
int		valueWidth;
int		boxHeight;
int		boxWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kDeviceState_Title, -1, yLoc, "Device State");

	yLoc			+=	2;
	nameLeft		=	2;
	nameWidth		=	(cClmWidth * 2) - 4;
	valueLeft		=	nameLeft + nameWidth  + 2;
	valueWidth		=	(cClmWidth * 4);
	boxHeight		=	cSmallBtnHt + 2;

	//*	save this info for not supported
	cFirstBoxXloc	=	nameLeft;
	cFirstBoxYloc	=	yLoc;

	boxWidth		=	nameWidth + 2 + valueWidth;
	SetWidget(				kDeviceState_Stats,	nameLeft,		yLoc,	boxWidth,	boxHeight);
	SetWidgetFont(			kDeviceState_Stats,	kFont_Medium);
	SetWidgetJustification(	kDeviceState_Stats,	kJustification_Center);
	SetWidgetValid(			kDeviceState_Stats,	true);
	yLoc			+=	boxHeight;
	yLoc			+=	4;

	boxNumber		=	kDeviceState_FirstBoxName;
	while (boxNumber <= kDeviceState_LastBoxName)
	{
		//*	set the name box
		SetWidget(				boxNumber,	nameLeft,		yLoc,	nameWidth,	boxHeight);
		SetWidgetFont(			boxNumber,	kFont_Medium);
		SetWidgetJustification(	boxNumber,	kJustification_Left);
		SetWidgetValid(			boxNumber,	false);
		boxNumber++;

		//*	set the value box
		SetWidget(				boxNumber,	valueLeft,		yLoc,	valueWidth,	boxHeight);
		SetWidgetFont(			boxNumber,	kFont_Medium);
		SetWidgetJustification(	boxNumber,	kJustification_Left);
		SetWidgetValid(			boxNumber,	false);
		boxNumber++;

		yLoc			+=	boxHeight;
		yLoc			+=	4;
	}

	yLoc		+=	4;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kDeviceState_IPaddr,
							kDeviceState_Readall,
							kDeviceState_DeviceState,
							-1,
							-1,
							-1,
							-1);
}

//*****************************************************************************
void	WindowTabDeviceState::SetDeviceStateNotSupported(void)
{
int		boxWidth;

	//*	reconfigure the first name box for full width
	boxWidth	=	cWidth - (cFirstBoxXloc * 2);
	SetWidget(				kDeviceState_FirstBoxName,	cFirstBoxXloc,		cFirstBoxYloc,	boxWidth,	cBoxHeight);
	SetWidgetFont(			kDeviceState_FirstBoxName,	kFont_Large);
	SetWidgetJustification(	kDeviceState_FirstBoxName,	kJustification_Center);
	SetWidgetValid(			kDeviceState_FirstBoxName,	true);
	SetWidgetText(			kDeviceState_FirstBoxName,	"Device State not supported");

	SetWidgetValid(			kDeviceState_Stats,			false);
}


////*****************************************************************************
//void	WindowTabDeviceState::ProcessButtonClick(const int buttonIdx, const int flags)
//{
//
//	CONSOLE_DEBUG(__FUNCTION__);
////	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
//
////	switch(buttonIdx)
////	{
////
////	}
////	DisplayLastAlpacaCommand();
//}

