//*****************************************************************************
//*		windowtab_MoonPhase.cpp		(c) 2024 by Mark Sproul
//*
//*	Description:	Window to display NASA moon phase
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
//*	Mar 25,	2024	<MLS> Created windowtab_MoonPhase.cpp
//*	Mar 26,	2024	<MLS> Added DrawMoonWidget() & DrawMoonGraphic()
//*	Mar 27,	2024	<MLS> Added UpdateCurrentMoonPhase()
//*	Mar 29,	2024	<MLS> Added HandleMouseClickedInGraph()
//*	Mar 30,	2024	<MLS> Added ability to double click and open current moon image
//*	Apr  5,	2024	<MLS> Added ability to download NASA moon image files
//*	Apr  5,	2024	<MLS> Added ELat & ELon to graph
//*	Apr  6,	2024	<MLS> Fixed initialization bug (cDisplayedImage)
//*****************************************************************************

#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"linuxerrors.h"
#include	"helper_functions.h"
#include	"fits_opencv.h"

#include	"NASA_moonphase.h"
#include	"windowtab.h"
#include	"windowtab_MoonPhase.h"
#include	"controller.h"
#include	"controller_image.h"



//**************************************************************************************
WindowTabMoonPhase::WindowTabMoonPhase(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
bool	validPhaseInfo;
char	textBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn				=	-1;
	cFirstLineIdx			=	0;
	cPhaseIndex_Now			=	0;
	cPhaseIndex_Displayed	=	0;
	cDisplayCurrentPhase	=	true;
	cPhaseSimulationEnabled	=	false;

	cGraphColor_Phase		=	W_CYAN;
	cGraphColor_Age			=	W_YELLOW;
	cGraphColor_Diam		=	W_MAGENTA;
	cGraphColor_Dist		=   W_ORANGE;
	cGraphColor_AxisA		=	W_GREEN;
	cGraphColor_ELat		=	W_PINK;
	cGraphColor_ELon		=	W_LIGHTGRAY;
	cGraphColor_Current		=	W_GREEN;
	cGraphColor_Displayed	=	W_RED;

	cEnableGraph_Phase		=	true;
	cEnableGraph_Age		=	false;
	cEnableGraph_Diam		=	false;
	cEnableGraph_Dist		=	false;
	cEnableGraph_AxisA		=	false;
	cEnableGraph_ELat		=	false;
	cEnableGraph_ELon		=	false;
	cEnableOverlay			=	true;

	cMoonOpenCVimage		=	NULL;
	cDisplayedImage			=	NULL;
	cMoonImageYear			=	0;
	cMoonImageIndex			=	-1;	//*	this is the number of hours since Jan 1st
	cMoonImageName[0]		=	0;
	cMoonImagePath[0]		=	0;

	SetWebHelpURLstring("moonphase.html");		//*	set the web help url string

	//*	initialize both to the current time
	validPhaseInfo	=	UpdateCurrentMoonPhase(&cCurrentMoonPhaseInfo);
	cDisplayedMoonPhaseInfo	=	cCurrentMoonPhaseInfo;
//	validPhaseInfo	=	UpdateCurrentMoonPhase(&cDisplayedMoonPhaseInfo);
	if (validPhaseInfo == false)
	{
		CONSOLE_DEBUG("Failed to get current moon phase");
	}

	SetupWindowControls();
	UpdateButtons();

	cMoonPhaseImageCnt	=	NASA_GetMoonImageCount(0);
	sprintf(textBuff, "Moon phase image cnt=%d", cMoonPhaseImageCnt);
	SetWidgetText(kMoonPhase_TotalImgCnt, textBuff);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMoonPhase::~WindowTabMoonPhase(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

#define	kMoonPhaseInfoBoxWidth	200
//**************************************************************************************
void	WindowTabMoonPhase::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		save_yLoc;
int		fileName_yloc;
int		graphWidth;
int		graphHeight;
int		buttonWidth;
int		xCenter;
int		checkBoxWidth;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kMoonPhase_Title, -1, yLoc, "NASA Moon Phase Chart");
//	yLoc	=	SetTitleBox(kMoonPhase_GraphTitle, -1, yLoc, "Moon Phase (percent illumination)");

	//------------------------------------------
	iii				=	kMoonPhase_EnablePhaseBtn;
	checkBoxWidth	=	105;
	xLoc			=	10;
	while (iii <= kMoonPhase_EnableELonBtn)
	{
		SetWidget(			iii,	xLoc,	yLoc,	checkBoxWidth,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_CheckBox);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		xLoc	+=	checkBoxWidth;
		xLoc	+=	2;
		iii++;
	}
	SetWidgetText(		kMoonPhase_EnablePhaseBtn,	"% Illum");
	SetWidgetTextColor(	kMoonPhase_EnablePhaseBtn,	cGraphColor_Phase);

	SetWidgetText(		kMoonPhase_EnableAgeBtn,	"Age");
	SetWidgetTextColor(	kMoonPhase_EnableAgeBtn,	cGraphColor_Age);

	SetWidgetText(		kMoonPhase_EnableDiamBtn,	"Diameter");
	SetWidgetTextColor(	kMoonPhase_EnableDiamBtn,	cGraphColor_Diam);

	SetWidgetText(		kMoonPhase_EnableDistBtn,	"Distance");
	SetWidgetTextColor(	kMoonPhase_EnableDistBtn,	cGraphColor_Dist);

	SetWidgetText(		kMoonPhase_EnableAxisBtn,	"Axis");
	SetWidgetTextColor(	kMoonPhase_EnableAxisBtn,	cGraphColor_AxisA);

	SetWidgetText(		kMoonPhase_EnableELatBtn,	"ELat");
	SetWidgetTextColor(	kMoonPhase_EnableELatBtn,	cGraphColor_ELat);

	SetWidgetText(		kMoonPhase_EnableELonBtn,	"ELon");
	SetWidgetTextColor(	kMoonPhase_EnableELonBtn,	cGraphColor_ELon);

	//------------------------------------------
	//*	year display and buttons
	#define	kYearBoxWidth	125
	#define	kButtonWidth	40

//	xCenter	=	cWidth / 2;
//	xLoc	=	xCenter - (kYearBoxWidth / 2);
//	xLoc	-=	kButtonWidth;
//	xLoc	-=	2;
//	SetWidget(			kMoonPhase_YearLftBtn,	xLoc,	yLoc,	kButtonWidth,	cBtnHeight);
//	SetWidgetType(		kMoonPhase_YearLftBtn,	kWidgetType_Button);
//	SetWidgetText(		kMoonPhase_YearLftBtn,	"-");
//	SetWidgetBGColor(	kMoonPhase_YearLftBtn,	CV_RGB(255,	255,	255));
//	SetWidgetTextColor(	kMoonPhase_YearLftBtn,	CV_RGB(0,	0,	0));

//	xLoc	=	xCenter - (kYearBoxWidth / 2);
	xLoc	+=	100;
	SetWidget(			kMoonPhase_YearText,	xLoc,	yLoc,	kYearBoxWidth,	cBtnHeight);
	SetWidgetType(		kMoonPhase_YearText,	kWidgetType_TextBox);
	SetWidgetBorder(	kMoonPhase_YearText,	false);
	SetWidgetNumber(	kMoonPhase_YearText,	2024);

//	xLoc	=	xCenter + (kYearBoxWidth / 2);
//	xLoc	+=	2;
//	SetWidget(			kMoonPhase_YearRgtBtn,	xLoc,	yLoc,	kButtonWidth,	cBtnHeight);
//	SetWidgetType(		kMoonPhase_YearRgtBtn,	kWidgetType_Button);
//	SetWidgetText(		kMoonPhase_YearRgtBtn,	"+");
//	SetWidgetBGColor(	kMoonPhase_YearRgtBtn,	CV_RGB(255,	255,	255));
//	SetWidgetTextColor(	kMoonPhase_YearRgtBtn,	CV_RGB(0,	0,	0));
//
//	SetWidgetOutlineBox(kMoonPhase_YearOutLine, kMoonPhase_YearLftBtn, (kMoonPhase_YearOutLine - 1));

	//------------------------------------------
	//*	set up the web help button
	buttonWidth	=	2 * kButtonWidth;
	xLoc		=	cWidth - buttonWidth;
	xLoc		-=	2;
	SetWidget(			kMoonPhase_HelpBtn,	xLoc,	yLoc,	buttonWidth,	cBtnHeight);
	SetWidgetType(		kMoonPhase_HelpBtn,	kWidgetType_Button);
	SetWidgetBGColor(	kMoonPhase_HelpBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kMoonPhase_HelpBtn,	CV_RGB(0,	0,	0));
	SetWidgetText(		kMoonPhase_HelpBtn,	"Help");


	yLoc	+=	cBtnHeight;
	yLoc	+=	2;
	//------------------------------------------
	xLoc		=	3;
	graphWidth	=	cWidth - 6;
	graphHeight	=	210;

	SetWidget(		kMoonPhase_MoonPhaseGraph,	xLoc,	yLoc,	graphWidth,	graphHeight);
	SetWidgetType(	kMoonPhase_MoonPhaseGraph,	kWidgetType_CustomGraphic);

	yLoc	+=	graphHeight;
	yLoc	+=	cRadioBtnHt;
	yLoc	+=	cRadioBtnHt;
//	yLoc	+=	2;

	save_yLoc	=	yLoc;
	//-------------------------------------------------
	xLoc		=	5;
	yLoc	=	SetupPhaseInfoBlock(xLoc,
									yLoc,
									kMoonPhase_Curr_PhaseBlockTitle,
									(kMoonPhase_Curr_PhaseBlockTitle + 1),
									kMoonPhase_Curr_OutLineBox);
	SetWidgetText(		kMoonPhase_Curr_PhaseBlockTitle,	"Current Moon Info");

	SetWidgetTextColor(	kMoonPhase_Curr_PhaseBlockTitle,	cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_DateValue,			cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_TimeValue,			cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_PhaseNameValue,		cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_PhasePercentValue,	cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_AgeValue,			cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_DiameterValue,		cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_DistanceValue,		cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_PolarAxisValue,		cGraphColor_Current);
	SetWidgetTextColor(	kMoonPhase_Curr_ELatELonValue,		cGraphColor_Current);

	//-------------------------------------------------------------
	//*	set the box for the moon drawing
	xCenter				=	cWidth / 2;
	cMoonDisplaySize	=	cHeight - save_yLoc;
	cMoonDisplaySize	-=	18;
	xLoc				=	xCenter - (cMoonDisplaySize / 2);
	yLoc				=	save_yLoc;
	SetWidget(			kMoonPhase_MoonBox,	xLoc,	yLoc,	cMoonDisplaySize,	cMoonDisplaySize);
	SetWidgetType(		kMoonPhase_MoonBox,	kWidgetType_Custom);

	SetWidget(			kMoonPhase_MoonBoxGraphicOverlay,	xLoc,	yLoc,	cMoonDisplaySize,	cMoonDisplaySize);
	SetWidgetType(		kMoonPhase_MoonBoxGraphicOverlay,	kWidgetType_Custom);

	//----------------------------------------------------
	fileName_yloc	=	cHeight - cBtnHeight;
	fileName_yloc	-=	1;
	SetWidget(		kMoonPhase_MoonFileName,	xLoc,	fileName_yloc,	cMoonDisplaySize,	cBtnHeight);
	SetWidgetType(	kMoonPhase_MoonFileName,	kWidgetType_TextBox);

	xLoc	+=	cMoonDisplaySize;
	xLoc	+=	2;
	SetWidget(		kMoonPhase_OverlayChkBox,	xLoc,	fileName_yloc,	cMoonDisplaySize,	cRadioBtnHt);
	SetWidgetType(	kMoonPhase_OverlayChkBox,	kWidgetType_CheckBox);
	SetWidgetFont(	kMoonPhase_OverlayChkBox,	kFont_RadioBtn);
	SetWidgetText(	kMoonPhase_OverlayChkBox,	"Enable Overlay");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	6;

	//-------------------------------------------------------------
//	xLoc	=	xCenter + (cMoonDisplaySize / 2) + 40;
	xLoc	=	cWidth - (kMoonPhaseInfoBoxWidth * 2);
	xLoc	-=	6;
	yLoc	=	SetupPhaseInfoBlock(xLoc,
									save_yLoc,
									kMoonPhase_Disp_PhaseBlockTitle,
									(kMoonPhase_Disp_PhaseBlockTitle + 1),
									kMoonPhase_Disp_OutLineBox);
	SetWidgetText(		kMoonPhase_Disp_PhaseBlockTitle,	"Displayed Moon Info");
	SetWidgetTextColor(	kMoonPhase_Disp_PhaseBlockTitle,	cGraphColor_Displayed);
	yLoc			+=	2;

	//-------------------------------------------------------------
//	xLoc	=	5 + kMoonPhaseInfoBoxWidth + 2;
	SetWidget(			kMoonPhase_TodayButton,	xLoc,	yLoc,	kMoonPhaseInfoBoxWidth,	cBtnHeight);
	SetWidgetType(		kMoonPhase_TodayButton,	kWidgetType_Button);
	SetWidgetText(		kMoonPhase_TodayButton,	"Today");
	SetWidgetBGColor(	kMoonPhase_TodayButton,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kMoonPhase_TodayButton,	CV_RGB(0,	0,	0));


	//-------------------------------------------------------------
//	xLoc	=	cWidth - (kMoonPhaseInfoBoxWidth * 2);
//	xLoc	-=	2;
	xLoc	+=	kMoonPhaseInfoBoxWidth;
	xLoc	+=	2;
	SetWidget(			kMoonPhase_RunButton,	xLoc,	yLoc,	kMoonPhaseInfoBoxWidth,	cBtnHeight);
	SetWidgetType(		kMoonPhase_RunButton,	kWidgetType_Button);
	SetWidgetText(		kMoonPhase_RunButton,	"Run");
	SetWidgetBGColor(	kMoonPhase_RunButton,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kMoonPhase_RunButton,	CV_RGB(0,	0,	0));

//	SetWidgetTextColor(kMoonPhase_Curr_PhaseNameLbl,	cGraphColor_Phase);
//	SetWidgetTextColor(kMoonPhase_Curr_AgeLbl,			cGraphColor_Age);
//	SetWidgetTextColor(kMoonPhase_Curr_DiameterLbl,		cGraphColor_Diam);
//	SetWidgetTextColor(kMoonPhase_Curr_DistanceLbl,		cGraphColor_Dist);

	//-------------------------------------------------------------
	//*	now start at the bottom left and put the file information
	xLoc	=	5;
	yLoc	=	cHeight - cBtnHeight;
	yLoc	-=	2;
	SetWidget(			kMoonPhase_TotalImgCnt,	xLoc,	yLoc,	(kMoonPhaseInfoBoxWidth * 2),	cBtnHeight);
	SetWidgetType(		kMoonPhase_TotalImgCnt,	kWidgetType_TextBox);
	SetWidgetFont(		kMoonPhase_TotalImgCnt,	kFont_TextList);
	SetWidgetText(		kMoonPhase_TotalImgCnt,	"image cnt");
	SetWidgetJustification(kMoonPhase_TotalImgCnt, kJustification_Left);
	yLoc	-=	cBtnHeight;
	yLoc	-=	2;

	SetWidget(			kMoonPhase_StartDownLoadBtn,	xLoc,	yLoc,	kMoonPhaseInfoBoxWidth,	cBtnHeight);
	SetWidgetType(		kMoonPhase_StartDownLoadBtn,	kWidgetType_Button);
	SetWidgetFont(		kMoonPhase_StartDownLoadBtn,	kFont_TextList);
	SetWidgetBGColor(	kMoonPhase_StartDownLoadBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kMoonPhase_StartDownLoadBtn,	CV_RGB(0,	0,	0));
	SetWidgetText(		kMoonPhase_StartDownLoadBtn,	"Start Download");
//	kMoonPhase_DownLoadStatus,

//	SetAlpacaLogoBottomCorner(kMoonPhase_AlpacaLogo);
}


