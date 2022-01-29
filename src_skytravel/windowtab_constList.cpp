//*****************************************************************************
//*		windowtab_constList.cpp		(c) 2021 by Mark Sproul
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
//*	Oct 27,	2021	<MLS> Created windowtab_constList.cpp
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"windowtab.h"
#include	"windowtab_constList.h"

static int	gSortColumn_ConstList;
static bool	gInvertSort_ConstList	=	false;

//**************************************************************************************
WindowTabConstellationList::WindowTabConstellationList(	const int	xSize,
										const int	ySize,
										CvScalar	backGrndColor,
										const char	*windowName)
										:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;
	cConstellationsPtr	=	NULL;
	cConstellationCnt	=	0;

	SetupWindowControls(windowName);

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabConstellationList::~WindowTabConstellationList(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabConstellationList::SetupWindowControls(const char *argWindowName)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{200, 300, 600, 700, 900, 950, 0, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kConstList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kConstList_Title, argWindowName);
	SetBGcolorFromWindowName(kConstList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc	=	1;
	iii				=	kConstList_ClmTitle1;
	while (iii <= kConstList_ClmTitle6)
	{
		clmnHdrWidth	=	tabArray[iii - kConstList_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,		yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kConstList_ClmTitle1];;
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kConstList_ClmTitle1,	"Name");
	SetWidgetText(		kConstList_ClmTitle2,	"Star Count");
	SetWidgetText(		kConstList_ClmTitle3,	"Center RA");
	SetWidgetText(		kConstList_ClmTitle4,	"Center Dec");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	5;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	textBoxWd	-=	20;			//*	leave room for the scroll bar
	for (iii = kConstList_Obj_01; iii <= kConstList_Obj_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBorder(		iii,	false);
//		SetWidgetBorder(		iii,	true);
//		SetWidgetNumber(		iii,	(iii - kConstList_Obj_01));

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}
	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(		kConstList_ScrollBar,	(xLoc + textBoxWd + 2),		yLoc2,		15,		600);
	SetWidgetType(	kConstList_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(	kConstList_ScrollBar,	kFont_Small);
//	CONSOLE_DEBUG_W_NUM("kConstList_ScrollBar\t=", kConstList_ScrollBar);

	//---------------------------------------------------------------------
	xLoc		=	0;
	widgetWidth	=	cWidth / 4;
	SetWidget(				kConstList_Obj_Total,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kConstList_Obj_Total,	kFont_Medium);
	SetWidgetText(			kConstList_Obj_Total,	"Total alerts =?");
	SetWidgetJustification(	kConstList_Obj_Total,	kJustification_Left);
	SetWidgetTextColor(		kConstList_Obj_Total,	CV_RGB(255,	255,	255));




	xLoc		+=	widgetWidth;
	xLoc		+=	2;
//	SetWidget(				kConstList_ExportCSV,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
//	SetWidgetType(			kConstList_ExportCSV,	kWidgetType_Button);
//	SetWidgetFont(			kConstList_ExportCSV,	kFont_Medium);
//	SetWidgetText(			kConstList_ExportCSV,	"Export to CSV");
//	SetWidgetJustification(	kConstList_ExportCSV,	kJustification_Center);
//	SetWidgetBGColor(		kConstList_ExportCSV,	CV_RGB(255,	255,	255));

//	SetAlpacaLogoBottomCorner(kConstList_AlpacaLogo);

}


//*****************************************************************************
void	WindowTabConstellationList::ProcessButtonClick(const int buttonIdx)
{
int	newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kConstList_ClmTitle1:
		case kConstList_ClmTitle2:
		case kConstList_ClmTitle3:
		case kConstList_ClmTitle4:
		case kConstList_ClmTitle5:
		case kConstList_ClmTitle6:
			newSortColumn	=	buttonIdx - kConstList_ClmTitle1;
			if (newSortColumn == cSortColumn)
			{
				gInvertSort_ConstList	=	!gInvertSort_ConstList;
			}
			else
			{
				gInvertSort_ConstList	=	false;
			}
			cSortColumn		=	newSortColumn;
			UpdateSortOrder();
			ForceUpdate();
			break;


	}
}

//*****************************************************************************
void	WindowTabConstellationList::ProcessDoubleClick(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
int		screenLineNum;
int		constellationIdx;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
	//*	check to see if it is one of the entries
	if ((widgetIdx >= kConstList_Obj_01) && (widgetIdx <= kConstList_Obj_Last))
	{
		if (cConstellationsPtr != NULL)
		{
			screenLineNum	=	widgetIdx - kConstList_Obj_01;
			constellationIdx		=	screenLineNum + cFirstLineIdx;
			CONSOLE_DEBUG_W_NUM("constellationIdx\t=", constellationIdx);
			if ((constellationIdx >= 0) && (constellationIdx < cConstellationCnt))
			{
				CONSOLE_DEBUG("--------------------------------------------------------");
				CONSOLE_DEBUG_W_STR("Constellation\t=", cConstellationsPtr[constellationIdx].constellationName);
				for (iii=0; iii<cConstellationsPtr[constellationIdx].starCount; iii++)
				{
					printf("Hip start #%d\t", cConstellationsPtr[constellationIdx].hippStars[iii].hippIdNumber);
					printf("%1.3f\t", DEGREES(cConstellationsPtr[constellationIdx].hippStars[iii].rtAscension)/15.0);
					printf("%1.3f\t", DEGREES(cConstellationsPtr[constellationIdx].hippStars[iii].declination));
					printf("\r\n");
				}
			//	CONSOLE_ABORT(__FUNCTION__);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("invalid constellationIdx\t=", constellationIdx);
				CONSOLE_ABORT(__FUNCTION__);
			}
		}
	}
}

//*****************************************************************************
void	WindowTabConstellationList::ProcessMouseWheelMoved(	const int	widgetIdx,
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
	ForceUpdate();
}

//*****************************************************************************
void	WindowTabConstellationList::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
//uint32_t			currentMillis;
//uint32_t			deltaMilliSecs;
//int					newSliderValue_int;

	CONSOLE_DEBUG(__FUNCTION__);
//	currentMillis	=	millis();

	switch(widgetIdx)
	{
		case kConstList_ScrollBar:
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
void	WindowTabConstellationList::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[256];
int		myDevCount;
int		constellationIdx;
//char	formatString[64];
//short	raHours;
//short	raMinutes;
//double	raSecs;
//short	declDegress;
//short	declMinutes;
//double	declSecs;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii					=	0;
	constellationIdx	=	cFirstLineIdx;
	myDevCount			=	0;
	boxId				=	0;
	//*	check to make sure we have valid data
	if (cConstellationsPtr != NULL)
	{
		while (	(iii <= (kConstList_Obj_Last - kConstList_Obj_01)) &&
				(constellationIdx < cConstellationCnt))
		{
			boxId	=	iii + kConstList_Obj_01;
			sprintf(textString, "%s\t%d\t",
							cConstellationsPtr[constellationIdx].constellationName,
							cConstellationsPtr[constellationIdx].starCount);

			SetWidgetText(boxId, textString);

			myDevCount++;
			constellationIdx++;
			iii++;
		}

	}
	else
	{
	//	CONSOLE_ABORT(__FUNCTION__);
	}
	//*	now set the rest of them to empty strings
	while (iii <= (kConstList_Obj_Last - kConstList_Obj_01))
	{
		boxId	=	iii + kConstList_Obj_01;
		SetWidgetText(boxId, "--");
		iii++;
	}

	sprintf(textString, "Total stars =%d", cConstellationCnt);
	SetWidgetText(	kConstList_Obj_Total,	textString);

	SetWidgetScrollBarLimits(	kConstList_ScrollBar, (kConstList_Obj_Last - kConstList_Obj_01), cConstellationCnt);
	SetWidgetScrollBarValue(	kConstList_ScrollBar, 100);

	SetWidgetBGColor(kConstList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kConstList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kConstList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kConstList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kConstList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kConstList_ClmTitle6,	((cSortColumn == 5) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}

//**************************************************************************************
void	WindowTabConstellationList::SetStarDataPointers(TYPE_ConstVector *argConstellationPtr, int argConstellationCount)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cConstellationsPtr	=	argConstellationPtr;
	cConstellationCnt	=	argConstellationCount;

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
void	WindowTabConstellationList::SetColumnOneTitle(const char *clmOneTitle)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (clmOneTitle != NULL)
	{
		SetWidgetText(		kConstList_ClmTitle1,	clmOneTitle);
	}
}


//**************************************************************************************
static  int StarDataSortProc_Constellations(const void *e1, const void *e2)
{
TYPE_ConstVector	*obj1, *obj2;
int					returnValue;

	obj1			=	(TYPE_ConstVector *)e1;
	obj2			=	(TYPE_ConstVector *)e2;

	returnValue	=	0;
	switch(gSortColumn_ConstList)
	{
		case 0:
			returnValue	=	strcasecmp(obj1->constellationName, obj2->constellationName);
			break;

		case 1:
			returnValue	=	obj1->starCount - obj2->starCount;
			break;

		case 2:
			break;

		case 3:
			break;
	}
	//*	if they are the same, sort by ID (alert ID)
	if (returnValue == 0)
	{
		returnValue	=	strcasecmp(obj1->constellationName, obj2->constellationName);
	}
	if (gInvertSort_ConstList)
	{
		returnValue	=	-returnValue;
	}
	return(returnValue);
}


//**************************************************************************************
void	WindowTabConstellationList::UpdateSortOrder(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevCnt\t=", cAlpacaDevCnt);

	if (cConstellationsPtr != NULL)
	{
		if (cSortColumn >= 0)
		{
			gSortColumn_ConstList	=	cSortColumn;
			qsort(	cConstellationsPtr,
					cConstellationCnt,
					sizeof(TYPE_ConstVector),
					StarDataSortProc_Constellations);

			UpdateOnScreenWidgetList();
		}
	}
}

