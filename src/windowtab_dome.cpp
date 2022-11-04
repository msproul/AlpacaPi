//*****************************************************************************
//*		windowtab_dome.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 18,	2020	<MLS> Created windowtab_dome.cpp
//*	May  1,	2020	<MLS> Added shutter and slaved to dome control display
//*	May  1,	2020	<MLS> Added OpenShutter() & CloseShutter()
//*	May  4,	2020	<MLS> Slave mode set/unset logic working now
//*	May  7,	2020	<MLS> Added SendShutterCommand() to repleace multiple other cmds
//*	Jan 15,	2021	<MLS> Got clarification of SUPPORTEDACTIONS cmd, fixed hidden controls
//*	Jan 24,	2021	<MLS> Added ParentIsSkyTravel flag
//*	Jan 28,	2021	<MLS> Added -20,-10,-5,-1,+1,+5,+10,+20 buttons
//*	Jan 28,	2021	<MLS> Added MoveDomeByAmount()
//*	Jan 31,	2021	<MLS> Added graphical representation of dome position
//*****************************************************************************

#ifdef _ENABLE_CTRL_DOME_

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"windowtab_dome.h"

#ifdef _ENABLE_SKYTRAVEL_
	#include	"controller_skytravel.h"
#endif

#include	"controller_dome.h"

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabDome::WindowTabDome(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName,
								const bool	parentIsSkyTravel)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cParentIsSkyTravel	=	parentIsSkyTravel;
	cDomePropPtr		=	NULL;
	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabDome::~WindowTabDome(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabDome::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		btnWidth;
int		iii;
int		myButtonHt;
int		myPlusMinusBtnWidth;
int		my_cWidth;
//	CONSOLE_DEBUG(__FUNCTION__);

	my_cWidth	=	cWidth;
	if (cWidth > kDomeWindowWidth)
	{
		my_cWidth	=	kDomeWindowWidth;
		ComputeWidgetColumns(my_cWidth);
	}

	//============================================
	yLoc			=	cTabVertOffset;
	myButtonHt		=	cBtnHeight - 3;
	//============================================
	SetWidget(kDomeBox_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kDomeBox_Title, "AlpacaPi Dome");
	SetBGcolorFromWindowName(kDomeBox_Title);

	//*	setup the connected indicator
	SetUpConnectedIndicator(kDomeBox_Connected, yLoc);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

#ifdef _ENABLE_SKYTRAVEL_
int		domeGraphic_yLoc;
	domeGraphic_yLoc	=	yLoc;
#endif

	btnWidth		=	(cClmWidth * 3);

	//============================================
	SetWidget(			kDomeBox_CurPosLabel,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_CurPosLabel,	CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_CurPosLabel,	"Position");
	SetWidgetBorder(	kDomeBox_CurPosLabel,	false);

	SetWidget(			kDomeBox_CurPosition,	cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_CurPosition,	CV_RGB(0,	255,	0));
	yLoc			+=	myButtonHt;
	yLoc			+=	2;


	//============================================
	SetWidget(			kDomeBox_AzimLabel,		cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_AzimLabel,		CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_AzimLabel,		"Azimuth");
	SetWidgetBorder(	kDomeBox_AzimLabel,		false);

	SetWidget(			kDomeBox_Azimuth,		cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_Azimuth,		"Azimuth");
	SetWidgetTextColor(	kDomeBox_Azimuth,		CV_RGB(0,	255,	0));
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//============================================
	SetWidget(			kDomeBox_AltitudeLabel,		cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_AltitudeLabel,		CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_AltitudeLabel,		"Altitude");
	SetWidgetBorder(	kDomeBox_AltitudeLabel,		false);

	SetWidget(			kDomeBox_Altitude,			cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_Altitude,			"Altitude");
	SetWidgetTextColor(	kDomeBox_Altitude,			CV_RGB(0,	255,	0));
	yLoc			+=	myButtonHt;
	yLoc			+=	2;


	//============================================
	SetWidget(			kDomeBox_ShutterLabel,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_ShutterLabel,	"Shutter");
	SetWidgetTextColor(	kDomeBox_ShutterLabel,	CV_RGB(255,	255,	255));
	SetWidgetBorder(	kDomeBox_ShutterLabel,	false);

	SetWidget(			kDomeBox_ShutterStatus,	cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_ShutterStatus,	CV_RGB(0,	255,	0));
	SetWidgetText(		kDomeBox_ShutterStatus,	"---");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//============================================
	SetWidget(			kDomeBox_SlavedLabel,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_SlavedLabel,	"Slaved");
	SetWidgetTextColor(	kDomeBox_SlavedLabel,	CV_RGB(255,	255,	255));
	SetWidgetBorder(	kDomeBox_SlavedLabel,	false);

	SetWidget(			kDomeBox_SlavedStatus,	cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetTextColor(	kDomeBox_SlavedStatus,	CV_RGB(0,	255,	0));
	SetWidgetText(		kDomeBox_SlavedStatus,	"---");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 3);
	SetWidget(		kDomeBox_GoHome,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(	kDomeBox_GoHome, 	"Go Home");

	SetWidget(		kDomeBox_GoPark,	cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(	kDomeBox_GoPark, 	"Go Park");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//**************************************************************
	myPlusMinusBtnWidth	=	((my_cWidth - cClm1_offset) / 8) - 1;
	xLoc				=	cClm1_offset;
	for (iii=kDomeBox_Minus20; iii<=kDomeBox_Plus20; iii++)
	{
		SetWidget(				iii,	xLoc,		yLoc,	myPlusMinusBtnWidth,	myButtonHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	iii,	CV_RGB(0,	0,	0));
		xLoc	+=	myPlusMinusBtnWidth;
	}
	SetWidgetText(		kDomeBox_Minus20, 	"-20");
	SetWidgetText(		kDomeBox_Minus10, 	"-10");
	SetWidgetText(		kDomeBox_Minus5, 	"-5");
	SetWidgetText(		kDomeBox_Minus1, 	"-1");
	SetWidgetText(		kDomeBox_Plus1, 	"+1");
	SetWidgetText(		kDomeBox_Plus5, 	"+5");
	SetWidgetText(		kDomeBox_Plus10, 	"+10");
	SetWidgetText(		kDomeBox_Plus20, 	"+20");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//**************************************************************
	btnWidth		=	cClmWidth * 2;
	SetWidget(			kDomeBox_GoLeft,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_GoLeft,	"Go Left");

	SetWidget(			kDomeBox_GoRight,	cClm5_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_GoRight,	"Go Right");

	SetWidget(			kDomeBox_Stop,	cClm3_offset+6,		yLoc,		btnWidth-8,		3 * myButtonHt);
	SetWidgetText(		kDomeBox_Stop,	"STOP");
	SetWidgetBGColor(	kDomeBox_Stop,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_Stop,	CV_RGB(255,	255,	255));
	SetWidgetFont(		kDomeBox_Stop,	kFont_Large);
	yLoc			+=	myButtonHt;
	yLoc			+=	2;


	//**************************************************************
	SetWidget(			kDomeBox_SlowLeft,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_SlowLeft,	"Slow Left");

	SetWidget(			kDomeBox_SlowRight,	cClm5_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_SlowRight, "Slow Right");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//**************************************************************
	SetWidget(			kDomeBox_BumpLeft,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_BumpLeft, "Bump Left");

	SetWidget(			kDomeBox_BumpRight,	cClm5_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_BumpRight, "Bump Right");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 6);
	SetWidget(			kDomeBox_ToggleSlaveMode,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_ToggleSlaveMode, 	"Enable Slave mode");
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	for (iii=kDomeBox_GoHome; iii<=kDomeBox_ToggleSlaveMode; iii++)
	{
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	iii,	CV_RGB(0,	0,	0));

	}

	//============================================
	//*	disable all of the extra commands until we know if they exist
	ResetKnownCommands();


	SetWidgetBGColor(	kDomeBox_Stop,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_Stop,		CV_RGB(255,	255,	255));
	SetWidgetFont(		kDomeBox_Stop,		kFont_Large);


	//============================================
	yLoc			+=	5;
	btnWidth		=	(cClmWidth * 3);
	SetWidget(			kDomeBox_OpenShutter,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetText(		kDomeBox_OpenShutter, 	"Open Shutter");
	SetWidgetType(		kDomeBox_OpenShutter,	kWidgetType_Button);
	SetWidgetBGColor(	kDomeBox_OpenShutter,	CV_RGB(0,	128,	240));
	SetWidgetTextColor(	kDomeBox_OpenShutter,	CV_RGB(0,	0,	0));

	SetWidget(			kDomeBox_CloseShutter,	cClm4_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetType(		kDomeBox_CloseShutter,	kWidgetType_Button);
	SetWidgetText(		kDomeBox_CloseShutter, 	"Close Shutter");
	SetWidgetBGColor(	kDomeBox_CloseShutter,	CV_RGB(0,	128,	240));
	SetWidgetTextColor(	kDomeBox_CloseShutter,	CV_RGB(0,	0,	0));

	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 6);
	SetWidget(			kDomeBox_StopShutter,	cClm1_offset,		yLoc,		btnWidth,		myButtonHt);
	SetWidgetType(		kDomeBox_StopShutter,	kWidgetType_Button);
	SetWidgetText(		kDomeBox_StopShutter, 	"STOP Shutter");
	SetWidgetBGColor(	kDomeBox_StopShutter,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_StopShutter,	CV_RGB(255,	255,	255));
	yLoc			+=	myButtonHt;
	yLoc			+=	2;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kDomeBox_IPaddr,
							kDomeBox_Readall,
							kDomeBox_AlpacaErrorMsg,
							kDomeBox_LastCmdString,
							kDomeBox_AlpacaLogo,
							-1);

#ifdef _ENABLE_SKYTRAVEL_
	if (cParentIsSkyTravel)
	{
	int	domeBoxSize;
	int	xLoc;
	int	compassLetterBxSize	=	80;
	int	compassBox_xloc;
	int	compassBox_yLoc;

		SetWidgetOutlineBox(kDomeBox_Outline, kDomeBox_CurPosLabel, kDomeBox_StopShutter);

		//------------------------------------------------------
		xLoc		=	600;
		yLoc		=	domeGraphic_yLoc;
		domeBoxSize	=	cWidth - xLoc;
		domeBoxSize	-=	20;

		SetWidget(			kDomeBox_DomeGraphic,	xLoc,	yLoc,	domeBoxSize,	domeBoxSize);
		SetWidgetType(		kDomeBox_DomeGraphic,	kWidgetType_CustomGraphic);
		SetWidgetBGColor(	kDomeBox_DomeGraphic,	CV_RGB(128,	128,	128));

		compassBox_xloc	=	xLoc + (domeBoxSize / 2) - (compassLetterBxSize / 2) - 10;
		compassBox_yLoc	=	yLoc;

		SetWidget(			kDomeBox_North,	compassBox_xloc,	compassBox_yLoc,	compassLetterBxSize,	compassLetterBxSize);
		SetWidgetType(		kDomeBox_North,	kWidgetType_TextBox);
		SetWidgetBGColor(	kDomeBox_North,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(	kDomeBox_North,	CV_RGB(255,	255,	0));
		SetWidgetFont(		kDomeBox_North,	kFont_Script_Large);
		SetWidgetText(		kDomeBox_North, 	"N");
		SetWidgetBorder(	kDomeBox_North, 	false);


		compassBox_yLoc	=	yLoc + domeBoxSize - compassLetterBxSize;

		SetWidget(			kDomeBox_South,	compassBox_xloc,	compassBox_yLoc,	compassLetterBxSize,	compassLetterBxSize);
		SetWidgetType(		kDomeBox_South,	kWidgetType_TextBox);
		SetWidgetBGColor(	kDomeBox_South,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(	kDomeBox_South,	CV_RGB(255,	255,	0));
		SetWidgetFont(		kDomeBox_South,	kFont_Script_Large);
		SetWidgetText(		kDomeBox_South, 	"S");
		SetWidgetBorder(	kDomeBox_South, 	false);

		compassBox_xloc	=	xLoc;
		compassBox_yLoc	=	yLoc + (domeBoxSize / 2) - (compassLetterBxSize / 2) - 10;

		SetWidget(			kDomeBox_West,	compassBox_xloc,	compassBox_yLoc,	compassLetterBxSize,	compassLetterBxSize);
		SetWidgetType(		kDomeBox_West,	kWidgetType_TextBox);
		SetWidgetBGColor(	kDomeBox_West,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(	kDomeBox_West,	CV_RGB(255,	255,	0));
		SetWidgetFont(		kDomeBox_West,	kFont_Script_Large);
		SetWidgetText(		kDomeBox_West, 	"W");
		SetWidgetBorder(	kDomeBox_West, 	false);

		compassBox_xloc	=	xLoc + domeBoxSize - compassLetterBxSize;

		SetWidget(			kDomeBox_East,	compassBox_xloc,	compassBox_yLoc,	compassLetterBxSize,	compassLetterBxSize);
		SetWidgetType(		kDomeBox_East,	kWidgetType_TextBox);
		SetWidgetBGColor(	kDomeBox_East,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(	kDomeBox_East,	CV_RGB(255,	255,	0));
		SetWidgetFont(		kDomeBox_East,	kFont_Script_Large);
		SetWidgetText(		kDomeBox_East, 	"E");
		SetWidgetBorder(	kDomeBox_East, 	false);
	}
#endif
}

//******************************************************************************
void	WindowTabDome::SetDomePropertiesPtr(TYPE_DomeProperties *domePropPtr)
{
	cDomePropPtr		=	domePropPtr;
}

//******************************************************************************
void	WindowTabDome::ResetKnownCommands(void)
{
int		iii;
	//============================================
	//*	disable all of the extra commands until we know if they exist
	for (iii=kDomeBox_GoLeft; iii<=kDomeBox_BumpRight; iii++)
	{
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
		SetWidgetValid(			iii,	false);
	}

}

//*****************************************************************************
void	WindowTabDome::UpdateControls(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(		kDomeBox_GoHome,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_GoPark,	cDomePropPtr->CanSetAzimuth);

	SetWidgetValid(		kDomeBox_Minus20,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Minus10,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Minus5,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Minus1,	cDomePropPtr->CanSetAzimuth);

	SetWidgetValid(		kDomeBox_Plus1,		cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Plus5,		cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Plus10,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_Plus20,	cDomePropPtr->CanSetAzimuth);

	SetWidgetValid(		kDomeBox_Stop,		cDomePropPtr->CanSetAzimuth);

	SetWidgetValid(		kDomeBox_ToggleSlaveMode,	cDomePropPtr->CanSlave);


	//*	these are all extras
	SetWidgetValid(		kDomeBox_GoLeft,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_GoRight,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_SlowLeft,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_SlowRight,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_BumpLeft,	cDomePropPtr->CanSetAzimuth);
	SetWidgetValid(		kDomeBox_BumpRight,	cDomePropPtr->CanSetAzimuth);

	CONSOLE_DEBUG("exit");
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabDome::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabDome::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
//******************************************************************************
{
cv::Rect	myCVrect;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);

	cOpenCV_Image	=	openCV_Image;

	switch(widgetIdx)
	{
#ifdef _ENABLE_SKYTRAVEL_
		case kDomeBox_DomeGraphic:
			DrawDomeGraphic(openCV_Image, &cWidgetList[widgetIdx]);
			break;
#endif // _ENABLE_SKYTRAVEL_

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

#ifdef _USE_OPENCV_CPP_
//*****************************************************************************
void	WindowTabDome::DrawDomeGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget)
#else
//*****************************************************************************
void	WindowTabDome::DrawDomeGraphic(IplImage *openCV_Image, TYPE_WIDGET *theWidget)
#endif // _USE_OPENCV_CPP_
{
cv::Point	centerLoc;
int			radius;
double		domeAzimuth_radians;
double		domeAzimuth_degrees;
//*	dome specs, will make these prefs later
//*	all are in inches
double		domeDiameter	=	15.0 * 12.0;	//*	 15 feet
double		doorWidth		=	48.0;
double		doorOpeningHalfAngle;
double		doorEdgeAzimuth;
double		slitEdgeAzimuth;
int			pointCntr;
int			edgeRadius;
int			centerX;
int			centerY;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
cv::Scalar	domeOpeningColor;

//	CONSOLE_DEBUG(__FUNCTION__);

	centerLoc.x		=	theWidget->left + (theWidget->height / 2);
	centerLoc.y		=	theWidget->top + (theWidget->height / 2);


	domeAzimuth_degrees	=	145.0;
	if (cDomePropPtr != NULL)
	{
		domeAzimuth_degrees	=	cDomePropPtr->Azimuth;
	}
	//*	fill in the main circle
	if (openCV_Image != NULL)
	{
		cOpenCV_Image	=	openCV_Image;
		LLD_SetColor(W_WHITE);
		centerX		=	theWidget->left + (theWidget->height / 2);
		centerY		=	theWidget->top + (theWidget->height / 2);
		radius		=	theWidget->height / 3;
		LLD_FillEllipse(centerX, centerY, radius, radius);

		domeAzimuth_radians	=	RADIANS(domeAzimuth_degrees - 90.0);

//		bgColor			=	CV_RGB(64, 64, 64);
//		borderColor		=	CV_RGB(64, 64, 64);

		doorOpeningHalfAngle	=	asin((doorWidth / 2.0) / (domeDiameter / 2.0));

		doorEdgeAzimuth			=	domeAzimuth_radians + doorOpeningHalfAngle;
		pt1_X					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1_Y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);


		doorEdgeAzimuth			=	domeAzimuth_radians - doorOpeningHalfAngle;
		pt2_X					=	centerLoc.x + (cos(doorEdgeAzimuth + M_PI) * radius);
		pt2_Y					=	centerLoc.y + (sin(doorEdgeAzimuth + M_PI) * radius);

		LLD_PenSize(2);
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_SetColor(W_DARKGRAY);
		LLD_LineTo(pt2_X, pt2_Y);


		doorEdgeAzimuth			=	domeAzimuth_radians - doorOpeningHalfAngle;
		pt1_X					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1_Y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);


		doorEdgeAzimuth			=	domeAzimuth_radians + doorOpeningHalfAngle;
		pt2_X					=	centerLoc.x + (cos(doorEdgeAzimuth + M_PI) * radius);
		pt2_Y					=	centerLoc.y + (sin(doorEdgeAzimuth + M_PI) * radius);

		LLD_PenSize(3);
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_SetColor(W_DARKGRAY);
		LLD_LineTo(pt2_X, pt2_Y);

		//*	build the point list for the opening
		pointCntr				=	0;

	#ifdef _USE_OPENCV_CPP_
cv::Point	pt1;
cv::Point	ptList[20];
cv::Point	pointLoc;
		//-------------------
		doorEdgeAzimuth			=	domeAzimuth_radians + doorOpeningHalfAngle;
		pt1.x					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1.y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);

		ptList[pointCntr++]		=	pt1;
		pointLoc.x		=	centerLoc.x + (cos(domeAzimuth_radians) * radius);
		pointLoc.y		=	centerLoc.y + (sin(domeAzimuth_radians) * radius);
		ptList[pointCntr++]		=	pointLoc;	//*	this is the middle of the outer edge of the dome opening

		//-------------------
		doorEdgeAzimuth			=	domeAzimuth_radians - doorOpeningHalfAngle;
		pt1.x					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1.y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);

		ptList[pointCntr++]	=	pt1;

		//-------------------
		slitEdgeAzimuth			=	domeAzimuth_radians - RADIANS(90.0);
		edgeRadius				=	((doorWidth / 2.0) / (domeDiameter / 2.0)) * radius;

		pt1.x					=	centerLoc.x + (cos(slitEdgeAzimuth) * edgeRadius);
		pt1.y					=	centerLoc.y + (sin(slitEdgeAzimuth) * edgeRadius);
		ptList[pointCntr++]	=	pt1;

		//-------------------
		slitEdgeAzimuth			=	domeAzimuth_radians + RADIANS(90.0);
		edgeRadius				=	((doorWidth / 2.0) / (domeDiameter / 2.0)) * radius;

		pt1.x					=	centerLoc.x + (cos(slitEdgeAzimuth) * edgeRadius);
		pt1.y					=	centerLoc.y + (sin(slitEdgeAzimuth) * edgeRadius);
		ptList[pointCntr++]	=	pt1;

		//*	determine a color for the dome door
		switch(cDomePropPtr->ShutterStatus)
		{
			case kShutterStatus_Open:
				domeOpeningColor	=	CV_RGB(0, 0, 0);
				break;

			case kShutterStatus_Closed:
				domeOpeningColor	=	CV_RGB(200, 200, 200);
				break;

			case kShutterStatus_Opening:
				domeOpeningColor	=	CV_RGB(0, 200, 0);
				break;

			case kShutterStatus_Closing:
				domeOpeningColor	=	CV_RGB(200, 0, 0);
				break;

			case kShutterStatus_Error:
			default:
				domeOpeningColor	=	CV_RGB(200, 200, 0);
				break;

		}

		cv::fillConvexPoly(	*openCV_Image,
							ptList,						//	const CvPoint* pts,
							pointCntr,					//	pointCntrint npts,
							domeOpeningColor,			//	color
							8,							//	int line_type CV_DEFAULT(8),
							0);							//	int shift CV_DEFAULT(0));
	#else
CvPoint	pt1;
CvPoint	ptList[20];
CvPoint	pointLoc;

		//-------------------
		doorEdgeAzimuth			=	domeAzimuth_radians + doorOpeningHalfAngle;
		pt1.x					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1.y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);

		ptList[pointCntr++]		=	pt1;
		pointLoc.x				=	centerLoc.x + (cos(domeAzimuth_radians) * radius);
		pointLoc.y				=	centerLoc.y + (sin(domeAzimuth_radians) * radius);
		ptList[pointCntr++]		=	pointLoc;	//*	this is the middle of the outer edge of the dome opening

		//-------------------
		doorEdgeAzimuth			=	domeAzimuth_radians - doorOpeningHalfAngle;
		pt1.x					=	centerLoc.x + (cos(doorEdgeAzimuth) * radius);
		pt1.y					=	centerLoc.y + (sin(doorEdgeAzimuth) * radius);

		ptList[pointCntr++]		=	pt1;

		//-------------------
		slitEdgeAzimuth			=	domeAzimuth_radians - RADIANS(90.0);
		edgeRadius				=	((doorWidth / 2.0) / (domeDiameter / 2.0)) * radius;

		pt1.x					=	centerLoc.x + (cos(slitEdgeAzimuth) * edgeRadius);
		pt1.y					=	centerLoc.y + (sin(slitEdgeAzimuth) * edgeRadius);
		ptList[pointCntr++]		=	pt1;

		//-------------------
		slitEdgeAzimuth			=	domeAzimuth_radians + RADIANS(90.0);
		edgeRadius				=	((doorWidth / 2.0) / (domeDiameter / 2.0)) * radius;

		pt1.x					=	centerLoc.x + (cos(slitEdgeAzimuth) * edgeRadius);
		pt1.y					=	centerLoc.y + (sin(slitEdgeAzimuth) * edgeRadius);
		ptList[pointCntr++]		=	pt1;

		//*	determine a color for the dome door
		switch(cDomePropPtr->ShutterStatus)
		{
			case kShutterStatus_Open:
				domeOpeningColor	=	CV_RGB(0, 0, 0);
				break;

			case kShutterStatus_Closed:
				domeOpeningColor	=	CV_RGB(200, 200, 200);
				break;

			case kShutterStatus_Opening:
				domeOpeningColor	=	CV_RGB(0, 200, 0);
				break;

			case kShutterStatus_Closing:
				domeOpeningColor	=	CV_RGB(200, 0, 0);
				break;

			case kShutterStatus_Error:
			default:
				domeOpeningColor	=	CV_RGB(200, 200, 0);
				break;

		}

		#if (CV_MAJOR_VERSION <= 3)
	//	ptList[pointCntr++]	=	centerLoc;
		cvFillConvexPoly(openCV_Image,
						ptList,						//	const CvPoint* pts,
						pointCntr,					//	pointCntrint npts,
						domeOpeningColor,			//	color
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
		#endif
	#endif // _USE_OPENCV_CPP_
		//*	put the dark border around the dome
		LLD_PenSize(5);
		LLD_SetColor(W_DARKGRAY);
		LLD_FrameEllipse(centerX, centerY, radius, radius);
		LLD_PenSize(1);
	}

}


//*****************************************************************************
bool	WindowTabDome::SendAlpacaCmdToDome(	const char		*theCommand,
											const char		*dataString,
											SJP_Parser_t	*jsonParser)
{
bool	validData	=	false;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, theCommand);

#ifdef _ENABLE_SKYTRAVEL_
	if (cParentIsSkyTravel)
	{
	ControllerSkytravel	*myControllerObj;

		CONSOLE_DEBUG("_ENABLE_SKYTRAVEL_");

		myControllerObj	=	(ControllerSkytravel *)cParentObjPtr;
		if (myControllerObj != NULL)
		{
		char	ipAddrStr[32];

			inet_ntop(AF_INET, &(myControllerObj->cDomeIpAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
			CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

			validData	=	AlpacaSendPutCmd(	&myControllerObj->cDomeIpAddress,
												myControllerObj->cDomeIpPort,
												"dome",
												myControllerObj->cDomeAlpacaDeviceNum,
												theCommand,
												dataString,
												jsonParser);

			//*	force quick update
			myControllerObj->cUpdateDelta	=	1;

		}
		else
		{
			CONSOLE_DEBUG("myControllerObj is NULL");
		}
	}
	else
#endif
	{
		CONSOLE_DEBUG("NORMAL");
		validData	=	AlpacaSendPutCmd(	"dome",	theCommand,	dataString, jsonParser);
	}
	return(validData);
}

//*****************************************************************************
//	curl -X PUT "http://dome:6800/api/v1/dome/0/slewtoazimuth"
//			-H  "accept: application/json" -H  "Content-Type: application/x-www-form-urlencoded"
//			-d "Azimuth=$1&ClientID=1&ClientTransactionID=223"
//*****************************************************************************
void	WindowTabDome::MoveDomeByAmount(const double moveAmount)
{
bool			validData;
double			newAzimuthValue;
char			dataString[64];
ControllerDome	*myDomeController;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("cWindowName\t=",	cWindowName);

	newAzimuthValue	=	0.0;

#ifdef _ENABLE_SKYTRAVEL_
	if (cParentIsSkyTravel)
	{
	ControllerSkytravel	*mySkyTravelController;

		mySkyTravelController	=	(ControllerSkytravel *)cParentObjPtr;
		if (mySkyTravelController != NULL)
		{
			newAzimuthValue			=	mySkyTravelController->cDomeProp.Azimuth;
		}
	}
	else
#endif // _ENABLE_SKYTRAVEL_
	{
		myDomeController	=	(ControllerDome *)cParentObjPtr;
		if (myDomeController != NULL)
		{
			newAzimuthValue			=	myDomeController->cDomeProp.Azimuth;
		}

	}
	CONSOLE_DEBUG_W_DBL("prev Azimuth value\t=", newAzimuthValue);

	newAzimuthValue	+=	moveAmount;
	if (newAzimuthValue < 0.0)
	{
		newAzimuthValue	+=	360.0;
	}
	if (newAzimuthValue >= 360.0)
	{
		newAzimuthValue	-=	360.0;
	}
	CONSOLE_DEBUG_W_DBL("newAzimuthValue\t=", newAzimuthValue);
	sprintf(dataString, "Azimuth=%f", newAzimuthValue);
	validData	=	SendAlpacaCmdToDome("slewtoazimuth",	dataString,	NULL);
	if (validData == false)
	{
		CONSOLE_DEBUG("failed command - slewtoazimuth");
	}
}

//*****************************************************************************
void	WindowTabDome::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool			validData;
SJP_Parser_t	jsonResponse;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	//*	clear out any existing error message
	SetWidgetText(	kDomeBox_AlpacaErrorMsg, 	"---");
	validData	=	true;
	switch(buttonIdx)
	{
		case kDomeBox_GoHome:
			validData	=	SendAlpacaCmdToDome("findhome",		"",	&jsonResponse);
			break;

		case kDomeBox_GoPark:
			validData	=	SendAlpacaCmdToDome("park",			"",	&jsonResponse);
			break;

		case kDomeBox_Minus20:	MoveDomeByAmount(-20.0);	break;
		case kDomeBox_Minus10:	MoveDomeByAmount(-10.0);	break;
		case kDomeBox_Minus5:	MoveDomeByAmount(-5.0);		break;
		case kDomeBox_Minus1:	MoveDomeByAmount(-1.0);		break;
		case kDomeBox_Plus1:	MoveDomeByAmount(1.0);		break;
		case kDomeBox_Plus5:	MoveDomeByAmount(5.0);		break;
		case kDomeBox_Plus10:	MoveDomeByAmount(10.0);		break;
		case kDomeBox_Plus20:	MoveDomeByAmount(20.0);		break;

		case kDomeBox_Stop:
			validData	=	SendAlpacaCmdToDome("abortslew",	"",	&jsonResponse);
			break;

		case kDomeBox_BumpLeft:
			validData	=	SendAlpacaCmdToDome("bumpleft",		"",	&jsonResponse);
			break;

		case kDomeBox_BumpRight:
			validData	=	SendAlpacaCmdToDome("bumpright",	"",	&jsonResponse);
			break;

		case kDomeBox_GoLeft:
			validData	=	SendAlpacaCmdToDome("goleft",		"",	&jsonResponse);
			break;

		case kDomeBox_GoRight:
			validData	=	SendAlpacaCmdToDome("goright",		"",	&jsonResponse);
			break;

		case kDomeBox_SlowLeft:
			validData	=	SendAlpacaCmdToDome("slowleft",		"",	&jsonResponse);
			break;

		case kDomeBox_SlowRight:
			validData	=	SendAlpacaCmdToDome("slowright",	"",	&jsonResponse);
			break;

		case kDomeBox_ToggleSlaveMode:
			ToggleSlaveMode();
			break;

		//*	the shutter commands use a different routine to support the external shutter device
		//*	this routine takes care of external/internal shutter control
		//*	if external shutter is not enabled, it calls the same routine as above
		case kDomeBox_OpenShutter:
			SendShutterCommand("openshutter");
			break;

		case kDomeBox_CloseShutter:
			SendShutterCommand("closeshutter");
			break;

		case kDomeBox_StopShutter:
			SendShutterCommand("abortslew");
			break;

	}
	if (validData == false)
	{
		CONSOLE_DEBUG("AlpacaSendPutCmd failed");
	}

	DisplayLastAlpacaCommand();
}

//*****************************************************************************
void	WindowTabDome::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kDomeBox_Title:
		case kDomeBox_Connected:
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
			break;
	}
}