//*****************************************************************************
void	WindowTabMoonPhase::ActivateWindow(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
int	WindowTabMoonPhase::SetupPhaseInfoBlock(	int		xLoc_start,
												int		yLoc_start,
												int		titleIdx,
												int		fistLblIdx,
												int		outlineIdx)
{
int		boxWidth;
int		xLoc;
int		yLoc;
int		iii;
int		myButtonHeight;

//	CONSOLE_DEBUG(__FUNCTION__);

	//-------------------------------------------------
	boxWidth		=	kMoonPhaseInfoBoxWidth;
	myButtonHeight	=	30;
	xLoc			=	xLoc_start;
	yLoc			=	yLoc_start;
	SetWidget(			titleIdx,	xLoc,	yLoc,	(boxWidth * 2),	myButtonHeight);
	SetWidgetType(		titleIdx,	kWidgetType_TextBox);
	SetWidgetFont(		titleIdx,	kFont_Medium);
	SetWidgetTextColor(	titleIdx,	CV_RGB(255,	255, 255));
	yLoc	+=	myButtonHeight;
	yLoc	+=	2;

	iii		=	fistLblIdx;
	while (iii < outlineIdx)
	{
		xLoc	=	xLoc_start;
		SetWidget(			iii,	xLoc,	yLoc,	boxWidth,	myButtonHeight);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetTextColor(	iii,	CV_RGB(255,	255, 255));
		xLoc	+=	boxWidth;
		xLoc	+=	2;
		iii++;

		SetWidget(			iii,	xLoc,	yLoc,	boxWidth,	myButtonHeight);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetTextColor(	iii,	CV_RGB(255,	255, 255));
		xLoc	+=	boxWidth;
		xLoc	+=	2;
		iii++;

		yLoc	+=	myButtonHeight;
		yLoc	+=	2;
	}
	iii		=	fistLblIdx;
	SetWidgetText(iii,	"Date");				iii	+=	2;
	SetWidgetText(iii,	"Time (UTC)");			iii	+=	2;
	SetWidgetText(iii,	"Phase");				iii	+=	2;
	SetWidgetText(iii,	"% Illumination");		iii	+=	2;
	SetWidgetText(iii,	"Age");					iii	+=	2;
	SetWidgetText(iii,	"Diameter (arc-secs)");	iii	+=	2;
	SetWidgetText(iii,	"Distance (km)");		iii	+=	2;
	SetWidgetText(iii,	"Polar Axis Angle");	iii	+=	2;
	SetWidgetText(iii,	"ELat / ELon");			iii	+=	2;

	SetWidgetOutlineBox(outlineIdx, titleIdx, (outlineIdx - 1));

	return(yLoc);
}

//*****************************************************************************
void	WindowTabMoonPhase::RunWindowBackgroundTasks(void)
{
char	lastImageDownloaded[64];
bool	downloadTaskIsRunning;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cPhaseSimulationEnabled)
	{
		if ((cPhaseIndex_Displayed >= 0) && (cPhaseIndex_Displayed < gMoonPhaseCnt))
		{
			cDisplayedMoonPhaseInfo	=	gMoonPhaseInfo[cPhaseIndex_Displayed];
			cPhaseIndex_Displayed	+=  cPhaseStepValue;
		}
		else
		{
			cPhaseIndex_Displayed	=	0;
			cPhaseSimulationEnabled	=	false;
			//*	turn off the fast background mode
			SetRunFastBackgroundMode(cPhaseSimulationEnabled);
		}
//		if (cDisplayCurrentPhase)
		{
			UpdateMoonImage(cDisplayedMoonPhaseInfo.Date_Year,
							cDisplayedMoonPhaseInfo.Date_Month,
							cDisplayedMoonPhaseInfo.Date_DOM,
							cDisplayedMoonPhaseInfo.Time_Hour);
		}
	}
	UpdateMoonPhaseTables();

	//*	check downloading status
	downloadTaskIsRunning	=	NASA_GetLatestDownLoadImageName(lastImageDownloaded);
	if (downloadTaskIsRunning)
	{
		SetWidgetText(kMoonPhase_TotalImgCnt, lastImageDownloaded);
	}

	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabMoonPhase::HandleKeyDown(const int keyPressed)
{
bool	updateDisplayedMoon;
int		theExtendedChar;

//	CONSOLE_DEBUG_W_HEX("keyPressed=", keyPressed);

	updateDisplayedMoon	=	false;
	theExtendedChar		=	keyPressed & 0x00ffff;
	switch(theExtendedChar)
	{
		//*	return, open current selected entry
		case 0x0d:
		case 0xff8d:
			break;

		//*	delete key, delete the current entry
		case 0x0000ff:
		case 0x00ffff:
		case 0x0080ff:
			break;

		//*	up arrow key
		case 0x00ff52:
			cPhaseIndex_Displayed	+=  1;
			updateDisplayedMoon	=	true;
			break;

		//*	down arrow key
		case 0x00ff54:
			cPhaseIndex_Displayed	-=  1;
			updateDisplayedMoon	=	true;
			break;

		case 0x00FF51:	//*	left arrow
			cPhaseIndex_Displayed	-=  cPhaseStepValue;
			updateDisplayedMoon	=	true;
			break;

		case 0x00FF53:	//*	right arrow
			cPhaseIndex_Displayed	+=  cPhaseStepValue;
			updateDisplayedMoon	=	true;
			break;

		default:
			CONSOLE_DEBUG_W_HEX("Ignored: keyPressed     \t=", keyPressed);
			CONSOLE_DEBUG_W_HEX("Ignored: theExtendedChar\t=", theExtendedChar);
			break;
	}
	if (updateDisplayedMoon)
	{
		if (cPhaseIndex_Displayed < 0)
		{
			cPhaseIndex_Displayed	=	0;
		}
		if (cPhaseIndex_Displayed > (366 * 24))
		{
			cPhaseIndex_Displayed	=	366 * 24;
		}
//		CONSOLE_DEBUG_W_NUM("cPhaseIndex_Displayed\t=", cPhaseIndex_Displayed);
		//*	an arrow key has been pressed to change which moon is displayed
		cDisplayCurrentPhase	=	false;
		cDisplayedMoonPhaseInfo	=	gMoonPhaseInfo[cPhaseIndex_Displayed];
		UpdateMoonImage(cDisplayedMoonPhaseInfo.Date_Year,
						cDisplayedMoonPhaseInfo.Date_Month,
						cDisplayedMoonPhaseInfo.Date_DOM,
						cDisplayedMoonPhaseInfo.Time_Hour);
	}
}

//*****************************************************************************
void	WindowTabMoonPhase::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	updateFlag;

//	CONSOLE_DEBUG(__FUNCTION__);
	updateFlag	=	true;
	switch(buttonIdx)
	{
		case kMoonPhase_EnablePhaseBtn:
			cEnableGraph_Phase	=	!cEnableGraph_Phase;
			break;

		case kMoonPhase_EnableAgeBtn:
			cEnableGraph_Age	=	!cEnableGraph_Age;
			break;

		case kMoonPhase_EnableDiamBtn:
			cEnableGraph_Diam	=	!cEnableGraph_Diam;
			break;

		case kMoonPhase_EnableDistBtn:
			cEnableGraph_Dist	=	!cEnableGraph_Dist;
			break;

		case kMoonPhase_EnableAxisBtn:
			cEnableGraph_AxisA	=	!cEnableGraph_AxisA;
			break;

		case kMoonPhase_EnableELatBtn:
			cEnableGraph_ELat	=	!cEnableGraph_ELat;
			break;

		case kMoonPhase_EnableELonBtn:
			cEnableGraph_ELon	=	!cEnableGraph_ELon;
			break;


		case kMoonPhase_OverlayChkBox:
			cEnableOverlay	=	!cEnableOverlay;
			break;

//		case kMoonPhase_YearLftBtn:
//		case kMoonPhase_YearRgtBtn:
//			break;

		case kMoonPhase_HelpBtn:
			LaunchWebHelp();
			break;

		case kMoonPhase_TodayButton:
			cDisplayCurrentPhase	=	true;
			cPhaseSimulationEnabled	=	false;
			UpdateCurrentMoonPhase(&cDisplayedMoonPhaseInfo);
			cPhaseIndex_Displayed	=	cPhaseIndex_Now;
			break;

		case kMoonPhase_RunButton:
			cPhaseSimulationEnabled	=	!cPhaseSimulationEnabled;
			SetRunFastBackgroundMode(cPhaseSimulationEnabled);
//			CONSOLE_DEBUG_W_BOOL("cPhaseSimulationEnabled\t=", cPhaseSimulationEnabled);
			if (cPhaseSimulationEnabled)
			{
				cDisplayCurrentPhase	=	false;
			}
			break;

		case kMoonPhase_StartDownLoadBtn:
			if (gMoonPhaseCnt < 8000)
			{
				NASA_DownloadMoonPhaseData();
			}
			NASA_StartMoonImageDownloadThread(2024);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
		UpdateButtons();
		ForceWindowUpdate();
	}
}

