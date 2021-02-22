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
//*****************************************************************************

#if defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)
#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_slitgraph.h"
#include	"controller.h"
#include	"controller_dome.h"



//**************************************************************************************
WindowTabSlitGraph::WindowTabSlitGraph(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
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
	SetWidgetType(	kSlitGraph_Graph, kWidgetType_Graphic);
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
	SetWidgetText(		kSlitGraph_DisplayRaw,		"DIsplay Raw Data");
	SetWidgetChecked(	kSlitGraph_DisplayRaw,		cDisplayRawData);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	//==========================================
	SetWidget(			kSlitGraph_DisplayAvg,		0,			yLoc,		(cWidth / 2),		cRadioBtnHt);
	SetWidgetType(		kSlitGraph_DisplayAvg,		kWidgetType_CheckBox);
	SetWidgetFont(		kSlitGraph_DisplayAvg,		kFont_RadioBtn);
	SetWidgetText(		kSlitGraph_DisplayAvg,		"DIsplay Averaged Data");
	SetWidgetChecked(	kSlitGraph_DisplayAvg,		cDisplayAvgData);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	//==========================================
//	SetWidget(		kSlitGraph_RemoteAddress,	cClm3_offset+ 8,		yLoc,		(cClmWidth * 4),	cBtnHeight);
//	SetWidgetText(	kSlitGraph_RemoteAddress,	"Remote address");
//	SetWidgetFont(	kSlitGraph_RemoteAddress,	kFont_Medium);



	SetAlpacaLogo(kSlitGraph_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kSlitGraph_IPaddr, kSlitGraph_Readall, kSlitGraph_AlpacaDrvrVersion, -1);

}



//**************************************************************************************
void	WindowTabSlitGraph::DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx)
{
	switch(widgetIdx)
	{
		case kSlitGraph_Graph:
			DrawSlitGraph(openCV_Image, &cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

#define	TRANSLATE_Y(rect, yyy)	((rect->y + rect->height - 4) - yyy)

//**************************************************************************************
void	WindowTabSlitGraph::DrawTickLine(IplImage *openCV_Image, CvRect *widgetRect, int yLoc)
{
CvPoint		pt1;
CvPoint		pt2;
char		tickLable[16];
CvScalar	lineColor;
int			xx;
int			maxXX;

	if (yLoc < 50)
	{
		lineColor	=	CV_RGB(0, 255, 0);
	}
	else
	{
		lineColor	=	CV_RGB(255, 255, 255);
	}
#define _DASHED_LINES_
#ifdef _DASHED_LINES_
#define	kDashWidth	4
	xx		=	5;
	maxXX	=	widgetRect->width - 40;
	pt1.y	=	TRANSLATE_Y(widgetRect, yLoc);
	pt2.y	=	TRANSLATE_Y(widgetRect, yLoc);
	while (xx < maxXX)
	{
		pt1.x	=	xx;
		pt2.x	=	xx + kDashWidth;

		cvLine(	openCV_Image,
				pt1,
				pt2,
				lineColor,				//	CvScalar color,
				1,						//	int thickness CV_DEFAULT(1),
				8,						//	int line_type CV_DEFAULT(8),
				0);						//	int shift CV_DEFAULT(0));
		xx	+=	(5 * kDashWidth);
	}
#else
	pt1.x	=	5;
	pt1.y	=	TRANSLATE_Y(widgetRect, yLoc);
	pt2.x	=	widgetRect->width - 40;
	pt2.y	=	TRANSLATE_Y(widgetRect, yLoc);

	cvLine(	openCV_Image,
			pt1,
			pt2,
			lineColor,				//	CvScalar color,
			1,						//	int thickness CV_DEFAULT(1),
			8,						//	int line_type CV_DEFAULT(8),
			0);						//	int shift CV_DEFAULT(0));

#endif // _DASHED_LINES_
	pt2.x	+=	2;
	pt2.y	+=	5;
	sprintf(tickLable, "%3d", yLoc);
	cvPutText(	openCV_Image,
				tickLable,
				pt2,
				&gTextFont[kFont_Medium],
				lineColor);

}

//**************************************************************************************
void	WindowTabSlitGraph::DrawSlitGraph(IplImage *openCV_Image, TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		pt1;
CvPoint		pt2;
char		textStr[32];
double		distanceInches;
double		previousDistance;
int			iii;
int			previousX;
int			yLoc;
int			clockIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	openCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));


	cvRectangleR(	openCV_Image,
					myCVrect,
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	//=========================================================
	//*	draw tick mark lines
	yLoc	=	50;
	while (yLoc < 325)
	{
		DrawTickLine(openCV_Image, &myCVrect, yLoc);
		yLoc	+=	50;
	}
	//*	draw a special one at 30
	DrawTickLine(openCV_Image, &myCVrect, 30);

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
						pt1.x			=	previousX;
						pt1.y			=	TRANSLATE_Y((&myCVrect), previousDistance);
						pt2.x			=	previousX + 1;
						pt2.y			=	TRANSLATE_Y((&myCVrect), distanceInches);

						cvLine(	openCV_Image,
								pt1,
								pt2,
								cSLitTrackColors[clockIdx],	//	CvScalar color,
								1,							//	int thickness CV_DEFAULT(1),
								8,							//	int line_type CV_DEFAULT(8),
								0);							//	int shift CV_DEFAULT(0));
						previousX			=	pt2.x;
						previousDistance	=	distanceInches;
					}
				}
			}
			if (cDisplayAvgData)
			{
			CvScalar	avgColor;

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
						pt1.x			=	previousX;
						pt1.y			=	TRANSLATE_Y((&myCVrect), previousDistance);
						pt2.x			=	previousX + 1;
						pt2.y			=	TRANSLATE_Y((&myCVrect), distanceInches);

						cvLine(	openCV_Image,
								pt1,
								pt2,
								avgColor,
								1,							//	int thickness CV_DEFAULT(1),
								8,							//	int line_type CV_DEFAULT(8),
								0);							//	int shift CV_DEFAULT(0));
						previousX			=	pt2.x;
						previousDistance	=	distanceInches;
					}
				}
			}
		}
	}
	//======================================================
	//*	put the number of samples in the bottom right corner
	pt1.x	=	myCVrect.width - 100;
	pt1.y	=	myCVrect.y + myCVrect.height - 10;
	sprintf(textStr, "samples=%d", gSlitLogIdx);
	cvPutText(	openCV_Image,
				textStr,
				pt1,
				&gTextFont[kFont_Small],
				CV_RGB(255, 255, 255));

}


//*****************************************************************************
void	WindowTabSlitGraph::ProcessButtonClick(const int buttonIdx)
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



#endif // _ENABLE_CTRL_DOME_

