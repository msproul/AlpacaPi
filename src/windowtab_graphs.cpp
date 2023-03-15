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

#include	"helper_functions.h"
#include	"windowtab_graphs.h"
#include	"alpacadriver_helper.h"


//**************************************************************************************
WindowTabGraph::WindowTabGraph(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor)
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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
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

	SetWidgetType(kGraphBox_TempGraph,		kWidgetType_CustomGraphic);
	SetWidgetType(kGraphBox_VoltageGraph,	kWidgetType_CustomGraphic);
	SetWidgetBorder(kGraphBox_TempGraph,	false);
	SetWidgetBorder(kGraphBox_VoltageGraph,	false);

	SetWidgetText(kGraphBox_TempLabel,		"Temperature Log");
	SetWidgetFont(kGraphBox_TempLabel,		kFont_Medium);
	SetWidgetText(kGraphBox_VoltageLabel,	"Voltage Log");
	SetWidgetFont(kGraphBox_VoltageLabel,	kFont_Medium);

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kGraphBox_IPaddr,
							kGraphBox_Readall,
							-1,		//	kGraphBox_AlpacaErrorMsg,
							-1,		//	kGraphBox_LastCmdString,
							kGraphBox_AlpacaLogo,
							-1);
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabGraph::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabGraph::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{

	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kGraphBox_TempGraph:
			DrawGraph(&cWidgetList[widgetIdx], cTemperatureLog, kMaxLogEnries);
			break;

		case kGraphBox_VoltageGraph:
			DrawGraph(&cWidgetList[widgetIdx], cVoltageLog, kMaxLogEnries);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

//**************************************************************************************
void	WindowTabGraph::DrawGraph(	TYPE_WIDGET	*graphWidget,
									double		*arrayData,
									int			arrayCount)
{
int		pt1_X;
int		pt1_Y;
int		pt2_X;
int		pt2_Y;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("arrayCount\t=", arrayCount);
	LLD_SetColor(W_RED);
	for (iii=0; iii<arrayCount; iii++)
	{
		pt1_X	=	iii;
		pt1_Y	=	graphWidget->top + graphWidget->height;

		pt2_X	=	iii;
		pt2_Y	=	pt1_Y - arrayData[iii];
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);
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