//**************************************************************************************
void	WindowTabMoonPhase::UpdateButtons(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetChecked(kMoonPhase_EnablePhaseBtn,	cEnableGraph_Phase);
	SetWidgetChecked(kMoonPhase_EnableDiamBtn,	cEnableGraph_Diam);
	SetWidgetChecked(kMoonPhase_EnableDistBtn,	cEnableGraph_Dist);
	SetWidgetChecked(kMoonPhase_EnableAgeBtn,	cEnableGraph_Age);
	SetWidgetChecked(kMoonPhase_EnableAxisBtn,	cEnableGraph_AxisA);

	SetWidgetChecked(kMoonPhase_EnableELatBtn,	cEnableGraph_ELat);
	SetWidgetChecked(kMoonPhase_EnableELonBtn,	cEnableGraph_ELon);


	SetWidgetChecked(kMoonPhase_OverlayChkBox,	cEnableOverlay);


}

//*****************************************************************************
void	WindowTabMoonPhase::ProcessDoubleClick(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
ControllerImage		*imageWindowController;
char				textBuffer[80];
//	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kMoonPhase_MoonBox:
		case kMoonPhase_MoonFileName:
			imageWindowController	=	new ControllerImage(cMoonImageName,
															cMoonImagePath);
			if (imageWindowController != NULL)
			{
				imageWindowController->SaveFitsHeaderLine("This JPEG image is from NASA Moon phase project");
				imageWindowController->SaveFitsHeaderLine("https://svs.gsfc.nasa.gov/gallery/moonphase/");
				imageWindowController->SaveFitsHeaderLine(cMoonImagePath);
				sprintf(textBuffer, "%4d-%s-%02d-%02d:%02d:%02dZ",
										cDisplayedMoonPhaseInfo.Date_Year,
										cDisplayedMoonPhaseInfo.MonthName,
										cDisplayedMoonPhaseInfo.Date_DOM,
										cDisplayedMoonPhaseInfo.Time_Hour,
										cDisplayedMoonPhaseInfo.Time_Minute,
										cDisplayedMoonPhaseInfo.Time_Second);
				imageWindowController->SaveFitsHeaderLine(textBuffer);
				imageWindowController->SaveFitsHeaderLine(cDisplayedMoonPhaseInfo.PhaseName);

				sprintf(textBuffer, "Illumination: %3.2f %%", cDisplayedMoonPhaseInfo.Phase);
				imageWindowController->SaveFitsHeaderLine(textBuffer);

				sprintf(textBuffer, "Age: %3.2f days", cDisplayedMoonPhaseInfo.Age);
				imageWindowController->SaveFitsHeaderLine(textBuffer);

				sprintf(textBuffer, "Diameter: %2.1f arc-seconds", cDisplayedMoonPhaseInfo.Diam);
				imageWindowController->SaveFitsHeaderLine(textBuffer);

				sprintf(textBuffer, "Distance:%d km", cDisplayedMoonPhaseInfo.Dist);
				imageWindowController->SaveFitsHeaderLine(textBuffer);

				sprintf(textBuffer, "Polar Axis: %3.2f  degrees", cDisplayedMoonPhaseInfo.AxisA);
				imageWindowController->SaveFitsHeaderLine(textBuffer);

				imageWindowController->UpdateFitsHeader();
			}
			break;
	}
}

