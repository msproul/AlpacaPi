//*****************************************************************************
//*		windowtab_nitecrawler.cpp		(c) 2020 by Mark Sproul
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
//*	Feb 24,	2020	<MLS> Created windowtab_focuser.cpp
//*	Feb 26,	2020	<MLS> Added NiteCrawler logo in memory so .png file is not needed
//*	Mar 12,	2020	<MLS> Changed windowtab_focuser to windowtab_nitecrawler
//*	Mar 23,	2020	<MLS> Added "Rotator Jog" & "Focuser Jog" labels
//*	Apr  4,	2020	<MLS> Added compass indicator to where we WANT to be
//*	Apr 17,	2020	<MLS> Added Alpaca Logo to NiteCrawler screen
//*	Dec 28,	2020	<MLS> Added ZERO button to zero Rotator and Aux values
//*	Feb 19,	2022	<MLS> Added DrawRotatorCompass()
//*	Sep  5,	2022	<MLS> Made focuser controller wider, adjusted compass routine
//*****************************************************************************

#ifdef _ENABLE_CTRL_FOCUSERS_

#include	<math.h>
#include	<stdio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"json_parse.h"
#include	"controller.h"
#include	"controller_focus.h"

//#include	"windowtab_focuser.h"
#include	"windowtab_nitecrawler.h"


#include	"nitecrawler_image.h"


