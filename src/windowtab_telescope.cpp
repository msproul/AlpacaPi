//*****************************************************************************
//*		windowtab_telescope.cpp		(c) 2021 by Mark Sproul
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
//*	Jan 22,	2021	<MLS> Created windowtab_telescope.cpp
//*	Jan 23,	2021	<MLS> Started on telescope commands, abort, tracking on/off
//*	Jan 23,	2021	<MLS> Slewing working with ASCOM Telescope simulator
//*	Feb 25,	2021	<MLS> Added tracking rate to telescope control window
//*	May 12,	2022	<MLS> Added UpdateTelescopeInfo()
//*	May 12,	2022	<MLS> Added up/down east/west button logic
//*	May 12,	2022	<MLS> Added up/down east/west Normal,Slewing,Disabled colors
//*	May 12,	2022	<MLS> Added Tracking rate button handling
//*	Jun 13,	2022	<MLS> Added ProcessMovementButtons() to simplify code
//*****************************************************************************

#ifdef _ENABLE_CTRL_TELESCOPE_


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_telescope.h"

#include	"windowtab.h"
#include	"windowtab_telescope.h"


//**************************************************************************************
WindowTabTelescope::WindowTabTelescope(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cCurrentSlewRate		=	kTelescope_SlewRate_Med;
	cSlewingUp				=	false;
	cSlewingDown			=	false;
	cSlewingEast			=	false;
	cSlewingWest			=	false;

	cSlewRate_RAmin			=	0.0;
	cSlewRate_RAmax			=	7.0;

	cSlewRate_DECmin		=	0.0;
	cSlewRate_DECmax		=	7.0;
	cAtPark					=	false;
	cSlewRates_RA_valid		=	false;
	cSlewRates_DEC_valid	=	false;

	SetupWindowControls();

	UpdateButtons();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabTelescope::~WindowTabTelescope(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
void	WindowTabTelescope::SetupWindowControls(void)
{
int			xLoc;
int			yLoc;
int			yLocSave;
int			yLocSave_SlewRate;
int			iii;

//*	create our own set of column offsets
int			myClm1_offset;
//int			myClm2_offset;
//int			myClm3_offset;
int			myClm4_offset;
//int			myClm5_offset;
//int			myClm6_offset;
int			myClm7_offset;
//int		myClm8_offset;
int			myClm9_offset;
//int			myClm10_offset;
//int			myClm11_offset;
//int		myClm12_offset;
int			myClmWidth;
int			directionBtnWidth;
int			directionBtnHeight;
int			trackingBtnWidth;
int			labelWidth;
int			valueWidth;
int			label_xLoc;
int			value_xLoc;
int			value_Xloc;
int			coordHeight;

	myClmWidth		=	cWidth / 12;
	myClmWidth		-=	2;
	myClm1_offset	=	3;
//	myClm2_offset	=	1 * myClmWidth;
//	myClm3_offset	=	2 * myClmWidth;
	myClm4_offset	=	3 * myClmWidth;
//	myClm5_offset	=	4 * myClmWidth;
//	myClm6_offset	=	5 * myClmWidth;
	myClm7_offset	=	6 * myClmWidth;
//	myClm8_offset	=	7 * myClmWidth;
	myClm9_offset	=	8 * myClmWidth;
//	myClm10_offset	=	9 * myClmWidth;
//	myClm11_offset	=	10 * myClmWidth;
//	myClm12_offset	=	11 * myClmWidth;


//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kTelescope_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kTelescope_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kTelescope_Title);

	//*	setup the connected indicator
   	SetUpConnectedIndicator(kTelescope_Connected, yLoc);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLocSave		=	yLoc;


	directionBtnWidth		=	2 * myClmWidth;
	directionBtnHeight		=	3 * cTitleHeight;
	cBtnBGcolor_Normal		=	CV_RGB(32,	32,	128);
	cBtnBGcolor_Slewing		=	CV_RGB(32,	128,	32);
	cBtnBGcolor_Disabled	=	CV_RGB(128,	128,	128);
	cBtnTXTcolor			=	CV_RGB(255,	255,	255);

	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_Parked,	myClm1_offset,		yLoc,		directionBtnWidth,		cBtnHeight);
	SetWidgetText(		kTelescope_Parked,	"PARKED");
	SetWidgetType(		kTelescope_Parked,	kWidgetType_TextBox);
	SetWidgetBGColor(	kTelescope_Parked,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kTelescope_Parked,	CV_RGB(255,	0,	0));
//	SetWidgetValid(		kTelescope_Parked,	false);
	yLoc			+=	cBtnHeight;
	yLoc			+=	3;

	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_PulseGuiding,	myClm1_offset,		yLoc,		directionBtnWidth,		cBtnHeight);
	SetWidgetText(		kTelescope_PulseGuiding,	"PulseGuiding");
	SetWidgetType(		kTelescope_PulseGuiding,	kWidgetType_TextBox);
	SetWidgetBorder(	kTelescope_PulseGuiding,	false);
	SetWidgetFont(		kTelescope_PulseGuiding,	kFont_Medium);
	SetWidgetBGColor(	kTelescope_PulseGuiding,	CV_RGB(0,	0,		0));
	SetWidgetTextColor(	kTelescope_PulseGuiding,	CV_RGB(0,	255,	0));
//	SetWidgetValid(		kTelescope_PulseGuiding,	false);



	yLoc	=	yLocSave;
	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_BtnUp,	myClm4_offset,		yLoc,		directionBtnWidth,		directionBtnHeight);
	SetWidgetText(		kTelescope_BtnUp,	"Up");
	SetWidgetType(		kTelescope_BtnUp,	kWidgetType_Button);
	SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnUp,	cBtnTXTcolor);
	yLoc			+=	directionBtnHeight;
	yLoc			+=	10;

	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_BtnEast,	myClm1_offset,		yLoc,		directionBtnWidth,		directionBtnHeight);
	SetWidgetText(		kTelescope_BtnEast,	"East");
	SetWidgetType(		kTelescope_BtnEast,	kWidgetType_Button);
	SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnEast,	cBtnTXTcolor);

	SetWidget(			kTelescope_BtnEmergencyStop,	myClm4_offset,		yLoc,		directionBtnWidth,		directionBtnHeight);
	SetWidgetText(		kTelescope_BtnEmergencyStop,	"STOP");
	SetWidgetType(		kTelescope_BtnEmergencyStop,	kWidgetType_Button);
	SetWidgetBGColor(	kTelescope_BtnEmergencyStop,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kTelescope_BtnEmergencyStop,	cBtnTXTcolor);


	SetWidget(			kTelescope_BtnWest,	myClm7_offset,		yLoc,		directionBtnWidth,		directionBtnHeight);
	SetWidgetText(		kTelescope_BtnWest,	"West");
	SetWidgetType(		kTelescope_BtnWest,	kWidgetType_Button);
	SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnWest,	cBtnTXTcolor);


	yLoc			+=	directionBtnHeight;
	yLoc			+=	10;
	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_BtnDown,	myClm4_offset,		yLoc,		directionBtnWidth,		directionBtnHeight);
	SetWidgetText(		kTelescope_BtnDown,	"Down");
	SetWidgetType(		kTelescope_BtnDown,	kWidgetType_Button);
	SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnDown,	cBtnTXTcolor);


	yLoc			+=	directionBtnHeight;
	yLoc			+=	10;

	//---------------------------------------------------------------------------------------
	//*	Tracking On/Off and Status
	xLoc				=	myClm1_offset;
	trackingBtnWidth	=	(2 * myClmWidth) + 30;
	SetWidget(			kTelescope_BtnTrackingOn,	xLoc,		yLoc,		trackingBtnWidth,		cBtnHeight);
	SetWidgetType(		kTelescope_BtnTrackingOn,	kWidgetType_Button);
	SetWidgetText(		kTelescope_BtnTrackingOn,	"Tracking On");
	SetWidgetFont(		kTelescope_BtnTrackingOn,	kFont_Medium);
	SetWidgetBGColor(	kTelescope_BtnTrackingOn,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnTrackingOn,	cBtnTXTcolor);
	xLoc	+=	trackingBtnWidth;
	xLoc	+=	3;

	SetWidget(			kTelescope_BtnTrackingOff,	xLoc,		yLoc,		trackingBtnWidth,		cBtnHeight);
	SetWidgetType(		kTelescope_BtnTrackingOff,	kWidgetType_Button);
	SetWidgetText(		kTelescope_BtnTrackingOff,	"Tracking Off");
	SetWidgetFont(		kTelescope_BtnTrackingOff,	kFont_Medium);
	SetWidgetBGColor(	kTelescope_BtnTrackingOff,	cBtnBGcolor_Normal);
	SetWidgetTextColor(	kTelescope_BtnTrackingOff,	cBtnTXTcolor);
	xLoc	+=	trackingBtnWidth;
	xLoc	+=	3;

	SetWidget(			kTelescope_TrackingStatus,	xLoc,		yLoc,		trackingBtnWidth,		cBtnHeight);
	SetWidgetText(		kTelescope_TrackingStatus,	"???");
	SetWidgetFont(		kTelescope_TrackingStatus,	kFont_Medium);


	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	yLoc			+=	2;
	yLoc			+=	2;
	SetWidget(		kTelescope_ErrorMsg,	0,		yLoc,		cWidth - 100,		cBtnHeight);
	SetWidgetFont(	kTelescope_ErrorMsg,	kFont_Medium);

	//---------------------------------------------------------------------------------------
	//*	go back to the top
	yLoc			=	yLocSave;
	yLoc			+=	2;