//*****************************************************************************
void	WindowTabDome::SendShutterCommand(const char *shutterCmd)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, shutterCmd);

#if defined(_ENABLE_EXTERNAL_SHUTTER_) && !defined(_ENABLE_SKYTRAVEL_)
	CONSOLE_DEBUG("_ENABLE_EXTERNAL_SHUTTER_");

	ControllerDome	*myDomeController;

		CONSOLE_DEBUG(__FUNCTION__);
		myDomeController	=	(ControllerDome *)cParentObjPtr;

		if (myDomeController != NULL)
		{
			myDomeController->SendShutterCommand(shutterCmd);
		}
		else
		{
			CONSOLE_DEBUG("myDomeController is NULL");
		}
#else
bool			validData;
SJP_Parser_t	jsonResponse;

	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	SendAlpacaCmdToDome(shutterCmd,	"",	&jsonResponse);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("failed shutter command - ", shutterCmd);
	}

#endif

}


//*****************************************************************************
void	WindowTabDome::ToggleSlaveMode(void)
{
#ifdef _ENABLE_SKYTRAVEL_
	SetWidgetText(kDomeBox_AlpacaErrorMsg, "Slave mode not enabled for SkyTravel yet");
	ForceWindowUpdate();
#else
bool			validData;
//int				myShutterStatus;
int				mySlavedMode;
SJP_Parser_t	jsonParser;
int				alpacaErrorCode;
char			alpacaErrorMsg[128];
char			textString[256];
ControllerDome	*myDomeController;

	CONSOLE_DEBUG(__FUNCTION__);
	myDomeController	=	(ControllerDome *)cParentObjPtr;

	if (myDomeController != NULL)
	{
		//*	we need to find out the current slaved mode
		mySlavedMode	=	myDomeController->cDomeProp.Slaved;
//		myShutterStatus	=	myDomeController->cShutterStatus;
		if (mySlavedMode)
		{
			CONSOLE_DEBUG("Slave mode is ON, turning off");
			//*	send the command to turn off slave mode
			validData	=	SendAlpacaCmdToDome("slaved",	"Slaved=false");
		}
		else
		{
			CONSOLE_DEBUG("Slave mode is OFF, turning ON");
			validData		=	SendAlpacaCmdToDome("slaved",	"Slaved=true", &jsonParser);
			alpacaErrorCode	=	myDomeController->AlpacaCheckForErrors(&jsonParser, alpacaErrorMsg, false);
			if (alpacaErrorCode != kASCOM_Err_Success)
			{
				CONSOLE_DEBUG_W_STR("alpacaErrorMsg\t=", alpacaErrorMsg);
				sprintf(textString, "Err# %d - %s", alpacaErrorCode, alpacaErrorMsg);
				SetWidgetText(	kDomeBox_AlpacaErrorMsg, 	textString);

			}

	//		if (myShutterStatus == kShutterStatus_Open)
	//		{
	//			validData	=	SendAlpacaCmdToDome("slaved",	"Slaved=true");
	//		}
	//		else
	//		{
	//			SetWidgetText(		kDomeBox_AlpacaErrorMsg, 	"Can't enable slave mode unless shutter is open");
	//		}
		}
		if (validData == false)
		{
			CONSOLE_DEBUG("AlpacaSendPutCmd failed");
		}
	}
	else
	{
		CONSOLE_DEBUG("myDomeController is NULL");
	}
#endif
}


#endif // _ENABLE_CTRL_DOME_

