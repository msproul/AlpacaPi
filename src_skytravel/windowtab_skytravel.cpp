//*****************************************************************************
//*		windowtab_skytravel.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:
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
//*	Dec 29,	2020	<MLS> Created windowtab_skytravel.cpp
//*	Dec 30,	2020	<MLS> Standard star data display starting to work
//*	Jan  2,	2021	<MLS> Double click centers display
//*	Jan  2,	2021	<MLS> Double click in LX200 RA/DEC box centers on scope
//*	Jan  3,	2021	<MLS> Added DrawGreatCircle() & DrawNorthSouthLine()
//*	Jan  3,	2021	<MLS> Rewrote DrawGrid()
//*	Jan  3,	2021	<MLS> Added support for Messier objects via Messier.tsc file
//*	Jan  3,	2021	<MLS> Changed old GreatCircle() to DrawHorizon()
//*	Jan  3,	2021	<MLS> Planet symbols now scale with zoom level
//*	Jan  4,	2021	<MLS> Added DrawDomeSlit() & DrawHorizontalArc()
//*	Jan  4,	2021	<MLS> Made change to DrawEcliptic to allow display at all zoom levels
//*	Jan  5,	2021	<MLS> Added DrawConstellationOutLines()
//*	Jan  5,	2021	<MLS> Added another zoom level
//*	Jan  6,	2021	<MLS> Added NGC and IC search support
//*	Jan  6,	2021	<MLS> Manually added Draco to Constellation line table
//*	Jan  7,	2021	<MLS> Added DrawConstellationVectors()
//*	Jan  7,	2021	<MLS> Added GetXYfromAz_Elev()
//*	Jan  8,	2021	<MLS> Added DrawVerticalArc()
//*	Jan 26,	2021	<MLS> Removed support for direct LX200 comm, using Alpaca instead
//*	Jan 29,	2021	<MLS> Finished removing all LX200 direct comm code
//*	Jan 29,	2021	<MLS> Added auto time update option
//*	Feb 15,	2021	<MLS> Added SyncTelescopeToCenter() & SlewTelescopeToCenter()
//*	Feb 15,	2021	<MLS> Added separate flag for dome slit display
//*	Feb 18,	2021	<MLS> Added display of Hipparcos numbers at high zoom level
//*	Feb 18,	2021	<MLS> Added DrawCommonStarNames()
//*	Feb 18,	2021	<MLS> Added SetView_Index()
//*	Feb 20,	2021	<MLS> Added display of NGC/IC numbers at high zoom level
//*	Mar  2,	2021	<MLS> Fixed bug in SyncTelescopeToCenter()
//*	Mar  4,	2021	<MLS> Added HYG data base which include HD stars (Henry Draper)
//*	Mar  9,	2021	<MLS> Added DrawTelescopeFOV()
//*	Mar  9,	2021	<MLS> Camera Field Of View overlay on star map working
//*	Mar 11,	2021	<MLS> Clif suggested a different way of drawing the FOV
//*****************************************************************************
//*	TODO
//*			star catalog lists
//*
//*	might be useful
//*		http://www.astrosurf.com/jephem/astro/skymap/sm200coordinates_en.htm
//*		https://github.com/dcf21/constellation-stick-figures
//*****************************************************************************




#ifdef _ENABLE_SKYTRAVEL_


//#define	_DISPLAY_MAP_TOKENS_
//#define	_ENBABLE_WHITE_CHART_

#include	<sys/time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_image.h"
#include	"observatory_settings.h"


#include	"SkyStruc.h"
#include	"StarData.h"
#include	"SkyTravelConstants.h"
#include	"eph.h"
#include	"SkyTravelTimeRoutines.h"
#include	"NGCcatalog.h"
#include	"YaleStarCatalog.h"
#include	"HipparcosCatalog.h"
#include	"SkyTravelExternal.h"
#include	"sidereal.h"

#include	"windowtab.h"
#include	"windowtab_skytravel.h"
#include	"controller_skytravel.h"

#include	"SHAPES.DTA"
#include	"DEEP.DTA"
#include	"CONSTEL.DTA"

//#define	kMinInformDist	10
#define	kMinInformDist	50

#define	TRUE	true
#define	FALSE	false

//#define	DEGREES(radians)	((radians) * 180.0 / M_PI)


//*	for the moment, these are hard coded
double	gDomeDiameter_inches	=	(15.0 * 12.0);
double	gSlitWidth_inches		=	41.0;

double	gSlitBottom_degrees		=	25.0;
double	gSlitTop_degrees		=	100.0;
double	gDomeAzimuth_degrees	=	90.0;


SkyTravelDispOptions	gST_DispOptions;


static bool		gDashedLines	=	false;
static double	GetRA_DEC_detla(int viewIndex);


//*****************************************************************************
static const char *gCompass_text[]	=
{
	"N",
	"NNE",
	"NE",
	"ENE",
	"E",
	"ESE",
	"SE",
	"SSE",
	"S",
	"SSW",
	"SW",
	"WSW",
	"W",
	"WNW",
	"NW",
	"NNW"
};

//*****************************************************************************
static const double gView_table[]	=
{
	1.0	* PI / 180.0,
	2.0	* PI / 180.0,
	5.0	* PI / 180.0,
	10.0* PI / 180.0,
	20.0* PI / 180.0,
	40.0* PI / 180.0,
	70.0* PI / 180.0,
	100.0* PI / 180.0,
	150.0* PI / 180.0,
	170.0* PI / 180.0,		//*	Added by <MLS>, these gets a bit tricky
	200.0* PI / 180.0,
	250.0* PI / 180.0,
	300.0* PI / 180.0,
	350.0* PI / 180.0,
	400.0* PI / 180.0,
	500.0* PI / 180.0,
	600.0* PI / 180.0,
	700.0* PI / 180.0,
	800.0* PI / 180.0
};
#define	kMaxViewAngleIndex	18

//*****************************************************************************
static	const char *gZodiac_names[]	=
{
	"Aries",
	"Taurus",
	"Gemini",
	"Cancer",
	"Leo",
	"Virgo",
	"Libra",
	"Scorpio",
	"Sagittarius",
	"Capricorn",
	"Aquarius",
	"Pisces"
};
#define	kZodiacCount	12

//*****************************************************************************
static	const char *gPlanet_names[kPlanetObjectCnt]	=
{
	"Moon",
	"Sun",
	"Mercury",
	"Venus",
	"Mars",
	"Jupiter",
	"Saturn",
	"Uranus",
	"Neptune",
	"Pluto"
};
//* the delta t's for planet

//*****************************************************************************
static const double	gDeltaPlanet[kPlanetObjectCnt]	=
{
	.03333,	//*mon
	.04950,	//*sun
	.01250,	//*mer
	.02500,	//*ven
	.05000,	//*mar
	.07500,	//*jup
	.10000,	//*sat
	.12500,	//*ura
	.15000,	//*nep
	.17500,	//*plu
};


static	TYPE_CelestData	*gZodiacPtr	=	NULL;




//**************************************************************************************
WindowTabSkyTravel::WindowTabSkyTravel(	const int	xSize,
										const int	ySize,
										CvScalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		ii;

	CONSOLE_DEBUG(__FUNCTION__);

	cDebugCounter			=	0;
	cWorkSpaceTopOffset		=	0;
	cWorkSpaceLeftOffset	=	0;


//	CONSOLE_DEBUG_W_NUM("sizeof(TYPE_Time)\t=",	sizeof(TYPE_Time));

	//*	zero out everything
	memset(&cCurrentTime,		0, sizeof(TYPE_Time));
	memset(&cCurrLatLon,		0, sizeof(TYPE_LatLon));

	cAutoAdvanceTime		=	true;
	cLastUpdateTime_ms		=	0;
	cLastClockUpdateTime_ms	=	0;
	cNightMode				=	false;
	cStarDataPtr			=	NULL;
	cStarCount				=	0;
	constelations			=	NULL;
	constelationCount		=	0;
	constStarPtr			=	NULL;
	constStarCount			=	0;
	cNGCobjectPtr			=	NULL;
	cNGCobjectCount			=	0;
	cHipObjectPtr			=	NULL;
	cHipObjectCount			=	0;

	cYaleStarDataPtr		=	NULL;
	cYaleStarCount			=	0;


	cMessierOjbectPtr		=	NULL;
	cMessierOjbectCount		=	0;

	cDraperObjectPtr		=	NULL;
	cDraperObjectCount		=	0;

	cSpecialObjectPtr		=	NULL;
	cSpecialObjectCount		=	0;

	cCameraFOVarrayPtr		=	NULL;

#ifdef _ENABLE_HYG_
	cHYGObjectPtr			=	NULL;
	cHYGObjectCount			=	0;
#endif // _ENABLE_HYG_

	cMagmin					=	0;

	cCursor_elev			=	0;
	cCursor_az				=	0;
	cCursor_ra				=	0;
	cCursor_decl			=	0;
	cCsrx					=	0;
	cCsry					=	0;
	cMouseDragInProgress	=	false;

	if (gObseratorySettings.ValidLatLon)
	{
		cCurrLatLon.latitude		=	RADIANS(gObseratorySettings.Latitude);
		cCurrLatLon.longitude		=	RADIANS(gObseratorySettings.Longitude);
	}
	else
	{
		//*	we need to have a default of someplace
		cCurrLatLon.latitude		=	RADIANS(41.361090);
		cCurrLatLon.longitude		=	RADIANS(-74.980333);
	}


	Compute_Timezone(&cCurrLatLon, &cCurrentTime);

	//	hometime(&gcCurrentTime);	//* default system Greenwich date/time
	SetCurrentTime();

	cCurrentTime.strflag			=	true;		//* say 2000 data present
	cCurrentTime.starDataModified	=	false;
	cCurrentTime.calflag			=	0;			//* auto calendar
	cCurrentTime.precflag			=	false;
	cCurrentTime.negflag			=	false;
	cCurrentTime.local_time_flag	=	true;		//* default use local time
	cCurrentTime.timeOfLastPrec		=	JD2000;		//* set last prec = Julian day 2000
	cCurrentTime.delprc				=	DELPRC0;	//* set delta prec to default

	CalanendarTime(&cCurrentTime);

	memset(&cDispOptions, 0, sizeof(TYPE_SkyDispOptions));

	cCurrentSkyColor				=	BLUE;
	cChart							=	false;
#if 0
	cDispOptions.dispDeep			=	true;
	cDispOptions.dispEarth			=	true;
	cDispOptions.dispGrid			=	true;
	cDispOptions.dispHorizon_line	=	true;
	cDispOptions.dispLines			=	true;
	cDispOptions.dispNames			=	true;
	cDispOptions.dispNGC			=	false;
 	cDispOptions.dispSymbols		=	true;
 	cDispOptions.dispMessier		=	true;
 	cDispOptions.dispConstOutlines	=	true;
 	cDispOptions.dispConstellations	=	true;
	cDispOptions.dispHIP			=	false;
	cDispOptions.dispCommonStarNames=	true;
	cDispOptions.dispHYG_all		=	false;
	cDispOptions.dispDraper			=	false;

	cDispOptions.dispTelescope		=	false;
	cDispOptions.dispDomeSlit		=	false;
#endif

	cDisplayedMagnitudeLimit		=	15.0;

	gST_DispOptions.EarthDispMode	=	0;


	cElev0			=	kHALFPI / 2;		//* 45 degrees
	cAz0			=	-PI / 3.0;			//* 60 degrees (ca. eastnortheast)
	cRa0			=	0.0;
	cDecl0			=	0.0;


	SetView_Index(6);

	cCsrx			=	cWind_x0;	//* center of field
	cCsry			=	cWind_y0;

	wind_ulx		=	0;
	wind_uly		=	0;
	cWind_width		=	xSize;
	cWind_height	=	ySize - 75;


	cStarDataPtr	=	GetDefaultStarData(&cStarCount, &cCurrentTime);
	CONSOLE_DEBUG_W_LONG("cStarCount\t=", cStarCount);


	for (ii=0;ii<10;ii++)
	{
		memset(&cPlanets[ii], 0, sizeof(TYPE_CelestData));

		cPlanets[ii].dataSrc		=	kDataSrc_Planets;
		cPlanets[ii].magn			=	0x00db + ii;
		cPlanets[ii].id				=	0x076c + ii;	//*fill in the id field

		memset(&cPlanetStruct[ii], 0, sizeof(planet_struct));

		cPlanetStruct[ii].delta_dte	=	gDeltaPlanet[ii];	//*interpolation deltas
		cPlanetStruct[ii].dte0		=	1.e20;				//*impossible dte0 so that eph does a full compute
	}


	if (gZodiacPtr == NULL)
	{
		gZodiacPtr	=	(TYPE_CelestData *)malloc(kZodiacCount * sizeof(TYPE_CelestData));

		for (ii=0;ii<kZodiacCount;ii++)
		{
			gZodiacPtr[ii].dataSrc	=	kDataSrc_Zodiac;
			gZodiacPtr[ii].ra		=	kTWOPI*((float)ii / 12.);	//* 12 evenly spaced ra's
			gZodiacPtr[ii].decl		=	ECLIPTIC_ANGLE * sin(kTWOPI * (float)ii / 12.);	//* 23.5 degrees for ecliptic tilt
		//*	gZodiacPtr[ii].magn		=	0x00e6 + ii;
			gZodiacPtr[ii].id		=	0x0777 + ii;
		}
	}

	BuildConstellationData();


	CONSOLE_DEBUG_W_LONG("cNGCobjectCount\t=",	cNGCobjectCount);
	cNGCobjectPtr	=	ReadNGCStarCatalog(&cNGCobjectCount);
	CONSOLE_DEBUG_W_LONG("cNGCobjectCount\t=",	cNGCobjectCount);


	cYaleStarDataPtr	=	ReadYaleStarCatalog(&cYaleStarCount);

	cMessierOjbectPtr	=	ReadMessierData(		"skytravel_data/",	kDataSrc_Messier,	&cMessierOjbectCount);
	cDraperObjectPtr	=	ReadHenryDraperCatalog(	"skytravel_data/",	kDataSrc_Draper,	&cDraperObjectCount);
#ifdef _ENABLE_HYG_
	cHYGObjectPtr		=	ReadHYGdata(			"skytravel_data/",	kDataSrc_HYG,		&cHYGObjectCount);
#endif // _ENABLE_HYG_

	cConstOutlinePtr	=	ReadConstellationOutlines("skytravel_data/constOutlines.txt", &cConstOutlineCount);




	cHipObjectPtr		=	ReadHipparcosStarCatalog(&cHipObjectCount);
	if (cHipObjectPtr != NULL)
	{
		ReadCommonStarNames(cHipObjectPtr, cHipObjectCount);
	}


	cSpecialObjectPtr	=	ReadSpecialData(kDataSrc_Special, &cSpecialObjectCount);


	Precess();		//*	make sure all of the data bases are sorted properly

	CONSOLE_DEBUG(__FUNCTION__);

	SetHipparcosDataPointers(cHipObjectPtr, cHipObjectCount);
	cConstVecotrPtr		=	ReadConstellationVectors(kSkyTravelDataDirectory, &cConstVectorCnt);
	if (cConstVecotrPtr != NULL)
	{
		//*	if we successfully read in the good constellation lines, turn off the old ones
		cDispOptions.dispLines			=	false;
	}


	//--------------------------------------------------------------
	//*	Telescope tracking stuff
	memset(&cTelescopeDisplayOptions,	0, sizeof(TYPE_TeleDispOptions));
	cTelescopeDisplayOptions.dispTeleScopeOutline		=	true;
	cTelescopeDisplayOptions.dispTeleScopeCrossHairs	=	true;
	cTelescopeDisplayOptions.dispFindScopeOutline		=	true;
	cTelescopeDisplayOptions.dispFindScopeCrossHairs	=	true;

	SetupWindowControls();

	ResetView();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSkyTravel::~WindowTabSkyTravel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (cStarDataPtr != NULL)
	{
		free(cStarDataPtr);
		cStarDataPtr	=	NULL;
	}
	if (cYaleStarDataPtr != NULL)
	{
		free(cYaleStarDataPtr);
		cYaleStarDataPtr	=	NULL;
	}
	if (cMessierOjbectPtr != NULL)
	{
		free(cMessierOjbectPtr);
		cMessierOjbectPtr	=	NULL;
	}
	if (cDraperObjectPtr != NULL)
	{
		free(cDraperObjectPtr);
		cDraperObjectPtr	=	NULL;
	}

#ifdef _ENABLE_HYG_
	if (cHYGObjectPtr != NULL)
	{
		free(cHYGObjectPtr);
		cHYGObjectPtr	=	NULL;
	}
#endif // _ENABLE_HYG_
}


//**************************************************************************************
void	WindowTabSkyTravel::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		labelWidth;
int		skyBoxHeight;
int		iii;
int		searchBoxWidth;
int		buttonWidthGoto;
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;


	labelWidth	=	cTitleHeight * 4;
	//------------------------------------------
	SetWidget(			kSkyTravel_Data,		0,			yLoc,		labelWidth,		cTitleHeight);
	SetWidgetText(		kSkyTravel_Data, 		"Data control");
	SetWidgetFont(		kSkyTravel_Data,		kFont_Small);
	SetWidgetBGColor(	kSkyTravel_Data,	CV_RGB(128,	128,	128));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	//------------------------------------------
	SetWidget(			kSkyTravel_Display,		0,			yLoc,		labelWidth,		cTitleHeight);
	SetWidgetText(		kSkyTravel_Display, 	"Display control");
	SetWidgetFont(		kSkyTravel_Display,		kFont_Small);
	SetWidgetBGColor(	kSkyTravel_Display,	CV_RGB(128,	128,	128));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	yLoc			=	cTabVertOffset;

	xLoc			=	labelWidth + 5;
	for (iii = kSkyTravel_Btn_DeepSky; iii < kSkyTravel_MsgTextBox; iii++)
	{
		if (iii == kSkyTravel_Btn_Reset)
		{
			//*	move to the next row
			xLoc	=	labelWidth + 5;
			yLoc	+=	cTitleHeight;
			yLoc	+=	2;
		}
		if (iii == kSkyTravel_UTCtime)
		{
			SetWidget(				iii,	xLoc,	yLoc,		labelWidth * 2,		cTitleHeight);
			SetWidgetType(			iii, 	kWidgetType_Text);
			SetWidgetFont(			iii,	kFont_Medium);
			SetWidgetText(			iii, 	"UTC Time");

			xLoc	+=	labelWidth;
		}
		else
		{
			SetWidget(				iii,	xLoc,	yLoc,		cTitleHeight,		cTitleHeight);
			SetWidgetType(			iii, 	kWidgetType_Button);
			xLoc	+=	cTitleHeight;
		}

		xLoc	+=	2;
	}

	SetWidgetType(		kSkyTravel_Btn_ZoomLevel, 	kWidgetType_Text);
	SetWidgetFont(		kSkyTravel_Btn_ZoomLevel, 	kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Btn_ZoomLevel, 	CV_RGB(255,	255,	255));


	SetWidgetHelpText(	kSkyTravel_Btn_Reset,			"Reset");

	SetWidgetHelpText(	kSkyTravel_Btn_AutoAdvTime,		"Toggle Auto Advance Time");
	SetWidgetHelpText(	kSkyTravel_Btn_Chart,			"Toggle Chart mode");
	SetWidgetHelpText(	kSkyTravel_Btn_DeepSky,			"Toggle Deep Sky Objects");
	SetWidgetHelpText(	kSkyTravel_Btn_Names,			"Toggle Name display");
	SetWidgetHelpText(	kSkyTravel_Btn_CommonStarNames,	"Toggle Common star names display");
	SetWidgetHelpText(	kSkyTravel_Btn_Lines,			"Toggle Line display");
	SetWidgetHelpText(	kSkyTravel_Btn_ConstOutline,	"Toggle Constellation outlines");
	SetWidgetHelpText(	kSkyTravel_Btn_Constellations,	"Toggle Constellations");
	SetWidgetHelpText(	kSkyTravel_Btn_NGC,				"Toggle NGC display");
	SetWidgetHelpText(	kSkyTravel_Btn_Earth,			"Toggle Earth display");
	SetWidgetHelpText(	kSkyTravel_Btn_Grid,			"Toggle Grid display");
	SetWidgetHelpText(	kSkyTravel_Btn_Equator,			"Toggle Equator display");
	SetWidgetHelpText(	kSkyTravel_Btn_Ecliptic,		"Toggle Ecliptic display");
	SetWidgetHelpText(	kSkyTravel_Btn_YaleCat,			"Toggle Yale display");
	SetWidgetHelpText(	kSkyTravel_Btn_Messier,			"Toggle Missier display");

	SetWidgetHelpText(	kSkyTravel_Btn_Hipparcos,		"Toggle Hipparcos display");
	SetWidgetHelpText(	kSkyTravel_Btn_NightMode,		"Toggle Night Mode");
	SetWidgetHelpText(	kSkyTravel_Btn_Symbols,			"Toggle Symbols for planets and zodiac");
	SetWidgetHelpText(	kSkyTravel_Btn_TscopeDisp,		"Toggle telescope position display");


	SetWidgetHelpText(	kSkyTravel_Btn_Plus,		"Zoom In");
	SetWidgetHelpText(	kSkyTravel_Btn_Minus,		"Zoom Out");
	SetWidgetHelpText(	kSkyTravel_Btn_ZoomLevel,	"Current Zoom level");

	SetWidgetHelpText(	kSkyTravel_Search_Text,	"Enter object to search for");
	SetWidgetHelpText(	kSkyTravel_Search_Btn,	"Click to search");


	SetWidgetText(		kSkyTravel_Btn_AutoAdvTime,		"@");
	SetWidgetText(		kSkyTravel_Btn_Reset,			"r");
	SetWidgetText(		kSkyTravel_Btn_Chart,			"c");
	SetWidgetText(		kSkyTravel_Btn_DeepSky,			"D");
	SetWidgetText(		kSkyTravel_Btn_Names,			"N");
	SetWidgetText(		kSkyTravel_Btn_CommonStarNames,	"m");
	SetWidgetText(		kSkyTravel_Btn_Lines,			"L");
	SetWidgetText(		kSkyTravel_Btn_ConstOutline,	"O");
	SetWidgetText(		kSkyTravel_Btn_Constellations,	"?");
	SetWidgetText(		kSkyTravel_Btn_NGC,				"G");
	SetWidgetText(		kSkyTravel_Btn_Earth,			"E");
	SetWidgetText(		kSkyTravel_Btn_Grid,			"#");
	SetWidgetText(		kSkyTravel_Btn_Equator,			"Q");
	SetWidgetText(		kSkyTravel_Btn_Ecliptic,		"C");
	SetWidgetText(		kSkyTravel_Btn_YaleCat,			"Y");
	SetWidgetText(		kSkyTravel_Btn_Messier,			"M");

	SetWidgetText(		kSkyTravel_Btn_Hipparcos,		"H");
	SetWidgetText(		kSkyTravel_Btn_NightMode,		"!");
	SetWidgetText(		kSkyTravel_Btn_Symbols,			"S");
	SetWidgetText(		kSkyTravel_Btn_TscopeDisp,		"t");

	SetWidgetText(		kSkyTravel_Btn_Plus,			"+");
	SetWidgetText(		kSkyTravel_Btn_Minus,			"-");

