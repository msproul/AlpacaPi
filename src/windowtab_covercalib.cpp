//*****************************************************************************
//*		windowtab_covercalib.cpp		(c) 2021 by Mark Sproul
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
//*	Feb 12,	2021	<MLS> Created windowtab_covercalib.cpp
//*	Feb 13,	2021	<MLS> Added SetBrightness()
//*****************************************************************************

#define _ENABLE_COVER_CALIBRATION_

#ifdef _ENABLE_COVER_CALIBRATION_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_covercalib.h"
#include	"controller.h"
#include	"helper_functions.h"


#include	"controller_covercalib.h"

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabCoverCalib::WindowTabCoverCalib(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cLastBrightnewssUpdate_Millis	=	0;

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabCoverCalib::~WindowTabCoverCalib(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabCoverCalib::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		boxID;
int		buttonWidth;
int		myPlusMinusBtnWidth;
int		myButtonHt;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//=======================================================
	SetWidget(kCoverCalib_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kCoverCalib_Title, "Cover Calibration");
	SetBGcolorFromWindowName(kCoverCalib_Title);
	//=======================================================
	//*	setup the connected indicator
   	SetUpConnectedIndicator(kCoverCalib_Connected, yLoc);

	yLoc			+=	cTitleHeight;
	yLoc			+=	20;


	//=======================================================
	xLoc		=	cClm2_offset;
	buttonWidth	=	cClmWidth + (cClmWidth / 2);

	SetWidget(			kCoverCalib_Cover_Label,	xLoc,	yLoc,	(buttonWidth * 3),	cTitleHeight);
	SetWidgetText(		kCoverCalib_Cover_Label,	"Cover");
	SetWidgetBorder(	kCoverCalib_Cover_Label,	false);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetWidget(			kCoverCalib_Cover_State,	xLoc,	yLoc,	(buttonWidth * 3),	cTitleHeight);
	SetWidgetText(		kCoverCalib_Cover_State,	"-cover state-");
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	boxID		=	kCoverCalib_Cover_Open;
	while (boxID <= kCoverCalib_Cover_Halt)
	{
		SetWidget(			boxID,	xLoc,		yLoc,		(buttonWidth - 5),		cTitleHeight);
		SetWidgetType(		boxID,	kWidgetType_Button);
		SetWidgetBGColor(	boxID,	CV_RGB(255, 255, 255));


		xLoc	+=	buttonWidth;
		xLoc	+=	5;

		boxID++;
	}
	SetWidgetText(kCoverCalib_Cover_Open,	"Open");
	SetWidgetText(kCoverCalib_Cover_Close,	"Close");
	SetWidgetText(kCoverCalib_Cover_Halt,	"Halt");

	SetWidgetOutlineBox(kCoverCalib_Cover_Outline, kCoverCalib_Cover_Label, kCoverCalib_Cover_Halt);

	yLoc			+=	cTitleHeight;
	yLoc			+=	4;


	//=======================================================
	yLoc			+=	50;

	SetWidget(		kCoverCalib_Brightness_Label,	cClm1_offset,	yLoc,	(cClmWidth * 3),		cTitleHeight);
	SetWidgetFont(	kCoverCalib_Brightness_Label,	kFont_Medium);
	SetWidgetText(	kCoverCalib_Brightness_Label,	"Calibration Brightness");

	SetWidget(		kCoverCalib_Brightness_Value,	cClm4_offset,	yLoc,	cClmWidth,		cTitleHeight);
	SetWidgetFont(	kCoverCalib_Brightness_Value,	kFont_Medium);
	SetWidgetText(	kCoverCalib_Brightness_Value,	"0");

	SetWidget(			kCoverCalib_Brightness_Off,	cClm5_offset,	yLoc,	(cClmWidth * 2),		cTitleHeight);
	SetWidgetType(		kCoverCalib_Brightness_Off,	kWidgetType_Button);
	SetWidgetText(		kCoverCalib_Brightness_Off,	"Turn Off");
	SetWidgetBGColor(	kCoverCalib_Brightness_Off,	CV_RGB(255, 255, 255));


	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//=======================================================
	SetWidget(				kCoverCalib_Brightness_Slider,	cClm1_offset,		yLoc,	(cClmWidth * 6),cTitleHeight);
	SetWidgetType(			kCoverCalib_Brightness_Slider,	kWidgetType_Slider);
	SetWidgetFont(			kCoverCalib_Brightness_Slider,	kFont_Medium);
	SetWidgetSliderLimits(	kCoverCalib_Brightness_Slider,	0.0, 2.0);
	SetWidgetSliderValue(	kCoverCalib_Brightness_Slider,	0.0);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//**************************************************************
	myPlusMinusBtnWidth	=	((cWidth - cClm1_offset) / 8) - 1;
	myButtonHt			=	cBtnHeight - 3;
	xLoc				=	cClm1_offset;
	for (boxID=kCoverCalib_Minus50; boxID<=kCoverCalib_Plus50; boxID++)
	{
		SetWidget(				boxID,	xLoc,		yLoc,	myPlusMinusBtnWidth,	myButtonHt);
		SetWidgetType(			boxID,	kWidgetType_Button);
		SetWidgetFont(			boxID,	kFont_Medium);
		SetWidgetBGColor(		boxID,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		boxID,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	boxID,	CV_RGB(0,	0,	0));
		xLoc	+=	myPlusMinusBtnWidth;
	}
	SetWidgetText(		kCoverCalib_Minus50, 	"-50");
	SetWidgetText(		kCoverCalib_Minus10, 	"-10");
	SetWidgetText(		kCoverCalib_Minus5, 	"-5");
	SetWidgetText(		kCoverCalib_Minus1, 	"-1");
	SetWidgetText(		kCoverCalib_Plus1, 		"+1");
	SetWidgetText(		kCoverCalib_Plus5, 		"+5");
	SetWidgetText(		kCoverCalib_Plus10, 	"+10");
	SetWidgetText(		kCoverCalib_Plus50, 	"+50");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;


	SetWidgetOutlineBox(kCoverCalib_Brightness_Outline, kCoverCalib_Brightness_Label, kCoverCalib_Plus50);


	yLoc			+=	cTitleHeight;
	yLoc			+=	50;


	SetAlpacaLogo(kCoverCalib_AlpacaLogo, kCoverCalib_LastCmdString);

	//=======================================================
	//*	IP address
	CONSOLE_DEBUG(__FUNCTION__);
	SetIPaddressBoxes(kCoverCalib_IPaddr, kCoverCalib_Readall, kCoverCalib_AlpacaDrvrVersion, -1);

}

//*****************************************************************************
void	WindowTabCoverCalib::SetBrightness(const int newBrightnessValue)
{
bool	validData;
char	dataString[64];

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, newBrightnessValue);

	sprintf(dataString, "Brightness=%d", newBrightnessValue);
	validData	=	AlpacaSendPutCmd(	"covercalibrator", "calibratoron",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG("Command failure - calibratoron")
	}
}

