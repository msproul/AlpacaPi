//*****************************************************************************
//*		windowtab_rotator.cpp		(c) 2020 by Mark Sproul
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
//*	Oct  8,	2022	<MLS> Created windowtab_rotator.cpp
//*	Jun 19,	2023	<MLS> Added DeviceState to rotator
//*****************************************************************************

#ifdef _ENABLE_CTRL_ROTATOR_

#include	<stdio.h>



#include	"alpaca_defs.h"

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"widget.h"
#include	"windowtab.h"
#include	"windowtab_rotator.h"

#include	"controller_rotator.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabRotator::WindowTabRotator(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cFirstRead	=	true;

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabRotator::~WindowTabRotator(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabRotator::SetupWindowControls(void)
{
int		yLoc;
int		save_yLoc;
//int		numberWidth;
int		btnWidth;
int		dblBtnWidth;
int		compassLeft;
int		compassWidth;
int		iii;
int		buttonStepValues[]	=	{1, 5, 10, 45, 90, 0, 0, 0};
int		valueIdx;
char	valueText[32];

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kRotatorCtrl_Title, kRotatorCtrl_Connected, yLoc, "AlpacaPi Rotator");

	//------------------------------------------
	//*	set up the rotor compass display
	compassWidth	=	cClmWidth * 4;
	compassLeft		=   (cWidth - compassWidth) / 2;

	SetWidget(kRotatorCtrl_RotorCompass,	compassLeft,	yLoc,	compassWidth,	compassWidth);
	SetWidgetType(kRotatorCtrl_RotorCompass, kWidgetType_CustomGraphic);
	yLoc			+=	compassWidth;
	yLoc			+=	8;

	//*	keep this value for the 2nd column
	save_yLoc		=	yLoc;
	//----------------------------------------------
	//*	setup the rotation buttons
	btnWidth	=	cClmWidth - 15;
	dblBtnWidth	=	(cClmWidth * 2) - 15;
	SetWidget(		kRotatorCtrl_Zero,	cClm1_offset,	yLoc,	dblBtnWidth,	cTitleHeight);
	SetWidgetType(	kRotatorCtrl_Zero,	kWidgetType_Button);
	SetWidgetFont(	kRotatorCtrl_Zero,	kFont_Medium);
	SetWidgetText(	kRotatorCtrl_Zero,	"Zero");
	SetWidgetBGColor(kRotatorCtrl_Zero,	CV_RGB(0xff, 0xff, 0xff));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	valueIdx	=	0;
	iii			=	kRotatorCtrl_Rot_m1;
	while (iii < kRotatorCtrl_Halt)
	{
		sprintf(valueText, "-%d", buttonStepValues[valueIdx]);
		SetWidget(		iii,	cClm1_offset,	yLoc,	btnWidth,	cTitleHeight);
		SetWidgetType(	iii,	kWidgetType_Button);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetText(	iii,	valueText);
		SetWidgetBGColor(iii,	CV_RGB(0xff, 0xff, 0xff));
		iii++;

		sprintf(valueText, "+%d", buttonStepValues[valueIdx]);
		SetWidget(		iii,	cClm2_offset,	yLoc,	btnWidth,	cTitleHeight);
		SetWidgetType(	iii,	kWidgetType_Button);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetText(	iii,	valueText);
		SetWidgetBGColor(iii,	CV_RGB(0xff, 0xff, 0xff));

		yLoc			+=	cTitleHeight;
		yLoc			+=	2;
		iii++;
		valueIdx++;
	}
	SetWidget(		kRotatorCtrl_Halt,	cClm1_offset,	yLoc,	dblBtnWidth,	cTitleHeight);
	SetWidgetType(	kRotatorCtrl_Halt,	kWidgetType_Button);
	SetWidgetFont(	kRotatorCtrl_Halt,	kFont_Medium);
	SetWidgetText(	kRotatorCtrl_Halt,	"HALT");
	SetWidgetBGColor(kRotatorCtrl_Halt,	CV_RGB(0xff, 0xff, 0xff));

	SetWidgetOutlineBox(kRotatorCtrl_BtnOutline, kRotatorCtrl_Zero, (kRotatorCtrl_BtnOutline - 1));

	//----------------------------------------------
	//*	2nd column, the values
	yLoc	=	save_yLoc;
	iii		=	kRotatorCtrl_IsMoving_Lbl;
	while (iii < kRotatorCtrl_PropOutline)
	{

		SetWidget(		iii,	cClm3_offset,	yLoc,	cClmWidth * 2,	cTitleHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetJustification(iii, kJustification_Left);
		iii++;

		SetWidget(		iii,	cClm5_offset,	yLoc,	cClmWidth * 2,	cTitleHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetText(	iii,	"---");

		yLoc			+=	cTitleHeight;
		yLoc			+=	2;
		iii++;
	}

	SetWidgetText(kRotatorCtrl_IsMoving_Lbl,		"Is Moving");
	SetWidgetText(kRotatorCtrl_MechPos_Lbl,			"Mech Position");
	SetWidgetText(kRotatorCtrl_Position_Lbl,		"Position");
	SetWidgetText(kRotatorCtrl_StepSize_Lbl,		"Step Size");
	SetWidgetText(kRotatorCtrl_TargetPosition_Lbl,	"Target Position");

	SetWidgetOutlineBox(kRotatorCtrl_PropOutline, kRotatorCtrl_IsMoving_Lbl, (kRotatorCtrl_PropOutline - 1));
	yLoc			+=	4;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kRotatorCtrl_IPaddr,
							kRotatorCtrl_Readall,
							kRotatorCtrl_DeviceState,
							kRotatorCtrl_AlpacaErrorMsg,
							kRotatorCtrl_LastCmdString,
							kRotatorCtrl_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabRotator::ProcessButtonClick(const int buttonIdx, const int flags)
{
double				moveDelta_Deg;
ControllerRotator	*myRotatorController;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
	myRotatorController	=	(ControllerRotator*)cParentObjPtr;

	moveDelta_Deg	=	0.0;
	switch(buttonIdx)
	{
		case kRotatorCtrl_Zero:
			if (myRotatorController != NULL)
			{
				myRotatorController->SendMoveAbsoluteCommand(0.0);
			}
			break;

		case kRotatorCtrl_Rot_m1:	moveDelta_Deg	=	-1.0;	break;
		case kRotatorCtrl_Rot_p1:	moveDelta_Deg	=	1.0;	break;
		case kRotatorCtrl_Rot_m5:	moveDelta_Deg	=	-5.0;	break;
		case kRotatorCtrl_Rot_p5:	moveDelta_Deg	=	5.0;	break;
		case kRotatorCtrl_Rot_m10:	moveDelta_Deg	=	-10.0;	break;
		case kRotatorCtrl_Rot_p10:	moveDelta_Deg	=	10.0;	break;
		case kRotatorCtrl_Rot_m45:	moveDelta_Deg	=	-45.0;	break;
		case kRotatorCtrl_Rot_p45:	moveDelta_Deg	=	45.0;	break;
		case kRotatorCtrl_Rot_m90:	moveDelta_Deg	=	-90.0;	break;
		case kRotatorCtrl_Rot_p90:	moveDelta_Deg	=	90.0;	break;

		case kRotatorCtrl_Halt:
			if (myRotatorController != NULL)
			{
				myRotatorController->SendHaltCommand();
			}
			break;

		default:
			break;
	}
	if (fabs(moveDelta_Deg) > 0.0)
	{
		//*	we have a move to make
		CONSOLE_DEBUG_W_DBL("moveDelta_Deg\t",	moveDelta_Deg);
		if (myRotatorController != NULL)
		{
			myRotatorController->SendMoveRelativeCommand(moveDelta_Deg);
		}
	}
	DisplayLastAlpacaCommand();
}


//*****************************************************************************
void	WindowTabRotator::UpdateProperties_Rotator(TYPE_RotatorProperties *rotatorProp)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(kRotatorCtrl_IsMoving_Value,			(rotatorProp->IsMoving ? "Moving" : "Stopped"));
	SetWidgetNumber(kRotatorCtrl_MechPos_Value,			rotatorProp->MechanicalPosition);
	SetWidgetNumber(kRotatorCtrl_Position_Value,		rotatorProp->Position);
	SetWidgetNumber(kRotatorCtrl_StepSize_Value,		rotatorProp->StepSize, 8);
	SetWidgetNumber(kRotatorCtrl_TargetPosition_Value,	rotatorProp->TargetPosition);

	cRotatorPosition_deg	=	rotatorProp->Position;
	cTargetPosition_deg		=	rotatorProp->TargetPosition;
}

#define	_DISPLAY_CCW_
//**************************************************************************************
void	WindowTabRotator::DrawRotatorCompass(TYPE_WIDGET *theWidget)
{
int			radius1;
int			radius2;
int			radiusTxt;
double		degrees;
double		radians;
int			center_X;
int			center_Y;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
char		lineBuff[32];

	center_X	=	theWidget->left + (theWidget->width / 2);
	center_Y	=	theWidget->top + (theWidget->height / 2);
	radius1		=	theWidget->width / 5;

	LLG_SetColor(W_WHITE);
	LLG_FrameEllipse(center_X, center_Y, radius1, radius1);

	//*	now draw the tick marks every 10 degrees
	radius1		=	(theWidget->height / 2) - 8;
	radius2		=	(theWidget->height / 2) - 16;
	radiusTxt	=	(theWidget->height / 3) + 10;
	degrees	=	0;
	while (degrees < 360)
	{
		radians	=	degrees * M_PI / 180.0;
		pt1_X	=	center_X + (cos(radians) * radius1);
		pt1_Y	=	center_Y + (sin(radians) * radius1);

		pt2_X	=	center_X + (cos(radians) * radius2);
		pt2_Y	=	center_Y + (sin(radians) * radius2);

		LLG_MoveTo(pt1_X, pt1_Y);
		LLG_LineTo(pt2_X, pt2_Y);

		degrees	+=	10.0;
	}
	//------------------------------------------------
	//*	nos put some text labels
	degrees	=	0;
	while (degrees < 360)
	{
		//*	on the sky, it is COUNTER-CLOCKWISE!!!!
		radians	=	(degrees - 90.0) * M_PI / 180.0;
	#ifdef _DISPLAY_CCW_
		pt1_X	=	center_X - (cos(radians) * radiusTxt);
	#else
		pt1_X	=	center_X + (cos(radians) * radiusTxt);
	#endif
		pt1_X	-=	12;
		pt1_Y	=	center_Y + (sin(radians) * radiusTxt);

		sprintf(lineBuff, "%2.0f", degrees);
		LLG_DrawCString(pt1_X, pt1_Y, lineBuff, kFont_Medium);

		degrees	+=	45.0;
	}

	//*	now figure out where it is pointing
	degrees	=	cRotatorPosition_deg;
	sprintf(lineBuff, "%1.1f", degrees);
//	CONSOLE_DEBUG_W_DBL("degrees\t=", degrees);
//	CONSOLE_DEBUG(lineBuff);

	//*	draw the numeric degrees in the center
	pt1_X	=	center_X - 18;
	pt1_Y	=	center_Y + 5;
	if (abs(degrees) < 10.0)
	{
		pt1_X	+=	5;
	}
	LLG_DrawCString(pt1_X, pt1_Y, lineBuff, kFont_Medium);


	//*	determine dimensions for red/green indicators
	degrees	-=	90;
	radians	=	degrees * M_PI / 180.0;
	radius1	=	(theWidget->height / 5) + 3;
	radius2	=	(theWidget->height / 2) - 20;

#ifdef _DISPLAY_CCW_
	pt1_X	=	center_X - (cos(radians) * radius1);
	pt2_X	=	center_X - (cos(radians) * radius2);
#else
	pt1_X	=	center_X + (cos(radians) * radius1);
	pt2_X	=	center_X + (cos(radians) * radius2);
#endif
	pt1_Y	=	center_Y + (sin(radians) * radius1);
	pt2_Y	=	center_Y + (sin(radians) * radius2);

	LLG_PenSize(2);
	LLG_SetColor(W_RED);
	LLG_MoveTo(pt1_X, pt1_Y);
	LLG_LineTo(pt2_X, pt2_Y);

	//*	now draw a small green line where we WANT to be
	degrees	=	cTargetPosition_deg;
	degrees	-=	90;
	radians	=	degrees * M_PI / 180.0;
	radius1	=	(theWidget->height / 2) - 20;
	radius2	=	(theWidget->height / 2);

#ifdef _DISPLAY_CCW_
	pt1_X	=	center_X - (cos(radians) * radius1);
	pt2_X	=	center_X - (cos(radians) * radius2);
#else
	pt1_X	=	center_X + (cos(radians) * radius1);
	pt2_X	=	center_X + (cos(radians) * radius2);
#endif

	pt1_Y	=	center_Y + (sin(radians) * radius1);
	pt2_Y	=	center_Y + (sin(radians) * radius2);
	LLG_SetColor(W_GREEN);
	LLG_MoveTo(pt1_X, pt1_Y);
	LLG_LineTo(pt2_X, pt2_Y);

	//*	reset back to normal
	LLG_PenSize(1);
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTabRotator::DrawWidgetCustomGraphic(	cv::Mat		*openCV_Image,
														const int	widgetIdx)
#else
//**************************************************************************************
void	WindowTabRotator::DrawWidgetCustomGraphic(	IplImage	*openCV_Image,
														const int	widgetIdx)
#endif // _USE_OPENCV_CPP_
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cOpenCV_Image	=	openCV_Image;

	switch(widgetIdx)
	{
		case kRotatorCtrl_RotorCompass:
			DrawRotatorCompass(&cWidgetList[widgetIdx]);
			break;
	}
}

#endif // _ENABLE_CTRL_ROTATOR_