//	yLoc			+=	cTitleHeight;
//	yLoc			+=	2;

	//------------------------------------------------------------------------------------
	//*	Dome/Telescope indicators
	SetWidgetText(		kSkyTravel_DomeIndicator,		"D");
	SetWidgetTextColor(	kSkyTravel_DomeIndicator,		CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_DomeIndicator,		CV_RGB(255,	0,	0));
	SetWidgetHelpText(	kSkyTravel_DomeIndicator,		"Indicates if Dome is OnLine (Grn=yes/Red=no)");
	SetWidgetType(		kSkyTravel_DomeIndicator, 		kWidgetType_Text);

	SetWidgetText(		kSkyTravel_TelescopeIndicator,	"T");
	SetWidgetTextColor(	kSkyTravel_TelescopeIndicator,	CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_TelescopeIndicator,	CV_RGB(255,	0,	0));
	SetWidgetHelpText(	kSkyTravel_TelescopeIndicator,	"Indicates if Telescope is OnLine (Grn=yes/Red=no)");
	SetWidgetType(		kSkyTravel_TelescopeIndicator, 	kWidgetType_Text);


	SetWidget(			kSkyTravel_Telescope_RA_DEC,	xLoc,	yLoc, cBtnWidth,	cTitleHeight);
	SetWidgetFont(		kSkyTravel_Telescope_RA_DEC,	kFont_Medium);
	SetWidgetText(		kSkyTravel_Telescope_RA_DEC,	"--");
	SetWidgetHelpText(	kSkyTravel_Telescope_RA_DEC,	"Double click to center screen on current telescope location");

	xLoc	+=	cBtnWidth;
	xLoc	+=	2;

	//------------------------------------------------------------------------------------
	buttonWidthGoto	=	cBtnWidth / 2;
	SetWidget(			kSkyTravel_Telescope_Sync,	xLoc,	yLoc, buttonWidthGoto,	cTitleHeight);
	SetWidgetText(		kSkyTravel_Telescope_Sync,	"Sync");
	SetWidgetFont(		kSkyTravel_Telescope_Sync,	kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Telescope_Sync,	CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_Telescope_Sync,	CV_RGB(255,	255,	255));
	SetWidgetType(		kSkyTravel_Telescope_Sync, 	kWidgetType_Button);
	SetWidgetHelpText(	kSkyTravel_Telescope_Sync,	"Sync telescope to center of screen");

	xLoc	+=	buttonWidthGoto;
	xLoc	+=	2;
	SetWidget(			kSkyTravel_Telescope_GoTo,	xLoc,	yLoc, buttonWidthGoto,	cTitleHeight);
	SetWidgetText(		kSkyTravel_Telescope_GoTo,	"GoTo");
	SetWidgetFont(		kSkyTravel_Telescope_GoTo,	kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Telescope_GoTo,	CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_Telescope_GoTo,	CV_RGB(255,	255,	255));
	SetWidgetType(		kSkyTravel_Telescope_GoTo, 	kWidgetType_Button);
	SetWidgetHelpText(	kSkyTravel_Telescope_GoTo,	"Slew telescope to center of screen");

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-----------------------------------------------
	SetWidget(				kSkyTravel_MsgTextBox,	1,		yLoc,	(cWidth - 2),		cTitleHeight);
	SetWidgetType(			kSkyTravel_MsgTextBox, kWidgetType_Text);
	SetWidgetFont(			kSkyTravel_MsgTextBox,	kFont_Medium);
	SetWidgetTextColor(		kSkyTravel_MsgTextBox, CV_RGB(128,	128, 128));
	SetWidgetText(			kSkyTravel_MsgTextBox,	"message text box");
	SetWidgetJustification(	kSkyTravel_MsgTextBox,	kJustification_Left);

	SetHelpTextBoxNumber(	kSkyTravel_MsgTextBox);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-----------------------------------------------
	SetWidget(				kSkyTravel_CursorInfoTextBox,	1,		yLoc,	(cWidth - 2),		cTitleHeight);
	SetWidgetType(			kSkyTravel_CursorInfoTextBox,	kWidgetType_Text);
	SetWidgetFont(			kSkyTravel_CursorInfoTextBox,	kFont_Medium);
	SetWidgetTextColor(		kSkyTravel_CursorInfoTextBox,	CV_RGB(128, 128, 128));
	SetWidgetText(			kSkyTravel_CursorInfoTextBox,	"Help text box");
	SetWidgetJustification(	kSkyTravel_CursorInfoTextBox,	kJustification_Left);


	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-----------------------------------------------
	//*	set up the search button/text
	xLoc	=	cWidth - (cTitleHeight + 2);
	searchBoxWidth	=	200;

	SetWidget(				kSkyTravel_Search_Btn,	xLoc,	cTabVertOffset,		cTitleHeight,		cTitleHeight);
	SetWidgetType(			kSkyTravel_Search_Btn, 	kWidgetType_Button);
	xLoc	-=  searchBoxWidth;
	xLoc	-=  2;
	SetWidget(				kSkyTravel_Search_Text,	xLoc,	cTabVertOffset,		searchBoxWidth,		cTitleHeight);
	SetWidgetType(			kSkyTravel_Search_Text, kWidgetType_TextInput);
	SetWidgetBGColor(		kSkyTravel_Search_Text,	CV_RGB(128,	128,	128));
	SetWidgetTextColor(		kSkyTravel_Search_Text,	CV_RGB(0,	0,	0));
	SetWidgetJustification(	kSkyTravel_Search_Text,	kJustification_Left);
	SetWidgetFont(			kSkyTravel_Search_Text,	kFont_Medium);



//-	cWorkSpaceTopOffset	=	yLoc;
	skyBoxHeight		=	cHeight - yLoc;
	CONSOLE_DEBUG_W_NUM("skyBoxHeight\t=", skyBoxHeight);
	CONSOLE_DEBUG_W_NUM("cWorkSpaceTopOffset\t=", cWorkSpaceTopOffset);

	SetWidget(				kSkyTravel_NightSky,	0,	yLoc,		cWidth,		skyBoxHeight);
	SetWidgetType(			kSkyTravel_NightSky, 	kWidgetType_Graphic);
	SetWidgetBGColor(		kSkyTravel_NightSky,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kSkyTravel_NightSky,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kSkyTravel_NightSky,	true);

	SetSkyDisplaySize(cWidth, skyBoxHeight);
	UpdateButtonStatus();
}


//*****************************************************************************
static void	FormatTimeString(struct timeval *tv, char *timeString)
{
struct tm	*linuxTime;

	if ((tv != NULL) && (timeString != NULL))
	{
		linuxTime		=	gmtime(&tv->tv_sec);

		sprintf(timeString, "%02d:%02d:%02d",
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);

	}
}

//*****************************************************************************
static void	FormatTimeString_TM(struct tm *timeStruct, char *timeString)
{

	if ((timeStruct != NULL) && (timeString != NULL))
	{

		sprintf(timeString, "%02d:%02d:%02d",
								timeStruct->tm_hour,
								timeStruct->tm_min,
								timeStruct->tm_sec);
	}
}

//**************************************************************************************
void WindowTabSkyTravel::RunBackgroundTasks(void)
{
uint32_t			currentMilliSecs;
uint32_t			deltaMilliSecs;
struct timeval		currentTime;
char				utcTimeString[32];
char				siderialTimeString[32];
char				textBuff[64];
struct tm			utcTime;
struct tm			siderealTime;


	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdateTime_ms;
//	if (deltaMilliSecs > 60000)
	if ((deltaMilliSecs > 60000) || (cAutoAdvanceTime && (deltaMilliSecs > 250) && (cView_index <= 4)))
	{
		if (cAutoAdvanceTime)
		{
			//*	this makes it real time.
			SetCurrentTime();
			ForceUpdate();
		}
		cLastUpdateTime_ms		=	millis();

	}
	deltaMilliSecs		=	currentMilliSecs - cLastClockUpdateTime_ms;
	if (deltaMilliSecs >= 1000)
	{
		gettimeofday(&currentTime, NULL);
		FormatTimeString(&currentTime, utcTimeString);

		gmtime_r(&currentTime.tv_sec, &utcTime);
		CalcSiderealTime(&utcTime, &siderealTime, gObseratorySettings.Longitude);
		FormatTimeString_TM(&siderealTime, siderialTimeString);



		sprintf(textBuff, "U%s / S%s", utcTimeString, siderialTimeString);
		SetWidgetText(kSkyTravel_UTCtime, textBuff);



		cLastClockUpdateTime_ms		=	millis();
	}

#if 0
	//*	check for telescope information
	if (gTelescopeUpdated)
	{
	char	ra_dec_string[128];

		CONSOLE_DEBUG("gTelescopeUpdated");

		gTelescopeUpdated	=	false;

		if (strlen(gTelescopeErrorString) > 0)
		{
			SetWidgetText(kSkyTravel_Telescope_RA_DEC, gTelescopeErrorString);
			gTelescopeErrorString[0]	=	0;
		}
		else
		{
			sprintf(ra_dec_string, "%s / %s (%d)", gTelescopeRA_String, gTelescopeDecl_String, gTelescopeUpdateCnt);
			SetWidgetText(kSkyTravel_Telescope_RA_DEC, ra_dec_string);
		}
//-		SetWidgetChecked(kSkyTravel_ConnLX200, gLX200_ThreadActive);

		//*	this makes it real time.
		SetCurrentTime();

		ForceUpdate();
	}
#endif
}

//*****************************************************************************
void	WindowTabSkyTravel::UpdateButtonStatus(void)
{
	SetWidgetChecked(		kSkyTravel_Btn_DeepSky,			cDispOptions.dispDeep);
	SetWidgetChecked(		kSkyTravel_Btn_Names,			cDispOptions.dispNames);
	SetWidgetChecked(		kSkyTravel_Btn_CommonStarNames,	cDispOptions.dispCommonStarNames);
	SetWidgetChecked(		kSkyTravel_Btn_Lines,			cDispOptions.dispLines);
	SetWidgetChecked(		kSkyTravel_Btn_ConstOutline,	cDispOptions.dispConstOutlines);
	SetWidgetChecked(		kSkyTravel_Btn_Constellations,	cDispOptions.dispConstellations);
	SetWidgetChecked(		kSkyTravel_Btn_NGC,				cDispOptions.dispNGC);
	SetWidgetChecked(		kSkyTravel_Btn_Messier,			cDispOptions.dispMessier);
	SetWidgetChecked(		kSkyTravel_Btn_YaleCat,			cDispOptions.dispYale);
	SetWidgetChecked(		kSkyTravel_Btn_Hipparcos,		cDispOptions.dispHIP);


	SetWidgetChecked(		kSkyTravel_Btn_AutoAdvTime,		cAutoAdvanceTime);

	SetWidgetChecked(		kSkyTravel_Btn_Chart,			cChart);
	SetWidgetChecked(		kSkyTravel_Btn_Earth,			cDispOptions.dispEarth);
	SetWidgetChecked(		kSkyTravel_Btn_Grid,			cDispOptions.dispGrid);
	SetWidgetChecked(		kSkyTravel_Btn_Equator,			cDispOptions.dispEquator_line);
	SetWidgetChecked(		kSkyTravel_Btn_Ecliptic,		cDispOptions.ecliptic_line);
	SetWidgetChecked(		kSkyTravel_Btn_NightMode,		cNightMode);
	SetWidgetChecked(		kSkyTravel_Btn_Symbols,			cDispOptions.dispSymbols);
	SetWidgetChecked(		kSkyTravel_Btn_TscopeDisp,		cDispOptions.dispTelescope);

	SetWidgetNumber(		kSkyTravel_Btn_ZoomLevel,		cView_index);

}


int	gRecursiveCounter	=	0;

