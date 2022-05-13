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
//*****************************************************************************

#ifdef _ENABLE_CTRL_TELESCOPE_


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

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

	cCurrentSlewRate	=	kTelescope_SlewRate_Med;
	cSlewingUp			=	false;
	cSlewingDown		=	false;
	cSlewingEast		=	false;
	cSlewingWest		=	false;

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
	labelWidth		=	(myClmWidth / 2) + (myClmWidth / 4);
	valueWidth		=	myClmWidth * 3;

	label_xLoc		=	myClm9_offset + 20;
	value_xLoc		=	label_xLoc + labelWidth + 5;


	//---------------------------------------------------------------------------------------
	for (iii = kTelescope_SlewRate_Title; iii <= kTelescope_SlewingStatus; iii++)
	{
		SetWidget(			iii,	label_xLoc + 2,		yLoc,		(labelWidth + valueWidth),	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}


	SetWidgetType(		kTelescope_SlewRate_Title,	kWidgetType_TextBox);
	SetWidgetType(		kTelescope_SlewingStatus,	kWidgetType_TextBox);
	SetWidgetText(		kTelescope_SlewingStatus,	"Slewing ???");

	SetWidgetText(		kTelescope_SlewRate_Title,		"Slew Rate");
	SetWidgetText(		kTelescope_SlewRate_Fast,		"Fast");
	SetWidgetText(		kTelescope_SlewRate_Med,		"Medium");
	SetWidgetText(		kTelescope_SlewRate_Slow,		"Slow");
	SetWidgetText(		kTelescope_SlewRate_VerySlow,	"Very Slow");
	SetWidgetOutlineBox(kTelescope_SlewRate_Outline, kTelescope_SlewRate_Title, kTelescope_SlewingStatus);

	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;


	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_RA_label,	label_xLoc,		yLoc,		labelWidth,		cBtnHeight);
	SetWidgetFont(		kTelescope_RA_label,	kFont_Medium);
	SetWidgetText(		kTelescope_RA_label,	"RA");

	SetWidget(			kTelescope_RA_value,	value_xLoc,		yLoc,		valueWidth,		cBtnHeight);
	SetWidgetFont(		kTelescope_RA_value,	kFont_Medium);
	SetWidgetText(		kTelescope_RA_value,	"----");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
//	SetWidget(			kTelescope_HA_label,	label_xLoc,		yLoc,		labelWidth,		cBtnHeight);
//	SetWidgetFont(		kTelescope_HA_label,	kFont_Medium);
//	SetWidgetText(		kTelescope_HA_label,	"HA");

//	SetWidget(			kTelescope_HA_value,	value_xLoc,		yLoc,		valueWidth,		cBtnHeight);
//	SetWidgetFont(		kTelescope_HA_value,	kFont_Medium);
//	SetWidgetText(		kTelescope_HA_value,	"----");
//	yLoc			+=	cBtnHeight;
//	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
	SetWidget(			kTelescope_DEC_label,	label_xLoc,		yLoc,		labelWidth,		cBtnHeight);
	SetWidgetFont(		kTelescope_DEC_label,	kFont_Medium);
	SetWidgetText(		kTelescope_DEC_label,	"DEC");

	SetWidget(			kTelescope_DEC_value,	value_xLoc,		yLoc,		valueWidth,		cBtnHeight);
	SetWidgetFont(		kTelescope_DEC_value,	kFont_Medium);
	SetWidgetText(		kTelescope_DEC_value,	"----");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	yLoc			+=	8;
	//---------------------------------------------------------------------------------------
	for (iii = kTelescope_TrackingRate_Title; iii <= kTelescope_TrackingRate_King; iii++)
	{
		SetWidget(			iii,	label_xLoc + 2,		yLoc,		(labelWidth + valueWidth),	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetType(		kTelescope_TrackingRate_Title,		kWidgetType_TextBox);
	SetWidgetText(		kTelescope_TrackingRate_Title,		"Tracking Rate");

	SetWidgetText(		kTelescope_TrackingRate_Sidereal,	"Sidereal");
	SetWidgetText(		kTelescope_TrackingRate_Lunar,		"Lunar");
	SetWidgetText(		kTelescope_TrackingRate_Solar,		"Solar");
	SetWidgetText(		kTelescope_TrackingRate_King,		"King");
	SetWidgetOutlineBox(kTelescope_TrackingRate_Outline, kTelescope_TrackingRate_Title, kTelescope_TrackingRate_King);




	SetAlpacaLogo(kTelescope_AlpacaLogo, kTelescope_LastCmdString);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kTelescope_IPaddr, kTelescope_Readall, kTelescope_AlpacaDrvrVersion, -1);

}

