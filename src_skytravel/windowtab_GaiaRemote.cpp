//*****************************************************************************
//*		windowtab_GaiaRemote.cpp		(c) 2022 by Mark Sproul
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
//*	Jan  9,	2022	<MLS> Created windowtab_GaiaRemote.cpp
//*	Jan 10,	2022	<MLS> Gaia remote list window fully working
//*****************************************************************************
#ifdef _ENABLE_REMOTE_SQL_

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"windowtab.h"
#include	"windowtab_GaiaRemote.h"
#include	"GaiaSQL.h"
#include	"RemoteGaia.h"


//**************************************************************************************
WindowTabGaiaRemote::WindowTabGaiaRemote(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
											:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

//	cSortColumn			=	-1;
	cFirstLineIdx		=	0;

	SetupWindowControls(windowName);

	UpdateOnScreenWidgetList();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabGaiaRemote::~WindowTabGaiaRemote(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabGaiaRemote::SetupWindowControls(const char *argWindowName)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
int		clmnHdr_xLoc;
int		clmnHdrWidth;
int		boxID;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kGaiaRemoteList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kGaiaRemoteList_Title, argWindowName);
	SetBGcolorFromWindowName(kGaiaRemoteList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc	=	1;
	iii				=	kGaiaRemoteList_ClmTitle1;
	while (iii <= kGaiaRemoteList_ClmTitle10)
	{
		clmnHdrWidth	=	tabArray[iii - kGaiaRemoteList_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,		yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kGaiaRemoteList_ClmTitle1];
		clmnHdr_xLoc	+=	2;


		iii++;
	}

	boxID	=	kGaiaRemoteList_ClmTitle1;
	SetWidgetText(		boxID++,	"Index");
	SetWidgetText(		boxID++,	"Valid");
	SetWidgetText(		boxID++,	"RA-Deg");
	SetWidgetText(		boxID++,	"RA-hrs");
	SetWidgetText(		boxID++,	"Declination");
	SetWidgetText(		boxID++,	"Star Count");
	SetWidgetText(		boxID++,	"time (ms)");
	SetWidgetText(		boxID++,	"Seq#");
	SetWidgetText(		boxID++,	"TimeStamp");
	SetWidgetText(		boxID++,	"Distance");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	5;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	textBoxWd	-=	20;			//*	leave room for the scroll bar
	for (iii = kGaiaRemoteList_Obj_01; iii <= kGaiaRemoteList_Average; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBorder(		iii,	false);
//		SetWidgetBorder(		iii,	true);
//		SetWidgetNumber(		iii,	(iii - kGaiaRemoteList_Obj_01));

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}
	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(		kGaiaRemoteList_ScrollBar,	(xLoc + textBoxWd + 2),		yLoc2,		15,		600);
	SetWidgetType(	kGaiaRemoteList_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(	kGaiaRemoteList_ScrollBar,	kFont_Small);
//	CONSOLE_DEBUG_W_NUM("kGaiaRemoteList_ScrollBar\t=", kGaiaRemoteList_ScrollBar);

	xLoc		=	0;
	//---------------------------------------------------------------------
	//*	set up the text message box
	widgetWidth	=			(3 * ((cWidth / 4) + 20)) + 12;
	SetWidget(				kGaiaRemoteList_MsgBox,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kGaiaRemoteList_MsgBox,	kFont_Medium);
	SetWidgetText(			kGaiaRemoteList_MsgBox,	"------------------");
	SetWidgetJustification(	kGaiaRemoteList_MsgBox,	kJustification_Left);
	SetWidgetTextColor(		kGaiaRemoteList_MsgBox,	CV_RGB(255,	255,	255));
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	3;

	//---------------------------------------------------------------------
	widgetWidth	=	(cWidth / 4) + 20;
	SetWidget(				kGaiaRemoteList_Obj_Total,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kGaiaRemoteList_Obj_Total,	kFont_Medium);
	SetWidgetText(			kGaiaRemoteList_Obj_Total,	"Total alerts =?");
	SetWidgetJustification(	kGaiaRemoteList_Obj_Total,	kJustification_Left);
	SetWidgetTextColor(		kGaiaRemoteList_Obj_Total,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	6;


	SetWidget(				kGaiaRemoteList_ErrorCnt,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kGaiaRemoteList_ErrorCnt,	kFont_Medium);
	SetWidgetText(			kGaiaRemoteList_Obj_Total,	"Error count =0");
	SetWidgetJustification(	kGaiaRemoteList_ErrorCnt,	kJustification_Left);
	SetWidgetTextColor(		kGaiaRemoteList_ErrorCnt,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	6;


	SetWidget(				kGaiaRemoteList_ClearButton,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetType(			kGaiaRemoteList_ClearButton,	kWidgetType_Button);
	SetWidgetFont(			kGaiaRemoteList_ClearButton,	kFont_Medium);
	SetWidgetText(			kGaiaRemoteList_ClearButton,	"Clear All data");
	SetWidgetTextColor(		kGaiaRemoteList_ClearButton,	CV_RGB(0,	0,	0));
	SetWidgetBGColor(		kGaiaRemoteList_ClearButton,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

	SetWidgetTextColor(	kGaiaRemoteList_Average,	CV_RGB(0,	255,	0));

//	SetAlpacaLogoBottomCorner(kGaiaRemoteList_AlpacaLogo);

}

//*****************************************************************************
//void	WindowTabGaiaRemote::RunWindowBackgroundTasks(void)
//{


//}

//*****************************************************************************
void	WindowTabGaiaRemote::ProcessButtonClick(const int buttonIdx, const int flags)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kGaiaRemoteList_ClmTitle1:
		case kGaiaRemoteList_ClmTitle2:
		case kGaiaRemoteList_ClmTitle3:
		case kGaiaRemoteList_ClmTitle4:
		case kGaiaRemoteList_ClmTitle5:
		case kGaiaRemoteList_ClmTitle6:
			break;

		case kGaiaRemoteList_ClearButton:
			ClearAllSQLdata();
			break;

	}
	UpdateOnScreenWidgetList();
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabGaiaRemote::ProcessDoubleClick(	const int	widgetIdx,
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
	if ((widgetIdx >= kGaiaRemoteList_Obj_01) && (widgetIdx <= kGaiaRemoteList_Obj_Last))
	{
		screenLineNum	=	widgetIdx - kGaiaRemoteList_Obj_01;
		starDataIdx		=	screenLineNum + cFirstLineIdx;
		CONSOLE_DEBUG_W_NUM("starDataIdx\t=", starDataIdx);
		if ((starDataIdx >= 0) && (starDataIdx < kMaxGaiaDataSets))
		{
			Center_CelestralObject(&gGaiaDataList[starDataIdx].gaiaData[0]);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("invalid starDataIdx\t=", starDataIdx);
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
}

//*****************************************************************************
void	WindowTabGaiaRemote::ProcessMouseWheelMoved(	const int	widgetIdx,
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
void	WindowTabGaiaRemote::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{

	CONSOLE_DEBUG(__FUNCTION__);

	switch(widgetIdx)
	{
		case kGaiaRemoteList_ScrollBar:
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
void	WindowTabGaiaRemote::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[256];
char	formatString[64];
int		myDevCount;
int		gaiaIdx;
int		totalStarCnt;
int		validEntryCnt;
int		totalMilliSecs;
int		avgMilliSecs;
int		avgStarCnt;
int		ra_hours;
int		ra_minutes;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii			=	0;
	gaiaIdx		=	cFirstLineIdx;
	boxId		=	0;

	while (	(iii <= (kGaiaRemoteList_Obj_Last - kGaiaRemoteList_Obj_01)) &&
			(gaiaIdx < kMaxGaiaDataSets))
	{
		boxId	=	iii + kGaiaRemoteList_Obj_01;

		sprintf(textString, "%2d\t%c\t",	gaiaIdx,
											(gGaiaDataList[gaiaIdx].validData ? 'V' : '-'));

		sprintf(formatString, "%d\t",			gGaiaDataList[gaiaIdx].block_RA_deg);
			strcat(textString, formatString);

//		sprintf(formatString, "%1.2f\t",		gGaiaDataList[gaiaIdx].centerRA_deg/15.0);
//			strcat(textString, formatString);

		ra_hours	=	gGaiaDataList[gaiaIdx].centerRA_deg/15.0;
		ra_minutes	=	((gGaiaDataList[gaiaIdx].centerRA_deg/15.0) - ra_hours) * 60;
		sprintf(formatString, "%2d:%02d\t",		ra_hours, ra_minutes);
			strcat(textString, formatString);

		sprintf(formatString, "%d\t",			gGaiaDataList[gaiaIdx].block_DEC_deg);
			strcat(textString, formatString);
		sprintf(formatString, "%d\t",			gGaiaDataList[gaiaIdx].gaiaDataCnt);
			strcat(textString, formatString);
		sprintf(formatString, "%d\t",			gGaiaDataList[gaiaIdx].elapsedMilliSecs);
			strcat(textString, formatString);
		sprintf(formatString, "%d\t",			gGaiaDataList[gaiaIdx].sequenceNum);
			strcat(textString, formatString);


		FormatTimeString_Local(&gGaiaDataList[iii].timeStamp, formatString);
			strcat(textString, formatString);

		sprintf(formatString, "\t%3.2f\t",		gGaiaDataList[gaiaIdx].distanceCtrScrn);
			strcat(textString, formatString);

		SetWidgetText(boxId, textString);

		if (gGaiaDataList[gaiaIdx].validData &&
			(gGaiaDataList[gaiaIdx].gaiaDataCnt > 0) &&
			(gGaiaDataList[gaiaIdx].gaiaData) != NULL)
		{
			SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	0));
		}
		else if (gGaiaDataList[gaiaIdx].validData)
		{
			SetWidgetTextColor(		boxId,	CV_RGB(255,	0,	0));
		}
		else
		{
			SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));
		}
		myDevCount++;
		gaiaIdx++;
		iii++;
	}
	//*	now set the rest of them to empty strings
	while (iii <= (kGaiaRemoteList_Obj_Last - kGaiaRemoteList_Obj_01))
	{
		boxId	=	iii + kGaiaRemoteList_Obj_01;
		SetWidgetText(boxId, "");
		iii++;
	}

	//*	count up the total
	totalStarCnt	=	0;
	validEntryCnt	=	0;
	totalMilliSecs	=	0;
	avgMilliSecs	=	0;
	avgStarCnt		=	0;
	for (iii=0; iii<kMaxGaiaDataSets; iii++)
	{
		totalStarCnt	+=	gGaiaDataList[iii].gaiaDataCnt;
		totalMilliSecs	+=	gGaiaDataList[iii].elapsedMilliSecs;

		if (gGaiaDataList[iii].validData && (gGaiaDataList[iii].gaiaDataCnt > 0))
		if (gGaiaDataList[iii].gaiaDataCnt > 0)
		{
			validEntryCnt++;
		}
	}
	if (validEntryCnt > 0)
	{
		avgMilliSecs	=	totalMilliSecs / validEntryCnt;
		avgStarCnt		=	totalStarCnt / validEntryCnt;
	}
	sprintf(textString, "avg\t\t\t\t\t%d\t%d", avgStarCnt, avgMilliSecs);
	SetWidgetText(kGaiaRemoteList_Average, textString);

	//------------------------------------------
	sprintf(textString, "Total stars =%d", totalStarCnt);
	if (totalStarCnt > 1000000)
	{
		sprintf(formatString, " (%1.1fm)", (totalStarCnt * 1.0) / 1000000.0);
		strcat(textString, formatString);
	}
	SetWidgetText(	kGaiaRemoteList_Obj_Total,	textString);

	sprintf(textString, "SQL Error count=%d", gSQLerror_Count);
	SetWidgetText(	kGaiaRemoteList_ErrorCnt,	textString);
	if (gSQLsever_MsgUpdated)
	{
		SetWidgetText(	kGaiaRemoteList_MsgBox,	gSQLsever_StatusMsg);
		gSQLsever_MsgUpdated	=	false;
	}
//
//	SetWidgetScrollBarLimits(	kGaiaRemoteList_ScrollBar, (kGaiaRemoteList_Obj_Last - kGaiaRemoteList_Obj_01), cStarListCount);
//	SetWidgetScrollBarValue(	kGaiaRemoteList_ScrollBar, 100);
//

	ForceWindowUpdate();

}

#endif // _ENABLE_REMOTE_SQL_
