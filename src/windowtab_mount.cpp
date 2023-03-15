//*****************************************************************************
//*		windowtab_mount.cpp		(c) 2022 by Mark Sproul
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
//*	Jun  2,	2022	<MLS> Created windowtab_mount.cpp
//*	Jun  3,	2022	<MLS> Added horizontal adjusting and scrolling
//*	Jun  3,	2022	<MLS> Added help messages
//*	Jun  3,	2022	<MLS> Added graphs of RA and DEC
//*	Jun  4,	2022	<MLS> Added DrawOneGraphSegment()
//*	Jun  6,	2022	<MLS> Added HandleKeyDown() to mount graph window
//*****************************************************************************

#if defined(_ENABLE_SKYTRAVEL_) && !defined(__ARM_ARCH)

#include	<stdbool.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_mount.h"
#include	"controller.h"
#include	"MountData.h"

//**************************************************************************************
WindowTabMount::WindowTabMount(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cVerticalMagnification		=	1;
	cHorizontalMagnification	=	1;
	cCurrentHorzOffset			=	0;
	cDisplayAverage				=	false;
	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMount::~WindowTabMount(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabMount::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		graphHeight;
int		boxWidth;
int		buttonWidth;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kMount_Title, -1, yLoc, "RA/DEC/HA tracking");

	graphHeight		=	cHeight - cTabVertOffset;
	graphHeight		-=	cBtnHeight;
	graphHeight		-=	150;
//	CONSOLE_DEBUG_W_NUM("graphHeight\t=", graphHeight);

	SetWidget(		kMount_Graph,		0,			yLoc,		cWidth,		graphHeight);
	SetWidgetType(	kMount_Graph, kWidgetType_CustomGraphic);

	//*	save the vertical center for drawing the graph
	cGraphRectTop		=	yLoc;
	cGraphRectBottom	=	yLoc + graphHeight;
	cDefaultVertCenter	=	yLoc + (graphHeight / 2);
	cCurrentVertCenter	=	cDefaultVertCenter;

	yLoc			+=	graphHeight;
	yLoc			+=	2;

	//==========================================
	SetWidget(				kMount_LegendTextBox,	0,			yLoc,		cWidth,		cBtnHeight);
	SetWidgetType(			kMount_LegendTextBox,	kWidgetType_TextBox);
	SetWidgetBorder(		kMount_LegendTextBox,	false);
	SetWidgetFont(			kMount_LegendTextBox,	kFont_RadioBtn);
	SetWidgetTextColor(		kMount_LegendTextBox,	CV_RGB(255, 255, 255));
	SetWidgetJustification(	kMount_LegendTextBox,	kJustification_Left);

	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//==========================================
	xLoc	=	5;
	iii		=	kMount_Reset;
	while (iii < kMount_ScaleInfoTextBox)
	{
		switch(iii)
		{
			case kMount_Reset:
			case kMount_Clear:
				buttonWidth	=	2 * cBtnHeight;
				break;
			default:
				buttonWidth	=	cBtnHeight;
				break;
		}
		SetWidget(			iii,	xLoc,			yLoc,		buttonWidth,		cBtnHeight);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(0,	0,		0));
		xLoc	+=	buttonWidth;
		xLoc	+=	3;

		iii++;
	}
	SetWidgetText(		kMount_VertMag_Plus,	"V+");
	SetWidgetText(		kMount_VertMag_Minus,	"V-");
	SetWidgetText(		kMount_HorzMag_Plus,	"H+");
	SetWidgetText(		kMount_HorzMag_Minus,	"H-");
	SetWidgetText(		kMount_Reset,			"Reset");
	SetWidgetText(		kMount_Clear,			"Clear");

	SetWidgetType(		kMount_VertMag_Value,	kWidgetType_TextBox);
	SetWidgetFont(		kMount_VertMag_Value,	kFont_RadioBtn);
	SetWidgetBGColor(	kMount_VertMag_Value,	CV_RGB(0,	0,		0));
	SetWidgetTextColor(	kMount_VertMag_Value,	CV_RGB(255,	0,		0));

	SetWidgetType(		kMount_HorzMag_Value,	kWidgetType_TextBox);
	SetWidgetFont(		kMount_HorzMag_Value,	kFont_RadioBtn);
	SetWidgetBGColor(	kMount_HorzMag_Value,	CV_RGB(0,	0,		0));
	SetWidgetTextColor(	kMount_HorzMag_Value,	CV_RGB(255,	0,		0));

	SetWidgetHelpText(kMount_Reset,			"Reset display parameters");
	SetWidgetHelpText(kMount_VertMag_Plus,	"Increase vertical magnification");
	SetWidgetHelpText(kMount_VertMag_Minus,	"Decrease vertical magnification");
	SetWidgetHelpText(kMount_VertMag_Value,	"Current vertical magnification");

	SetWidgetHelpText(kMount_HorzMag_Plus,	"Compress horizontal scale");
	SetWidgetHelpText(kMount_HorzMag_Minus,	"De-Compress horizontal scale");
	SetWidgetHelpText(kMount_HorzMag_Value,	"Current horizontal scale");
	SetWidgetHelpText(kMount_Clear,			"Clear all data");

	//=======================================================
	boxWidth	=	cWidth - xLoc;
	SetWidget(		kMount_ScaleInfoTextBox,	xLoc,	yLoc,		boxWidth,		cBtnHeight);
	SetWidgetType(	kMount_ScaleInfoTextBox,	kWidgetType_TextBox);
	SetWidgetFont(	kMount_ScaleInfoTextBox,	kFont_TextList);
	SetWidgetTextColor(	kMount_ScaleInfoTextBox,	CV_RGB(255,	255, 255));
	SetWidgetJustification(	kMount_ScaleInfoTextBox, kJustification_Left);

	SetHelpTextBoxNumber(kMount_HelpMsgTextBox);

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kMount_IPaddr,
							kMount_Readall,
							kMount_HelpMsgTextBox,
							-1,		//	kMount_LastCmdString,
							-1,		//	kMount_AlpacaLogo,
							-1);
	SetWidgetText(kMount_HelpMsgTextBox, "");

	UpdateButtons();
}

