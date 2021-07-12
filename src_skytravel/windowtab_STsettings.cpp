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
//*****************************************************************************


#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"windowtab.h"
#include	"windowtab_STsettings.h"
#include	"controller.h"
#include	"controller_skytravel.h"
#include	"observatory_settings.h"

#include	"SkyStruc.h"

#define	kSkyT_SettingsHeight	100

//**************************************************************************************
WindowTabSTsettings::WindowTabSTsettings(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
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
int		labelWidth;
int		valueWitdth1;
int		valueWitdth2;
int		iii;
char	textString[64];

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
		SetWidgetNumber(kSkyT_Settings_LatValue1, gObseratorySettings.Latitude);
		SetWidgetNumber(kSkyT_Settings_LonValue1, gObseratorySettings.Longitude);

		FormatHHMMSS(gObseratorySettings.Latitude, textString, true);
		SetWidgetText(		kSkyT_Settings_LatValue2,	textString);

		FormatHHMMSS(gObseratorySettings.Longitude, textString, true);
		SetWidgetText(		kSkyT_Settings_LonValue2,	textString);

		SetWidgetText(		kSkyT_Settings_TimeZoneTxt,	gObseratorySettings.TimeZone);
		SetWidgetNumber(	kSkyT_Settings_UTCoffset,	gObseratorySettings.UTCoffset);

	}

	//-----------------------------------------------------
	yLoc	+=	12;
	xLoc	=	5;
	iii		=	kSkyT_Settings_EarthLable;
	while (iii < kSkyT_Settings_EarthOutline)
	{
		SetWidget(		iii,	xLoc,			yLoc,		labelWidth,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_RadioBtn);
		SetWidgetType(	iii,	kWidgetType_RadioButton);

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
		iii++;
	}
	SetWidgetType(		kSkyT_Settings_EarthLable,	kWidgetType_Text);
	SetWidgetText(		kSkyT_Settings_EarthLable,		"Earth Display");
	SetWidgetText(		kSkyT_Settings_EarthThin,		"Thin Lines");
	SetWidgetText(		kSkyT_Settings_EarthThick,		"Thick Lines");
	SetWidgetText(		kSkyT_Settings_EarthSolidBrn,	"Solid Brown");
	SetWidgetText(		kSkyT_Settings_EarthSolidGrn,	"Solid Green");
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
	SetWidgetType(		kSkyT_Settings_GridLable,	kWidgetType_Text);
	SetWidgetText(		kSkyT_Settings_GridLable,	"Grid Lines");
	SetWidgetText(		kSkyT_Settings_GridSolid,	"Solid Lines");
	SetWidgetText(		kSkyT_Settings_GridDashed,	"Dashed Lines");
	SetWidgetOutlineBox(kSkyT_Settings_GridOutline, kSkyT_Settings_GridLable, (kSkyT_Settings_GridOutline -1));



	//-----------------------------------------------------
	//*	data value settings
	yLoc		=	cTabVertOffset;
	yLoc		+=	cTitleHeight;
	yLoc		+=	6;
	xLoc		=	cWidth / 2;
	labelWidth	=	200;

	SetWidget(		kSkyT_Settings_DataTitle,	xLoc,		yLoc,	(labelWidth + 75),		cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_DataTitle,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_DataTitle,	kWidgetType_Text);
	SetWidgetText(	kSkyT_Settings_DataTitle,	"Star Data counts");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	iii			=	kSkyT_Settings_DataAAVSO_txt;
	while (iii < kSkyT_Settings_DataOutLine)
	{
		SetWidget(		iii,	xLoc,			yLoc,		labelWidth,		cBoxHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_Text);
		SetWidgetJustification(iii, kJustification_Left);
		iii++;

		SetWidget(		iii,	(xLoc + labelWidth),	yLoc,		75,		cBoxHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_Text);
		iii++;


		yLoc			+=	cBoxHeight;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(kSkyT_Settings_DataOutLine, kSkyT_Settings_DataTitle, (kSkyT_Settings_DataOutLine -1));

	//*	set the star counts
	SetWidgetText(		kSkyT_Settings_DataAAVSO_txt,		"AAVSO Data count");
	SetWidgetNumber(	kSkyT_Settings_DataAAVSO_cnt,		gAAVSOalertsCnt);

	SetWidgetText(		kSkyT_Settings_DataDRAPER_txt,		"DRAPER Data count");
	SetWidgetNumber(	kSkyT_Settings_DataDRAPER_cnt,		gDraperObjectCount);

	SetWidgetText(		kSkyT_Settings_DataHipparcos_txt,	"Hipparcos Data count");
	SetWidgetNumber(	kSkyT_Settings_DataHipparcos_cnt,	gHipObjectCount);

	SetWidgetText(		kSkyT_Settings_DataNGC_txt,			"NGC Data count");
	SetWidgetNumber(	kSkyT_Settings_DataNGC_cnt,			gNGCobjectCount);

	SetWidgetText(		kSkyT_Settings_DataYALE_txt,		"YALE Data count");
	SetWidgetNumber(	kSkyT_Settings_DataYALE_cnt,		gYaleStarCount);


	SetAlpacaLogoBottomCorner(kSkyT_Settings_AlpacaLogo);

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
			UpdateSettings();
			break;

		case kSkyT_Settings_GridSolid:
			gDashedLines	=	false;
			UpdateSettings();
			break;

		case kSkyT_Settings_GridDashed:
			gDashedLines	=	true;
			UpdateSettings();
			break;
	}
}

//*****************************************************************************
void	WindowTabSTsettings::UpdateSettings(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetChecked(	kSkyT_Settings_EarthThin,		(gST_DispOptions.EarthDispMode == 0));
	SetWidgetChecked(	kSkyT_Settings_EarthThick,		(gST_DispOptions.EarthDispMode == 1));
	SetWidgetChecked(	kSkyT_Settings_EarthSolidBrn,	(gST_DispOptions.EarthDispMode == 2));
	SetWidgetChecked(	kSkyT_Settings_EarthSolidGrn,	(gST_DispOptions.EarthDispMode == 3));

	SetWidgetChecked(	kSkyT_Settings_GridSolid,		(!gDashedLines));
	SetWidgetChecked(	kSkyT_Settings_GridDashed,		(gDashedLines));

	ForceUpdate();

}


