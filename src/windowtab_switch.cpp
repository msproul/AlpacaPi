//*****************************************************************************
//*		windowtab_switch.cpp		(c) 2020 by Mark Sproul
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
//*	Feb 29,	2020	<MLS> Created windowtab_switch.cpp
//*	Feb 28,	2021	<MLS> Added TurnAllSwitchesOff()
//*	Jun 19,	2023	<MLS> Added DeviceState to Switch
//*****************************************************************************

#ifdef _ENABLE_CTRL_SWITCHES_

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_switch.h"
#include	"controller_switch.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabSwitch::WindowTabSwitch(	const int	xSize,
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
WindowTabSwitch::~WindowTabSwitch(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabSwitch::SetupWindowControls(void)
{
int		yLoc;
int		numberWidth;
int		btnWidth;
int		switchNumber;
int		boxNumber;
int		boxLeft;
int		nameLeft;
int		nameWidth;
int		descLeft;
int		descWidth;
int		valueLeft;
int		valueWidth;
int		stateLeft;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kSwitchBox_Title, kSwitchBox_Connected, yLoc, "AlpacaPi Switches");

	yLoc			+=	10;
	btnWidth		=	35;
	numberWidth		=	30;

	nameWidth		=	120;
	boxLeft			=	5;
	nameLeft		=	boxLeft + numberWidth + 2;

	descLeft		=	nameLeft + nameWidth + 2;
	descWidth		=	265;

	valueLeft		=	descLeft + descWidth  + 2;
	valueWidth		=	btnWidth * 2;
	stateLeft		=	valueLeft + valueWidth  + 2;


	switchNumber	=	1;
	boxNumber		=	kSwitchBox_Switch01;
	while (switchNumber <= kMaxSwitchControlBoxes)
	{
		//*	first the button number box
		SetWidget(				(boxNumber + 0),	boxLeft,		yLoc,		numberWidth,	cBtnHeight);
		SetWidgetNumber(		(boxNumber + 0),	switchNumber);
		SetWidgetFont(			(boxNumber + 0),	kFont_Medium);

		//*	now the name
		SetWidget(				(boxNumber + 1),	nameLeft,		yLoc,		nameWidth,		cBtnHeight);
		SetWidgetFont(			(boxNumber + 1),	kFont_Medium);
		SetWidgetText(			(boxNumber + 1),	"-----");

		//*	now the description
		SetWidget(				(boxNumber + 2),	descLeft,		yLoc,		descWidth,		cBtnHeight);
//		SetWidgetType(			(boxNumber + 3),	kWidgetType_MultiLineText);
		SetWidgetFont(			(boxNumber + 2),	kFont_Small);
		SetWidgetJustification(	(boxNumber + 2),	kJustification_Left);
		SetWidgetText(			(boxNumber + 2),	"-----------");

		//*	and the Value
		SetWidget(				(boxNumber + 3),	valueLeft,		yLoc,		valueWidth,		cBtnHeight);
		SetWidgetType(			(boxNumber + 3),	kWidgetType_TextBox);
		SetWidgetFont(			(boxNumber + 3),	kFont_Medium);
		SetWidgetText(			(boxNumber + 3),	"--");
//		SetWidgetBGColor(		(boxNumber + 3),	CV_RGB(255, 255, 255));


		//*	and the state
		SetWidget(				(boxNumber + 4),	stateLeft,		yLoc,		btnWidth,		cBtnHeight);
		SetWidgetType(			(boxNumber + 4),	kWidgetType_Button);
		SetWidgetFont(			(boxNumber + 4),	kFont_Medium);
		SetWidgetText(			(boxNumber + 4),	"OFF");
		SetWidgetBGColor(		(boxNumber + 4),	CV_RGB(255, 255, 255));



		boxNumber		+=	kBoxesPerSwitch;
		yLoc			+=	cBtnHeight;
		yLoc			+=	4;

		switchNumber++;
	}

	//*	now go through and disable ALL of the switches
	for (iii=kSwitchBox_Switch01; iii<=kSwitchBox_State16; iii++)
	{
		SetWidgetValid(iii, false);
	}

	yLoc			+=	4;

//	CONSOLE_DEBUG_W_NUM("boxLeft\t",	boxLeft);
//	CONSOLE_DEBUG_W_NUM("yLoc\t",		yLoc);
	SetWidget(			kSwitchBox_AllOff,	boxLeft,			yLoc,		descWidth,		cBtnHeight);
	SetWidgetType(		kSwitchBox_AllOff,	kWidgetType_Button);
	SetWidgetFont(		kSwitchBox_AllOff,	kFont_Medium);
	SetWidgetText(		kSwitchBox_AllOff,	"ALL OFF");
	SetWidgetBGColor(	kSwitchBox_AllOff,	CV_RGB(255, 255, 255));
	SetWidgetValid(		kSwitchBox_AllOff, true);

	cFirstRead	=	true;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kSwitchBox_IPaddr,
							kSwitchBox_Readall,
							kSwitchBox_DeviceState,
							kSwitchBox_AlpacaErrorMsg,
							kSwitchBox_LastCmdString,
							kSwitchBox_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabSwitch::ProcessButtonClick(const int buttonIdx, const int flags)
{

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	switch(buttonIdx)
	{
		case kSwitchBox_State01:	ToggleSwitchState(0);	break;
		case kSwitchBox_State02:	ToggleSwitchState(1);	break;
		case kSwitchBox_State03:	ToggleSwitchState(2);	break;
		case kSwitchBox_State04:	ToggleSwitchState(3);	break;
		case kSwitchBox_State05:	ToggleSwitchState(4);	break;
		case kSwitchBox_State06:	ToggleSwitchState(5);	break;
		case kSwitchBox_State07:	ToggleSwitchState(6);	break;
		case kSwitchBox_State08:	ToggleSwitchState(7);	break;

		case kSwitchBox_State09:	ToggleSwitchState(8);	break;
		case kSwitchBox_State10:	ToggleSwitchState(9);	break;
		case kSwitchBox_State11:	ToggleSwitchState(10);	break;
		case kSwitchBox_State12:	ToggleSwitchState(11);	break;
		case kSwitchBox_State13:	ToggleSwitchState(12);	break;
		case kSwitchBox_State14:	ToggleSwitchState(13);	break;
		case kSwitchBox_State15:	ToggleSwitchState(14);	break;
		case kSwitchBox_State16:	ToggleSwitchState(15);	break;

		case kSwitchBox_AllOff:		TurnAllSwitchesOff();	break;

	}
	DisplayLastAlpacaCommand();
}

//*****************************************************************************
void	WindowTabSwitch::ToggleSwitchState(const int switchNum)
{
ControllerSwitch	*myControllerSwitch;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cParentObjPtr != NULL)
	{
		myControllerSwitch		=	(ControllerSwitch *)cParentObjPtr;
		myControllerSwitch->ToggleSwitchState(switchNum);
	}
}

//*****************************************************************************
void	WindowTabSwitch::SetActiveSwitchCount(const int validSwitches)
{
int		iii;
int		jjj;
int		boxNum;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("validSwitches\t=", validSwitches);

	cValidSwitchCount	=	validSwitches;
	boxNum				=	kSwitchBox_Switch01;
	for (iii=0; iii<validSwitches; iii++)
	{
		for (jjj=0; jjj<kBoxesPerSwitch; jjj++)
		{
			SetWidgetValid(boxNum++, true);
		}
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	WindowTabSwitch::TurnAllSwitchesOff(void)
{
ControllerSwitch	*myControllerSwitch;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cParentObjPtr != NULL)
	{
		myControllerSwitch		=	(ControllerSwitch *)cParentObjPtr;
		myControllerSwitch->TurnAllSwitchesOff();
	}
}



#endif // _ENABLE_CTRL_SWITCHES_
