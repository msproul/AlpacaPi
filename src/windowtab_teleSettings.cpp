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
	CONSOLE_DEBUG(__FUNCTION__);
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
	xLoc	=	5;
	yLoc	=   cTabVertOffset;
	yLoc	+=	2;

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
		//*	label
		SetWidget(		iii,	cClm1_offset + 2,		yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_Medium);
		iii++;

		//*	Value
		SetWidget(		iii,	cClm2_offset + 4,		yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetText(	iii,	notAvailableStr);
		iii++;

		//*	button
		SetWidget(			iii,	cClm3_offset + 7,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(0,	0,		0));
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
	SetWidgetOutlineBox(	kTeleSettings_HomePark_Outline,
							kTeleSettings_AtHome_Lbl,
							(kTeleSettings_HomePark_Outline - 1));

	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;

	//---------------------------------------------------------------------------------------
	SetWidget(		kTeleSettings_Rates_Title,	cClm1_offset + 2,		yLoc,	((cBtnWidth * 3) -4),	cSmallBtnHt);
	SetWidgetType(	kTeleSettings_Rates_Title,	kWidgetType_TextBox);
	SetWidgetFont(	kTeleSettings_Rates_Title,	kFont_Medium);
	SetWidgetText(	kTeleSettings_Rates_Title,	"Movement rates");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	3;

	iii			=   kTeleSettings_RateRA_Lbl;
	while (iii < kTeleSettings_GuideRateDec_Val)
	{
		//*	label
		SetWidget(		iii,	cClm1_offset + 2,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_TextList);
		iii++;

		//*	Value
		SetWidget(		iii,	cClm2_offset + 4,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_TextList);
		SetWidgetText(	iii,	notAvailableStr);
		iii++;

		//*	Units
		SetWidget(		iii,	cClm3_offset + 7,	yLoc,	myBtnWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetFont(	iii,	kFont_TextList);
		SetWidgetBorder(iii,	false);

		SetWidgetText(	iii,	"units");
		iii++;


		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kTeleSettings_RateRA_Lbl,			"RA");
	SetWidgetText(		kTeleSettings_RateDec_Lbl,			"Dec");

	SetWidgetText(		kTeleSettings_GuideRateRA_Lbl,		"Guide-RA");
	SetWidgetText(		kTeleSettings_GuideRateDec_Lbl,		"Guide-Dec");

	SetWidgetText(		kTeleSettings_RateRA_Units,			"arcsecs/sec");
	SetWidgetText(		kTeleSettings_RateDec_Units,		"arcsecs/sec");
	SetWidgetText(		kTeleSettings_GuideRateRA_Units,	"degrees/sec");
	SetWidgetText(		kTeleSettings_GuideRateDec_Units,	"degrees/sec");

	SetWidgetOutlineBox(kTeleSettings_GuideRate_Outline,
						kTeleSettings_Rates_Title,
						(kTeleSettings_GuideRate_Outline - 1));



	yLocSave	=	yLoc;
	leftColmX	=	cClm4_offset + 10;
	//---------------------------------------------------------------------------------------
	//*	2nd COLUMN
	//---------------------------------------------------------------------------------------
	yLoc	=   cTabVertOffset;
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
	SetWidget(		kTeleSettings_ErrorMsg,	0,		yLoc,		cWidth - 100,		cBtnHeight);
	SetWidgetFont(	kTeleSettings_ErrorMsg,	kFont_Medium);

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kTeleSettings_IPaddr,
							kTeleSettings_Readall,
							kTeleSettings_AlpacaErrorMsg,
							kTeleSettings_LastCmdString,
							kTeleSettings_AlpacaLogo,
							-1);

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
	SetWidgetText(kTeleSettings_ErrorMsg, "");
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
		SetWidgetText(kTeleSettings_AtHome_Val,			(telescopeProp->AtHome ? "Yes" : "No"));
	}
	else
	{
		SetWidgetValid(kTeleSettings_AtHome_Btn, false);
	}
	//------------------------------------------
	//*	deal with Park
	if (telescopeProp->CanPark)
	{
		SetWidgetValid(kTeleSettings_AtPark_Btn, true);
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
