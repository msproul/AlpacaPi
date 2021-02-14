//*****************************************************************************
//*		windowtab_graphs.cpp		(c) 2020 by Mark Sproul
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
//*	Feb 24,	2020	<MLS> Created windowtab_graphs.cpp
//*	Feb 24,	2020	<MLS> Added LogVoltage() & LogTemperature()
//*****************************************************************************



#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"windowtab_graphs.h"
#include	"alpacadriver_helper.h"






//**************************************************************************************
WindowTabGraph::WindowTabGraph(const int xSize, const int ySize, CvScalar backGrndColor)
	:WindowTab(xSize, ySize, backGrndColor)
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kMaxLogEnries; iii++)
	{
	//	cVoltageLog[iii]		=	15 + (sin(iii / 4.0) * 10);
	//	cTemperatureLog[iii]	=	15 + (sin(iii / 6.0) * 10);
		cVoltageLog[iii]		=	0;
		cTemperatureLog[iii]	=	0;
	}


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabGraph::~WindowTabGraph(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

#define	kGraphHeight	200
#define	kLabelHeight	25
//**************************************************************************************
void	WindowTabGraph::SetupWindowControls(void)
{
int		yLoc;

//	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kGraphBox_TempGraph,		0,		yLoc,		cWidth,		kGraphHeight);
	yLoc			+=	kGraphHeight;
	yLoc			+=	2;

	SetWidget(kGraphBox_TempLabel,		0,		yLoc,		cWidth,		kLabelHeight);
	yLoc			+=	kLabelHeight;
	yLoc			+=	2;

	SetWidget(kGraphBox_VoltageGraph,	0,		yLoc,		cWidth,		kGraphHeight);
	yLoc			+=	kGraphHeight;
	yLoc			+=	2;

	SetWidget(kGraphBox_VoltageLabel,	0,		yLoc,		cWidth,		kLabelHeight);
	yLoc			+=	kLabelHeight;
	yLoc			+=	2;

	SetWidgetType(kGraphBox_TempGraph,		kWidgetType_Graphic);
	SetWidgetType(kGraphBox_VoltageGraph,	kWidgetType_Graphic);

	SetWidgetText(kGraphBox_TempLabel,		"Temperature Log");
	SetWidgetFont(kGraphBox_TempLabel,		kFont_Medium);
	SetWidgetText(kGraphBox_VoltageLabel,	"Voltage Log");
	SetWidgetFont(kGraphBox_VoltageLabel,	kFont_Medium);

	SetAlpacaLogo(kGraphBox_AlpacaLogo, -1);
	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kGraphBox_IPaddr, kGraphBox_Readall, kGraphBox_AlpacaDrvrVersion, -1);

}

//**************************************************************************************
void	WindowTabGraph::DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx)
{
CvRect		myCVrect;

	myCVrect.x		=	cWidgetList[widgetIdx].left;
	myCVrect.y		=	cWidgetList[widgetIdx].top;
	myCVrect.width	=	cWidgetList[widgetIdx].width;
	myCVrect.height	=	cWidgetList[widgetIdx].height;


	cvRectangleR(	openCV_Image,
					myCVrect,
					cWidgetList[widgetIdx].bgColor,			//	CvScalar color,
					CV_FILLED,								//	int thickness CV_DEFAULT(1),
					8,										//	int line_type CV_DEFAULT(8),
					0);										//	int shift CV_DEFAULT(0));

//	cvRectangleR(	openCV_Image,
//					myCVrect,
//					cWidgetList[widgetIdx].boarderColor,	//	CvScalar color,
//					1,										//	int thickness CV_DEFAULT(1),
//					8,										//	int line_type CV_DEFAULT(8),
//					0);										//	int shift CV_DEFAULT(0));

	switch(widgetIdx)
	{
		case kGraphBox_TempGraph:
			DrawGraph(openCV_Image, &cWidgetList[widgetIdx], cTemperatureLog, kMaxLogEnries);
			break;

		case kGraphBox_VoltageGraph:
			DrawGraph(openCV_Image, &cWidgetList[widgetIdx], cVoltageLog, kMaxLogEnries);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

//**************************************************************************************
void	WindowTabGraph::DrawGraph(	IplImage	*openCV_Image,
									TYPE_WIDGET	*graphWidget,
									double		*arrayData,
									int			arrayCount)
{
CvPoint	pt1;
CvPoint	pt2;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("arrayCount\t=", arrayCount);

	for (iii=0; iii<arrayCount; iii++)
	{
		pt1.x	=	iii;
		pt1.y	=	graphWidget->top + graphWidget->height;

		pt2.x	=	iii;
		pt2.y	=	pt1.y - arrayData[iii];
		cvLine(	openCV_Image,
				pt1,
				pt2,
				CV_RGB(255,	0,	0),	//	CvScalar color,
				1,							//	int thickness CV_DEFAULT(1),
				8,							//	int line_type CV_DEFAULT(8),
				0);							//	int shift CV_DEFAULT(0));

	}
}


//**************************************************************************************
void	WindowTabGraph::LogVoltage(double voltageValue)
{
long	currMillsecs;
int		secondsSinceStart;
int		minutesSinceStart;
int		iii;

	currMillsecs		=	millis();
	secondsSinceStart	=	currMillsecs / 1000;
	minutesSinceStart	=	secondsSinceStart / 60;
	if (minutesSinceStart < kMaxLogEnries)
	{
		cVoltageLog[minutesSinceStart]	=	voltageValue * 4;
	}
	else
	{
		//*	move everything back one
		for (iii=0; iii<(kMaxLogEnries-1); iii++)
		{
			cVoltageLog[iii]	=	cVoltageLog[iii + 1];
		}
		cVoltageLog[kMaxLogEnries-1]	=	voltageValue * 4;
	}
}

//**************************************************************************************
void	WindowTabGraph::LogTemperature(double temp_degC)
{
long	currMillsecs;
int		secondsSinceStart;
int		minutesSinceStart;
int		iii;

	currMillsecs		=	millis();
	secondsSinceStart	=	currMillsecs / 1000;
	minutesSinceStart	=	secondsSinceStart / 60;

	if (minutesSinceStart < kMaxLogEnries)
	{
		cTemperatureLog[minutesSinceStart]	=	temp_degC * 3;
	}
	else
	{
		//*	move everything back one
		for (iii=0; iii<(kMaxLogEnries-1); iii++)
		{
			cTemperatureLog[iii]	=	cTemperatureLog[iii + 1];
		}
		cTemperatureLog[kMaxLogEnries-1]	=	temp_degC * 3;
	}
}

