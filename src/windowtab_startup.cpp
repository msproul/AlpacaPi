//*****************************************************************************
//*		windowtab_startup.cpp		(c) 2023 by Mark Sproul
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
//*	May 21,	2023	<MLS> Created windowtab_startup.cpp
//*	May 23,	2023	<MLS> Added splash info
//*****************************************************************************

#include	<stdlib.h>
#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_startup.h"
#include	"controller.h"
#include	"helper_functions.h"

#define	kCapabilitiesHeight	100

//**************************************************************************************
WindowTabStartup::WindowTabStartup(	const int	xSize,
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
WindowTabStartup::~WindowTabStartup(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}


//**************************************************************************************
void	WindowTabStartup::SetupWindowControls(void)
{
int			yLoc;
int			iii;
short		tabStopList[]	=	{300, 350, 400, 0 };
int			boxHeight;
cv::Scalar	spashColor;
//time_t		secsSinceEpoch;
//
//	secsSinceEpoch	=	GetSecondsSinceEpoch();
//	srand(secsSinceEpoch);
//	spashColor		=	CV_RGB(	(0 + (rand() & 0x00ff)),
//								(0 + (rand() & 0x00ff)),
//								(0 + (rand() & 0x00ff)));
	spashColor		=	CV_RGB(0,	255,	0);

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;
	yLoc			+=	4;


	for (iii=kStartup_Title1; iii<=kStartup_Title2; iii++)
	{
		switch(iii)
		{
			case kStartup_Title1:	boxHeight	=	95;	break;
			case kStartup_Title2:	boxHeight	=	30;	break;
		}
		SetWidget(				iii,	5,	yLoc,	(cWidth - 10),	boxHeight);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetBorder(		iii,	false);
		SetWidgetJustification(	iii,	kJustification_Center);
		SetWidgetFont(			iii,	kFont_Large);
		SetWidgetTextColor(		iii,	spashColor);
		yLoc			+=	boxHeight;
//		yLoc			+=	2;
	}
	SetWidgetFont(kStartup_Title1,	kFont_Script_Large);
	SetWidgetText(kStartup_Title1, "SkyTravel");

	SetWidgetFont(kStartup_Title2,	kFont_Triplex_Small);
	SetWidgetText(kStartup_Title2, "(C) Mark Sproul msproul@skychariot.com");

	SetWidgetOutlineBox(kStartup_TitleOutline, kStartup_Title1, (kStartup_TitleOutline -1));
	SetWidgetBorderColor(kStartup_TitleOutline, spashColor);
	yLoc			+=	10;
	//------------------------------------------

	for (iii=kStartup_TextBox1; iii<=kStartup_TextBoxN; iii++)
	{
		SetWidget(				iii,	0,			yLoc,		cWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetTabStops(		iii,	tabStopList);
		SetWidgetBorder(		iii,	false);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));

		yLoc			+=	cRadioBtnHt;
//		yLoc			+=	2;
	}

//	SetAlpacaLogoBottomCorner(kStartup_AlpacaLogo);
}




