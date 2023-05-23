//*****************************************************************************
//*		windowtab_time.cpp		(c) 2023 by Mark Sproul
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
//*	Mar 17,	2023	<MLS> Created windowtab_time.cpp
//*	Mar 17,	2023	<MLS> Added UpdateTime()
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"observatory_settings.h"
#include	"sidereal.h"
#include	"SkyStruc.h"

#include	"controller.h"
#include	"windowtab.h"
#include	"windowtab_time.h"

//**************************************************************************************
WindowTabTime::WindowTabTime(	const int	xSize,
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
WindowTabTime::~WindowTabTime(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
const char	gTimeDocumentaiton[]	=
{
		"SkyTravel defaults to live time based on the system clock. "
		"However, it can work in non-real time as well, forward or backwards 1000's of years."
};


//**************************************************************************************
void	WindowTabTime::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		yLoc_Save;
int		textBoxHt;
int		textBoxWidth;
int		iii;
int		timeBoxWidth;
int		timeLabelWidth;
int		timeValueWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;
	yLoc			=	SetTitleBox(kTimeTab_Title, -1, yLoc, "SkyTravel time");
	xLoc			=	5;
	textBoxHt		=	cTitleHeight * 4;
	textBoxWidth	=	cWidth - 10;
//	CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);
//	CONSOLE_DEBUG_W_NUM("textBoxHt   \t=",	textBoxHt);
//	CONSOLE_DEBUG_W_NUM("textBoxWidth\t=",	textBoxWidth);

	SetWidget(				kTimeTab_Documentation,		xLoc,	yLoc,	textBoxWidth,	textBoxHt);
	SetWidgetType(			kTimeTab_Documentation,		kWidgetType_MultiLineText);
	SetWidgetJustification(	kTimeTab_Documentation,		kJustification_Left);
	SetWidgetFont(			kTimeTab_Documentation,		kFont_Medium);
	SetWidgetTextColor(		kTimeTab_Documentation,		CV_RGB(255,	255,	255));
	SetWidgetText(			kTimeTab_Documentation,		gTimeDocumentaiton);
	yLoc			+=	textBoxHt;
	yLoc			+=	2;
	yLoc_Save		=	yLoc;

	timeBoxWidth	=	cClmWidth + 100;
	timeLabelWidth	=	timeBoxWidth / 3;
	timeValueWidth	=	timeLabelWidth * 2;

	SetWidget(		kTimeTab_LinuxTimeTitle,	xLoc,	yLoc,	timeBoxWidth,	cTitleHeight);
	SetWidgetFont(	kTimeTab_LinuxTimeTitle,	kFont_Medium);
	SetWidgetText(	kTimeTab_LinuxTimeTitle,	"Linux Time");
	SetWidgetTextColor(kTimeTab_LinuxTimeTitle,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	textBoxHt	=	60;
	iii			=	kTimeTab_LinuxTimeUTC_lbl;

	while  (iii < kTimeTab_LinuxTimeOutline)
	{
		//*	label field
		SetWidget(				iii,	xLoc,			yLoc,		timeLabelWidth,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		//*	value field
		SetWidget(				iii,	xLoc + timeLabelWidth + 2,	yLoc,	timeValueWidth,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetJustification(	iii,	kJustification_Center);
		SetWidgetFont(			iii,	kFont_Large);
		SetWidgetTextColor(		iii,	CV_RGB(0,	255,	0));
		iii++;

		yLoc			+=	textBoxHt;
		yLoc			+=	2;
//		CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);

	}
	SetWidgetOutlineBox(kTimeTab_LinuxTimeOutline, kTimeTab_LinuxTimeTitle, (kTimeTab_LinuxTimeOutline - 1));

	SetWidgetText(	kTimeTab_LinuxTimeUTC_lbl,		"UTC");
	SetWidgetText(	kTimeTab_LinuxTimeLocal_lbl,	"Local");
	SetWidgetText(	kTimeTab_LinuxTimeSidereal_lbl,	"Sidereal");

	//--------------------------------------------------
	yLoc			=	yLoc_Save;
	xLoc			=	cWidth / 2;
	textBoxHt		=	cTitleHeight * 4;
	textBoxWidth	=	cWidth - 10;
//	CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);
//	CONSOLE_DEBUG_W_NUM("textBoxHt   \t=",	textBoxHt);
//	CONSOLE_DEBUG_W_NUM("textBoxWidth\t=",	textBoxWidth);

//	CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);

	timeBoxWidth	=	cClmWidth + 100;
	timeLabelWidth	=	timeBoxWidth / 3;
	timeValueWidth	=	timeLabelWidth * 2;

	SetWidget(		kTimeTab_SkyTravelTitle,	xLoc,	yLoc,	timeBoxWidth,	cTitleHeight);
	SetWidgetFont(	kTimeTab_SkyTravelTitle,	kFont_Medium);
	SetWidgetText(	kTimeTab_SkyTravelTitle,	"SkyTravel Time");
	SetWidgetTextColor(kTimeTab_SkyTravelTitle,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
//	CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);

	textBoxHt	=	60;
	iii			=	kTimeTab_SkyTravelUTC_lbl;

	while  (iii < kTimeTab_SkyTravelOutline)
	{
		//*	label field
		SetWidget(				iii,	xLoc,			yLoc,		timeLabelWidth,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		//*	value field
		SetWidget(				iii,	xLoc + timeLabelWidth + 2,	yLoc,	timeValueWidth,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetJustification(	iii,	kJustification_Center);
		SetWidgetFont(			iii,	kFont_Large);
		SetWidgetTextColor(		iii,	CV_RGB(0,	255,	0));
		iii++;

		yLoc			+=	textBoxHt;
		yLoc			+=	2;
//		CONSOLE_DEBUG_W_NUM("yLoc        \t=",	yLoc);

	}
	SetWidgetOutlineBox(kTimeTab_SkyTravelOutline, kTimeTab_SkyTravelTitle, (kTimeTab_SkyTravelOutline - 1));

	SetWidgetText(	kTimeTab_SkyTravelUTC_lbl,				"UTC");
	SetWidgetText(	kTimeTab_SkyTravelLocal_lbl,			"Local");
	SetWidgetText(	kTimeTab_SkyTravelSidereal_lbl,			"Sidereal");
	SetWidgetText(	kTimeTab_SkyTravelTimeZone_lbl,			"Timezone");
	SetWidgetText(	kTimeTab_SkyTravelDayLightSavTm_lbl,	"DST");


	SetAlpacaLogoBottomCorner(kTimeTab_AlpacaLogo);

	UpdateTime();
}

//**************************************************************************************
void	WindowTabTime::RunWindowBackgroundTasks(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	UpdateTime();

}

//**************************************************************************************
void	WindowTabTime::UpdateTime(void)
{
struct timeval		currentTimeVal;
char				utcTimeString[32];
char				siderealTimeString[32];
char				localTImeString[128];
struct tm			utcTime;
struct tm			siderealTime;
time_t				currentTime;
struct tm			*linuxTime;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	deal with UTC time
	gettimeofday(&currentTimeVal, NULL);
	FormatTimeString(&currentTimeVal, utcTimeString);
	SetWidgetText(	kTimeTab_LinuxTimeUTC,	utcTimeString);

	//*	local time
	currentTime		=	time(NULL);
	if (currentTime != -1)
	{
		linuxTime		=	localtime(&currentTime);
		sprintf(localTImeString,	"%02d:%02d:%02d",
									linuxTime->tm_hour,
									linuxTime->tm_min,
									linuxTime->tm_sec);
		SetWidgetText(	kTimeTab_LinuxTimeLocal,	localTImeString);
	}

	//*	sidereal time
	gmtime_r(&currentTimeVal.tv_sec, &utcTime);
	CalcSiderealTime(&utcTime, &siderealTime, gObseratorySettings.Longitude_deg);
	FormatTimeString_TM(&siderealTime, siderealTimeString);

	SetWidgetText(	kTimeTab_LinuxTimeSidereal,	siderealTimeString);

	//-------------------------------------------------------------
	sprintf(utcTimeString,		"%02d:%02d:%02d",	gCurrentSkyTime.hour, gCurrentSkyTime.min, gCurrentSkyTime.sec);
	sprintf(localTImeString,	"%02d:%02d:%02d",	gCurrentSkyTime.local_hour, 0, 0);

	SetWidgetText(		kTimeTab_SkyTravelUTC,		utcTimeString);
	SetWidgetText(		kTimeTab_SkyTravelLocal,	localTImeString);

	SetWidgetNumber(	kTimeTab_SkyTravelSidereal, gCurrentSkyTime.fSiderealTime);
	SetWidgetNumber(	kTimeTab_SkyTravelTimeZone, gCurrentSkyTime.timezone);

	SetWidgetText(	kTimeTab_SkyTravelDayLightSavTm, (gCurrentSkyTime.dstflag ? "enabled" : "disabled"));
}

