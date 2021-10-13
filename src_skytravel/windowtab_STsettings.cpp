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
//*****************************************************************************


#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"alpaca_defs.h"
#include	"windowtab.h"
#include	"windowtab_STsettings.h"
#include	"controller.h"
#include	"controller_skytravel.h"
#include	"observatory_settings.h"
#include	"controller_starlist.h"

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
int		xLoc2;
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
		SetWidgetNumber(kSkyT_Settings_LatValue1, gObseratorySettings.Latitude);
		SetWidgetNumber(kSkyT_Settings_LonValue1, gObseratorySettings.Longitude);

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

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	2;

	//-----------------------------------------------------
	//*	Line Width settings
	labelWidth	=	225;
	SetWidget(		kSkyT_Settings_LineWidthTitle,	xLoc,		yLoc,	labelWidth + (4 * cRadioBtnHt),	cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_LineWidthTitle,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_LineWidthTitle,	kWidgetType_Text);
	SetWidgetText(	kSkyT_Settings_LineWidthTitle,	"Line Widths");
	yLoc			+=	cBoxHeight;
	yLoc			+=	2;

	iii			=	kSkyT_Settings_LineW_Const;
	while (iii < kSkyT_Settings_LineW_BoxOutline)
	{
		xLoc2	=	xLoc;
		SetWidget(		iii,	xLoc2,			yLoc,		labelWidth,		cRadioBtnHt);
		SetWidgetFont(	iii,	kFont_Medium);
		SetWidgetType(	iii,	kWidgetType_Text);
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
	SetWidgetText(	kSkyT_Settings_LineW_Grid,			"Grid Lines");

//	yLoc			+=	cBoxHeight;
	yLoc			+=	2;


	SetWidget(			kSkyT_Settings_ResetToDefault,	xLoc,		yLoc,	labelWidth + (4 * cRadioBtnHt),	cBoxHeight);
	SetWidgetFont(		kSkyT_Settings_ResetToDefault,	kFont_Medium);
	SetWidgetType(		kSkyT_Settings_ResetToDefault,	kWidgetType_Button);
	SetWidgetBGColor(	kSkyT_Settings_ResetToDefault,	CV_RGB(255,	255,	255));
	SetWidgetText(		kSkyT_Settings_ResetToDefault,	"Reset to default");



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
	while (iii < kSkyT_Settings_DataHelpText)
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
	SetWidget(		kSkyT_Settings_DataHelpText,	xLoc,			yLoc,		labelWidth + 75,		cBoxHeight);
	SetWidgetFont(	kSkyT_Settings_DataHelpText,	kFont_Medium);
	SetWidgetType(	kSkyT_Settings_DataHelpText,	kWidgetType_Text);
	SetWidgetJustification(kSkyT_Settings_DataHelpText, kJustification_Center);
	SetWidgetText(			kSkyT_Settings_DataHelpText,	"Double click on green");
	SetWidgetTextColor(		kSkyT_Settings_DataHelpText,	CV_RGB(0,	255,	0));



	SetWidgetOutlineBox(	kSkyT_Settings_DataOutLine,
							kSkyT_Settings_DataTitle,
							(kSkyT_Settings_DataOutLine -1));

	//*	set the star counts
	SetWidgetText(		kSkyT_Settings_DataAAVSO_txt,		"AAVSO Data count");
	SetWidgetNumber(	kSkyT_Settings_DataAAVSO_cnt,		gAAVSOalertsCnt);

	SetWidgetText(		kSkyT_Settings_DataDRAPER_txt,		"DRAPER Data count");
	SetWidgetNumber(	kSkyT_Settings_DataDRAPER_cnt,		gDraperObjectCount);

	SetWidgetText(		kSkyT_Settings_DataHipparcos_txt,	"Hipparcos Data count");
	SetWidgetNumber(	kSkyT_Settings_DataHipparcos_cnt,	gHipObjectCount);

	SetWidgetText(		kSkyT_Settings_DataMessier_txt,		"Messier Data count");
	SetWidgetNumber(	kSkyT_Settings_DataMessier_cnt,		gMessierOjbectCount);

	SetWidgetText(		kSkyT_Settings_DataNGC_txt,			"NGC Data count");
	SetWidgetNumber(	kSkyT_Settings_DataNGC_cnt,			gNGCobjectCount);

	SetWidgetText(		kSkyT_Settings_DataYALE_txt,		"YALE Data count");
	SetWidgetNumber(	kSkyT_Settings_DataYALE_cnt,		gYaleStarCount);

	//*	set the ones we can double click to green
	SetWidgetTextColor(	kSkyT_Settings_DataAAVSO_txt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataAAVSO_cnt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataMessier_txt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataMessier_cnt,	CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataNGC_txt,		CV_RGB(0,	255,	0));
	SetWidgetTextColor(	kSkyT_Settings_DataNGC_cnt,		CV_RGB(0,	255,	0));


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
			break;

		case kSkyT_Settings_GridSolid:
			gDashedLines	=	false;
			break;

		case kSkyT_Settings_GridDashed:
			gDashedLines	=	true;
			break;

		case kSkyT_Settings_LineW_Const1:
			gLineWidth_Constellations	=	1;
			break;

		case kSkyT_Settings_LineW_Const2:
			gLineWidth_Constellations	=	2;
			break;

		case kSkyT_Settings_LineW_ConstOutlines1:
			gLineWidth_ConstOutlines	=	1;
			break;

		case kSkyT_Settings_LineW_ConstOutlines2:
			gLineWidth_ConstOutlines	=	2;
			break;

		case kSkyT_Settings_LineW_Grid1:
			gLineWidth_GridLines	=	1;
			break;

		case kSkyT_Settings_LineW_Grid2:
			gLineWidth_GridLines	=	2;
			break;

		case kSkyT_Settings_ResetToDefault:
			gST_DispOptions.EarthDispMode	=	0;
			gDashedLines					=	false;
			gLineWidth_Constellations		=	1;
			gLineWidth_ConstOutlines		=	1;
			gLineWidth_GridLines			=	1;
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
			if ((gMessierOjbectPtr != NULL) && (gMessierOjbectCount > 0))
			{
				CreateStarlistWindow("Messier List", gMessierOjbectPtr, gMessierOjbectCount);
			}
			break;

		case kSkyT_Settings_DataNGC_txt:
		case kSkyT_Settings_DataNGC_cnt:
			if ((gNGCobjectPtr != NULL) && (gNGCobjectCount > 0))
			{
				CreateStarlistWindow("NGC List", gNGCobjectPtr, gNGCobjectCount);
			}
			break;


		case kSkyT_Settings_DataYALE_txt:
		case kSkyT_Settings_DataYALE_cnt:
			if ((gYaleStarDataPtr != NULL) && (gYaleStarCount > 0))
			{
//				CreateStarlistWindow("Yale Star Catalog", gYaleStarDataPtr, gYaleStarCount);
			}
			break;

	}
}


//*****************************************************************************
//*	update the various settings check boxes etc
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

	SetWidgetChecked(	kSkyT_Settings_LineW_Const1,			(gLineWidth_Constellations == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_Const2,			(gLineWidth_Constellations == 2));

	SetWidgetChecked(	kSkyT_Settings_LineW_ConstOutlines1,	(gLineWidth_ConstOutlines == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_ConstOutlines2,	(gLineWidth_ConstOutlines == 2));

	SetWidgetChecked(	kSkyT_Settings_LineW_Grid1,				(gLineWidth_GridLines == 1));
	SetWidgetChecked(	kSkyT_Settings_LineW_Grid2,				(gLineWidth_GridLines == 2));

	ForceUpdate();
}