//**************************************************************************************
WindowTabNitecrawler::WindowTabNitecrawler(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const int	comMode,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (gNiteCrawlerImgPtr == NULL)
	{
		gNiteCrawlerImgPtr	=	GetNiteCrawlerImage();

		if (gNiteCrawlerImgPtr != NULL)
		{
		//	WriteOutImageAsCode(gNiteCrawlerImgPtr);
		//	CONSOLE_DEBUG_W_NUM("nc image width\t=", gNiteCrawlerImgPtr->width);
		//	CONSOLE_DEBUG_W_NUM("nc image height\t=", gNiteCrawlerImgPtr->height);
		}
	}


	cRotatorPosition	=	0;
	cStepsPerRev		=	kStepsPerRev_WR30;
	cComMode			=	comMode;
//	CONSOLE_DEBUG_W_HEX("cWidgetList=", cWidgetList);

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabNitecrawler::~WindowTabNitecrawler(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabNitecrawler::SetupWindowControls(void)
{
int		iii;
int		xLoc;
int		yLoc;
int		yloc2;
int		buttonNumValues[5]	=	{1, 10, 100, 1000, 5000};
int		btnIdx;
char	textBuff[32];
int		homeBtnWidth;
int		logoWidth;
int		logoHeight;
int		compassWidth;
int		compassHeight;

//	CONSOLE_DEBUG(__FUNCTION__);

	//==========================================
	yLoc			=	cTabVertOffset;

	//==========================================
	if (gNiteCrawlerImgPtr != NULL)
	{
	#ifdef _USE_OPENCV_CPP_
		logoWidth	=	gNiteCrawlerImgPtr->cols;
		logoHeight	=	gNiteCrawlerImgPtr->rows;
	#else
		logoWidth	=	gNiteCrawlerImgPtr->width;
		logoHeight	=	gNiteCrawlerImgPtr->height;
	#endif // _USE_OPENCV_CPP_
		if (logoWidth < cWidth)
		{
			xLoc	=	(cWidth - logoWidth) / 2;
		}
		else
		{
			xLoc	=	0;
		}

		SetWidget(	kNiteCrawlerTab_logo,
					xLoc,
					yLoc,
					logoWidth,
					logoHeight);
		SetWidgetBorder(kNiteCrawlerTab_logo, false);

		yLoc		+=	logoHeight;

		SetWidgetImage(kNiteCrawlerTab_logo, gNiteCrawlerImgPtr);
	}
	else
	{
		SetWidget(kNiteCrawlerTab_logo,		0,			yLoc,		cWidth,		kLogoHeight	);
		yLoc			+=	kLogoHeight;
	}
	yLoc			+=	2;
	SetWidgetType(kNiteCrawlerTab_logo, kWidgetType_Image);

	//==========================================
	SetWidget(		kNiteCrawlerTab_Model,	0,	yLoc,		cWidth,	cBtnHeight);
	SetWidgetFont(	kNiteCrawlerTab_Model, kFont_Medium);

   	SetUpConnectedIndicator(kNiteCrawlerTab_Connected, yLoc);

	yLoc	+=	cBtnHeight;
	yLoc	+=	2;

	SetBGcolorFromWindowName(kNiteCrawlerTab_Model);

	//==========================================
	SetWidget(		kNiteCrawlerTab_rotValue,		0,			yLoc,		(cWidth/2),	40);
	SetWidget(		kNiteCrawlerTab_focValue,		(cWidth/2),	yLoc,		(cWidth/2),	40);

	yLoc			+=	40;
	yLoc			+=	2;

	//==========================================
	SetWidget(kNiteCrawlerTab_rotLabel,	0,			yLoc,		(cFullWidthBtn/2),	30);
	SetWidget(kNiteCrawlerTab_focLabel,	(cWidth/2),	yLoc,		(cFullWidthBtn/2),	30);

	SetWidgetFont(kNiteCrawlerTab_rotLabel, kFont_Medium);
	SetWidgetFont(kNiteCrawlerTab_focLabel, kFont_Medium);
	SetWidgetText(kNiteCrawlerTab_rotLabel, "ROTATION");
	SetWidgetText(kNiteCrawlerTab_focLabel, "FOCUSER");
	SetWidgetTextColor(kNiteCrawlerTab_rotLabel,		CV_RGB(255, 255, 255));
	SetWidgetTextColor(kNiteCrawlerTab_focLabel,		CV_RGB(255, 255, 255));
	yLoc			+=	30;
	yLoc			+=	2;


	//==========================================
	SetWidget(kNiteCrawlerTab_rotDesired,	0,			yLoc,		(cWidth/2),	40);
	SetWidget(kNiteCrawlerTab_focDesired,	(cWidth/2),	yLoc,		(cWidth/2),	40);
	yLoc			+=	40;
	yLoc			+=	2;


	//==========================================
	//*	create the compass wheel
	compassWidth	=	cWidth / 2;
	compassHeight	=	151;
	CONSOLE_DEBUG_W_NUM("compassWidth\t=",	compassWidth);
	CONSOLE_DEBUG_W_NUM("compassHeight\t=",	compassHeight);
	SetWidget(		kNiteCrawlerTab_RotatorCompass,	0,		yLoc,	compassWidth,	compassHeight);
	SetWidgetType(	kNiteCrawlerTab_RotatorCompass, kWidgetType_CustomGraphic);

	//==========================================
	yloc2	=	yLoc;

	SetWidget(			kNiteCrawlerTab_Temperature,	(cWidth/2),	yloc2,		(cWidth/2),	30);
	SetWidgetFont(		kNiteCrawlerTab_Temperature,	kFont_Medium);
	SetWidgetTextColor(	kNiteCrawlerTab_Temperature,	Color16BitTo24Bit(0xfe00));
	yloc2			+=	30;
	yloc2			+=	2;

	SetWidget(			kNiteCrawlerTab_Voltage,		(cWidth/2),	yloc2,		(cWidth/2),	30);
	SetWidgetFont(		kNiteCrawlerTab_Voltage,		kFont_Medium);
	SetWidgetTextColor(	kNiteCrawlerTab_Voltage,		Color16BitTo24Bit(0x07ff));

	yloc2			+=	30;
	yloc2			+=	2;

#define	kSwitchHeight	20
	SetWidget(			kNiteCrawlerTab_SwitchIN,		(cWidth/2),	yloc2,		cBtnWidth,	kSwitchHeight);
	SetWidget(			kNiteCrawlerTab_SwitchROT,		cClm5_offset,	yloc2,		cBtnWidth,	kSwitchHeight);
	SetWidget(			kNiteCrawlerTab_SwitchOUT,		cClm6_offset,	yloc2,		cBtnWidth,	kSwitchHeight);

	yloc2			+=	kSwitchHeight;
	yloc2			+=	2;

	SetWidget(			kNiteCrawlerTab_SwitchLbl,		(cWidth/2),	yloc2,		(cWidth/2),	kSwitchHeight);
	yloc2			+=	kSwitchHeight;
	yloc2			+=	12;
	SetWidget(		kNiteCrawlerTab_StopMotors,	(cWidth/2),	yloc2,		(cWidth/2),	30);
	SetWidgetType(	kNiteCrawlerTab_StopMotors,		kWidgetType_Button);

	SetWidgetFont(	kNiteCrawlerTab_SwitchIN,			kFont_Medium);
	SetWidgetFont(	kNiteCrawlerTab_SwitchROT,			kFont_Medium);
	SetWidgetFont(	kNiteCrawlerTab_SwitchOUT,			kFont_Medium);
	SetWidgetFont(	kNiteCrawlerTab_SwitchLbl,			kFont_Medium);
	SetWidgetFont(	kNiteCrawlerTab_StopMotors,			kFont_Medium);

	SetWidgetText(	kNiteCrawlerTab_SwitchIN,			"IN");
	SetWidgetText(	kNiteCrawlerTab_SwitchROT,			"ROT");
	SetWidgetText(	kNiteCrawlerTab_SwitchOUT,			"OUT");
	SetWidgetText(	kNiteCrawlerTab_SwitchLbl,			"SWITCH");
	SetWidgetText(	kNiteCrawlerTab_StopMotors,			"STOP MOTORS");
	SetWidgetTextColor(		kNiteCrawlerTab_StopMotors,	CV_RGB(255, 255, 255));
	SetWidgetBGColor(		kNiteCrawlerTab_StopMotors,	CV_RGB(255, 0, 0));
	SetWidgetBorderColor(	kNiteCrawlerTab_SwitchLbl,	CV_RGB(0, 0, 0));

	SetWidgetBGColor(		kNiteCrawlerTab_SwitchIN,		kSwitchColorOff);
	SetWidgetBGColor(		kNiteCrawlerTab_SwitchROT,	kSwitchColorOff);
	SetWidgetBGColor(		kNiteCrawlerTab_SwitchOUT,	kSwitchColorOff);

	SetWidgetTextColor(		kNiteCrawlerTab_SwitchIN,	CV_RGB(255, 255, 255));
	SetWidgetTextColor(		kNiteCrawlerTab_SwitchROT,	CV_RGB(255, 255, 255));
	SetWidgetTextColor(		kNiteCrawlerTab_SwitchOUT,	CV_RGB(255, 255, 255));


	//*	move down to the bottom of the compass
	yLoc		+=	compassHeight;

	SetWidget(		kNiteCrawlerTab_RotationJog,	cClm1_offset,	yLoc,	(cClmWidth * 2),	cBoxHeight);
	SetWidget(		kNiteCrawlerTab_FocusJog,		cClm5_offset,	yLoc,	(cClmWidth * 2),	cBoxHeight);

	SetWidgetType(	kNiteCrawlerTab_RotationJog,	kWidgetType_MultiLineText);
	SetWidgetType(	kNiteCrawlerTab_FocusJog,		kWidgetType_MultiLineText);

	SetWidgetText(	kNiteCrawlerTab_RotationJog,	"Rotation\rJog");
	SetWidgetText(	kNiteCrawlerTab_FocusJog,		"Focus\rJog");

	SetWidgetFont(	kNiteCrawlerTab_RotationJog,	kFont_RadioBtn);
	SetWidgetFont(	kNiteCrawlerTab_FocusJog,		kFont_RadioBtn);

	SetWidgetJustification(	kNiteCrawlerTab_RotationJog,	kJustification_Center);
	SetWidgetJustification(	kNiteCrawlerTab_FocusJog,		kJustification_Center);

	SetWidgetBorderColor(	kNiteCrawlerTab_RotationJog,	CV_RGB(0, 0, 0));
	SetWidgetBorderColor(	kNiteCrawlerTab_FocusJog, 		CV_RGB(0, 0, 0));

	yLoc			+=	cBoxHeight;

	homeBtnWidth	=	(cBtnWidth * 2) - 10;
	SetWidget(		kNiteCrawlerTab_Home,	(cBtnWidth * 2),	yLoc,			homeBtnWidth,	cBtnHeight);
	SetWidgetFont(	kNiteCrawlerTab_Home, kFont_Small);
	SetWidgetText(	kNiteCrawlerTab_Home, "HOME");

	if (cComMode == kNCcomMode_USB)
	{
		SetWidget(		kNiteCrawlerTab_Zero,	(cBtnWidth * 2),	(yLoc + cBoxHeight), homeBtnWidth,	cBtnHeight);
		SetWidgetFont(	kNiteCrawlerTab_Zero, kFont_Small);
		SetWidgetText(	kNiteCrawlerTab_Zero, "Zero");
	}


	//*	this will use the buttons in the order they are defined to set up the widgets
	//*	the above order MUST match
	btnIdx	=	kNiteCrawlerTab_Rot_m1;
	for (iii=0; iii<5; iii++)
	{
		SetWidget(		btnIdx,	cClm1_offset,	yLoc,	cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		sprintf(textBuff, "-%d", buttonNumValues[iii]);
		SetWidgetText(btnIdx, textBuff);
		btnIdx++;

		SetWidget(		btnIdx,	cClm2_offset,	yLoc,	cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		sprintf(textBuff, "+%d", buttonNumValues[iii]);
		SetWidgetText(btnIdx, textBuff);
		btnIdx++;

		SetWidget(		btnIdx,	cClm5_offset,	yLoc,	cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		sprintf(textBuff, "-%d", buttonNumValues[iii]);
		SetWidgetText(btnIdx, textBuff);
		btnIdx++;

		SetWidget(		btnIdx,	cClm6_offset,	yLoc,	cBtnWidth,	cBtnHeight);
		SetWidgetFont(	btnIdx, kFont_Small);
		sprintf(textBuff, "+%d", buttonNumValues[iii]);
		SetWidgetText(btnIdx, textBuff);
		btnIdx++;

		yLoc	+=	cBtnHeight;
		yLoc	+=	2;
	}

	//*	set the colors for the buttons
	for (iii=kNiteCrawlerTab_Home; iii<=kNiteCrawlerTab_Foc_p5000; iii++)
	{
		SetWidgetType(			iii,		kWidgetType_Button);
		SetWidgetBGColor(		iii,		CV_RGB(255, 255, 255));
		SetWidgetBorderColor(	iii,		CV_RGB(0, 0, 0));
		SetWidgetTextColor(		iii,		CV_RGB(0, 0, 0));
	}

	yLoc	+=	cBtnHeight;
	yLoc	+=	2;

	//*	set default text strings
	SetWidgetText(	kNiteCrawlerTab_focValue,		"-----");
	SetWidgetText(	kNiteCrawlerTab_Model,			"-----");
	SetWidgetText(	kNiteCrawlerTab_focValue,		"-----");
	SetWidgetText(	kNiteCrawlerTab_rotValue,		"-----");
	SetWidgetText(	kNiteCrawlerTab_focDesired,		"-----");
	SetWidgetText(	kNiteCrawlerTab_rotDesired,		"-----");
	SetWidgetText(	kNiteCrawlerTab_Temperature,	"-----");
	SetWidgetText(	kNiteCrawlerTab_Voltage,		"-----");
	SetWidgetText(	kNiteCrawlerTab_IPaddr,			"-----");


	if (cComMode == kNCcomMode_Alpaca)
	{
		//*	now set the Alpaca Logo
		LoadAlpacaLogo();
		if (gAlpacaLogoPtr != NULL)
		{

		#ifdef _USE_OPENCV_CPP_
			logoWidth	=	gAlpacaLogoPtr->cols;
			logoHeight	=	gAlpacaLogoPtr->rows;
		#else
			logoWidth	=	gAlpacaLogoPtr->width;
			logoHeight	=	gAlpacaLogoPtr->height;
		#endif
			xLoc		=	((cWidth/2) - (logoWidth/2));
			xLoc		-=	6;
			yLoc		=	cHeight - (1 * cBtnHeight);
			yLoc		-=	logoHeight;

			SetWidget(		kNiteCrawlerTab_AlpacaLogo,	xLoc,	yLoc,	logoWidth,	logoHeight);
			SetWidgetImage(	kNiteCrawlerTab_AlpacaLogo, gAlpacaLogoPtr);
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
	}

	switch(cComMode)
	{
		case kNCcomMode_Alpaca:
			SetIPaddressBoxes(kNiteCrawlerTab_IPaddr, kNiteCrawlerTab_Readall, -1, -1);
			break;

		case kNCcomMode_USB:
			SetIPaddressBoxes(kNiteCrawlerTab_IPaddr, kNiteCrawlerTab_Readall, -1, kNiteCrawlerTab_Connect);
			break;
	}
}

//**************************************************************************************
void	WindowTabNitecrawler::DrawRotatorCompass(TYPE_WIDGET *theWidget)
{
int			radius1;
int			radius2;
double		degrees;
double		radians;
int			center_X;
int			center_Y;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
char		lineBuff[32];

	center_X	=	theWidget->left + (theWidget->width / 2);
	center_Y	=	theWidget->top + (theWidget->height / 2);
	radius1		=	theWidget->width / 5;

	LLD_SetColor(W_WHITE);
	LLD_FrameEllipse(center_X, center_Y, radius1, radius1);

	//*	now draw the tick marks every 10 degrees
	radius1	=	(theWidget->height / 2) - 8;
	radius2	=	(theWidget->height / 2) - 16;
	degrees	=	0;
	while (degrees < 360)
	{
		radians	=	degrees * M_PI / 180.0;
		pt1_X	=	center_X + (cos(radians) * radius1);
		pt1_Y	=	center_Y + (sin(radians) * radius1);

		pt2_X	=	center_X + (cos(radians) * radius2);
		pt2_Y	=	center_Y + (sin(radians) * radius2);

		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);

		degrees	+=	10.0;
	}
	//*	now figure out where it is pointing
	degrees	=	(cRotatorPosition * 360.0) / cStepsPerRev;
	sprintf(lineBuff, "%1.1f", degrees);
//	CONSOLE_DEBUG_W_DBL("degrees\t=", degrees);
//	CONSOLE_DEBUG(lineBuff);

	//*	draw the numeric degrees in the center
	pt1_X	=	center_X - 18;
	pt1_Y	=	center_Y + 5;
	if (abs(degrees) < 10.0)
	{
		pt1_X	+=	5;
	}
	LLD_DrawCString(pt1_X, pt1_Y, lineBuff, kFont_Medium);


	//*	determine dimensions for red/green indicators
	degrees	-=	90;
	radians	=	degrees * M_PI / 180.0;
	radius1	=	(theWidget->height / 5) + 3;
	radius2	=	(theWidget->height / 2) - 20;

	pt1_X	=	center_X + (cos(radians) * radius1);
	pt1_Y	=	center_Y + (sin(radians) * radius1);

	pt2_X	=	center_X + (cos(radians) * radius2);
	pt2_Y	=	center_Y + (sin(radians) * radius2);

	LLD_PenSize(2);
	LLD_SetColor(W_RED);
	LLD_MoveTo(pt1_X, pt1_Y);
	LLD_LineTo(pt2_X, pt2_Y);

	//*	now draw a small green line where we WANT to be
	degrees	=	(cRotatorDesiredPos * 360.0) / cStepsPerRev;
	degrees	-=	90;
	radians	=	degrees * M_PI / 180.0;
	radius1	=	(theWidget->height / 2) - 20;
	radius2	=	(theWidget->height / 2);

	pt1_X	=	center_X + (cos(radians) * radius1);
	pt1_Y	=	center_Y + (sin(radians) * radius1);

	pt2_X	=	center_X + (cos(radians) * radius2);
	pt2_Y	=	center_Y + (sin(radians) * radius2);
	LLD_SetColor(W_GREEN);
	LLD_MoveTo(pt1_X, pt1_Y);
	LLD_LineTo(pt2_X, pt2_Y);

	//*	reset back to normal
	LLD_PenSize(1);
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabNitecrawler::DrawWidgetCustomGraphic(	cv::Mat		*openCV_Image,
														const int	widgetIdx)
#else
//**************************************************************************************
void	WindowTabNitecrawler::DrawWidgetCustomGraphic(	IplImage	*openCV_Image,
														const int	widgetIdx)
#endif // _USE_OPENCV_CPP_
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cOpenCV_Image	=	openCV_Image;

	switch(widgetIdx)
	{
		case kNiteCrawlerTab_RotatorCompass:
			DrawRotatorCompass(&cWidgetList[widgetIdx]);
			break;
	}
}

//*****************************************************************************
void	WindowTabNitecrawler::ProcessButtonClick(const int buttonIdx, const int flags)
{
int				prevDesiredFocusPosition;
int				prevDesiredRotatorPosition;
ControllerFocus	*focusController;
int				myFocuserDesiredPotion;
int				myRotatorDesiredPotion;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	focusController				=	(ControllerFocus*)cParentObjPtr;
	if (focusController != NULL)
	{
		prevDesiredFocusPosition	=	focusController->cFocuserDesiredPos;
		prevDesiredRotatorPosition	=	focusController->cRotatorDesiredPos;

		myFocuserDesiredPotion	=	prevDesiredFocusPosition;
		myRotatorDesiredPotion	=	prevDesiredRotatorPosition;
		switch(buttonIdx)
		{
			case kNiteCrawlerTab_Rot_m1:	myRotatorDesiredPotion	-=	1;		break;
			case kNiteCrawlerTab_Rot_p1:	myRotatorDesiredPotion	+=	1;		break;
			case kNiteCrawlerTab_Foc_m1:	myFocuserDesiredPotion	-=	1;		break;
			case kNiteCrawlerTab_Foc_p1:	myFocuserDesiredPotion	+=	1;		break;

			case kNiteCrawlerTab_Rot_m10:	myRotatorDesiredPotion	-=	10;		break;
			case kNiteCrawlerTab_Rot_p10:	myRotatorDesiredPotion	+=	10;		break;
			case kNiteCrawlerTab_Foc_m10:	myFocuserDesiredPotion	-=	10;		break;
			case kNiteCrawlerTab_Foc_p10:	myFocuserDesiredPotion	+=	10;		break;

			case kNiteCrawlerTab_Rot_m100:	myRotatorDesiredPotion	-=	100;	break;
			case kNiteCrawlerTab_Rot_p100:	myRotatorDesiredPotion	+=	100;	break;
			case kNiteCrawlerTab_Foc_m100:	myFocuserDesiredPotion	-=	100;	break;
			case kNiteCrawlerTab_Foc_p100:	myFocuserDesiredPotion	+=	100;	break;

			case kNiteCrawlerTab_Rot_m1000:	myRotatorDesiredPotion	-=	1000;	break;
			case kNiteCrawlerTab_Rot_p1000:	myRotatorDesiredPotion	+=	1000;	break;
			case kNiteCrawlerTab_Foc_m1000:	myFocuserDesiredPotion	-=	1000;	break;
			case kNiteCrawlerTab_Foc_p1000:	myFocuserDesiredPotion	+=	1000;	break;

			case kNiteCrawlerTab_Rot_m5000:	myRotatorDesiredPotion	-=	5000;	break;
			case kNiteCrawlerTab_Rot_p5000:	myRotatorDesiredPotion	+=	5000;	break;
			case kNiteCrawlerTab_Foc_m5000:	myFocuserDesiredPotion	-=	5000;	break;
			case kNiteCrawlerTab_Foc_p5000:	myFocuserDesiredPotion	+=	5000;	break;

			case kNiteCrawlerTab_StopMotors:
				CONSOLE_DEBUG("kNiteCrawlerTab_Connect");
				focusController->SendStopMotorsCommand();
				break;

			case kNiteCrawlerTab_Home:
				CONSOLE_DEBUG("HOME not finished");
				break;

			case kNiteCrawlerTab_Zero:
				//*	reset the motor values
				focusController->ZeroMotorValues();
				break;

			case kNiteCrawlerTab_Connect:
				if (focusController->cUSBportOpen)
				{
					CONSOLE_DEBUG("DisConnect");
					focusController->CloseUSBport();
				}
				else
				{
					CONSOLE_DEBUG("Connect");
					focusController->OpenUSBport();
				}
				break;

			default:
				CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
				break;
		}

		if (myFocuserDesiredPotion < 0)
		{
			myFocuserDesiredPotion	=	0;
		}

		//*	do we need to update the focuser position
		if (myFocuserDesiredPotion != prevDesiredFocusPosition)
		{
			focusController->SendMoveFocuserCommand(myFocuserDesiredPotion);
		}
		if (myRotatorDesiredPotion != prevDesiredRotatorPosition)
		{
			focusController->SendMoveRotatorCommand(myRotatorDesiredPotion);
		}

		SetWidgetNumber(kNiteCrawlerTab_focDesired, myFocuserDesiredPotion);
		SetWidgetNumber(kNiteCrawlerTab_rotDesired, myRotatorDesiredPotion);
	}
}

//*****************************************************************************
void	WindowTabNitecrawler::ProcessDoubleClick(const int buttonIdx)
{
ControllerFocus	*focusController;

	focusController				=	(ControllerFocus*)cParentObjPtr;

	switch(buttonIdx)
	{
		case kNiteCrawlerTab_rotLabel:
		case kNiteCrawlerTab_focLabel:
			if (focusController != NULL)
			{
				focusController->UpdateFromFirstRead();
			}
			break;

		default:
			CONSOLE_DEBUG_W_NUM("Double click on button #", buttonIdx);
			break;

	}
}

#endif	//	_ENABLE_CTRL_FOCUSERS_
