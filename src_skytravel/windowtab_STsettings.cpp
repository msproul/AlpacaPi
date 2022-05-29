//*****************************************************************************
//*		windowtab_STsettings.cpp		(c) 2021 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 21,	2020	<MLS> Created windowtab_STsettings.cpp
//*	Jul  5,	2021	<MLS> Added star count table to settings tab
//*	Jul 17,	2021	<MLS> Added double click to SkyTravelSettings window
//*	Sep 12,	2021	<MLS> Added settings for line widths
//*	Sep 12,	2021	<MLS> Added reset-to-default button
//*	Oct 28,	2021	<MLS> Added day/night sky option, blue sky during the day
//*	Dec 13,	2021	<MLS> Added buttons to run Startup and Shutdown scripts
//*	Dec 13,	2021	<MLS> Added RunShellScript_Thead() and RunShellScript()
//*	Dec 31,	2021	<MLS> Added Asteroid count display to settings window
//*	Jan 29,	2022	<MLS> Added special list count display to settings window
//*	Mar 29,	2022	<MLS> Added SaveSkyTravelSystemInfo()
//*****************************************************************************

#include	<pthread.h>
#include	<fitsio.h>
#include	<gnu/libc-version.h>

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"cpu_stats.h"
#include	"linuxerrors.h"
#include	"alpaca_defs.h"
#include	"windowtab.h"
#include	"windowtab_STsettings.h"
#include	"controller.h"
#include	"controller_skytravel.h"
#include	"observatory_settings.h"
#include	"controller_starlist.h"
#include	"controller_constList.h"
#include	"discoverythread.h"

#include	"SkyStruc.h"
#include	"AsteroidData.h"

#ifdef _ENABLE_REMOTE_GAIA_
	#include	"RemoteGaia.h"
#endif

#define	kSkyT_SettingsHeight	100

static void	SaveSkyTravelSystemInfo(void);


