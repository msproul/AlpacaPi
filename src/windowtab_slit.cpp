//*****************************************************************************
//*		windowtab_slit.cpp		(c) 2020 by Mark Sproul
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
//*	May  1,	2020	<MLS> Created windowtab_slit.cpp
//*	May  1,	2020	<MLS> Slit display clock face working
//*	May  9,	2020	<MLS> Added ToggleLogData()
//*	May 31,	2020	<MLS> Added up vector from gravity data
//*	Jun 19,	2023	<MLS> Added DeviceState to slit window
//*****************************************************************************


//#if defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)
#if defined (_ENABLE_SLIT_TRACKER_)

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_slit.h"
#include	"controller.h"

#include	"controller_slit.h"

#include	"slittracker_data.h"

#ifdef _SLIT_TRACKER_DIRECT_
	#error "Slit Tracker Direct needs some work"
#endif


//**************************************************************************************
WindowTabSlitTracker::WindowTabSlitTracker(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cClockDisplayMode	=	kClockDisplay_Slit;
	cUpdateColorIdx		=	0;

	SetupWindowControls();

	UpdateClockRadioBtns();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSlitTracker::~WindowTabSlitTracker(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabSlitTracker::SetupWindowControls(void)
{
int		yLoc;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kSlitTracker_Title, kSlitTracker_Connected, yLoc, "AlpacaPi Slit Tracker");

	//==========================================
	//*	create the clock face
	SetWidget(		kSlitTracker_SlitClockFace,	0,		yLoc,		(cWidth),	(cWidth));
	SetWidgetType(	kSlitTracker_SlitClockFace, kWidgetType_CustomGraphic);
	yLoc			+=	cWidth;

	SetWidget(				kSlitTracker_LastUpdate,	2,		yLoc - cRadioBtnHt -2,	(cWidth / 4),	cRadioBtnHt);
	SetWidgetFont(			kSlitTracker_LastUpdate,	kFont_RadioBtn);
	SetWidgetBorder(		kSlitTracker_LastUpdate,	false);
	SetWidgetJustification(	kSlitTracker_LastUpdate,	kJustification_Left);

	yLoc			+=	2;
	yLoc			+=	2;

	//==========================================
	SetWidget(		kSlitTracker_RadioBtnSlit,	0,		yLoc,		(cWidth / 3),	cRadioBtnHt);
	SetWidgetText(	kSlitTracker_RadioBtnSlit,	"Slit");
	SetWidgetType(	kSlitTracker_RadioBtnSlit,	kWidgetType_RadioButton);
	SetWidgetFont(	kSlitTracker_RadioBtnSlit,	kFont_RadioBtn);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	SetWidget(		kSlitTracker_RadioBtnCalib,	0,		yLoc,		(cWidth / 3),	cRadioBtnHt);
	SetWidgetText(	kSlitTracker_RadioBtnCalib,	"Calibration");
	SetWidgetType(	kSlitTracker_RadioBtnCalib,	kWidgetType_RadioButton);
	SetWidgetFont(	kSlitTracker_RadioBtnCalib,	kFont_RadioBtn);
	SetWidgetOutlineBox(kSlitTracker_RadioBtnOutline, kSlitTracker_RadioBtnSlit, kSlitTracker_RadioBtnCalib);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	2;

	SetWidget(		kSlitTracker_LogDataCheckBox,	0,		yLoc,		(cWidth / 3),	cRadioBtnHt);
	SetWidgetText(	kSlitTracker_LogDataCheckBox,	"Log to disk");
	SetWidgetType(	kSlitTracker_LogDataCheckBox,	kWidgetType_CheckBox);
	SetWidgetFont(	kSlitTracker_LogDataCheckBox,	kFont_RadioBtn);


	//=======================================================
	cDotColor[0]		=	CV_RGB(0,	255,	255);
	cDotColor[1]		=	CV_RGB(255,	0,		255);
	cDotColor[2]		=	CV_RGB(255,	255,	0);

	cDotColor[3]		=	CV_RGB(255,	0,		0);
	cDotColor[4]		=	CV_RGB(0,	255,	0);
	cDotColor[5]		=	CV_RGB(0,	0,		255);

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kSlitTracker_IPaddr,
							kSlitTracker_Readall,
							kSlitTracker_DeviceState,
							kSlitTracker_AlpacaErrorMsg,
							kSlitTracker_LastCmdString,
							kSlitTracker_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabSlitTracker::ProcessButtonClick(const int buttonIdx, const int flags)
{
int	previousDispMode;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	previousDispMode	=	cClockDisplayMode;
	switch(buttonIdx)
	{
		case kSlitTracker_RadioBtnSlit:
			cClockDisplayMode	=	kClockDisplay_Slit;
		#ifdef _SLIT_TRACKER_DIRECT_
			SendSlitTrackerCmd("s");
		#endif // _SLIT_TRACKER_DIRECT_
			break;

		case kSlitTracker_RadioBtnCalib:
			cClockDisplayMode	=	kClockDisplay_Calib;
		#ifdef _SLIT_TRACKER_DIRECT_
			SendSlitTrackerCmd("f");
		#endif // _SLIT_TRACKER_DIRECT_
			break;


		case kSlitTracker_LogDataCheckBox:
			ToggleLogData();
			break;
	}
	if (cClockDisplayMode != previousDispMode)
	{
		UpdateClockRadioBtns();
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTabSlitTracker::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabSlitTracker::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kSlitTracker_SlitClockFace:
			DrawClockFace(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}


//**************************************************************************************
void	WindowTabSlitTracker::DrawClockFace(TYPE_WIDGET *theWidget)
{
ControllerSlit	*myControllerObj;
int				center_X;
int				center_Y;
int				radius1;
int				radius2;
double			degrees;
double			radians;
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
char			textString[32];
int				clockPosition;		//*	0	= 12 o'clock
double			distanceInches;
int				fillColor_Wnum;
int				textWidthPixels;


	center_X	=	theWidget->left + (theWidget->width / 2);
	center_Y	=	theWidget->top + (theWidget->height / 2);


	//*	now draw a circle at each clock position
	radius1	=	(theWidget->width / 2) - 45;
	radius2	=	(theWidget->width / 2) - 100;
	for (clockPosition=0; clockPosition<12; clockPosition++)
	{
		if (gSlitDistance[clockPosition].validData)
		{
			distanceInches	=	gSlitDistance[clockPosition].distanceInches;
			if (distanceInches < 30.0)
			{
				fillColor_Wnum	=	W_RED;
			}
			else if (distanceInches < 60.0)
			{
				fillColor_Wnum	=	W_YELLOW;
			}
			else
			{
				fillColor_Wnum	=	W_GREEN;
			}

			switch(cClockDisplayMode)
			{
				case kClockDisplay_Slit:
					if (distanceInches > 335.0)
					{
						strcpy(textString, "-INF-");
					}
					else
					{
						sprintf(textString, "%1.0f", distanceInches);
					}
					break;

				case kClockDisplay_Calib:
					fillColor_Wnum	=	W_WHITE;

					sprintf(textString, "%1.2f", distanceInches);
					break;
			}
		}
		else
		{
			fillColor_Wnum	=	W_LIGHTGRAY;
			strcpy(textString, "---");
		}

		degrees	=	-90.0 + (clockPosition * 30.0);
		radians	=	degrees * M_PI / 180.0;
		pt1_X	=	center_X + (cos(radians) * radius1);
		pt1_Y	=	center_Y + (sin(radians) * radius1);
		pt2_X	=	center_X + (cos(radians) * radius2);
		pt2_Y	=	center_Y + (sin(radians) * radius2);

#define	kRadius1	40
		LLG_SetColor(fillColor_Wnum);
		LLG_FillEllipse(pt1_X, pt1_Y, kRadius1, kRadius1);
		LLG_SetColor(W_WHITE);
		LLG_FrameEllipse(pt1_X, pt1_Y, kRadius1, kRadius1);

		textWidthPixels	=   LLG_GetTextSize(textString, kFont_Medium);
		pt1_X	-=	(textWidthPixels / 2);
		pt1_Y	+=	5;
		LLG_SetColor(W_BLACK);
		LLG_DrawCString(pt1_X, pt1_Y, textString, kFont_Medium);


		//*	this puts a little dot to indicate it recently got updated
		if (gSlitDistance[clockPosition].updated)
		{
			LLG_SetColor(cUpdateColorIdx);
			LLG_FillEllipse(pt2_X, pt2_Y, 5, 5);

			gSlitDistance[clockPosition].updated	=	false;
		}

		degrees	+=	30.0;
	}
	cUpdateColorIdx++;
	if (cUpdateColorIdx >= W_COLOR_LAST)
	{
		cUpdateColorIdx	=	0;
	}

	SetWidgetChecked(kSlitTracker_RadioBtnSlit,		(cClockDisplayMode == kClockDisplay_Slit));
	SetWidgetChecked(kSlitTracker_RadioBtnCalib,	(cClockDisplayMode == kClockDisplay_Calib));

	//============================================================
	//*	draw the UP vector
	myControllerObj	=	(ControllerSlit *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
	double	upDegrees;
	double	upRadians;
	int		upAngleRadius	=	30;

		if (myControllerObj->cValidGravity)
		{
			upDegrees	=	myControllerObj->cUpAngle_Deg;
			upDegrees	=	360.0 - upDegrees;

			//*	the -90 is to make 0 degrees straight up
			upRadians	=	(upDegrees -90.0) * M_PI / 180.0;

			pt1_X	=	center_X + (cos(upRadians) * upAngleRadius);
			pt1_Y	=	center_Y + (sin(upRadians) * upAngleRadius);
			pt2_X	=	center_X + (cos(upRadians) * radius2);
			pt2_Y	=	center_Y + (sin(upRadians) * radius2);
			LLG_SetColor(W_RED);
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt2_X, pt2_Y);

			pt1_X	=	center_X;
			pt1_Y	=	center_Y;

			LLG_SetColor(W_WHITE);
			LLG_FrameEllipse(pt1_X, pt1_Y, upAngleRadius, upAngleRadius);

			while (upDegrees > 360.0)
			{
				upDegrees	-=	360.0;
			}
			sprintf(textString, "%1.1f", upDegrees);
			textWidthPixels	=   LLG_GetTextSize(textString, kFont_Medium);
			pt1_X	-=	(textWidthPixels / 2);
			pt1_Y	+=	5;
			LLG_SetColor(W_WHITE);
			LLG_DrawCString(pt1_X, pt1_Y, textString, kFont_Medium);
		}
	}
	else
	{
		CONSOLE_DEBUG("myControllerObj is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}
}


//**************************************************************************************
void	WindowTabSlitTracker::UpdateClockRadioBtns(void)
{
ControllerSlit	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetChecked(kSlitTracker_RadioBtnSlit,		(cClockDisplayMode == kClockDisplay_Slit));
	SetWidgetChecked(kSlitTracker_RadioBtnCalib,	(cClockDisplayMode == kClockDisplay_Calib));

	myControllerObj	=	(ControllerSlit *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cUpdateWindow		=	true;
	}
	else
	{
		CONSOLE_DEBUG("myControllerObj is NULL");
//		CONSOLE_DEBUG_W_HEX("cParentObjPtr\t=", cParentObjPtr);
//		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
void	WindowTabSlitTracker::ToggleLogData(void)
{
bool			currentLogState;
	ControllerSlit	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(ControllerSlit *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		currentLogState	=	myControllerObj->cLogSlitData;
		if (currentLogState)
		{
			CONSOLE_DEBUG("Turning slit track logging OFF");
			//*	its on, stop the logging
			myControllerObj->CloseSlitTrackerDataFile();
		}
		else
		{
			//*	its off, so turn it on
			CONSOLE_DEBUG("Turning slit track logging ON");
			myControllerObj->cLogSlitData	=	true;
		}
		SetWidgetChecked(kSlitTracker_LogDataCheckBox,	myControllerObj->cLogSlitData);

		myControllerObj->cUpdateWindow		=	true;
	}
	else
	{
		CONSOLE_DEBUG("myControllerObj is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}
}


#endif // _ENABLE_SLIT_TRACKER_
