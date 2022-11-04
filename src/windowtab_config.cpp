//*****************************************************************************
//*		windowtab_config.cpp		(c) 2020 by Mark Sproul
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
//*	Feb 24,	2020	<MLS> Created windowtab_config.cpp
//*****************************************************************************

#ifdef _ENABLE_CTRL_FOCUSERS_



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"controller.h"
#include	"windowtab_config.h"
#include	"nitecrawler_colors.h"

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabConfig::WindowTabConfig(	const int	xSize,
									const int	ySize,
									cv::Scalar backGrndColor)
	:WindowTab(xSize, ySize, backGrndColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);

//	CONSOLE_DEBUG_W_HEX("cWidgetList=", cWidgetList);

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabConfig::~WindowTabConfig(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabConfig::SetupWindowControls(void)
{
int		yLoc;
int		iii;
int		colorBoxNum;
int		textBoxNum;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kCongfigBox_Config1,		0,			yLoc,		cWidth,		cBoxHeight	);
	SetWidgetFont(kCongfigBox_Config1, kFont_Medium);
	SetWidgetText(kCongfigBox_Config1, "config1");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	SetWidget(kCongfigBox_Config2,		0,			yLoc,		cWidth,		cBoxHeight	);
	SetWidgetText(kCongfigBox_Config2, "config2");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	SetWidget(kCongfigBox_Config3,		0,			yLoc,		cWidth,		cBoxHeight	);
	SetWidgetText(kCongfigBox_Config3, "config3");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	//========================================================
	//*	set up the color boxes

	yLoc		+=	10;
	colorBoxNum	=	kCongfigBox_Color01;
	textBoxNum	=	kCongfigBox_Text01;
	for (iii=0; iii<12; iii++)
	{
		SetWidget(			colorBoxNum,	cClm3_offset,			yLoc,		cClmWidth,		cRadioBtnHt);
		SetWidgetBGColor(	colorBoxNum,	Color16BitTo24Bit(gNiteCrawlerColors[iii].color16bitDefault));

		SetWidget(			textBoxNum,		cClm4_offset,			yLoc,		cClmWidth * 3,	cRadioBtnHt);
		SetWidgetText(		textBoxNum,		gNiteCrawlerColors[iii].name);
		SetWidgetFont(		textBoxNum,		kFont_Small);
		SetWidgetJustification(textBoxNum, kJustification_Left);
		SetWidgetTextColor(textBoxNum, CV_RGB(255, 255, 255));
		SetWidgetBorder(textBoxNum, false);
		colorBoxNum++;
		textBoxNum++;
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
//******************************************************************************
//*	<C 20# >00
//*	<C 01# >ffff
//*	<C 02# >0000
//*	<C 03# >39e7
//*	<C 04# >f800
//*	<C 05# >f800
//*	<C 06# >8000
//*	<C 07# >fe00
//*	<C 08# >07ff
//*	<C 12# >39e7
//*	<C 14# >ffff
//*	<C 11# >000f
//*	<C 13# >f800
//******************************************************************************
#if 0
	iii	=	kCongfigBox_Color01;

	SetWidgetBGColor(iii++, CV_RGB(255,	255,	255));
	SetWidgetBGColor(iii++, CV_RGB(255,	0,		0));
	SetWidgetBGColor(iii++, CV_RGB(0,	255,	0));
	SetWidgetBGColor(iii++, CV_RGB(0,	0,		255));

	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x0000));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0xffff));


	SetWidgetBGColor(iii++, Color16BitTo24Bit(0xf800));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x07E0));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x001f));

	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x39e7));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0xf800));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0xfe00));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x07ff));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x000f));
	SetWidgetBGColor(iii++, Color16BitTo24Bit(0x8000));
#endif

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kCongfigBox_IPaddr,
							kCongfigBox_Readall,
							kCongfigBox_AlpacaErrorMsg,
							kCongfigBox_LastCmdString,
							kCongfigBox_AlpacaLogo,
							-1);

}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabConfig::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabConfig::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
//******************************************************************************
{

	cOpenCV_Image	=	openCV_Image;


//	switch(widgetIdx)
//	{
//		case kCongfigBox_Config1:
//		case kCongfigBox_Config2:
//		case kCongfigBox_Config3:
//			break;
//
//		default:
//			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
//			break;
//	}
}


#endif // _ENABLE_CTRL_FOCUSERS_
