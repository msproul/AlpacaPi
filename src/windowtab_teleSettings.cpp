//*****************************************************************************
//*		windowtab_teleSettings.cpp		(c) 2022 by Mark Sproul
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
//*	May 29,	2022	<MLS> Created windowtab_teleSettings.cpp
//*	May 30,	2022	<MLS> Added buttons for FindHome, Park
//*	May 31,	2022	<MLS> Added support for DoesRefraction
//*	Jun 14,	2022	<MLS> DoesRefraction button now working properly
//*	Jun 19,	2023	<MLS> Added DeviceState to TeleSettings
//*****************************************************************************

#ifdef _ENABLE_CTRL_TELESCOPE_

#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_telescope.h"

#include	"windowtab.h"
#include	"windowtab_teleSettings.h"


//**************************************************************************************
WindowTabTeleSettings::WindowTabTeleSettings(	const int	xSize,
												const int	ySize,
												cv::Scalar	backGrndColor,
												const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	//*	local copies of values that we need to remember
	cAtPark			=	false;
	cDoesRefraction	=	false;
	cTracking		=	false;
	SetupWindowControls();

//	UpdateButtons();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabTeleSettings::~WindowTabTeleSettings(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabTeleSettings::SetupWindowControls(void)
{
int			xLoc;
int			yLoc;
int			yLocSave;
int			iii;
int			myBtnWidth;
int			leftColmX;
char		notAvailableStr[]	=	"---N/A---";
int			yLoc_2ndColumn;
int			groupBoxWidth;

	xLoc	=	5;
	yLoc	=   cTabVertOffset;
	yLoc	=	SetTitleBox(kTeleSettings_Title, -1, yLoc, "Telescope settings");
	yLoc_2ndColumn	=	yLoc;
	//---------------------------------------------------------------------------------------
	for (iii = kTeleSettings_TrackingRate_Title; iii <= kTeleSettings_TrackingRate_King; iii++)
	{
		SetWidget(			iii,	xLoc + 2,		yLoc,		(cBtnWidth -3),	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetType(		kTeleSettings_TrackingRate_Title,		kWidgetType_TextBox);
	SetWidgetText(		kTeleSettings_TrackingRate_Title,		"Tracking Rate");

	SetWidgetText(		kTeleSettings_TrackingRate_Sidereal,	"Sidereal");
	SetWidgetText(		kTeleSettings_TrackingRate_Lunar,		"Lunar");
	SetWidgetText(		kTeleSettings_TrackingRate_Solar,		"Solar");
	SetWidgetText(		kTeleSettings_TrackingRate_King,		"King");
	SetWidgetOutlineBox(kTeleSettings_TrackingRate_Outline, kTeleSettings_TrackingRate_Title, kTeleSettings_TrackingRate_King);

	yLoc			+=	2;
	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
	myBtnWidth	=	cBtnWidth - 2;
	iii			=   kTeleSettings_AtHome_Lbl;
	while (iii < kTeleSettings_HomePark_Outline)
	{
		xLoc		=	5;
		//*	label
		SetWidget(			iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		//*	Value
		SetWidget(			iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_TextBox);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetText(		iii,	notAvailableStr);
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		//*	GO button
		SetWidget(			iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(0,	0,		0));
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		//*	Set button
		SetWidget(			iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(0,	0,		0));
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kTeleSettings_AtHome_Lbl,			"AtHome");
	SetWidgetText(		kTeleSettings_AtPark_Lbl,			"AtPark");
	SetWidgetText(		kTeleSettings_Refraction_Lbl,		"Refraction");

	SetWidgetText(		kTeleSettings_AtHome_Btn,			"Go Home");
	SetWidgetText(		kTeleSettings_AtPark_Btn,			"Go Park");
	SetWidgetText(		kTeleSettings_Refraction_Btn,		"On/Off");


	SetWidgetText(		kTeleSettings_AtHome_Set,			"Set Home");
	SetWidgetText(		kTeleSettings_AtPark_Set,			"Set Park");
	SetWidgetValid(		kTeleSettings_Refraction_Set,		false);

	SetWidgetHelpText(kTeleSettings_AtHome_Btn,	"Move mount to home position (mechanical 0,0)");
	SetWidgetHelpText(kTeleSettings_AtHome_Set,	"Sets current location to home (mechanical 0,0)");

	SetWidgetHelpText(kTeleSettings_AtPark_Btn,	"Move mount to pre-set PARK location");
	SetWidgetHelpText(kTeleSettings_AtPark_Set,	"Sets current location to the PARK position");

	SetWidgetHelpText(kTeleSettings_Refraction_Btn,	"Enable/Disable refraction compensation in mount");



	SetWidgetOutlineBox(	kTeleSettings_HomePark_Outline,
							kTeleSettings_AtHome_Lbl,
							(kTeleSettings_HomePark_Outline - 1));

	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
	groupBoxWidth	=	((cBtnWidth * 4) - 2);
	xLoc			=	cClm1_offset + 2;
	SetWidget(		kTeleSettings_Rates_Title,	xLoc,		yLoc,	groupBoxWidth,	cSmallBtnHt);
	SetWidgetType(	kTeleSettings_Rates_Title,	kWidgetType_TextBox);
	SetWidgetFont(	kTeleSettings_Rates_Title,	kFont_Medium);
	SetWidgetText(	kTeleSettings_Rates_Title,	"Movement rates");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	3;

	iii			=   kTeleSettings_RateRA_Lbl;
	while (iii < kTeleSettings_GuideRateDec_Val)
	{
		xLoc			=	cClm1_offset + 2;
		//*	label
		SetWidget(		iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_TextList);
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		//*	Value
		SetWidget(		iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_TextList);
		SetWidgetText(	iii,	notAvailableStr);
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;

		//*	Units
		SetWidget(				iii,	xLoc,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetJustification(	iii, kJustification_Left);
		SetWidgetBorder(		iii,	false);
		SetWidgetText(			iii,	"units");
		xLoc	+=	myBtnWidth;
		xLoc	+=	2;
		iii++;


		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kTeleSettings_RateRA_Lbl,			"RA");
	SetWidgetText(		kTeleSettings_RateDec_Lbl,			"Dec");
	SetWidgetText(		kTeleSettings_RateRA_Units,			"secs/sidereal sec");
	SetWidgetText(		kTeleSettings_RateDec_Units,		"arcsecs/sec");

	SetWidgetText(		kTeleSettings_GuideRateRA_Lbl,		"Guide-RA");
	SetWidgetText(		kTeleSettings_GuideRateDec_Lbl,		"Guide-Dec");
	SetWidgetText(		kTeleSettings_GuideRateRA_Units,	"degrees/sec");
	SetWidgetText(		kTeleSettings_GuideRateDec_Units,	"degrees/sec");

	SetWidgetHelpText(kTeleSettings_RateRA_Lbl,			"RA rate OFFSET (seconds of RA per sidereal second.)");
	SetWidgetHelpText(kTeleSettings_RateDec_Lbl,		"DEC rate OFFSET (seconds of DEC per atomic second.)");

	SetWidgetHelpText(kTeleSettings_GuideRateRA_Lbl,	"RA rate used in pulse guiding");
	SetWidgetHelpText(kTeleSettings_GuideRateDec_Lbl,	"DEC rate used in pulse guiding");

	SetWidgetOutlineBox(kTeleSettings_GuideRate_Outline,
						kTeleSettings_Rates_Title,
						(kTeleSettings_GuideRate_Outline - 1));

	yLocSave	=	yLoc;
	leftColmX	=	cClm4_offset + 10;
	//---------------------------------------------------------------------------------------
	//*	2nd COLUMN
	//---------------------------------------------------------------------------------------
	yLoc	=   yLoc_2ndColumn;
	yLoc	+=	2;
	iii		=   kTeleSettings_LatLabel;
	while (iii < kTeleSettings_ElevValue)
	{
		SetWidget(		iii,	leftColmX,		yLoc,	(cBtnWidth - 10),		cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_Medium);
		iii++;

		SetWidget(		iii,	cClm5_offset,	yLoc,	(cBtnWidth * 2),cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_Medium);
		iii++;

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kTeleSettings_LatLabel,		"Lat:");
	SetWidgetText(		kTeleSettings_LonLabel,		"Lon:");
	SetWidgetText(		kTeleSettings_ElevLabel,	"Elev:");

	SetWidgetText(		kTeleSettings_LatValue,		"---");
	SetWidgetText(		kTeleSettings_LonValue,		"---");
	SetWidgetText(		kTeleSettings_ElevValue,	"---");

	SetWidget(		kTeleSettings_ObsSettingsText,	leftColmX,		yLoc,	(cBtnWidth * 3) -10,	cSmallBtnHt);
	SetWidgetType(	kTeleSettings_ObsSettingsText,	kWidgetType_TextBox);
	SetWidgetFont(	kTeleSettings_ObsSettingsText,	kFont_RadioBtn);
	SetWidgetTextColor(	kTeleSettings_ObsSettingsText,	CV_RGB(255,	255,	255));
//	SetWidgetBGColor(	kTeleSettings_ObsSettingsText,	CV_RGB(255,	255,	255));
//	SetWidgetTextColor(	kTeleSettings_ObsSettingsText,	CV_RGB(0,	0,	0));
	SetWidgetText(		kTeleSettings_ObsSettingsText,	"Values from observatorysettings.txt on driver");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;


	//---------------------------------------------------------------------------------------
	if (yLocSave > yLoc)
	{
		yLoc	=	yLocSave;
	}
	yLoc			+=	2;
	yLoc			+=	2;
	SetWidget(		kTeleSettings_HelpMsg,	0,		yLoc,		cWidth,		cBtnHeight);
	SetWidgetType(	kTeleSettings_HelpMsg,	kWidgetType_TextBox);
	SetWidgetFont(	kTeleSettings_HelpMsg,	kFont_Medium);
	SetHelpTextBoxNumber(kTeleSettings_HelpMsg);

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kTeleSettings_IPaddr,
							kTeleSettings_Readall,
							kTeleSettings_DeviceState,
							kTeleSettings_AlpacaErrorMsg,
							kTeleSettings_LastCmdString,
							kTeleSettings_AlpacaLogo,
							-1);

	//*	debugging
//	SetWidgetText(kTeleSettings_HelpMsg,		"kTeleSettings_HelpMsg");
//	SetWidgetText(kTeleSettings_AlpacaErrorMsg,	"kTeleSettings_AlpacaErrorMsg");
//	DumpWidgetList(0, kTeleSettings_last, __FILE__);

}

//*****************************************************************************
void	WindowTabTeleSettings::ProcessButtonClick(const int buttonIdx, const int flags)
{
char	dataString[128];
bool	validData;
int		trackingRate;
bool	update;

//	CONSOLE_DEBUG(__FUNCTION__);

	ClearLastAlpacaCommand();
	SetWidgetText(kTeleSettings_AlpacaErrorMsg,	"");

	update		=	true;
	validData	=	true;
	switch(buttonIdx)
	{
		case kTeleSettings_TrackingRate_Sidereal:	//*	Sidereal tracking rate (15.041 arcseconds per second).
		case kTeleSettings_TrackingRate_Lunar:		//*	Lunar tracking rate (14.685 arcseconds per second).
		case kTeleSettings_TrackingRate_Solar:		//*	Solar tracking rate (15.0 arcseconds per second).
		case kTeleSettings_TrackingRate_King:		//*	King tracking rate (15.0369 arcseconds per second).
			trackingRate	=	buttonIdx - kTeleSettings_TrackingRate_Sidereal;
			sprintf(dataString, "TrackingRate=%d", trackingRate);
			validData	=	AlpacaSendPutCmd(	"telescope", "trackingrate",	dataString);
			ForceAlpacaUpdate();
			break;

		case kTeleSettings_AtHome_Btn:
			CONSOLE_DEBUG("Sending FINDHOME command");
			validData	=	AlpacaSendPutCmd(	"telescope", "findhome",	NULL);
			ForceAlpacaUpdate();
			break;

		case kTeleSettings_AtHome_Set:
		case kTeleSettings_AtPark_Set:
			CONSOLE_DEBUG("Sending setpark command");
			validData	=	AlpacaSendPutCmd(	"telescope", "setpark",	NULL);
			break;

		case kTeleSettings_AtPark_Btn:
			if (cAtPark)
			{
				CONSOLE_DEBUG("Sending UP-PARK command");
				validData	=	AlpacaSendPutCmd(	"telescope", "unpark",	NULL);
			}
			else
			{
				CONSOLE_DEBUG("Sending PARK command");
				if (cTracking)
				{
					sprintf(dataString, "Tracking=false");
					validData	=	AlpacaSendPutCmd(	"telescope", "tracking",	dataString);
				}
				validData	=	AlpacaSendPutCmd(	"telescope", "park",	NULL);
			}
			ForceAlpacaUpdate();
			break;



		case kTeleSettings_Refraction_Btn:
			sprintf(dataString, "DoesRefraction=%s", cDoesRefraction ? "false" : "true");
			CONSOLE_DEBUG_W_STR("Sending DoesRefraction command:", dataString);
			validData	=	AlpacaSendPutCmd(	"telescope", "doesrefraction",	dataString);
			ForceAlpacaUpdate();
			break;

		default:
			update	=	false;
			break;
	}
	if (validData == false)
	{
		CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
		CONSOLE_DEBUG_W_STR("Failure with:", dataString);
	}
	if (update)
	{
		DisplayLastAlpacaCommand();
		ForceWindowUpdate();
	}
//	DumpWidgetList(0, kTeleSettings_last, __FILE__);
}

//*****************************************************************************
void	WindowTabTeleSettings::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
//	switch(buttonIdx)
//	{
//		default:
//			break;
//	}
}

//*****************************************************************************
//*	false means only update the dynamic properties
//*****************************************************************************
void	WindowTabTeleSettings::UpdateTelescopeInfo(TYPE_TelescopeProperties *telescopeProp, bool updateAll)
{
char		notAvailableStr[]	=	"---N/A---";

//	CONSOLE_DEBUG(__FUNCTION__);

	if (updateAll)
	{
		SetWidgetNumber6F(kTeleSettings_LatValue,	telescopeProp->SiteLatitude);
		SetWidgetNumber6F(kTeleSettings_LonValue,	telescopeProp->SiteLongitude);
		SetWidgetNumber(kTeleSettings_ElevValue,	telescopeProp->SiteElevation);

		//------------------------------------------
		//*	deal with Refraction
		SetWidgetValid(kTeleSettings_Refraction_Btn, telescopeProp->driverSupportsRefraction);
	}

	//*	set the tracking rate
	SetWidgetChecked(	kTeleSettings_TrackingRate_Sidereal,
											(telescopeProp->TrackingRate == kDriveRate_driveSidereal));

	SetWidgetChecked(	kTeleSettings_TrackingRate_Lunar,
											(telescopeProp->TrackingRate == kDriveRate_driveLunar));

	SetWidgetChecked(	kTeleSettings_TrackingRate_Solar,
											(telescopeProp->TrackingRate == kDriveRate_driveSolar));

	SetWidgetChecked(	kTeleSettings_TrackingRate_King,
											(telescopeProp->TrackingRate == kDriveRate_driveKing));

	//------------------------------------------
	//*	deal with FindHome
	if (telescopeProp->CanFindHome)
	{
		SetWidgetValid(kTeleSettings_AtHome_Btn, true);
		SetWidgetValid(kTeleSettings_AtHome_Set, true);

		SetWidgetText(kTeleSettings_AtHome_Val,			(telescopeProp->AtHome ? "Yes" : "No"));
	}
	else
	{
		SetWidgetValid(kTeleSettings_AtHome_Btn, false);
		SetWidgetValid(kTeleSettings_AtHome_Set, false);
	}
	//------------------------------------------
	//*	deal with Park
	if (telescopeProp->CanPark)
	{
		SetWidgetValid(kTeleSettings_AtPark_Btn, true);
		SetWidgetValid(kTeleSettings_AtPark_Set, true);
		SetWidgetText(kTeleSettings_AtPark_Val,			(telescopeProp->AtPark ? "Yes" : "No"));
		//*	save the parked state
		cAtPark	=	telescopeProp->AtPark;
		if (cAtPark)
		{
			SetWidgetText(kTeleSettings_AtPark_Btn,		"Un-Park");
		}
		else
		{
			SetWidgetText(kTeleSettings_AtPark_Btn,		"Park");
		}
	}
	else
	{
		SetWidgetValid(kTeleSettings_AtPark_Btn, false);
		SetWidgetValid(kTeleSettings_AtPark_Set, false);
	}

	cTracking	=	telescopeProp->Tracking;
//	//------------------------------------------
//	//*	deal with pulse guiding
//	if (telescopeProp->CanPulseGuide)
//	{
//		SetWidgetValid(kTeleSettings_IsPulseGuiding_Btn, true);
//		SetWidgetText(kTeleSettings_IsPulseGuiding_Val,	(telescopeProp->IsPulseGuiding ? "Yes" : "No"));
//	}
//	else
//	{
//		SetWidgetValid(kTeleSettings_IsPulseGuiding_Btn, false);
//	}

	//------------------------------------------
	//*	deal with Refraction
	if (telescopeProp->driverSupportsRefraction)
	{
		SetWidgetText(kTeleSettings_Refraction_Val,	(telescopeProp->DoesRefraction ? "On" : "Off"));

		cDoesRefraction	=	telescopeProp->DoesRefraction;
	}

	//------------------------------------------
	//*	deal with RightAscensionRate
	if (telescopeProp->CanSetRightAscensionRate)
	{
		SetWidgetNumber(kTeleSettings_RateRA_Val,	telescopeProp->RightAscensionRate);
	}
	else
	{
		SetWidgetText(kTeleSettings_RateRA_Val,	notAvailableStr);
	}
	//------------------------------------------
	//*	deal with DeclinationRate
	if (telescopeProp->CanSetDeclinationRate)
	{
		SetWidgetNumber(kTeleSettings_RateDec_Val,	telescopeProp->DeclinationRate);
	}
	else
	{
		SetWidgetText(kTeleSettings_RateDec_Val,	notAvailableStr);
	}
	//------------------------------------------
	//*	deal with GuideRates
	if (telescopeProp->CanSetGuideRates)
	{
		SetWidgetNumber(kTeleSettings_GuideRateRA_Val,	telescopeProp->GuideRateRightAscension);
		SetWidgetNumber(kTeleSettings_GuideRateDec_Val,	telescopeProp->GuideRateDeclination);
	}
	else
	{
		SetWidgetText(kTeleSettings_GuideRateRA_Val,	notAvailableStr);
		SetWidgetText(kTeleSettings_GuideRateDec_Val,	notAvailableStr);
	}

	ForceWindowUpdate();
}


#endif // _ENABLE_CTRL_TELESCOPE_