//*****************************************************************************
void	WindowTabMoonPhase::HandleMouseMovedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY)
{
//int				time_TotalMin;
//int				time_Hours;
//int				time_Minutes;
//int				temp_degF;
//char			msgString[400];
//char			shortString[32];
//int				iii;
//TYPE_CPU_SORT	myCPUnameList[kMaxCPUs];
//
//	//*	each pixel is worth 2 minutes
//	time_TotalMin	=	box_XXX * 2;
//	time_Hours		=	time_TotalMin / 60;
//	time_Minutes	=	time_TotalMin % 60;
//	temp_degF		=	theWidget->height - box_YYY;
//	temp_degF		-=	4;
//
//	//*	make sure we are in valid data territory
//	if ((temp_degF >= 0) && (time_TotalMin < (24 * 60)))
//	{
//		sprintf(msgString, "Time: %02d:%02d, ", time_Hours, time_Minutes);
//
//		temp_degF		=	theWidget->height - box_YYY;
//		temp_degF		-=	4;
//
//		//*	get a list of the valid temperatures
//		cpuIdx	=	0;
//		for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
//		{
//			if (gAlpacaUnitList[iii].cpuTempValid)
//			{
//				cpuTemp		=	gAlpacaUnitList[iii].cpuTempLog[box_XXX];
//				if (cpuTemp > 20)
//				{
//					myCPUnameList[cpuIdx].cpuTemp		=	gAlpacaUnitList[iii].cpuTempLog[box_XXX];
//					if (strlen(gAlpacaUnitList[iii].hostName) > 0)
//					{
//						strcpy(myCPUnameList[cpuIdx].cpuName, gAlpacaUnitList[iii].hostName);
//					}
//					else
//					{
//						inet_ntop(AF_INET, &(gAlpacaUnitList[iii].deviceAddress.sin_addr), myCPUnameList[cpuIdx].cpuName, INET_ADDRSTRLEN);
//					}
//					cpuIdx++;
//				}
//			}
//		}
//
//		//*	now we have a list, sort it.
//		qsort(myCPUnameList, cpuIdx, sizeof(TYPE_CPU_SORT), CPUtempSortProc);
//
//		//*	now build the string to display
//		//*	the list is in descending order
//		for (iii=0; iii<cpuIdx; iii++)
//		{
//			sprintf(shortString, " %s=%2.1f",	myCPUnameList[iii].cpuName,
//												myCPUnameList[iii].cpuTemp);
//			strcat(msgString, shortString);
//		}
//		SetWidgetText(kMoonPhase_StatusMsg, msgString);
//		ForceWindowUpdate();
//	}
//	else
//	{
//	//		CONSOLE_DEBUG(__FUNCTION__);
//	}
}