//**************************************************************************************
WindowTabSTsettings::WindowTabSTsettings(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();

	UpdateSettings();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSTsettings::~WindowTabSTsettings(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabSTsettings::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		xLoc2;
int		yLoc2;
int		saveXloc;
int		saveYloc;
int		labelWidth;
int		valueWitdth1;
int		valueWitdth2;
int		iii;
char	textString[64];
int		myBoxHeight;
int		radioBtnWidth;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kSkyT_Settings_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kSkyT_Settings_Title, "SkyTravel settings");
	SetBGcolorFromWindowName(kSkyT_Settings_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;


	//----------------------------------------------------------------------
	labelWidth		=	150;
	valueWitdth1	=	100;
	valueWitdth2	=	150;
	xLoc			=	5;

	SetWidget(			kSkyT_Settings_LatLonTitle,	xLoc,	yLoc,	(labelWidth + valueWitdth1 + valueWitdth2),	cTitleHeight);
	SetWidgetFont(		kSkyT_Settings_LatLonTitle,	kFont_Medium);
	SetWidgetText(		kSkyT_Settings_LatLonTitle,	"Site Information");
	SetWidgetBorder(	kSkyT_Settings_LatLonTitle, false);
	SetWidgetTextColor(kSkyT_Settings_LatLonTitle,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	saveXloc		=	5 + (labelWidth + valueWitdth1 + valueWitdth2);
	saveXloc		+=	15;

	//--------------------------------------------------------------------------
	iii	=	kSkyT_Settings_LatLable;
	while (iii < (kSkyT_Settings_SiteInfoOutline -1))
	{
		xLoc		=	5;
		SetWidget(			iii,	xLoc,			yLoc,		labelWidth,		cTitleHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;
		xLoc		+=	labelWidth;
		xLoc		+=	3;

		SetWidget(			iii,	xLoc,			yLoc,		valueWitdth1,		cTitleHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;

		xLoc		+=	valueWitdth1;
		xLoc		+=	3;

		SetWidget(			iii,	xLoc,			yLoc,		valueWitdth2,		cTitleHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;


		yLoc			+=	cTitleHeight;
		yLoc			+=	2;
	}
	SetWidgetText(		kSkyT_Settings_LatLable,	"Latitude");
	SetWidgetText(		kSkyT_Settings_LonLable,	"Longitude");
	SetWidgetText(		kSkyT_Settings_ElevLable,	"Elevation");
	SetWidgetText(		kSkyT_Settings_TimeZoneLbl,	"UTC/Time Zone");



	xLoc			=	5;
	SetWidget(			kSkyT_Settings_ObsSettingsText,	xLoc,	yLoc,	(labelWidth + valueWitdth1 + valueWitdth2),	cTitleHeight);
	SetWidgetFont(		kSkyT_Settings_ObsSettingsText,	kFont_TextList);
	SetWidgetText(		kSkyT_Settings_ObsSettingsText,	"Edit observatorysettings.txt to change these values");
	SetWidgetBorder(	kSkyT_Settings_ObsSettingsText, false);
	SetWidgetTextColor(kSkyT_Settings_ObsSettingsText,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetWidgetOutlineBox(kSkyT_Settings_SiteInfoOutline, kSkyT_Settings_LatLonTitle, (kSkyT_Settings_SiteInfoOutline -1));

	//-----------------------------------------------------
	if (gObseratorySettings.ValidLatLon)
	{
		SetWidgetNumber6F(kSkyT_Settings_LatValue1, gObseratorySettings.Latitude);
		SetWidgetNumber6F(kSkyT_Settings_LonValue1, gObseratorySettings.Longitude);

		FormatHHMMSS(gObseratorySettings.Latitude, textString, true);
		SetWidgetText(		kSkyT_Settings_LatValue2,	textString);

		FormatHHMMSS(gObseratorySettings.Longitude, textString, true);
		SetWidgetText(		kSkyT_Settings_LonValue2,	textString);

		SetWidgetText(		kSkyT_Settings_TimeZoneTxt,	gObseratorySettings.TimeZone);
		SetWidgetNumber(	kSkyT_Settings_UTCoffset,	gObseratorySettings.UTCoffset);

		sprintf(textString, "%1.1f m", gObseratorySettings.Elevation_m);
		SetWidgetText(		kSkyT_Settings_ElevValueMeters,	textString);

		sprintf(textString, "%1.0f ft", gObseratorySettings.Elevation_ft);
		SetWidgetText(		kSkyT_Settings_ElevValueFeet,	textString);
	}

	//--------------------------------------------------------------------------
	yLoc	+=	12;
	xLoc	=	5;
	iii		=	kSkyT_Settings_EarthLable;
	while (iii < kSkyT_Settings_EarthOutline)
	{
		if (iii == kSkyT_Settings_DayNightSky)
		{
			//*	leave some extra space
			yLoc			+=	cRadioBtnHt;
		}
		SetWidget(		iii,	xLoc,			yLoc,		labelWidth,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_RadioBtn);
		SetWidgetType(	iii,	kWidgetType_RadioButton);

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
		iii++;
	}
	SetWidgetType(		kSkyT_Settings_EarthLable,		kWidgetType_TextBox);
	SetWidgetText(		kSkyT_Settings_EarthLable,		"Earth Display");
	SetWidgetText(		kSkyT_Settings_EarthThin,		"Thin Lines");
	SetWidgetText(		kSkyT_Settings_EarthThick,		"Thick Lines");
	SetWidgetText(		kSkyT_Settings_EarthSolidBrn,	"Solid Brown");
	SetWidgetText(		kSkyT_Settings_EarthSolidGrn,	"Solid Green");

	SetWidgetText(		kSkyT_Settings_DayNightSky,		"Day/Night sky");
	SetWidgetType(		kSkyT_Settings_DayNightSky,		kWidgetType_CheckBox);


	SetWidgetOutlineBox(kSkyT_Settings_EarthOutline, kSkyT_Settings_EarthLable, (kSkyT_Settings_EarthOutline -1));


	//-----------------------------------------------------
	yLoc	+=	12;
	xLoc	=	5;
	iii		=	kSkyT_Settings_GridLable;
	while (iii < kSkyT_Settings_GridOutline)
	{
		SetWidget(		iii,	xLoc,			yLoc,		labelWidth,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_RadioBtn);
		SetWidgetType(	iii,	kWidgetType_RadioButton);

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
		iii++;
	}
	SetWidgetType(		kSkyT_Settings_GridLable,	kWidgetType_TextBox);
	SetWidgetText(		kSkyT_Settings_GridLable,	"Grid Lines");
	SetWidgetText(		kSkyT_Settings_GridSolid,	"Solid Lines");
	SetWidgetText(		kSkyT_Settings_GridDashed,	"Dashed Lines");
	SetWidgetOutlineBox(kSkyT_Settings_GridOutline, kSkyT_Settings_GridLable, (kSkyT_Settings_GridOutline -1));

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	2;

	//-----------------------------------------------------
	//*	Line Width settings
	labelWidth	=	225;
	SetWidget(		kSkyT_Settings_LineWidthTitle,	xLoc,		yLoc,	labelWidth + (4 * cRadioBtnHt),	cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_LineWidthTitle,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_LineWidthTitle,	kWidgetType_TextBox);
	SetWidgetText(	kSkyT_Settings_LineWidthTitle,	"Line Widths");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	iii			=	kSkyT_Settings_LineW_Const;
	while (iii < kSkyT_Settings_LineW_BoxOutline)
	{
		xLoc2	=	xLoc;
		SetWidget(		iii,	xLoc2,			yLoc,		labelWidth,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetJustification(iii, kJustification_Left);

		xLoc2	+=	labelWidth;
		iii++;

		SetWidget(		iii,	xLoc2,			yLoc,		cRadioBtnHt * 2,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_RadioButton);
		SetWidgetText(	iii,	"1");
		xLoc2	+=	cRadioBtnHt * 2;
		iii++;

		SetWidget(		iii,	xLoc2,			yLoc,		cRadioBtnHt * 2,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_RadioButton);
		SetWidgetText(	iii,	"2");
		iii++;

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(	kSkyT_Settings_LineW_BoxOutline,
							kSkyT_Settings_LineWidthTitle,
							(kSkyT_Settings_LineW_BoxOutline -1));
	SetWidgetText(	kSkyT_Settings_LineW_Const,			"Constellations");
	SetWidgetText(	kSkyT_Settings_LineW_ConstOutlines,	"Constellation Outlines");
	SetWidgetText(	kSkyT_Settings_LineW_NGCoutlines,	"NGC Outlines");
	SetWidgetText(	kSkyT_Settings_LineW_Grid,			"Grid Lines");

//	yLoc			+=	cBoxHeight;
	yLoc			+=	2;


	SetWidget(			kSkyT_Settings_ResetToDefault,	xLoc,		yLoc,	labelWidth + (4 * cRadioBtnHt),	cBoxHeight);
	SetWidgetFont(		kSkyT_Settings_ResetToDefault,	kFont_Medium);
	SetWidgetType(		kSkyT_Settings_ResetToDefault,	kWidgetType_Button);
	SetWidgetBGColor(	kSkyT_Settings_ResetToDefault,	CV_RGB(255,	255,	255));
	SetWidgetText(		kSkyT_Settings_ResetToDefault,	"Reset to default");

	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	SetWidget(			kSkyT_Settings_SaveSystemInfo,	xLoc,		yLoc,	labelWidth + (4 * cRadioBtnHt),	cBoxHeight);
	SetWidgetFont(		kSkyT_Settings_SaveSystemInfo,	kFont_Medium);
	SetWidgetType(		kSkyT_Settings_SaveSystemInfo,	kWidgetType_Button);
	SetWidgetBGColor(	kSkyT_Settings_SaveSystemInfo,	CV_RGB(255,	255,	255));
	SetWidgetText(		kSkyT_Settings_SaveSystemInfo,	"Save System Info");


	//-----------------------------------------------------
	//*	data value settings
	yLoc		=	cTabVertOffset;
	yLoc		+=	cTitleHeight;
	yLoc		+=	6;
//	xLoc		=	cWidth / 2;
	xLoc		=	saveXloc;
	labelWidth	=	200;

	SetWidget(		kSkyT_Settings_DataTitle,	xLoc,		yLoc,	(labelWidth + 75),		cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_DataTitle,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_DataTitle,	kWidgetType_TextBox);
	SetWidgetText(	kSkyT_Settings_DataTitle,	"Star Data counts");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	saveXloc		=	xLoc + (labelWidth + 75);
	saveXloc		+=	15;

	iii			=	kSkyT_Settings_OriginalData_txt;
	while (iii < kSkyT_Settings_DataHelpText)
	{
		SetWidget(		iii,	xLoc,			yLoc,		labelWidth,		cBoxHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		SetWidgetJustification(iii, kJustification_Left);
		iii++;

		SetWidget(		iii,	(xLoc + labelWidth),	yLoc,		75,		cBoxHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_TextBox);
		iii++;


		yLoc			+=	cBoxHeight;
		yLoc			+=	2;
	}
	SetWidget(		kSkyT_Settings_DataHelpText,	xLoc,			yLoc,		labelWidth + 75,	cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_DataHelpText,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_DataHelpText,	kWidgetType_TextBox);
	SetWidgetJustification(kSkyT_Settings_DataHelpText, kJustification_Center);
	SetWidgetText(			kSkyT_Settings_DataHelpText,	"Double click on green");
	SetWidgetTextColor(		kSkyT_Settings_DataHelpText,	CV_RGB(0,	255,	0));

	yLoc			+=	cBoxHeight;
	yLoc			+=	2;


	SetWidgetOutlineBox(	kSkyT_Settings_DataOutLine,
							kSkyT_Settings_DataTitle,
							(kSkyT_Settings_DataOutLine -1));

	//*	set the star counts
	SetWidgetText(		kSkyT_Settings_OriginalData_txt,	"OrigDB star count");
	SetWidgetNumber(	kSkyT_Settings_OriginalData_cnt,	gStarCount);


	SetWidgetText(		kSkyT_Settings_DataAAVSO_txt,		"AAVSO Alert count");
	SetWidgetNumber(	kSkyT_Settings_DataAAVSO_cnt,		gAAVSOalertsCnt);

	SetWidgetText(		kSkyT_Settings_DataDRAPER_txt,		"Henry Draper count");
	SetWidgetNumber(	kSkyT_Settings_DataDRAPER_cnt,		gDraperObjectCount);

	SetWidgetText(		kSkyT_Settings_DataHipparcos_txt,	"Hipparcos count");
	SetWidgetNumber(	kSkyT_Settings_DataHipparcos_cnt,	gHipObjectCount);

	SetWidgetText(		kSkyT_Settings_DataHYG_txt,			"HYG count");
	SetWidgetNumber(	kSkyT_Settings_DataHYG_cnt,			gHYGObjectCount);

	SetWidgetText(		kSkyT_Settings_DataMessier_txt,		"Messier count");
	SetWidgetNumber(	kSkyT_Settings_DataMessier_cnt,		gMessierObjectCount);

	SetWidgetText(		kSkyT_Settings_DataNGC_txt,			"NGC count");
	SetWidgetNumber(	kSkyT_Settings_DataNGC_cnt,			gNGCobjectCount);

	if (strlen(gNGCDatbase) > 0)
	{
		strcpy(textString, "NGC count (");
		strcat(textString, gNGCDatbase);
		strcat(textString, ")");
		SetWidgetText(		kSkyT_Settings_DataNGC_txt,		textString);
	}


	SetWidgetText(		kSkyT_Settings_DataYALE_txt,		"YALE count");
	SetWidgetNumber(	kSkyT_Settings_DataYALE_cnt,		gYaleStarCount);

	SetWidgetText(		kSkyT_Settings_Constellations_txt,	"Constellations");
	SetWidgetNumber(	kSkyT_Settings_Constellations_cnt,	gConstVectorCnt);

	SetWidgetText(		kSkyT_Settings_ConstOutLines_txt,	"Outlines");
	SetWidgetNumber(	kSkyT_Settings_ConstOutLines_cnt,	gConstOutlineCount);

#ifdef _ENABLE_ASTERIODS_
	strcpy(textString, "Asteroids");
	if (strlen(gAsteroidDatbase) > 0)
	{
		strcat(textString, " (");
		strcat(textString, gAsteroidDatbase);
		strcat(textString, ")");
	}
#else
	strcpy(textString, "Asteroids disabled");
#endif
	SetWidgetText(		kSkyT_Settings_Asteroids_txt,		textString);
	SetWidgetNumber(	kSkyT_Settings_Asteroids_cnt,		gAsteroidCnt);

	SetWidgetText(		kSkyT_Settings_Special_txt,			"Special Data count");
	SetWidgetNumber(	kSkyT_Settings_Special_cnt,			gSpecialObjectCount);


	//*	set the ones we can double click to green

	SetWidgetTextColor(	kSkyT_Settings_OriginalData_txt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_OriginalData_cnt,	CV_RGB(0,	255,	0));

	SetWidgetTextColor(	kSkyT_Settings_DataAAVSO_txt,		CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataAAVSO_cnt,		CV_RGB(0,	255,	0));

	SetWidgetTextColor(	kSkyT_Settings_DataMessier_txt,		CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataMessier_cnt,		CV_RGB(0,	255,	0));

	SetWidgetTextColor(	kSkyT_Settings_DataNGC_txt,			CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataNGC_cnt,			CV_RGB(0,	255,	0));

	SetWidgetTextColor(	kSkyT_Settings_DataYALE_txt,		CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataYALE_cnt,		CV_RGB(0,	255,	0));

	SetWidgetTextColor(	kSkyT_Settings_Constellations_txt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_Constellations_cnt,	CV_RGB(0,	255,	0));

	if (gSpecialObjectCount > 0)
	{
		SetWidgetTextColor(	kSkyT_Settings_Special_txt,		CV_RGB(0,	255,	0));
		SetWidgetTextColor(	kSkyT_Settings_Special_cnt,		CV_RGB(0,	255,	0));
	}

	//-------------------------------------------------------------------------
	yLoc		=	cTabVertOffset;
	yLoc		+=	cTitleHeight;
	yLoc		+=	6;
//	xLoc		=	cWidth / 2;
	xLoc		=	saveXloc;

	//*	set up OBAFGKM color box
//	xLoc			=	5;
	myBoxHeight		=	75;
	SetWidget(			kSkyT_Settings_OBAFGKM,	xLoc,	yLoc,	450,	myBoxHeight);
	SetWidgetType(		kSkyT_Settings_OBAFGKM,	kWidgetType_CustomGraphic);
	SetWidgetTextColor(	kSkyT_Settings_OBAFGKM,		CV_RGB(0,	0,	0));

	yLoc			+=	myBoxHeight;
	yLoc			+=	2;

	//-------------------------------------------------------------------------
	SetWidget(			kSkyT_Settings_DispMag,	xLoc,	yLoc,	450,	cSmallBtnHt);
	SetWidgetType(		kSkyT_Settings_DispMag,	kWidgetType_CheckBox);
	SetWidgetFont(		kSkyT_Settings_DispMag,	kFont_Medium);
	SetWidgetText(		kSkyT_Settings_DispMag,		"Display Star Magnitude");

	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	//-------------------------------------------------------------------------
	SetWidget(			kSkyT_Settings_DispSpectral,	xLoc,	yLoc,	450,	cSmallBtnHt);
	SetWidgetType(		kSkyT_Settings_DispSpectral,	kWidgetType_CheckBox);
	SetWidgetFont(		kSkyT_Settings_DispSpectral,	kFont_Medium);
	SetWidgetText(		kSkyT_Settings_DispSpectral,	"Display Star Spectral Class");

	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;
	saveYloc		=	yLoc;
	radioBtnWidth	=	300;
	for (iii=kSkyT_Settings_DispDynMagnitude; iii<= kSkyT_Settings_DispAllMagnitude; iii++)
	{
		SetWidget(			iii,	xLoc,	yLoc,	radioBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_Medium);
//		SetWidgetBorder(	iii,	true);

		if (iii == kSkyT_Settings_DispSpecifiedMagnitude)
		{
			saveYloc	=	yLoc;
		}

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kSkyT_Settings_DispDynMagnitude,		"Display dynamic magnitude");
	SetWidgetText(		kSkyT_Settings_DispSpecifiedMagnitude,	"Display specified magnitude");
	SetWidgetText(		kSkyT_Settings_DispAllMagnitude,		"Display all magnitudes");

	SetWidget(			kSkyT_Settings_MagnitudeLimit,	(xLoc + radioBtnWidth),
														saveYloc,	100,	cSmallBtnHt);
	SetWidgetType(		kSkyT_Settings_MagnitudeLimit,	kWidgetType_TextBox);
	SetWidgetFont(		kSkyT_Settings_MagnitudeLimit,	kFont_Medium);

	xLoc2		=	xLoc + radioBtnWidth + 100;
	xLoc2		+=	2;
	yLoc2		=	saveYloc - (cSmallBtnHt / 2);

	SetWidget(			kSkyT_Settings_MagUpArrow,		xLoc2,	yLoc2,	cSmallBtnHt,	cSmallBtnHt);
	yLoc2		+=	cSmallBtnHt;
	SetWidget(			kSkyT_Settings_MagDownArrow,	xLoc2,	yLoc2,	cSmallBtnHt,	cSmallBtnHt);

	SetWidgetBorder(	kSkyT_Settings_MagUpArrow, true);

	SetWidgetIcon(kSkyT_Settings_MagUpArrow,	kIcon_UpArrow);
	SetWidgetIcon(kSkyT_Settings_MagDownArrow,	kIcon_DownArrow);
	SetWidgetBorder(	kSkyT_Settings_MagUpArrow, true);
	SetWidgetBorder(	kSkyT_Settings_MagDownArrow, true);


	SetWidgetOutlineBox(	kSkyT_Settings_StarOutLine,
							kSkyT_Settings_OBAFGKM,
							(kSkyT_Settings_StarOutLine -1));


	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	for (iii=kSkyT_Settings_RunStartup; iii<=kSkyT_Settings_RunShutdown; iii++)
	{
		SetWidget(			iii,	xLoc,	yLoc,	radioBtnWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetFont(		iii,	kFont_Medium);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(0,	0,	0));

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kSkyT_Settings_RunStartup,	"Run Startup script (startup.sh)");
	SetWidgetText(		kSkyT_Settings_RunShutdown,	"Run Shutdown script (shutdown.sh)");


	SetAlpacaLogoBottomCorner(kSkyT_Settings_AlpacaLogo);

}

//**************************************************************************************
//*	gets called when the window tab changes
//**************************************************************************************
void	WindowTabSTsettings::ActivateWindow(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	UpdateSettings();
}

//**************************************************************************************
#ifdef _USE_OPENCV_CPP_
void	WindowTabSTsettings::DrawOBAFGKM(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget)
#else
//**************************************************************************************
void	WindowTabSTsettings::DrawOBAFGKM(IplImage *openCV_Image, TYPE_WIDGET *theWidget)
#endif // _USE_OPENCV_CPP_
{
int				radius1;
int				iii;
char			textString[64];
char			obafkgmLetters[]	=	"OBAFGKM";
int				colourNum;
int				center_X;
int				center_Y;
int				pt1_X;
int				pt1_Y;

//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;

//	//*	Erase the rectangle
//	cCurrentColor	=	theWidget->bgColor;
//	LLD_FillRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);
//
//
//	//*	draw the boarder
//	cCurrentColor	=	theWidget->borderColor;
//	LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);

	center_X		=	theWidget->left + (theWidget->height / 2);
	center_Y		=	theWidget->top + (theWidget->height / 2);

	radius1			=	(theWidget->height - 25) / 2;
	iii				=	0;
	colourNum		=	W_STAR_O;

	while (obafkgmLetters[iii] > 0)
	{
		LLD_SetColor(colourNum);
		LLD_FillEllipse(center_X, center_Y, radius1, radius1);

		textString[0]	=	obafkgmLetters[iii];
		textString[1]	=	0;
		pt1_X			=	center_X - 10;
		pt1_Y			=	center_Y + 7;

		cCurrentColor	=	theWidget->textColor;
		LLD_DrawCString(pt1_X, pt1_Y, textString, kFont_Triplex_Large);

		center_X		+=	2 * radius1;
		center_X		+=	10;

		colourNum++;
		iii++;
	}
}


//**************************************************************************************
#ifdef _USE_OPENCV_CPP_
void	WindowTabSTsettings::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabSTsettings::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif
{
	switch(widgetIdx)
	{
		case kSkyT_Settings_OBAFGKM:
			DrawOBAFGKM(openCV_Image, &cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

static	pthread_t	gShellScript_ThreadID;
static	char		gShellCmdLine[64];
static	bool		gShellThreadIsRunning	=	false;

//*****************************************************************************
static void	*RunShellScript_Thead(void *arg)
{
int		systemRetCode;

	gShellThreadIsRunning	=	true;
	CONSOLE_DEBUG(__FUNCTION__);

	if (arg != NULL)
	{

		CONSOLE_DEBUG((char *)arg);

		CONSOLE_DEBUG_W_STR("cmdLine\t=",	(char *)arg);
		systemRetCode	=	system((char *)arg);
		if (systemRetCode == 0)
		{
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
		}
	}
	else
	{
		CONSOLE_DEBUG("arg is NULL");
	}
	CONSOLE_DEBUG("Thread exiting");

	gShellThreadIsRunning	=	false;
	return(NULL);
}

//*****************************************************************************
void	RunShellScript(const char *commandLine)
{
int		threadErr;

	CONSOLE_DEBUG(__FUNCTION__);

	if (gShellThreadIsRunning == false)
	{
		strcpy(gShellCmdLine, commandLine);
		CONSOLE_DEBUG_W_STR("gShellCmdLine\t=",	gShellCmdLine);

		threadErr	=	pthread_create(	&gShellScript_ThreadID,
										NULL,
										&RunShellScript_Thead,
										(void *)gShellCmdLine);
		if (threadErr == 0)
		{
			CONSOLE_DEBUG("Shell script thread created successfully");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
		}
		CONSOLE_DEBUG("EXIT");
	}
	else
	{
		CONSOLE_DEBUG("Thread currently busy!!!!");
	}
}

//*****************************************************************************
void	WindowTabSTsettings::ProcessButtonClick(const int buttonIdx)
{

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	switch(buttonIdx)
	{
		case kSkyT_Settings_EarthThin:
		case kSkyT_Settings_EarthThick:
		case kSkyT_Settings_EarthSolidBrn:
		case kSkyT_Settings_EarthSolidGrn:
			gST_DispOptions.EarthDispMode	=	buttonIdx - kSkyT_Settings_EarthThin;
			break;

		case kSkyT_Settings_DayNightSky:
			gST_DispOptions.DayNightSkyColor	=	!gST_DispOptions.DayNightSkyColor;
			break;

		case kSkyT_Settings_GridSolid:
			gST_DispOptions.DashedLines	=	false;
			break;

		case kSkyT_Settings_GridDashed:
			gST_DispOptions.DashedLines	=	true;
			break;

		//*	Constellastions
		case kSkyT_Settings_LineW_Const1:
			gST_DispOptions.LineWidth_Constellations	=	1;
			break;

		case kSkyT_Settings_LineW_Const2:
			gST_DispOptions.LineWidth_Constellations	=	2;
			break;

		//*	Constellation outlines
		case kSkyT_Settings_LineW_ConstOutlines1:
			gST_DispOptions.LineWidth_ConstOutlines	=	1;
			break;

		case kSkyT_Settings_LineW_ConstOutlines2:
			gST_DispOptions.LineWidth_ConstOutlines	=	2;
			break;

		//*	NGC Outlines
		case kSkyT_Settings_LineW_NGCoutlines1:
			gST_DispOptions.LineWidth_NGCoutlines	=	1;
			break;

		case kSkyT_Settings_LineW_NGCoutlines2:
			gST_DispOptions.LineWidth_NGCoutlines	=	2;
			break;

		case kSkyT_Settings_LineW_Grid1:
			gST_DispOptions.LineWidth_GridLines		=	1;
			break;

		case kSkyT_Settings_LineW_Grid2:
			gST_DispOptions.LineWidth_GridLines		=	2;
			break;

		case kSkyT_Settings_ResetToDefault:
			gST_DispOptions.EarthDispMode				=	0;
			gST_DispOptions.DashedLines					=	false;
			gST_DispOptions.DayNightSkyColor			=	false;
			gST_DispOptions.LineWidth_Constellations	=	1;
			gST_DispOptions.LineWidth_ConstOutlines		=	1;
			gST_DispOptions.LineWidth_GridLines			=	1;
			gST_DispOptions.LineWidth_NGCoutlines		=	1;
			break;

		case kSkyT_Settings_SaveSystemInfo:
			SaveSkyTravelSystemInfo();
			break;

		case kSkyT_Settings_DispMag:
			gST_DispOptions.DispMagnitude		=	!gST_DispOptions.DispMagnitude;
			break;

		case kSkyT_Settings_DispSpectral:
			gST_DispOptions.DispSpectralType	=	!gST_DispOptions.DispSpectralType;
			break;

		case kSkyT_Settings_DispDynMagnitude:
			gST_DispOptions.MagnitudeMode	=	kMagnitudeMode_Dynamic;
			break;

		case kSkyT_Settings_DispSpecifiedMagnitude:
			gST_DispOptions.MagnitudeMode	=	kMagnitudeMode_Specified;
			break;

		case kSkyT_Settings_DispAllMagnitude:
			gST_DispOptions.MagnitudeMode	=	kMagnitudeMode_All;
			break;

		case kSkyT_Settings_MagUpArrow:
			gST_DispOptions.DisplayedMagnitudeLimit++;
			if (gST_DispOptions.DisplayedMagnitudeLimit > 25)
			{
				gST_DispOptions.DisplayedMagnitudeLimit	=	25;
			}
			break;

		case kSkyT_Settings_MagDownArrow:
			gST_DispOptions.DisplayedMagnitudeLimit--;
			if (gST_DispOptions.DisplayedMagnitudeLimit < 0)
			{
				gST_DispOptions.DisplayedMagnitudeLimit	=	0;
			}
			break;

		case kSkyT_Settings_RunStartup:
			RunShellScript("./startup.sh");
			break;

		case kSkyT_Settings_RunShutdown:
			RunShellScript("./shutdown.sh");
			break;


	}
	UpdateSettings();
}

//*****************************************************************************
void	WindowTabSTsettings::ProcessDoubleClick(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{

	switch(widgetIdx)
	{
		case kSkyT_Settings_OriginalData_txt:
		case kSkyT_Settings_OriginalData_cnt:
			if ((gStarDataPtr != NULL) && (gStarCount > 0))
			{
				CreateStarlistWindow("Original Database", gStarDataPtr, gStarCount);
			}
			break;

		case kSkyT_Settings_DataAAVSO_txt:
		case kSkyT_Settings_DataAAVSO_cnt:
		//	CreateAAVSOlistWindow();
			if ((gAAVSOalertsPtr != NULL) && (gAAVSOalertsCnt > 0))
			{
				CreateStarlistWindow("AAVSO List", gAAVSOalertsPtr, gAAVSOalertsCnt);
			}
			break;

		case kSkyT_Settings_DataMessier_txt:
		case kSkyT_Settings_DataMessier_cnt:
			if ((gMessierObjectPtr != NULL) && (gMessierObjectCount > 0))
			{
				CreateStarlistWindow("Messier List", gMessierObjectPtr, gMessierObjectCount, "M#");
			}
			break;

		case kSkyT_Settings_DataNGC_txt:
		case kSkyT_Settings_DataNGC_cnt:
			if ((gNGCobjectPtr != NULL) && (gNGCobjectCount > 0))
			{
				CreateStarlistWindow("NGC List", gNGCobjectPtr, gNGCobjectCount, "NGC/IC#");
			}
			break;


		case kSkyT_Settings_DataYALE_txt:
		case kSkyT_Settings_DataYALE_cnt:
			if ((gYaleStarDataPtr != NULL) && (gYaleStarCount > 0))
			{
				CreateStarlistWindow("Yale Star Catalog", gYaleStarDataPtr, gYaleStarCount);
			}
			break;

		case kSkyT_Settings_Constellations_txt:
		case kSkyT_Settings_Constellations_cnt:
			if ((gConstVecotrPtr != NULL) && (gConstVectorCnt > 0))
			{
				CreateConstellationListWindow(gConstVecotrPtr, gConstVectorCnt);
			}
			break;

		case kSkyT_Settings_Special_txt:
		case kSkyT_Settings_Special_cnt:
			if ((gSpecialObjectPtr != NULL) && (gSpecialObjectCount > 0))
			{
				CreateStarlistWindow("special.txt list", gSpecialObjectPtr, gSpecialObjectCount, "--");
			}
			break;
	}
}


//*****************************************************************************
//*	update the various settings check boxes etc
//*****************************************************************************
void	WindowTabSTsettings::UpdateSettings(void)
{
char	textString[32];

//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetChecked(	kSkyT_Settings_EarthThin,				(gST_DispOptions.EarthDispMode == 0));
	SetWidgetChecked(	kSkyT_Settings_EarthThick,				(gST_DispOptions.EarthDispMode == 1));
	SetWidgetChecked(	kSkyT_Settings_EarthSolidBrn,			(gST_DispOptions.EarthDispMode == 2));
	SetWidgetChecked(	kSkyT_Settings_EarthSolidGrn,			(gST_DispOptions.EarthDispMode == 3));

	SetWidgetChecked(	kSkyT_Settings_DayNightSky,				gST_DispOptions.DayNightSkyColor);

	SetWidgetChecked(	kSkyT_Settings_GridSolid,				(!gST_DispOptions.DashedLines));
	SetWidgetChecked(	kSkyT_Settings_GridDashed,				(gST_DispOptions.DashedLines));

	SetWidgetChecked(	kSkyT_Settings_LineW_Const1,			(gST_DispOptions.LineWidth_Constellations == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_Const2,			(gST_DispOptions.LineWidth_Constellations == 2));

	SetWidgetChecked(	kSkyT_Settings_LineW_ConstOutlines1,	(gST_DispOptions.LineWidth_ConstOutlines == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_ConstOutlines2,	(gST_DispOptions.LineWidth_ConstOutlines == 2));

	SetWidgetChecked(	kSkyT_Settings_LineW_NGCoutlines1,		(gST_DispOptions.LineWidth_NGCoutlines == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_NGCoutlines2,		(gST_DispOptions.LineWidth_NGCoutlines == 2));

	SetWidgetChecked(	kSkyT_Settings_LineW_Grid1,				(gST_DispOptions.LineWidth_GridLines == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_Grid2,				(gST_DispOptions.LineWidth_GridLines == 2));

	SetWidgetChecked(	kSkyT_Settings_DispMag,					gST_DispOptions.DispMagnitude);
	SetWidgetChecked(	kSkyT_Settings_DispSpectral,			gST_DispOptions.DispSpectralType);


	SetWidgetChecked(	kSkyT_Settings_DispDynMagnitude,
				(gST_DispOptions.MagnitudeMode == kMagnitudeMode_Dynamic));

	SetWidgetChecked(	kSkyT_Settings_DispSpecifiedMagnitude,
				(gST_DispOptions.MagnitudeMode == kMagnitudeMode_Specified));

	SetWidgetChecked(	kSkyT_Settings_DispAllMagnitude,
				(gST_DispOptions.MagnitudeMode == kMagnitudeMode_All));

	sprintf(textString, "%1.1f", gST_DispOptions.DisplayedMagnitudeLimit);
	SetWidgetText(kSkyT_Settings_MagnitudeLimit, textString);

	ForceUpdate();
}

//**************************************************************************************
static void	SaveSkyTravelSystemInfo(void)
{
FILE	*filePointer;
char	linuxErrStr[128];
int		iii;
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);
	filePointer	=	fopen("SkyTravelSystemInfo.txt", "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "#########################################################\n");
		fprintf(filePointer, "SkyTravel system information, include this in bug reports\n");

		fprintf(filePointer, "Version: %s\n",	kVersionString);
		fprintf(filePointer, "Build:%d\n",		kBuildNumber);
		fprintf(filePointer, "Compiled on:%s\n", __DATE__);


		fprintf(filePointer, "%s\n",	gOsReleaseString);
		fprintf(filePointer, "%s\n",	gCpuInfoString);
		fprintf(filePointer, "%s\n",	gPlatformString);
		fprintf(filePointer, "gcc version:%s\n", __VERSION__);
		fprintf(filePointer, "libc version:%s\n", gnu_get_libc_version());

	#ifdef __ARM_NEON
		fprintf(filePointer, "ARM NEON instructions\n");
	#endif

#ifdef _ENABLE_FITS_
		//**************************************************************
		//*	cfitsio version
	#ifdef CFITSIO_MICRO
		fprintf(filePointer, "FITS (cfitsio) V%d.%d.%d\n", CFITSIO_MAJOR, CFITSIO_MINOR, CFITSIO_MICRO);
	#else
		fprintf(filePointer, "FITS (cfitsio) V%d.%d\n", CFITSIO_MAJOR, CFITSIO_MINOR);
	#endif
#endif // _ENABLE_FITS_

		//**************************************************************
		//*	OpenCV
		fprintf(filePointer, "OpenCV %s\n", CV_VERSION);

#ifdef _ENABLE_REMOTE_GAIA_
		fprintf(filePointer, "%s\n",	gSQLclientVersion);
#endif // _ENABLE_REMOTE_GAIA_

		//**************************************************************
		fprintf(filePointer, "***********************************************************\n");
		fprintf(filePointer, "Current active Alpaca devices, count=%d\n", gRemoteCnt);
		for (iii=0; iii<gRemoteCnt; iii++)
		{
			inet_ntop(AF_INET, &(gRemoteList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			fprintf(filePointer, "%3d %s:%d\t%-20s\t%-35s\t%-20s\n",
												(iii + 1),
												ipAddrStr,
												gRemoteList[iii].port,
												gRemoteList[iii].deviceTypeStr,
												gRemoteList[iii].deviceNameStr,
												gRemoteList[iii].versionString
												);
		}
		fclose(filePointer);
	}
	else
	{
		//*	something went wrong, we failed to create the file
		GetLinuxErrorString(errno, linuxErrStr);
		CONSOLE_DEBUG_W_STR("Failed to create template file:", linuxErrStr);
	}
}