//*****************************************************************************
void	WindowTabSkyTravel::HandleKeyDown(const int keyPressed)
{
bool			reDrawSky;

	gRecursiveCounter++;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", (keyPressed & 0x007f));
	reDrawSky			=	true;
	switch(keyPressed & 0x0fff)
	{
		case '!':	//*	toggle night mode
			cNightMode	=	!cNightMode;
			break;

	//	case kLeftArrowKey:	//change azimuth
		case '1':	//change azimuth
		case 0x0f51:
			cAz0	+=	(0.002 *  (cView_index + 1));
			if (cAz0 > kTWOPI)
			{
				cAz0	-=	kTWOPI;
			}
			break;

	//	case kRightArrowKey:
		case '2':
		case 0x0f53:
			cAz0	-=	(0.002 * (cView_index + 1));
			if (cAz0 < -kTWOPI)
			{
				cAz0	+=	kTWOPI;
			}
			break;

//		case kUpArrowKey:		//change altitude
		case '.':
		case 0x0f52:
			cElev0	+=	(0.002 * (cView_index + 1));
			if (cElev0 >= kHALFPI)
			{
				cElev0	=	kHALFPI - kEPSILON;
			}
			break;

//		case kDownArrowKey:
		case ',':
		case 0x0f54:
			cElev0	-=	(0.002 * (cView_index + 1));
			if (cElev0 <= -kHALFPI)
			{
				cElev0	=	-(kHALFPI + kEPSILON);
			}
			break;

		case '@':
			cAutoAdvanceTime	=	!cAutoAdvanceTime;
			if (cAutoAdvanceTime)
			{
				SetCurrentTime();
			}
			break;

		case '#':	//*	toggle GRID
			cDispOptions.dispGrid	=	!cDispOptions.dispGrid;
			break;

		case '?':	//*	toggle Constellation lines (new stylle)
			cDispOptions.dispConstellations	=	!cDispOptions.dispConstellations;
			break;

		case 'C':	//*	toggle ECLIPTIC LINE
			cDispOptions.ecliptic_line	=	!cDispOptions.ecliptic_line;
			break;

		case 'c':	//*	toggle CHART MODE
			cChart	=	!cChart;
			break;

		case 'D':	//*	toggle DEEP SKY
			cDispOptions.dispDeep	=	!cDispOptions.dispDeep;
			break;

		case 'd':
			gDashedLines	=	!gDashedLines;
			break;

		case 'E':	//*	toggle EARTH
			cDispOptions.dispEarth	=	!cDispOptions.dispEarth;
			break;

		case 'e':	//*		goto EAST
			cTrack	=	false;
			cAz0	=	-(kHALFPI - kEPSILON);

			break;

		case 'f':	//*	goto nadir (feet)
			cTrack	=	false;
			cElev0	=	-(kHALFPI - kEPSILON);	//	go to feet
			break;

		case 'F':	//*	Draw Find cross hairs
//			DrawCrossHairsAtCenter();
			reDrawSky	=	false;
			break;

		case 'G':	//*	toggle NGC objects
			cDispOptions.dispNGC		=	!cDispOptions.dispNGC;
			break;

		case 'g':
			SetCurrentTime();
//-			cCurrLatLon.latitude		=	RADIANS(41.361090);
//-			cCurrLatLon.longitude		=	RADIANS(-74.980333);
			break;

		case 'H':	//*	toggle HORIZON LINE
			cDispOptions.dispHorizon_line	=	!cDispOptions.dispHorizon_line;
			break;

		case 'h':	//*	go to HORIZON
			cTrack	=	false;
			cElev0	=	2.0 * kEPSILON;
			break;

		case 'L':	//*	toggle LINES
			cDispOptions.dispLines	=	!cDispOptions.dispLines;
			break;

		case 'M':	//*	toggle Messier objects
			cDispOptions.dispMessier	=	!cDispOptions.dispMessier;
			break;

		case 'm':	//*	toggle common star names
			cDispOptions.dispCommonStarNames	=	!cDispOptions.dispCommonStarNames;
			break;


		case 'N': 	//*	toggle NAMES
			cDispOptions.dispNames	=	!cDispOptions.dispNames;
			break;

		case 'n':					//	go NORTH
			cTrack	=	false;
			cAz0	=	-kEPSILON;
			if (cView_index >= 14)
			{
				//*	attempt to make it center on the north pole.....
			//	CONSOLE_DEBUG_W_DBL("cElev0\t=", cElev0);
			//	cElev0	=	(kHALFPI / 2) - cCurrLatLon.latitude;
			//	cElev0	=	-cCurrLatLon.latitude;
			}
			break;

		case 'O':
			cDispOptions.dispConstOutlines	=	! 	cDispOptions.dispConstOutlines;
			break;

		case 'o':					//	go OPPOSITE
			cTrack	=	false;
			cAz0	=	cAz0 + PI;
			if (cAz0 > PI)
			{
				cAz0	-=	kTWOPI;
			}
			else if (cAz0 < -PI)
			{
				cAz0	+=	kTWOPI;
			}
			cElev0	=	-cElev0;
			break;

		case 'Q':	//*	toggle EQUATOR LINE
			cDispOptions.dispEquator_line	=	!cDispOptions.dispEquator_line;
			break;

		case 'r':		//*	reset to defaults
			ResetView();
			break;


		case 'S':	//*	toggle SYMBOLS
			cDispOptions.dispSymbols	=	!cDispOptions.dispSymbols;
			break;

		case 's':	//*	goto SOUTH
			cTrack	=	false;
			cAz0	=	PI;
			break;

		case 'T':			//* cTrack toggle
			cTrack		^=	1;	//*	toggle cTrack flag
			cTrack		&=	1;	//* clean higher bits
			cFindFlag	=	true;
			break;


		case 't':			//* telescope toggle
			cDispOptions.dispTelescope	=	!cDispOptions.dispTelescope;
			break;

		case 'w':	//*	goto WEST
			cTrack	=	false;
			cAz0	=	kHALFPI - kEPSILON;
			break;

		case 'x':
			cDispOptions.dispDraper		=	!cDispOptions.dispDraper;
			break;

		case 'y':
			cDispOptions.dispHYG_all	=	!cDispOptions.dispHYG_all;
			break;


		case 'Y':	//*	Toggle Yale catalog
			cDispOptions.dispYale		=	!cDispOptions.dispYale;
			break;

		case 'z':	//*	goto ZENITH
			cTrack	=	false;
			cElev0	=	kHALFPI - kEPSILON;
			break;

//		case kPageDnKey:
		case '>':	//*	zoom in
		case '+':	//*	zoom in
			SetView_Index(cView_index - 1);
			break;

//		case kPageUpKey:
		case '<':	//*	zoom out
		case '-':	//*	zoom out
			SetView_Index(cView_index + 1);
			break;

//		case '+':
//			cCurrentTime.negflag	=	false;
//			break;

//		case '-':
//			cCurrentTime.negflag	=	true;
//			break;

		case '|':			//* dome slit toggle
			cDispOptions.dispDomeSlit	=	!cDispOptions.dispDomeSlit;
			break;


		case 0x07:	//* cntl-g local time/Gmtime
			cCurrentTime.local_time_flag	^=	1;
			break;

		case '=':	//*	run some tests
			{
			int	ii;

				for (ii=0; ii<20; ii++)
				{
					printf("%3d\t%10.10f\t%10.10f\r\n",	ii,
														DEGREES(cStarDataPtr[ii].ra),
														DEGREES(cStarDataPtr[ii].decl));
				}
			}
			cDisplayedMagnitudeLimit		-=	0.5;

			gST_DispOptions.EarthDispMode++;
			if (gST_DispOptions.EarthDispMode >= 4)
			{
				gST_DispOptions.EarthDispMode	=	0;
			}
			break;

		case '/':
			CenterOnDomeSlit();
			break;

		default:
//			CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//			CONSOLE_DEBUG_W_HEX("keyPressed\t=", (keyPressed & 0x007f));
			reDrawSky			=	false;
			break;
	}
	if (reDrawSky)
	{
		ForceReDrawSky();
	}
	gRecursiveCounter--;
	if (gRecursiveCounter > 0)
	{
		CONSOLE_DEBUG_W_NUM("gRecursiveCounter\t=", gRecursiveCounter);
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessButtonClick(const int buttonIdx)
{
bool	reDrawSky;
char	searchText[128];

	reDrawSky	=	true;
	switch(buttonIdx)
	{
		case kSkyTravel_Btn_Reset:
			ResetView();
			break;

		case kSkyTravel_Btn_AutoAdvTime:
			cAutoAdvanceTime	=	!cAutoAdvanceTime;
			SetWidgetChecked(		kSkyTravel_Btn_AutoAdvTime,		cAutoAdvanceTime);
			if (cAutoAdvanceTime)
			{
				SetCurrentTime();
			}
			break;

		case kSkyTravel_Btn_Chart:
			cChart	=	!cChart;
			SetWidgetChecked(		kSkyTravel_Btn_Chart,		cChart);
			break;

		case kSkyTravel_Btn_DeepSky:
			cDispOptions.dispDeep	=	!cDispOptions.dispDeep;
			SetWidgetChecked(		kSkyTravel_Btn_DeepSky,	cDispOptions.dispDeep);
			break;

		case kSkyTravel_Btn_Names:
			cDispOptions.dispNames	=	!cDispOptions.dispNames;
			SetWidgetChecked(		kSkyTravel_Btn_Names,	cDispOptions.dispNames);
			break;

		case kSkyTravel_Btn_CommonStarNames:
			cDispOptions.dispCommonStarNames	=	!cDispOptions.dispCommonStarNames;
			SetWidgetChecked(		kSkyTravel_Btn_CommonStarNames,	cDispOptions.dispCommonStarNames);
			break;

		case kSkyTravel_Btn_Lines:
			cDispOptions.dispLines	=	!cDispOptions.dispLines;
			SetWidgetChecked(		kSkyTravel_Btn_Lines,	cDispOptions.dispLines);
			break;

		case kSkyTravel_Btn_ConstOutline:
			cDispOptions.dispConstOutlines	=	!cDispOptions.dispConstOutlines;
			SetWidgetChecked(		kSkyTravel_Btn_ConstOutline,cDispOptions.dispConstOutlines);
			break;

		case kSkyTravel_Btn_Constellations:
			cDispOptions.dispConstellations	=	!cDispOptions.dispConstellations;
			SetWidgetChecked(		kSkyTravel_Btn_Constellations, cDispOptions.dispConstellations);
			break;

		case kSkyTravel_Btn_NGC:
			cDispOptions.dispNGC		=	!cDispOptions.dispNGC;
			SetWidgetChecked(		kSkyTravel_Btn_NGC,	cDispOptions.dispNGC);
			break;

		case kSkyTravel_Btn_Earth:
			cDispOptions.dispEarth		=	!cDispOptions.dispEarth;
			SetWidgetChecked(		kSkyTravel_Btn_Earth,	cDispOptions.dispEarth);
			break;

		case kSkyTravel_Btn_Grid:
			cDispOptions.dispGrid	=	!cDispOptions.dispGrid;
			SetWidgetChecked(		kSkyTravel_Btn_Grid,	cDispOptions.dispGrid);
			break;

		case kSkyTravel_Btn_Equator:
			cDispOptions.dispEquator_line	=	!cDispOptions.dispEquator_line;
			SetWidgetChecked(		kSkyTravel_Btn_Equator,	cDispOptions.dispEquator_line);
			break;

		case kSkyTravel_Btn_Ecliptic:
			cDispOptions.ecliptic_line	=	!cDispOptions.ecliptic_line;
			SetWidgetChecked(		kSkyTravel_Btn_Ecliptic,	cDispOptions.ecliptic_line);
			break;

		case kSkyTravel_Btn_YaleCat:
			cDispOptions.dispYale		=	!cDispOptions.dispYale;
			SetWidgetChecked(		kSkyTravel_Btn_YaleCat,	cDispOptions.dispYale);
			break;

		case kSkyTravel_Btn_Messier:
			cDispOptions.dispMessier		=	!cDispOptions.dispMessier;
			SetWidgetChecked(		kSkyTravel_Btn_Messier,		cDispOptions.dispMessier);
			break;

		case kSkyTravel_Btn_Hipparcos:
			cDispOptions.dispHIP	=	!cDispOptions.dispHIP;
			SetWidgetChecked(		kSkyTravel_Btn_Hipparcos,	cDispOptions.dispHIP);
			break;

		case kSkyTravel_Btn_NightMode:
			cNightMode		=	!cNightMode;
			SetWidgetChecked(		kSkyTravel_Btn_NightMode,	cNightMode);
			break;

		case kSkyTravel_Btn_Symbols:
			cDispOptions.dispSymbols		=	!cDispOptions.dispSymbols;
			SetWidgetChecked(		kSkyTravel_Btn_Symbols,	cDispOptions.dispSymbols);
			break;

		case kSkyTravel_Btn_TscopeDisp:
			cDispOptions.dispTelescope		=	!cDispOptions.dispTelescope;
			SetWidgetChecked(kSkyTravel_Btn_TscopeDisp,	cDispOptions.dispTelescope);
			break;


		case kSkyTravel_Btn_Plus:		//*	zoom in
			SetView_Index(cView_index - 1);
			break;

		case kSkyTravel_Btn_Minus:	//*	zoom out
			SetView_Index(cView_index + 1);
			break;

		case kSkyTravel_Search_Btn:
			GetWidgetText(kSkyTravel_Search_Text, searchText);
			SearchSkyObjects(searchText);
			break;


		case kSkyTravel_Telescope_Sync:
			SyncTelescopeToCenter();
			break;

		case kSkyTravel_Telescope_GoTo:
			SlewTelescopeToCenter();
			break;


		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
			reDrawSky	=	false;

			break;

	}
	if (reDrawSky)
	{
		ForceReDrawSky();
	}
}


//*****************************************************************************
//*	convert Right Ascension from radians to hours (0.0 -> 24.0)
//*****************************************************************************
static double	ConvertRightAscensionToHours(double rightAscen_radians)
{
double		myRightAscen_Hours;

	//*	convert Right Ascension from radians to hours (0.0 -> 24.0)
	myRightAscen_Hours	=	DEGREES(rightAscen_radians / 15.0);
	while (myRightAscen_Hours < 0.0)
	{
		myRightAscen_Hours	+=	24.0;
	}
	while (myRightAscen_Hours >= 24.0)
	{
		myRightAscen_Hours	-=	24.0;
	}
	return(myRightAscen_Hours);
}

//*****************************************************************************
//	RightAscension=12.123&Declination=34.123
//*****************************************************************************
bool	WindowTabSkyTravel::SyncTelescopeToCenter(void)
{
bool			validData;
char			dataString[64];
SJP_Parser_t	jsonParser;
double			myRightAscen_Hours;

	//*	convert Right Ascension from radians to hours (0.0 -> 24.0)
	myRightAscen_Hours	=	ConvertRightAscensionToHours(cRa0);
	sprintf(dataString, "RightAscension=%f&Declination=%f", myRightAscen_Hours, DEGREES(cDecl0));
	CONSOLE_DEBUG_W_STR("Sending", dataString);

	validData	=	SendAlpacaCmdToTelescope("synctocoordinates",	dataString, &jsonParser);
	return(validData);
}


//*****************************************************************************
//	RightAscension=12.123&Declination=34.123
//*****************************************************************************
bool	WindowTabSkyTravel::SlewTelescopeToCenter(void)
{
bool			validData;
char			dataString[64];
SJP_Parser_t	jsonParser;
double			myRightAscen_Hours;


//	CONSOLE_DEBUG_W_DBL("DEGREES(cRa0 / 15.0)\t=", DEGREES(cRa0 / 15.0));
//	CONSOLE_DEBUG_W_DBL("DEGREES(cDecl0)\t\t=", DEGREES(cDecl0));

	myRightAscen_Hours	=	ConvertRightAscensionToHours(cRa0);

	sprintf(dataString, "RightAscension=%f&Declination=%f", myRightAscen_Hours, DEGREES(cDecl0));
//	CONSOLE_DEBUG_W_STR("Sending", dataString);

	validData	=	SendAlpacaCmdToTelescope("slewtocoordinatesasync",	dataString, &jsonParser);
	return(validData);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SendAlpacaCmdToTelescope(	const char		*theCommand,
														const char		*dataString,
														SJP_Parser_t	*jsonParser)
{
bool				validData	=	false;
ControllerSkytravel	*myControllerObj;
char				ipAddrStr[32];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, theCommand);

	myControllerObj	=	(ControllerSkytravel *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		if (myControllerObj->cTelescopeAddressValid)
		{
			inet_ntop(AF_INET, &(myControllerObj->cTelescopeIpAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
			CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

			validData	=	AlpacaSendPutCmd(	&myControllerObj->cTelescopeIpAddress,
												myControllerObj->cTelescopeIpPort,
												"telescope",
												myControllerObj->cTelescopeAlpacaDeviceNum,
												theCommand,
												dataString,
												jsonParser);
		}
		else
		{
			CONSOLE_DEBUG("Telescope IP address not valid");
		}
	}
	else
	{
		CONSOLE_DEBUG("myControllerObj is NULL");
	}
	return(validData);
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessDoubleClick(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
bool			reDrawSky;

	reDrawSky	=	true;

	switch(widgetIdx)
	{
		case kSkyTravel_Btn_ZoomLevel:
			SetView_Index(8);
			break;

		//*	double click in the RA/DEC box from the Alpaca telescope device and makes it center on the current telescope position
		case kSkyTravel_Telescope_RA_DEC:
			cDispOptions.dispTelescope	=	true;
			cRa0		=	gTelescopeRA_Radians;
			cDecl0		=	gTelescopeDecl_Radians;
			cFindFlag	=	true;

			break;

		//*	center on the double clicked locations
		case kSkyTravel_NightSky:
			cAz0	=	cCursor_az;
			cElev0	=	cCursor_elev;
			break;

		default:
			reDrawSky	=	false;
			break;
	}
	if (reDrawSky)
	{
		ForceReDrawSky();

		CONSOLE_DEBUG(__FUNCTION__);
		printf("Center of screen=%07.4f:%07.4f\r\n", DEGREES(cRa0 / 15), DEGREES(cDecl0));

	}
}


//*****************************************************************************
void	WindowTabSkyTravel::ForceReDrawSky(void)
{
ControllerImage	*myControllerObj;

	UpdateButtonStatus();

	if (cMouseDragInProgress == false)
	{
		cCurrentTime.precflag	=	true;	//*	force precession
		CalanendarTime(&cCurrentTime);
		Precess();
	}

	//*	now force the window to update
	myControllerObj	=	(ControllerImage *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cUpdateWindow		=	true;
		myControllerObj->HandleWindowUpdate();
		myControllerObj->cUpdateWindow		=	false;
		cvWaitKey(20);
	}
}


//*****************************************************************************
void	WindowTabSkyTravel::ProcessMouseEvent(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
	if (widgetIdx == kSkyTravel_NightSky)
	{
//		CONSOLE_DEBUG("kSkyTravel_NightSky");
		cCsrx	=	xxx;
		cCsry	=	yyy;

		cCsrx	-=	cWorkSpaceLeftOffset;
		cCsry	-=	cWorkSpaceTopOffset;

		//*	still need the offset for doing cursor calculations
		cCsry	-=	cCursorOffsetY;

		Compute_cursor(&cCurrentTime, &cCurrLatLon);

		if (cStarCount < 10000)
		{
			FindObjectNearCursor();
		}
		else
		{
			cInform_dist	=	0x7fff;
			cInform_id	=	-1;
		}

		DrawCursorLocationInfo();

		UpdateWindowAsNeeded();

//		SetCursorFromXY(argLocalPt->h, argLocalPt->v);
	}
	else
	{
//		CONSOLE_DEBUG(__FUNCTION__);
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessMouseLeftButtonDown(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);

	if (widgetIdx == kSkyTravel_NightSky)
	{
		cSavedMouseClick_X	=	xxx;
		cSavedMouseClick_Y	=	yyy;

	}
	else
	{
//		CONSOLE_DEBUG(__FUNCTION__);
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessMouseLeftButtonUp(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
	if (widgetIdx == kSkyTravel_NightSky)
	{
//		if (cDispOptions.dispHIP)
		{
			ForceReDrawSky();
		}
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessMouseLeftButtonDragged(	const int	widgetIdx,
															const int	event,
															const int	xxx,
															const int	yyy,
															const int	flags)
{
double				deltaXX;
double				deltaYY;
double				moveAmount;
TYPE_SkyDispOptions	savedDispOptions;

	if (cMouseDragInProgress == false)
	{
		cMouseDragInProgress	=	true;
//		CONSOLE_DEBUG("--------------------------------------------------------");
//		CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);

		if (widgetIdx == kSkyTravel_NightSky)
		{

			deltaXX	=	xxx - cSavedMouseClick_X;
			deltaYY	=	yyy - cSavedMouseClick_Y;

			cSavedMouseClick_X	=	xxx;
			cSavedMouseClick_Y	=	yyy;

		//	CONSOLE_DEBUG_W_NUM("cView_index\t=", cView_index);
			switch (cView_index )
			{
				//*	these values were determined by good old fashion trial and error
				case 0:		moveAmount	=	0.00002;	break;
				case 1:		moveAmount	=	0.00002;	break;
				case 2:		moveAmount	=	0.00003;	break;
				case 3:		moveAmount	=	0.00005;	break;
				case 4:		moveAmount	=	0.00008;	break;
				case 5:		moveAmount	=	0.00015;	break;
				case 6:		moveAmount	=	0.00020;	break;
				case 7:		moveAmount	=	0.00022;	break;
				default:	moveAmount	=	0.00030;	break;
			}
		//	moveAmount	=	cView_angle / cWind_width;
		//	CONSOLE_DEBUG_W_DBL("moveAmount\t=", moveAmount);
			//--------------------------------------
			cAz0	+=	(deltaXX * moveAmount *  (cView_index + 1));
			if (cAz0 > kTWOPI)
			{
				CONSOLE_DEBUG(" > kTWOPI");
				cAz0	-=	kTWOPI;
			}
			if (cAz0 < -kTWOPI)
			{
//				CONSOLE_DEBUG(" < kTWOPI");
				cAz0	+=	kTWOPI;
			}
			//--------------------------------------
			cElev0	+=	(deltaYY * moveAmount * (cView_index + 1));
			if (cElev0 >= kHALFPI)
			{
//				CONSOLE_DEBUG(" > kHALFPI");
				cElev0	=	kHALFPI - kEPSILON;
			}
			if (cElev0 <= -kHALFPI)
			{
				CONSOLE_DEBUG(" < kHALFPI");
				cElev0	=	-(kHALFPI + kEPSILON);
			}

			//*	we cannot draw the hippacors database while dragging
			savedDispOptions			=	cDispOptions;
			cDispOptions.dispHIP		=	false;
			cDispOptions.dispYale		=	false;
			cDispOptions.dispNGC		=	false;
			if (cView_index > 3)
			{
				cDispOptions.dispHYG_all	=	false;
				cDispOptions.dispDraper		=	false;
			}

			ForceReDrawSky();
			//*	restore to original state
			cDispOptions			=	savedDispOptions;
		}
		else
		{
	//		CONSOLE_DEBUG(__FUNCTION__);
		}
		cMouseDragInProgress	=	false;
	}
	else
	{
	//	CONSOLE_DEBUG("cMouseDragInProgress");
	}
}


//*****************************************************************************
//* compute cursor elev, az, ra, decl
void	WindowTabSkyTravel::Compute_cursor(TYPE_Time *timeptr, TYPE_LatLon *locptr)
{
double			aa, bb, cc;
TYPE_SpherTrig	sph;


	aa	=	(cCsrx - cWind_x0) / cXfactor;
	bb	=	-(cCsry - cWind_y0) / cYfactor;

	if ((fabs(aa) + fabs(bb)) < kEPSILON)	//* very close to center?
	{
		cCursor_az		=	cAz0;
		cCursor_elev	=	cElev0;
		cCursor_ra		=	cRa0;
		cCursor_decl	=	cDecl0;
	}
	else
	{
		sph.cside			=	sqrt((aa * aa) + (bb * bb));
		cc					=	sph.cside;
		sph.alpha			=	acos(aa / cc);
		if (bb < 0.)
		{
			sph.alpha	*=	-1.0;			//*	negate
		}
		sph.alpha			+=	1.5 * PI;	//*	always add 270 degrees
		if (sph.alpha > kTWOPI)
		{
			sph.alpha	-=	kTWOPI;
		}
		//***************************************************************************************
		//*	from Clif Aschcraft 1/9/2021
		//*	I don’t remember enough about the code to make the statement you were looking for,
		//*	except that chart mode is strictly done in RA and Dec, completely ignoring the horizon,
		//*	Alt/El coordinates, and the angle that Dec circles make with the horizon.
		//*	North is always up on the charts, and the RA meridian in the center of the chart is a vertical
		//*	straight line and the Dec circle going through the center is a horizontal straight line.
		//*	Just like a printed star chart should be, ie, no longer a planetarium showing the sky as
		//*	it looks to a person standing perpendicular to the earth’s surface.
		//***************************************************************************************
//		if (0)
		if (cChart)
		{
			//*	chart mode calculations
			sph.bside		=	kHALFPI - cDecl0;
			sphsas(&sph);
			cCursor_decl	=	kHALFPI - sph.aside;
			sphsss(&sph);
			cCursor_ra		=	sph.gamma + cRa0;

			sph.bside		=	kHALFPI - cElev0;
			sph.alpha		=	sph.alpha - cChart_gamma;
			sphsas(&sph);
			cCursor_elev	=	kHALFPI - sph.aside;
			sphsss(&sph);
			cCursor_az		=	sph.gamma + cAz0;
		}
		else
		{
			//*	normal mode calculations
			sph.bside		=	kHALFPI - cElev0;
			sphsas(&sph);
			cCursor_elev	=	kHALFPI - sph.aside;
			sphsss(&sph);
			cCursor_az		=	sph.gamma + cAz0;

			sph.cside		=	kHALFPI - cCursor_elev;
			sph.bside		=	kHALFPI - locptr->latitude;
			sph.alpha		=	cCursor_az;
			sphsas(&sph);
			cCursor_decl	=	kHALFPI - sph.aside;
			sphsss(&sph);
			cCursor_ra		=	(kTWOPI *timeptr->fSiderealTime) - sph.gamma + locptr->longitude;
		}

		//*	Jan  2,	2021	<MLS> Added fix to displayed cursor RA to make sure its > 0
//		CONSOLE_DEBUG_W_DBL("cCursor_ra\t=", cCursor_ra);
		while (cCursor_ra < 0.0)
		{
			cCursor_ra	+=	kTWOPI;
		}
	}

//	if (OptionKeyDown() && ControlKeyDown())
//	{
//		gTelescopeRA_Radians	=	cCursor_ra;
//		gTelescopeDecl_Radians	=	cCursor_decl;
//		UpdateTelescopePosition();
//	}
}


//*****************************************************************************
//* convert az/el to screen coordinates,
//*	this is basically Compute_cursor() in reverse
//*****************************************************************************
bool	WindowTabSkyTravel::GetXYfromAz_Elev(double azimuth_radians, double elev_radians, short *xx, short *yy)
{
bool			ptIsOnScreen;
TYPE_SpherTrig	sph;
short			xcoord;
short			ycoord;

	ptIsOnScreen	=	false;
#if 0
double			aa, bb, cc;
	sph.cside		=	kHALFPI - elev_radians;
	sph.bside		=	kHALFPI - cCurrLatLon.latitude;
	sph.alpha		=	azimuth_radians;
	sphsas(&sph);
//	cCursor_decl	=	kHALFPI - sph.aside;
	sph.aside		=	kHALFPI - cCursor_decl;
	sphsss(&sph);

//	cCursor_az		=	sph.gamma + cAz0;
	sph.gamma		=	azimuth_radians - cAz0;

	sph.bside		=	kHALFPI - cElev0;
	sphsas(&sph);
//	cCursor_elev	=	kHALFPI - sph.aside;
//	cCursor_elev + sph.aside	=	kHALFPI;
	sph.aside		=	kHALFPI - elev_radians;
	sphsss(&sph);
#endif // 0


#if 0
TYPE_LatLon	myLatLon;

	myLatLon.latitude	=	elev_radians;
	myLatLon.longitude	=	azimuth_radians;
	ConvertLatLonToRaDec(&myLatLon, &cCurrentTime);


#else
	//*	this kind of works
	//*	new code as 1/8/2021
	//*	lifted from MapTokens()
	sph.bside		=	kHALFPI - cElev0;
		sph.bside	+=	elev_radians;			//*	added...

	sph.cside		=	kHALFPI - kEPSILON;
	sph.alpha		=	cAz0 - azimuth_radians;
	if (sph.alpha > PI)
	{
		sph.alpha	-=	kTWOPI;
	}
	else if (sph.alpha < -PI)
	{
		sph.alpha	+=	kTWOPI;
	}
	sphsas(&sph);
	sphsss(&sph);
	xcoord		=	cWind_x0 + (cXfactor * sph.aside * sin(sph.gamma));
	if ((xcoord >= wind_ulx) && (xcoord <= (wind_ulx + cWind_width)))
	{
		ycoord	=	cWind_y0 - (cYfactor * sph.aside * cos(sph.gamma));
		if ((ycoord >= wind_uly) && (ycoord <= (wind_uly + cWind_height)))
		{
			ptIsOnScreen	=	true;
			*xx				=	xcoord;
			*yy				=	ycoord;
		}
	}
#endif
	return(ptIsOnScreen);
}


//*********************************************************************
void	WindowTabSkyTravel::SetSkyDisplaySize(short xSize, short ySize)
{
	wind_ulx		=	0;
	wind_uly		=	0;


	cWind_width		=	xSize - (2 * wind_ulx);	//*620 700
//?	cWind_height	=	ySize - (2 * wind_uly);	//*460 500
	//*	leave room for text
//	cWind_height	=	ySize - cWorkSpaceTopOffset;
	cWind_height	=	ySize;


	cWind_x0	=	wind_ulx + (cWind_width / 2);
	cWind_y0	=	wind_uly + (cWind_height / 2);

	cXfactor	=	cWind_width / cView_angle;
	cYfactor	=	cXfactor;			//* 1:1 aspect ratio

	cWind_height	=	ySize;

}

//*********************************************************************
void	WindowTabSkyTravel::SetCurrentTime(void)
{
struct timeval	currentTimeVal;
struct tm		*linuxTime;

//	CONSOLE_DEBUG(__FUNCTION__);

	gettimeofday(&currentTimeVal, NULL);

	linuxTime	=	gmtime(&currentTimeVal.tv_sec);

	cCurrentTime.year	=	(1900 + linuxTime->tm_year);
	cCurrentTime.month	=	(1 + linuxTime->tm_mon);
	cCurrentTime.day	=	linuxTime->tm_mday;
	cCurrentTime.hour	=	linuxTime->tm_hour;
	cCurrentTime.min	=	linuxTime->tm_min;
	cCurrentTime.sec	=	linuxTime->tm_sec;

	CalanendarTime(&cCurrentTime);

}

//**************************************************************************
//* convert lat, long, elevation, azimuth to ra, dec
//* enter with new lat, long in locptr->latitude, locptr->longitude
//**************************************************************************
void	WindowTabSkyTravel::ConvertLatLonToRaDec(TYPE_LatLon *locptr, TYPE_Time *timeptr)
{
double			temp;
TYPE_SpherTrig	sphptr;
double			xangle, yangle, rangle;

	sphptr.bside	=	kHALFPI - locptr->latitude;
	sphptr.cside	=	kHALFPI - cElev0;
	sphptr.alpha	=	cAz0;
	sphsas(&sphptr);
	sphsss(&sphptr);
	cDecl0	=	kHALFPI - sphptr.aside;
	cRa0	=	(kTWOPI * timeptr->fSiderealTime) - sphptr.gamma + locptr->longitude;
	if (cRa0 > PI)
	{
		cRa0	-=	kTWOPI;
	}
	else if (cRa0 < -PI)
	{
		cRa0	+=	kTWOPI;
	}
	temp			=	sphptr.bside;
	sphptr.bside	=	sphptr.cside;	//* swap b and c
	sphptr.cside	=	temp;
	sphsas(&sphptr);
	sphsss(&sphptr);
	cGamang			=	sphptr.gamma;
	if (cChart)
	{
		cChart_gamma	=	cGamang;
		cGamang			=	0.0;
	}


	cRamax	=	0.0;	//*	default ramax=0.
	temp	=	kHALFPI - fabs(cDecl0);
	if (temp > cRadmax)
	{
		cRamax	=	asin(sin(cRadmax) / sin(temp));
	}

	xangle		=	cView_angle / 2.0;
	yangle		=	xangle * cWind_height / cWind_width;
	rangle		=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
	cRadmax		=	rangle;

}

//*****************************************************************************************
void	WindowTabSkyTravel::PlotObjectsByDataSource(TYPE_CelestData	*objectptr, long maxObjects)
{
short	dataSource;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (objectptr != NULL)
	{
		//*	all of the objects in a list should have come from the same source,
		//*	therefore look at the type of the first object
		dataSource	=	objectptr[0].dataSrc;

		switch(dataSource)
		{
			case kDataSrc_YaleBrightStar:
			case kDataSrc_NGC2000:
			case kDataSrc_NGC2000IC:
			case kDataSrc_HubbleGSC:
			case kDataSrc_Hipparcos:
//				if (ControlKeyDown())
				{
					Search_and_plot(objectptr, maxObjects);
				}
//				else
//				{
//					PlotObjects(objectptr, maxObjects);
//				}
				break;

			case kDataSrc_Orginal:			//*	Frank and Cliffs original data file
			default:
				Search_and_plot(objectptr, maxObjects);
				break;
		}
	}
	else
	{
		CONSOLE_DEBUG("objectptr is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
//* find_altaz - given location, ra,dec, sidereal time, compute az, elev
//*****************************************************************************
void	WindowTabSkyTravel::FindAltAz(TYPE_LatLon *locptr, TYPE_Time *timeptr)
{
TYPE_SpherTrig	sphptr;

	sphptr.alpha	=	kTWOPI * timeptr->fSiderealTime - cRa + locptr->longitude;
	if (sphptr.alpha>PI)
	{
		sphptr.alpha	-=	kTWOPI;
	}
	else if (sphptr.alpha<-PI)
	{
		sphptr.alpha	+=	kTWOPI;
	}
	sphptr.bside	=	kHALFPI - locptr->latitude;
	sphptr.cside	=	kHALFPI - cDecl;
	sphsas(&sphptr);
	sphsss(&sphptr);
	cElev	=	kHALFPI - sphptr.aside;
	cAz		=	sphptr.gamma;
}


//*********************************************************************
void	WindowTabSkyTravel::DrawSkyAll(void)
{
short		ii;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

	CalanendarTime(&cCurrentTime);
	Local_Time(&cCurrentTime);		//* compute local time from gmt and timezone

	eph(	&cCurrentTime,
			&cCurrLatLon,
//?			(planet_struct *)&cPlanetStruct,
			cPlanetStruct,	//*	changed 2/4/2021
			&cSunMonStruct);

//	cCurrentSkyColor	=	Set_Skycolor((planet_struct *)&cPlanetStruct, &cSunMonStruct, cDispOptions.dispEarth);
#ifdef _ENBABLE_WHITE_CHART_
	if (cChart)
	{
		cSkyRGBvalue.red	=	255;
		cSkyRGBvalue.grn	=	255;
		cSkyRGBvalue.blu	=	255;
	}
	else
#endif
	{
		cSkyRGBvalue.red	=	0;
		cSkyRGBvalue.grn	=	0;
		cSkyRGBvalue.blu	=	0;
	}
	SetWidgetBGColor(kSkyTravel_NightSky, CV_RGB(	cSkyRGBvalue.red,
													cSkyRGBvalue.grn,
													cSkyRGBvalue.blu));


	//* transfer planetary ra/dec data except for moon
	for (ii=1;ii<10;ii++)
	{
		cPlanets[ii].decl	=	cPlanetStruct[ii].decl;
		cPlanets[ii].ra		=	cPlanetStruct[ii].ra;
		cPlanets[ii].magn	=	cPlanetStruct[ii].magn;
	}

	//* moon topo coords

	cPlanets[0].decl	=	cSunMonStruct.mon_topo_decl;
	cPlanets[0].ra		=	cSunMonStruct.mon_topo_ra;

	mon_geo_decl		=	cPlanetStruct[MON].decl;
	mon_geo_ra			=	cPlanetStruct[MON].ra;

	//*	needed for crescent moon and sun graphics

	cPhase_angle			=	cSunMonStruct.smdist;
	cPosition_angle			=	-cSunMonStruct.smang;

	cSun_radius				=	cPlanetStruct[SUN].radius;
	cMoon_radius			=	cPlanetStruct[MON].radius;
	cEarth_shadow_radius	=	cSunMonStruct.earth_shadow_radius;

	if (cSunMonStruct.lunar_ecl_flag)
	{
		cLunarEclipseFlag	=	true;
	}
	else
	{
		cLunarEclipseFlag	=	FALSE;
	}


	//* if track or find, force center of field to match ra and decl

	if (cTrack || cFindFlag)
	{
		if (cTrack > 1)	//* track a planet
		{
			ii		=	(cTrack / 2) - 1;	//* extract the planet #
			cRa		=	cPlanets[ii].ra;
			cDecl	=	cPlanets[ii].decl;
		}
		else
		{
			cRa		=	cRa0 + kEPSILON;
			cDecl	=	cDecl0 + kEPSILON;
		}
		FindAltAz(&cCurrLatLon, &cCurrentTime);
		cAz0		=	cAz;
		cElev0		=	cElev;
		cCsrx		=	cWind_x0;
		cCsry		=	cWind_y0;
		cFindFlag	=	false;	//* always reset
	}


	ConvertLatLonToRaDec(&cCurrLatLon, &cCurrentTime);

//	DRAW HERE

#ifdef _ENABLE_HYG_
	//*	draw the dense stuff first so the other stuff is on top
	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispHYG_all && (cHYGObjectPtr != NULL) && (cHYGObjectCount > 0))
	{
		PlotObjectsByDataSource(cHYGObjectPtr, cHYGObjectCount);
	}
#endif
	//*--------------------------------------------------------------------------------
	//*	draw the faint Hipparcos stuff first
	if (cDispOptions.dispHIP && (cHipObjectPtr != NULL) && (cHipObjectCount > 0))
	{
		PlotObjectsByDataSource(cHipObjectPtr, cHipObjectCount);
	}

	if (cDispOptions.dispDraper && (cDraperObjectPtr != NULL) && (cDraperObjectCount > 0))
	{
		PlotObjectsByDataSource(cDraperObjectPtr, cDraperObjectCount);
	}


	//*--------------------------------------------------------------------------------
	//*	if we are to much zoomed in, dont bother with the outlines
	if (cDispOptions.dispConstOutlines && (cView_index > 4))
	{
		DrawConstellationOutLines();
	}
	//*--------------------------------------------------------------------------------
	//*	this is my new constellation vectors, far better than the original ones
	if (cDispOptions.dispConstellations)
	{
		DrawConstellationVectors();
	}

	//*--------------------------------------------------------------------------------
	//*	common star names are added to the Hipparcos data, we plot it separately
	if (cDispOptions.dispCommonStarNames && (cView_index < 9))
	{
		DrawCommonStarNames();
	}


	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispLines && (cView_index > 3))
	{
		DrawConstellationLines();
	}
	//*--------------------------------------------------------------------------------
	if ((cStarDataPtr != NULL) && (cStarCount > 0))
	{
		PlotObjectsByDataSource(cStarDataPtr, cStarCount);
	}
	else
	{
		CONSOLE_DEBUG("cStarDataPtr is NULL")
		CONSOLE_ABORT("cStarDataPtr is NULL")
	}

	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispYale && (cYaleStarDataPtr != NULL) && (cYaleStarCount > 0))
	{
		PlotObjectsByDataSource(cYaleStarDataPtr, cYaleStarCount);
	}

	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispNGC && (cNGCobjectPtr != NULL) && (cNGCobjectCount > 0))
	{
		PlotObjectsByDataSource(cNGCobjectPtr, cNGCobjectCount);
	}


	//*--------------------------------------------------------------------------------
	if (gZodiacPtr != NULL)
	{
//		CONSOLE_DEBUG("Plotting zodiac");
		PlotSkyObjects(gZodiacPtr, gZodiac_names, zodiac_shapes, kZodiacCount);
	}

	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispMessier && (cMessierOjbectPtr != NULL) && (cMessierOjbectCount > 0))
	{
//		CONSOLE_DEBUG("Plotting Messier objects");
		PlotObjectsByDataSource(cMessierOjbectPtr, cMessierOjbectCount);
	}


	//*--------------------------------------------------------------------------------
	if ((cSpecialObjectPtr != NULL) && (cSpecialObjectCount > 0))
	{
		PlotObjectsByDataSource(cSpecialObjectPtr, cSpecialObjectCount);
	}


	PlotSkyObjects(cPlanets, gPlanet_names, planet_shapes, kPlanetObjectCnt);	//* planets

	SetColor(BLACK);

}

//*****************************************************************************
void	WindowTabSkyTravel::SetView_Index(const int newViewIndex)
{
	cView_index		=	newViewIndex;

	if (cView_index < 0)
	{
		cView_index	=	0;
	}
	if (cView_index > kMaxViewAngleIndex)
	{
		cView_index	=	kMaxViewAngleIndex;
	}
	cView_angle	=	gView_table[cView_index];

}

//*****************************************************************************
void	WindowTabSkyTravel::ResetView(void)
{
	SetView_Index(8);

	cElev0								=	kHALFPI / 2;		//* 45 degrees
	cAz0								=	0.0;				//* north
	cRa0								=	0.0;
	cDecl0								=	0.0;
	cDisplayedMagnitudeLimit			=	15.0;

	cChart								=	false;
	cDispOptions.dispDeep				=	true;
	cDispOptions.dispEarth				=	true;
	cDispOptions.dispGrid				=	true;
	cDispOptions.dispHorizon_line		=	true;
	cDispOptions.dispNames				=	true;
	cDispOptions.dispNGC				=	false;
 	cDispOptions.dispSymbols			=	true;
 	cDispOptions.dispMessier			=	true;
 	cDispOptions.dispConstOutlines		=	true;
 	cDispOptions.dispConstellations		=	true;
	cDispOptions.dispHIP				=	false;
	cDispOptions.dispCommonStarNames	=	true;
	cDispOptions.dispHYG_all			=	false;
	cDispOptions.dispDraper				=	false;

	if (cConstVecotrPtr != NULL)
	{
		//*	if we successfully read in the good constellation lines, turn off the old ones
		cDispOptions.dispConstellations	=	true;
		cDispOptions.dispLines			=	false;
	}
	else
	{
		cDispOptions.dispLines			=	true;
	}
	UpdateButtonStatus();
}

//*****************************************************************************
//*	set the skycolor as function of sun's elevetion if no eclipse
//*	if eclipse, determine the area not eclipsed
//*	the skycolor is the darker of the two; this gives the proper sky
//*	color if eclipse is happening below the horizon
//*****************************************************************************
short	WindowTabSkyTravel::Set_Skycolor(	planet_struct	*planptr,
											sun_moon_struct	*sunmonptr,
											bool			earthFlag)
{
int			skycolor,skycolor1,skycolor2;			//* skycolor1 from sun elevation, 2 from solar eclipse
double		r1,r2,d,d1,d2,theta1,theta2,a0,a1,a2;	//* 1 = sun 2 = moon
double		frac;

	skycolor	=	0;	//* dark sky if no earth

	if (cNightMode)
	{
		cSkyRGBvalue.red	=	0;
		cSkyRGBvalue.grn	=	0;
		cSkyRGBvalue.blu	=	0;
		return(skycolor);
	}



	if (earthFlag)
	{
		skycolor2	=	63;	//* bright sky
		if (sunmonptr->solar_ecl_flag)
		{
		//* a1 is area of eclipsed segment of Sun, a2 is area of eclipsed segment of Moon
		//* a0 is area of Sun
		//* frac is fractional area of Sun visible

			r1	=	planptr[SUN].radius;
			r2	=	planptr[MON].radius;
			d	=	sunmonptr->smdist;	//* dist between centers Sun-Moon
			a0	=	PI * r1 * r1;
			if (d <= fabs(r1-r2))		//* if true we have a total eclipse
			{
				if (r2>r1)
				{
					frac	=	0.0;		//* full total eclipse
				}
				if (r1>=r2)
				{
					frac	=	PI*((r1 + r2)*(r1-r2))/a0;	//* annular total eclipse
				}
			}
			else	//* its a partial eclipse
			{
				d1		=	((r1 + r2)*(r1-r2) + (d*d))/(2.*d);
				d2		=	d - d1;
				theta1	=	2. * acos(d1/r1);
				theta2	=	2. * acos(d2/r2);
				a1		=	((r1 * r1) * (theta1 - sin(theta1))) / 2.0;	//* area of Sun segment
				a2		=	((r2 * r2) * (theta2 - sin(theta2))) / 2.0;	//* area of Moon segment
				frac	=	1.-((a1 + a2)/a0);					//* area of uneclipsed portion of Sun
				if (frac < 0.)
				{
					frac	=	0.0;
				}
			}
			frac	*=	8.0;	//* magnify the effect
			if (frac > 1.)
			{
				frac	=	1.0;
			}
			skycolor2	=	(int)(63. * sqrt(frac));	//* (sqrt magnifies it further)
		}

		//*************************************************************
		//* -.174 radians = -10 degrees is the point where sky is black
		//* .244 radians = 14 degrees is total range over which sky color changes
		//* i.e. sky starts to darken at sun elev = 4 degrees, black at -10 degrees

		skycolor1					=	(int)(63. * (sunmonptr->sun_elev + .174)/.244);
		if (skycolor1 < 0)
		{
			skycolor1=	0;
		}
		if (skycolor > 63)
		{
			skycolor	=	63;
		}

		//* use the darker of the two sky colors

		skycolor	=	((skycolor1>skycolor2) ? skycolor2:skycolor1);
	}

	cSkyRGBvalue.red	=	0;
	cSkyRGBvalue.grn	=	0;
	cSkyRGBvalue.blu	=	(skycolor * 4) << 8;


	SetWidgetBGColor(kSkyTravel_NightSky, CV_RGB(	cSkyRGBvalue.red,
													cSkyRGBvalue.grn,
													cSkyRGBvalue.blu));

	return(skycolor);
}



//*****************************************************************************
void	WindowTabSkyTravel::BuildConstellationData(void)
{
long	ii;
long	jj;
long	kk;
long	sCount;
long	vectorTabIdex;
long	totalStars;
long	starID;
long	ss;
long	cc;


	ii			=	0;
	while (gConstel_names[ii][0] != '-')
	{
		ii++;
	}
	constelationCount	=	ii;

	if (constelationCount > 0)
	{
		totalStars		=	0;
		constelations	=	(TYPE_Constelation *)malloc(constelationCount * sizeof(TYPE_Constelation));
		if (constelations != NULL)
		{
			for (ii=0; ii<constelationCount; ii++)
			{
				sCount			=	0;
				vectorTabIdex	=	gValid_name[ii] & 0xff;

				if (vectorTabIdex != 99)
				{
					//*	count the stars in the vector index
					jj	=	0;
					while (gConstellationVecor[vectorTabIdex][jj] != 0)
					{
						jj++;
					}
					sCount		=	jj;
					totalStars	+=	sCount;
				}

//				strcpy(constelations[ii].name,				gConstel_names[ii]);
				strcpy(constelations[ii].name,				gConstel_LongNames[ii]);

				strncpy(constelations[ii].shortName,		gConstel_names[ii], 3);
				constelations[ii].shortName[4]				=	0;
				constelations[ii].indexIntoConstStarTable	=	vectorTabIdex | 0x8000;
				constelations[ii].starsInConstelation		=	sCount;
			}

			if (cStarDataPtr != NULL)
			{
				//*	now allocate an array for the constellation stars and FIND them
				constStarPtr	=	(TYPE_CelestData *)malloc((totalStars + 5) * sizeof(TYPE_CelestData));
				if (constStarPtr != NULL)
				{
					constStarCount	=	totalStars;

					kk	=	0;
					for (ii=0; ii<kConstVectCount; ii++)
					{
						//*	find the constelation that points to this one
						for (cc=0; cc<constelationCount; cc++)
						{
							vectorTabIdex	=	constelations[cc].indexIntoConstStarTable;
							if (vectorTabIdex < 0)
							{
								if (ii == (vectorTabIdex & 0x7fff))
								{
									constelations[cc].indexIntoConstStarTable	=	kk;
									break;
								}
							}
						}


						for (jj=0; jj<kConstPtsPerVect; jj++)
						{
							if (gConstellationVecor[ii][jj] != 0)
							{
								if (kk < totalStars)
								{
									constStarPtr[kk].id	=	gConstellationVecor[ii][jj];
									//*	now find this id in the master list
									starID	=	gConstellationVecor[ii][jj] & 0x3fff;
									for (ss=0; ss<cStarCount; ss++)
									{
										if (starID == cStarDataPtr[ss].id)
										{
											//*	copy the entire star over
											constStarPtr[kk]	=	cStarDataPtr[ss];

											//*	but use the ID from the cector table
											constStarPtr[kk].id	=	gConstellationVecor[ii][jj];
											break;
										}
									}
									kk++;
								}
							}
						}
					}
				}
			}
		}
	}
}


//*****************************************************************************
void	WindowTabSkyTravel::DrawConstellationLines(void)
{
short			ii;
short			jj;
short			kk;
double			xangle,yangle,rangle;
double			temp,angle;
double			alpha,aside,cside,gamma;
double			sin_bside,cos_bside;
short			xPos, yPos;
short			firstStarInCurConstellation;
short			starsInCurConstellation;
unsigned int	idword;
bool			firstMove;

	if ((constelations != NULL) && (constStarPtr != NULL))
	{
		xangle		=	cView_angle / 2.0;
		yangle		=	xangle * cWind_height / cWind_width;
		rangle		=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
		cRadmax		=	rangle;

		sin_bside		=	sin(kHALFPI - cDecl0);
		cos_bside		=	cos(kHALFPI - cDecl0);

		rangle		*=	LFACT;

		cDecmax		=	cDecl0 + rangle;
		if (cDecmax > kHALFPI)
		{
			cDecmax	=	(kHALFPI - kEPSILON);	//* clip at 90 degrees
		}
		cDecmin		=	cDecl0 - rangle;
		if (cDecmin < -kHALFPI)
		{
			cDecmin	=	-(kHALFPI - kEPSILON);	//* clip at -90 degrees
		}
		cRamax	=	0.0;						//*default cRamax	=	0.
		temp	=	kHALFPI - (cDecl0);
		if (temp > rangle)
		{
		double	sinRatio;
			sinRatio=	sin(rangle)/sin(temp);
			if (sinRatio < 1.0)
			{
				cRamax	=	asin(sinRatio);
			}
		}
		cWind_x0	=	wind_ulx + (cWind_width / 2);
		cWind_y0	=	wind_uly + (cWind_height / 2);

		//*	compute pixels per radian
		cXfactor	=	cWind_width / cView_angle;
		cYfactor	=	cXfactor;	//* 1:1 aspect ratio

		SetColor(RED);
		for (jj=0; jj<constelationCount; jj++)
		{
			if ((constelations[jj].indexIntoConstStarTable >= 0) && (constelations[jj].starsInConstelation > 0))
			{
				firstStarInCurConstellation	=	constelations[jj].indexIntoConstStarTable;
				starsInCurConstellation		=	constelations[jj].starsInConstelation;

				firstMove					=	true;
				for (kk=0; kk<starsInCurConstellation; kk++)
				{
					ii		=	firstStarInCurConstellation + kk;
			  		alpha	=	cRa0 - constStarPtr[ii].ra;
			  		if (alpha > PI)
			  		{
			  			alpha	-=	kTWOPI;
			  		}
			  		else if (alpha<-PI)
			  		{
			  			alpha	+=	kTWOPI;
					}
			  		if ((cRamax == 0.0) || (fabs(alpha) <= cRamax))	//* in bounds for ra?
			  		{
			  			cside	=	kHALFPI - constStarPtr[ii].decl;
			  			aside	=	acos((cos_bside * cos(cside)) + (sin_bside * sin(cside) * cos(alpha)));
			  			if (aside < cRadmax)	//* within bounding circle?
			  			{
			  				if (aside>kEPSILON)
			  				{
			  					gamma	=	asin(sin(cside)*sin(alpha)/sin(aside));
			  				}
			  				else
			  				{
			  					gamma	=	0.0;
			  				}
			  				if (cos(cside)<(cos_bside*cos(aside)))
			  				{
			  					gamma	=	PI-gamma;	//* supplement gamma if cos(c)<cos(b)*cos(a)
							}
			  				angle	=	gamma + cGamang;

							//*compute x and y coordinates
							//* x	=	x0+cXfactor*aside*cos(angle)
							//* y	=	y0-cYfactor*aside*sin(angle) (minus sign is because plus y is down)

			  				xPos		=	cWind_x0 + (int)(cXfactor * aside*sin(angle));
			  				yPos		=	cWind_y0 - (int)(cYfactor * aside*cos(angle));
			  				idword		=	constStarPtr[ii].id;

							if (firstMove || ((idword & 0x4000) == 0x4000))
							{
								CMoveTo(xPos, yPos);
								firstMove	=	false;
							}
							else
							{
								CLineTo(xPos, yPos);
							}
						}
						else
						{
							firstMove	=	true;
						}
					}
				}
			}
		}
		SetColor(BLACK);
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::DrawConstellationOutLines(void)
{
int					iii;
int					jjj;
TYPE_ConstOutline	*myOutLineObj;
bool				ptInView;
short				pt_XX, pt_YY;
bool				drawFlag;
int					totalPts;			//*	number of points in the current outline
int					pointsDrawnCnt;		//*	these are so we can draw the name in the middle
int					minPixelX;
int					maxPixelX;
int					minPixelY;
int					maxPixelY;
int					nameLen;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cNightMode)
	{
		SetColor(DARKRED);
	}
	else
	{
		SetColor(DARKGREEN);
	}
	if ((cConstOutlinePtr != NULL) && (cConstOutlineCount > 0))
	{
		for (iii=0; iii<cConstOutlineCount; iii++)
		{
			//*	this makes it easier to follow the code
			myOutLineObj	=	&cConstOutlinePtr[iii];
			drawFlag		=	false;
			totalPts		=	0;
			pointsDrawnCnt	=	0;
			minPixelX		=	9999;
			maxPixelX		=	0;
			minPixelY		=	9999;
			maxPixelY		=	0;
			jjj				=	0;
			while ((myOutLineObj->path[jjj].rtAscension > 0.0) && (jjj < kMaxConstPointCnt))
			{
				totalPts++;
				ptInView		=	GetXYfromRA_Decl(	myOutLineObj->path[jjj].rtAscension,
														myOutLineObj->path[jjj].declination,
														&pt_XX,
														&pt_YY);
				//*	is the point on the screen
				if (ptInView)
				{
					pointsDrawnCnt++;
					if (drawFlag)
					{
						CLineTo(pt_XX, pt_YY);
					}
					else
					{
						CMoveTo(pt_XX, pt_YY);
						drawFlag		=	true;
					}

					//*	keep track of the min and max for label drawing
					if (pt_XX < minPixelX)
					{
						minPixelX	=	pt_XX;
					}
					if (pt_XX > maxPixelX)
					{
						maxPixelX	=	pt_XX;
					}
					if (pt_YY < minPixelY)
					{
						minPixelY	=	pt_YY;
					}
					if (pt_YY > maxPixelY)
					{
						maxPixelY	=	pt_YY;
					}
				}
				else
				{
					drawFlag		=	false;
				}
				jjj++;
			}
			//*	only draw the name if we drew at least 2/3 of the points
			if (pointsDrawnCnt >= ((2 * totalPts) / 3))
			{

				pt_XX	=	(minPixelX + maxPixelX) / 2;
				pt_YY	=	(minPixelY + maxPixelY) / 2;
				nameLen	=	strlen(myOutLineObj->longName);
				if (nameLen > 0)
				{
					DrawCString((pt_XX - (nameLen * 6)), pt_YY, myOutLineObj->longName);
				//	DrawCString(pt_XX, pt_YY+12, myOutLineObj->longName);
				}
				else
				{
					DrawCString(pt_XX, pt_YY, myOutLineObj->shortName);
				}
			}
			ptInView		=	GetXYfromRA_Decl(	myOutLineObj->rtAscension,
													myOutLineObj->declination,
													&pt_XX,
													&pt_YY);
			if (ptInView)
			{
				SetColor(PINK);
				DrawCString(pt_XX, pt_YY, myOutLineObj->shortName);
				SetColor(DARKGREEN);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("cConstOutlinePtr is null");
	}
}


//int	gLongestVectorSeg[]	=	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

//**************************************************************************************
//*	this is my new constellation vectors, far better than the original ones
//**************************************************************************************
void	WindowTabSkyTravel::DrawConstellationVectors(void)
{
int					iii;
int					jjj;
TYPE_ConstVector	*myConstPtr;	//*	to make the code easier to follow
bool				ptInView;
short				pt_XX, pt_YY;
short				prev_XX, prev_YY;
bool				moveFlag;
bool				offScreenFlg;
int					hippStarCnt;
int					ptsOnScreenCnt;
short				deltaPixels;
//int					longestDelta	=	0;
//	CONSOLE_DEBUG(__FUNCTION__);

	if (cConstVecotrPtr != NULL)
	{
		SetColor(PINK);
		ptsOnScreenCnt	=	0;
//		CONSOLE_DEBUG_W_NUM("cConstVectorCnt\t=",cConstVectorCnt);
		for (iii=0; iii<cConstVectorCnt; iii++)
		{

			myConstPtr		=	&cConstVecotrPtr[iii];

		//	if (strncasecmp(myConstPtr->longName, "AND", 3) == 0)
		//	{
		//		SetColor(CYAN);
		//	}
		//	else
		//	{
		//		SetColor(PINK);
		//	}

			offScreenFlg	=	true;
			hippStarCnt		=	myConstPtr->starCount;
			prev_XX			=	-1;
			prev_YY			=	-1;
//			CONSOLE_DEBUG_W_NUM("hippStarCnt\t=",hippStarCnt);
			for (jjj=0; jjj<hippStarCnt; jjj++)
			{
				moveFlag		=	myConstPtr->hippStars[jjj].moveFlag;
				ptInView		=	GetXYfromRA_Decl(	myConstPtr->hippStars[jjj].rtAscension,
														myConstPtr->hippStars[jjj].declination,
														&pt_XX,
														&pt_YY);
				if (ptInView)
				{
					if ((cView_index > 8) && (prev_XX >= 0))
					{
						//*	check for distorted line
						deltaPixels	=	abs(pt_XX - prev_XX) + abs(pt_YY - prev_YY);
						if (deltaPixels > (cWidth / 5))
						{
							//*	force a move
							moveFlag	=	true;
						}
					//	if (deltaPixels > longestDelta)
					//	{
					//		longestDelta	=	deltaPixels;
					//	}
					}
					if (moveFlag || offScreenFlg)
					{
						CMoveTo(pt_XX, pt_YY);
					}
					else
					{
						CLineTo(pt_XX, pt_YY);
					}

					prev_XX	=	pt_XX;
					prev_YY	=	pt_YY;

					offScreenFlg	=	false;
					ptsOnScreenCnt++;
				}
				else
				{
					offScreenFlg	=	true;
				}
			}
		}
//		CONSOLE_DEBUG_W_NUM("ptsOnScreenCnt\t=",ptsOnScreenCnt);

	//	CONSOLE_DEBUG("-------------------------------");
	//	if (longestDelta > gLongestVectorSeg[cView_index])
	//	{
	//		gLongestVectorSeg[cView_index]	=	longestDelta;
	//	}
	//	for (iii=0; iii<kMaxViewAngleIndex; iii++)
	//	{
	//		CONSOLE_DEBUG_W_NUM("gLongestVectorSeg\t=", gLongestVectorSeg[iii]);
	//	}
	}
	else
	{
		CONSOLE_DEBUG("cConstVecotrPtr is null");
	}
}


//**************************************************************************************
void	WindowTabSkyTravel::DrawCommonStarNames(void)
{
int		iii;
short	pt_XX, pt_YY;
bool	ptInView;

//	CONSOLE_DEBUG("-------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);

	if ((cHipObjectPtr != NULL) && (cHipObjectCount > 0))
	{
		SetColor(WHITE);
		for (iii = 0; iii < cHipObjectCount; iii++)
		{
			//*	most of them don't have names, so check for that first
			if (cHipObjectPtr[iii].longName[0] != 0)
			{
				ptInView		=	GetXYfromRA_Decl(	cHipObjectPtr[iii].ra,
														cHipObjectPtr[iii].decl,
														&pt_XX,
														&pt_YY);
				if (ptInView)
				{
					DrawStar_shape(pt_XX, pt_YY, 0);	//*	many of these stars are not drawn
//					CONSOLE_DEBUG(cHipObjectPtr[iii].longName);
					if (cDispOptions.dispHIP && (cView_index <= 3))
					{
						pt_YY	+=	12;
					}
					DrawCString(pt_XX + 10, pt_YY, cHipObjectPtr[iii].longName);
				}
			}
		}
	}
}

//**************************************************************************************
void	WindowTabSkyTravel::DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx)
{
CvRect		myCVrect;
CvScalar	myBackGroundColor;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	cWidgetList[widgetIdx].left;
	myCVrect.y		=	cWidgetList[widgetIdx].top;
	myCVrect.width	=	cWidgetList[widgetIdx].width;
	myCVrect.height	=	cWidgetList[widgetIdx].height;

	cOpenCV_Image	=	openCV_Image;
	cCursorOffsetY	=	cWidgetList[widgetIdx].top;

	switch(widgetIdx)
	{
		case kSkyTravel_NightSky:
//			CONSOLE_DEBUG_W_NUM("myCVrect.x\t=",	myCVrect.x);
//			CONSOLE_DEBUG_W_NUM("myCVrect.y\t=",	myCVrect.y);
//			CONSOLE_DEBUG_W_NUM("myCVrect.width\t=",	myCVrect.width);
//			CONSOLE_DEBUG_W_NUM("myCVrect.height\t=",	myCVrect.height);
			if (cOpenCV_Image != NULL)
			{
				if (0)
				{
					myBackGroundColor	=	CV_RGB(	0,0,0);
				}
				else
				{
					myBackGroundColor	=	CV_RGB(	cSkyRGBvalue.red,
													cSkyRGBvalue.grn,
													cSkyRGBvalue.blu);
				}
				cvRectangleR(	cOpenCV_Image,
								myCVrect,
								myBackGroundColor,		//	CvScalar color,
								CV_FILLED,				//	int thickness CV_DEFAULT(1),
								8,						//	int line_type CV_DEFAULT(8),
								0);						//	int shift CV_DEFAULT(0));

				cvSetImageROI(cOpenCV_Image,  myCVrect);
				DrawSkyAll();
				DrawWindowOverlays();
				cvResetImageROI(cOpenCV_Image);

                //*	this has to be done AFTER roi is reset
				Compute_cursor(&cCurrentTime, &cCurrLatLon);
				DrawCursorLocationInfo();

			}
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			CONSOLE_DEBUG("We shouldnt be here");
//			CONSOLE_ABORT("We shouldnt be here");
			break;
	}
}




//*****************************************************************************
static void	SysBeep(int x)
{
}

//***********************************************************************
//*	QSortProc proc for sorting files
static  int CelestObjDeclinationQsortProc(const void *e1, const void *e2)
{
TYPE_CelestData	*obj1, *obj2;
int				returnValue;


	obj1		=	(TYPE_CelestData *)e1;
	obj2		=	(TYPE_CelestData *)e2;

	//*	we are sorting by declination value
	if (obj1->decl < obj2->decl)
	{
		returnValue	=	1;
	}
	else if (obj1->decl > obj2->decl)
	{
		returnValue	=	-1;
	}
	else
	{
		returnValue	=	0;
	}

	return(returnValue);
}


//********************************************************************
//* precess if necessary, re-load objall if necessary
//* i.e. precession math always starts with the year 2000 data
//*	returns TRUE if precess occurred
//********************************************************************
bool	WindowTabSkyTravel::Precess(TYPE_CelestData	*celestObjPtr,
									long			celestObjCount,
									bool			sortFlag,
									bool			forcePrecession)
{
long			ii;
double			ipart;
double			epoch,zee,pzeta,ptheta;
double			alpha,aside,cside,gamma;
double			sin_bside,cos_bside;
long			startIndex, stopIndex;
bool			pressesOccurred;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_LONG("celestObjCount\t=",	celestObjCount);

	pressesOccurred	=	false;
	if ((celestObjPtr != NULL) || (celestObjCount < 2))
	{
		//*	if we arent being forced to precess, then check to see if we need it
		if (!cCurrentTime.precflag && !forcePrecession)
		{
			if ((fabs(cCurrentTime.fJulianDay - cCurrentTime.timeOfLastPrec)) < cCurrentTime.delprc)
			{
				CONSOLE_DEBUG("precess not necessary");
				return(false);	//*	precess not necessary
			}


			if ((fabs(cCurrentTime.fJulianDay - JD2000)) < cCurrentTime.delprc)
			{
				CONSOLE_DEBUG("precess not necessary");
				return(false);	//* return if precession not needed
			}
		}

//		DisplayHelpMessage("Precessing");

		cCurrentTime.precflag			=	false;			//*	force off
		cCurrentTime.strflag			=	false;			//*	say year 2000 data not present
		cCurrentTime.starDataModified	=	true;			//*	say year 2000 data not present
		cCurrentTime.timeOfLastPrec		=	cCurrentTime.fJulianDay;	//*	set new lst prc

		//* precession math starts
		epoch		=	cCurrentTime.cent - 1.0;			//*!!!68000 source code says use 1-cent !!!

		//* PZETA	=	T*(A+T*(B+C*T)) WHERE T=EPOCH
		pzeta		=	epoch*(PRECA + (epoch*((PRECB + (PRECC*epoch)))));

		//* Z=PZETA+T*T*PRECZ
		zee			=	pzeta + (epoch*epoch*PRECZ);

		//* PTHETA=T*(D+T*(E+F*T))
		ptheta		=	epoch*(PRECD + (epoch*(PRECE + (PRECF*epoch))));

		//* convert to radians
		ptheta		=	kTWOPI * modf(ptheta,	&ipart);
		zee			=	kTWOPI * modf(zee,		&ipart);
		pzeta		=	kTWOPI * modf(pzeta,		&ipart);

		//* for fast sphsas
		cos_bside	=	cos(ptheta);
		sin_bside	=	sin(ptheta);

//		CONSOLE_DEBUG_W_NUM("celestObjCount\t=",	celestObjCount);
		if ((celestObjCount == 3885) && (sortFlag == true))
		{
			//* precess all objects except N and S poles
			startIndex	=	1;
			stopIndex	=	celestObjCount-1;
		}
		else
		{
			startIndex	=	0;
			stopIndex	=	celestObjCount;
		}
		for (ii=startIndex; ii < stopIndex; ii++)
		{
			cside	=	kHALFPI - celestObjPtr[ii].org_decl;
			alpha	=	pzeta + celestObjPtr[ii].org_ra;

			//* here we use inline code for sphsas and sphsss because bside is constant
			//* so we avoid repeated invocations of sin(bside) and cos(bside)

			aside	=	acos((cos_bside*cos(cside)) + (sin_bside*sin(cside)*cos(alpha)));

			celestObjPtr[ii].decl	=	kHALFPI - aside;

			//* sphsss

			if (aside > kEPSILON)
			{
				gamma	=	asin(sin(cside)*sin(alpha)/sin(aside));
			}
			else
			{
				gamma	=	0.0;
			}
			if (cos(cside)<(cos_bside*cos(aside)))
			{
				gamma	=	PI - gamma;		//* supplement gamma if cos(c)<cos(b)*cos(a)
			}
			if (epoch > 0.)
			{
				gamma	=	PI - gamma;				//* supplement gamma when epoch > 0
			}
			celestObjPtr[ii].ra	=	zee + gamma;
		}
		pressesOccurred	=	true;
		if (sortFlag)
		{
//			DisplayHelpMessage("Sorting (qsort)");
//			CONSOLE_DEBUG("Sorting (qsort)");
//			CONSOLE_DEBUG_W_NUM("sizeof(obj)  \t=",	sizeof(TYPE_CelestData));
			qsort(celestObjPtr, celestObjCount, sizeof(TYPE_CelestData), CelestObjDeclinationQsortProc);
		}
	}
	else
	{
		CONSOLE_DEBUG("celestObjPtr is NULL");
	}
	return(pressesOccurred);
}

#define	kForcePression		true
#define	kPressionIfNeeded	false
#define	kSortData			true
#define	kDoNotSort			false

//********************************************************************
bool	WindowTabSkyTravel::Precess(void)
{
bool			pressesOccurred;
//unsigned long	startTicks, endTicks, elapsedTicks;
//char			ticksMsg[64];

	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

//	startTicks		=	TickCount();
	pressesOccurred	=	Precess(cStarDataPtr, cStarCount, kSortData, kPressionIfNeeded);

	//*	if precess occurred for the stars, we want to force it for the constellations
	if (pressesOccurred)
	{
//		CONSOLE_DEBUG("precess occurred");

		//*	NGC objects
		if ((cNGCobjectPtr != NULL) && (cNGCobjectCount > 0))
		{
			Precess(cNGCobjectPtr, cNGCobjectCount, kSortData, kForcePression);
		}

		//*	Yale start catalog
		if ((cYaleStarDataPtr != NULL) && (cYaleStarCount > 0))
		{
			Precess(cYaleStarDataPtr, cYaleStarCount, kSortData, kForcePression);
		}

		//*	Hippacos database
		if ((cHipObjectPtr != NULL) && (cHipObjectCount > 0))
		{
			Precess(cHipObjectPtr, cHipObjectCount, kSortData, kForcePression);
		}

		//*	Messier objects
		if ((cMessierOjbectPtr != NULL) && (cMessierOjbectCount > 0))
		{
			Precess(cMessierOjbectPtr, cMessierOjbectCount, kSortData, kForcePression);
		}

		//*	Henry Draper catalog
		if ((cDraperObjectPtr != NULL) && (cDraperObjectCount > 0))
		{
			Precess(cDraperObjectPtr, cDraperObjectCount, kSortData, kForcePression);
		}


#ifdef _ENABLE_HYG_
		if ((cHYGObjectPtr != NULL) && (cHYGObjectCount > 0))
		{
			Precess(cHYGObjectPtr, cHYGObjectCount, kSortData, kForcePression);
		}
#endif

		if ((cSpecialObjectPtr != NULL) && (cSpecialObjectCount > 0))
		{
			Precess(cSpecialObjectPtr, cSpecialObjectCount, kSortData, kForcePression);
		}


		if ((constStarPtr != NULL) && (constStarCount > 0))
		{
			Precess(constStarPtr, constStarCount, kDoNotSort, kForcePression);
		}




//		endTicks		=	TickCount();
//		elapsedTicks	=	endTicks - startTicks;

//		sprintf(ticksMsg, "Ticks = %ld", elapsedTicks);
//		DisplayHelpMessage(ticksMsg);
	}
	else
	{
		CONSOLE_DEBUG("DID NOT precess");
	}

//	SetCursorArrow();
	return(pressesOccurred);
}

//*****************************************************************************
void	WindowTabSkyTravel::DrawObjectByShape(int xcoord, int ycoord, int shape, int magn)
{
short		constelNameIdx;
int			scale;
char		symb[16];

	switch(shape)
	{
		case ST_NAME:
			constelNameIdx	=	magn - 0x0080;
			if ((constelNameIdx >= 0) && (constelNameIdx < kMaxConstelNames))
			{
				SetColor(RED);
			//	strcpy(symb, gConstel_names[constelNameIdx]);
			//	DrawCString(xcoord, ycoord, symb);
				DrawCString(xcoord, ycoord, gConstel_LongNames[constelNameIdx]);
			}
			else
			{
				SysBeep(1);
			}
			break;

		case ST_STAR:
			if (cNightMode)
			{
				SetColor(RED);
			}
		#ifdef _ENBABLE_WHITE_CHART_
			else if (cChart)
			{
				SetColor(BLACK);
			}
		#endif
			else
			{
				SetColor(WHITE);
			}
			DrawStar_shape(xcoord, ycoord, magn & 0x07);
			break;

		case ST_DEEP:
			{
				scale	=	1 + ((7 - cView_index) / 2);
				switch(magn & 0xf0)	//*	isolate hi 4 bits
				{
					case 0x10:	//*	globular
						DrawVector(LIGHTGRAY, xcoord, ycoord, scale, globular_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x20:	//*	elliptical
						DrawVector(BLUE, xcoord, ycoord, scale, elliptical_shapes[magn & 0x0f]);		//*	draw the object
						break;

					case 0x30:	//*	spiral
						DrawVector(GREEN, xcoord, ycoord, scale, spiral_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x40:	//*	planetary
						DrawVector(CYAN, xcoord, ycoord, scale, planetary_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x50:	//	*bright diffuse
						DrawVector(RED, xcoord, ycoord, scale, bdn_shapes[magn & 0x0f]);			//*	draw the object
						break;

					case 0x60:	//*	open galactic
						DrawVector(MAGENTA, xcoord, ycoord, scale, ogc_shapes[magn & 0x0f]);		//*	draw the object
						break;

					case 0x70:	//*	smc/lmc outline
						SetColor(GREEN);
						if (magn == 0x0070)
						{
							//*	Small Magellanic Cloud
							strcpy(symb,"smc");
						}
						else if (magn == 0x0072)
						{
							//*	Large Magellanic Cloud
							strcpy(symb,"lmc");
						}
						else if (magn < 0x73)
						{
							strcpy(symb, ".");
							SetColor(LIGHTGRAY);
						}
						else
						{
							CONSOLE_ABORT("Magellanic Cloud");
						}
						DrawCString(xcoord, ycoord, symb);
						break;
				}
			}
			break;

		case ST_ALWAYS:
			CONSOLE_ABORT("ST_ALWAYS");
			break;
	}

}

//*****************************************************************************
long	WindowTabSkyTravel::Search_and_plot(TYPE_CelestData	*objectptr,
											long			maxObjects)
{
bool				goflag;
int			 		ii;
int					magn;
int					shape;
int					xcoord;
int					ycoord;
double				temp,angle;
double				alpha,aside,cside,gamma;
double				sin_bside,cos_bside;
double				xangle,yangle,rangle;
unsigned int		myCount;
short				dataSource;
char				labelString[32];
bool				printLabel;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("cDecl0\t\t=",		cDecl0);
#ifdef _ENBABLE_WHITE_CHART_
	if (cChart)
	{
		SetColor(BLACK);
	}
	else
#endif // _ENBABLE_WHITE_CHART_
	{
		SetColor(WHITE);
	}

	myCount			=	0;
	xangle			=	cView_angle / 2.0;
	yangle			=	xangle * cWind_height / cWind_width;
	rangle			=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
	cRadmax			=	rangle;
	dataSource		=	objectptr[0].dataSrc;

	sin_bside		=	sin(kHALFPI - cDecl0);
	cos_bside		=	cos(kHALFPI - cDecl0);

	cDecmax			=	cDecl0 + rangle;
	if (cDecmax > kHALFPI)
	{
		cDecmax		=	(kHALFPI - kEPSILON);	//* clip at 90 degrees
	}
	cDecmin			=	cDecl0 - rangle;
	if (cDecmin < -kHALFPI)
	{
		cDecmin		=	-(kHALFPI - kEPSILON);	//* clip at -90 degrees
	}

	cRamax	=	0.0;		//*	default cRamax = 0.
	temp	=	kHALFPI - fabs(cDecl0);
	if (temp > cRadmax)
	{
		cRamax	=	asin(sin(cRadmax) / sin(temp));
	}
	cWind_x0	=	wind_ulx + (cWind_width / 2);
	cWind_y0	=	wind_uly + (cWind_height / 2);

	cXfactor	=	cWind_width / cView_angle;
	cYfactor	=	cXfactor;	//* 1:1 aspect ratio

	//*	the database is sorted by declination, from high value (north pole) to low
	//*	value (south pole)
	//*	skip quickly until we find the first star within our current display

	//* continue scan until decl<decmin
	ii		=	0;
	while ((objectptr[ii].decl > cDecmax) && (ii < maxObjects))	//* skip down to where decl < cDecmax
	{
		objectptr[ii].curXX	=	-100;
		objectptr[ii].curYY	=	-100;
		ii++;
	}
	//*	debuging code
	if (ii >= maxObjects)
	{
		SysBeep(1);
	}

//	CONSOLE_DEBUG_W_LONG("maxObjects\t=", maxObjects);

	while (ii < maxObjects)
	{
	//	CONSOLE_DEBUG_W_NUM("ii\t=", ii);
		objectptr[ii].curXX	=	-100;
		objectptr[ii].curYY	=	-100;

		magn	=	objectptr[ii].magn & 0x00ff;

		if (magn < 0x0010)
		{
			shape	=	ST_STAR;
		}
		else if (magn < 0x0073)
		{
			shape	=	ST_DEEP;
		}
		else if (magn < 0x00db)
		{
			shape	=	ST_NAME;
		}
		else
		{
			shape	=	ST_ALWAYS;
		}

		goflag	=	true;
		if (dataSource == kDataSrc_Orginal)
		{
			switch(shape)
			{
				case ST_NAME: if (!cDispOptions.dispNames)	goflag	=	false; break;
				case ST_DEEP: if (!cDispOptions.dispDeep)	goflag	=	false; break;
	//?			case ST_STAR: if (magn < cMagmin)			goflag	=	false; break;
			}
		}

		//*	check for displayed magnitude limits
		if (dataSource == kDataSrc_Hipparcos)
		{
			if (objectptr[ii].realMagnitude > cDisplayedMagnitudeLimit)
			{
				goflag	=	false;
			}
		}

		if (goflag)	//* try to plot it
		{
//			CONSOLE_DEBUG(__FUNCTION__);
			alpha	=	cRa0 - objectptr[ii].ra;
			if (alpha > PI)
			{
				alpha	-=	kTWOPI;
			}
			else if (alpha < -PI)
			{
				alpha	+=	kTWOPI;
			}
			if ((cRamax == 0.0) || (fabs(alpha) <= cRamax))		//* in bounds for ra?
			{
				cside	=	kHALFPI - objectptr[ii].decl;

				//* here we use inline code for sphsas and sphsss because bside is constant
				//* so we avoid repeated invocations of sin(bside) and cos(bside)

				aside	=	acos((cos_bside * cos(cside)) + (sin_bside * sin(cside) * cos(alpha)));
				if (aside < cRadmax)	//* within bounding circle?
				{
					if (aside > kEPSILON)
					{
						gamma	=	asin(sin(cside) * sin(alpha) / sin(aside));
					}
					else
					{
						gamma	=	0.0;
					}
					if (cos(cside) < (cos_bside * cos(aside)))
					{
						gamma	=	PI-gamma;	//* supplement gamma if cos(c)<cos(b)*cos(a)
					}
					angle	=	gamma + cGamang;

					//*compute x and y coordinates
					//* x	=	x0 + cXfactor * aside * cos(angle)
					//* y	=	y0 - cYfactor * aside * sin(angle) (minus sign is because plus y is down)

					xcoord	=	cWind_x0 + (cXfactor * aside * sin(angle));

					//* are they both within window?

					if ((xcoord >= wind_ulx) && (xcoord <= wind_ulx + cWind_width))
					{
						objectptr[ii].curXX	=	xcoord;
						ycoord				=	cWind_y0 - (cYfactor * aside * cos(angle));
						if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
						{
							objectptr[ii].curYY	=	ycoord;

							DrawObjectByShape(xcoord, ycoord, shape, magn);

							switch(dataSource)
							{
							//*	draw Messier M numbers
								case kDataSrc_Messier:
									if (cNightMode)
									{
										SetColor(RED);
									}
									else
									{
										SetColor(CYAN);
									}
									DrawCString(xcoord, ycoord, objectptr[ii].shortName);
									if (cView_index < 4)
									{
										DrawCString(xcoord + 45, ycoord, objectptr[ii].longName);
									}
									break;

								case kDataSrc_Hipparcos:
									//*	draw common H numbers and names if present
									if (cDispOptions.dispNames && (cView_index <= 3))
									{
										sprintf(labelString, "H%ld", objectptr[ii].id);

										if (objectptr[ii].longName[0] > 0x20)
										{
											sprintf(labelString, "H%ld-%s", objectptr[ii].id, objectptr[ii].longName);
										}
										SetColor(RED);
										DrawCString(xcoord + 10, ycoord, labelString);
									}
									break;

								case kDataSrc_NGC2000:
								case kDataSrc_NGC2000IC:
									if (cDispOptions.dispNames && (cView_index <= 4))
									{
										if (objectptr[ii].dataSrc == kDataSrc_NGC2000IC)
										{
											sprintf(labelString, "IC%ld", objectptr[ii].id);
											SetColor(CYAN);
										}
										else
										{
											sprintf(labelString, "NGC%ld", objectptr[ii].id);
											SetColor(YELLOW);
										}

										if (objectptr[ii].longName[0] > 0x20)
										{
											strcat(labelString, "-");
											strcat(labelString, objectptr[ii].longName);
										}
										DrawCString(xcoord + 10, ycoord, labelString);
									}
									break;

								case kDataSrc_HYG:
									if (cDispOptions.dispNames && (cView_index <= 3))
									{
										printLabel	=	false;
										if ((objectptr[ii].id > 0) && (objectptr[ii].longName[0] > 0x20))
										{
											sprintf(labelString, "HD%ld-%s", objectptr[ii].id, objectptr[ii].longName);
											printLabel	=	true;
										}
										else if (objectptr[ii].id > 0)
										{
											sprintf(labelString, "HD%ld", objectptr[ii].id);
											printLabel	=	true;
										}
										else if (objectptr[ii].longName[0] > 0x20)
										{
											sprintf(labelString, "%s", objectptr[ii].longName);
											printLabel	=	true;
										}
										if (printLabel)
										{
											SetColor(RED);
											DrawCString(xcoord + 10, ycoord + 12, labelString);
										//	CONSOLE_DEBUG_W_NUM("ii         \t=",	ii)
										//	CONSOLE_DEBUG_W_NUM("id         \t=",	objectptr[ii].id)
										//	CONSOLE_DEBUG_W_STR("longName   \t=",	objectptr[ii].longName)
										//	CONSOLE_DEBUG_W_STR("labelString\t=",	labelString)
										}
									}
									break;

								case kDataSrc_Draper:
									if (cDispOptions.dispNames && (cView_index <= 2))
									{
										SetColor(GREEN);
										DrawCString(xcoord + 10, ycoord, objectptr[ii].longName);
									}
									break;

								case kDataSrc_Special:
									DrawCString(xcoord + 10, ycoord, objectptr[ii].longName);
									break;
							}
							myCount++;
						}
					}
				}
			}
		}
		ii++;

		if (objectptr[ii].decl < cDecmin)
		{
			break;
		}
	}

	//*	make sure any remaining stars are set back to not on screen
	while (ii < maxObjects)
	{
		objectptr[ii].curXX	=	-100;
		objectptr[ii].curYY	=	-100;
		ii++;
	}

//	CONSOLE_DEBUG_W_NUM("myCount\t\t=",		myCount);
	return(myCount);
}


//*****************************************************************************
//* draw an object described by vectors at position x,y and color
void	WindowTabSkyTravel::DrawVector(	short		color,
										short		xx,
										short		yy,
										short		scale,
										const char	*shape_data)
{
unsigned char	movedata;
unsigned int	nmoves;
unsigned int	movetype;
unsigned int	nn;
short			myScale;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("scale\t=", scale);

	if (cNightMode)
	{
		SetColor(RED);
	}
	else
	{
		SetColor(color);
	}

	myScale	=	scale;
	if (myScale < 1)
	{
		myScale	=	1;
	}
	CMoveTo(xx, yy);

	nn	=	0;
	while((movedata = shape_data[nn++]) != 0)	//* loop until a zero byte is reached
	{
		nmoves		=	(movedata & 0x0f) * myScale;
		movetype	=	((movedata) & 0x00f0) >> 4;	//* extract move type
		switch(movetype & 0x0f)
		{
			case 0x00:
			case 0x01:
				xx	+=	nmoves;
				break;

			case 0x02:
			case 0x03:
				xx	+=	nmoves;
				yy	-=	nmoves;
				break;

			case 0x04:
			case 0x05:
				yy	-=	nmoves;
				break;

			case 0x06:
			case 0x07:
				xx	-=	nmoves;
				yy	-=	nmoves;
				break;

			case 0x08:
			case 0x09:
				xx	-=	nmoves;
				break;

			case 0x0a:
			case 0x0b:
				xx	-=	nmoves;
				yy	+=	nmoves;
				break;

			case 0x0c:
			case 0x0d:
				yy	+=	nmoves;
				break;

			case 0x0e:
			case 0x0f:
				xx	+=	nmoves;
				yy	+=	nmoves;
				break;
		}
		if (movetype & 0x01)
		{
			CMoveTo(xx, yy);
		}
		else
		{
			CLineTo(xx, yy);
		}
	}
}



//*****************************************************************************
//*draw a star-like shape at xcoord, ycoord based on index(=6 brightest)
void WindowTabSkyTravel::DrawStar_shape(short xcoord, short ycoord, short index)
{
int 	ii;
int		color1;
int		color2;
//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENBABLE_WHITE_CHART_
	if (cChart)
	{
		color1	=	BLACK;
		color2	=	BLACK;
	}
	else
#endif // _ENBABLE_WHITE_CHART_
	{
		color1	=	WHITE;
		color2	=	LIGHTGRAY;
	}

	switch(index)
	{
		case 0:
	//		Putpixel(xcoord,	ycoord,		color2);
			Putpixel(xcoord,	ycoord,		color1);
			break;

		case 1:
			Putpixel(xcoord,	ycoord,		color1);
			break;

		case 2:
			Putpixel(xcoord,	ycoord,		color1);
			Putpixel(xcoord +1,	ycoord,		color2);
			Putpixel(xcoord-1,	ycoord,		color2);
			Putpixel(xcoord,	ycoord+1,	color2);
			Putpixel(xcoord,	ycoord-1,	color2);
			break;

		case 3:
			Putpixel(xcoord+1,	ycoord,		color1);
			Putpixel(xcoord-1,	ycoord,		color1);
			Putpixel(xcoord,	ycoord,		color1);
			Putpixel(xcoord,	ycoord+1,	color1);
			Putpixel(xcoord,	ycoord-1,	color1);
			break;

		case 4:
			Putpixel(xcoord-2,	ycoord,		color2);

			Putpixel(xcoord-1,	ycoord,		color1);
			Putpixel(xcoord,	ycoord,		color1);
			Putpixel(xcoord+1,	ycoord,		color1);

			Putpixel(xcoord+2,	ycoord,		color1);

			Putpixel(xcoord-1,	ycoord+1,	color1);
			Putpixel(xcoord,	ycoord+1,	color1);
			Putpixel(xcoord+1,	ycoord+1,	color1);
			Putpixel(xcoord-1,	ycoord-1,	color1);
			Putpixel(xcoord,	ycoord-1,	color1);
			Putpixel(xcoord+1,	ycoord-1,	color1);

			Putpixel(xcoord,	ycoord-2,	color2);
			break;

		case 5:
			for (ii = -1; ii < 2; ii++)
			{
				Putpixel(xcoord-2,	ycoord+ii,	color1);
				Putpixel(xcoord-1,	ycoord+ii,	color1);
				Putpixel(xcoord,	ycoord+ii,	color1);
				Putpixel(xcoord+1,	ycoord+ii,	color1);
				Putpixel(xcoord+2,	ycoord+ii,	color1);
			}

			Putpixel(xcoord-1,	ycoord+2,	color1);
			Putpixel(xcoord,	ycoord+2,	color1);
			Putpixel(xcoord+1,	ycoord+2,	color1);

			Putpixel(xcoord-1,	ycoord-2,	color1);
			Putpixel(xcoord,	ycoord-2,	color1);
			Putpixel(xcoord+1,	ycoord-2,	color1);

			break;

		case 6:
			SetColor(color1);
			DrawCString(xcoord,	ycoord,	"*");
			break;
	}
}

//*********************************************************************
void	WindowTabSkyTravel::DrawWindowOverlays(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	not sure what state the color is in, clear it
	currentForeColor	=	-1;


	if (cDispOptions.ecliptic_line)
	{
	//	if ((fabs(cDecl0) < (cView_angle + ECLIPTIC_ANGLE)))
		{
			DrawEcliptic();
		}
	}

	if (!cChart && (cDispOptions.dispEarth || cDispOptions.dispHorizon_line))	//* horizon line or earth
	{
		if (fabs(cElev0) < (cView_angle / 2.0))			//* is horizon in field?
		{
			DrawHorizon();								//* plot the horizon
#ifdef _DISPLAY_MAP_TOKENS_
			MapTokens(&cCurrentTime, &cCurrLatLon);		//* look for map tokens
#endif // _DISPLAY_MAP_TOKENS_
		}
		else if (cDispOptions.dispEarth && (cElev0 < 0.0))
		{
//			FillWithEarth();	//* fill whole screen
		}
	}



	if (cDispOptions.dispTelescope)
	{
	bool	telescopeIsInView;
	short	telescopeXX, telescopeYY;

		//*	this is for testing
//		DrawGreatCircle(gTelescopeDecl_Radians, true);

		telescopeIsInView	=	GetXYfromRA_Decl(	gTelescopeRA_Radians,
													gTelescopeDecl_Radians,
													&telescopeXX,
													&telescopeYY);

#if 0
		//*	are we keeping the display centered on the telescope position
		if (cTrackTelescope)
		{
		CvRect	centerRect;
//		Point	telescopePt;

			SetRect(&centerRect, 0, 0, cWind_width, cWind_height);
			InsetRect(&centerRect, (cWind_width / 3), (cWind_height / 3));
			while (centerRect.width < 50)
			{
				InsetRect(&centerRect, -5, 0);
			}

			while (centerRect.height < 50)
			{
				InsetRect(&centerRect, 0, -5);
			}
			telescopePt.h	=	telescopeXX;
			telescopePt.v	=	telescopeYY;
			if (!PtInRect(telescopePt, &centerRect))
			{
				//*	force an update
				cRa0		=	gTelescopeRA_Radians;
				cDecl0		=	gTelescopeDecl_Radians;
				cFindFlag	=	true;
				ReDrawWorkSpace();
				InvalWindowRect();
			}
		}
#endif

		//*	do we display the telescope cross hairs
		if (telescopeIsInView)
		{
		int	fovCount;

			telescopeXX	+=	cWorkSpaceTopOffset;
			telescopeYY	+=	cWorkSpaceLeftOffset;

			fovCount	=	DrawTelescopeFOV();
			if (fovCount == 0)
			{
				DrawTelescopeReticle(telescopeXX, telescopeYY);
			}

		}
		else
		{
		//	CONSOLE_DEBUG("-------------------------");
		//	CONSOLE_DEBUG_W_STR("gTelescopeRA_String   \t=",	gTelescopeRA_String);
		//	CONSOLE_DEBUG_W_DBL("gTelescopeRA_Radians  \t=",	DEGREES(gTelescopeRA_Radians / 15.0));
		//	CONSOLE_DEBUG_W_STR("gTelescopeDecl_String \t=",	gTelescopeDecl_String);
		//	CONSOLE_DEBUG_W_DBL("gTelescopeDecl_Radians\t=",	DEGREES(gTelescopeDecl_Radians));
		}
	}

	if (cDispOptions.dispDomeSlit)
	{
		DrawDomeSlit();
	}

	if (cDispOptions.dispGrid)
	{
		DrawGrid(cCurrentSkyColor);
	}

	//*	draw the equator AFTER the grid
	if (cDispOptions.dispEquator_line)	//* equator line
	{
		if (fabs(cDecl0) < cView_angle)
		{
			SetColor(MAGENTA);
			DrawGreatCircle(0.0);
		}
	}
	if (cDispOptions.dispEarth && ((cElev0 - (-kHALFPI)) < cRadmax))
	{
		//*	we are looking down at our feet, draw them
		DrawFeet();		//* -90 degrees
	}

	DrawCompass();

//	DrawDisplayInfo();

}

//*****************************************************************************
void	WindowTabSkyTravel::DrawTelescopeReticle(int screenXX, int screenYY)
{
#define	kFinderRadius	40
#define	kTlescopeRadius	15

	//*	draw the finder scope
	SetColor(BLUE);
	if (cTelescopeDisplayOptions.dispFindScopeOutline)
	{
		FrameEllipse(screenXX, screenYY, kFinderRadius, kFinderRadius);
	}

	if (cTelescopeDisplayOptions.dispFindScopeCrossHairs)
	{
		CMoveTo(screenXX - kFinderRadius, screenYY - kFinderRadius);
		CLineTo(screenXX + kFinderRadius, screenYY + kFinderRadius);

		CMoveTo(screenXX + kFinderRadius, screenYY - kFinderRadius);
		CLineTo(screenXX - kFinderRadius, screenYY + kFinderRadius);
	}

	//*	now draw the actual telescope

	SetColor(GREEN);

	if (cTelescopeDisplayOptions.dispTeleScopeOutline)
	{
		FrameEllipse(screenXX, screenYY, kTlescopeRadius, kTlescopeRadius);
	}

	if (cTelescopeDisplayOptions.dispTeleScopeCrossHairs)
	{
		CMoveTo(screenXX - kTlescopeRadius, screenYY - kTlescopeRadius);
		CLineTo(screenXX + kTlescopeRadius, screenYY + kTlescopeRadius);

		CMoveTo(screenXX + kTlescopeRadius, screenYY - kTlescopeRadius);
		CLineTo(screenXX - kTlescopeRadius, screenYY + kTlescopeRadius);
	}
}

//**************************************************************************************
void	WindowTabSkyTravel::SetCameraFOVptr(TYPE_CameraFOV	*cameraFOVarrayPtr)
{
	cCameraFOVarrayPtr	=	cameraFOVarrayPtr;
}

//*****************************************************************************
bool	WindowTabSkyTravel::DrawTelescopeFOV(TYPE_CameraFOV *fovPtr, short	telescopeXX, short telescopeYY)
{
bool	fovWasDrawn;
short	topLeft_XX;
short	topLeft_YY;
short	topRight_XX;
short	topRight_YY;
short	btmLeft_XX;
short	btmLeft_YY;
short	btmRight_XX;
short	btmRight_YY;

double	fovWidth_RAD;
double	fovHeight_RAD;
int		pixelsWide;
int		pixelsTall;


	SetColor(RED);
//	SetColor(WHITE);
	SetColor(fovPtr->OutLineColor);

	fovWasDrawn	=	false;
	if (fovPtr->IsValid && fovPtr->FOVenabled)
	{
		fovWasDrawn	=	true;
		//*	get FOV width and height in radians
		fovWidth_RAD		=	RADIANS(fovPtr->FOV_X_arcSeconds / 3600.0);
		fovHeight_RAD		=	RADIANS(fovPtr->FOV_Y_arcSeconds / 3600.0);

		//	cXfactor is	pixels per radian
		pixelsWide	=	fovWidth_RAD * cXfactor;
		pixelsTall	=	fovHeight_RAD * cYfactor;

		topLeft_XX	=	telescopeXX - (pixelsWide / 2);
		topLeft_YY	=	telescopeYY + (pixelsTall / 2);

		topRight_XX	=	telescopeXX + (pixelsWide / 2);
		topRight_YY	=	telescopeYY + (pixelsTall / 2);

		btmLeft_XX	=	telescopeXX - (pixelsWide / 2);
		btmLeft_YY	=	telescopeYY - (pixelsTall / 2);

		btmRight_XX	=	telescopeXX + (pixelsWide / 2);
		btmRight_YY	=	telescopeYY - (pixelsTall / 2);

		//*	check to see if this camera has an offset
		if (fovPtr->RighttAscen_Offset != 0)
		{
			topLeft_XX	+=	RADIANS(fovPtr->RighttAscen_Offset) * cXfactor;
			topRight_XX	+=	RADIANS(fovPtr->RighttAscen_Offset) * cXfactor;
			btmLeft_XX	+=	RADIANS(fovPtr->RighttAscen_Offset) * cXfactor;
			btmRight_XX	+=	RADIANS(fovPtr->RighttAscen_Offset) * cXfactor;
		}
		if (fovPtr->Declination_Offset != 0)
		{
			topLeft_YY	+=	RADIANS(fovPtr->Declination_Offset) * cXfactor;
			topRight_YY	+=	RADIANS(fovPtr->Declination_Offset) * cXfactor;
			btmLeft_YY	+=	RADIANS(fovPtr->Declination_Offset) * cXfactor;
			btmRight_YY	+=	RADIANS(fovPtr->Declination_Offset) * cXfactor;
		}
		//-------------------------------------------------------------------------------------
		//*	top line
		//*	draw straight line
		CMoveTo(topLeft_XX,		topLeft_YY);
		CLineTo(topRight_XX,	topRight_YY);

		//*	bottom line
		CMoveTo(btmLeft_XX,		btmLeft_YY);
		CLineTo(btmRight_XX,	btmRight_YY);

		//*	left line
		CMoveTo(topLeft_XX,		topLeft_YY);
		CLineTo(btmLeft_XX,		btmLeft_YY);

		//*	right line
		CMoveTo(topRight_XX,	topRight_YY);
		CLineTo(btmRight_XX,	btmRight_YY);

		//*	now do the label
		if ((btmLeft_XX > 0) && (btmLeft_YY > 0))
		{
			DrawCString(btmLeft_XX, btmLeft_YY, fovPtr->CameraName);
		}
		else if ((topLeft_XX > 0) && (topLeft_YY > 0))
		{
			DrawCString(topLeft_XX, topLeft_YY, fovPtr->CameraName);
		}
		else if ((topRight_XX > 0) && (topRight_YY > 0))
		{
			DrawCString(topRight_XX, topRight_YY, fovPtr->CameraName);
		}
		else if ((btmRight_XX > 0) && (btmRight_YY > 0))
		{
			DrawCString(btmRight_XX, btmRight_YY, fovPtr->CameraName);
		}
	}
	return(fovWasDrawn);
}

//*****************************************************************************
//*	returns the # of FOV's drawn
//*****************************************************************************
int	WindowTabSkyTravel::DrawTelescopeFOV(void)
{
int		fovCount;
bool	fovWasDrawn;
int		iii;
bool	telescopeIsInView;
short	telescopeXX, telescopeYY;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

	fovCount	=	0;
	SetColor(RED);
//	SetColor(WHITE);
	telescopeIsInView	=	GetXYfromRA_Decl(	gTelescopeRA_Radians,
												gTelescopeDecl_Radians,
												&telescopeXX,
												&telescopeYY);
	if (telescopeIsInView && (cCameraFOVarrayPtr != NULL))
	{
		for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
		{
			fovWasDrawn	=	DrawTelescopeFOV(&cCameraFOVarrayPtr[iii], 	telescopeXX,  telescopeYY);

			if (fovWasDrawn)
			{
				fovCount++;
			}
		}
	}
	return(fovCount);
}


//*****************************************************************************
//*	arguments in degrees
static void		CovertAzEl_to_RA_DEC(	double	latitude,		//*	phi
										double	azimuth,		//*	x
										double	elev,			//*	y
										double	*ra,			//*	P
										double	*dec)			//	Q
{
double	sq;
double	q;
double	p;
double	cp;
double	x;
double	y;
double	phi;

	CONSOLE_DEBUG_W_DBL("ra\t\t=",	*ra);


#if 1
	phi	=	RADIANS(latitude);
	x	=	RADIANS(azimuth);
	y	=	RADIANS(elev);

	sq	=	(sin(y) * sin(phi)) + (cos(y) * cos(phi) * cos(x));
	q	=	asin(sq);
	cp	=	(sin(y) - (sin(phi) * sin(q))) / (cos(phi) * cos(q));
	p	=	acos(cp);
#else
double	cosPhi;
double	sinPhi;
double	sinX;
double	cosX;
double	sinY;
double	cosY;
double	a;
double	cq;

#define	FNASN(W)	atan(W / (sqrt(1.0 - (W * W) + 1E-20)))
#define	FNACS(W)	(M_PI / 2.0)-FNASN(W)
	cosPhi	=	cos(RADIANS(latitude));
	sinPhi	=	sin(RADIANS(latitude));

	sinX	=	sin(RADIANS(azimuth));
	cosX	=	cos(RADIANS(azimuth));

	sinY	=	sin(RADIANS(elev));
	cosY	=	cos(RADIANS(elev));

	sq		=	(sinY * sinPhi) + (cosY * cosPhi * cosX);
	q		=	FNASN(sq);
	cq		=	cos(q);
	a		=	cosPhi * cq;
	if (a < 1E-20)
	{
		a	=	1E-20;
	}
	cp		=	(sinY - (sinPhi * sq)) / a;
    p		=	FNACS(cp);
#endif

    *ra		=	(DEGREES(p))/ 15.0;
    *dec	=	DEGREES(q);

	CONSOLE_DEBUG_W_DBL("ra\t\t=",	*ra);
	CONSOLE_DEBUG_W_DBL("dec\t=",	*dec);
}


//*****************************************************************************
void	WindowTabSkyTravel::DrawDomeSlit(void)
{
double	slitWidth_Radians;
//double	slitHeight_Radians;
double	slitLeft_Radians;
double	slitRight_Radians;
double	skyTravelDomeAsimuth;	//*	Skytravel is reversed coordinates

	skyTravelDomeAsimuth	=	360.0 - gDomeAzimuth_degrees;

	CONSOLE_DEBUG(__FUNCTION__);
	SetColor(CYAN);


	slitWidth_Radians	=	2.0 * atan2((gSlitWidth_inches / 2.0), (gDomeDiameter_inches / 2.0));
//	slitHeight_Radians	=	RADIANS(gSlitTop_degrees) - RADIANS(gSlitBottom_degrees);

//	CONSOLE_DEBUG_W_DBL("slitHeight_Degrees\t\t=",	slitHeight_Degrees);
//	CONSOLE_DEBUG_W_DBL("Slit width degrees\t\t=",	DEGREES(slitWidth_Radians));
//	CONSOLE_DEBUG_W_DBL("Slit height degrees\t=",	DEGREES(slitHeight_Radians));

	slitLeft_Radians	=	RADIANS(skyTravelDomeAsimuth) - (slitWidth_Radians / 2);
	slitRight_Radians	=	RADIANS(skyTravelDomeAsimuth) + (slitWidth_Radians / 2);
#if 0


	//	cXfactor is	pixels per radian
	pixelsWide	=	slitWidth_Radians * cXfactor;
	pixelsTall	=	slitHeight_Radians * cYfactor;

	topLeft_XX	=	telescopeXX - (pixelsWide / 2);
	topLeft_YY	=	telescopeYY + (pixelsTall / 2);

	topRight_XX	=	telescopeXX + (pixelsWide / 2);
	topRight_YY	=	telescopeYY + (pixelsTall / 2);

	btmLeft_XX	=	telescopeXX - (pixelsWide / 2);
	btmLeft_YY	=	telescopeYY - (pixelsTall / 2);

	btmRight_XX	=	telescopeXX + (pixelsWide / 2);
	btmRight_YY	=	telescopeYY - (pixelsTall / 2);

	//-------------------------------------------------------------------------------------
	//*	top line
	//*	draw straight line
	CMoveTo(topLeft_XX,		topLeft_YY);
	CLineTo(topRight_XX,	topRight_YY);

	//*	bottom line
	CMoveTo(btmLeft_XX,		btmLeft_YY);
	CLineTo(btmRight_XX,	btmRight_YY);

	//*	left line
	CMoveTo(topLeft_XX,		topLeft_YY);
	CLineTo(btmLeft_XX,		btmLeft_YY);

	//*	right line
	CMoveTo(topRight_XX,	topRight_YY);
	CLineTo(btmRight_XX,	btmRight_YY);
#else
	DrawHorizontalArc(RADIANS(gSlitBottom_degrees), slitLeft_Radians, slitRight_Radians);
	DrawHorizontalArc(RADIANS(gSlitTop_degrees),	slitLeft_Radians, slitRight_Radians);

	DrawVerticalArc(slitLeft_Radians, RADIANS(gSlitBottom_degrees), RADIANS(gSlitTop_degrees));
	DrawVerticalArc(slitRight_Radians, RADIANS(gSlitBottom_degrees), RADIANS(gSlitTop_degrees));
#endif

#if 1
double		slitBtm_RA_deg;
double		slitBtm_Dec_deg;
double		slitTop_RA_deg;
double		slitTop_Dec_deg;
short		x1, y1;
short		x2, y2;
bool		slitInView;
//*	arguments in degrees


	CONSOLE_DEBUG_W_DBL("gDomeAzimuth_degrees\t=",	gDomeAzimuth_degrees);
	CONSOLE_DEBUG_W_DBL("skyTravelDomeAsimuth\t=",	skyTravelDomeAsimuth);



	CovertAzEl_to_RA_DEC(	gObseratorySettings.Latitude,	//*	phi
							skyTravelDomeAsimuth,			//*	az
							gSlitTop_degrees,				//	el,				//*	y
							&slitTop_RA_deg,				//*	P
							&slitTop_Dec_deg);				//	Q

	CovertAzEl_to_RA_DEC(	gObseratorySettings.Latitude,	//*	phi
							skyTravelDomeAsimuth,			//*	az
							gSlitBottom_degrees,			//	el,				//*	y
							&slitBtm_RA_deg,				//*	P
							&slitBtm_Dec_deg);				//	Q



	slitTop_RA_deg	-=	-74.980333;
	slitBtm_RA_deg	-=	-74.980333;


	slitInView	=	GetXYfromRA_Decl(	RADIANS(slitTop_RA_deg),
										RADIANS(slitTop_Dec_deg),
										&x1,
										&y1);

	slitInView	=	GetXYfromRA_Decl(	RADIANS(slitBtm_RA_deg),
										RADIANS(slitBtm_Dec_deg),
										&x2,
										&y2);
	if (slitInView)
	{
		CMoveTo(x1,	y1);
		CLineTo(x2,	y2);
	}
#endif

}

//*****************************************************************************
void	WindowTabSkyTravel::CenterOnDomeSlit(void)
{
double	skyTravelDomeAsimuth;	//*	Skytravel is reversed coordinates

	skyTravelDomeAsimuth	=	360.0 - gDomeAzimuth_degrees;

	cAz0	=	RADIANS(skyTravelDomeAsimuth);
	cElev0	=	RADIANS((gSlitBottom_degrees + gSlitTop_degrees) / 2);

	cDispOptions.dispDomeSlit		=	true;

	ForceReDrawSky();
}



//*****************************************************************************
void	WindowTabSkyTravel::DrawEcliptic(void)
{
double			alpha;
double			alpha_start;
double			alpha_end;
double			alpha_delta;
double			angle;
double			myRamax;
int				xcoord;
int				ycoord;
bool			drawLineFlag	=	false;
TYPE_SpherTrig	sphptr;

//	CONSOLE_DEBUG(__FUNCTION__);

	sphptr.bside	=	kHALFPI - cDecl0;	//* this stays constant

	SetColor(BLUE);
	CPenSize(2);
//	for (alpha = (cRa0 - cRamax); alpha < (cRa0 + cRamax); alpha += (cRamax / 10.0))
	//*	<MLS> 1/4/2021, this allows the Ecliptic to be drawn at all zoom levels
	myRamax		=	cRamax;
	if ((cView_index >= 7) || (myRamax < 0.1))
	{
		myRamax	=	(M_PI / 2);
	}
	alpha_start	=	cRa0 - myRamax;
	alpha_end	=	cRa0 + myRamax;
	alpha_delta	=	myRamax / 50.0;

//	CONSOLE_DEBUG_W_DBL("cRamax\t\t=", cRamax);
//	CONSOLE_DEBUG_W_DBL("myRamax\t\t=", myRamax);
//	CONSOLE_DEBUG_W_DBL("alpha_start\t=", alpha_start);
//	CONSOLE_DEBUG_W_DBL("alpha_end  \t=", alpha_end);
//	CONSOLE_DEBUG_W_DBL("alpha_delta\t=", alpha_delta);

	for (alpha = alpha_start; alpha < alpha_end; alpha += alpha_delta)
	{
		sphptr.cside	=	kHALFPI - ECLIPTIC_ANGLE * sin(alpha);
		sphptr.alpha	=	cRa0 - alpha;
		sphsas(&sphptr);
		sphsss(&sphptr);

		angle	=	sphptr.gamma + cGamang;
		xcoord	=	cWind_x0 + (cXfactor * sphptr.aside * sin(angle));
		ycoord	=	cWind_y0 - (cYfactor * sphptr.aside * cos(angle));

//		CONSOLE_DEBUG_W_NUM("xcoord\t=", xcoord);
//		CONSOLE_DEBUG_W_NUM("ycoord\t=", ycoord);

		if (drawLineFlag)
		{
			CLineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
		else
		{
			CMoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
			drawLineFlag	=	true;
		}
	}
	CPenSize(1);
}

//*********************************************************************
void	WindowTabSkyTravel::DrawGrid(short theSkyColor)
{
double		degrees;
int			myColor;
//setlinestyle(USERBIT_LINE,0x0f0f,NORM_WIDTH);
	if (cNightMode)
	{
		myColor	=	DARKRED;
	}
	else if (theSkyColor > 60)
	{
	//	myColor	=	BLACK;	//*	was blue
		myColor	=	DARKGRAY;
	}
	else
	{
		myColor	=	DARKGRAY;
	}
	SetColor(myColor);

	//-------------------------------------------------------------------------
	//*	draw the great circles
	degrees	=	-80.0;
	while (degrees < 81.0)
	{
		SetColor(myColor);
		DrawGreatCircle(RADIANS(degrees));
		degrees	+=	10.0;
	}

	if (cView_index <= 5)
	{
		SetColor(myColor);
		DrawGreatCircle(RADIANS(89.5));
		SetColor(myColor);
		DrawGreatCircle(RADIANS(89.7));
	}

	if (cView_index <= 3)
	{
		SetColor(myColor);
		DrawGreatCircle(RADIANS(89.9));
	}

	if (cView_index <= 1)
	{
		SetColor(myColor);
		DrawGreatCircle(RADIANS(89.95));
	}

	//-------------------------------------------------------------------------
	//*	draw the north/south lines
	degrees	=	0.0;
	while (degrees < 360.0)
	{
		SetColor(myColor);
		DrawNorthSouthLine(RADIANS(degrees));
		degrees	+=	15.0;
//		degrees	+=	15.0;	//*	put it in twice to match K-Stars
	}
}


//*****************************************************************************
//* draw a great circle centered on (zenith-horizon) (npole-equator) rotated by angle2
//*****************************************************************************
void	WindowTabSkyTravel::DrawHorizon(void)
{
double	alpha;
double	aside;
double	cos_aside;
double	cos_bside;
double	sin_bside;
double	gamma;
double	gam;
double	delta_ra;
double	rtasc;
double	codecl;
int		xcoord,ycoord,ftflag	=	0;

	SetColor(BROWN);	//* make horizon brown

	gam			=	0.0;
	codecl		=	kHALFPI - cElev0;

	rtasc		=	cView_angle / 2.0;	//* starting point
	sin_bside	=	sin(codecl);
	cos_bside	=	cos(codecl);

	switch(gST_DispOptions.EarthDispMode)
	{
		case 0:
			delta_ra	=	rtasc / 100.0;		//*increment
			break;

		case 1:
			delta_ra	=	rtasc / 20.0;		//*increment
			CPenSize(2);
			break;

		case 2:
			delta_ra	=	rtasc / 900.0;		//*increment
			break;

		case 3:
			SetColor(DARKGREEN);
			delta_ra	=	rtasc / 900.0;		//*increment
			break;

		default:
			delta_ra	=	rtasc / 100.0;		//*increment
			break;

	}

	for (alpha = -rtasc; alpha < rtasc; alpha += delta_ra)
	{
		cos_aside	=	sin_bside * cos(alpha);
		aside		=	acos(cos_aside);
		if (fabs(aside) > kEPSILON)
		{
			gamma	=	asin(sin(alpha) / sin(aside));
		}
		else
		{
			gamma	=	0;
		}
		if ((cos_bside * cos_aside) < 0.0)
		{
			gamma	=	PI - gamma;
		}

		xcoord	=	cWind_x0	+	(cXfactor * aside * sin(gamma - gam));
		ycoord	=	cWind_y0	+	(cYfactor * aside * cos(gamma - gam));
		if (ftflag)
		{
			CLineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
		else
		{
			CMoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
			ftflag++;
		}

		if (cDispOptions.dispEarth)
		{
			//*	this fills with earth
			CLineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + wind_uly + cWind_height);
			CMoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
	}
	CPenSize(1);
}


//*****************************************************************************
//* draw a line horizontal with respect to the horizon
//*	this is for drawing the dome slit.
//*****************************************************************************
void	WindowTabSkyTravel::DrawHorizontalArc(double elevAngle, double startAz, double endAz)
{
double	myAZvalue1;
double	myAZvalue2;
double	azDelta;
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;

//	CONSOLE_DEBUG(__FUNCTION__);

	myAZvalue1	=	startAz;
	azDelta		=	GetRA_DEC_detla(cView_index);

	while (myAZvalue1 < (endAz - azDelta))
	{
		//*	we are going draw a line between 2 points
		//*	calculate the 2nd pt
		myAZvalue2	=	myAZvalue1 + azDelta;

		pt1inView	=	GetXYfromAz_Elev(	myAZvalue1,
											elevAngle,
											&pt1_XX, &pt1_YY);

		pt2inView	=	GetXYfromAz_Elev(	myAZvalue2,
											elevAngle,
											&pt2_XX, &pt2_YY);
		//*	if both pints are on the screen, draw it
		if (pt1inView && pt2inView)
		{
//			CONSOLE_DEBUG_W_NUM("pt1_XX\t=", pt1_XX);
//			CONSOLE_DEBUG_W_NUM("pt1_YY\t=", pt1_YY);
//			CONSOLE_DEBUG_W_NUM("pt2_XX\t=", pt2_XX);
//			CONSOLE_DEBUG_W_NUM("pt2_YY\t=", pt2_YY);
			CMoveTo(pt1_XX,	pt1_YY);
			CLineTo(pt2_XX,	pt2_YY);
		}
		myAZvalue1	+=	azDelta;
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::DrawVerticalArc(double azimuthAngle, double startElev, double endElev)
{
double	myELEVvalue1;
double	myELEVvalue2;
double	elevDelta;
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;

//	CONSOLE_DEBUG(__FUNCTION__);

	myELEVvalue1	=	startElev;
	elevDelta		=	GetRA_DEC_detla(cView_index);

//	while (myELEVvalue1 < (endElev - elevDelta))
	while (myELEVvalue1 < endElev)
	{
		//*	we are going draw a line between 2 points
		//*	calculate the 2nd pt
		myELEVvalue2	=	myELEVvalue1 + elevDelta;
		if (myELEVvalue2 > endElev)
		{
			myELEVvalue2	=	endElev;
		}
		pt1inView	=	GetXYfromAz_Elev(	azimuthAngle,
											myELEVvalue1,
											&pt1_XX, &pt1_YY);

		pt2inView	=	GetXYfromAz_Elev(	azimuthAngle,
											myELEVvalue2,
											&pt2_XX, &pt2_YY);
		//*	if both pints are on the screen, draw it
		if (pt1inView && pt2inView)
		{
			CMoveTo(pt1_XX,	pt1_YY);
			CLineTo(pt2_XX,	pt2_YY);
		}
		myELEVvalue1	+=	elevDelta;
	}
}

//*****************************************************************************
static double	GetRA_DEC_detla(int viewIndex)
{
double	angleDelta;

	switch(viewIndex)
	{
		case 0:		angleDelta	=	M_PI / 3000.0;	break;	//	0.001046667
		case 1:		angleDelta	=	M_PI / 2500.0;	break;
		case 2:		angleDelta	=	M_PI / 2000.0;	break;
		case 3:		angleDelta	=	M_PI / 1500.0;	break;
		case 4:		angleDelta	=	M_PI / 1000.0;	break;
		case 5:		angleDelta	=	M_PI / 500.0;	break;
		case 6:		angleDelta	=	M_PI / 300.0;	break;
		case 7:		angleDelta	=	M_PI / 250.0;	break;
		case 8:		angleDelta	=	M_PI / 150.0;	break;
		case 9:		angleDelta	=	M_PI / 100.0;	break;
		case 10:	angleDelta	=	M_PI / 100.0;	break;
		case 11:	angleDelta	=	M_PI / 100.0;	break;

		default:angleDelta	=	M_PI / 100.0;	break;
	}
	return(angleDelta);
}


//*****************************************************************************
//*	draw a great circle at the specified dec angle
//*	returns the number of points draw, 0 means nothing was drawn
//*	declinationAngle is in radians
//*****************************************************************************
int	WindowTabSkyTravel::DrawGreatCircle(double declinationAngle, bool rainbow)
{
double	rtAscen1;
double	rtAscen2;
double	rtAscenDelta;
double	minDeltaRA;
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;
int		theColor;
int		segmentsDrnCnt;
int		adjustmentCnt	=	0;	//*	for adjusting angle delta table in GetRA_DEC_detla()
int		leftMost_X;
int		leftMost_Y;
int		rightMost_X;
int		rightMost_Y;
int		topMost_X;
int		topMost_Y;
int		btmMost_X;
int		btmMost_Y;
char	numberStr[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	leftMost_X		=	10000;
	leftMost_Y		=	10000;
	topMost_X		=	10000;
	topMost_Y		=	10000;
	rightMost_X		=	0;
	rightMost_Y		=	0;
	btmMost_X		=	0;
	btmMost_Y		=	0;


	segmentsDrnCnt	=	0;
	minDeltaRA		=	GetRA_DEC_detla(0) / 10;
	rtAscenDelta	=	GetRA_DEC_detla(cView_index);
	rtAscen1		=	0.0;
	while (rtAscen1 < ((2 * M_PI) - rtAscenDelta))
	{
		//*	we are going draw a line between 2 points
		//*	calculate the 2nd pt
		rtAscen2	=	rtAscen1 + rtAscenDelta;

		pt1inView	=	GetXYfromRA_Decl(	rtAscen1,
											declinationAngle,
											&pt1_XX, &pt1_YY);

		pt2inView	=	GetXYfromRA_Decl(	rtAscen2,
											declinationAngle,
											&pt2_XX, &pt2_YY);

		//*	if both pints are on the screen, draw it
		if (pt1inView && pt2inView)
		{
			if (rainbow)
			{
				theColor	=	(rtAscen1 / (2 * M_PI)) * COLOR_LAST;
				if (theColor == BLACK)
				{
					theColor	=	WHITE;
				}
				SetColor(theColor);
			}

			//*	check the step size, we want to make sure nothing bigger than 20 pixels
			//*	I tried to do this in a separate routine but it slowed down way too much
			while (((abs(pt2_XX - pt1_XX) + abs(pt2_YY - pt1_YY)) > 20) && (rtAscenDelta > minDeltaRA))
			{
				rtAscenDelta	-=	0.001;
				//*	check to make sure we didnt go too far
				if (rtAscenDelta < minDeltaRA)
				{
					rtAscenDelta	=	minDeltaRA;
				}

				//*	re-calculate point 2
				rtAscen2	=	rtAscen1 + rtAscenDelta;
				pt2inView	=	GetXYfromRA_Decl(	rtAscen2,
													declinationAngle,
													&pt2_XX, &pt2_YY);
				adjustmentCnt++;
			}
			//*	now we can go ahead and draw the line
			CMoveTo(pt1_XX,	pt1_YY);
			CLineTo(pt2_XX,	pt2_YY);
			segmentsDrnCnt++;


		}
		rtAscen1	+=	rtAscenDelta;
		if (gDashedLines)
		{
			rtAscen1	+=	rtAscenDelta;
		}

		//*	this is to calculate where to draw the numbers along the edges of the screen
		if (pt1inView)
		{
			if (pt1_XX < leftMost_X)
			{
				leftMost_X	=	pt1_XX;
				leftMost_Y	=	pt1_YY;
			}
			if (pt1_XX > rightMost_X)
			{
				rightMost_X	=	pt1_XX;
				rightMost_Y	=	pt1_YY;
			}
			if (pt1_YY < topMost_Y)
			{
				topMost_X	=	pt1_XX;
				topMost_Y	=	pt1_YY;
			}
			if (pt1_YY > btmMost_Y)
			{
				btmMost_X	=	pt1_XX;
				btmMost_Y	=	pt1_YY;
			}
		}
		if (pt2inView)
		{
			if (pt2_XX < leftMost_X)
			{
				leftMost_X	=	pt2_XX;
				leftMost_Y	=	pt2_YY;
			}
			if (pt2_XX > rightMost_X)
			{
				rightMost_X	=	pt2_XX;
				rightMost_Y	=	pt2_YY;
			}
			if (pt2_YY < topMost_Y)
			{
				topMost_X	=	pt2_XX;
				topMost_Y	=	pt2_YY;
			}
			if (pt2_YY > btmMost_Y)
			{
				btmMost_X	=	pt2_XX;
				btmMost_Y	=	pt2_YY;
			}
		}

	}
//	if (adjustmentCnt > 0)
//	{
//		CONSOLE_DEBUG_W_NUM("adjustmentCnt\t=", adjustmentCnt);
//	}


	//----------------------------------------------------
	//*	draw numbers along the edges
	if (declinationAngle == 0.0)
	{
		strcpy(numberStr, "EQ");
	}
	else if (declinationAngle > 1.553343)
	{
		sprintf(numberStr, "%1.1fN", DEGREES(declinationAngle));
	}
	else if (declinationAngle > 0.0)
	{
		sprintf(numberStr, "%1.0fN", DEGREES(declinationAngle));
	}
	else
	{
		sprintf(numberStr, "%1.0fS", -DEGREES(declinationAngle));
	}
	SetColor(DARKGREEN);
	if (leftMost_X < 30)
	{
		DrawCString(3, leftMost_Y, numberStr);
	}
	if (rightMost_X > (cWind_width - 30))
	{
		DrawCString((cWind_width - 30), rightMost_Y, numberStr);
	}
	if (topMost_Y < 15)
	{
		DrawCString(topMost_X, 15, numberStr);
	}
	if (btmMost_Y > (cWind_height - 15))
	{
		DrawCString(btmMost_X, (cWind_height - 4), numberStr);
	}


//	CONSOLE_DEBUG_W_NUM("segmentsDrnCnt\t=", segmentsDrnCnt);
	return(segmentsDrnCnt);

}

//*****************************************************************************
//*	returns the number of points draw, 0 means nothing was drawn
//*****************************************************************************
int	WindowTabSkyTravel::DrawNorthSouthLine(double rightAscen_Rad)
{
double	declination1;
double	declination2;
double	declDelta;
double	minDeltaDec;
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;
int		segmentsDrnCnt;
char	numberStr[32];
int		leftMost_X;
int		leftMost_Y;
int		rightMost_X;
int		rightMost_Y;
int		topMost_X;
int		topMost_Y;
int		btmMost_X;
int		btmMost_Y;

	segmentsDrnCnt	=	0;
	minDeltaDec		=	GetRA_DEC_detla(0) / 10.0;
	declDelta		=	GetRA_DEC_detla(cView_index);
//	declDelta		=	declDelta / 2.0;

	leftMost_X		=	10000;
	leftMost_Y		=	10000;
	topMost_X		=	10000;
	topMost_Y		=	10000;
	rightMost_X		=	0;
	rightMost_Y		=	0;
	btmMost_X		=	0;
	btmMost_Y		=	0;

	//*	start at the top (north pole)
	declination1	=	RADIANS(80);
	while (declination1 > RADIANS(-80.0))
	{
		declination2	=	declination1 - declDelta;
		if (declination2 < RADIANS(-80.0))
		{
			declination2	=	RADIANS(-80.0);
		}
		pt1inView	=	GetXYfromRA_Decl(	rightAscen_Rad,
											declination1,
											&pt1_XX, &pt1_YY);

		pt2inView	=	GetXYfromRA_Decl(	rightAscen_Rad,
											declination2,
											&pt2_XX, &pt2_YY);

		if (pt1inView && pt2inView)
		{
			//*	check the step size, we want to make sure nothing bigger than 20 pixels
			//*	I tried to do this in a separate routine but it slowed down way too much
			while (((abs(pt2_XX - pt1_XX) + abs(pt2_YY - pt1_YY)) > 20) && (declDelta > minDeltaDec))
			{
				declDelta	-=	0.001;
				//*	check to make sure we didnt go too far
				if (declDelta < minDeltaDec)
				{
					declDelta	=	minDeltaDec;
				}

				//*	re-calculate point 2
				declination2	=	declination1 - declDelta;
				pt2inView	=	GetXYfromRA_Decl(	rightAscen_Rad,
													declination2,
													&pt2_XX, &pt2_YY);
//				adjustmentCnt++;
			}


			CMoveTo(pt1_XX,	pt1_YY);
			CLineTo(pt2_XX,	pt2_YY);
			segmentsDrnCnt++;
		}

		//*	this is to calculate where to draw the numbers along the edges of the screen
		if (pt1inView)
		{
			if (pt1_XX < leftMost_X)
			{
				leftMost_X	=	pt1_XX;
				leftMost_Y	=	pt1_YY;
			}
			if (pt1_XX > rightMost_X)
			{
				rightMost_X	=	pt1_XX;
				rightMost_Y	=	pt1_YY;
			}
			if (pt1_YY < topMost_Y)
			{
				topMost_X	=	pt1_XX;
				topMost_Y	=	pt1_YY;
			}
			if (pt1_YY > btmMost_Y)
			{
				btmMost_X	=	pt1_XX;
				btmMost_Y	=	pt1_YY;
			}
		}
		if (pt2inView)
		{
			if (pt2_XX < leftMost_X)
			{
				leftMost_X	=	pt2_XX;
				leftMost_Y	=	pt2_YY;
			}
			if (pt2_XX > rightMost_X)
			{
				rightMost_X	=	pt2_XX;
				rightMost_Y	=	pt2_YY;
			}
			if (pt2_YY < topMost_Y)
			{
				topMost_X	=	pt2_XX;
				topMost_Y	=	pt2_YY;
			}
			if (pt2_YY > btmMost_Y)
			{
				btmMost_X	=	pt2_XX;
				btmMost_Y	=	pt2_YY;
			}
		}
		declination1	-=	declDelta;
		if (gDashedLines)
		{
			declination1	-=	declDelta;
		}
	}
	//----------------------------------------------------
	//*	draw numbers along the edges
	sprintf(numberStr, "%1.0fh", DEGREES(rightAscen_Rad / 15.0));
	if (leftMost_X < 30)
	{
		DrawCString(3, leftMost_Y, numberStr);
	}
	if (rightMost_X > (cWind_width - 30))
	{
		DrawCString((cWind_width - 30), rightMost_Y, numberStr);
	}
	if (topMost_Y < 15)
	{
		DrawCString(topMost_X, 15, numberStr);
	}
	if (btmMost_Y > (cWind_height - 15))
	{
		DrawCString(btmMost_X, (cWind_height - 4), numberStr);
	}


//	CONSOLE_DEBUG_W_NUM("segmentsDrnCnt\t=", segmentsDrnCnt);
	return(segmentsDrnCnt);

}


//*****************************************************************************
void	WindowTabSkyTravel::DrawCompass(void)
{
double	dtemp,angle,left_edge,right_edge;
int		ii;
int		xpos;
int		ypos;

//	CONSOLE_DEBUG(__FUNCTION__);

	SetColor(YELLOW);
	left_edge	=	-cAz0 - (cView_angle / 2.0);

	while (left_edge < -kTWOPI)
	{
		left_edge	+=	kTWOPI;
	}
	while (left_edge > kTWOPI)
	{
		left_edge	-=	kTWOPI;
	}

	right_edge	=	left_edge + cView_angle;

	for (angle = -kTWOPI,ii = 0;angle < kTWOPI; angle += (kTWOPI/16.), ii++)
	{
		if ((angle >= left_edge) && (angle <= right_edge))
		{
			dtemp	=	angle - left_edge;
			if (dtemp >= 0.0)
			{
				xpos	=	wind_ulx + (dtemp * (1.0 * cWind_width) / cView_angle);
				ypos	=	wind_uly + cWind_height - (THGT / 2);
				DrawCString(cWorkSpaceTopOffset + xpos,
							cWorkSpaceLeftOffset  + ypos, gCompass_text[ii & 0x0f]);
			}
		}
	}
}



//*****************************************************************************
//	plot from an unsorted (in declination) list of objects pointed to
//	by objectptr array i.e. cPlanets or gZodPtr
//	with n celest_data_struct and with names pointed to by name array
//	this routine takes care of planets (n=10) and zodiac (n=12)
//*****************************************************************************
void	WindowTabSkyTravel::PlotSkyObjects(	TYPE_CelestData	*objectptr,
											const char		**name,
											const char		**shapes,
											long			objCnt)
{
TYPE_SpherTrig	sphptr;
int				ii;
int				jj;
short			xcoord, ycoord;
short			xx, yy;
char			not_sunmoon;				//* for convenience in determining if object is not Sun or Moon
double			angle,radius,theta,phasang,posang,minang;
double			radx,rady,dx,dy;
double			rad0[3];
double			deldecl, delra;
int				myColor;

//	CONSOLE_DEBUG("---------------------------------------------------------------");
//	CONSOLE_DEBUG_W_LONG("objCnt\t=",		objCnt);
//	CONSOLE_DEBUG_W_NUM("cWind_width\t=",	cWind_width);
//	CONSOLE_DEBUG_W_NUM("cWind_height\t=",	cWind_height);
//	CONSOLE_DEBUG_W_NUM("wind_ulx\t=",		wind_ulx);
//	CONSOLE_DEBUG_W_NUM("wind_uly\t=",		wind_uly);
	rad0[0]			=	cMoon_radius;
	rad0[1]			=	cSun_radius;
	rad0[2]			=	cEarth_shadow_radius;

	sphptr.bside	=	kHALFPI - cDecl0;	//* this remains constant throughout

	switch(objCnt)
	{
		case 10:			myColor	=	CYAN;			break;	//* planet names
		case kZodiacCount:	myColor	=	LIGHTMAGENTA;	break;	//* zodiac sign names
		default:			myColor	=	WHITE;			break;
	}
	SetColor(myColor);

	for (ii=objCnt-1; ii>=0; ii--)
	{
		objectptr[ii].curXX	=	-100;
		objectptr[ii].curYY	=	-100;

		if ((objCnt == kZodiacCount) || ((objCnt == kPlanetObjectCnt) && (ii > 1)))
		{
			not_sunmoon	=	true;
		}
		else
		{
			not_sunmoon	=	false;
		}
		sphptr.alpha	=	cRa0 - objectptr[ii].ra;

		if (sphptr.alpha > PI)
		{
			sphptr.alpha	-=	kTWOPI;
		}
		else if (sphptr.alpha < -PI)
		{
			sphptr.alpha	+=	kTWOPI;
		}

		if ((cRamax == 0.0) || (fabs(sphptr.alpha) <= cRamax))	//* in bounds for ra?
		{

			sphptr.cside	=	kHALFPI - objectptr[ii].decl;
			sphsas(&sphptr);
			if (sphptr.aside < cRadmax)	//* within bounding circle?
			{
				sphsss(&sphptr);
				angle	=	sphptr.gamma + cGamang;

				//*	compute x and y coordinates
				//*	x	=	x0 + cXfactor * aside * cos(angle)
				//*	y	=	y0 - cYfactor * aside * sin(angle) (minus sign is because plus y is down)

				xcoord	=	cWind_x0 + (cXfactor * sphptr.aside * sin(angle));

				//* are they both within window?
				if ((xcoord >= wind_ulx) && (xcoord <= wind_ulx + cWind_width))
				{
					objectptr[ii].curXX	=	xcoord;

					ycoord	=	cWind_y0 - (cYfactor * sphptr.aside * cos(angle));
					if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
					{
						objectptr[ii].curYY	=	ycoord;

						if ((objCnt == kPlanetObjectCnt) && (ii>1) && !cDispOptions.dispSymbols && !cDispOptions.dispNames)
						{
							DrawStar_shape(xcoord, ycoord, objectptr[ii].magn);
						}
						if ((objCnt == kPlanetObjectCnt) && (ii<2))	//* Moon(0) or Sun(1)
						{
							SetColor(WHITE);
						//	setfillstyle(SOLID_FILL,WHITE);
							radius	=	rad0[ii];
							if (ii == 1)	//* Sun, filled ellipse
							{
								radx	=	radius * cXfactor;
								rady	=	radius * cYfactor;
								if (radx < 3.0)	//* don't draw if too small
								{
									DrawVector(WHITE, xcoord, ycoord, 1, shapes[ii]);
									goto next1;
								}
								FillEllipse(xcoord, ycoord, radx, rady);
							}
							if (ii == 0)	//* Moon, filled crescent
							{
								minang	=	(cView_index + 1) * (7.0 * PI/180.0);	//* to avoid floodfill problems
								phasang	=	cPhase_angle;
								if (phasang < 1.0e-2)
								{
									phasang	=	0.0;
								}
								posang	=	cPosition_angle + cGamang;
								if (cPhase_angle < 0.0)
								{
									posang	=	posang + PI;
								}
								radx	=	radius * cXfactor;
								rady	=	radius * cYfactor;

								if (radx < 3.0)	//* don't draw if too small
								{
									DrawVector(LIGHTGRAY, xcoord, ycoord, 1, shapes[ii]);
									goto next1;
								}
								SetColor(DARKGRAY);	//* complete circle
								FillEllipse(xcoord,ycoord, radx, rady);

								if (phasang != 0.0)
								{
									SetColor(YELLOW);	//* semi-ellipse
									theta	=	posang;
									for (jj=0; theta < (posang + PI); jj++)
									{
										xx	=	xcoord + (radx * cos(theta));
										yy	=	ycoord + (rady * sin(theta));
										if (jj)
										{
											CLineTo(xx, yy);
										}
										else
										{
											CMoveTo(xx, yy);
										}
										theta	+=	PI / 64.0;
									}

									//* 64 points in a semiellipse
									for (jj=0,theta = -PI/2.0; theta < (33.0 * PI / 64.0); jj++, theta += PI /64.0)
									{
										dx	=	radx * cos(theta)*cos(phasang);
										dy	=	rady * sin(theta);

										//* coordinate rotation by posang

										yy	=	ycoord + ((dx * cos(posang)) + (dy * sin(posang)));
										xx	=	xcoord + ((dy * cos(posang)) - (dx * sin(posang)));

										CLineTo(xx, yy);
									}

									//* fill the crescent part

									if (fabs(cPhase_angle) > minang)
									{
								//+		setfillstyle(SOLID_FILL,YELLOW);
										theta	=	posang+PI/ 2.0;
										xx		=	xcoord + ((radx - 1.5) * cos(theta));	//* make sure it is inside
										yy		=	ycoord + ((rady - 1.5) * sin(theta));
								//+		floodfill(x,y,YELLOW);
									}
								}
								if (cLunarEclipseFlag)	//* plot the anti-sun
								{
									//* correction for parallax

									deldecl	=	objectptr[MON].decl - mon_geo_decl;
									delra	=	objectptr[MON].ra-mon_geo_ra;

									//* position of anti-sun (earth shadow)

									sphptr.alpha	=	cRa0 - PI - objectptr[SUN].ra - delra;	//*	antisun ra
									sphptr.cside	=	kHALFPI + objectptr[SUN].decl - deldecl;		//* antisun decl

									sphsas(&sphptr);
									sphsss(&sphptr);

									angle	=	sphptr.gamma + cGamang;
									xcoord	=	cWind_x0 + (cXfactor * sphptr.aside * sin(angle));
									ycoord	=	cWind_y0 - (cYfactor * sphptr.aside * cos(angle));
									radx	=	rad0[2] * cXfactor;
									rady	=	rad0[2] * cYfactor;
								//	SetColor(BLACK);
									SetColor(WHITE);
									FillEllipse(xcoord, ycoord, radx, rady);
								}
							}
						}
						//*	no symbols or names for sun and moon
						if (not_sunmoon)
						{
							if (cDispOptions.dispSymbols)	//* symbols except if moon or sun
							{
							short	planetScale;
						//		CONSOLE_DEBUG_W_NUM("cView_index\t=", cView_index);
								if (cView_index <= 2)
								{
									planetScale	=	3;
								}
								else if (cView_index <= 5)
								{
									planetScale	=	2;
								}
								else
								{
									planetScale	=	1;
								}
								DrawVector(myColor, xcoord, ycoord, planetScale, shapes[ii]);

								if (cView_index <= 4)
								{
									//*	if we are this far zoomed in, show the name as well
									DrawCString(xcoord + 18, ycoord, name[ii]);
								}

							}
							else if (cDispOptions.dispNames)	//* names except if moon or sun
							{
								DrawCString(xcoord, ycoord, name[ii]);
							}
						}
					}
				}
			}
		}
	next1:;
	}
}



//*********************************************************************
void	ConvertRadiansToDegMinSec(	double	radianValue,
									short	*argDegrees,
									short	*argMinutes,
									double	*argSeconds)
{
double	degreeValue;
short	signValue;
short	degrees;
short	minutes;
double	seconds;

	signValue	=	1;
	degreeValue	=	radianValue * 180 / PI;
	if (degreeValue < 0)
	{
		signValue	=	-1;
		degreeValue	=	-degreeValue;
	}
	degrees	=	degreeValue;
	minutes	=	(degreeValue - degrees) * 60;
	seconds	=	(degreeValue * 3600) - (degrees * 3600) - (minutes * 60);

	*argDegrees	=	degrees * signValue;
	*argMinutes	=	minutes;
	*argSeconds	=	seconds;
}

//*********************************************************************
void	FromatRa_Dec_toString(double rtAsencValue, double declValue, char *formatString)
{
short	raHours;
short	raMinutes;
double	raSecs;
short	declDegress;
short	declMinutes;
double	declSecs;


	ConvertRadiansToDegMinSec((rtAsencValue / 15), &raHours, &raMinutes, &raSecs);

	ConvertRadiansToDegMinSec(declValue, &declDegress, &declMinutes, &declSecs);

	sprintf(formatString, "RA=%3d:%02d:%04.1f, DECL=%3d:%02d:%04.1f",
															raHours,
															raMinutes,
															raSecs,
															declDegress,
															declMinutes,
															declSecs);

}



//*****************************************************************************
void	WindowTabSkyTravel::DrawCursorLocationInfo(void)
{
char	cursorString[512];
long	elevDegress;
long	azDegress;
short	raHours;
short	raMinutes;
double	raSecs;
short	declDegress;
short	declMinutes;
double	declSecs;

	elevDegress	=	cCursor_elev * 180 / PI;
	azDegress	=	cCursor_az * 180 / PI;

	ConvertRadiansToDegMinSec((cCursor_ra / 15), &raHours, &raMinutes, &raSecs);

	ConvertRadiansToDegMinSec(cCursor_decl, &declDegress, &declMinutes, &declSecs);

	elevDegress	=	cCursor_elev * 180 / PI;
	azDegress	=	cCursor_az * 180 / PI;

	//******************************************************************
	//*	<MLS> 1/10/2021
	azDegress	=	-azDegress;
	if (azDegress < 0.0)
	{
		azDegress	+=	360.0;
	}
	//******************************************************************

	sprintf(cursorString, "RA=%3d:%02d:%04.1f, DECL=%3d:%02d:%04.1f AZ=%ld ELEV=%ld",
															raHours,
															raMinutes,
															raSecs,
															declDegress,
															declMinutes,
															declSecs,
															azDegress,
															elevDegress);


	if (cInform_dist < kMinInformDist)
	{
		if (strlen(cInform_name) > 0)
		{
			strcat(cursorString, " - ");
			strcat(cursorString, cInform_name);
		}
	}

	SetWidgetText(kSkyTravel_CursorInfoTextBox,	cursorString);

}


//*********************************************************************
static long		FindXX_YYinObjectList(	TYPE_CelestData	*objectDataBase,
										long			objectCount,
										short			findXX,
										short			findYY,
										TYPE_CelestData	*closestObject)
{
long	ii;
long	curFoundIndex;
long	curFoundDistSQRD;
long	distanceSQRD;	//*	no reason to take sqrt, just compare squares
long	distanceInPixels;
long	deltaXX;
long	deltaYY;

	curFoundDistSQRD	=	999999;
	curFoundIndex		=	-1;

	//*	go thru the list and find the closest object
	if (objectDataBase != NULL)
	{

		for (ii=0; ii<objectCount; ii++)
		{
			if ((objectDataBase[ii].curXX >= 0) && (objectDataBase[ii].curYY >= 0))
			{
				deltaXX			=	abs(findXX - objectDataBase[ii].curXX);
				deltaYY			=	abs(findYY - objectDataBase[ii].curYY);
				distanceSQRD	=	(deltaXX * deltaXX) + (deltaYY * deltaYY);
				if (distanceSQRD < curFoundDistSQRD)
				{
					curFoundIndex		=	ii;
					curFoundDistSQRD	=	distanceSQRD;
				}
			}
		}
		if (curFoundIndex >= 0)
		{
			*closestObject	=	objectDataBase[curFoundIndex];
		}
	}
	distanceInPixels	=	sqrt(curFoundDistSQRD);
	return(distanceInPixels);
}

//*********************************************************************
void	WindowTabSkyTravel::FindObjectNearCursor(void)
{
TYPE_CelestData	foundObject;
TYPE_CelestData	cloestObject;
long			cloestDistance;
long			pixDist;

	memset(&foundObject, 0, sizeof(TYPE_CelestData));
	memset(&cloestObject, 0, sizeof(TYPE_CelestData));

	cloestDistance	=	9999;

	if (cStarDataPtr != NULL)
	{
		pixDist	=	FindXX_YYinObjectList(	cStarDataPtr,
											cStarCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		//*	since this is the first one, it will always be true
		if (pixDist < cloestDistance)
		{
			cloestDistance	=	pixDist;
			cloestObject	=	foundObject;
		}
	}
	pixDist	=	FindXX_YYinObjectList(	cPlanets,
										kPlanetObjectCnt,
										cCsrx,				//*	current x location of cursor,
										cCsry,				//*	current y location of cursor,
										&foundObject);		//*	closestObject
	if (pixDist < cloestDistance)
	{
		cloestDistance	=	pixDist;
		cloestObject	=	foundObject;
	}

	if (cDispOptions.dispNGC && (cNGCobjectPtr != NULL) && (cNGCobjectCount > 0))
	{
		pixDist	=	FindXX_YYinObjectList(	cNGCobjectPtr,
											cNGCobjectCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < cloestDistance)
		{
			cloestDistance	=	pixDist;
			cloestObject	=	foundObject;
		}
	}

	if (gZodiacPtr != NULL)
	{
		pixDist	=	FindXX_YYinObjectList(	gZodiacPtr,
											kZodiacCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < cloestDistance)
		{
//			CONSOLE_DEBUG("Zodiac");
			cloestDistance	=	pixDist;
			cloestObject	=	foundObject;
		}
	}

	//*	kDataSrc_YaleBrightStar
	if (cDispOptions.dispYale && (cYaleStarDataPtr != NULL) && (cYaleStarCount > 0))
	{
		pixDist	=	FindXX_YYinObjectList(	cYaleStarDataPtr,
											cYaleStarCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < cloestDistance)
		{
			cloestDistance	=	pixDist;
			cloestObject	=	foundObject;
		}
	}


	//*	kDataSrc_Hipparcos
	if (cDispOptions.dispHIP && (cHipObjectPtr != NULL) && (cHipObjectCount > 0))
	{
		pixDist	=	FindXX_YYinObjectList(	cHipObjectPtr,
											cHipObjectCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < cloestDistance)
		{
			cloestDistance	=	pixDist;
			cloestObject	=	foundObject;
		}
	}


	cInform_dist	=	0x7fff;

	//*	lets make sure we have something semi valid
	if (cloestDistance < kMinInformDist)
	{
	short	ii;
	char	tmpString[8];
	bool	foundMoreInfo;
	char	moreInfo[128];
	double	distance;

		cInform_dist		=	cloestDistance;


		cInform_dist	=	1;
		switch(cloestObject.dataSrc)
		{
			case kDataSrc_Orginal:
				sprintf(cInform_name, "OrgDB = %04ld", cloestObject.id);
				break;

			case kDataSrc_Planets:
				//*	figure out which planet it is
				short	planetIndex;
				planetIndex	=	-1;
				for (ii=0; ii<kPlanetObjectCnt; ii++)
				{
					if (cloestObject.id == cPlanets[ii].id)
					{
						planetIndex	=	ii;
						break;
					}
				}
				if (planetIndex >= 0)
				{
					sprintf(cInform_name, "Planet = %s", gPlanet_names[planetIndex]);
				}
				else
				{
					sprintf(cInform_name, "Planet");
				}
				break;

			case kDataSrc_Zodiac:
				//*	figure out which zodiac it is
				if (gZodiacPtr != NULL)
				{
					short	zodiacIndex;
					zodiacIndex	=	-1;
					for (ii=0; ii < kZodiacCount; ii++)
					{
						if (cloestObject.id == gZodiacPtr[ii].id)
						{
							zodiacIndex	=	ii;
							break;
						}
					}
					if (zodiacIndex >= 0)
					{
						sprintf(cInform_name, "Zodiac = %s", gZodiac_names[zodiacIndex]);
					}
					else
					{
						sprintf(cInform_name, "Zodiac");
					}
				}
				break;

			case kDataSrc_YaleBrightStar:
				sprintf(cInform_name, "Yale = %04ld mag=%4.2f", cloestObject.id, cloestObject.realMagnitude);
				if (cloestObject.parallax > 0.0)
				{
					//*	1 / parallax = # of parsecs
					//*	1 parsec = 3.26 light years

					distance	=	(1.0 / cloestObject.parallax) * 3.26;

					sprintf(moreInfo, " parallax = %5.4f distance=%4.2f light years", cloestObject.parallax, distance);
					strcat(cInform_name, moreInfo);

				}
				break;


			case kDataSrc_NGC2000:
			case kDataSrc_NGC2000IC:
				tmpString[0]	=	(cloestObject.type >> 24) & 0x7f;
				tmpString[1]	=	(cloestObject.type >> 16) & 0x7f;
				tmpString[2]	=	(cloestObject.type >> 8) & 0x7f;
				tmpString[3]	=	(cloestObject.type) & 0x7f;
				tmpString[4]	=	0;

				sprintf(cInform_name, "NGC = %04ld %s mag=%4.2f", cloestObject.id, tmpString, cloestObject.realMagnitude);

				foundMoreInfo	=	GetObjectDescription(&cloestObject, moreInfo, 128);
				if (foundMoreInfo)
				{
					strcat(cInform_name, " ");
					strcat(cInform_name, moreInfo);
				}
				break;

			case kDataSrc_Hipparcos:
				sprintf(cInform_name, "Hipparcos = %04ld mag=%4.2f", cloestObject.id, cloestObject.realMagnitude);
				if (cloestObject.parallax > 0.0)
				{
					//*	1 / parallax = # of parsecs
					//*	1 parsec = 3.26 light years

					distance	=	(1.0 / cloestObject.parallax) * 3.26;

					sprintf(moreInfo, " parallax = %5.4f distance=%4.2f light years", cloestObject.parallax, distance);
					strcat(cInform_name, moreInfo);

				}
				break;
		}
	}
}

//*****************************************************************************
//*	returns true if on current screen
void	WindowTabSkyTravel::SearchSkyObjects(char *objectName)
{
bool	foundSomething;
int		iii;
char	firstChar;
int		searchStrLen;
double	newRA;
double	newDec;
char	foundName[64];
char	database[32];
char	msgString[256];
int		objectIDnum;
char	*argPtr;
long	hippObjectId;

//	CONSOLE_DEBUG("-----------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Searching for", objectName);

	foundSomething	=	false;
	firstChar		=	toupper(objectName[0]);
	searchStrLen	=	strlen(objectName);
	strcpy(database, "");
	strcpy(foundName, "");
	//-------------------------------------------------------------------------------
	//*	check to see if they specified an NGC object
	if (strncasecmp(objectName, "NGC", 3) == 0)
	{
		if ((cNGCobjectPtr != NULL) && (cNGCobjectCount > 0))
		{
			argPtr	=	objectName;
			argPtr	+=	3;
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
			objectIDnum	=	atoi(argPtr);
			iii	=	0;
			while ((foundSomething == false) && (iii < cNGCobjectCount))
			{
				if ((objectIDnum == cNGCobjectPtr[iii].id) && (cNGCobjectPtr[iii].dataSrc == kDataSrc_NGC2000))
				{
					newRA	=	cNGCobjectPtr[iii].org_ra;
					newDec	=	cNGCobjectPtr[iii].org_decl;

					strcpy(database, "NGC");
					sprintf(foundName, "NGC-%d mag=%f2.1", objectIDnum, cNGCobjectPtr[iii].realMagnitude);;
					cDispOptions.dispNGC	=	true;
					foundSomething			=	true;
			//		DumpCelestDataStruct(__FUNCTION__, &cNGCobjectPtr[iii]);
				}
				iii++;
			}
		}
	}

	//-------------------------------------------------------------------------------
	//*	check to see if they specified an IC object
	if (strncasecmp(objectName, "IC", 2) == 0)
	{
		if ((cNGCobjectPtr != NULL) && (cNGCobjectCount > 0))
		{
			argPtr	=	objectName;
			argPtr	+=	2;
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
			objectIDnum	=	atoi(argPtr);
			iii	=	0;
			while ((foundSomething == false) && (iii < cNGCobjectCount))
			{
				if ((objectIDnum == cNGCobjectPtr[iii].id) && (cNGCobjectPtr[iii].dataSrc == kDataSrc_NGC2000IC))
				{
					newRA	=	cNGCobjectPtr[iii].org_ra;
					newDec	=	cNGCobjectPtr[iii].org_decl;

					strcpy(database, "IC");
					sprintf(foundName, "IC-%d mag=%f2.1", objectIDnum, cNGCobjectPtr[iii].realMagnitude);;
					cDispOptions.dispNGC	=	true;
					foundSomething			=	true;
			//		DumpCelestDataStruct(__FUNCTION__, &cNGCobjectPtr[iii]);
				}
				iii++;
			}
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for planets
	iii	=	0;
	while ((foundSomething == false) && (iii < 10))
	{
		if (strncasecmp(objectName, gPlanet_names[iii], searchStrLen) == 0)
		{
//			CONSOLE_DEBUG("found in planets");
			newRA	=	cPlanets[iii].ra;
			newDec	=	cPlanets[iii].decl;

			strcpy(database, "Planets");
			strcpy(foundName, gPlanet_names[iii]);;

			foundSomething	=	true;
		}
		iii++;
	}

	//-------------------------------------------------------------------------------
	//*	look for messier objects
	if ((foundSomething == false) && (cMessierOjbectPtr != NULL) && (cMessierOjbectCount > 0))
	{
		if ((firstChar == 'M') && isdigit(objectName[1]))
		{
			//*	ok, lets look
			iii	=	0;
			while ((foundSomething == false) && (iii < cMessierOjbectCount))
			{
                if (strcasecmp(objectName, cMessierOjbectPtr[iii].shortName) == 0)
                {
					CONSOLE_DEBUG("found in messier");
					newRA	=	cMessierOjbectPtr[iii].org_ra;
					newDec	=	cMessierOjbectPtr[iii].org_decl;

					strcpy(database, "Messier catalog");
					strcpy(foundName, cMessierOjbectPtr[iii].shortName);;

					cDispOptions.dispMessier	=	true;
					foundSomething				=	true;
                }
                iii++;
			}
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for Hipparcos numbers
	if ((foundSomething == false) && (cHipObjectPtr != NULL) && (cHipObjectCount > 0))
	{
		if ((firstChar == 'H') && isdigit(objectName[1]))
		{
			hippObjectId	=	atol(&objectName[1]);
			CONSOLE_DEBUG_W_LONG("Searching Hipparcosfor ID\t=", hippObjectId);
			//*	ok, lets look
			iii	=	0;
			while ((foundSomething == false) && (iii < cHipObjectCount))
			{
                if (hippObjectId == cHipObjectPtr[iii].id)
                {
					CONSOLE_DEBUG("found in Hipparcos");
				//	newRA	=	cHipObjectPtr[iii].org_ra;
				//	newDec	=	cHipObjectPtr[iii].org_decl;

					newRA	=	cHipObjectPtr[iii].ra;
					newDec	=	cHipObjectPtr[iii].decl;

					strcpy(database, "Hipparcos catalog");
					sprintf(foundName, "H%ld", cHipObjectPtr[iii].id);
					if (cHipObjectPtr[iii].longName[0] > 0x20)
					{
						strcat(foundName, "-");
						strcat(foundName, cHipObjectPtr[iii].longName);
					}

					cDispOptions.dispHIP	=	true;

					if (cView_index > 3)
					{
						SetView_Index(3);
					}
					foundSomething			=	true;
                }
                iii++;
			}
		}
	}


	//-------------------------------------------------------------------------------
	if ((foundSomething == false) && cDispOptions.dispConstOutlines  &&
		(cConstOutlinePtr != NULL) && (cConstOutlineCount > 0))
	{
		//*	lets look through all of the long names first
		iii	=	0;
		while ((foundSomething == false) && (iii < cConstOutlineCount))
		{
			if (strncasecmp(objectName, cConstOutlinePtr[iii].longName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG("found in Constellation outlines");

				newRA	=	cConstOutlinePtr[iii].rtAscension;
				newDec	=	cConstOutlinePtr[iii].declination;
				strcpy(database, "Constellation outlines");
				strcpy(foundName, cConstOutlinePtr[iii].longName);
				foundSomething	=	true;
			}
			iii++;
		}
		//*	now if we didnt find anything, look at the short names
		iii	=	0;
		while ((foundSomething == false) && (iii < cConstOutlineCount))
		{
			if (strncasecmp(objectName, cConstOutlinePtr[iii].shortName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG_W_STR("Found", cConstOutlinePtr[iii].shortName);
				newRA	=	cConstOutlinePtr[iii].rtAscension;
				newDec	=	cConstOutlinePtr[iii].declination;
				strcpy(database, "Constellation outlines");
				strcpy(foundName, cConstOutlinePtr[iii].shortName);
				foundSomething	=	true;
			}
			iii++;
		}

		//*	outlines dont get displayed unless we are greater than 4
		if (foundSomething && (cView_index < 5))
		{
			SetView_Index(5);
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for for common star names in the Hipparcos list
	if ((foundSomething == false) && (cHipObjectPtr != NULL) && (cHipObjectCount > 0))
	{
		CONSOLE_DEBUG_W_STR("Searching Hipparcosfor \t=", objectName);

		iii	=	0;
		while ((foundSomething == false) && (iii < cHipObjectCount))
		{
			if (strncasecmp(objectName, cHipObjectPtr[iii].longName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG_W_STR("Found", cHipObjectPtr[iii].longName);
				newRA	=	cHipObjectPtr[iii].ra;
				newDec	=	cHipObjectPtr[iii].decl;

				strcpy(database, "Hipparcos catalog");
				sprintf(foundName, "H%ld-%s", cHipObjectPtr[iii].id, cHipObjectPtr[iii].longName);
				foundSomething	=	true;
			}
			iii++;
		}
	}

	//-------------------------------------------------------------------------------
	//*	check to see if they specified an Henry Draper lists
	if (strncasecmp(objectName, "HD", 2) == 0)
	{
		CONSOLE_DEBUG("looking for henry draper objects");
		if ((cDraperObjectPtr != NULL) && (cDraperObjectCount > 0))
		{
			argPtr	=	objectName;
			argPtr	+=	2;
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
			objectIDnum	=	atoi(argPtr);
			iii			=	0;
			CONSOLE_DEBUG_W_NUM("looking for HD", objectIDnum);
			while ((foundSomething == false) && (iii < cDraperObjectCount))
			{
				if ((objectIDnum == cDraperObjectPtr[iii].id))
				{
					newRA	=	cDraperObjectPtr[iii].org_ra;
					newDec	=	cDraperObjectPtr[iii].org_decl;

					strcpy(database, "HD");
					sprintf(foundName, "HD-%d mag=%f2.1", objectIDnum, cDraperObjectPtr[iii].realMagnitude);;
					cDispOptions.dispNGC	=	true;
					foundSomething			=	true;
			//		DumpCelestDataStruct(__FUNCTION__, &cDraperObjectPtr[iii]);
				}
				iii++;
			}
		}
	}


#ifdef _ENABLE_HYG_
	//-------------------------------------------------------------------------------
	//*	check to see if they specified an Henry Draper lists
	if (strncasecmp(objectName, "HD", 2) == 0)
	{
		CONSOLE_DEBUG("looking for henry draper objects")
		if ((cHYGObjectPtr != NULL) && (cHYGObjectCount > 0))
		{
			argPtr	=	objectName;
			argPtr	+=	2;
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
			objectIDnum	=	atoi(argPtr);
			iii			=	0;
			while ((foundSomething == false) && (iii < cHYGObjectCount))
			{
			//	if ((objectIDnum == cHYGObjectPtr[iii].id) && (cHYGObjectPtr[iii].dataSrc == kDataSrc_NGC2000IC))
				if ((objectIDnum == cHYGObjectPtr[iii].id))
				{
					newRA	=	cHYGObjectPtr[iii].org_ra;
					newDec	=	cHYGObjectPtr[iii].org_decl;

					strcpy(database, "IC");
					sprintf(foundName, "IC-%d mag=%f2.1", objectIDnum, cHYGObjectPtr[iii].realMagnitude);;
					cDispOptions.dispNGC	=	true;
					foundSomething			=	true;
			//		DumpCelestDataStruct(__FUNCTION__, &cHYGObjectPtr[iii]);
				}
				iii++;
			}
		}
	}
#endif // _ENABLE_HYG_


	if (foundSomething)
	{
	char	raDecString[64];

		if (newRA < 0.0)
		{
			newRA	+=	(2 * M_PI);
		}

		FromatRa_Dec_toString(newRA, newDec, raDecString);
		sprintf(msgString, "Found %s in %s at %s", foundName, database, raDecString);

		CONSOLE_DEBUG_W_STR("Found      \t",	foundName);
		CONSOLE_DEBUG_W_DBL("newRA(deg) \t=",	DEGREES(newRA / 15.0));
		CONSOLE_DEBUG_W_DBL("newDec(deg)\t=",	DEGREES(newDec));
		CONSOLE_DEBUG_W_STR("located at \t=",	raDecString);

		cRa0		=	newRA;
		cDecl0		=	newDec;
		cFindFlag	=	true;
		ForceReDrawSky();
	}
	else
	{
		sprintf(msgString, "Nothing Found for %s", objectName);
	}
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(128,	128, 128));
	SetWidgetText(		kSkyTravel_MsgTextBox, msgString);
}


//*****************************************************************************
//*	returns true if on current screen
bool	WindowTabSkyTravel::GetXYfromRA_Decl(double argRA_radians, double argDecl_radians, short *xx, short *yy)
{
int					xcoord;
int					ycoord;
double				temp;
double				angle;
double				alpha;
double				aside;
double				cside;
double				gamma;
double				sin_bside;
double				cos_bside;
double				xangle;
double				yangle;
double				rangle;
bool				inCurrentWindow;

//	CONSOLE_DEBUG_W_DBL("argRA_radians\t=",		argRA_radians);
//	CONSOLE_DEBUG_W_DBL("argDecl_radians\t=",	argDecl_radians);

//	CONSOLE_DEBUG_W_DBL("RA \t\t=",		DEGREES(argRA_radians));
//	CONSOLE_DEBUG_W_DBL("DEC\t\t=",		DEGREES(argDecl_radians));

//+++
//	CalanendarTime(&cCurrentTime);
//	Local_Time(&cCurrentTime);		//* compute local time from gmt and timezone
//	ConvertLatLonToRaDec(&cCurrLatLon, &cCurrentTime);
//+++


	inCurrentWindow	=	false;
	xcoord			=	0.0;
	ycoord			=	0.0;
	xangle			=	cView_angle / 2.0;
	yangle			=	xangle * cWind_height / cWind_width;
	rangle			=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
	cRadmax			=	rangle;

	sin_bside		=	sin(kHALFPI - cDecl0);
	cos_bside		=	cos(kHALFPI - cDecl0);

	cDecmax			=	cDecl0 + rangle;
	if (cDecmax > kHALFPI)
	{
		cDecmax		=	(kHALFPI - kEPSILON);	//* clip at 90 degrees
	}
	cDecmin			=	cDecl0 - rangle;
	if (cDecmin < -kHALFPI)
	{
		cDecmin		=	-(kHALFPI - kEPSILON);	//* clip at -90 degrees
	}

	cRamax	=	0.0;	//*default ramax=0.
	temp	=	kHALFPI - fabs(cDecl0);
	if (temp > cRadmax)
	{
		cRamax	=	asin(sin(cRadmax) / sin(temp));
	}
	cWind_x0	=	wind_ulx + (cWind_width / 2);
	cWind_y0	=	wind_uly + (cWind_height / 2);

	cXfactor	=	cWind_width / cView_angle;
	cYfactor	=	cXfactor;	//* 1:1 aspect ratio


	alpha	=	cRa0 - argRA_radians;
	if (alpha > PI)
	{
		alpha	-=	kTWOPI;
	}
	else if (alpha < -PI)
	{
		alpha	+=	kTWOPI;
	}
	if ((cRamax == 0.0) || (fabs(alpha) <= cRamax))		//* in bounds for ra?
	{
		cside	=	kHALFPI - argDecl_radians;

		//* here we use in-line code for sphsas and sphsss because bside is constant
		//* so we avoid repeated invocations of sin(bside) and cos(bside)

		aside	=	acos((cos_bside * cos(cside)) + (sin_bside * sin(cside) * cos(alpha)));
		if (aside < cRadmax)	//* within bounding circle?
		{
			if (aside>kEPSILON)
			{
				gamma	=	asin(sin(cside) * sin(alpha) / sin(aside));
			}
			else
			{
				gamma	=	0.0;
			}
			if (cos(cside) < (cos_bside * cos(aside)))
			{
				gamma	=	PI - gamma;	//* supplement gamma if cos(c) < cos(b) * cos(a)
			}
			angle	=	gamma + cGamang;

			//*	compute x and y coordinates
			//* x	=	x0 + cXfactor * aside * cos(angle)
			//* y	=	y0 - cYfactor * aside * sin(angle) (minus sign is because plus y is down)

			xcoord	=	cWind_x0 + (cXfactor * aside * sin(angle));
			ycoord	=	cWind_y0 - (cYfactor * aside * cos(angle));

			//* are they both within window?
			if ((xcoord >= wind_ulx) && (xcoord <= wind_ulx + cWind_width))
			{
				if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
				{
					inCurrentWindow	=	true;
				}
			}
		}
		*xx	=	xcoord;
		*yy	=	ycoord;
	}
	else
	{
		*xx	=	-1;
		*yy	=	-1;
	}
	return(inCurrentWindow);
}

//*****************************************************************************
//* draw picture of feet
void	WindowTabSkyTravel::DrawFeet(void)
{
int		yy;
double	yangle;

//*	SPECIAL GRAPHICS FOR FEET

char feet_shape[]=
	{
	"\xDF\x69\xAF\xC9\xA2\xCA\xA2\xC6\xA2"
	"\xC6\xA2\xC6\xA2\xC6\xA2\xC6\x1A"
	"\x1A\x1F\x1F\x1F\x1A\x46\x62\x46"
	"\x62\x46\x62\x46\x62\x48\x62\x43"
	"\x62\x44\x6F\x61\xA9\xCF\xCF\xCF"
	"\xC1\x92\x4F\x4F\x4F\x4C\x61\x4C\x21\x4A"
	"\x61\x47\x64\x87\xA8\xC1\xA2\xC1"
	"\x41\xCC\xE1\xC4\xE1\xC8\x22\x48"
	"\x25\x01\x46\x62\xA3\x14\x07\x14"
	"\x63\xA2\xC6\x42\x61\x81\x61\x81"
	"\x61\x05\xA2\x81\xA2\x87\xC2\x02"
	"\x21\x02\x21\x0B\x21\x61\x86\xA2"
	"\x41\xA1\xC1\x02\x12\x02\xE4\xC6"
	"\xDC\x15\x4F\x42\x61\x4B\x21\x47"
	"\x24\x07\xE8\xC1\xE2\xC1\xE1\xCD"
	"\xA1\xC4\xA1\xCC\x81\x61\x4B\x65"
	"\x46\x22\xE3\x94\x8A\x93\x41\x22"
	"\xE2\x25\x42\x24\xE2\xD6\xA7\x83"
	"\xA1\x86\xA1\x21\x06\x45\xC6\xA4"
	"\x7E\x00"
	};

	yangle	=	cElev0 - (-kHALFPI);
	yy		=	cWind_y0 + (yangle * cYfactor);
	if ((yy >= wind_uly) && (yy <= wind_uly + cWind_height))
	{
		DrawVector(	BROWN,
					cWorkSpaceLeftOffset + cWind_x0,
					cWorkSpaceTopOffset + yy,
					1,
					feet_shape);
	}
}



#ifdef _DISPLAY_MAP_TOKENS_

#include	"MAPTOK.DTA"


//******************************************************
//* map_tokens - draw map tokens on horizon, if present
void	WindowTabSkyTravel::MapTokens(TYPE_Time *timeptr, TYPE_LatLon *locptr)
{
int				xcoord,ycoord;
int				ii;
TYPE_SpherTrig	sphptr;

	for (ii=0; ii<kNMAPOBJS; ii++)
	{
	//	if (fabs(mapptr[ii].map_lat-locptr->latitude) > mapptr[ii].map_tol)		continue;	//* not close enough
	//	if (fabs(mapptr[ii].map_long-locptr->longitude) > mapptr[ii].map_tol)	continue;	//* not close enough
	//	if (timeptr->fJulianDay < mapptr[ii].jd_begin)							continue;	//* too early
	//	if (timeptr->fJulianDay > mapptr[ii].jd_end)							continue;	//* too late
	//	if (fabs(mapptr[ii].map_az - cAz0) > cView_angle)						continue;	//* outside the azimuth range

		sphptr.bside	=	kHALFPI - cElev0;
		sphptr.cside	=	kHALFPI - kEPSILON;
		sphptr.alpha	=	cAz0 - mapptr[ii].map_az;
		if (sphptr.alpha>PI)
		{
			sphptr.alpha	-=	kTWOPI;
		}
		else if (sphptr.alpha<-PI)
		{
			sphptr.alpha	+=	kTWOPI;
		}
		sphsas(&sphptr);
		sphsss(&sphptr);
		xcoord	=	cWind_x0 + (cXfactor * sphptr.aside * sin(sphptr.gamma));
		if ((xcoord >= wind_ulx) && (xcoord <= wind_ulx + cWind_width))
		{
			ycoord	=	cWind_y0 - (cYfactor * sphptr.aside * cos(sphptr.gamma));
			if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
			{
				DrawVector(	WHITE,
							cWorkSpaceLeftOffset + xcoord,
							cWorkSpaceTopOffset + ycoord,
							8 - cView_index,
							map_shape[ii]);	//* draw the object
			}
		}
	}
}
#endif // _DISPLAY_MAP_TOKENS_


#endif // _ENABLE_SKYTRAVEL_