//*****************************************************************************
void	WindowTabMount::HandleKeyDown(const int keyPressed)
{
bool	forceUpdate;
int		yPixelValue;
int		loopCount;
int		vertStepAmount;
int		minimuY;
int		maximumY;

//	CONSOLE_DEBUG_W_HEX(__FUNCTION__, keyPressed);

	forceUpdate	=	true;
	switch(keyPressed & 0x0fff)
	{
		case 'a':
			CONSOLE_DEBUG_W_HEX(__FUNCTION__, keyPressed);
			cDisplayAverage	=	cDisplayAverage ? false : true;
			break;

		case 'h':	//*	center on hour angle
			//*	find the most recent value
			CONSOLE_DEBUG_W_DBL("cCurrentHAvalue\t=", cCurrentHAvalue);
			yPixelValue			=	0;
			loopCount			=	0;
			cCurrentVertCenter	=	cDefaultVertCenter;
			vertStepAmount		=	cVerticalMagnification / 2;
			minimuY				=	cGraphRectTop + 75;
			maximumY			=	cGraphRectBottom - 75;
			if (vertStepAmount < 1)
			{
				vertStepAmount	=	1;
			}
			if (vertStepAmount > 300)
			{
				vertStepAmount	=	300;
			}
			CONSOLE_DEBUG_W_NUM("minimuY  \t=", minimuY);
			CONSOLE_DEBUG_W_NUM("maximumY \t=", maximumY);
			while ((yPixelValue < minimuY ) || (yPixelValue > maximumY))
			{
				yPixelValue	=	cCurrentVertCenter - (cVerticalMagnification * cCurrentHAvalue);

				CONSOLE_DEBUG_W_NUM("cCurrentVertCenter\t=", cCurrentVertCenter);
				CONSOLE_DEBUG_W_NUM("yPixelValue       \t=", yPixelValue);

				if (yPixelValue < minimuY)
				{
					cCurrentVertCenter	+=	vertStepAmount;
				}
				else if (yPixelValue > maximumY)
				{
					cCurrentVertCenter	-=	vertStepAmount;
				}

				loopCount++;
				if (loopCount > 200)
				{
					break;
				}
			}
			CONSOLE_DEBUG_W_NUM("loopCount\t=", loopCount);
			break;

		default:
			forceUpdate	=	false;
			break;
	}

	if (forceUpdate)
	{
		ForceWindowUpdate();
	}
}

