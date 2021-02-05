//*****************************************************************************
//*		windowtab_about.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 21,	2020	<MLS> Created windowtab_about.cpp
//*	Dec 14,	2020	<MLS> Updated web link in about box
//*	Jan 17,	2021	<MLS> Added CPU info to about box
//*****************************************************************************



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_about.h"
#include	"controller.h"
#include	"cpu_stats.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabAbout::WindowTabAbout(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabAbout::~WindowTabAbout(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
static char	gAlpacaPiTxt1[]	=
{
	"AlpacaPi Project\r"
	"(C) Mark Sproul 2019-2020\r"
	"msproul@skychariot.com\r"
	"www.skychariot.com/dome\r"
//	"www.skychariot.com/dome/alpacapi\r"

	"AlpacaPi is open source for non-commercial use\r"
	"https://github.com/msproul/AlpacaPi\r"

};

//**************************************************************************************
static char	gAlpacaPiTxt2[]	=
{
	"AlpacaPi is a series of drivers and applications utilizing the Alpaca Protocol "
	"designed to run on Raspberry Pi and other Linux systems."
};

//**************************************************************************************
static char	gAlpacaPiTxt3[]	=
{
	"Quick help\r"
	"^q to quit\r"
	"^s will save a screen shot\r"
	"^w Closes current window, quits if only one\r"
	"Clicking the round x in the title bar does not work\r"
};


//**************************************************************************************
void	WindowTabAbout::SetupWindowControls(void)
{
int		yLoc;
int		textBoxHt;
int		iii;
char	multiLineTextBuff[512];

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kAboutBox_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kAboutBox_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kAboutBox_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetWidget(		kAboutBox_ControllerVersion,	0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(	kAboutBox_ControllerVersion,	kFont_Medium);
	SetWidgetText(	kAboutBox_ControllerVersion,	gFullVersionString);
	SetWidgetTextColor(kAboutBox_ControllerVersion,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	textBoxHt		=	125;
	for (iii=kAboutBox_TextBox1; iii<=kAboutBox_CPUinfo; iii++)
	{
		SetWidget(				iii,	0,			yLoc,		cWidth,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_MultiLineText);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));

		yLoc			+=	textBoxHt;
		yLoc			+=	2;
	}

	SetWidgetText(kAboutBox_TextBox1, gAlpacaPiTxt1);
	SetWidgetText(kAboutBox_TextBox2, gAlpacaPiTxt2);
	SetWidgetText(kAboutBox_TextBox3, gAlpacaPiTxt3);

	strcpy(multiLineTextBuff,	gOsReleaseString);
	strcat(multiLineTextBuff,	"\r");
	strcat(multiLineTextBuff,	gCpuInfoString);
	strcat(multiLineTextBuff,	"\r");
	strcat(multiLineTextBuff,	gPlatformString);
	strcat(multiLineTextBuff,	"\r");

//	CONSOLE_DEBUG(gOsReleaseString);
//	CONSOLE_DEBUG(gCpuInfoString);
//	CONSOLE_DEBUG(gPlatformString);
//	CONSOLE_DEBUG(multiLineTextBuff);
//	CONSOLE_DEBUG_W_NUM("strlen()", strlen(multiLineTextBuff));

#ifdef __ARM_NEON
	strcat(multiLineTextBuff,	"ARM NEON instructions\r");

#endif

	SetWidgetText(kAboutBox_CPUinfo, multiLineTextBuff);

	SetAlpacaLogo(kAboutBox_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kAboutBox_IPaddr, kAboutBox_Readall, kAboutBox_AlpacaDrvrVersion, -1);
}