//	labelWidth		=	(myClmWidth / 2) + (myClmWidth / 4);
	labelWidth		=	myClmWidth + myClmWidth;
//	valueWidth		=	myClmWidth + (labelWidth / 2);
	valueWidth		=	myClmWidth + myClmWidth;

	label_xLoc		=	myClm9_offset + 20;
	value_xLoc		=	label_xLoc + labelWidth + 5;
	SetWidget(				kTelescope_SlewRate_Title,	label_xLoc + 2,		yLoc,	(labelWidth + valueWidth + 2),	cRadioBtnHt);
	SetWidgetType(			kTelescope_SlewRate_Title,	kWidgetType_TextBox);
	SetWidgetFont(			kTelescope_SlewRate_Title,	kFont_RadioBtn);
	SetWidgetText(			kTelescope_SlewRate_Title,	"Slew Rate");
	SetWidgetJustification(	kTelescope_SlewRate_Title,	kJustification_Center);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	yLocSave_SlewRate	=	yLoc;
	//---------------------------------------------------------------------------------------
	for (iii = kTelescope_SlewRate_VerySlow; iii <= kTelescope_SlewRate_Fast; iii++)
	{
		SetWidget(			iii,	label_xLoc + 2,		yLoc,	labelWidth,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		SetWidgetBorder(	iii,	true);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kTelescope_SlewRate_VerySlow,	"Very Slow");
	SetWidgetText(		kTelescope_SlewRate_Slow,		"Slow");
	SetWidgetText(		kTelescope_SlewRate_Med,		"Medium");
	SetWidgetText(		kTelescope_SlewRate_Fast,		"Fast");

	SetWidgetType(			kTelescope_SlewingStatus,	kWidgetType_TextBox);
	SetWidget(				kTelescope_SlewingStatus,	label_xLoc + 2,		yLoc,	(labelWidth + valueWidth + 2),	cRadioBtnHt);
	SetWidgetFont(			kTelescope_SlewingStatus,	kFont_RadioBtn);
	SetWidgetType(			kTelescope_SlewingStatus,	kWidgetType_TextBox);
	SetWidgetText(			kTelescope_SlewingStatus,	"Slewing ???");
	yLoc				+=	cRadioBtnHt;
	yLoc				+=	2;


	value_Xloc			=	label_xLoc + 2 + labelWidth + 2;
	yLoc				=	yLocSave_SlewRate;
	//---------------------------------------------------------------------------------------
	for (iii = kTelescope_SlewRate_VerySlow_Val; iii <= kTelescope_SlewRate_Fast_Val; iii++)
	{
		SetWidget(			iii,	value_Xloc,		yLoc,		valueWidth,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		SetWidgetBorder(	iii,	true);
		SetWidgetNumber(	iii,	(iii - kTelescope_SlewRate_VerySlow_Val));
		SetWidgetJustification(	iii,	kJustification_Center);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	yLoc				+=	cRadioBtnHt;
	yLoc				+=	2;

	SetWidgetOutlineBox(kTelescope_SlewRate_Outline, kTelescope_SlewRate_Title, kTelescope_SlewingStatus);
	SetWidgetBorderColor(kTelescope_SlewRate_Outline, cBtnBGcolor_Slewing);

	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
	coordHeight	=	cBtnHeight - 11;
	iii			=	kTelescope_HA_label;
	while (iii < kTelescope_PhysSideOfPier_value)
	{
		SetWidget(			iii,	label_xLoc,		yLoc,		labelWidth,		coordHeight);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;

		SetWidget(			iii,	value_xLoc,		yLoc,		valueWidth,		coordHeight);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;

		yLoc			+=	coordHeight;
	//	yLoc			+=	1;
	}
	SetWidgetText(		kTelescope_HA_label,				"HA");
	SetWidgetText(		kTelescope_RA_label,				"RA");
	SetWidgetText(		kTelescope_DEC_label,				"DEC");
	SetWidgetText(		kTelescope_SideOfPier_label,		"Side Of Pier");
	SetWidgetText(		kTelescope_PhysSideOfPier_label,	"Phys Side");

	SetWidgetText(		kTelescope_HA_value,				"----");
	SetWidgetText(		kTelescope_RA_value,				"----");
	SetWidgetText(		kTelescope_DEC_value,				"----");
	SetWidgetText(		kTelescope_SideOfPier_value,		"----");
	SetWidgetText(		kTelescope_PhysSideOfPier_value,	"----");

	SetWidgetOutlineBox(kTelescope_Coord_Outline, kTelescope_HA_label, (kTelescope_Coord_Outline -1));


	yLoc			+=	8;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kTelescope_IPaddr,
							kTelescope_Readall,
							kTelescope_AlpacaErrorMsg,
							kTelescope_LastCmdString,
							kTelescope_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabTelescope::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	validData;
bool	update;
bool	updateButtons;
//-int		trackingRate;

//	CONSOLE_DEBUG(__FUNCTION__);

	ClearLastAlpacaCommand();
	SetWidgetText(kTelescope_ErrorMsg, "");


	update			=	true;
	validData		=	true;
	updateButtons	=	false;
	switch(buttonIdx)
	{
		case kTelescope_BtnUp:
		case kTelescope_BtnDown:
		case kTelescope_BtnEast:
		case kTelescope_BtnWest:
		case kTelescope_BtnTrackingOn:
		case kTelescope_BtnTrackingOff:
			if (cAtPark == false)
			{
				updateButtons	=	ProcessMovementButtons(buttonIdx, flags);
			}
			else
			{
				SetWidgetText(kTelescope_ErrorMsg,	"Invalid when Parked");
			}
			break;

		case kTelescope_BtnEmergencyStop:
			validData		=	AlpacaSendPutCmd(	"telescope", "abortslew",	NULL);
			cSlewingUp		=	false;
			cSlewingDown	=	false;
			cSlewingEast	=	false;
			cSlewingWest	=	false;
			break;


		case kTelescope_SlewRate_VerySlow:
		case kTelescope_SlewRate_Slow:
		case kTelescope_SlewRate_Med:
		case kTelescope_SlewRate_Fast:
			cCurrentSlewRate	=	buttonIdx;
			updateButtons		=	true;
			break;


		default:
			update	=	false;
//			CONSOLE_DEBUG(__FUNCTION__);
//			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
			break;
	}
	if (validData == false)
	{
		CONSOLE_DEBUG_W_NUM("Command failure, buttonIdx\t=", buttonIdx);
	}
	if (updateButtons)
	{
		UpdateButtons();
	}

	if (update)
	{
		DisplayLastAlpacaCommand();
		ForceWindowUpdate();
		ForceAlpacaUpdate();
	}
}

//*****************************************************************************
bool	WindowTabTelescope::ProcessMovementButtons(const int buttonIdx, const int flags)
{
char	dataString[128];
bool	validData;
bool	updateButtons;
double	ra_SlewRate_degPerSec;
double	dec_SlewRate_degPerSec;

	CONSOLE_DEBUG(__FUNCTION__);
	updateButtons	=	false;
	switch(cCurrentSlewRate)
	{
		case kTelescope_SlewRate_VerySlow:
			ra_SlewRate_degPerSec	=	cRA_slewRates[0];
			dec_SlewRate_degPerSec	=	cDEC_slewRates[0];
			break;

		case kTelescope_SlewRate_Slow:
			ra_SlewRate_degPerSec	=	cRA_slewRates[1];
			dec_SlewRate_degPerSec	=	cDEC_slewRates[1];
			break;

		case kTelescope_SlewRate_Med:
			ra_SlewRate_degPerSec	=	cRA_slewRates[2];
			dec_SlewRate_degPerSec	=	cDEC_slewRates[2];
			break;

		case kTelescope_SlewRate_Fast:
			ra_SlewRate_degPerSec	=	cRA_slewRates[3];
			dec_SlewRate_degPerSec	=	cDEC_slewRates[3];
			break;

		default:
			ra_SlewRate_degPerSec	=	cRA_slewRates[0];
			dec_SlewRate_degPerSec	=	cDEC_slewRates[0];
			break;

	}
	//*	make sure we have non-zero values
	if (ra_SlewRate_degPerSec <= 0.0)
	{
		ra_SlewRate_degPerSec	=	1.0;
	}
	if (dec_SlewRate_degPerSec <= 0.0)
	{
		dec_SlewRate_degPerSec	=	1.0;
	}

	CONSOLE_DEBUG_W_NUM("cCurrentSlewRate      \t=",	cCurrentSlewRate);
	CONSOLE_DEBUG_W_DBL("ra_SlewRate_degPerSec \t=",	ra_SlewRate_degPerSec);
	CONSOLE_DEBUG_W_DBL("dec_SlewRate_degPerSec\t=",	dec_SlewRate_degPerSec);

	switch(buttonIdx)
	{
		case kTelescope_BtnUp:
			if (cSlewingDown == false)
			{
				if (cSlewingUp)
				{
					//*	we are already slewing up, set the rate to zero
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_DEC, 0.0);
				}
				else
				{
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_DEC, dec_SlewRate_degPerSec);
				}
				validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
				if (validData)
				{
					if (cLastAlpacaErrNum == kASCOM_Err_Success)
					{
						//*	the command was sent properly, set the button to indicate that it is moving
						cSlewingUp		=	!cSlewingUp;
						updateButtons	=	true;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=", cLastAlpacaErrNum);
						CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
					}
				}
				else
				{
					CONSOLE_DEBUG("kTelescope_BtnUp: AlpacaSendPutCmd() failed");
				}
			}
			else
			{
				SetWidgetText(kTelescope_ErrorMsg,	"Invalid when slewing down");
			}
			break;

		case kTelescope_BtnDown:
			if (cSlewingUp == false)
			{
				if (cSlewingDown)
				{
					//*	we are already slewing down, set the rate to zero
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_DEC, 0.0);
				}
				else
				{
					sprintf(dataString, "Axis=%d&Rate=-%f", kAxis_DEC, dec_SlewRate_degPerSec);
				}
				validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
				if (validData)
				{
					if (cLastAlpacaErrNum == kASCOM_Err_Success)
					{
						//*	the command was sent properly, set the button to indicate that it is moving
						cSlewingDown	=	!cSlewingDown;
						updateButtons	=	true;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=", cLastAlpacaErrNum);
						CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
					}
				}
				else
				{
					CONSOLE_DEBUG("kTelescope_BtnDown: AlpacaSendPutCmd() failed");
				}
			}
			else
			{
				SetWidgetText(kTelescope_ErrorMsg,	"Invalid when slewing up");
			}
			break;


		case kTelescope_BtnEast:
			if (cSlewingWest == false)
			{
				if (cSlewingEast)
				{
					//*	we are already slewing east, set the rate to zero
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_RA, 0.0);
				}
				else
				{
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_RA, -ra_SlewRate_degPerSec);
				}
				validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
				if (validData)
				{
					//*	the command was sent properly, set the button to indicate that it is moving
					cSlewingEast	=	!cSlewingEast;
					updateButtons	=	true;
				}
				else
				{
					CONSOLE_DEBUG("kTelescope_BtnUp: AlpacaSendPutCmd() failed");
				}
			}
			else
			{
				SetWidgetText(kTelescope_ErrorMsg,	"Invalid when slewing west");
			}
			break;

		case kTelescope_BtnWest:
			if (cSlewingEast == false)
			{
				if (cSlewingWest)
				{
					//*	we are already slewing west, set the rate to zero
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_RA, 0.0);
				}
				else
				{
					sprintf(dataString, "Axis=%d&Rate=%f", kAxis_RA, ra_SlewRate_degPerSec);
				}
				validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
				if (validData)
				{
					//*	the command was sent properly, set the button to indicate that it is moving
					cSlewingWest	=	!cSlewingWest;
					updateButtons	=	true;
				}
				else
				{
					CONSOLE_DEBUG("kTelescope_BtnUp: AlpacaSendPutCmd() failed");
				}
			}
			else
			{
				SetWidgetText(kTelescope_ErrorMsg,	"Invalid when slewing east");
			}
			break;

		case kTelescope_BtnTrackingOn:
			sprintf(dataString, "Tracking=true");
			validData	=	AlpacaSendPutCmd(	"telescope", "tracking",	dataString);
			break;

		case kTelescope_BtnTrackingOff:
			sprintf(dataString, "Tracking=false");
			validData	=	AlpacaSendPutCmd(	"telescope", "tracking",	dataString);
			break;
	}
	return(updateButtons);
}