//*****************************************************************************
void	WindowTabMoonPhase::HandleMouseClickedInGraph(	TYPE_WIDGET *theWidget,
														const int	box_XXX,
														const int	box_YYY)
{
int		phaseIndex;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	figure out the phase index based on where the click occured
	phaseIndex	=	(box_XXX - theWidget->left) * cPhaseStepValue;

	if ((phaseIndex >=0) && (phaseIndex < gMoonPhaseCnt))
	{
		cPhaseIndex_Displayed	=	phaseIndex;
		cDisplayedMoonPhaseInfo	=	gMoonPhaseInfo[phaseIndex];
		cDisplayCurrentPhase	=	false;
		UpdateMoonImage(cDisplayedMoonPhaseInfo.Date_Year,
						cDisplayedMoonPhaseInfo.Date_Month,
						cDisplayedMoonPhaseInfo.Date_DOM,
						cDisplayedMoonPhaseInfo.Time_Hour);
		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG("We should not be here, the phase index is out of bounds");
		CONSOLE_DEBUG_W_NUM("phaseIndex   \t=", phaseIndex);
		CONSOLE_DEBUG_W_NUM("gMoonPhaseCnt\t=", gMoonPhaseCnt);
	}
}

//*****************************************************************************
void	WindowTabMoonPhase::ProcessMouseEvent(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
int		box_XXX;
int		box_YYY;

	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			if (widgetIdx == kMoonPhase_MoonPhaseGraph)
			{
				//*	compute the offsets within the box
				box_XXX	=	xxx - cWidgetList[widgetIdx].left;
				box_YYY	=	yyy - cWidgetList[widgetIdx].top;
				HandleMouseMovedInGraph(&cWidgetList[widgetIdx], box_XXX, box_YYY);
			}
			break;

		case cv::EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONDOWN", widgetIdx);
			cLeftButtonDown	=	true;
			break;

		case cv::EVENT_LBUTTONUP:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONUP", widgetIdx);
			cLeftButtonDown	=	false;
			if (widgetIdx == kMoonPhase_MoonPhaseGraph)
			{
				//*	compute the offsets within the box
				box_XXX	=	xxx - cWidgetList[widgetIdx].left;
				box_YYY	=	yyy - cWidgetList[widgetIdx].top;
				HandleMouseClickedInGraph(&cWidgetList[widgetIdx], box_XXX, box_YYY);
			}
			break;

//
//		case cv::EVENT_RBUTTONDOWN:
//			cRightButtonDown		=	true;
//			break;
//
//		case cv::EVENT_MBUTTONDOWN:
//			break;
//
//		case cv::EVENT_RBUTTONUP:
//			cRightButtonDown		=	false;
//			break;
//
//		case cv::EVENT_MBUTTONUP:
//			break;
//
//		case cv::EVENT_LBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_RBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_MBUTTONDBLCLK:
//			break;
//
#if (CV_MAJOR_VERSION >= 3)
		case cv::EVENT_MOUSEWHEEL:
			break;
		case cv::EVENT_MOUSEHWHEEL:
			break;
#endif
		default:
//			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
	}
}

