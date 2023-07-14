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
int			xLoc;
int			yLoc;
int			iii;
short		tabStopList[]	=	{300, 350, 400, 0 };
int			boxHeight;
int			buttonWidth;
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
	SetWidgetFont(		kStartup_Title1,	kFont_Script_Large);
//	SetWidgetTextColor(	kStartup_Title1,	CV_RGB(0x91,	0x30,	0xFA));
	SetWidgetTextColor(	kStartup_Title1,	CV_RGB(0xA1,	0x40,	0xFA));
//	SetWidgetTextColor(	kStartup_Title1,	CV_RGB(255,		0,		0));
	SetWidgetText(		kStartup_Title1, "SkyTravel");

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
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);

		yLoc			+=	cRadioBtnHt;
//		yLoc			+=	2;
	}
	yLoc			+=	2;

	//--------------------------------------------
	yLoc			=	cHeight - cBtnHeight;
	yLoc			-=	10;
	buttonWidth		=	100;
	xLoc			=	(cWidth - buttonWidth) / 2;
	SetWidget(			kStartup_CloseBtn,	xLoc,			yLoc,	buttonWidth,	cBtnHeight);
	SetWidgetType(		kStartup_CloseBtn,	kWidgetType_Button);
	SetWidgetBGColor(	kStartup_CloseBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kStartup_CloseBtn,	CV_RGB(0,	0,	0));
	SetWidgetText(		kStartup_CloseBtn, "Close");
	SetWidgetValid(		kStartup_CloseBtn, false);
	xLoc	+=	buttonWidth;
	xLoc	+=	2;

	SetWidget(			kStartup_DumpBtn,	xLoc,			yLoc,	buttonWidth,	cBtnHeight);
	SetWidgetType(		kStartup_DumpBtn,	kWidgetType_Button);
	SetWidgetBGColor(	kStartup_DumpBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kStartup_DumpBtn,	CV_RGB(0,	0,	0));
	SetWidgetText(		kStartup_DumpBtn, "Dump");


	SetAlpacaLogoBottomCorner(kStartup_AlpacaLogo);
	LLG_SetColor(W_BLACK);
}

//*****************************************************************************
void	WindowTabStartup::ProcessButtonClick(const int buttonIdx, const int flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kStartup_CloseBtn:
			CloseWindow();
			break;

		case kStartup_DumpBtn:
			DumpWidgetList(kStartup_Title1, kStartup_AlpacaLogo, __FILE__);
			break;

	}
	ForceWindowUpdate();
}