//       EVENT_FLAG_CTRLKEY   = 8, //!< indicates that CTRL Key is pressed.
//       EVENT_FLAG_SHIFTKEY  = 16,//!< indicates that SHIFT Key is pressed.
//       EVENT_FLAG_ALTKEY    = 32 //!< indicates that ALT Key is pressed.

//*****************************************************************************
void	WindowTabMount::ProcessButtonClick(const int buttonIdx, const int flags)
{
int		deltaMag;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
//	CONSOLE_DEBUG_W_HEX("flags\t=", flags);
	if (flags & cv::EVENT_FLAG_CTRLKEY)
	{
		deltaMag	=	50;
	}
	else if (flags & cv::EVENT_FLAG_SHIFTKEY)
	{
		deltaMag	=	20;
	}
	else
	{
		deltaMag	=	1;
	}

	switch(buttonIdx)
	{
		case kMount_Reset:
			cVerticalMagnification		=	1;
			cHorizontalMagnification	=	1;
			cCurrentHorzOffset			=	0;
			cCurrentVertCenter			=	cDefaultVertCenter;
			break;

		case kMount_VertMag_Plus:
			cVerticalMagnification	+=	deltaMag;
			break;

		case kMount_VertMag_Minus:
			cVerticalMagnification	-=	deltaMag;
			if (cVerticalMagnification < 1)
			{
				cVerticalMagnification	=	1;
			}
			break;

		case kMount_HorzMag_Plus:
			cHorizontalMagnification	+=	1;
			break;

		case kMount_HorzMag_Minus:
			cHorizontalMagnification	-=	1;
			if (cHorizontalMagnification < 1)
			{
				cHorizontalMagnification	=	1;
			}
			break;

		case kMount_Clear:	//*	reset all data to 0
			MountData_Init();
			break;

		default:
			CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
			break;
	}
	UpdateButtons();
	ForceWindowUpdate();
}

//*****************************************************************************
//*	this routine can be overloaded
void	WindowTabMount::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	WindowTabMount::UpdateButtons(void)
{
char	scaleText[128];
double	vScale;
double	hScale;

	SetWidgetNumber(	kMount_VertMag_Value,	cVerticalMagnification);
	SetWidgetNumber(	kMount_HorzMag_Value,	cHorizontalMagnification);

	hScale	=	4.0 * cHorizontalMagnification;
	if (cVerticalMagnification >= 200)
	{
		vScale	=	3600.0 / cVerticalMagnification;
		sprintf(scaleText, "Vert: 1 px = %3.2f arc-seconds | Horz: 1 px =%5.3f seconds", vScale, hScale);
	}
	else if (cVerticalMagnification >= 10)
	{
		vScale	=	60.0 / cVerticalMagnification;
		sprintf(scaleText, "Vert: 1 px = %5.3f arc-minutes | Horz: 1 px =%5.3f seconds", vScale, hScale);
	}
	else
	{
		vScale	=	1.0 / cVerticalMagnification;
		sprintf(scaleText, "Vert: 1 px = %5.3f deg | Horz: 1 px =%5.3f seconds", vScale, hScale);
	}
	SetWidgetText(	kMount_ScaleInfoTextBox,	scaleText);
}


