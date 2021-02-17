//*****************************************************************************
//*		windowtab_moon.cpp		(c) 2021 by Mark Sproul
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
//*	Feb  4,	2021	<MLS> Created windowtab_moon.cpp
//*	Feb 14,	2021	<MLS> Added moon image, will add phases later
//*****************************************************************************



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_moon.h"
#include	"controller.h"

#include	"moonphase.h"


//**************************************************************************************
WindowTabMoon::WindowTabMoon(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cLastUpdateTime_ms	=	0;
	cFirstRead			=	true;

	SetupWindowControls();

	RunBackgroundTasks();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMoon::~WindowTabMoon(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabMoon::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		iii;
int		saveYloc;
int		moonXloc;
int		availablePixels;
int		moonImageSize;
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kMoon_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kMoon_Title, "Phase of the Moon");
	SetBGcolorFromWindowName(kMoon_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	saveYloc		=	yLoc;

	yLoc			+=	12;

	iii	=	kMoon_AgeLbl;
	while (iii < kMoon_AlpacaLogo)
	{
		SetWidget(				iii,	cClm1_offset,			yLoc,		cBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		SetWidget(				iii,	cClm2_offset,			yLoc,		cBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		yLoc			+=	cBtnHeight;
		yLoc			+=	2;
	}

	SetWidgetText(kMoon_AgeLbl,				"Moon age (days)");
	SetWidgetText(kMoon_PhaseLbl,			"Moon Phase");
	SetWidgetText(kMoon_IlluminationLbl,	"Illumination (%)");

	availablePixels	=	cWidth - cClm3_offset;
	moonImageSize	=   675;
	moonXloc		=	cClm3_offset + ((availablePixels - moonImageSize) / 2);
	saveYloc		+=	5;

	SetWidget(				kMoon_Image,	moonXloc,		saveYloc,	moonImageSize,		moonImageSize);
	SetWidgetType(			kMoon_Image,	kWidgetType_Text);
	SetWidgetText(			kMoon_Image,	"Moon image goes here");
	SetWidgetBorder(		kMoon_Image,	false);
//	SetWidgetBorderColor(	kMoon_Image,	CV_RGB(0,	0,	255));
	SetWidgetOutlineBox(	kMoon_MoonImgOutline, kMoon_Image, kMoon_Image);

	cMoonImage	=	cvLoadImage("moon_fits/moon.jpg", CV_LOAD_IMAGE_COLOR);
	if (cMoonImage != NULL)
	{
		SetWidgetImage(kMoon_Image, cMoonImage);
	}


	SetAlpacaLogoBottomCorner(kMoon_AlpacaLogo);

}

//**************************************************************************************
void WindowTabMoon::RunBackgroundTasks(void)
{
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;
double		moonAge;
double		moonIllumination;
char		moonPhaseStr[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdateTime_ms;
//	if (cFirstRead || (deltaMilliSecs > 60000))
	if (cFirstRead || (deltaMilliSecs > 5000))
	{
		GetCurrentMoonPhase(moonPhaseStr);
		moonAge				=	CalcDaysSinceNewMoon(0, 0, 0);	//*	zero -> current time
		moonIllumination	=	CalcMoonIllumination(0, 0, 0);	//*	zero -> current time

		SetWidgetNumber(kMoon_Age,			moonAge);
		SetWidgetText(	kMoon_Phase,		moonPhaseStr);
		SetWidgetNumber(kMoon_Illumination,	moonIllumination);


		cLastUpdateTime_ms	=	currentMilliSecs;

		ForceUpdate();


		cFirstRead	=	false;
	}
}





