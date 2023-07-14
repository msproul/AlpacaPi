//*****************************************************************************
//*		windowtab_starlist.cpp		(c) 2021 by Mark Sproul
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
//*	Jun 30,	2021	<MLS> Created windowtab_aavsolist.cpp
//*	Jul 18,	2021	<MLS> aavso list starting to work
//*	Aug 10,	2021	<MLS> Double Click now moves the star to the center
//*	Sep  7,	2021	<MLS> Created windowtab_starlist.cpp
//*	Sep  7,	2021	<MLS> Star list working for Messier objects
//*	Sep 12,	2021	<MLS> Merged aavso list with starlist
//*	Sep 14,	2021	<MLS> Added export to CSV file (not finished)
//*	Oct 23,	2021	<MLS> Added SetColumnOneTitle()
//*	Oct 26,	2021	<MLS> Added magnitude to star list display
//*	Mar 28,	2022	<MLS> Now we copy the data so we can sort properly
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"windowtab.h"
#include	"windowtab_starlist.h"

static int	gSortColumn_StarList;
static bool	gInvertSort_StarList	=	false;

//**************************************************************************************
WindowTabStarList::WindowTabStarList(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
										:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;
	cStarListPtr		=	NULL;
	cStarListCount		=	0;

	SetupWindowControls(windowName);

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabStarList::~WindowTabStarList(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	//*	dispose of the data
	if (cStarListPtr != NULL)
	{
		CONSOLE_DEBUG("Releasing memory for star list");
		free(cStarListPtr);
		cStarListPtr		=	NULL;
		cStarListCount		=	0;
	}
}

//**************************************************************************************
void	WindowTabStarList::SetupWindowControls(const char *argWindowName)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{100, 400, 550, 700, 900, 950, 0, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kStarList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kStarList_Title, argWindowName);
	SetBGcolorFromWindowName(kStarList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc	=	1;
	iii				=	kStarList_ClmTitle1;
	while (iii <= kStarList_ClmTitle6)
	{
		clmnHdrWidth	=	tabArray[iii - kStarList_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,		yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kStarList_ClmTitle1];
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kStarList_ClmTitle1,	"AlertID");
	SetWidgetText(		kStarList_ClmTitle2,	"Star Name");
	SetWidgetText(		kStarList_ClmTitle3,	"Right Ascension");
	SetWidgetText(		kStarList_ClmTitle4,	"Declination");
	SetWidgetText(		kStarList_ClmTitle5,	"Org RA/DEC");
	SetWidgetText(		kStarList_ClmTitle6,	"Mag");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	5;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	textBoxWd	-=	20;			//*	leave room for the scroll bar
	for (iii = kStarList_Obj_01; iii <= kStarList_Obj_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBorder(		iii,	false);
//		SetWidgetBorder(		iii,	true);
//		SetWidgetNumber(		iii,	(iii - kStarList_Obj_01));

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}
	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(		kStarList_ScrollBar,	(xLoc + textBoxWd + 2),		yLoc2,		15,		600);
	SetWidgetType(	kStarList_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(	kStarList_ScrollBar,	kFont_Small);
//	CONSOLE_DEBUG_W_NUM("kStarList_ScrollBar\t=", kStarList_ScrollBar);

	//---------------------------------------------------------------------
	xLoc		=	0;
	widgetWidth	=	cWidth / 4;
	SetWidget(				kStarList_Obj_Total,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kStarList_Obj_Total,	kFont_Medium);
	SetWidgetText(			kStarList_Obj_Total,	"Total alerts =?");
	SetWidgetJustification(	kStarList_Obj_Total,	kJustification_Left);
	SetWidgetTextColor(		kStarList_Obj_Total,	CV_RGB(255,	255,	255));




	xLoc		+=	widgetWidth;
	xLoc		+=	2;
	SetWidget(				kStarList_ExportCSV,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetType(			kStarList_ExportCSV,	kWidgetType_Button);
	SetWidgetFont(			kStarList_ExportCSV,	kFont_Medium);
	SetWidgetText(			kStarList_ExportCSV,	"Export to CSV");
	SetWidgetJustification(	kStarList_ExportCSV,	kJustification_Center);
	SetWidgetBGColor(		kStarList_ExportCSV,	CV_RGB(255,	255,	255));

//	SetAlpacaLogoBottomCorner(kStarList_AlpacaLogo);

}

//*****************************************************************************
//void	WindowTabStarList::RunWindowBackgroundTasks(void)
//{


//}

//*****************************************************************************
void	WindowTabStarList::ProcessButtonClick(const int buttonIdx, const int flags)
{
int	newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kStarList_ClmTitle1:
		case kStarList_ClmTitle2:
		case kStarList_ClmTitle3:
		case kStarList_ClmTitle4:
		case kStarList_ClmTitle5:
		case kStarList_ClmTitle6:
			newSortColumn	=	buttonIdx - kStarList_ClmTitle1;
			if (newSortColumn == cSortColumn)
			{
				gInvertSort_StarList	=	!gInvertSort_StarList;
			}
			else
			{
				gInvertSort_StarList	=	false;
			}
			cSortColumn		=	newSortColumn;
			UpdateSortOrder();
			ForceWindowUpdate();
			break;

		case kStarList_ExportCSV:
			if ((cStarListPtr != NULL) && (cStarListCount > 0))
			{
			int		iii;

				for (iii=0; iii<cStarListCount; iii++)
				{
					printf("%ld\t%s",	cStarListPtr[iii].id,
										cStarListPtr[iii].longName);
					printf("\r\n");
				}
			}
			break;

	}
}

//*****************************************************************************
void	WindowTabStarList::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
int		screenLineNum;
int		starDataIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
	//*	check to see if it is one of the entries
	if ((widgetIdx >= kStarList_Obj_01) && (widgetIdx <= kStarList_Obj_Last))
	{
		if (cStarListPtr != NULL)
		{
			screenLineNum	=	widgetIdx - kStarList_Obj_01;
			starDataIdx		=	screenLineNum + cFirstLineIdx;
			CONSOLE_DEBUG_W_NUM("starDataIdx\t=", starDataIdx);
			if ((starDataIdx >= 0) && (starDataIdx < cStarListCount))
			{
				CONSOLE_DEBUG_W_STR("Double clicked on: ", cStarListPtr[starDataIdx].shortName);
				CONSOLE_DEBUG_W_STR("Double clicked on: ", cStarListPtr[starDataIdx].longName);
				Center_CelestralObject(&cStarListPtr[starDataIdx]);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("invalid starDataIdx\t=", starDataIdx);
				CONSOLE_ABORT(__FUNCTION__);
			}
		}
	}
}

//*****************************************************************************
void	WindowTabStarList::ProcessMouseWheelMoved(	const int	widgetIdx,
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

//*****************************************************************************
void	WindowTabStarList::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
//uint32_t			currentMillis;
//uint32_t			deltaMilliSecs;
//int					newSliderValue_int;

	CONSOLE_DEBUG(__FUNCTION__);
//	currentMillis	=	millis();

	switch(widgetIdx)
	{
		case kStarList_ScrollBar:
		//	deltaMilliSecs	=	currentMillis - cLastExposureUpdate_Millis;
		//	if (deltaMilliSecs > 4)
			{
		//		cLastExposureUpdate_Millis	=	millis();
			}
			break;
	}
	ForceWindowUpdate();
}

//**************************************************************************************
void	WindowTabStarList::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[256];
char	formatString[64];
int		myDevCount;
int		starDataIdx;
short	raHours;
short	raMinutes;
double	raSecs;
short	declDegress;
short	declMinutes;
double	declSecs;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii			=	0;
	starDataIdx	=	cFirstLineIdx;
	myDevCount	=	0;
	boxId		=	0;
	//*	check to make sure we have valid data
	if (cStarListPtr != NULL)
	{
		while (	(iii <= (kStarList_Obj_Last - kStarList_Obj_01)) &&
				(starDataIdx < cStarListCount))
		{
			boxId	=	iii + kStarList_Obj_01;
			switch(cStarListPtr[starDataIdx].dataSrc)
			{
				case kDataSrc_AAVSOalert:
				case kDataSrc_NGC2000:
				case kDataSrc_NGC2000IC:
				case kDataSrc_OpenNGC:
					sprintf(textString, "%ld\t%s",	cStarListPtr[starDataIdx].id,
													cStarListPtr[starDataIdx].longName);
					break;

				default:
					sprintf(textString, "%s\t%s",
											cStarListPtr[starDataIdx].shortName,
											cStarListPtr[starDataIdx].longName);
					break;
			}

			//---------------------------------------------------
			//*	print out the RA/DEC values
			ConvertRadiansToDegMinSec((cStarListPtr[starDataIdx].ra / 15),
									&raHours,
									&raMinutes,
									&raSecs);

			ConvertRadiansToDegMinSec(cStarListPtr[starDataIdx].decl,
									&declDegress,
									&declMinutes,
									&declSecs);

			sprintf(formatString, "\t%3d:%02d:%04.2f\t%3d:%02d:%04.2f",
															raHours,
															raMinutes,
															raSecs,
															declDegress,
															declMinutes,
															declSecs);
			strcat(textString, formatString);
			//---------------------------------------------------
			//*	now do the ORIGINAL RA/DEC
			ConvertRadiansToDegMinSec((cStarListPtr[starDataIdx].org_ra / 15),
									&raHours,
									&raMinutes,
									&raSecs);

			ConvertRadiansToDegMinSec(cStarListPtr[starDataIdx].org_decl,
									&declDegress,
									&declMinutes,
									&declSecs);

			sprintf(formatString, "\t%3d:%02d:%04.1f/%3d:%02d:%04.1f",
															raHours,
															raMinutes,
															raSecs,
															declDegress,
															declMinutes,
															declSecs);

			strcat(textString, formatString);
//			CONSOLE_DEBUG(textString);
			//*	add the magnitude
			sprintf(formatString, "\t%1.1f", cStarListPtr[starDataIdx].realMagnitude);
			strcat(textString, formatString);

			SetWidgetText(boxId, textString);

			myDevCount++;
			starDataIdx++;
			iii++;
		}
		//*	now set the rest of them to empty strings
		while (iii <= (kStarList_Obj_Last - kStarList_Obj_01))
		{
			boxId	=	iii + kStarList_Obj_01;
			SetWidgetText(boxId, "");
			iii++;
		}
	}
	else
	{
	//	CONSOLE_ABORT(__FUNCTION__);
	}

	sprintf(textString, "Total stars =%d", cStarListCount);
	SetWidgetText(	kStarList_Obj_Total,	textString);

	SetWidgetScrollBarLimits(	kStarList_ScrollBar, (kStarList_Obj_Last - kStarList_Obj_01), cStarListCount);
	SetWidgetScrollBarValue(	kStarList_ScrollBar, 100);

	SetWidgetBGColor(kStarList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kStarList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kStarList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kStarList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kStarList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kStarList_ClmTitle6,	((cSortColumn == 5) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}

//**************************************************************************************
void	WindowTabStarList::SetStarDataPointers(TYPE_CelestData *argStarList, int argStarListCount)
{
size_t	memorySize;
	CONSOLE_DEBUG(__FUNCTION__);

	if ((argStarList != NULL) && (argStarListCount > 0))
	{
		cStarListPtr	=	(TYPE_CelestData *)calloc((argStarListCount + 5), sizeof(TYPE_CelestData));
		if (cStarListPtr != NULL)
		{
			memorySize		=	argStarListCount * sizeof(TYPE_CelestData);
			memcpy(cStarListPtr, argStarList, memorySize);
			cStarListCount	=	argStarListCount;
			cDataSource		=	cStarListPtr[0].dataSrc;
		}
	}
	UpdateOnScreenWidgetList();
}

//**************************************************************************************
void	WindowTabStarList::SetColumnOneTitle(const char *clmOneTitle)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (clmOneTitle != NULL)
	{
		SetWidgetText(		kStarList_ClmTitle1,	clmOneTitle);
	}
}

//**************************************************************************************
static  int StarDataSortProc(const void *e1, const void *e2)
{
TYPE_CelestData	*obj1, *obj2;
int				returnValue;

	obj1		=	(TYPE_CelestData *)e1;
	obj2		=	(TYPE_CelestData *)e2;

	returnValue	=	0;
	switch(gSortColumn_StarList)
	{
		case 0:
			returnValue	=	strcasecmp(obj1->shortName, obj2->shortName);
			break;

		case 1:
			returnValue	=	strcasecmp(obj1->longName, obj2->longName);
			break;

		case 2:
			if (obj2->ra > obj1->ra)
			{
				returnValue	=	-1;
			}
			else if (obj2->ra < obj1->ra)
			{
				returnValue	=	1;
			}
			break;

		case 3:
			if (obj2->decl > obj1->decl)
			{
				returnValue	=	-1;
			}
			else if (obj2->decl < obj1->decl)
			{
				returnValue	=	1;
			}
			break;

		case 5:
			if (obj2->realMagnitude > obj1->realMagnitude)
			{
				returnValue	=	-1;
			}
			else if (obj2->realMagnitude < obj1->realMagnitude)
			{
				returnValue	=	1;
			}
			break;
	}
	//*	if they are the same, sort by ID (alert ID)
	if (returnValue == 0)
	{
		returnValue	=	obj1->id - obj2->id;
	}
	if (gInvertSort_StarList)
	{
		returnValue	=	-returnValue;
	}
	return(returnValue);
}

//**************************************************************************************
static  int StarDataSortProc_Messier(const void *e1, const void *e2)
{
TYPE_CelestData	*obj1, *obj2;
int				returnValue;
int				m_num1;
int				m_num2;
	obj1		=	(TYPE_CelestData *)e1;
	obj2		=	(TYPE_CelestData *)e2;

	returnValue	=	0;
	switch(gSortColumn_StarList)
	{
		case 0:
		//	returnValue	=	strcasecmp(obj1->shortName, obj2->shortName);
			m_num1		=	atoi(&obj1->shortName[1]);
			m_num2		=	atoi(&obj2->shortName[1]);
			returnValue	=	m_num1 - m_num2;
			break;

		case 1:
			returnValue	=	strcasecmp(obj1->longName, obj2->longName);
			break;

		case 2:
			if (obj2->ra > obj1->ra)
			{
				returnValue	=	-1;
			}
			else if (obj2->ra < obj1->ra)
			{
				returnValue	=	1;
			}
			break;

		case 3:
			if (obj2->decl > obj1->decl)
			{
				returnValue	=	-1;
			}
			else if (obj2->decl < obj1->decl)
			{
				returnValue	=	1;
			}
			break;
	}
	//*	if they are the same, sort by ID (alert ID)
	if (returnValue == 0)
	{
		returnValue	=	obj1->id - obj2->id;
	}
	if (gInvertSort_StarList)
	{
		returnValue	=	-returnValue;
	}
	return(returnValue);
}


//**************************************************************************************
void	WindowTabStarList::UpdateSortOrder(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevCnt\t=", cAlpacaDevCnt);

	if (cStarListPtr != NULL)
	{
		if (cSortColumn >= 0)
		{
			gSortColumn_StarList	=	cSortColumn;
			switch(cDataSource)
			{
				case kDataSrc_Messier:
					qsort(	cStarListPtr,
							cStarListCount,
							sizeof(TYPE_CelestData),
							StarDataSortProc_Messier);
					break;

				default:
					qsort(	cStarListPtr,
							cStarListCount,
							sizeof(TYPE_CelestData),
							StarDataSortProc);
					break;

			}

			UpdateOnScreenWidgetList();
		}
	}
}

