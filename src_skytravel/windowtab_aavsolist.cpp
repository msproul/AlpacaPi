//*****************************************************************************
//*		windowtab_aavsolist.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:	C++ to list AAVSO alerts
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
//*	Jun 30,	2021	<MLS> Created windowtab_aavsolist.cpp
//*	Jul 18,	2021	<MLS> aavso list starting to work
//*	Aug 10,	2021	<MLS> Double Click now moves the star to the center
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"windowtab.h"
#include	"windowtab_aavsolist.h"

static int	gSortColumn;
static bool	gInvertSort	=	false;

//**************************************************************************************
WindowTabAAVSOlist::WindowTabAAVSOlist(	const int	xSize,
										const int	ySize,
										CvScalar	backGrndColor,
										const char	*windowName)
										:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;

	SetupWindowControls();

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabAAVSOlist::~WindowTabAAVSOlist(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabAAVSOlist::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{100, 400, 600, 700, 900, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kAAVSOlist_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kAAVSOlist_Title, "AAVSO Object List");
	SetBGcolorFromWindowName(kAAVSOlist_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc	=	1;
	iii				=	kAAVSOlist_ClmTitle1;
	while(iii <= kAAVSOlist_ClmTitle5)
	{
		clmnHdrWidth	=	tabArray[iii - kAAVSOlist_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,		yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kAAVSOlist_ClmTitle1];;
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kAAVSOlist_ClmTitle1,	"AlertID");
	SetWidgetText(		kAAVSOlist_ClmTitle2,	"Star Name");
	SetWidgetText(		kAAVSOlist_ClmTitle3,	"Bar");
	SetWidgetText(		kAAVSOlist_ClmTitle4,	"Baz");
	SetWidgetText(		kAAVSOlist_ClmTitle5,	"???");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	5;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	textBoxWd	-=	20;			//*	leave room for the scroll bar
	for (iii=kAAVSOlist_Obj_01; iii<=kAAVSOlist_Obj_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBorder(		iii,	false);
//		SetWidgetBorder(		iii,	true);
//		SetWidgetNumber(		iii,	(iii - kAAVSOlist_Obj_01));

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}
	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(		kAAVSOlist_ScrollBar,	(xLoc + textBoxWd + 2),		yLoc2,		15,		600);
	SetWidgetType(	kAAVSOlist_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(	kAAVSOlist_ScrollBar,	kFont_Small);

//	CONSOLE_DEBUG_W_NUM("kAAVSOlist_ScrollBar\t=", kAAVSOlist_ScrollBar);

	//---------------------------------------------------------------------
	xLoc		=	0;
	widgetWidth	=	cWidth / 4;
	SetWidget(				kAAVSOlist_Obj_Total,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kAAVSOlist_Obj_Total,	kFont_Medium);
	SetWidgetText(			kAAVSOlist_Obj_Total,	"Total alerts =?");
	SetWidgetJustification(	kAAVSOlist_Obj_Total,	kJustification_Left);
	SetWidgetTextColor(		kAAVSOlist_Obj_Total,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

//	SetAlpacaLogoBottomCorner(kAAVSOlist_AlpacaLogo);

}

//*****************************************************************************
//void	WindowTabAAVSOlist::RunBackgroundTasks(void)
//{


//}

//*****************************************************************************
void	WindowTabAAVSOlist::ProcessButtonClick(const int buttonIdx)
{
int	newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kAAVSOlist_ClmTitle1:
		case kAAVSOlist_ClmTitle2:
		case kAAVSOlist_ClmTitle3:
		case kAAVSOlist_ClmTitle4:
		case kAAVSOlist_ClmTitle5:
			newSortColumn	=	buttonIdx - kAAVSOlist_ClmTitle1;
			if (newSortColumn == cSortColumn)
			{
				gInvertSort	=	!gInvertSort;
			}
			else
			{
				gInvertSort	=	false;
			}
			cSortColumn		=	newSortColumn;
			UpdateSortOrder();
			ForceUpdate();
			break;

	}
}

//*****************************************************************************
void	WindowTabAAVSOlist::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
int		screenLineNum;
int		aavsoIdx;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
	//*	check to see if it is one of the entries
	if ((widgetIdx >= kAAVSOlist_Obj_01) && (widgetIdx <= kAAVSOlist_Obj_Last))
	{
		screenLineNum	=	widgetIdx - kAAVSOlist_Obj_01;
		aavsoIdx		=	screenLineNum + cFirstLineIdx;
		CONSOLE_DEBUG_W_NUM("aavsoIdx\t=", aavsoIdx);
		if ((aavsoIdx >= 0) && (aavsoIdx < gAAVSOalertsCnt))
		{
			CONSOLE_DEBUG_W_LONG("entry\t=", gAAVSOalertsPtr[aavsoIdx].id);
			CONSOLE_DEBUG_W_STR("entry\t=", gAAVSOalertsPtr[aavsoIdx].longName);
			Goto_RA_DEC(gAAVSOalertsPtr[aavsoIdx].ra, gAAVSOalertsPtr[aavsoIdx].decl);
		}
		else
		{
		}
	}
}

//*****************************************************************************
void	WindowTabAAVSOlist::ProcessMouseWheelMoved(const int widgetIdx, const int event, const int xxx, const int yyy, const int wheelMovement)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	cFirstLineIdx	+=	wheelMovement;
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}
	UpdateOnScreenWidgetList();
	ForceUpdate();
}