//*****************************************************************************
void	WindowTabMoonPhase::ProcessMouseWheelMoved(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	wheelMovement,
													const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

//	//*	make sure we have something to scroll
//	if (gAlpacaUnitCnt > 0)
//	{
//		cFirstLineIdx	+=	wheelMovement;
//		if (cFirstLineIdx < 0)
//		{
//			cFirstLineIdx	=	0;
//		}
//		if (cFirstLineIdx >= gAlpacaUnitCnt)
//		{
//			cFirstLineIdx	=	gAlpacaUnitCnt -1;
//		}
//		UpdateOnScreenWidgetList();
//		ForceWindowUpdate();
//	}
}


//**************************************************************************************
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
void	WindowTabMoonPhase::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
void	WindowTabMoonPhase::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kMoonPhase_MoonPhaseGraph:
			DrawMoonPhaseGraph(&cWidgetList[widgetIdx]);
			break;

		case kMoonPhase_MoonBoxGraphicOverlay:
			if (cEnableOverlay)
			{
				DrawMoonWidget(&cWidgetList[widgetIdx]);
			}
			break;

		case kMoonPhase_MoonBox:
//			CONSOLE_DEBUG_W_NUM("kMoonPhase_MoonBox\t",	widgetIdx);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

#define	TRANSLATE_Y(rect, ddd)	((rect->y + rect->height - 4) - ddd)

//**************************************************************************************
static int	TranslateYvalue(cv::Rect	*myCVrect, double yValue)
{
int	myYvalue;

//	if (yValue > 50)
//	{
//		myYvalue	=	TRANSLATE_Y(myCVrect, (2 * yValue));
//		myYvalue	=	myYvalue + 125;
//	}
//	else
	{
		myYvalue	=	TRANSLATE_Y(myCVrect, yValue);
	}
	return(myYvalue);
}

//**************************************************************************************
void	WindowTabMoonPhase::DrawMoonPhaseGraph(TYPE_WIDGET *theWidget)
{
cv::Rect		myCVrect;
int				previousX;
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
int				graphIdx;
int				graphValue;
double			graphValueDbl;
int				previousGraphValue;
char			previousMonthStr[16]	=	"";
struct timeval	currentTimeVal;
struct tm		*linuxTime;
int				currYear;
int				currMonth;
int				currDay;
int				currHour;
bool			validPhaseInfo;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	if (gMoonPhaseCnt > 0)
	{
		//*	calculate the horizontal scale factor for the graph
		cPhaseStepValue		=	gMoonPhaseCnt / myCVrect.width;
		cPhaseStepValue		+=	1;
		//=========================================================
		//*	draw months and tick marks
		LLG_SetColor(W_WHITE);
		previousX			=	theWidget->left;
		pt1_X				=	0;
		graphIdx			=	0;
		while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
		{
			pt1_X		=	previousX;
			pt2_X		=	previousX + 1;
			previousX	=	pt2_X;
			graphIdx	+=	cPhaseStepValue;
			//*	Check for month change
			if (strcmp(gMoonPhaseInfo[graphIdx].MonthName, previousMonthStr) != 0)
			{
				//*	draw a month label
				pt2_Y	=	myCVrect.y + myCVrect.height + 15;
				LLG_DrawCString(pt2_X + 4, pt2_Y, gMoonPhaseInfo[graphIdx].MonthName, kFont_Medium);
				strcpy(previousMonthStr, gMoonPhaseInfo[graphIdx].MonthName);

				//*	draw a tick mark
				pt1_Y		=	myCVrect.y + myCVrect.height;
				pt2_Y		=	pt1_Y + 25;
				LLG_MoveTo(pt2_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);
			}
		}

		//=========================================================
		//*	now draw a vertical line for the CURRENT time
		gettimeofday(&currentTimeVal, NULL);
		linuxTime	=	gmtime(&currentTimeVal.tv_sec);

		currYear	=	(1900 + linuxTime->tm_year);
		currMonth	=	(1 + linuxTime->tm_mon);
		currDay		=	linuxTime->tm_mday;
		currHour	=	linuxTime->tm_hour;
		graphIdx	=	NASA_GetPhaseIndex(currYear, currMonth, currDay, currHour);
//		CONSOLE_DEBUG_W_NUM("graphIdx\t=", graphIdx);
		if (graphIdx >= 0)
		{
			cPhaseIndex_Now	=	graphIdx;
			graphIdx		=	graphIdx / cPhaseStepValue;
			pt1_X			=	myCVrect.x + graphIdx;
			pt1_Y			=	myCVrect.y;
			pt2_X			=	myCVrect.x + graphIdx;
			pt2_Y			=	myCVrect.y + myCVrect.height;
			LLG_SetColor(cGraphColor_Current);
//			LLG_PenSize(2);
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt2_X, pt2_Y);
//			LLG_PenSize(1);
		}
		if (cDisplayCurrentPhase)
		{
			UpdateMoonImage(currYear, currMonth, currDay, currHour);
		}

		//=========================================================
		//*	Draw the phase graph
		if (cEnableGraph_Phase)
		{
			LLG_SetColor(cGraphColor_Phase);
			previousGraphValue	=	gMoonPhaseInfo[0].Phase * 2;
			graphIdx			=	0;
			previousX			=	theWidget->left;
			pt1_X				=	0;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValue	=   gMoonPhaseInfo[graphIdx].Phase * 2;
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
		//=========================================================
		//*	Draw the age graph
		if (cEnableGraph_Age)
		{
			LLG_SetColor(cGraphColor_Age);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			pt1_X				=	0;
			previousGraphValue	=	gMoonPhaseInfo[graphIdx].Age * 3;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValue	=   gMoonPhaseInfo[graphIdx].Age * 3;
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
		//=========================================================
		//*	Draw the diameter graph
		if (cEnableGraph_Diam)
		{
			LLG_SetColor(cGraphColor_Diam);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			pt1_X				=	0;
			previousGraphValue	=	(gMoonPhaseInfo[graphIdx].Diam - 1500) / 3;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValue	=   (gMoonPhaseInfo[graphIdx].Diam - 1500) / 3;
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
		//=========================================================
		//*	Draw the distance graph
		if (cEnableGraph_Dist)
		{
			LLG_SetColor(cGraphColor_Dist);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			pt1_X				=	0;
			previousGraphValue	=   (gMoonPhaseInfo[graphIdx].Dist - 350000) / 300;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValue	=   (gMoonPhaseInfo[graphIdx].Dist - 350000) / 300;
	//			CONSOLE_DEBUG_W_NUM("distvalue\t=", graphValue);
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
		//=========================================================
		//*	Draw the axis graph
		if (cEnableGraph_AxisA)
		{
			LLG_SetColor(cGraphColor_AxisA);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			pt1_X				=	0;
			previousGraphValue	=   0;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValueDbl	=   gMoonPhaseInfo[graphIdx].AxisA;
				graphValue		=	100 - (graphValueDbl * 2);
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
#define	kELatELonMag	(7.0)
		//=========================================================
		//*	Draw the ELat graph (Latitude of point that Earth is directly overhead)
		if (cEnableGraph_ELat)
		{
			LLG_SetColor(cGraphColor_ELat);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			previousGraphValue	=	0;
			pt1_X				=	0;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValueDbl	=   gMoonPhaseInfo[graphIdx].ELat;
				graphValue		=	100 - (graphValueDbl * kELatELonMag);
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}
		//=========================================================
		//*	Draw the ELon graph (Longitude of point that Earth is directly overhead)
		if (cEnableGraph_ELon)
		{
			LLG_SetColor(cGraphColor_ELon);
			graphIdx			=	0;
			previousX			=	theWidget->left;
			previousGraphValue	=	0;
			pt1_X				=	0;
			while ((graphIdx < gMoonPhaseCnt) && (pt1_X < myCVrect.width))
			{
				graphValueDbl	=   gMoonPhaseInfo[graphIdx].ELon;
				graphValue		=	100 - (graphValueDbl * kELatELonMag);
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousGraphValue);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), graphValue);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX			=	pt2_X;
				previousGraphValue	=	graphValue;
				graphIdx			+=	cPhaseStepValue;
			}
		}

		//=========================================================
		if (cPhaseIndex_Displayed != cPhaseIndex_Now)
		{
			pt1_X		=	myCVrect.x + (cPhaseIndex_Displayed / cPhaseStepValue);
			pt1_Y		=	myCVrect.y;
			pt2_X		=	myCVrect.x + (cPhaseIndex_Displayed / cPhaseStepValue);
			pt2_Y		=	myCVrect.y + myCVrect.height;
			LLG_SetColor(cGraphColor_Displayed);
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt2_X, pt2_Y);
		}

		//=========================================================
		//*	fill in the values for the current time
		validPhaseInfo	=	UpdateCurrentMoonPhase(&cCurrentMoonPhaseInfo);
		if (cDisplayCurrentPhase)
		{
			cDisplayedMoonPhaseInfo	=	cCurrentMoonPhaseInfo;
		}
		if (validPhaseInfo == false)
		{
			CONSOLE_DEBUG("UpdateCurrentMoonPhase failed!!!");
		}
		UpdateMoonPhaseTables();
	}
}