//*****************************************************************************
void	WindowTabTelescope::UpdateButtons(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetChecked(kTelescope_SlewRate_Fast,		(cCurrentSlewRate == kTelescope_SlewRate_Fast));
	SetWidgetChecked(kTelescope_SlewRate_Med,		(cCurrentSlewRate == kTelescope_SlewRate_Med));
	SetWidgetChecked(kTelescope_SlewRate_Slow,		(cCurrentSlewRate == kTelescope_SlewRate_Slow));
	SetWidgetChecked(kTelescope_SlewRate_VerySlow,	(cCurrentSlewRate == kTelescope_SlewRate_VerySlow));


	if (cAtPark)
	{
		SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Disabled);
		SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Disabled);
		SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Disabled);
		SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Disabled);
	}
	else
	{
		//*	deal with the slewing buttons
//		CONSOLE_DEBUG_W_BOOL("cSlewingUp  \t=",	cSlewingUp);
//		CONSOLE_DEBUG_W_BOOL("cSlewingDown\t=",	cSlewingDown);
//		CONSOLE_DEBUG_W_BOOL("cSlewingEast\t=",	cSlewingEast);
//		CONSOLE_DEBUG_W_BOOL("cSlewingWest\t=",	cSlewingWest);
		if (cSlewingUp)
		{
			SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Slewing);
			SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Disabled);
		}
		else if (cSlewingDown)
		{
			SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Disabled);
			SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Slewing);
		}
		else
		{
			SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Normal);
			SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Normal);
		}


		if (cSlewingEast)
		{
			SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Slewing);
			SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Disabled);
		}
		else if (cSlewingWest)
		{
			SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Disabled);
			SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Slewing);
		}
		else
		{
			SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Normal);
			SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Normal);
		}
	}
	ForceWindowUpdate();
}