//*****************************************************************************
void	WindowTabCoverCalib::AdjustBrightness(const int howMuch)
{
int		newBrightnessValue;

	newBrightnessValue	=	cCoverCalibrationPropPtr->Brightness;

	newBrightnessValue	+=	howMuch;
	if (newBrightnessValue < 0.0)
	{
		newBrightnessValue	=	0;
	}
	if (newBrightnessValue > cCoverCalibrationPropPtr->MaxBrightness)
	{
		newBrightnessValue	=	cCoverCalibrationPropPtr->MaxBrightness;
	}
	SetBrightness(newBrightnessValue);
}

//*****************************************************************************
void	WindowTabCoverCalib::ProcessButtonClick(const int buttonIdx)
{
bool	validData;

	switch(buttonIdx)
	{
		case kCoverCalib_Brightness_Slider:
			break;

		case kCoverCalib_Brightness_Off:
			validData	=	AlpacaSendPutCmd(	"covercalibrator", "calibratoroff",	"");
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to turn off calibration unit");
			}
			break;

		case kCoverCalib_Cover_Open:
			validData	=	AlpacaSendPutCmd(	"covercalibrator", "opencover",	"");
			break;

		case kCoverCalib_Cover_Close:
			validData	=	AlpacaSendPutCmd(	"covercalibrator", "closecover",	"");
			break;

		case kCoverCalib_Cover_Halt:
			validData	=	AlpacaSendPutCmd(	"covercalibrator", "haltcover",	"");
			break;


		case kCoverCalib_Minus50:	AdjustBrightness(-50.0);	break;
		case kCoverCalib_Minus10:	AdjustBrightness(-10.0);	break;
		case kCoverCalib_Minus5:	AdjustBrightness(-5.0);		break;
		case kCoverCalib_Minus1:	AdjustBrightness(-1.0);		break;
		case kCoverCalib_Plus1:		AdjustBrightness(1.0);		break;
		case kCoverCalib_Plus5:		AdjustBrightness(5.0);		break;
		case kCoverCalib_Plus10:	AdjustBrightness(10.0);		break;
		case kCoverCalib_Plus50:	AdjustBrightness(50.0);		break;

		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;

	}
	DisplayLastAlpacaCommand();
	ForceUpdate();
}

//*****************************************************************************
void	WindowTabCoverCalib::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
uint32_t	currentMillis;
uint32_t	deltaMilliSecs;
int			newBrightnessValue;
int			deltaBrightness;

	currentMillis	=	millis();
	deltaMilliSecs	=	currentMillis - cLastBrightnewssUpdate_Millis;

	newBrightnessValue	=	newSliderValue;

	deltaBrightness	=	abs(newBrightnessValue - cCoverCalibrationPropPtr->Brightness);
	if ((deltaBrightness > 2) && (deltaMilliSecs > 4))
	{
		SetWidgetSliderValue(widgetIdx, newSliderValue);
		SetBrightness(newBrightnessValue);
	}
	else
	{
	//	CONSOLE_DEBUG_W_NUM("deltaBrightness\t=", deltaBrightness);
	//	CONSOLE_DEBUG_W_NUM("deltaMilliSecs\t=", deltaMilliSecs);
	}
	ForceUpdate();
	cLastBrightnewssUpdate_Millis	=	millis();
}

//*****************************************************************************
void	WindowTabCoverCalib::ProcessMouseLeftButtonUp(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kCoverCalib_Brightness_Slider:
			CONSOLE_DEBUG("kCoverCalib_Brightness_Slider");
			ForceUpdate();
			break;
	}
}

//******************************************************************************
void	WindowTabCoverCalib::SetCoverCalibPropPtr(TYPE_CoverCalibrationProperties *argPropPtr)
{
	cCoverCalibrationPropPtr		=	argPropPtr;
}


#endif // _ENABLE_USB_FOCUSERS_
