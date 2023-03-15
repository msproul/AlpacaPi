//*****************************************************************************
//*		windowtab_auxmotor.cpp		(c) 2020 by Mark Sproul
//*				Controller base class
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
//*	Feb 24,	2020	<MLS> Created windowtab_auxmotor.cpp
//*	Feb 25,	2020	<MLS> Buttons working in aux motor window
//*	Mar 17,	2020	<MLS> Added IP box to bottom of AuxMotor tab
//*****************************************************************************

#ifdef _ENABLE_CTRL_FOCUSERS_


#include	"windowtab_auxmotor.h"
#include	"controller.h"
#include	"controller_focus.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



//**************************************************************************************
WindowTabAuxMotor::WindowTabAuxMotor(	const int	xSize,
										const int	ySize,
										cv::Scalar backGrndColor)
	:WindowTab(xSize, ySize, backGrndColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabAuxMotor::~WindowTabAuxMotor(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabAuxMotor::SetupWindowControls(void)
{
int		yLoc;
char	lineBuff[32];
int		buttonOffsets[5]	=	{1, 10, 100, 1000, 5000};
int		btnIdx;
int		iii;


	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kAuxMotorBox_Title, -1, yLoc, "Auxiliary stepper");

	//------------------------------------------
	SetWidget(kAuxMotorBox_Position,	0,		yLoc,		cWidth / 2,		cBoxHeight);
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	SetWidget(		kAuxMotorBox_Label,	0,		yLoc,		cWidth / 3,		cBoxHeight);
	SetWidgetFont(	kAuxMotorBox_Label, kFont_Medium);
	SetWidgetText(	kAuxMotorBox_Label, "AUX MOTOR");
	SetWidgetTextColor(		kAuxMotorBox_Label,		CV_RGB(255, 255, 255));

	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	SetWidget(kAuxMotorBox_DesiredPos,	0,		yLoc,		cWidth / 2,		cBoxHeight);
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	//*	this will use the buttons in the order they are defined to set up the widgets
	//*	the above order MUST match
	btnIdx	=	kAuxMotorBox_m1;
	for (iii=0; iii<5; iii++)
	{
		sprintf(lineBuff, "%d", -buttonOffsets[iii]);
		SetWidget(		btnIdx,	cClm1_offset,	yLoc,			cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		SetWidgetText(	btnIdx, lineBuff);
		btnIdx++;

		sprintf(lineBuff, "+%d", buttonOffsets[iii]);
		SetWidget(		btnIdx,	cClm2_offset,	yLoc,			cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		SetWidgetText(	btnIdx, lineBuff);
		btnIdx++;

		yLoc	+=	cBtnHeight;
		yLoc	+=	2;
	}

	for (iii=kAuxMotorBox_m1; iii<=kAuxMotorBox_p5000; iii++)
	{
		SetWidgetBGColor(		iii,		CV_RGB(255, 255, 255));
		SetWidgetBorderColor(	iii,		CV_RGB(0, 0, 0));
		SetWidgetTextColor(		iii,		CV_RGB(0, 0, 0));
	}

	yLoc	+=	5;
	SetWidget(kAuxMotorBox_Switch1,	0,					yLoc,		cWidth / 4,		20);
	SetWidget(kAuxMotorBox_Switch2,	(cWidth / 4) + 2,	yLoc,		cWidth / 4,		20);
	SetWidgetText(			kAuxMotorBox_Switch1, "SW1");
	SetWidgetText(			kAuxMotorBox_Switch2, "SW2");
	SetWidgetFont(			kAuxMotorBox_Switch1, kFont_Medium);
	SetWidgetFont(			kAuxMotorBox_Switch2, kFont_Medium);
	SetWidgetTextColor(		kAuxMotorBox_Switch1,	CV_RGB(255, 255, 255));
	SetWidgetTextColor(		kAuxMotorBox_Switch2,	CV_RGB(255, 255, 255));

	yLoc			+=	20;
	yLoc			+=	2;

	SetWidget(				kAuxMotorBox_SwitchLbl,	0,		yLoc,		cWidth / 2,		20);
	SetWidgetFont(			kAuxMotorBox_SwitchLbl,	kFont_Medium);
	SetWidgetText(			kAuxMotorBox_SwitchLbl,	"SWITCH");
	SetWidgetTextColor(		kAuxMotorBox_SwitchLbl,	CV_RGB(255, 0, 0));
	SetWidgetBorderColor(	kAuxMotorBox_SwitchLbl,	CV_RGB(0, 0, 0));

	SetWidgetText(	kAuxMotorBox_Position,		"-----");
	SetWidgetText(	kAuxMotorBox_DesiredPos,	"-----");

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kAuxMotorBox_IPaddr,
							kAuxMotorBox_Readall,
							kAuxMotorBox_AlpacaErrorMsg,
							kAuxMotorBox_LastCmdString,
							kAuxMotorBox_AlpacaLogo,
							-1);

}

//*****************************************************************************
void	WindowTabAuxMotor::ProcessButtonClick(const int buttonIdx, const int flags)
{
int				previousAuxMotorDesiredPosition;
ControllerFocus	*focusController;
int				myAuxMotorDesiredPosition;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	if (cParentObjPtr != NULL)
	{
		focusController					=	(ControllerFocus*)cParentObjPtr;
		previousAuxMotorDesiredPosition	=	focusController->cAuxMotorDesiredPos;

		myAuxMotorDesiredPosition	=	previousAuxMotorDesiredPosition;
		CONSOLE_DEBUG_W_NUM("myAuxMotorDesiredPosition\t",	myAuxMotorDesiredPosition);
		switch(buttonIdx)
		{
			case kAuxMotorBox_m1:		myAuxMotorDesiredPosition	-=	1;		break;
			case kAuxMotorBox_p1:		myAuxMotorDesiredPosition	+=	1;		break;

			case kAuxMotorBox_m10:		myAuxMotorDesiredPosition	-=	10;		break;
			case kAuxMotorBox_p10:		myAuxMotorDesiredPosition	+=	10;		break;

			case kAuxMotorBox_m100:		myAuxMotorDesiredPosition	-=	100;	break;
			case kAuxMotorBox_p100:		myAuxMotorDesiredPosition	+=	100;	break;

			case kAuxMotorBox_m1000:	myAuxMotorDesiredPosition	-=	1000;	break;
			case kAuxMotorBox_p1000:	myAuxMotorDesiredPosition	+=	1000;	break;

			case kAuxMotorBox_m5000:	myAuxMotorDesiredPosition	-=	5000;	break;
			case kAuxMotorBox_p5000:	myAuxMotorDesiredPosition	+=	5000;	break;

		}
		CONSOLE_DEBUG_W_NUM("myAuxMotorDesiredPosition\t",	myAuxMotorDesiredPosition);

		if (myAuxMotorDesiredPosition < 0)
		{
			myAuxMotorDesiredPosition	=	0;
		}

		//*	do we need to update the focuser position
		if (myAuxMotorDesiredPosition != previousAuxMotorDesiredPosition)
		{
			focusController->SendMoveAuxMotorCommand(myAuxMotorDesiredPosition);
		}
	}
	else
	{
		CONSOLE_DEBUG("No parent");
	}
}

//**************************************************************************************
void	WindowTabAuxMotor::DisableAuxMotorButons(void)
{
int		iii;

	for (iii=kAuxMotorBox_m1; iii<=kAuxMotorBox_p5000; iii++)
	{
		SetWidgetBGColor(		iii,		CV_RGB(128, 128, 128));
//		SetWidgetBorderColor(	iii,		CV_RGB(0, 0, 0));
//		SetWidgetTextColor(		iii,		CV_RGB(0, 0, 0));
	}

}


#endif	//	_ENABLE_CTRL_FOCUSERS_