//*****************************************************************************
static void	GetSideOfPierString(TYPE_PierSide sideOfPier, char *sideOfPierString)
{
	switch(sideOfPier)
	{
		case kPierSide_NotAvailable:
			strcpy(sideOfPierString, "Not available");
			break;

		//*	Normal pointing state - Mount on the East side of pier (looking West)
		case kPierSide_pierEast:
			strcpy(sideOfPierString, "East");
			break;

		//*	Through the pole pointing state - Mount on the West side of pier (looking East)
		case kPierSide_pierWest:
			strcpy(sideOfPierString, "West");
			break;

		case kPierSide_pierUnknown:
			strcpy(sideOfPierString, "unkown");
			break;

		default:
			strcpy(sideOfPierString, "error");
			break;
	}
}

//*****************************************************************************
void	WindowTabTelescope::UpdateTelescopeInfo(TYPE_TelescopeProperties *telescopeProp, bool updateAll)
{
char	dataString[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	update telescope tracking
	if (telescopeProp->Tracking)
	{
		SetWidgetText(		kTelescope_TrackingStatus, "Tracking is On");
		SetWidgetBGColor(	kTelescope_TrackingStatus,	CV_RGB(0,	255,	0));
		SetWidgetTextColor(	kTelescope_TrackingStatus,	CV_RGB(0,	0,	0));
	}
	else
	{
		SetWidgetText(		kTelescope_TrackingStatus, "Tracking is Off");
		SetWidgetBGColor(	kTelescope_TrackingStatus,	CV_RGB(0,	0,	0));
		SetWidgetTextColor(	kTelescope_TrackingStatus,	CV_RGB(255,	0,	0));
	}

	//*	update slewing
	if (telescopeProp->Slewing)
	{
		SetWidgetText(		kTelescope_SlewingStatus,	"Slewing is On");
		SetWidgetBGColor(	kTelescope_SlewingStatus,	CV_RGB(0,	255,	0));
		SetWidgetTextColor(	kTelescope_SlewingStatus,	CV_RGB(0,	0,	0));
	}
	else
	{
		SetWidgetText(		kTelescope_SlewingStatus, "Slewing is Off");
		SetWidgetBGColor(	kTelescope_SlewingStatus,	CV_RGB(0,	0,	0));
		SetWidgetTextColor(	kTelescope_SlewingStatus,	CV_RGB(255,	0,	0));

		//*	set all of the slewing buttons to normal
		SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Normal);
		SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Normal);
		SetWidgetBGColor(	kTelescope_BtnEast,	cBtnBGcolor_Normal);
		SetWidgetBGColor(	kTelescope_BtnWest,	cBtnBGcolor_Normal);
	}

	//*	deal with Park
	if (telescopeProp->CanPark)
	{
		cAtPark	=	telescopeProp->AtPark;
		SetWidgetValid(		kTelescope_Parked,	cAtPark);
		SetWidgetText(		kTelescope_Parked,	"PARKED");
	}
	else
	{
		SetWidgetValid(		kTelescope_Parked,	false);
	}

	//*	deal with Home
	if (telescopeProp->CanFindHome && (cAtPark == false))
	{
		if (telescopeProp->AtHome)
		{
			SetWidgetValid(		kTelescope_Parked,	true);
			SetWidgetText(		kTelescope_Parked,	"HOME");
		}
	}

	//*	deal with pulse guiding
	if (telescopeProp->CanPulseGuide)
	{
		SetWidgetValid(	kTelescope_PulseGuiding,	telescopeProp->IsPulseGuiding);
	}
	else
	{
		SetWidgetValid(	kTelescope_PulseGuiding,	false);
	}

	//*	side of pier - logical
	GetSideOfPierString(telescopeProp->SideOfPier, dataString);
	SetWidgetText(		kTelescope_SideOfPier_value,	dataString);

	//*	side of pier - physical
	GetSideOfPierString(telescopeProp->PhysicalSideOfPier, dataString);
	SetWidgetText(		kTelescope_PhysSideOfPier_value,	dataString);

	//*	Hour angle (this is not part of Alpaca/ASCOM specs)
	if (telescopeProp->hourAngleIsValid)
	{
		FormatHHMMSS(telescopeProp->HourAngle, dataString, false);
		SetWidgetText(		kTelescope_HA_value,	dataString);
	}

	//--------------------------------------------------------------------------
	if (updateAll)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "UPDATE ALLL!!!!!!");
		//*	calculate the slew rates for both RA and DEC
		if ((cSlewRates_RA_valid == false) || (cSlewRates_DEC_valid == false))
		{
		double	axisRateDelta;
		double	axisRateStep;
		double	slewValue;;
		int		iii;
		char	valueString[48];

			CONSOLE_DEBUG(__FUNCTION__);

			if (telescopeProp->AxisRates[kAxis_RA].Minimum < telescopeProp->AxisRates[kAxis_RA].Maximum)
			{
				//*	ok, we have valid info
				cSlewRate_RAmin	=	telescopeProp->AxisRates[kAxis_RA].Minimum;
				cSlewRate_RAmax	=	telescopeProp->AxisRates[kAxis_RA].Maximum;
				axisRateDelta	=	cSlewRate_RAmax - cSlewRate_RAmin;
				axisRateStep	=	axisRateDelta / kSupportedSlewRates;

				CONSOLE_DEBUG_W_DBL("cSlewRate_RAmin\t=", cSlewRate_RAmin);
				CONSOLE_DEBUG_W_DBL("cSlewRate_RAmax\t=", cSlewRate_RAmax);
				CONSOLE_DEBUG_W_DBL("axisRateDelta  \t=", axisRateDelta);
				CONSOLE_DEBUG_W_DBL("axisRateStep   \t=", axisRateStep);

				slewValue	=	cSlewRate_RAmin + axisRateStep;
				for (iii=0; iii<kSupportedSlewRates; iii++)
				{
					CONSOLE_DEBUG_W_DBL("slewValue      \t=", slewValue);
					cRA_slewRates[iii]	=	slewValue;

					slewValue	+=	axisRateStep;
				}
				cSlewRates_RA_valid		=	true;
			}

			if (telescopeProp->AxisRates[kAxis_DEC].Minimum < telescopeProp->AxisRates[kAxis_DEC].Maximum)
			{
				//*	ok, we have valid info
				cSlewRate_DECmin	=	telescopeProp->AxisRates[kAxis_DEC].Minimum;
				cSlewRate_DECmax	=	telescopeProp->AxisRates[kAxis_DEC].Maximum;
				axisRateDelta		=	cSlewRate_RAmax - cSlewRate_DECmin;
				axisRateStep		=	axisRateDelta / kSupportedSlewRates;

				CONSOLE_DEBUG_W_DBL("cSlewRate_DECmin\t=", cSlewRate_DECmin);
				CONSOLE_DEBUG_W_DBL("cSlewRate_DECmax\t=", cSlewRate_DECmax);
				CONSOLE_DEBUG_W_DBL("axisRateDelta  \t=", axisRateDelta);
				CONSOLE_DEBUG_W_DBL("axisRateStep   \t=", axisRateStep);

				slewValue	=	cSlewRate_DECmin + axisRateStep;
				for (iii=0; iii<kSupportedSlewRates; iii++)
				{
					CONSOLE_DEBUG_W_DBL("slewValue      \t=", slewValue);
					cDEC_slewRates[iii]	=	slewValue;

					slewValue	+=	axisRateStep;
				}
				cSlewRates_DEC_valid	=	true;
			}

			//*	now set the text boxes with the values
			for (iii=0; iii<kSupportedSlewRates; iii++)
			{
				if ((cRA_slewRates[iii] > 0.0) && (cDEC_slewRates[iii] > 0.0) && (cRA_slewRates[iii] < 1000.0) && (cDEC_slewRates[iii] < 1000.0))
				{
					sprintf(valueString, "%2.1f / %2.1f", cRA_slewRates[iii], cDEC_slewRates[iii]);
				}
				else
				{
					CONSOLE_DEBUG_W_DBL("cRA_slewRates[iii] \t=",	cRA_slewRates[iii]);
					CONSOLE_DEBUG_W_DBL("cDEC_slewRates[iii]\t=",	cDEC_slewRates[iii]);
					strcpy(valueString, "-N/A-");
				}
				CONSOLE_DEBUG_W_STR("valueString\t=", valueString);
				SetWidgetText((kTelescope_SlewRate_VerySlow_Val + iii), valueString);
			}
		}
	}
	UpdateButtons();
}

#endif // _ENABLE_CTRL_TELESCOPE_