//*****************************************************************************
void	WindowTabTelescope::ProcessButtonClick(const int buttonIdx)
{
char	dataString[128];
bool	validData;
bool	update;
bool	updateButtons;
double	slewRate	=	2.51234;
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTelescope_ErrorMsg, "");

	switch(cCurrentSlewRate)
	{
		case kTelescope_SlewRate_Fast:		slewRate	=	3.0;	break;
		case kTelescope_SlewRate_Med:		slewRate	=	2.0;	break;
		case kTelescope_SlewRate_Slow:		slewRate	=	1.0;	break;
		case kTelescope_SlewRate_VerySlow:	slewRate	=	0.25;	break;
		default:							slewRate	=	2.0;	break;
	}

	update			=	true;
	validData		=	true;
	updateButtons	=	false;
	switch(buttonIdx)
	{
		case kTelescope_BtnUp:
			sprintf(dataString, "Axis=1&Rate=%f", slewRate);
			validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
			if (validData)
			{
				//*	the command was sent properly, set the button to indicate that it is moving
				cSlewingUp	=	true;
				SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Slewing);
				SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Disabled);
			}
			break;

		case kTelescope_BtnDown:
			sprintf(dataString, "Axis=1&Rate=-%f", slewRate);
			validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
			if (validData)
			{
				//*	the command was sent properly, set the button to indicate that it is moving
				SetWidgetBGColor(	kTelescope_BtnDown,	cBtnBGcolor_Slewing);
				SetWidgetBGColor(	kTelescope_BtnUp,	cBtnBGcolor_Disabled);
			}
			break;


		case kTelescope_BtnEast:
			sprintf(dataString, "Axis=0&Rate=%f", slewRate);
			validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
			break;

		case kTelescope_BtnWest:
			sprintf(dataString, "Axis=0&Rate=-%f", slewRate);
			validData	=	AlpacaSendPutCmd(	"telescope", "moveaxis",	dataString);
			break;

		case kTelescope_BtnEmergencyStop:
			validData	=	AlpacaSendPutCmd(	"telescope", "abortslew",	NULL);
			break;

		case kTelescope_BtnTrackingOn:
			sprintf(dataString, "Tracking=true");
			validData	=	AlpacaSendPutCmd(	"telescope", "tracking",	dataString);
			break;

		case kTelescope_BtnTrackingOff:
			sprintf(dataString, "Tracking=false");
			validData	=	AlpacaSendPutCmd(	"telescope", "tracking",	dataString);
			break;

		case kTelescope_SlewRate_Fast:
		case kTelescope_SlewRate_Med:
		case kTelescope_SlewRate_Slow:
		case kTelescope_SlewRate_VerySlow:
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
		ForceUpdate();
	}
}

//*****************************************************************************
void	WindowTabTelescope::UpdateButtons(void)
{

	SetWidgetChecked(kTelescope_SlewRate_Fast,		(cCurrentSlewRate == kTelescope_SlewRate_Fast));
	SetWidgetChecked(kTelescope_SlewRate_Med,		(cCurrentSlewRate == kTelescope_SlewRate_Med));
	SetWidgetChecked(kTelescope_SlewRate_Slow,		(cCurrentSlewRate == kTelescope_SlewRate_Slow));
	SetWidgetChecked(kTelescope_SlewRate_VerySlow,	(cCurrentSlewRate == kTelescope_SlewRate_VerySlow));

}



#endif // _ENABLE_CTRL_IMAGE_