//**************************************************************************************
void	WindowTabMoonPhase::DrawMoonWidget(TYPE_WIDGET *theWidget)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cMoonDisplayRect.x		=	theWidget->left;
	cMoonDisplayRect.y		=	theWidget->top;
	cMoonDisplayRect.width	=	theWidget->width;
	cMoonDisplayRect.height	=	theWidget->height;

	if (cDisplayCurrentPhase)
	{
		UpdateCurrentMoonPhase(&cDisplayedMoonPhaseInfo);
	}
	DrawMoonGraphic();
}

//**************************************************************************************
void	WindowTabMoonPhase::DrawMoonGraphic(void)
{
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
int				centerX;
int				centerY;
int				moonRadius;
double			polarAxisRadius;
double			polarAxisAngle_degrees;
double			polarAxisAngle_radians;

//	CONSOLE_DEBUG(__FUNCTION__);

	centerX			=	cMoonDisplayRect.x + (cMoonDisplayRect.width / 2);
	centerY			=	cMoonDisplayRect.y + (cMoonDisplayRect.height / 2);
	moonRadius		=	cMoonDisplayRect.width / 2;
	moonRadius		-=	20;

//	LLG_SetColor(W_BLACK);
//	LLG_FillRect(&cMoonDisplayRect);
	LLG_SetColor(W_WHITE);
//	LLG_FrameRect(&cMoonDisplayRect);
	LLG_FrameEllipse(centerX, centerY, moonRadius, moonRadius);

	//----------------------------------------------------
	polarAxisAngle_degrees	=	-90 - cDisplayedMoonPhaseInfo.AxisA;
	polarAxisAngle_radians	=	RADIANS(polarAxisAngle_degrees);
	polarAxisRadius			=	cMoonDisplayRect.width / 2;

	if (cDisplayCurrentPhase)
	{
		LLG_SetColor(cGraphColor_Current);
	}
	else
	{
		LLG_SetColor(cGraphColor_Displayed);
	}
	LLG_PenSize(3);
	pt1_X	=	centerX + (polarAxisRadius * cos(polarAxisAngle_radians));
	pt1_Y	=	centerY + (polarAxisRadius * sin(polarAxisAngle_radians));

	pt2_X	=	centerX - (polarAxisRadius * cos(polarAxisAngle_radians));
	pt2_Y	=	centerY - (polarAxisRadius * sin(polarAxisAngle_radians));
	LLG_MoveTo(pt1_X, pt1_Y);
	LLG_LineTo(pt2_X, pt2_Y);

	LLG_PenSize(1);
	LLG_SetColor(W_WHITE);

}

//**************************************************************************************
bool	WindowTabMoonPhase::UpdateCurrentMoonPhase(TYPE_MoonPhase *moonPhaseInfo)
{
struct timeval	currentTimeVal;
struct tm		*linuxTime;
int				currYear;
int				currMonth;
int				currDay;
int				currHour;
int				currMinute;
int				currSecond;
bool			validPhaseInfo;

//	CONSOLE_DEBUG(__FUNCTION__);

	gettimeofday(&currentTimeVal, NULL);
	linuxTime		=	gmtime(&currentTimeVal.tv_sec);

	currYear		=	(1900 + linuxTime->tm_year);
	currMonth		=	(1 + linuxTime->tm_mon);
	currDay			=	linuxTime->tm_mday;
	currHour		=	linuxTime->tm_hour;
	currMinute		=	linuxTime->tm_min;
	currSecond		=	linuxTime->tm_sec;
	//*	fill in the values for the current time
	validPhaseInfo	=	NASA_GetMoonPhaseInfo(	currYear,
												currMonth,
												currDay,
												currHour,
												currMinute,
												currSecond,
												moonPhaseInfo);
	return(validPhaseInfo);
}


