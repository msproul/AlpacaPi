//*****************************************************************************
//*		windowtab_cpustats.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:	List window of star objects
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
//*	Jul 15,	2022	<MLS> Created windowtab_cpustats.cpp
//*****************************************************************************

#ifdef _ENABLE_CPU_STATS_

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"windowtab.h"
#include	"windowtab_cpustats.h"


//**************************************************************************************
WindowTabCpuStats::WindowTabCpuStats(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
										:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;

	SetupWindowControls(windowName);

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabCpuStats::~WindowTabCpuStats(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//**************************************************************************************
void	WindowTabCpuStats::SetupWindowControls(const char *argWindowName)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabSizes[kMaxTabStops]	=	{200, 100, 100, 100, 100, 100, 100, 100, -1};
short	tabArray[kMaxTabStops];
short	currentTabPos;
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);
	currentTabPos	=	0;
	iii				=	0;
	while ((iii < kMaxTabStops) && (tabSizes[iii] > 0))
	{
		currentTabPos	+=	tabSizes[iii];
		tabArray[iii]	=	currentTabPos;
		iii++;
	}

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kCpuStats_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kCpuStats_Title, argWindowName);
	SetBGcolorFromWindowName(kCpuStats_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc	=	1;
	iii				=	kCpuStats_ClmTitle1;
	while (iii <= kCpuStats_ClmTitle6)
	{
		clmnHdrWidth	=	tabArray[iii - kCpuStats_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,		yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kCpuStats_ClmTitle1];;
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kCpuStats_ClmTitle1,	"Window");
	SetWidgetText(		kCpuStats_ClmTitle2,	"foo");
	SetWidgetText(		kCpuStats_ClmTitle3,	"foo");
	SetWidgetText(		kCpuStats_ClmTitle4,	"foo");
	SetWidgetText(		kCpuStats_ClmTitle5,	"foo");
	SetWidgetText(		kCpuStats_ClmTitle6,	"foo");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	5;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	textBoxWd	-=	20;			//*	leave room for the scroll bar
	for (iii = kCpuStats_Obj_01; iii <= kCpuStats_Obj_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBorder(		iii,	false);

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}
	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(		kCpuStats_ScrollBar,	(xLoc + textBoxWd + 2),		yLoc2,		15,		600);
	SetWidgetType(	kCpuStats_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(	kCpuStats_ScrollBar,	kFont_Small);
//	CONSOLE_DEBUG_W_NUM("kCpuStats_ScrollBar\t=", kCpuStats_ScrollBar);

	//---------------------------------------------------------------------
	xLoc		=	0;
	widgetWidth	=	cWidth / 4;

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

	SetAlpacaLogoBottomCorner(kCpuStats_AlpacaLogo);

}

//*****************************************************************************
//void	WindowTabCpuStats::RunWindowBackgroundTasks(void)
//{


//}

//**************************************************************************************
void	WindowTabCpuStats::ActivateWindow(void)
{
	UpdateOnScreenWidgetList();
}
//*****************************************************************************
void	WindowTabCpuStats::ProcessButtonClick(const int buttonIdx, const int flags)
{
//int	newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kCpuStats_ClmTitle1:
		case kCpuStats_ClmTitle2:
		case kCpuStats_ClmTitle3:
		case kCpuStats_ClmTitle4:
		case kCpuStats_ClmTitle5:
		case kCpuStats_ClmTitle6:
//			newSortColumn	=	buttonIdx - kCpuStats_ClmTitle1;
//			if (newSortColumn == cSortColumn)
//			{
//				gInvertSort_StarList	=	!gInvertSort_StarList;
//			}
//			else
//			{
//				gInvertSort_StarList	=	false;
//			}
//			cSortColumn		=	newSortColumn;
//			UpdateSortOrder();
//			ForceWindowUpdate();
			break;

	}
}

//*****************************************************************************
void	WindowTabCpuStats::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
//int		screenLineNum;
//int		starDataIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
	//*	check to see if it is one of the entries
	if ((widgetIdx >= kCpuStats_Obj_01) && (widgetIdx <= kCpuStats_Obj_Last))
	{
//		if (cStarListPtr != NULL)
//		{
//			screenLineNum	=	widgetIdx - kCpuStats_Obj_01;
//			starDataIdx		=	screenLineNum + cFirstLineIdx;
//			CONSOLE_DEBUG_W_NUM("starDataIdx\t=", starDataIdx);
//			if ((starDataIdx >= 0) && (starDataIdx < cStarListCount))
//			{
//				CONSOLE_DEBUG_W_STR("Double clicked on: ", cStarListPtr[starDataIdx].shortName);
//				CONSOLE_DEBUG_W_STR("Double clicked on: ", cStarListPtr[starDataIdx].longName);
//				Center_CelestralObject(&cStarListPtr[starDataIdx]);
//			}
//			else
//			{
//				CONSOLE_DEBUG_W_NUM("invalid starDataIdx\t=", starDataIdx);
//				CONSOLE_ABORT(__FUNCTION__);
//			}
//		}
	}
}

//*****************************************************************************
void	WindowTabCpuStats::ProcessMouseWheelMoved(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	wheelMovement,
													const int	flags)

{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	cFirstLineIdx	+=	wheelMovement;
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}
	UpdateOnScreenWidgetList();
	ForceWindowUpdate();
}

//**************************************************************************************
void	WindowTabCpuStats::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[256];
char	formatString[64];
int		lineIndex;

	CONSOLE_DEBUG(__FUNCTION__);

	lineIndex	=	0;

	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			boxId	=	lineIndex + kCpuStats_Obj_01;

			strcpy(textString, gControllerList[iii]->cWindowName);

			sprintf(formatString, "\t%ld\t%ld\t%ld\t%ld",
								(gControllerTime[iii].Count),
								(gControllerTime[iii].RecentNanoSecons),
								(gControllerTime[iii].AverageNanoSecons),
								(gControllerTime[iii].TotalNanoSecons / 1000));

			strcat(textString, formatString);
			SetWidgetText(boxId, textString);

			lineIndex++;
		}
	}

	//*	set the unused ones to null string
	while (lineIndex < kMaxControllers)
	{
		boxId	=	lineIndex + kCpuStats_Obj_01;
		SetWidgetText(boxId, "");

		lineIndex++;
	}

//	SetWidgetScrollBarLimits(	kCpuStats_ScrollBar, (kCpuStats_Obj_Last - kCpuStats_Obj_01), cStarListCount);
	SetWidgetScrollBarValue(	kCpuStats_ScrollBar, 100);
}



#endif // _ENABLE_CPU_STATS_