//*****************************************************************************
void	WindowTabMount::ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags)
{
int		movementAmount;

	//*	check to see if the control key is down
	if (flags & cv::EVENT_FLAG_CTRLKEY)
	{
		movementAmount	=	40 * wheelMovement;
	}
	else
	{
		movementAmount	=	5 * wheelMovement;
	}

	if (flags & cv::EVENT_FLAG_SHIFTKEY)
	{
		cCurrentHorzOffset	+=	10 * movementAmount;
		if (cCurrentHorzOffset < 0)
		{
			cCurrentHorzOffset	=	0;
		}
	}
	else
	{
		cCurrentVertCenter	-=	movementAmount;
	}
	ForceWindowUpdate();
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabMount::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabMount::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
//**************************************************************************************
{
	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kMount_Graph:
			DrawRA_DEC_HA_Graph(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

#define	TRANSLATE_Y(rect, yyy)	((rect->y + rect->height - 4) - yyy)

//**************************************************************************************
void	WindowTabMount::DrawTickLine(cv::Rect *widgetRect, int yLoc)
{
char		tickLable[16];
cv::Scalar	lineColor;
int			xx;
int			maxXX;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

	if (yLoc == 0)
	{
		LLD_SetColor(W_RED);
	}
	else
	{
		LLD_SetColor(W_WHITE);
	}
#define	kDashWidth	4
	xx		=	5;
	maxXX	=	widgetRect->width - 40;
	pt1_X	=	xx;
	pt1_Y	=	cCurrentVertCenter - (yLoc * cVerticalMagnification);
	pt2_X	=	xx;
	pt2_Y	=	cCurrentVertCenter - (yLoc * cVerticalMagnification);
	//*	make sure we are still within the box

	if ((pt1_Y > cGraphRectTop) && (pt1_Y < cGraphRectBottom))
	{
		while (xx < maxXX)
		{
			pt1_X	=	xx;
			pt2_X	=	xx + kDashWidth;
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);

			xx	+=	(5 * kDashWidth);
		}

		pt2_X	+=	2;
		pt2_Y	+=	5;
		sprintf(tickLable, "%3d", yLoc);
		LLD_SetColor(W_MAGENTA);
		LLD_DrawCString(pt2_X, pt2_Y, tickLable, kFont_Medium);
	}
}

//**************************************************************************************
void	WindowTabMount::DrawOneGraphSegment(	TYPE_WIDGET		*theWidget,
												const double	*dataArray,
												const int		firstDataPtIdx,
												const char		*labelString)
{
cv::Point	pt1;
cv::Point	pt2;
int			iii;
int			previousX;
int			previousY;
int			xLoc;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
bool		okToDraw;
int			rightMostPoint;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, labelString);
//	CONSOLE_DEBUG_W_NUM("firstDataPtIdx\t=", firstDataPtIdx);
//	CONSOLE_DEBUG_W_NUM("cHorizontalMagnification\t=", cHorizontalMagnification);

	iii				=	firstDataPtIdx;
	previousX		=	theWidget->left;
	previousY		=	cCurrentVertCenter;
	rightMostPoint	=	theWidget->left + theWidget->width;
	xLoc			=	theWidget->left;
	while ((xLoc < rightMostPoint) && (iii < kMaxMountData))
	{
		if (dataArray[iii] != 0.0)
		{
			pt1_X			=	previousX;
			pt1_Y			=	previousY;
			pt2_X			=	xLoc;
			pt2_Y			=	cCurrentVertCenter - (cVerticalMagnification * dataArray[iii]);

			//*	lets check to make sure the line is on the screen
			okToDraw	=	true;
			if ((pt1_X > rightMostPoint) && (pt2_X > rightMostPoint))
			{
				okToDraw	=	false;
			}
			if ((pt1_Y > cGraphRectBottom) && (pt2_Y > cGraphRectBottom))
			{
				okToDraw	=	false;
			}
			if ((pt1_Y < cGraphRectTop) && (pt2_Y < cGraphRectTop))
			{
				okToDraw	=	false;
			}
			if (okToDraw)
			{
				LLD_MoveTo(pt1_X, pt1_Y);
				LLD_LineTo(pt2_X, pt2_Y);
			}
			previousX		=	xLoc;
			previousY		=	pt2_Y;
		}
		xLoc++;
		iii	+=	cHorizontalMagnification;
	}
	LLD_DrawCString(previousX + 2, previousY, labelString, kFont_RadioBtn);
}


//**************************************************************************************
void	WindowTabMount::DrawWidgetCustomGraphicAvg(	TYPE_WIDGET		*theWidget,
												const double	*dataArray,
												const int		firstDataPtIdx,
												const int		numPtsToAverage)
{
cv::Point	pt1;
cv::Point	pt2;
int			iii;
int			jjj;
int			previousX;
int			previousY;
int			xLoc;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
bool		okToDraw;
int			rightMostPoint;
double		valueSum;
double		averageValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii				=	firstDataPtIdx;
	previousX		=	theWidget->left;
	previousY		=	cCurrentVertCenter;
	rightMostPoint	=	theWidget->left + theWidget->width;
	xLoc			=	theWidget->left;
	while ((xLoc < rightMostPoint) && (iii < kMaxMountData))
	{
		if (dataArray[iii] != 0.0)
		{
			//*	compute the average of the previous n values
			if ((iii - firstDataPtIdx) > numPtsToAverage)
			{
				valueSum	=	0.0;
				for (jjj=0; jjj<numPtsToAverage; jjj++)
				{
					valueSum	+=	dataArray[iii - jjj];
				}
				averageValue	=	valueSum / numPtsToAverage;
			}
			else
			{
				averageValue	=	dataArray[iii];
			}
			pt1_X			=	previousX;
			pt1_Y			=	previousY;
			pt2_X			=	xLoc;
			pt2_Y			=	cCurrentVertCenter - (cVerticalMagnification * averageValue);

			//*	lets check to make sure the line is on the screen
			okToDraw	=	true;
			if ((pt1_X > rightMostPoint) && (pt2_X > rightMostPoint))
			{
				okToDraw	=	false;
			}
			if ((pt1_Y > cGraphRectBottom) && (pt2_Y > cGraphRectBottom))
			{
				okToDraw	=	false;
			}
			if ((pt1_Y < cGraphRectTop) && (pt2_Y < cGraphRectTop))
			{
				okToDraw	=	false;
			}
			if (okToDraw)
			{
				LLD_MoveTo(pt1_X, pt1_Y);
				LLD_LineTo(pt2_X, pt2_Y);
			}
			previousX		=	xLoc;
			previousY		=	pt2_Y;
		}
		xLoc++;
		iii	+=	cHorizontalMagnification;
	}
}

//**************************************************************************************
void	WindowTabMount::DrawRA_DEC_HA_Graph(TYPE_WIDGET *theWidget)
{
cv::Rect	myCVrect;
cv::Point	pt1;
cv::Point	pt2;
int			iii;
int			yLoc;
int			firstDataPtIdx;
int			tickLineDelta;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	//=========================================================
	//*	draw tick mark lines
	if (cVerticalMagnification >= 60)
	{
		tickLineDelta	=	1;
	}
	else if (cVerticalMagnification >= 50)
	{
		tickLineDelta	=	2;
	}
	else if (cVerticalMagnification >= 25)
	{
		tickLineDelta	=	5;
	}
	else if (cVerticalMagnification >= 10)
	{
		tickLineDelta	=	10;
	}
	else if (cVerticalMagnification >= 5)
	{
		tickLineDelta	=	25;
	}
	else
	{
		tickLineDelta	=	50;
	}
	yLoc	=	-150;
	while (yLoc <= 180)
	{
		DrawTickLine(&myCVrect, yLoc);
		yLoc	+=	tickLineDelta;
	}
	DrawTickLine(&myCVrect, 0);
	DrawTickLine(&myCVrect, 180);
	DrawTickLine(&myCVrect, -180);

	//*	first go through and find the first non-zero data point
	firstDataPtIdx	=	-1;
	iii				=	0;
	while ((firstDataPtIdx < 0) && (iii <kMaxMountData))
	{
		if (gHourAngleData[iii] != 0.0)
		{
			firstDataPtIdx	=	iii;
			cCurrentHAvalue	=	gHourAngleData[iii];
		}
		iii++;
	}
	//*	adjust for the horizontal scrolling
	firstDataPtIdx	+=	cCurrentHorzOffset;

	//----------------------------------------------------------------
	//*	set up to draw the Hour Angle Line
	//*	draw the averages first, so that the real line goes on top
	if (cDisplayAverage)
	{
		LLD_SetColor(W_LIGHTGRAY);
		DrawWidgetCustomGraphicAvg(theWidget,	gHourAngleData,		firstDataPtIdx,	50);
	}
	LLD_SetColor(W_GREEN);
	DrawOneGraphSegment(theWidget,	gHourAngleData,		firstDataPtIdx,	"HA");

	//----------------------------------------------------------------
	//*	set up to draw the Declination Line
	LLD_SetColor(W_LIGHTMAGENTA);
	DrawOneGraphSegment(theWidget,	gDeclinationData,	firstDataPtIdx,	"Dec");

	//----------------------------------------------------------------
	//*	set up to draw the RA Line
	LLD_SetColor(W_ORANGE);
	DrawOneGraphSegment(theWidget,	gRightAsceData,		firstDataPtIdx,	"RA");
}

#endif // defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)