//**************************************************************************************
void	WindowTabMoonPhase::UpdateMoonPhaseTables(void)
{
char		valueText[64];
int			textColor;
//	CONSOLE_DEBUG(__FUNCTION__);
	//---------------------------------------------------------------------
	sprintf(valueText, "%4d-%s-%02d",	cCurrentMoonPhaseInfo.Date_Year,
										cCurrentMoonPhaseInfo.MonthName,
										cCurrentMoonPhaseInfo.Date_DOM);
	SetWidgetText(		kMoonPhase_Curr_DateValue,			valueText);

	sprintf(valueText, "%02d:%02d:%02d",	cCurrentMoonPhaseInfo.Time_Hour,
											cCurrentMoonPhaseInfo.Time_Minute,
											cCurrentMoonPhaseInfo.Time_Second);
	SetWidgetText(		kMoonPhase_Curr_TimeValue,			valueText);
	SetWidgetText(		kMoonPhase_Curr_PhaseNameValue,		cCurrentMoonPhaseInfo.PhaseName);
	SetWidgetNumber(	kMoonPhase_Curr_PhasePercentValue,	cCurrentMoonPhaseInfo.Phase);
	SetWidgetNumber(	kMoonPhase_Curr_AgeValue,			cCurrentMoonPhaseInfo.Age);
	SetWidgetNumber(	kMoonPhase_Curr_DiameterValue,		cCurrentMoonPhaseInfo.Diam);
	SetWidgetNumber(	kMoonPhase_Curr_DistanceValue,		cCurrentMoonPhaseInfo.Dist);
	SetWidgetNumber(	kMoonPhase_Curr_PolarAxisValue,		cCurrentMoonPhaseInfo.AxisA);
	sprintf(valueText, "%4.3f / %4.3f",	cCurrentMoonPhaseInfo.ELat,
										cCurrentMoonPhaseInfo.ELon);
	SetWidgetText(		kMoonPhase_Curr_ELatELonValue,			valueText);

	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	sprintf(valueText, "%4d-%s-%02d",	cDisplayedMoonPhaseInfo.Date_Year,
										cDisplayedMoonPhaseInfo.MonthName,
										cDisplayedMoonPhaseInfo.Date_DOM);
	SetWidgetText(		kMoonPhase_Disp_DateValue,			valueText);

	sprintf(valueText, "%02d:%02d:%02d",	cDisplayedMoonPhaseInfo.Time_Hour,
											cDisplayedMoonPhaseInfo.Time_Minute,
											cDisplayedMoonPhaseInfo.Time_Second);
	SetWidgetText(		kMoonPhase_Disp_TimeValue,			valueText);
	SetWidgetText(		kMoonPhase_Disp_PhaseNameValue,		cDisplayedMoonPhaseInfo.PhaseName);
	SetWidgetNumber(	kMoonPhase_Disp_PhasePercentValue,	cDisplayedMoonPhaseInfo.Phase);
	SetWidgetNumber(	kMoonPhase_Disp_AgeValue,			cDisplayedMoonPhaseInfo.Age);
	SetWidgetNumber(	kMoonPhase_Disp_DiameterValue,		cDisplayedMoonPhaseInfo.Diam);
	SetWidgetNumber(	kMoonPhase_Disp_DistanceValue,		cDisplayedMoonPhaseInfo.Dist);
	SetWidgetNumber(	kMoonPhase_Disp_PolarAxisValue,		cDisplayedMoonPhaseInfo.AxisA);
	sprintf(valueText, "%4.3f / %4.3f",	cDisplayedMoonPhaseInfo.ELat,
										cDisplayedMoonPhaseInfo.ELon);
	SetWidgetText(		kMoonPhase_Disp_ELatELonValue,			valueText);


	//*	this plays with the text color to indicate if the displayed value matches the current value
	//*	current value == green text
	//*	some other value == red text
	if (cDisplayCurrentPhase)
	{
		textColor	=	cGraphColor_Current;
	}
	else
	{
		textColor	=	cGraphColor_Displayed;
	}

	SetWidgetTextColor(	kMoonPhase_Disp_PhaseBlockTitle,	textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_DateValue,			textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_TimeValue,			textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_PhaseNameValue,		textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_PhasePercentValue,	textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_AgeValue,			textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_DiameterValue,		textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_DistanceValue,		textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_PolarAxisValue,		textColor);
	SetWidgetTextColor(	kMoonPhase_Disp_ELatELonValue,		textColor);
}

//**************************************************************************************
void	WindowTabMoonPhase::UpdateMoonImage(int year, int month, int day, int hour)
{
int		phaseIndex;
bool	imageIsPresnt;
char	imageFilePath[256];
char	imageFileName[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	phaseIndex		=	NASA_GetPhaseIndex(year, month, day, hour);
//	CONSOLE_DEBUG("-------------------------------------");
//	CONSOLE_DEBUG_W_NUM("year           \t=",	year);
//	CONSOLE_DEBUG_W_NUM("month          \t=",	month);
//	CONSOLE_DEBUG_W_NUM("day            \t=",	day);
//	CONSOLE_DEBUG_W_NUM("hour           \t=",	hour);
//	CONSOLE_DEBUG_W_NUM("phaseIndex     \t=",	phaseIndex);
//	CONSOLE_DEBUG_W_NUM("cMoonImageIndex\t=",	cMoonImageIndex);
	if (phaseIndex != cMoonImageIndex)
	{
		//*	we need to update the image
		imageIsPresnt	=	NASA_GetMoonImageFilePath(year, month, day, hour, imageFilePath, imageFileName);
//		CONSOLE_DEBUG_W_BOOL("imageIsPresnt", imageIsPresnt);
		SetWidgetText(		kMoonPhase_MoonFileName, imageFileName);
		if (imageIsPresnt)
		{
			cMoonImageYear	=	year;
			strcpy(cMoonImageName,	imageFileName);
			strcpy(cMoonImagePath,	imageFilePath);
			if (cMoonOpenCVimage != NULL)
			{
//				delete cMoonOpenCVimage;
				cMoonOpenCVimage	=	NULL;
			}
//			CONSOLE_DEBUG_W_STR("Calling ReadImageIntoOpenCVimage with", imageFilePath);
			cMoonOpenCVimage	=   ReadImageIntoOpenCVimage(imageFilePath);
			if (cMoonOpenCVimage != NULL)
			{
//				CONSOLE_DEBUG("Image loaded OK");

				//*	if the displayed image has not been allocated go ahead and create it
				if (cDisplayedImage == NULL)
				{
					cDisplayedImage	=	new cv::Mat(cv::Size(	cMoonDisplaySize,
																cMoonDisplaySize),
																CV_8UC3);
				}
				if (cDisplayedImage != NULL)
				{
					//*	copy the image to the displayed image
					cv::resize(	*cMoonOpenCVimage,
								*cDisplayedImage,
								cDisplayedImage->size(),
								0,
								0,
								cv::INTER_LINEAR);

					SetWidgetType(	kMoonPhase_MoonBox,	kWidgetType_Image);
					SetWidgetImage(	kMoonPhase_MoonBox, cDisplayedImage);
				}
			}
			else
			{
				SetWidgetImage(	kMoonPhase_MoonBox, NULL);
				CONSOLE_DEBUG("Image Failed");
			}
			SetWidgetTextColor(	kMoonPhase_MoonFileName, W_GREEN);
		}
		else
		{
			SetWidgetImage(	kMoonPhase_MoonBox, NULL);
			SetWidgetTextColor(	kMoonPhase_MoonFileName, W_RED);
			CONSOLE_DEBUG_W_STR("Moon image has not been downloaded:", imageFileName);
		}
	}
//	else
//	{
//		CONSOLE_DEBUG_W_NUM("phaseIndex     \t=", phaseIndex);
//	}
	cMoonImageIndex	=	phaseIndex;
}


