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
//*****************************************************************************


#if defined(_ENABLE_CTRL_DOME_) && defined(_ENABLE_SLIT_TRACKER_)
#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_slit.h"
#include	"controller.h"
#include	"controller_dome.h"



//**************************************************************************************
WindowTabSlitTracker::WindowTabSlitTracker(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
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
//	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabSlitTracker::SetupWindowControls(void)
{
int		yLoc;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kSlitTracker_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kSlitTracker_Title, "AlpacaPi Slit Tracker");
	SetBGcolorFromWindowName(kSlitTracker_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	//==========================================
	//*	create the clock face
	SetWidget(		kSlitTracker_SlitClockFace,	0,		yLoc,		(cWidth),	(cWidth));
	SetWidgetType(	kSlitTracker_SlitClockFace, kWidgetType_Graphic);
	yLoc			+=	cWidth;

	SetWidget(				kSlitTracker_LastUpdate,	2,		yLoc - cRadioBtnHt -2,	(cWidth / 4),	cRadioBtnHt);
	SetWidgetFont(			kSlitTracker_LastUpdate,	kFont_RadioBtn);
	SetWidgetBorder(		kSlitTracker_LastUpdate,	false);
	SetWidgetJustification(	kSlitTracker_LastUpdate,	kJustification_Left);



	yLoc			+=	2;

	yLoc			+=	2;

	//==========================================
	SetWidget(		kSlitTracker_RemoteAddress,	cClm3_offset+ 8,		yLoc,		(cClmWidth * 4),	cBtnHeight);
	SetWidgetText(	kSlitTracker_RemoteAddress,	"Slit tracker not available");
	SetWidgetFont(	kSlitTracker_RemoteAddress,	kFont_Medium);


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



	SetAlpacaLogo(kSlitTracker_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kSlitTracker_IPaddr, kSlitTracker_Readall, kSlitTracker_AlpacaDrvrVersion, -1);

	//=======================================================
	cDotColor[0]		=	CV_RGB(0,	255,	255);
	cDotColor[1]		=	CV_RGB(255,	0,		255);
	cDotColor[2]		=	CV_RGB(255,	255,	0);

	cDotColor[3]		=	CV_RGB(255,	0,		0);
	cDotColor[4]		=	CV_RGB(0,	255,	0);
	cDotColor[5]		=	CV_RGB(0,	0,		255);

}

//*****************************************************************************
void	WindowTabSlitTracker::ProcessButtonClick(const int buttonIdx)
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



//**************************************************************************************
void	WindowTabSlitTracker::DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx)
{
	switch(widgetIdx)
	{
		case kSlitTracker_SlitClockFace:
			DrawClockFace(openCV_Image, &cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}


//**************************************************************************************
void	WindowTabSlitTracker::DrawClockFace(IplImage *openCV_Image, TYPE_WIDGET *theWidget)
{
ControllerDome	*myDomeControllerObj;
CvRect			myCVrect;
CvPoint			myCVcenter;
int				radius1;
int				radius2;
double			degrees;
double			radians;
CvPoint			pt1;
CvPoint			pt2;
char			textString[32];
int				clockPosition;		//*	0	= 12 o'clock
double			distanceInches;
CvScalar		fillColor;
CvSize			textSize;
int				baseLine;


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

	myCVcenter.x	=	myCVrect.x + (myCVrect.width / 2);
	myCVcenter.y	=	myCVrect.y + (myCVrect.height / 2);


	//*	now draw a circle at each clock position
	radius1	=	(myCVrect.width / 2) - 45;
	radius2	=	(myCVrect.width / 2) - 100;
	for (clockPosition=0; clockPosition<12; clockPosition++)
	{
		if (gSlitDistance[clockPosition].validData)
		{
			distanceInches	=	gSlitDistance[clockPosition].distanceInches;
			if (distanceInches < 30.0)
			{
				fillColor	=	CV_RGB(255, 0, 0);
			}
			else if (distanceInches < 60.0)
			{
				fillColor	=	CV_RGB(255, 255, 0);
			}
			else
			{
				fillColor	=	CV_RGB(0, 255, 0);
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
				//	fillColor	=	CV_RGB(128, 0, 255);
					fillColor	=	CV_RGB(255, 255, 255);
					sprintf(textString, "%1.2f", distanceInches);
					break;
			}
		}
		else
		{
			fillColor	=	CV_RGB(128, 128, 128);
			strcpy(textString, "---");
		}
		cvGetTextSize(	textString,
						&gTextFont[kFont_Medium],
						&textSize,
						&baseLine);

		degrees	=	-90.0 + (clockPosition * 30.0);
		radians	=	degrees * M_PI / 180.0;
		pt1.x	=	myCVcenter.x + (cos(radians) * radius1);
		pt1.y	=	myCVcenter.y + (sin(radians) * radius1);
		pt2.x	=	myCVcenter.x + (cos(radians) * radius2);
		pt2.y	=	myCVcenter.y + (sin(radians) * radius2);
		cvCircle(	openCV_Image,
					pt1,
					40,
					fillColor,
					CV_FILLED,							//	int thickness CV_DEFAULT(1),
					8,									//	int line_type CV_DEFAULT(8),
					0);									//	int shift CV_DEFAULT(0));

		cvCircle(	openCV_Image,
					pt1,
					40,
					theWidget->borderColor,
					1,									//	int thickness CV_DEFAULT(1),
					8,									//	int line_type CV_DEFAULT(8),
					0);									//	int shift CV_DEFAULT(0));

		pt1.x	-=	(textSize.width / 2);
		pt1.y	+=	5;
		cvPutText(	openCV_Image,
					textString,
					pt1,
					&gTextFont[kFont_Medium],
					CV_RGB(0, 0, 0)
					);

		//*	this puts a little dot to indicate it recently got updated
		if (gSlitDistance[clockPosition].updated)
		{


			cvCircle(	openCV_Image,
						pt2,
						5,
						cDotColor[cUpdateColorIdx],
						CV_FILLED,							//	int thickness CV_DEFAULT(1),
						8,									//	int line_type CV_DEFAULT(8),
						0);									//	int shift CV_DEFAULT(0));
			gSlitDistance[clockPosition].updated	=	false;
		}

		degrees	+=	30.0;
	}
	cUpdateColorIdx++;
	if (cUpdateColorIdx >= kMaxDotColors)
	{
		cUpdateColorIdx	=	0;
	}

	SetWidgetChecked(kSlitTracker_RadioBtnSlit,		(cClockDisplayMode == kClockDisplay_Slit));
	SetWidgetChecked(kSlitTracker_RadioBtnCalib,	(cClockDisplayMode == kClockDisplay_Calib));

	//============================================================
	//*	draw the UP vector
	myDomeControllerObj	=	(ControllerDome *)cParentObjPtr;
	if (myDomeControllerObj != NULL)
	{
	double	upDegrees;
	double	upRadians;
	int		upAngleRadius	=	30;

		if (myDomeControllerObj->cValidGravity)
		{
			upDegrees	=	myDomeControllerObj->cUpAngle_Deg;
			upDegrees	=	360.0 - upDegrees;

			//*	the -90 is to make 0 degrees straight up
			upRadians	=	(upDegrees -90.0) * M_PI / 180.0;

			pt1.x	=	myCVcenter.x + (cos(upRadians) * upAngleRadius);
			pt1.y	=	myCVcenter.y + (sin(upRadians) * upAngleRadius);
			pt2.x	=	myCVcenter.x + (cos(upRadians) * radius2);
			pt2.y	=	myCVcenter.y + (sin(upRadians) * radius2);

			cvLine(	openCV_Image,
					pt1,
					pt2,
					CV_RGB(255, 0, 0),		//	CvScalar color,
					2,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));

			pt1.x	=	myCVcenter.x;
			pt1.y	=	myCVcenter.y;

			cvCircle(	openCV_Image,
						pt1,
						upAngleRadius,
						CV_RGB(255, 255, 255),
						1,									//	int thickness CV_DEFAULT(1),
						8,									//	int line_type CV_DEFAULT(8),
						0);									//	int shift CV_DEFAULT(0));

			while (upDegrees > 360.0)
			{
				upDegrees	-=	360.0;
			}
			sprintf(textString, "%1.1f", upDegrees);
			cvGetTextSize(	textString,
							&gTextFont[kFont_Medium],
							&textSize,
							&baseLine);
			pt1.x	-=	(textSize.width / 2);
			pt1.y	+=	5;
			cvPutText(	openCV_Image,
						textString,
						pt1,
						&gTextFont[kFont_Medium],
						CV_RGB(255, 255, 255)
						);
		}
	}
}


//**************************************************************************************
void	WindowTabSlitTracker::UpdateClockRadioBtns(void)
{
ControllerDome	*myDomeControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetChecked(kSlitTracker_RadioBtnSlit,		(cClockDisplayMode == kClockDisplay_Slit));
	SetWidgetChecked(kSlitTracker_RadioBtnCalib,	(cClockDisplayMode == kClockDisplay_Calib));


	myDomeControllerObj	=	(ControllerDome *)cParentObjPtr;
	if (myDomeControllerObj != NULL)
	{
		myDomeControllerObj->cUpdateWindow		=	true;
	}
}

//**************************************************************************************
void	WindowTabSlitTracker::ToggleLogData(void)
{
ControllerDome	*myDomeControllerObj;
bool			currentLogState;

//	CONSOLE_DEBUG(__FUNCTION__);

	myDomeControllerObj	=	(ControllerDome *)cParentObjPtr;
	if (myDomeControllerObj != NULL)
	{
		currentLogState	=	myDomeControllerObj->cLogSlitData;
		if (currentLogState)
		{
			//*	its on, stop the logging
			myDomeControllerObj->CloseSlitTrackerDataFile();
		}
		else
		{
			//*	its off, so turn it on
			myDomeControllerObj->cLogSlitData	=	true;
		}
		SetWidgetChecked(kSlitTracker_LogDataCheckBox,	myDomeControllerObj->cLogSlitData);

		myDomeControllerObj->cUpdateWindow		=	true;
	}
	else
	{
		CONSOLE_DEBUG("myDomeControllerObj is NULL");
	}
}


#endif // _ENABLE_CTRL_DOME_
