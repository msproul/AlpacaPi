//*****************************************************************************
//*		windowtab_slitgraph.cpp		(c) 2020 by Mark Sproul
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
//*	May 10,	2020	<MLS> Created windowtab_slitgraph.cpp
//*	May 10,	2020	<MLS> Added graph of slit distance values
//*	Feb 20,	2022	<MLS> Slitgraph working with OpenCV++
//*****************************************************************************

#if defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_slitgraph.h"
#include	"controller.h"
#include	"controller_dome.h"
#include	"slittracker_data.h"



//**************************************************************************************
WindowTabSlitGraph::WindowTabSlitGraph(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;
	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kSensorValueCnt; iii++)
	{
		cDisplayClockData[iii]	=	true;
	}

	iii	=	0;
	cSLitTrackColors[iii++]	=	CV_RGB(255,	0,		0),
	cSLitTrackColors[iii++]	=	CV_RGB(0,	255,	0),
	cSLitTrackColors[iii++]	=	CV_RGB(0,	0,		255),
	cSLitTrackColors[iii++]	=	CV_RGB(255,	255,	0),
	cSLitTrackColors[iii++]	=	CV_RGB(255,	0,		255),
	cSLitTrackColors[iii++]	=	CV_RGB(0,	255,	255),
	cSLitTrackColors[iii++]	=	CV_RGB(255,	128,	128),
	cSLitTrackColors[iii++]	=	CV_RGB(128,	255,	128),
	cSLitTrackColors[iii++]	=	CV_RGB(128,	64,		255),
	cSLitTrackColors[iii++]	=	CV_RGB(128,	128,	128),
	cSLitTrackColors[iii++]	=	CV_RGB(64,	255,	255),
	cSLitTrackColors[iii++]	=	CV_RGB(255,	64,		255),

	cDisplayRawData		=	true;
	cDisplayAvgData		=	true;

	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSlitGraph::~WindowTabSlitGraph(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabSlitGraph::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		iii;
int		checkBoxIdx;
int		save_yLoc;
int		graphHeight;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kSlitGraph_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kSlitGraph_Title, "AlpacaPi Slit Graph");
	SetBGcolorFromWindowName(kSlitGraph_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	graphHeight		=	350;
	SetWidget(		kSlitGraph_Graph,		0,			yLoc,		cWidth,		graphHeight);
	SetWidgetType(	kSlitGraph_Graph, kWidgetType_CustomGraphic);
	yLoc			+=	graphHeight;
	yLoc			+=	2;


	//==========================================
	checkBoxIdx	=	kSlitGraph_Clock00;
	save_yLoc	=	yLoc;
	xLoc		=	0;
	while (checkBoxIdx <kSlitGraph_Clock11)
	{
		SetWidget(			checkBoxIdx,		xLoc,			yLoc,		cClmWidth,		cRadioBtnHt);
		SetWidgetType(		checkBoxIdx,		kWidgetType_CheckBox);
		SetWidgetFont(		checkBoxIdx,		kFont_RadioBtn);
		SetWidgetNumber(	checkBoxIdx,		(checkBoxIdx - kSlitGraph_Clock00));
		SetWidgetChecked(	checkBoxIdx, true);
		checkBoxIdx++;
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;


		SetWidget(			checkBoxIdx,		xLoc,			yLoc,		cClmWidth,		cRadioBtnHt);
		SetWidgetType(		checkBoxIdx,		kWidgetType_CheckBox);
		SetWidgetFont(		checkBoxIdx,		kFont_RadioBtn);
		SetWidgetNumber(	checkBoxIdx,		(checkBoxIdx - kSlitGraph_Clock00));
		SetWidgetChecked(	checkBoxIdx, true);
		checkBoxIdx++;

		//*	reset for next column
		xLoc			+=	cClmWidth;
		yLoc			=	save_yLoc;
	}
	//*	now set the text colors
	for (iii=0; iii<kSensorValueCnt; iii++)
	{
		SetWidgetTextColor((kSlitGraph_Clock00 + iii), cSLitTrackColors[iii]);
	}
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	//==========================================
	SetWidget(			kSlitGraph_DisplayRaw,		0,			yLoc,		(cWidth / 2),		cRadioBtnHt);
	SetWidgetType(		kSlitGraph_DisplayRaw,		kWidgetType_CheckBox);
	SetWidgetFont(		kSlitGraph_DisplayRaw,		kFont_RadioBtn);
	SetWidgetText(		kSlitGraph_DisplayRaw,		"Display Raw Data");
	SetWidgetChecked(	kSlitGraph_DisplayRaw,		cDisplayRawData);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	//==========================================
	SetWidget(			kSlitGraph_DisplayAvg,		0,			yLoc,		(cWidth / 2),		cRadioBtnHt);
	SetWidgetType(		kSlitGraph_DisplayAvg,		kWidgetType_CheckBox);
	SetWidgetFont(		kSlitGraph_DisplayAvg,		kFont_RadioBtn);
	SetWidgetText(		kSlitGraph_DisplayAvg,		"Display Averaged Data");
	SetWidgetChecked(	kSlitGraph_DisplayAvg,		cDisplayAvgData);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kSlitGraph_IPaddr,
							kSlitGraph_Readall,
							kSlitGraph_AlpacaErrorMsg,
							kSlitGraph_LastCmdString,
							kSlitGraph_AlpacaLogo,
							-1);
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabSlitGraph::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabSlitGraph::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
//**************************************************************************************
{
	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kSlitGraph_Graph:
			DrawSlitGraph(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

#define	TRANSLATE_Y(rect, yyy)	((rect->y + rect->height - 4) - yyy)

//**************************************************************************************
//void	WindowTabSlitGraph::DrawTickLine(CvRect *widgetRect, int yLoc)
void	WindowTabSlitGraph::DrawTickLine(cv::Rect *widgetRect, int yLoc)
{
char		tickLable[16];
cv::Scalar	lineColor;
int			xx;
int			maxXX;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

	if (yLoc < 50)
	{
		cCurrentColor	=	CV_RGB(0, 255, 0);
	}
	else
	{
		cCurrentColor	=	CV_RGB(255, 255, 255);
	}
#define _DASHED_LINES_
#ifdef _DASHED_LINES_
#define	kDashWidth	4
	xx		=	5;
	maxXX	=	widgetRect->width - 40;
	pt1_X	=	xx;
	pt1_Y	=	TRANSLATE_Y(widgetRect, yLoc);
	pt2_X	=	xx;
	pt2_Y	=	TRANSLATE_Y(widgetRect, yLoc);
	while (xx < maxXX)
	{
		pt1_X	=	xx;
		pt2_X	=	xx + kDashWidth;
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);

		xx	+=	(5 * kDashWidth);
	}
#else
	pt1_X	=	5;
	pt1_Y	=	TRANSLATE_Y(widgetRect, yLoc);
	pt2_X	=	widgetRect->width - 40;
	pt2_Y	=	TRANSLATE_Y(widgetRect, yLoc);
	LLD_MoveTo(pt1_X, pt1_Y);
	LLD_LineTo(pt2_X, pt2_Y);

#endif // _DASHED_LINES_
	pt2_X	+=	2;
	pt2_Y	+=	5;
	sprintf(tickLable, "%3d", yLoc);
	LLD_DrawCString(pt2_X, pt2_Y, tickLable, kFont_Medium);
}

//**************************************************************************************
void	WindowTabSlitGraph::DrawSlitGraph(TYPE_WIDGET *theWidget)
{
cv::Rect	myCVrect;
cv::Point	pt1;
cv::Point	pt2;
char		textStr[32];
double		distanceInches;
double		previousDistance;
int			iii;
int			previousX;
int			yLoc;
int			clockIdx;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	//=========================================================
	//*	draw tick mark lines
	yLoc	=	50;
	while (yLoc < 325)
	{
		DrawTickLine(&myCVrect, yLoc);
		yLoc	+=	50;
	}
	//*	draw a special one at 30
	DrawTickLine(&myCVrect, 30);

	//=========================================================
	for (clockIdx=0; clockIdx<kSensorValueCnt; clockIdx++)
	{
		if (cDisplayClockData[clockIdx])
		{
			if (cDisplayRawData)
			{
				//=====================================
				//*	plot the raw data
				previousDistance	=	gSlitLog[0].distanceInches[clockIdx];
				previousX			=	2;
				for (iii=0; iii<kSlitLogCount; iii++)
				{
					if (gSlitLog[iii].validData)
					{
						distanceInches	=	gSlitLog[iii].distanceInches[clockIdx];

						pt1_X			=	previousX;
						pt1_Y			=	TRANSLATE_Y((&myCVrect), previousDistance);
						pt2_X			=	previousX + 1;
						pt2_Y			=	TRANSLATE_Y((&myCVrect), distanceInches);
						cCurrentColor	=	cSLitTrackColors[clockIdx];
						LLD_MoveTo(pt1_X, pt1_Y);
						LLD_LineTo(pt2_X, pt2_Y);

						previousX			=	pt2.x;
						previousDistance	=	distanceInches;
					}
				}
			}
			if (cDisplayAvgData)
			{
			cv::Scalar	avgColor;

				if (cDisplayRawData)
				{
					avgColor	=	CV_RGB(255, 255, 255);
				}
				else
				{
					avgColor	=	cSLitTrackColors[clockIdx];
				}

				//=====================================
				//*	plot the average
				previousDistance	=	gSlitLog[0].average20pt[clockIdx];
				previousX			=	2;
				for (iii=0; iii<kSlitLogCount; iii++)
				{
					if (gSlitLog[iii].validData)
					{
						distanceInches	=	gSlitLog[iii].average20pt[clockIdx];

						pt1_X			=	previousX;
						pt1_Y			=	TRANSLATE_Y((&myCVrect), previousDistance);
						pt2_X			=	previousX + 1;
						pt2_Y			=	TRANSLATE_Y((&myCVrect), distanceInches);
						cCurrentColor	=	cSLitTrackColors[clockIdx];
						LLD_MoveTo(pt1_X, pt1_Y);
						LLD_LineTo(pt2_X, pt2_Y);

						previousX			=	pt2.x;
						previousDistance	=	distanceInches;
					}
				}
			}
		}
	}
	//======================================================
	//*	put the number of samples in the bottom right corner
	sprintf(textStr, "samples=%d", gSlitLogIdx);

	cCurrentColor	=	CV_RGB(255, 255, 255);
	pt1_X	=	myCVrect.width - 100;
	pt1_Y	=	myCVrect.y + myCVrect.height - 10;

	LLD_DrawCString(pt1_X, pt1_Y, textStr, kFont_Small);
}


//*****************************************************************************
void	WindowTabSlitGraph::ProcessButtonClick(const int buttonIdx, const int flags)
{
int		clockIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	switch(buttonIdx)
	{
		case kSlitGraph_Clock00:		//*	00 is 12 o'clock
		case kSlitGraph_Clock01:
		case kSlitGraph_Clock02:
		case kSlitGraph_Clock03:
		case kSlitGraph_Clock04:
		case kSlitGraph_Clock05:
		case kSlitGraph_Clock06:
		case kSlitGraph_Clock07:
		case kSlitGraph_Clock08:
		case kSlitGraph_Clock09:
		case kSlitGraph_Clock10:
		case kSlitGraph_Clock11:
			clockIdx	=	buttonIdx - kSlitGraph_Clock00;
			if ((clockIdx >= 0) && (clockIdx < kSensorValueCnt))
			{
				cDisplayClockData[clockIdx]	=	!cDisplayClockData[clockIdx];
				SetWidgetChecked(buttonIdx, cDisplayClockData[clockIdx]);
				gUpdateSLitWindow	=	true;
			}
			break;

		case kSlitGraph_DisplayRaw:
			cDisplayRawData	=	!cDisplayRawData;
			SetWidgetChecked(	kSlitGraph_DisplayRaw,		cDisplayRawData);
			gUpdateSLitWindow	=	true;
			break;

		case kSlitGraph_DisplayAvg:
			cDisplayAvgData	=	!cDisplayAvgData;
			SetWidgetChecked(	kSlitGraph_DisplayAvg,		cDisplayAvgData);
			gUpdateSLitWindow	=	true;
			break;
	}
}

#endif // defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)

