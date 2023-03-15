//*****************************************************************************
//*		windowtab_alpacaUnit.cpp		(c) 2020 by Mark Sproul
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
//*	Dec  1,	2022	<MLS> Created windowtab_alpacaUnit.cpp
//*****************************************************************************

#include	<fitsio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"cpu_stats.h"

#include	"windowtab.h"
#include	"windowtab_alpacaUnit.h"
#include	"controller.h"
#include	"controller_alpacaUnit.h"


#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#define	kAlpacaUnitHeight	100



//**************************************************************************************
WindowAlpacaUnit::WindowAlpacaUnit(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cColor_CPU		=	LLD_GetColor(W_MAGENTA);
	cColor_Camera	=	LLD_GetColor(W_YELLOW);
	cColor_Focuser	=	LLD_GetColor(W_GREEN);

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowAlpacaUnit::~WindowAlpacaUnit(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}




//**************************************************************************************
void	WindowAlpacaUnit::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		yLocSave;
int		graphHeight;
int		textBoxHt;
int		textBoxWd;
int		iii;
short	tabArray[kMaxTabStops]	=	{200, 400, 600, 1000, 1199, 0};

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kAlpacaUnit_Title, -1, yLoc, "AlpacaPi project");

	SetAlpacaLogoBottomCorner(kAlpacaUnit_AlpacaLogo);

	//------------------------------------------
	xLoc		=	5;
	graphHeight	=	175;

	SetWidget(			kAlpacaUnit_CPUtempGraph,	xLoc,	yLoc,	kMaxCPUtempEntries,	graphHeight);
	SetWidgetType(		kAlpacaUnit_CPUtempGraph,	kWidgetType_CustomGraphic);

	yLocSave	=	yLoc;

	//------------------------------------------
	//*	Legend on right side
	xLoc		=	5 + kMaxCPUtempEntries + 5;
	for (iii=kAlpacaUnit_Legend_CPU; iii<=kAlpacaUnit_Legend_Focuser; iii++)
	{
		SetWidget(				iii,	xLoc,	yLoc,	cClmWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_MultiLineText);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetBorder(		iii,	false);		//*	turn the border off

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetTextColor(		kAlpacaUnit_Legend_CPU,		cColor_CPU);
	SetWidgetTextColor(		kAlpacaUnit_Legend_Camera,	cColor_Camera);
	SetWidgetTextColor(		kAlpacaUnit_Legend_Focuser,	cColor_Focuser);

	SetWidgetText(		kAlpacaUnit_Legend_CPU,		"CPU temp");
	SetWidgetText(		kAlpacaUnit_Legend_Camera,	"Camera temp");
	SetWidgetText(		kAlpacaUnit_Legend_Focuser,	"Focuser Temp");

	SetWidgetValid(		kAlpacaUnit_Legend_Camera,	false);
	SetWidgetValid(		kAlpacaUnit_Legend_Focuser,	false);

	yLoc			=	yLocSave;
	yLoc			+=	graphHeight;
	yLoc			+=	2;


	//------------------------------------------------------------
	//*	this is the list of text boxes for the device list
	xLoc		=	2;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 5);
	for (iii=kAlpacaUnit_AlpacaDev_01; iii < kAlpacaUnit_Outline; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}
	SetWidgetOutlineBox(kAlpacaUnit_Outline, kAlpacaUnit_AlpacaDev_01, (kAlpacaUnit_Outline - 1));

	//-------------------------------------------------------------
	//*	the box for the remote cpu info
	textBoxHt		=	cHeight - yLoc;
	textBoxHt		-=	1;

	SetWidget(				kAlpacaUnit_CPUinfo,	0,			yLoc,		cWidth,		textBoxHt);
	SetWidgetType(			kAlpacaUnit_CPUinfo,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kAlpacaUnit_CPUinfo,	kJustification_Left);
	SetWidgetFont(			kAlpacaUnit_CPUinfo,	kFont_Medium);
	SetWidgetTextColor(		kAlpacaUnit_CPUinfo,	CV_RGB(255,	255,	255));

	yLoc			+=	textBoxHt;
	yLoc			+=	2;

	SetWidgetText(kAlpacaUnit_CPUinfo, "REMOTE DEVICE: data not available");
}


//**************************************************************************************
#ifdef _USE_OPENCV_CPP_
void	WindowAlpacaUnit::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
void	WindowAlpacaUnit::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
ControllerAlpacaUnit	*myParentCtrlPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kAlpacaUnit_CPUtempGraph:
//			DrawCpuTempGraph(&cWidgetList[widgetIdx]);
			myParentCtrlPtr	=	(ControllerAlpacaUnit *)cParentObjPtr;
			if (myParentCtrlPtr != NULL)
			{
				if (myParentCtrlPtr->cMagicCookie == kMagicCookieValue)
				{
					//*	draw the temperature graph lines
					if (myParentCtrlPtr->cHasCamera && (myParentCtrlPtr->cCameraTempCnt > 0))
					{
						SetWidgetValid(kAlpacaUnit_Legend_Camera,	true);
						LLD_SetColor(W_YELLOW);
						DrawGraph(	&cWidgetList[widgetIdx],
									myParentCtrlPtr->cCameraTempCnt,
									myParentCtrlPtr->cCameraTempLog, false, 2);
					}
					if (myParentCtrlPtr->cHasFocuser && (myParentCtrlPtr->cFocusTempCnt > 0))
					{
						SetWidgetValid(kAlpacaUnit_Legend_Focuser,	true);
						LLD_SetColor(W_GREEN);
						DrawGraph(	&cWidgetList[widgetIdx],
									myParentCtrlPtr->cFocusTempCnt,
									myParentCtrlPtr->cFocusTempLog, false, 2);
					}
					if (myParentCtrlPtr->cCPUTtempCnt > 0)
					{
						LLD_SetColor(W_MAGENTA);
						DrawGraph(		&cWidgetList[widgetIdx],
										myParentCtrlPtr->cCPUTtempCnt,
										myParentCtrlPtr->cCPUtempLog, true, 2);
					}
				}
				else
				{
					CONSOLE_DEBUG("Stack corruption !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
				}
			}
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

