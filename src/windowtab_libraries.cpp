//*****************************************************************************
//*		windowtab_libraries.cpp		(c) 2023 by Mark Sproul
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
//*	Dec 23,	2023	<MLS> Created windowtab_libraries.cpp
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cpu_stats.h"

#include	"windowtab.h"
#include	"windowtab_libraries.h"
#include	"controller.h"
#include	"controller_alpacaUnit.h"


#define	kLibrariesHeight	100



//**************************************************************************************
WindowTabLibraries::WindowTabLibraries(	const int	xSize,
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
WindowTabLibraries::~WindowTabLibraries(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabLibraries::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		textBoxHt;
int		textBoxWd;
//int		iii;
//short	tabArray[kMaxTabStops]	=	{200, 400, 600, 1000, 1199, 0};

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kLibraries_Title, -1, yLoc, "AlpacaPi project");

	xLoc		=	3;
	textBoxWd	=	cWidth - 6;
	SetWidget(				kLibraries_DescriptionBox,	xLoc,	yLoc,	textBoxWd,		cTitleHeight);
	SetWidgetType(			kLibraries_DescriptionBox,	kWidgetType_TextBox);
	SetWidgetJustification(	kLibraries_DescriptionBox,	kJustification_Center);
	SetWidgetFont(			kLibraries_DescriptionBox,	kFont_Medium);
	SetWidgetTextColor(		kLibraries_LibrariesBox,	CV_RGB(255,	255,	255));

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-------------------------------------------------------------
	//*	the box for the remote cpu info
	textBoxHt		=	cHeight - yLoc;
	textBoxHt		-=	200;

	SetWidget(				kLibraries_LibrariesBox,	xLoc,	yLoc,	textBoxWd,		textBoxHt);
	SetWidgetType(			kLibraries_LibrariesBox,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kLibraries_LibrariesBox,	kJustification_Left);
	SetWidgetFont(			kLibraries_LibrariesBox,	kFont_Medium);
	SetWidgetTextColor(		kLibraries_LibrariesBox,	CV_RGB(255,	255,	255));

	yLoc			+=	textBoxHt;
	yLoc			+=	2;
	SetWidgetText(kLibraries_DescriptionBox, "List of software libraries / versions on remote system");
	SetWidgetText(kLibraries_LibrariesBox, "List of software libraries / versions on remote system");



//	SetAlpacaLogoBottomCorner(kLibraries_AlpacaLogo);
	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kLibraries_IPaddr,		//	const int	ipaddrBox,
							-1,						//	const int	readAllBox,
							-1,						//	const int	deviceStateBox,
							-1,						//	const int	errorMsgBox,
							-1,						//	const int	lastCmdWidgetIdx,
							kLibraries_AlpacaLogo,	//	const int	logoWidgetIdx,
							-1,						//	const int	helpBtnBox ,
							-1,						//	const bool	logoSideOfScreen,
							-1);					//	const int	connectBtnBox

}

