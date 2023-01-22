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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 21,	2020	<MLS> Created windowtab_about.cpp
//*	Dec 14,	2020	<MLS> Updated web link in about box
//*	Jan 17,	2021	<MLS> Added CPU info to about box
//*	Nov 24,	2022	<MLS> Changed CPU info to show "REMOTE DEVICE: data not available" by default
//*	Nov 24,	2022	<MLS> Added SetLocalDeviceInfo(), only called by SkyTravel Controller
//*****************************************************************************

#include	<fitsio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_about.h"
#include	"controller.h"
#include	"cpu_stats.h"

#ifdef _ENABLE_REMOTE_GAIA_
	#include	"RemoteGaia.h"
#endif
#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabAbout::WindowTabAbout(	const int	xSize,
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
WindowTabAbout::~WindowTabAbout(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
static char	gAlpacaPiTxt1[]	=
{
	"AlpacaPi Project\r"
	"(C) Mark Sproul 2019-2022\r"
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
	"^w Closes current window, quits if only one.\r"
	"Clicking the round x in the title bar does NOT work.\r"
	"Clicking the square red X in the top left corner closes the window.\r"
};


//**************************************************************************************
void	WindowTabAbout::SetupWindowControls(void)
{
int		yLoc;
int		availSpace;
int		textBoxHt;
int		iii;

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

	SetAlpacaLogoBottomCorner(kAboutBox_AlpacaLogo);

	availSpace		=	cHeight;
	availSpace		-=	cTabVertOffset;
	availSpace		-=	cTitleHeight + 2;
	availSpace		-=	cTitleHeight + 2;
	availSpace		-=	5;

	textBoxHt		=	availSpace / 4;
	textBoxHt		-=	2;
//	CONSOLE_DEBUG_W_NUM("textBoxHt\t=", textBoxHt);

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

	SetWidgetText(kAboutBox_CPUinfo, "REMOTE DEVICE: data not available");
}

//**************************************************************************************
void	WindowTabAbout::SetLocalDeviceInfo(void)
{
char	lineBuffer[64];
char	multiLineTextBuff[512];


	multiLineTextBuff[0]	=	0;

	strcat(multiLineTextBuff,	"LOCAL DEVICE:\r");
	strcat(multiLineTextBuff,	gOsReleaseString);
	strcat(multiLineTextBuff,	"\r");
	strcat(multiLineTextBuff,	gCpuInfoString);
	strcat(multiLineTextBuff,	"\r");
	strcat(multiLineTextBuff,	gPlatformString);
	strcat(multiLineTextBuff,	"\r");

#ifdef __ARM_NEON
	strcat(multiLineTextBuff,	"ARM NEON instructions\r");
#endif

#ifdef _ENABLE_FITS_
	//**************************************************************
	//*	cfitsio version

	#ifdef CFITSIO_MICRO
		sprintf(lineBuffer,	"FITS (cfitsio) V%d.%d.%d\r", CFITSIO_MAJOR, CFITSIO_MINOR, CFITSIO_MICRO);
	#else
		sprintf(lineBuffer,	"FITS (cfitsio) V%d.%d\r", CFITSIO_MAJOR, CFITSIO_MINOR);
	#endif
	strcat(multiLineTextBuff,	lineBuffer);

//float	cfitsioVersion;
//	ffvers(&cfitsioVersion);
//	sprintf(lineBuffer,	"FITS (cfitsio) V%3.2f\r", cfitsioVersion);
//	strcat(multiLineTextBuff,	lineBuffer);
//	CONSOLE_DEBUG_W_DBL("cfitsioVersion", cfitsioVersion);

#endif // _ENABLE_FITS_

	//**************************************************************
	//*	OpenCV
	sprintf(lineBuffer,	"OpenCV (include version) %s\r", CV_VERSION);
	strcat(multiLineTextBuff,	lineBuffer);

#if (CV_MAJOR_VERSION >= 4)
	sprintf(lineBuffer,	"OpenCV (library version) %s\r", cv::getVersionString().c_str());
	strcat(multiLineTextBuff,	lineBuffer);
//	printf("%s\r\n",	cv::getBuildInformation().c_str());
#endif


#ifdef _ENABLE_REMOTE_GAIA_
	strcat(multiLineTextBuff,	gSQLclientVersion);
	strcat(multiLineTextBuff,	"\r");
#endif // _ENABLE_REMOTE_GAIA_

	SetWidgetText(kAboutBox_CPUinfo, multiLineTextBuff);
}
