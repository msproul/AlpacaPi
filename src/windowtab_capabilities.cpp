//*****************************************************************************
//*		windowtab_capabilities.cpp		(c) 2021 by Mark Sproul
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
//*	Feb 19,	2021	<MLS> Created windowtab_capabilities.cpp
//*****************************************************************************



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_capabilities.h"
#include	"controller.h"


#define	kCapabilitiesHeight	100

//**************************************************************************************
WindowTabCapabilities::WindowTabCapabilities(	const int	xSize,
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
WindowTabCapabilities::~WindowTabCapabilities(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}



//**************************************************************************************
void	WindowTabCapabilities::SetupWindowControls(void)
{
int		yLoc;
int		iii;
short	tabStopList[]	=	{250, 300, 350, 400, 0 };

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kCapabilities_Title, -1, yLoc, "Driver capabilites");

	for (iii=kCapabilities_TextBox1; iii<=kCapabilities_TextBoxN; iii++)
	{
		SetWidget(				iii,	0,			yLoc,		cWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetTabStops(		iii,	tabStopList);
		SetWidgetBorder(		iii,	false);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
//		SetWidgetText(			iii,	"---\t---");

		yLoc			+=	cRadioBtnHt;
//		yLoc			+=	2;
	}

	SetAlpacaLogoBottomCorner(kCapabilities_AlpacaLogo);
}