//*****************************************************************************
void	WindowTabAAVSOlist::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
//uint32_t			currentMillis;
//uint32_t			deltaMilliSecs;
//int					newSliderValue_int;


	CONSOLE_DEBUG(__FUNCTION__);
//	currentMillis	=	millis();


	switch(widgetIdx)
	{
		case kAAVSOlist_ScrollBar:
		//	deltaMilliSecs	=	currentMillis - cLastExposureUpdate_Millis;
		//	if (deltaMilliSecs > 4)
			{
		//		cLastExposureUpdate_Millis	=	millis();
			}
			break;

	}

	ForceUpdate();
}

//**************************************************************************************
void	WindowTabAAVSOlist::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[128];
int		myDevCount;
int		aavsoIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii			=	0;
	aavsoIdx	=	cFirstLineIdx;
	myDevCount	=	0;
	boxId		=	0;
	//*	check to make sure we have valid data
	if (gAAVSOalertsPtr != NULL)
	{
		while (	(iii <= (kAAVSOlist_Obj_Last - kAAVSOlist_Obj_01)) &&
				(aavsoIdx < gAAVSOalertsCnt))
		{
			boxId	=	iii + kAAVSOlist_Obj_01;
			sprintf(textString, "%ld\t%s",
									gAAVSOalertsPtr[aavsoIdx].id,
									gAAVSOalertsPtr[aavsoIdx].longName);


//			CONSOLE_DEBUG(textString);
			SetWidgetText(boxId, textString);

			myDevCount++;
			aavsoIdx++;
			iii++;
		}
		//*	now set the rest of them to empty strings
		while (iii <= (kAAVSOlist_Obj_Last - kAAVSOlist_Obj_01))
		{
			boxId	=	iii + kAAVSOlist_Obj_01;
			SetWidgetText(boxId, "");
			iii++;
		}
	}

	sprintf(textString, "Total alerts =%ld", gAAVSOalertsCnt);
	SetWidgetText(	kAAVSOlist_Obj_Total,	textString);

	SetWidgetScrollBarLimits(	kAAVSOlist_ScrollBar, (kAAVSOlist_Obj_Last - kAAVSOlist_Obj_01), gAAVSOalertsCnt);
	SetWidgetScrollBarValue(	kAAVSOlist_ScrollBar, 100);

	SetWidgetBGColor(kAAVSOlist_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAAVSOlist_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAAVSOlist_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAAVSOlist_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAAVSOlist_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}


//**************************************************************************************
static  int AAVSOalertSortProc(const void *e1, const void *e2)
{
TYPE_CelestData	*obj1, *obj2;
int				returnValue;

	obj1		=	(TYPE_CelestData *)e1;
	obj2		=	(TYPE_CelestData *)e2;

	returnValue	=	0;
	switch(gSortColumn)
	{
		case 0:
			returnValue	=	0;	//*	let the default code below take care of this
			break;

		case 1:
			returnValue	=	strcasecmp(obj1->longName, obj2->longName);
			break;

//		case 2:
//			returnValue	=	obj2->id - obj1->id;
//			break;

//		case 3:
//			returnValue	=	strcasecmp(obj2->longName, obj1->longName);
//			break;
	}
	//*	if they are the same, sort by ID (alert ID)
	if (returnValue == 0)
	{
		returnValue	=	obj1->id - obj2->id;
	}
	if (gInvertSort)
	{
		returnValue	=	-returnValue;
	}
	return(returnValue);
}


//**************************************************************************************
void	WindowTabAAVSOlist::UpdateSortOrder(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevCnt\t=", cAlpacaDevCnt);

	if (gAAVSOalertsPtr != NULL)
	{
		if (cSortColumn >= 0)
		{
			gSortColumn	=	cSortColumn;
			qsort(gAAVSOalertsPtr, gAAVSOalertsCnt, sizeof(TYPE_CelestData), AAVSOalertSortProc);

			UpdateOnScreenWidgetList();
		}
	}
}

