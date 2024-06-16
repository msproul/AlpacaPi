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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 28,	2020	<MLS> Re-discovered skytravel source code on an old computer
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
//*	Apr 10,	2021	<MLS> Added support for AAVSO Target Tool Alerts
//*	Jun 26,	2021	<MLS> Fixed bug in old style constellation drawing, only showed up in 32 bit
//*	Jul  6,	2021	<MLS> Added forceNumber option to DrawGreatCircle()
//*	Jul  6,	2021	<MLS> Added South Pole circles
//*	Aug  9,	2021	<MLS> Added ProcessMouseWheelMoved(), adjusts view level
//*	Aug  9,	2021	<MLS> Updated Search_and_plot() to handle unsorted data
//*	Aug 10,	2021	<MLS> Changed most of the cView_index checks to cView_angle_Rads
//*	Aug 10,	2021	<MLS> Added Center_RA_DEC()
//*	Aug 11,	2021	<MLS> Added Center_CelestralObject()
//*	Aug 12,	2021	<MLS> Added SetAAVSOdisplayFlag()
//*	Sep  1,	2021	<MLS> Changed '<' / '>' to be +/- for changing hour
//*	Sep  1,	2021	<MLS> Changed '[' / ']' to be +/- for changing day
//*	Sep  9,	2021	<MLS> Changed '{' / '}' to be +/- for changing month
//*	Sep 12,	2021	<MLS> Added variable line widths of grids, constellations / outlines
//*	Oct 23,	2021	<MLS> Added display of real magnitude of the star
//*	Oct 23,	2021	<MLS> Added ProcessSingleCharCmd()
//*	Oct 24,	2021	<MLS> Added DrawStarFancy()
//*	Oct 24,	2021	<MLS> Stars now colored based on spectral class O,B,A,F,G,K,M
//*	Oct 24,	2021	<MLS> Night mode now works nicely
//*	Oct 24,	2021	<MLS> Added "$" command for original database toggle
//*	Oct 25,	2021	<MLS> Added ZoomViewAngle()
//*	Oct 25,	2021	<MLS> Trying to eliminate all references to cView_index
//*	Oct 26,	2021	<MLS> Added SetMinimumViewAngle() & SetMaximumViewAngle()
//*	Oct 26,	2021	<MLS> Deleted SetView_Index()
//*	Oct 28,	2021	<MLS> Added '(' / ')' for plus minus Year, '0' / '9 for minute
//*	Oct 28,	2021	<MLS> Verified that eclipse stuff still works
//*	Oct 30,	2021	<MLS> Moved moon drawing code to separate routine: DrawMoon()
//*	Nov  1,	2021	<MLS> All dependencies on cView_index resolved, used for display only
//*	Nov  9,	2021	<MLS> Working on Gaia star catalog support
//*	Nov 10,	2021	<MLS> Added _ENABLE_GAIA_
//*	Nov 13,	2021	<MLS> Added support for getting remote images from SDSS
//*	Nov 13,	2021	<MLS> Added DrawScale(), "scale of miles" indicator, chart mode only
//*	Dec 18,	2021	<MLS> Added SearchSkyObjectsDataListByNumber()
//*	Dec 19,	2021	<MLS> Added SearchSkyObjectsDataListByShortName()
//*	Dec 19,	2021	<MLS> Added SearchSkyObjectsDataListByLongName()
//*	Dec 19,	2021	<MLS> Added SearchSkyObjectsConstellations()
//*	Dec 19,	2021	<MLS> Added SearchSkyObjectsConstOutlines()
//*	Dec 29,	2021	<MLS> Added DrawAsteroids()
//*	Dec 30,	2021	<MLS> Added DrawStarFancy_Label()
//*	Dec 30,	2021	<MLS> Added DrawAsteroidFancy()
//*	Dec 31,	2021	<MLS> Added SearchAsteroids()
//*	Jan  9,	2022	<MLS> Changed minimum view angle to 0.01 degrees
//*	Jan 16,	2022	<MLS> Sorting of Gaia remote data made big difference in zoom in/out speed
//*	Jan 18,	2022	<MLS> Changed star drawing order so the dense catalogs are on the bottom
//*	Jan 22,	2022	<MLS> Fixed bug when drawing stars with negative magnitudes
//*	Jan 23,	2022	<MLS> Added user settable parameters for star size calculations (cFaintLimit_B)
//*	Jan 23,	2022	<MLS> Control and mouse wheel adjust Faint Limit (cFaintLimit_B)
//*	Feb  3,	2022	<MLS> Added support for OpenNGC catalog
//*	Feb  4,	2022	<MLS> Added DrawOpenNGC_Outlines()
//*	Feb 18,	2022	<MLS> SkyTravel working with OpenCV-C++
//*	Apr  7,	2022	<MLS> ProcessMouseEvent() ignores nearest object when mouse drag in progress
//*	Apr 30,	2022	<MLS> Added Messier objects to FindObjectNearCursor()
//*	May 26,	2022	<MLS> Removed support for reading GAIA data directly
//*	Jun 15,	2022	<MLS> Added camera name to telescope display if enough room
//*	Jun 21,	2022	<MLS> Changed logic for clock update so it happens at beginning of the second
//*	Jul 21,	2022	<MLS> Added _USE_LARGE_FONT_FOR_PUBLICATION_ for GAIA paper
//*	Aug  2,	2022	<MLS> Added SAO star catalog
//*	Sep  3,	2022	<MLS> Fixed bug in SearchSkyObjectsDataListByNumber() that made HIP search not work
//*	Sep  3,	2022	<MLS> Added SAO star catalog to search routine
//*	Oct  2,	2022	<MLS> Cleaned up button setting code in SetupWindowControls()
//*	Oct  2,	2022	<MLS> Added which SQL database in use to main display
//*	Oct  5,	2022	<MLS> Added HandleSpecialKeys() to skytravel window
//*	Mar 22,	2023	<MLS> Added ConvertAzEl_to_RaDec()
//*	May 27,	2023	<MLS> Added parallax to label display
//*	Jun 18,	2023	<MLS> Fixed bug in SearchSkyObjectsDataListByNumber for non-numeric values
//*	Jul 21,	2023	<MLS> Added AAVSO alerts to FindObjectNearCursor()
//*	Jul 23,	2023	<MLS> Fixed startup color bug by calling Set_Skycolor() from constructor
//*	Aug 24,	2023	<MLS> Added ellipse drawing for OpenNGC objects major/minor axis
//*	Aug 25,	2023	<MLS> Ellipse angle working properly
//*	May 29,	2024	<MLS> Added DrawOutlines()
//*	May 29,	2024	<MLS> Added support for MilkyWay outlines
//*	May 31,	2024	<MLS> Added filled polygons to Milkyway display options
//*	May 31,	2024	<MLS> ';' toggles Milkyway filled polygons, not perfect
//*	Jun  1,	2024	<MLS> MilkyWay is now outlined in white
//*	Jun  9,	2024	<MLS> Added planet size drawing in PlotSkyObjects()
//*	Jun 11,	2024	<MLS> Changed cAutoAdvanceTime to global, gAutoAdvanceTime
//*****************************************************************************
//*	TODO
//*			star catalog lists
//*
//*	might be useful
//*		http://www.astrosurf.com/jephem/astro/skymap/sm200coordinates_en.htm
//*		https://github.com/dcf21/constellation-stick-figures
//*****************************************************************************

#ifndef _ENABLE_SKYTRAVEL_
	#define _ENABLE_SKYTRAVEL_
#endif // _ENABLE_SKYTRAVEL_

#ifdef _ENABLE_SKYTRAVEL_


//#define	_USE_LARGE_FONT_FOR_PUBLICATION_

//#define	_DISPLAY_MAP_TOKENS_
//#define	_ENBABLE_WHITE_CHART_

#include	<sys/time.h>
#include	<unistd.h>
#include	<math.h>

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"observatory_settings.h"
#include	"helper_functions.h"
#include	"controller_startup.h"
#include	"julianTime.h"


#include	"SkyStruc.h"
#include	"StarData.h"
#include	"SAO_stardata.h"
#include	"SkyTravelConstants.h"
#include	"SkyTravelTimeRoutines.h"
#include	"NGCcatalog.h"
#include	"OpenNGC.h"
#include	"YaleStarCatalog.h"
#include	"HipparcosCatalog.h"
#include	"SkyTravelExternal.h"
#include	"sidereal.h"
#include	"aavso_data.h"
#include	"RemoteImage.h"
#include	"eph.h"
#include	"milkyway.h"

#include	"windowtab.h"
#include	"windowtab_skytravel.h"
#include	"controller_skytravel.h"
#include	"polaralign.h"

#include	"AsteroidData.h"
#include	"OpenNGC.h"

#ifdef _ENABLE_REMOTE_GAIA_
	#include	"RemoteGaia.h"
	#include	"GaiaSQL.h"
#endif


#include	"SHAPES.DTA"
#include	"DEEP.DTA"
#include	"CONSTEL.DTA"




//#define	kMinInformDist	10
#define	kMinInformDist	50

#define	TRUE	true
#define	FALSE	false


//#define	DEGREES(radians)	((radians) * 180.0 / M_PI)


//*	for the moment, these are hard coded
double	gDomeDiameter_inches		=	(15.0 * 12.0);
double	gSlitWidth_inches			=	41.0;
double	gSlitBottom_degrees			=	25.0;
double	gSlitTop_degrees			=	100.0;
double	gDomeAzimuth_degrees		=	90.0;
bool	gAutoAdvanceTime			=	true;


SkyTravelDispOptions	gST_DispOptions;

//static double	GetRA_DEC_delta(int viewIndex);
//static double	GetRA_DEC_delta(int viewIndex, double viewAngle);
static double	GetRA_DEC_delta(const double viewAngle);

#define	kMinViewAngle		0.003

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



//*	this number was derived by Clif Ashcraft, Oct 25, 2021 for the purposes of zoom increment
#define	kViewAngleMultipler	(1.18189148050459)


#define	kMaxViewAngleIndex		RADIANS(800)
#define	kMinViewAngle_Degrees	(0.002777778)		//*	10 arc seconds
#define	kMaxViewAngle_Degrees	(800)

static	double	gAAVSO_maxViewAngle	=	3.0;
//#define	kAAVSO_ViewAngle			36

static	WindowTabSkyTravel	*gSkyTravelWindow	=	NULL;

//#define		kView_old3				12
//#define		kView_old4				14
//#define		kView_old5				18

//#define		kView_Default			34		//*	old 8
#define		kViewAngle_Default		RADIANS(160.0)


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


//*****************************************************************************
#define		CHECK_FOR_INIT(x, y)		\
	if (x != NULL)						\
	{									\
		CONSOLE_ABORT("X is not NULL");	\
	}									\
	if (y != 0)							\
	{									\
		CONSOLE_ABORT("Y is not zero");	\
	}									\


//**************************************************************************************
WindowTabSkyTravel::WindowTabSkyTravel(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;
int		startupWidgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("RemoteGAIAenabled is", (gST_DispOptions.RemoteGAIAenabled ? "enabled" : "disabled"));

	startupWidgetIdx	=	SetStartupText("Remote SQL Data:");
	SetStartupTextStatus(startupWidgetIdx, (gST_DispOptions.RemoteGAIAenabled ? "Enabled" : "Disabled"));

//#if defined(_USE_OPENCV_CPP_) &&  (CV_MAJOR_VERSION < 4)
//	CONSOLE_DEBUG_W_SIZE("sizeof(CvScalar)  \t=",	sizeof(CvScalar));
//	CONSOLE_DEBUG_W_SIZE("sizeof(cv::Scalar)\t=",	sizeof(cv::Scalar));
//
//	CONSOLE_DEBUG_W_SIZE("sizeof(CvFont)  \t=",		sizeof(CvFont));
//
//	CONSOLE_DEBUG_W_SIZE("sizeof(CvRect)  \t=",		sizeof(CvRect));
//	CONSOLE_DEBUG_W_SIZE("sizeof(cv::rect)\t=",		sizeof(cv::Rect));
//
//	CONSOLE_DEBUG_W_SIZE("sizeof(CvPoint) \t=",		sizeof(CvPoint));
//	CONSOLE_DEBUG_W_SIZE("sizeof(cv::point)\t=",	sizeof(cv::Point));
//
////	CONSOLE_ABORT(__FUNCTION__);
//#endif // defined

	gSkyTravelWindow		=	this;
	cDebugCounter			=	0;
	cWorkSpaceTopOffset		=	0;
	cWorkSpaceLeftOffset	=	0;


	//*	zero out everything
	memset(&gCurrentSkyTime,	0, sizeof(TYPE_SkyTime));
	memset(&cCurrLatLon,		0, sizeof(TYPE_LatLon));

	gAutoAdvanceTime			=	true;
	cLastUpdateTime_ms			=	0;
	cLastRedrawTime_ms			=	0;
	cLastRemoteImageUpdate_ms	=	0;
	cNightMode					=	false;
	gStarDataPtr				=	NULL;
	gStarCount					=	0;

	//*	for computing size of stars to draw
	cFaintLimit_B				=	4.5224;
	cMaxRadius_D				=	20;


	CHECK_FOR_INIT(gConstStarPtr,		gConstStarCount);
	CHECK_FOR_INIT(gNGCobjectPtr,		gNGCobjectCount);
	CHECK_FOR_INIT(gHipObjectPtr,		gHipObjectCount);
	CHECK_FOR_INIT(gYaleStarDataPtr,	gYaleStarCount);
	CHECK_FOR_INIT(gAAVSOalertsPtr,		gAAVSOalertsCnt);
	CHECK_FOR_INIT(gMessierObjectPtr,	gMessierObjectCount);
	CHECK_FOR_INIT(gDraperObjectPtr,	gDraperObjectCount);
	CHECK_FOR_INIT(gSpecialObjectPtr,	gSpecialObjectCount);

	cCameraFOVarrayPtr		=	NULL;

#ifdef _ENABLE_HYG_
	gHYGObjectPtr			=	NULL;
	gHYGObjectCount			=	0;
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
		cCurrLatLon.latitude		=	RADIANS(gObseratorySettings.Latitude_deg);
		cCurrLatLon.longitude		=	RADIANS(gObseratorySettings.Longitude_deg);
	}
	else
	{
		//*	we need to have a default of someplace
		cCurrLatLon.latitude		=	RADIANS(41.361090);
		cCurrLatLon.longitude		=	RADIANS(-74.980333);
	}

	Compute_Timezone(&cCurrLatLon, &gCurrentSkyTime);
	SetCurrentTime();		//* default system Greenwich date/time

	gCurrentSkyTime.strflag				=	true;		//* say 2000 data present
	gCurrentSkyTime.starDataModified	=	false;
	gCurrentSkyTime.calflag				=	0;			//* auto calendar
	gCurrentSkyTime.precflag			=	false;
	gCurrentSkyTime.negflag				=	false;
	gCurrentSkyTime.local_time_flag		=	true;		//* default use local time
	gCurrentSkyTime.timeOfLastPrec		=	JD2000;		//* set last prec = Julian day 2000
	gCurrentSkyTime.delprc				=	DELPRC0;	//* set delta prec to default
	cCurrentSkyColor					=	W_BLUE;
//	cCurrentSkyColor					=	W_BLACK;
	cChartMode							=	false;

	CalanendarTime(&gCurrentSkyTime);

	memset(&cDispOptions,		0, sizeof(TYPE_SkyDispOptions));
	SetWebHelpURLstring("skytravel.html");		//*	set the web help url string

	gST_DispOptions.DisplayedMagnitudeLimit		=	15.0;
	gST_DispOptions.EarthDispMode				=	0;
	gST_DispOptions.EarthDispMode				=	0;
	gST_DispOptions.DashedLines					=	false;
	gST_DispOptions.DayNightSkyColor			=	false;
	gST_DispOptions.LineWidth_Constellations	=	1;
	gST_DispOptions.LineWidth_ConstOutlines		=	1;
	gST_DispOptions.LineWidth_GridLines			=	1;
	gST_DispOptions.LineWidth_NGCoutlines		=	1;
	gST_DispOptions.LineWidth_MilkyWay			=	1;

	cElev0			=	kHALFPI / 2;		//* 45 degrees
	cAz0			=	-PI / 3.0;			//* 60 degrees (ca. eastnortheast)
	cRa0			=	0.0;
	cDecl0			=	0.0;
	cCsrx			=	cWind_x0;	//* center of field
	cCsry			=	cWind_y0;
	wind_ulx		=	0;
	wind_uly		=	0;
	cWind_width		=	xSize;
	cWind_height	=	ySize - 75;


	gStarDataPtr	=	ReadDefaultStarData(&gStarCount, &gCurrentSkyTime);
//	CONSOLE_DEBUG_W_LONG("gStarCount\t=", gStarCount);

	for (iii=0; iii < kPlanetObjectCnt; iii++)
	{
		memset(&cPlanets[iii], 0, sizeof(TYPE_CelestData));

		cPlanets[iii].dataSrc			=	kDataSrc_Planets;
		cPlanets[iii].magn				=	0x00db + iii;
		cPlanets[iii].id				=	0x076c + iii;	//*fill in the id field

		strcpy(cPlanets[iii].shortName,  gPlanet_names[iii]);
//		CONSOLE_DEBUG_W_STR("cPlanets[iii].shortName\t=", cPlanets[iii].shortName);


		memset(&cPlanetStruct[iii], 0, sizeof(planet_struct));

		cPlanetStruct[iii].delta_dte	=	gDeltaPlanet[iii];	//*interpolation deltas
		cPlanetStruct[iii].dte0			=	1.e20;				//*impossible dte0 so that eph does a full compute
	}

	if (gZodiacPtr == NULL)
	{
		gZodiacPtr	=	(TYPE_CelestData *)calloc(kZodiacCount, sizeof(TYPE_CelestData));

		for (iii=0; iii < kZodiacCount; iii++)
		{
			gZodiacPtr[iii].dataSrc		=	kDataSrc_Zodiac;
			gZodiacPtr[iii].ra			=	kTWOPI*((float)iii / 12.);	//* 12 evenly spaced ra's
			gZodiacPtr[iii].decl		=	ECLIPTIC_ANGLE * sin(kTWOPI * (float)iii / 12.);	//* 23.5 degrees for ecliptic tilt
		//*	gZodiacPtr[iii].magn		=	0x00e6 + iii;
			gZodiacPtr[iii].id			=	0x0777 + iii;
		}
	}

	BuildConstellationData();
	//------------------------------------------------------------------
	gMilkyWay_outlines	=	Milkyway_ReadOutlines("d3-celestial/data/milkyway.json", &gMilkyWay_outlineCnt);
//	CONSOLE_DEBUG_W_LONG("gMilkyWay_outlineCnt\t=", gMilkyWay_outlineCnt);

	//------------------------------------------------------------------
	//*	try the OpenNGC catalog first
	gNGCobjectPtr	=	Read_OpenNGC_StarCatalog(&gNGCobjectCount);
	if (gNGCobjectPtr == NULL)
	{
		gNGCobjectPtr	=	ReadNGCStarCatalog(&gNGCobjectCount);
	}
//	CONSOLE_DEBUG_W_LONG("gNGCobjectCount\t=",	gNGCobjectCount);
	gOpenNGC_outlines	=	Read_OpenNGC_Outline_catgen(&gOpenNGC_outlineCnt);

	//------------------------------------------------------------------
	gYaleStarDataPtr	=	ReadYaleStarCatalog(&gYaleStarCount);

	//------------------------------------------------------------------
	gMessierObjectPtr	=	ReadMessierData(		"skytravel_data/",	kDataSrc_Messier,	&gMessierObjectCount);

	//------------------------------------------------------------------
	gDraperObjectPtr	=	ReadHenryDraperCatalog(	"skytravel_data/",	kDataSrc_Draper,	&gDraperObjectCount);


#ifdef _ENABLE_HYG_
	//------------------------------------------------------------------
	gHYGObjectPtr		=	ReadHYGdata(			"skytravel_data/",	kDataSrc_HYG,		&gHYGObjectCount);
#endif // _ENABLE_HYG_

	//------------------------------------------------------------------
	gConstOutlinePtr	=	ReadConstellationOutlines("skytravel_data/constOutlines.txt", &gConstOutlineCount);

	//------------------------------------------------------------------
	gHipObjectPtr		=	ReadHipparcosStarCatalog(&gHipObjectCount);
	if (gHipObjectPtr != NULL)
	{
		ReadCommonStarNames(gHipObjectPtr, gHipObjectCount);
	}

	//------------------------------------------------------------------
	//*	read the SAO database
	gSAOobjectPtr		=	SAO_ReadFile(&gSAOobjectCount);
//	CONSOLE_DEBUG_W_HEX("gSAOobjectPtr\t=",		gSAOobjectPtr);
//	CONSOLE_DEBUG_W_LONG("gSAOobjectCount\t=",	gSAOobjectCount);

	//------------------------------------------------------------------
	gAAVSOalertsPtr		=	ReadAAVSO_TargetData(&gAAVSOalertsCnt);
//	CONSOLE_DEBUG_W_LONG("gAAVSOalertsCnt\t=", gAAVSOalertsCnt);
//	CONSOLE_ABORT(__FUNCTION__);


#define	kMaxPolarAlignCenterPts		200
	//*	read the special.txt file
	gSpecialObjectPtr	=	ReadSpecialData(kDataSrc_Special, &gSpecialObjectCount);
	if (gSpecialObjectPtr != NULL)
	{
	char			theFirstChar;
	int				polarStarCnt;
	int				qqq;

//		CONSOLE_DEBUG("Creating Alignment object array");

		gPolarAlignObjectPtr	=	(TYPE_CelestData *)calloc(kMaxPolarAlignCenterPts, sizeof(TYPE_CelestData));

		if (gPolarAlignObjectPtr != NULL)
		{
			theFirstChar	=	'A';
			while (theFirstChar <= 'Z')
			{
				polarStarCnt	=	ComputeCenterFromStarList(	gSpecialObjectPtr,
																gSpecialObjectCount,
																theFirstChar,
																gPolarAlignObjectPtr,
																kMaxPolarAlignCenterPts);

				if (polarStarCnt > 3)
				{
				//	CONSOLE_DEBUG_W_HEX("theFirstChar\t=", theFirstChar);
				}
				theFirstChar++;
			}
			//*	figure out how many we actually have
			qqq	=	0;
			while (qqq < kMaxPolarAlignCenterPts)
			{
				if (gPolarAlignObjectPtr[qqq].dataSrc == 0)
				{
					break;
				}
				qqq++;
			}

			gPolarAlignObjectCount	=	qqq;
//			CONSOLE_DEBUG_W_LONG("gPolarAlignObjectCount\t=", gPolarAlignObjectCount);

//			CONSOLE_ABORT(__FUNCTION__);
		}
	}


#ifdef _ENABLE_ASTEROIDS_
	//==================================================================
	//*	Read Asteroid data
	gAsteroidPtr	=	ReadAsteroidData(NULL, &gAsteroidCnt, gAsteroidDatbase);
#endif // _ENABLE_ASTEROIDS_

	Precess();		//*	make sure all of the data bases are sorted properly

	//********************************************************************************
	//*	NOTE: It is IMPORTANT that the precess is called
	//*		AFTER we have loaded hippacos
	//*		BEFORE we have read in the constellations
	//*	The constellations use hippacos star numbers and copy over the ra/dec values
	//*	If we have not recessed, then they will not be in the right position.
	//*
	//*	In the future, for large changes of time, we may have to re-do the constellations
	//********************************************************************************

	SetHipparcosDataPointers(gHipObjectPtr, gHipObjectCount);
	gConstVecotrPtr		=	ReadConstellationVectors(kSkyTravelDataDirectory, &gConstVectorCnt);
	if (gConstVecotrPtr != NULL)
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

	cCurrentSkyColor	=	Set_Skycolor((planet_struct *)&cPlanetStruct, &cSunMonStruct, cDispOptions.dispEarth);
	ResetView();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSkyTravel::~WindowTabSkyTravel(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	gSkyTravelWindow		=	NULL;

	if (gStarDataPtr != NULL)
	{
		free(gStarDataPtr);
		gStarDataPtr	=	NULL;
	}
	if (gYaleStarDataPtr != NULL)
	{
		free(gYaleStarDataPtr);
		gYaleStarDataPtr	=	NULL;
	}
	if (gMessierObjectPtr != NULL)
	{
		free(gMessierObjectPtr);
		gMessierObjectPtr	=	NULL;
	}
	if (gDraperObjectPtr != NULL)
	{
		free(gDraperObjectPtr);
		gDraperObjectPtr	=	NULL;
	}

#ifdef _ENABLE_HYG_
	if (gHYGObjectPtr != NULL)
	{
		free(gHYGObjectPtr);
		gHYGObjectPtr	=	NULL;
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
int		buttonBoxWidth;
int		buttonStartX;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;


	labelWidth	=	(cTitleHeight * 3) + 8;
	//------------------------------------------
	SetWidget(			kSkyTravel_Data,	0,			yLoc,		labelWidth,		cTitleHeight);
	SetWidgetText(		kSkyTravel_Data, 	"Data control");
	SetWidgetFont(		kSkyTravel_Data,	kFont_Small);
	SetWidgetBGColor(	kSkyTravel_Data,	CV_RGB(128,	128,	128));
	SetWidgetTextColor(	kSkyTravel_Data,	CV_RGB(0,	0,	0));

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	//------------------------------------------------------------------------------
	SetWidget(			kSkyTravel_Display,	0,			yLoc,		labelWidth,		cTitleHeight);
	SetWidgetText(		kSkyTravel_Display, "Display control");
	SetWidgetFont(		kSkyTravel_Display,	kFont_Small);
	SetWidgetBGColor(	kSkyTravel_Display,	CV_RGB(128,	128,	128));
	SetWidgetTextColor(	kSkyTravel_Display,	CV_RGB(0,	0,	0));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	yLoc			=	cTabVertOffset;

	//------------------------------------------------------------------------------
	//*	set up all of the buttons
	buttonStartX	=	labelWidth + 2;
	xLoc			=	buttonStartX;
	for (iii = kSkyTravel_Btn_First; iii <= kSkyTravel_Help_Btn; iii++)
	{
		buttonBoxWidth	=	cTitleHeight - 2;

		//*	we have some special cases
		switch(iii)
		{
			case kSkyTravel_UTCtime:
				buttonBoxWidth	=	(cTitleHeight * 10);
				break;

		#ifdef _ENABLE_REMOTE_GAIA_
			case kSkyTravel_SQL_database:
				buttonBoxWidth		=	cTitleHeight * 2;
				break;
		#endif // _ENABLE_REMOTE_GAIA_

			case kSkyTravel_Btn_Reset:
				//*	move to the next row
				xLoc	=	buttonStartX;
				yLoc	+=	cTitleHeight;
				yLoc	+=	2;
				break;

			case kSkyTravel_Screen_ViewAngle:
				buttonBoxWidth		=	(cBtnWidth / 2) + 15;
				break;

			case kSkyTravel_DisplayedStarCnt:
				buttonBoxWidth		=	(cTitleHeight * 2) + 10;
				break;

			case kSkyTravel_Telescope_RA_DEC:
				buttonBoxWidth		=	cBtnWidth;
				break;

			case kSkyTravel_Telescope_Sync:
			case kSkyTravel_Telescope_GoTo:
			case kSkyTravel_Help_Btn:
				buttonBoxWidth	=	(cBtnWidth / 2) - 30;
				break;

			case kSkyTravel_Search_Text:
				buttonBoxWidth	=	200;
				break;

			default:
				break;
		}
		SetWidget(			iii,	xLoc,	yLoc,		buttonBoxWidth,		cTitleHeight);
		SetWidgetType(		iii, 	kWidgetType_Button);
		xLoc	+=	buttonBoxWidth;
		xLoc	+=	2;
	}

	//-----------------------------------------------
	SetWidgetType(		kSkyTravel_Btn_ZoomLevel, 		kWidgetType_TextBox);
	SetWidgetFont(		kSkyTravel_Btn_ZoomLevel, 		kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Btn_ZoomLevel, 		CV_RGB(255,	255,	255));

	//-----------------------------------------------
	SetWidgetType(		kSkyTravel_UTCtime, 			kWidgetType_TextBox);
	SetWidgetFont(		kSkyTravel_UTCtime,				kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_UTCtime,				CV_RGB(255,	255,	255));
	SetWidgetText(		kSkyTravel_UTCtime, 			"UTC Time");

	//-----------------------------------------------
	//*	set up the search button/text
	SetWidgetType(			kSkyTravel_Search_Text,		kWidgetType_TextInput);
	SetWidgetFont(			kSkyTravel_Search_Text,		kFont_Medium);
	SetWidgetBGColor(		kSkyTravel_Search_Text,		CV_RGB(128,	128,	128));
	SetWidgetTextColor(		kSkyTravel_Search_Text,		CV_RGB(0,	0,	0));
	SetWidgetJustification(	kSkyTravel_Search_Text,		kJustification_Left);

	//-----------------------------------------------
	SetWidgetType(		kSkyTravel_Screen_ViewAngle,	kWidgetType_TextBox);
	SetWidgetFont(		kSkyTravel_Screen_ViewAngle,	kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Screen_ViewAngle,	CV_RGB(255,	255,	255));
	SetWidgetText(		kSkyTravel_Screen_ViewAngle,	"---");

	//-----------------------------------------------
	SetWidgetType(		kSkyTravel_DisplayedStarCnt,	kWidgetType_TextBox);
	SetWidgetFont(		kSkyTravel_DisplayedStarCnt,	kFont_Medium);
	SetWidgetText(		kSkyTravel_DisplayedStarCnt,	"---");
	SetWidgetTextColor(	kSkyTravel_DisplayedStarCnt,	CV_RGB(255,	255,	255));

	//------------------------------------------------------------------------------------
	SetWidgetText(		kSkyTravel_Telescope_Sync,		"Sync");
	SetWidgetFont(		kSkyTravel_Telescope_Sync,		kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Telescope_Sync,		CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_Telescope_Sync,		CV_RGB(255,	255,	255));
	SetWidgetHelpText(	kSkyTravel_Telescope_Sync,		"Sync telescope to center of screen");

	//-----------------------------------------------
	SetWidgetText(		kSkyTravel_Telescope_GoTo,		"GoTo");
	SetWidgetFont(		kSkyTravel_Telescope_GoTo,		kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Telescope_GoTo,		CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_Telescope_GoTo,		CV_RGB(255,	255,	255));
	SetWidgetHelpText(	kSkyTravel_Telescope_GoTo,		"Slew telescope to center of screen");

	//-----------------------------------------------
	SetWidgetText(		kSkyTravel_Help_Btn,			"Help");
	SetWidgetFont(		kSkyTravel_Help_Btn,			kFont_Medium);
	SetWidgetTextColor(	kSkyTravel_Help_Btn,			CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_Help_Btn,			CV_RGB(255,	255,	255));
	SetWidgetHelpText(	kSkyTravel_Help_Btn,			"Click to launch web documentation");

	//---------------------------------------------------------------------------
	//*	set the help text for the rest of them
	SetWidgetHelpText(	kSkyTravel_Btn_Reset,			"Reset");
	SetWidgetHelpText(	kSkyTravel_Btn_Lines,			"Toggle Line display");
	SetWidgetHelpText(	kSkyTravel_Btn_OrigDatabase,	"Toggle Original star database");
	SetWidgetHelpText(	kSkyTravel_Btn_AutoAdvTime,		"Toggle Auto Advance Time");
	SetWidgetHelpText(	kSkyTravel_Btn_Chart,			"Toggle Chart mode");
	SetWidgetHelpText(	kSkyTravel_Btn_DeepSky,			"Toggle Deep Sky Objects");
	SetWidgetHelpText(	kSkyTravel_Btn_Names,			"Toggle Name display");
	SetWidgetHelpText(	kSkyTravel_Btn_CommonStarNames,	"Toggle Common star names display");
	SetWidgetHelpText(	kSkyTravel_Btn_ConstOutline,	"Toggle Constellation outlines");
	SetWidgetHelpText(	kSkyTravel_Btn_Constellations,	"Toggle Constellations");
	SetWidgetHelpText(	kSkyTravel_Btn_MilkyWayDisp,	"Toggle MilkyWay outlines");
	SetWidgetHelpText(	kSkyTravel_Btn_NGC,				"Toggle NGC display");
	SetWidgetHelpText(	kSkyTravel_Btn_Earth,			"Toggle Earth display");
	SetWidgetHelpText(	kSkyTravel_Btn_Grid,			"Toggle Grid display");
	SetWidgetHelpText(	kSkyTravel_Btn_Equator,			"Toggle Equator display");
	SetWidgetHelpText(	kSkyTravel_Btn_Ecliptic,		"Toggle Ecliptic display");
	SetWidgetHelpText(	kSkyTravel_Btn_YaleCat,			"Toggle Yale display");
	SetWidgetHelpText(	kSkyTravel_Btn_Messier,			"Toggle Messier display");
	SetWidgetHelpText(	kSkyTravel_Btn_AAVSOalerts,		"Toggle AAVSO alerts");

#ifdef _ENABLE_REMOTE_GAIA_
	SetWidgetHelpText(	kSkyTravel_Btn_Gaia,			"Toggle GAIA/SQL display");
	SetWidgetText(		kSkyTravel_SQL_database,		gSQLsever_Database);
	SetWidgetFont(		kSkyTravel_SQL_database,		kFont_RadioBtn);
	SetWidgetHelpText(	kSkyTravel_SQL_database,		"Currently selected database");
#endif
#ifdef _ENABLE_ASTEROIDS_
	SetWidgetHelpText(	kSkyTravel_Btn_Asteroids,		"Toggle Asteroid display");
#endif

	SetWidgetHelpText(	kSkyTravel_Btn_MagnitudeDisp,	"Toggle Magnitude display");
	SetWidgetHelpText(	kSkyTravel_UTCtime,				"Time UTC and Sidereal (in live mode)");
	SetWidgetHelpText(	kSkyTravel_Btn_Hipparcos,		"Toggle Hipparcos display");
	SetWidgetHelpText(	kSkyTravel_Btn_NightMode,		"Toggle Night Mode");
	SetWidgetHelpText(	kSkyTravel_Btn_Symbols,			"Toggle Symbols for planets and zodiac");
	SetWidgetHelpText(	kSkyTravel_Btn_AllMagnitudes,	"Toggle all magnitudes");
	SetWidgetHelpText(	kSkyTravel_Btn_TscopeDisp,		"Toggle telescope position display");
	SetWidgetHelpText(	kSkyTravel_Btn_Draper,			"Toggle Draper display");
	SetWidgetHelpText(	kSkyTravel_Btn_SAO,				"Toggle SAO display");
	SetWidgetHelpText(	kSkyTravel_Btn_Plus,			"Zoom In");
	SetWidgetHelpText(	kSkyTravel_Btn_Minus,			"Zoom Out");
	SetWidgetHelpText(	kSkyTravel_Btn_ZoomLevel,		"Indicates Current Zoom level");
	SetWidgetHelpText(	kSkyTravel_Screen_ViewAngle,	"Indicates Current View Angle (radians/degrees) : dd:mm:ss if below 2 degrees");
	SetWidgetHelpText(	kSkyTravel_DisplayedStarCnt,	"Indicates the number of stars currently displayed");
	SetWidgetHelpText(	kSkyTravel_Search_Text,			"Enter object to search for");
	SetWidgetHelpText(	kSkyTravel_Search_Btn,			"Click to search");


	//---------------------------------------------------------------------------
	//*	set the button characters (names)
	SetWidgetText(		kSkyTravel_Btn_AutoAdvTime,		"@");
	SetWidgetText(		kSkyTravel_Btn_Reset,			"r");
	SetWidgetText(		kSkyTravel_Btn_Chart,			"c");
	SetWidgetText(		kSkyTravel_Btn_AAVSOalerts,		"a");
	SetWidgetText(		kSkyTravel_Btn_DeepSky,			"D");
	SetWidgetText(		kSkyTravel_Btn_Draper,			"d");
	SetWidgetText(		kSkyTravel_Btn_SAO,				"^");
	SetWidgetText(		kSkyTravel_Btn_Names,			"N");
	SetWidgetText(		kSkyTravel_Btn_CommonStarNames,	"m");
	SetWidgetText(		kSkyTravel_Btn_Lines,			"L");
	SetWidgetText(		kSkyTravel_Btn_OrigDatabase,	"$");
	SetWidgetText(		kSkyTravel_Btn_ConstOutline,	"O");
	SetWidgetText(		kSkyTravel_Btn_Constellations,	"?");
	SetWidgetText(		kSkyTravel_Btn_MilkyWayDisp,	"W");
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
	SetWidgetText(		kSkyTravel_Btn_AllMagnitudes,	"A");
	SetWidgetText(		kSkyTravel_Btn_TscopeDisp,		"t");
	SetWidgetText(		kSkyTravel_Btn_Plus,			"+");
	SetWidgetText(		kSkyTravel_Btn_Minus,			"-");
	SetWidgetText(		kSkyTravel_Btn_MagnitudeDisp,	".");
#if defined(_ENABLE_REMOTE_GAIA_)
	SetWidgetText(		kSkyTravel_Btn_Gaia,			"g");
#endif
#ifdef _ENABLE_ASTEROIDS_
	SetWidgetText(		kSkyTravel_Btn_Asteroids,		",");
#endif


	//------------------------------------------------------------------------------------
	//*	Dome/Telescope indicators
	SetWidgetType(		kSkyTravel_DomeIndicator, 		kWidgetType_TextBox);
	SetWidgetText(		kSkyTravel_DomeIndicator,		"D");
	SetWidgetTextColor(	kSkyTravel_DomeIndicator,		CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_DomeIndicator,		CV_RGB(255,	0,	0));
	SetWidgetHelpText(	kSkyTravel_DomeIndicator,		"Indicates if Dome is OnLine (Grn=yes/Red=no)");

	//------------------------------------------------------------------------------------
	SetWidgetType(		kSkyTravel_TelescopeIndicator, 	kWidgetType_TextBox);
	SetWidgetText(		kSkyTravel_TelescopeIndicator,	"T");
	SetWidgetTextColor(	kSkyTravel_TelescopeIndicator,	CV_RGB(0,	0, 0));
	SetWidgetBGColor(	kSkyTravel_TelescopeIndicator,	CV_RGB(255,	0,	0));
	SetWidgetHelpText(	kSkyTravel_TelescopeIndicator,	"Indicates if Telescope is OnLine (Grn=yes/Red=no)");


	//------------------------------------------------------------------------------------
	SetWidgetFont(		kSkyTravel_Telescope_RA_DEC,	kFont_Medium);
	SetWidgetText(		kSkyTravel_Telescope_RA_DEC,	"--");
	SetWidgetHelpText(	kSkyTravel_Telescope_RA_DEC,	"Current telescope location, Double click to center screen");

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-----------------------------------------------
	SetWidget(				kSkyTravel_MsgTextBox,	1,		yLoc,	(cWidth - 2),		cTitleHeight);
	SetWidgetType(			kSkyTravel_MsgTextBox, kWidgetType_TextBox);
	SetWidgetFont(			kSkyTravel_MsgTextBox,	kFont_Medium);
	SetWidgetTextColor(		kSkyTravel_MsgTextBox, CV_RGB(128,	128, 128));
	SetWidgetText(			kSkyTravel_MsgTextBox,	"message text box");
	SetWidgetJustification(	kSkyTravel_MsgTextBox,	kJustification_Left);

	SetHelpTextBoxNumber(	kSkyTravel_MsgTextBox);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//-----------------------------------------------
	SetWidget(				kSkyTravel_CursorInfoTextBox,	1,		yLoc,	(cWidth - 2),		cTitleHeight);
	SetWidgetType(			kSkyTravel_CursorInfoTextBox,	kWidgetType_TextBox);
	SetWidgetFont(			kSkyTravel_CursorInfoTextBox,	kFont_Medium);
	SetWidgetTextColor(		kSkyTravel_CursorInfoTextBox,	CV_RGB(128, 128, 128));
	SetWidgetText(			kSkyTravel_CursorInfoTextBox,	"Help text box");
	SetWidgetJustification(	kSkyTravel_CursorInfoTextBox,	kJustification_Left);


	yLoc			+=	cTitleHeight;
	yLoc			+=	2;



//-	cWorkSpaceTopOffset	=	yLoc;
	skyBoxHeight		=	cHeight - yLoc;
//	CONSOLE_DEBUG_W_NUM("skyBoxHeight\t=", skyBoxHeight);
//	CONSOLE_DEBUG_W_NUM("cWorkSpaceTopOffset\t=", cWorkSpaceTopOffset);

	SetWidget(				kSkyTravel_NightSky,	0,	yLoc,		cWidth,		skyBoxHeight);
	SetWidgetType(			kSkyTravel_NightSky, 	kWidgetType_CustomGraphic);
	SetWidgetBGColor(		kSkyTravel_NightSky,	CV_RGB(0,	0,		0));
	SetWidgetBorderColor(	kSkyTravel_NightSky,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kSkyTravel_NightSky,	true);

	SetSkyDisplaySize(cWidth, skyBoxHeight);
	UpdateButtonStatus();
}

//**************************************************************************************
void WindowTabSkyTravel::ActivateWindow(void)
{
	UpdateButtonStatus();
#if defined(_ENABLE_REMOTE_GAIA_)
	SetWidgetText(		kSkyTravel_SQL_database,		gSQLsever_Database);
#endif
}

//**************************************************************************************
void WindowTabSkyTravel::RunWindowBackgroundTasks(void)
{
uint32_t			currentMilliSecs;
uint32_t			deltaMilliSecs;
struct timeval		currentTimeVal;
char				utcTimeString[32];
char				siderealTimeString[32];
char				textBuff[128];
struct tm			utcTime;
struct tm			siderealTime;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	currentMilliSecs	=	millis();

	gettimeofday(&currentTimeVal, NULL);
	if (cForceClockUpdate || (currentTimeVal.tv_sec != cPreviousClockUpdateTVsecs))
	{
		if (gAutoAdvanceTime)
		{
			SetCurrentTime();

			gettimeofday(&currentTimeVal, NULL);
			FormatTimeString(&currentTimeVal, utcTimeString);

			gmtime_r(&currentTimeVal.tv_sec, &utcTime);
			CalcSiderealTime(&utcTime, &siderealTime, gObseratorySettings.Longitude_deg);
			FormatTimeString_TM(&siderealTime, siderealTimeString);
			sprintf(textBuff, "U%s / S%s", utcTimeString, siderealTimeString);
		}
		else
		{
			sprintf(textBuff, "%02d/%02d/%02d U%02d:%02d:%02d - L%02d",
									gCurrentSkyTime.year,
									gCurrentSkyTime.month,
									gCurrentSkyTime.day,
									gCurrentSkyTime.hour,
									gCurrentSkyTime.min,
									gCurrentSkyTime.sec,
									gCurrentSkyTime.local_hour);
		}
		SetWidgetText(kSkyTravel_UTCtime, textBuff);
//		CONSOLE_DEBUG(textBuff);
		cPreviousClockUpdateTVsecs	=	currentTimeVal.tv_sec;
		ForceWindowUpdate();
	}
#ifdef _ENABLE_REMOTE_GAIA_
	deltaMilliSecs		=	currentMilliSecs - cLastGaiaUpdate_ms;
	//*	update the GAIA background task ever second
	if (deltaMilliSecs >= 1000)
	{
		//*	if GAIA is active, let the GAIA background thread know about any changes
		//*	dont update anything if mouse dragging is in progress
		if ((gST_DispOptions.RemoteGAIAenabled) && (cDispOptions.dispGaia) && (cMouseDragInProgress == false))
		{
		bool	requestStarted;
		double	request_RA_Degrees;
		double	request_Dec_Degrees;
		double	viewAngle_Degrees;
		int		numRequests;
		int		iii;
			//*	we are going to request 1, 3, or 5 blocks.
			//*		Only 1 if above the polar limit
			//*		5 if between the polar limit and 80 degrees Declination
			//*		3 otherwised
			request_RA_Degrees	=	DEGREES(cRa0);
			request_Dec_Degrees	=	DEGREES(cDecl0);
			viewAngle_Degrees	=	DEGREES(cView_angle_Rads);
			numRequests			=	1;


			if (request_RA_Degrees < 0.0)
			{
				request_RA_Degrees	+=	360.0;
			}

			//*	figure out how many requests to make
			if (fabs(request_Dec_Degrees) >= kPolarDeclinationLimit)
			{
				numRequests			=	1;
			}
			else if ((request_Dec_Degrees >= 80) || (request_Dec_Degrees <= -80))
			{
				request_RA_Degrees	=	request_RA_Degrees - 2.0;
				numRequests			=	5;
			}
			else if (viewAngle_Degrees < 2.0)
			{
				numRequests			=	1;
			}
			else if (gST_DispOptions.GaiaRequestMode != kGaiaRequestMode_1x1)
			{
				request_RA_Degrees	=	request_RA_Degrees - 1.0;
				numRequests			=	3;
			}

			//*	yes, we need to check this again
			if (request_RA_Degrees < 0.0)
			{
				request_RA_Degrees	+=	360.0;
			}

			requestStarted	=	0;
			for (iii=0; iii<numRequests; iii++)
			{
				requestStarted		+=	UpdateSkyTravelView(request_RA_Degrees,
															request_Dec_Degrees,
															viewAngle_Degrees);

				//*	are we doing the 3x3 requests
				if (gST_DispOptions.GaiaRequestMode == kGaiaRequestMode_3x3)
				{
					requestStarted		+=	UpdateSkyTravelView(request_RA_Degrees,
																request_Dec_Degrees - 1,
																viewAngle_Degrees);

					requestStarted		+=	UpdateSkyTravelView(request_RA_Degrees,
																request_Dec_Degrees + 1,
																viewAngle_Degrees);
				}
				request_RA_Degrees	+=	1.0;
				if (request_RA_Degrees >= 360.0)
				{
					request_RA_Degrees	=	0.0;
				}
			}
			if (requestStarted > 0)
			{
				sprintf(textBuff, "GAIA SQL request started for RA=%1.0fD(%1.1fH) DEC=%1.0f",
									floor(request_RA_Degrees),
									floor(request_RA_Degrees) / 15,
									floor(request_Dec_Degrees));

				SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(0, 255, 0));
				SetWidgetText(kSkyTravel_MsgTextBox, textBuff);
				ForceWindowUpdate();
			}
		}
		cLastGaiaUpdate_ms	=	currentMilliSecs;
	}
#endif // _ENABLE_REMOTE_GAIA_


	//-----------------------------------------------------------
	//*	check on remote imaging
	//*	remote imaging refers to downloading an image from stsci.edu or other image source
	deltaMilliSecs		=	currentMilliSecs - cLastRemoteImageUpdate_ms;
	if (deltaMilliSecs >= 2000)
	{
		//*	every 1 second
		if (IsRemoteImageRunning())
		{
			SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(0,	255, 0));
			SetWidgetText(kSkyTravel_MsgTextBox, gRemoteImageStatusMsg);
		}
		else if (RemoteDataMsgUpdated())
		{
			SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(0,	255, 0));
			SetWidgetText(kSkyTravel_MsgTextBox, gRemoteImageStatusMsg);
			ClearRemoteDataMsgFlag();

		}
		cLastRemoteImageUpdate_ms		=	millis();
	}
//#if 0
//	//*	check for telescope information
//	if (gTelescopeUpdated)
//	{
//	char	ra_dec_string[128];
//
//		CONSOLE_DEBUG("gTelescopeUpdated");
//
//		gTelescopeUpdated	=	false;
//
//		if (strlen(gTelescopeErrorString) > 0)
//		{
//			SetWidgetText(kSkyTravel_Telescope_RA_DEC, gTelescopeErrorString);
//			gTelescopeErrorString[0]	=	0;
//		}
//		else
//		{
//			sprintf(ra_dec_string, "%s / %s (%d)", gTelescopeRA_String, gTelescopeDecl_String, gTelescopeUpdateCnt);
//			SetWidgetText(kSkyTravel_Telescope_RA_DEC, ra_dec_string);
//		}
////-		SetWidgetChecked(kSkyTravel_ConnLX200, gLX200_ThreadActive);
//
//		//*	this makes it real time.
//		SetCurrentTime();
//
//		ForceWindowUpdate();
//	}
//#endif
}

//*****************************************************************************
void	WindowTabSkyTravel::UpdateButtonStatus(void)
{

	SetWidgetChecked(		kSkyTravel_Btn_DeepSky,			cDispOptions.dispDeep);
	SetWidgetChecked(		kSkyTravel_Btn_Draper,			cDispOptions.dispDraper);
	SetWidgetChecked(		kSkyTravel_Btn_SAO,				cDispOptions.dispSAO);
	SetWidgetChecked(		kSkyTravel_Btn_Names,			cDispOptions.dispNames);
	SetWidgetChecked(		kSkyTravel_Btn_CommonStarNames,	cDispOptions.dispCommonStarNames);
	SetWidgetChecked(		kSkyTravel_Btn_Lines,			cDispOptions.dispLines);
	SetWidgetChecked(		kSkyTravel_Btn_OrigDatabase,	cDispOptions.dispDefaultData);

	SetWidgetChecked(		kSkyTravel_Btn_ConstOutline,	cDispOptions.dispConstOutlines);
	SetWidgetChecked(		kSkyTravel_Btn_Constellations,	cDispOptions.dispConstellations);
	SetWidgetChecked(		kSkyTravel_Btn_MilkyWayDisp,	cDispOptions.dispMilkyWayOutline);
	SetWidgetChecked(		kSkyTravel_Btn_NGC,				cDispOptions.dispNGC);
	SetWidgetChecked(		kSkyTravel_Btn_Messier,			cDispOptions.dispMessier);
	SetWidgetChecked(		kSkyTravel_Btn_YaleCat,			cDispOptions.dispYale);
#ifdef _ENABLE_ASTEROIDS_
	SetWidgetChecked(		kSkyTravel_Btn_Asteroids,		cDispOptions.dispAsteroids);
#endif // _ENABLE_ASTEROIDS_
#if defined(_ENABLE_REMOTE_GAIA_)
	SetWidgetChecked(		kSkyTravel_Btn_Gaia,			cDispOptions.dispGaia);
#endif

	SetWidgetChecked(		kSkyTravel_Btn_Hipparcos,		cDispOptions.dispHipparcos);
	SetWidgetChecked(		kSkyTravel_Btn_AAVSOalerts,		cDispOptions.dispAAVSOalerts);

	SetWidgetChecked(		kSkyTravel_Btn_MagnitudeDisp,	gST_DispOptions.DispMagnitude);

	SetWidgetChecked(		kSkyTravel_Btn_AutoAdvTime,		gAutoAdvanceTime);

	SetWidgetChecked(		kSkyTravel_Btn_Chart,			cChartMode);
	SetWidgetChecked(		kSkyTravel_Btn_Earth,			cDispOptions.dispEarth);
	SetWidgetChecked(		kSkyTravel_Btn_Grid,			cDispOptions.dispGrid);
	SetWidgetChecked(		kSkyTravel_Btn_Equator,			cDispOptions.dispEquator_line);
	SetWidgetChecked(		kSkyTravel_Btn_Ecliptic,		cDispOptions.ecliptic_line);
	SetWidgetChecked(		kSkyTravel_Btn_NightMode,		cNightMode);
	SetWidgetChecked(		kSkyTravel_Btn_Symbols,			cDispOptions.dispSymbols);
	SetWidgetChecked(		kSkyTravel_Btn_AllMagnitudes,	gST_DispOptions.MagnitudeMode);

	SetWidgetChecked(		kSkyTravel_Btn_TscopeDisp,		cDispOptions.dispTelescope);

	SetWidgetNumber(		kSkyTravel_Btn_ZoomLevel,		cView_index);

	UpdateViewAngleDisplay();

}

//*****************************************************************************
void	WindowTabSkyTravel::UpdateViewAngleDisplay(void)
{
char	textString[64];

	//*	display the view angle in radians and degrees
	if (cView_angle_Rads < RADIANS(2.0))
	{
		FormatHHMMSS(DEGREES(cView_angle_Rads), textString, false);
	}
	else if (cView_angle_Rads < 1.0)
	{
		sprintf(textString, "%3.3f/%3.2f", cView_angle_Rads, DEGREES(cView_angle_Rads));
	}
	else
	{
		sprintf(textString, "%3.3f/%3.1f", cView_angle_Rads, DEGREES(cView_angle_Rads));
	}
	SetWidgetText(		kSkyTravel_Screen_ViewAngle,	textString);
}


//*****************************************************************************
//*	Process single char command, this is to unify the screen buttons and key presses
//*	to eliminate duplicate code
//*	returns FALSE if char not processed
//*****************************************************************************
bool	WindowTabSkyTravel::ProcessSingleCharCmd(const int cmdChar)
{
bool	charWasProcessed;
bool	controlKeyDown;
double	newView_angle;

//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", cmdChar);
	controlKeyDown	=	false;
	if (cmdChar & 0x040000)
	{
		controlKeyDown	=	true;
	}
	charWasProcessed	=	true;
	switch(cmdChar  & 0x0fff)
	{
		case '~':
			ResetView();
			cDispOptions.dispAAVSOalerts		=	false;
			cDispOptions.dispAsteroids			=	false;
			cDispOptions.dispCommonStarNames	=	false;
			cDispOptions.dispConstOutlines		=	false;
			cDispOptions.dispConstellations		=	false;
			cDispOptions.dispDeep				=	false;
			cDispOptions.dispDefaultData		=	false;
			cDispOptions.dispDraper				=	false;
			cDispOptions.dispEarth				=	false;
			cDispOptions.dispGaia				=	false;
		//	cDispOptions.dispGrid				=	false;
			cDispOptions.dispHipparcos			=	false;
			cDispOptions.dispHorizon_line		=	false;
			cDispOptions.dispHYG_all			=	false;
			cDispOptions.dispMessier			=	false;
			cDispOptions.dispNames				=	false;
			cDispOptions.dispNGC				=	false;
			cDispOptions.dispSAO				=	false;
			cDispOptions.dispSpecialObjects		=	kSpecialDisp_Off;
			cDispOptions.dispSymbols			=	false;
			cDispOptions.dispYale				=	false;

			gST_DispOptions.DispMagnitude		=	false;
			gST_DispOptions.DispSpectralType	=	false;
			gST_DispOptions.DispParallax		=	false;
			gST_DispOptions.DashedLines			=	false;
			gST_DispOptions.MagnitudeMode		=	kMagnitudeMode_Dynamic;
			UpdateButtonStatus();
			break;

		//*	adjust the magnitude limit for what gets displayed
		case '`':
			gST_DispOptions.MagnitudeMode		=	kMagnitudeMode_Specified;
			if (controlKeyDown)
			{
				gST_DispOptions.DisplayedMagnitudeLimit	-=	1.0;
			}
			else
			{
				gST_DispOptions.DisplayedMagnitudeLimit	+=	1.0;
			}
			if (gST_DispOptions.DisplayedMagnitudeLimit > 25.0)
			{
				gST_DispOptions.DisplayedMagnitudeLimit	=	25.0;
			}
			if (gST_DispOptions.DisplayedMagnitudeLimit < 5.0)
			{
				gST_DispOptions.DisplayedMagnitudeLimit	=	5.0;
			}
			CONSOLE_DEBUG_W_DBL("DisplayedMagnitudeLimit\t=", gST_DispOptions.DisplayedMagnitudeLimit);
			break;

		case '!':	//*	toggle night mode
			cNightMode	=	!cNightMode;
			SetWidgetChecked(		kSkyTravel_Btn_NightMode,	cNightMode);
			break;

		case '$':
			cDispOptions.dispDefaultData	=	!cDispOptions.dispDefaultData;
			SetWidgetChecked(		kSkyTravel_Btn_OrigDatabase,	cDispOptions.dispDefaultData);
			break;

		case '^':
			cDispOptions.dispSAO	=	!cDispOptions.dispSAO;
			break;

		case '=':	//*	reset the clock to now
		case '<':	//*	Back one hour
		case '>':	//*	Forward one hour
		case '[':	//*	Back one day
		case ']':	//*	Forward one day
		case '{':	//*	Back one month
		case '}':	//*	Forward one month
		case '(':	//*	Back one year
		case ')':	//*	Forward one year
		case '9':	//*	Back one minute
		case '0':	//*	Forward one minute
			ProcessTimeAdjustmentChar(cmdChar  & 0x07f);
			gAutoAdvanceTime	=	false;
			cForceClockUpdate	=	true;		//*	force clock on screen to update
			ForceWindowUpdate();
			break;

		case '.':
			gST_DispOptions.DispMagnitude		=	!gST_DispOptions.DispMagnitude;
			gST_DispOptions.DispSpectralType	=	gST_DispOptions.DispMagnitude;

			SetWidgetChecked(		kSkyTravel_Btn_MagnitudeDisp,	gST_DispOptions.DispMagnitude);
			break;

#ifdef _ENABLE_ASTEROIDS_
		case ',':
			cDispOptions.dispAsteroids		=	!cDispOptions.dispAsteroids;
			SetWidgetChecked(		kSkyTravel_Btn_Asteroids,	cDispOptions.dispAsteroids);
			break;
#endif // _ENABLE_ASTEROIDS_


	//	case kLeftArrowKey:	//change azimuth
		case '1':	//change azimuth
		case 0x0f51:
			cAz0	+=	(cView_angle_Rads / 16);
			if (cAz0 > kTWOPI)
			{
				cAz0	-=	kTWOPI;
			}
			break;


	//	case kRightArrowKey:
		case '2':
		case 0x0f53:
			cAz0	-=	(cView_angle_Rads / 16);
			if (cAz0 < -kTWOPI)
			{
				cAz0	+=	kTWOPI;
			}
			break;

//		case kUpArrowKey:		//change altitude
//		case '.':
		case 0x0f52:
			cElev0	+=	(cView_angle_Rads / 16);
			if (cElev0 >= kHALFPI)
			{
				cElev0	=	kHALFPI - kEPSILON;
			}
			break;

//		case kDownArrowKey:
//		case ',':
		case 0x0f54:
			cElev0	-=	(cView_angle_Rads / 16);
			if (cElev0 <= -kHALFPI)
			{
				cElev0	=	-(kHALFPI + kEPSILON);
			}
			break;

		case '@':
			gAutoAdvanceTime	=	!gAutoAdvanceTime;
			if (gAutoAdvanceTime)
			{
				SetCurrentTime();
			}
			SetWidgetChecked(		kSkyTravel_Btn_AutoAdvTime,		gAutoAdvanceTime);
			cForceClockUpdate	=	true;		//*	force clock on screen to update
			break;

		case '#':	//*	toggle GRID
			cDispOptions.dispGrid	=	!cDispOptions.dispGrid;
			SetWidgetChecked(		kSkyTravel_Btn_Grid,	cDispOptions.dispGrid);
			break;

		case '?':	//*	toggle Constellation lines (new style)
			cDispOptions.dispConstellations	=	!cDispOptions.dispConstellations;
			SetWidgetChecked(		kSkyTravel_Btn_Constellations, cDispOptions.dispConstellations);
			break;

		case 'a':	//*	toggle AAVSO Alerts
			SetAAVSOdisplayFlag(!cDispOptions.dispAAVSOalerts);
			break;

		case 0x27:		//*	the quote char
			gST_DispOptions.MagnitudeMode++;
			if (gST_DispOptions.MagnitudeMode > kMagnitudeMode_All)
			{
				gST_DispOptions.MagnitudeMode	=	0;
			}
			CONSOLE_DEBUG_W_NUM("gST_DispOptions.MagnitudeMode\t=", gST_DispOptions.MagnitudeMode);
			break;

		case 'A':
			//*	toggle between all and dynamic
			gST_DispOptions.MagnitudeMode	=	(gST_DispOptions.MagnitudeMode ? 0 : kMagnitudeMode_All);
			break;

		case 'C':	//*	toggle ECLIPTIC LINE
			cDispOptions.ecliptic_line	=	!cDispOptions.ecliptic_line;
			SetWidgetChecked(		kSkyTravel_Btn_Ecliptic,	cDispOptions.ecliptic_line);
			break;

		case 'c':	//*	toggle CHART MODE
			cChartMode	=	!cChartMode;
			SetWidgetChecked(		kSkyTravel_Btn_Chart,		cChartMode);
			break;

		case 'D':	//*	toggle DEEP SKY
			cDispOptions.dispDeep	=	!cDispOptions.dispDeep;
			SetWidgetChecked(		kSkyTravel_Btn_DeepSky,	cDispOptions.dispDeep);
			break;

		//*	Toggle Draper data base
		case 'd':
			cDispOptions.dispDraper		=	!cDispOptions.dispDraper;
			SetWidgetChecked(		kSkyTravel_Btn_Draper,	cDispOptions.dispDraper);
			break;

		case 'E':	//*	toggle EARTH
			cDispOptions.dispEarth	=	!cDispOptions.dispEarth;
			SetWidgetChecked(		kSkyTravel_Btn_Earth,	cDispOptions.dispEarth);
			break;

		case 'e':	//*		goto EAST
			cTrack	=	false;
			cAz0	=	-(kHALFPI - kEPSILON);

			break;

		case 'f':	//*	goto nadir (feet)
			cTrack	=	false;
			cElev0	=	-(kHALFPI - kEPSILON);	//	go to feet
			break;

//		case 'F':	//*	Draw Find cross hairs
//			DrawCrossHairsAtCenter();
//			reDrawSky	=	false;
//			break;

		case 'G':	//*	toggle NGC objects
			cDispOptions.dispNGC		=	!cDispOptions.dispNGC;
			SetWidgetChecked(		kSkyTravel_Btn_NGC,	cDispOptions.dispNGC);
			break;

		case 'g':
			cDispOptions.dispGaia	=	!cDispOptions.dispGaia;
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
			SetWidgetChecked(		kSkyTravel_Btn_Lines,	cDispOptions.dispLines);
			break;

		case 'M':	//*	toggle Messier objects
			cDispOptions.dispMessier	=	!cDispOptions.dispMessier;
			SetWidgetChecked(		kSkyTravel_Btn_Messier,		cDispOptions.dispMessier);
			break;

		case 'm':	//*	toggle common star names
			cDispOptions.dispCommonStarNames	=	!cDispOptions.dispCommonStarNames;
			SetWidgetChecked(		kSkyTravel_Btn_CommonStarNames,	cDispOptions.dispCommonStarNames);
			break;

		case 'N': 	//*	toggle NAMES
			cDispOptions.dispNames	=	!cDispOptions.dispNames;
			SetWidgetChecked(		kSkyTravel_Btn_Names,	cDispOptions.dispNames);
			break;

		case 'n':					//	go NORTH
			cTrack	=	false;
			cAz0	=	-kEPSILON;
			break;

		case 'O':
			cDispOptions.dispConstOutlines	=	!cDispOptions.dispConstOutlines;
			SetWidgetChecked(		kSkyTravel_Btn_ConstOutline,cDispOptions.dispConstOutlines);
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

		case 'p':
			cDispOptions.dispSpecialObjects++;
			if (cDispOptions.dispSpecialObjects >= kSpecialDisp_Last)
			{
				cDispOptions.dispSpecialObjects	=	kSpecialDisp_Off;
			}
			break;

		case 'Q':	//*	toggle EQUATOR LINE
			cDispOptions.dispEquator_line	=	!cDispOptions.dispEquator_line;
			SetWidgetChecked(		kSkyTravel_Btn_Equator,	cDispOptions.dispEquator_line);
			break;

		case 'r':		//*	reset to defaults
			ResetView();
			break;


		case 'S':	//*	toggle SYMBOLS
			cDispOptions.dispSymbols	=	!cDispOptions.dispSymbols;
			SetWidgetChecked(		kSkyTravel_Btn_Symbols,	cDispOptions.dispSymbols);
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
			SetWidgetChecked(kSkyTravel_Btn_TscopeDisp,	cDispOptions.dispTelescope);
			break;

		case 'w':	//*	goto WEST
			cTrack	=	false;
			cAz0	=	kHALFPI - kEPSILON;
			break;

		case 'W':	//*	toggle MilkyWay display
			cDispOptions.dispMilkyWayOutline	=	!cDispOptions.dispMilkyWayOutline;
			break;
		case ';':	//*	toggle MilkyWay display
			cDispOptions.dispMilkyWayFilled	=	!cDispOptions.dispMilkyWayFilled;
			break;

		case 'y':	//*	Toggle HYG catalog
			cDispOptions.dispHYG_all	=	!cDispOptions.dispHYG_all;
			break;

		case 'Y':	//*	Toggle Yale catalog
			cDispOptions.dispYale		=	!cDispOptions.dispYale;
			SetWidgetChecked(		kSkyTravel_Btn_YaleCat,	cDispOptions.dispYale);
			break;

		case 'z':	//*	goto ZENITH
			cTrack	=	false;
			cElev0	=	kHALFPI - kEPSILON;
			break;

//		case kPageDnKey:
		case '+':	//*	zoom in
			cMouseDragInProgress	=	true;
			ZoomViewAngle(-1);
			cMouseDragInProgress	=	false;
			break;

//		case kPageUpKey:
		case '-':	//*	zoom out
			cMouseDragInProgress	=	true;
			ZoomViewAngle(+1);
			cMouseDragInProgress	=	false;
			break;

//		case '+':
//			gCurrentSkyTime.negflag	=	false;
//			break;

//		case '-':
//			gCurrentSkyTime.negflag	=	true;
//			break;

		case '|':			//* dome slit toggle
			cDispOptions.dispDomeSlit	=	!cDispOptions.dispDomeSlit;
			break;


		case 0x07:	//* cntl-g local time/Gmtime
			gCurrentSkyTime.local_time_flag	^=	1;
			break;


////			{
////			int	ii;
////
////				for (ii=0; ii<20; ii++)
////				{
////					printf("%3d\t%10.10f\t%10.10f\r\n",	ii,
////														DEGREES(gStarDataPtr[ii].ra),
////														DEGREES(gStarDataPtr[ii].decl));
////				}
////			}
//			gST_DispOptions.DisplayedMagnitudeLimit		-=	0.5;
//			CONSOLE_DEBUG_W_DBL("DisplayedMagnitudeLimit\t=", gST_DispOptions.DisplayedMagnitudeLimit);
//
//			gST_DispOptions.EarthDispMode++;
//			if (gST_DispOptions.EarthDispMode >= 4)
//			{
//				gST_DispOptions.EarthDispMode	=	0;
//			}
//			break;

		case '/':
			CenterOnDomeSlit();
			charWasProcessed			=	false;
			break;

		//*	flat earth view, north pole at the center, zoomed all the way out
		case '_':
			newView_angle	=	RADIANS(kMaxViewAngle_Degrees);
			cElev0			=	RADIANS(gObseratorySettings.Latitude_deg);
			//*	these values were determined by testing, not sure how they are derived
			cAz0			=	0.0007317573150906753198797;
			cRa0			=	1.3105927892006148383075015;
			cDecl0			=	1.5702432849567986572481004;

//			cAz0			=	0.0;
//			cElev0			=	RADIANS(gObseratorySettings.Latitude_deg);
//			cRa0			=	0.0;
//			cDecl0			=	0.0;

			SetView_Angle(newView_angle);
//			CONSOLE_DEBUG_W_DBL("newView_angle\t=", newView_angle);
//			CONSOLE_DEBUG_W_DBL("cAz0         \t=", cAz0);
//			CONSOLE_DEBUG_W_DBL("cAz0         \t=", DEGREES(cAz0));
//			CONSOLE_DEBUG_W_DBL("cElev0       \t=", cElev0);
//			CONSOLE_DEBUG_W_DBL("cElev0       \t=", DEGREES(cElev0));
//			CONSOLE_DEBUG_W_DBL("cRa0         \t=", cRa0);
//			CONSOLE_DEBUG_W_DBL("cRa0         \t=", DEGREES(cRa0));
//			CONSOLE_DEBUG_W_DBL("cDecl0       \t=", cDecl0);
//			CONSOLE_DEBUG_W_DBL("cDecl0       \t=", DEGREES(cDecl0));
			break;

		case 0x10FFE3:	//*	the control key, ignore it
			break;

		default:
			CONSOLE_DEBUG_W_HEX("Unknown char command", cmdChar);
			charWasProcessed			=	false;
			break;
	}

	return(charWasProcessed);
}

int	gRecursiveCounter	=	0;

//*****************************************************************************
void	WindowTabSkyTravel::HandleKeyDown(const int keyPressed)
{
bool			reDrawSky;

//	CONSOLE_DEBUG(__FUNCTION__);

	gRecursiveCounter++;

//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//	reDrawSky	=	ProcessSingleCharCmd(keyPressed & 0x0fff);
	reDrawSky	=	ProcessSingleCharCmd(keyPressed);

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
void	WindowTabSkyTravel::HandleSpecialKeys(const int keyPressed)
{
uint32_t	timeSinceLastRedraw;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(keyPressed)
	{
		case 0x10FF50:	//*	home key
			SetView_Angle(kViewAngle_Default);
			cElev0					=	kHALFPI / 2;		//* 45 degrees
			cAz0					=	0.0;				//* north
			cRa0					=	0.0;
			cDecl0					=	0.0;
			break;

		case 0x10FF56:	//*	page down
			ZoomViewAngle(-2);
			break;

		case 0x10FF55:	//*	page up
			ZoomViewAngle(2);
			break;

		case 0x10FF57:	//*	end
			break;
	}
	timeSinceLastRedraw	=	millis() - cLastRedrawTime_ms;
	if (timeSinceLastRedraw > 200)
	{
		cMouseDragInProgress	=	true;
		ForceReDrawSky();
		cMouseDragInProgress	=	false;
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	reDrawSky;
char	searchText[128];

	reDrawSky	=	true;
	switch(buttonIdx)
	{
		//*	these are in command char order for ease of finding them
		case kSkyTravel_Btn_NightMode:			ProcessSingleCharCmd('!');	break;
		case kSkyTravel_Btn_OrigDatabase:		ProcessSingleCharCmd('$');	break;
		case kSkyTravel_Btn_MagnitudeDisp:		ProcessSingleCharCmd('.');	break;
#ifdef _ENABLE_ASTEROIDS_
		case kSkyTravel_Btn_Asteroids:			ProcessSingleCharCmd(',');	break;
#endif
		case kSkyTravel_Btn_Grid:				ProcessSingleCharCmd('#');	break;
		case kSkyTravel_Btn_Plus:				ProcessSingleCharCmd('+');	break;	//*	zoom in
		case kSkyTravel_Btn_Minus:				ProcessSingleCharCmd('-');	break;	//*	zoom out
		case kSkyTravel_Btn_Constellations:		ProcessSingleCharCmd('?');	break;
		case kSkyTravel_Btn_MilkyWayDisp:		ProcessSingleCharCmd('W');	break;
		case kSkyTravel_Btn_AutoAdvTime:		ProcessSingleCharCmd('@');	break;
		case kSkyTravel_Btn_AAVSOalerts:		ProcessSingleCharCmd('a');	break;
		case kSkyTravel_Btn_Ecliptic:			ProcessSingleCharCmd('C');	break;
		case kSkyTravel_Btn_Chart:				ProcessSingleCharCmd('c');	break;
		case kSkyTravel_Btn_DeepSky:			ProcessSingleCharCmd('D');	break;
		case kSkyTravel_Btn_Draper:				ProcessSingleCharCmd('d');	break;
		case kSkyTravel_Btn_SAO:				ProcessSingleCharCmd('^');	break;
		case kSkyTravel_Btn_Earth:				ProcessSingleCharCmd('E');	break;
#if defined(_ENABLE_REMOTE_GAIA_)
		case kSkyTravel_Btn_Gaia:				ProcessSingleCharCmd('g');	break;
#endif
		case kSkyTravel_Btn_NGC:				ProcessSingleCharCmd('G');	break;
		case kSkyTravel_Btn_Lines:				ProcessSingleCharCmd('L');	break;
		case kSkyTravel_Btn_Messier:			ProcessSingleCharCmd('M');	break;
		case kSkyTravel_Btn_CommonStarNames:	ProcessSingleCharCmd('m');	break;
		case kSkyTravel_Btn_Names:				ProcessSingleCharCmd('N');	break;
		case kSkyTravel_Btn_ConstOutline:		ProcessSingleCharCmd('O');	break;
		case kSkyTravel_Btn_Equator:			ProcessSingleCharCmd('Q');	break;
		case kSkyTravel_Btn_Reset:				ProcessSingleCharCmd('r');	break;
		case kSkyTravel_Btn_Symbols:			ProcessSingleCharCmd('S');	break;
		case kSkyTravel_Btn_AllMagnitudes:		ProcessSingleCharCmd('A');	break;
		case kSkyTravel_Btn_TscopeDisp:			ProcessSingleCharCmd('t');	break;
		case kSkyTravel_Btn_YaleCat:			ProcessSingleCharCmd('Y');	break;

		case kSkyTravel_Btn_Hipparcos:
			cDispOptions.dispHipparcos	=	!cDispOptions.dispHipparcos;
			SetWidgetChecked(		kSkyTravel_Btn_Hipparcos,	cDispOptions.dispHipparcos);
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

		case kSkyTravel_Help_Btn:
			LaunchWebHelp();
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
//enum
//{
//    CV_EVENT_FLAG_LBUTTON   =1,
//    CV_EVENT_FLAG_RBUTTON   =2,
//    CV_EVENT_FLAG_MBUTTON   =4,
//    CV_EVENT_FLAG_CTRLKEY   =8,
//    CV_EVENT_FLAG_SHIFTKEY  =16,
//    CV_EVENT_FLAG_ALTKEY    =32
//};
//enum  	cv::MouseEventFlags {
//  cv::EVENT_FLAG_LBUTTON = 1,
//  cv::EVENT_FLAG_RBUTTON = 2,
//  cv::EVENT_FLAG_MBUTTON = 4,
//  cv::EVENT_FLAG_CTRLKEY = 8,
//  cv::EVENT_FLAG_SHIFTKEY = 16,
//  cv::EVENT_FLAG_ALTKEY = 32
//}
//	if (flags & CV_EVENT_FLAG_CTRLKEY)
	if (flags & cv::EVENT_FLAG_CTRLKEY)
	{
		ProcessDoubleClick_RtBtn(widgetIdx,
									event,
									xxx,
									yyy,
									flags);
		return;
	}

	reDrawSky	=	true;

	switch(widgetIdx)
	{
		case kSkyTravel_Btn_ZoomLevel:
		case kSkyTravel_Screen_ViewAngle:
			SetView_Angle(kViewAngle_Default);
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

#if defined(_ENABLE_REMOTE_GAIA_)
		case kSkyTravel_Btn_Gaia:
			SetView_Angle(RADIANS(4.9999));
			break;
#endif
		default:
			reDrawSky	=	false;
			break;
	}
	if (reDrawSky)
	{
		ForceReDrawSky();

	//	CONSOLE_DEBUG(__FUNCTION__);
	//	printf("Center of screen=%07.4f:%07.4f\r\n", DEGREES(cRa0 / 15), DEGREES(cDecl0));

	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessDoubleClick_RtBtn(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
bool			reDrawSky;
double			viewAngle_Deg;
double			degreesPerPixel;
double			arcSecPerPixel;
int				threadStatus;
char			remoteThreadStatusMsg[64];
TYPE_CelestData	closestObject;
char			myObjectName[128];

	CONSOLE_DEBUG(__FUNCTION__);
	reDrawSky	=	true;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kSkyTravel_Btn_ZoomLevel:
		case kSkyTravel_Screen_ViewAngle:
			SetView_Angle(kViewAngle_Default);
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
			//*	this will make it center on the location as well
//			cAz0			=	cCursor_az;
//			cElev0			=	cCursor_elev;
//			reDrawSky		=	true;

			viewAngle_Deg	=	DEGREES(cView_angle_Rads);
			degreesPerPixel	=	viewAngle_Deg / cWidth;
			arcSecPerPixel	=	degreesPerPixel * 3600.0;
//			CONSOLE_DEBUG_W_DBL("degreesPerPixel\t=", degreesPerPixel);
//			CONSOLE_DEBUG_W_DBL("arcSecPerPixel \t=", arcSecPerPixel);

			//*	returns 0=OK, -1, failed to create, +1 busy

//			CONSOLE_DEBUG(__FUNCTION__);
			memset(&closestObject,	0,	sizeof(TYPE_CelestData));
			FindObjectNearCursor(&closestObject);
			CONSOLE_DEBUG_W_STR("closestObject.longName \t=", closestObject.longName);
			CONSOLE_DEBUG_W_STR("closestObject.shortName\t=", closestObject.shortName);

			strcpy(myObjectName, "");
			if (strlen(closestObject.shortName) > 0)
			{
				strcpy(myObjectName, closestObject.shortName);
			}
			if (strlen(closestObject.longName) > 0)
			{
				strcat(myObjectName, "-");
				strcat(myObjectName, closestObject.longName);
			}

			threadStatus	=	GetRemoteImage(	cCursor_ra,
												cCursor_decl,
												arcSecPerPixel,
												viewAngle_Deg,
												myObjectName);
			if (threadStatus == 0)
			{
				SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(0,	255, 0));
				strcpy(remoteThreadStatusMsg, "Remote Image Thread started");
			}
			else if (threadStatus > 0)
			{
				strcpy(remoteThreadStatusMsg, "Remote Image Thread busy!!!");
			}
			else if (threadStatus > 0)
			{
				SetWidgetTextColor(	kSkyTravel_MsgTextBox, CV_RGB(255,	0, 0));
				strcpy(remoteThreadStatusMsg, "Remote Image Thread Failed!!!");
			}
			SetWidgetText(kSkyTravel_MsgTextBox, remoteThreadStatusMsg);
			break;

		default:
			reDrawSky	=	false;
			break;
	}
	if (reDrawSky)
	{
		ForceReDrawSky();

//		CONSOLE_DEBUG(__FUNCTION__);
//		printf("Center of screen=%07.4f:%07.4f\r\n", DEGREES(cRa0 / 15), DEGREES(cDecl0));

	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ForceReDrawSky(void)
{
Controller	*myControllerObj;

	UpdateButtonStatus();

	if (cMouseDragInProgress == false)
	{
		gCurrentSkyTime.precflag	=	true;	//*	force precession
		CalanendarTime(&gCurrentSkyTime);
		Precess();
	}

	//*	now force the window to update
	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cUpdateWindow		=	true;
		myControllerObj->HandleWindowUpdate();
		myControllerObj->cUpdateWindow		=	false;
		cv::waitKey(20);

		cLastRedrawTime_ms	=	millis();
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::ProcessMouseEvent(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			if (widgetIdx == kSkyTravel_NightSky)
			{
				if (cLeftButtonDown == false)
				{
	//				CONSOLE_DEBUG_W_NUM("kSkyTravel_NightSky", cDebugCounter++);
					cCsrx	=	xxx;
					cCsry	=	yyy;

					cCsrx	-=	cWorkSpaceLeftOffset;
					cCsry	-=	cWorkSpaceTopOffset;

					//*	still need the offset for doing cursor calculations
					cCsry	-=	cCursorOffsetY;

					Compute_cursor(&gCurrentSkyTime, &cCurrLatLon);

					if ((gStarCount < 10000) && (cMouseDragInProgress == false))
					{
	//					CONSOLE_DEBUG(__FUNCTION__);
						FindObjectNearCursor();
					}
					else
					{
						cInform_dist	=	0x7fff;
						cInform_id		=	-1;
					}

					DrawCursorLocationInfo();

					UpdateWindowAsNeeded();

			//		SetCursorFromXY(argLocalPt->h, argLocalPt->v);
				}
			}
			else
			{
		//		CONSOLE_DEBUG(__FUNCTION__);
			}
			break;

		case cv::EVENT_LBUTTONDOWN:
			cLeftButtonDown			=	true;
			break;

		case cv::EVENT_RBUTTONDOWN:
			cRightButtonDown		=	true;
			break;

		case cv::EVENT_MBUTTONDOWN:
			break;

		case cv::EVENT_LBUTTONUP:
			cLeftButtonDown			=	false;
			break;

		case cv::EVENT_RBUTTONUP:
			cRightButtonDown		=	false;
			break;

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
		case cv::EVENT_MOUSEHWHEEL:
			break;
#endif
		default:
			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
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
//	CONSOLE_DEBUG(__FUNCTION__);
	if (widgetIdx == kSkyTravel_NightSky)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		ForceReDrawSky();
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

			moveAmount	=	cView_angle_Rads / cWind_width;
//			CONSOLE_DEBUG_W_DBL("moveAmount\t=", moveAmount);
			//--------------------------------------
			cAz0	+=	(deltaXX * moveAmount);
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
			cElev0	+=	(deltaYY * moveAmount);
			if (cElev0 >= kHALFPI)
			{
//				CONSOLE_DEBUG(" > kHALFPI");
				cElev0	=	kHALFPI - kEPSILON;
			}
			if (cElev0 <= -kHALFPI)
			{
//				CONSOLE_DEBUG(" < kHALFPI");
				cElev0	=	-(kHALFPI + kEPSILON);
			}

			//*	we cannot draw the large databases while dragging

			ForceReDrawSky();
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
void	WindowTabSkyTravel::ProcessMouseWheelMoved(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	wheelMovement,
													const int	flags)
{
uint32_t	timeSinceLastRedraw;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);
//	CONSOLE_DEBUG_W_HEX("flags\t=", flags);

	//*	check to see if the control key is down
	if (flags & 0x08)
	{
		AdjustFaintLimit(-wheelMovement * 0.25);
	}
	else
	{
		ZoomViewAngle(wheelMovement);
		timeSinceLastRedraw	=	millis() - cLastRedrawTime_ms;
		if (timeSinceLastRedraw > 200)
		{
			cMouseDragInProgress	=	true;
			ForceReDrawSky();
			cMouseDragInProgress	=	false;
		}
	}
	ForceWindowUpdate();
}

//*****************************************************************************
//* compute cursor elev, az, ra, decl
void	WindowTabSkyTravel::Compute_cursor(TYPE_SkyTime *timeptr, TYPE_LatLon *locptr)
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
		if (cChartMode)
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
	ConvertLatLonToRaDec(&myLatLon, &gCurrentSkyTime);


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

	cXfactor	=	cWind_width / cView_angle_Rads;
	cYfactor	=	cXfactor;			//* 1:1 aspect ratio

	cWind_height	=	ySize;

}

//*********************************************************************
void	WindowTabSkyTravel::SetCurrentTime(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetCurrentTimeStruct(&gCurrentSkyTime);

}

//**************************************************************************
//* convert lat, long, elevation, azimuth to ra, dec
//* enter with new lat, long in locptr->latitude, locptr->longitude
//**************************************************************************
void	WindowTabSkyTravel::ConvertLatLonToRaDec(TYPE_LatLon *locptr, TYPE_SkyTime *timeptr)
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
	if (cChartMode)
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

	xangle		=	cView_angle_Rads / 2.0;
	yangle		=	xangle * cWind_height / cWind_width;
	rangle		=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
	cRadmax		=	rangle;

}

//*****************************************************************************************
void	WindowTabSkyTravel::PlotObjectsByDataSource(bool			enabled,
													TYPE_CelestData	*objectptr,
													long			maxObjects)
{
short	dataSource;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (enabled && (objectptr != NULL) && (maxObjects > 0))
	{
		//*	all of the objects in a list should have come from the same source,
		//*	therefore look at the type of the first object
		dataSource	=	objectptr[0].dataSrc;

		switch(dataSource)
		{
			case kDataSrc_Special:
			case kDataSrc_PolarAlignCenter:
			case kDataSrc_AAVSOalert:
			case kDataSrc_Messier:
				Search_and_plot(objectptr, maxObjects, false);	//*	data is NOT sorted
				break;

			case kDataSrc_GAIA_SQL:
//				CONSOLE_DEBUG_W_NUM("Data source\t=", objectptr->dataSrc);
//				CONSOLE_DEBUG_W_NUM("should be  \t=", kDataSrc_GAIA_SQL);
				Search_and_plot(objectptr, maxObjects, true);	//*	data is sorted
				break;


			case kDataSrc_Orginal:			//*	Frank and Cliffs original data file
			case kDataSrc_YaleBrightStar:
			case kDataSrc_NGC2000:
			case kDataSrc_NGC2000IC:
			case kDataSrc_HubbleGSC:
			case kDataSrc_Hipparcos:
			case kDataSrc_SAO:
			default:
				Search_and_plot(objectptr, maxObjects);
				break;
		}
	}
}

//*****************************************************************************
//* find_altaz - given location, ra,dec, sidereal time, compute az, elev
//*****************************************************************************
void	WindowTabSkyTravel::FindAltAz(TYPE_LatLon *locptr, TYPE_SkyTime *timeptr)
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
short		iii;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

	cDisplayedStarCount	=	0;

	CalanendarTime(&gCurrentSkyTime);
	Local_Time(&gCurrentSkyTime);		//* compute local time from gmt and timezone

	eph(	&gCurrentSkyTime,
			&cCurrLatLon,
//?			(planet_struct *)&cPlanetStruct,
			cPlanetStruct,	//*	changed 2/4/2021
			&cSunMonStruct);

#ifdef _ENBABLE_WHITE_CHART_
	if (cChartMode)
	{
		cSkyRGBvalue.red	=	255;
		cSkyRGBvalue.grn	=	255;
		cSkyRGBvalue.blu	=	255;
	}
	else
#endif
	if (gST_DispOptions.DayNightSkyColor)
	{
		CONSOLE_DEBUG("Using SkyColor!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		cCurrentSkyColor	=	Set_Skycolor((planet_struct *)&cPlanetStruct, &cSunMonStruct, cDispOptions.dispEarth);
	}
	else
	{
		cSkyRGBvalue.red	=	0;
		cSkyRGBvalue.grn	=	0;
		cSkyRGBvalue.blu	=	0;
	}
//	CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.red\t=",	cSkyRGBvalue.red);
//	CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.red\t=",	cSkyRGBvalue.grn);
//	CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.blu\t=",	cSkyRGBvalue.blu);
	SetWidgetBGColor(kSkyTravel_NightSky, CV_RGB(	cSkyRGBvalue.red,
													cSkyRGBvalue.grn,
													cSkyRGBvalue.blu));
	//--------------------------------------------------------
	//* transfer planetary ra/dec data except for moon
	for (iii=1; iii<10; iii++)
	{
		cPlanets[iii].decl	=	cPlanetStruct[iii].decl;
		cPlanets[iii].ra	=	cPlanetStruct[iii].ra;
		cPlanets[iii].magn	=	cPlanetStruct[iii].magn;
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

	cLunarEclipseFlag		=	cSunMonStruct.lunar_ecl_flag;
//	if (cSunMonStruct.lunar_ecl_flag)
//	{
//		cLunarEclipseFlag	=	true;
//	}
//	else
//	{
//		cLunarEclipseFlag	=	FALSE;
//	}


	//* if track or find, force center of field to match ra and decl
	if (cTrack || cFindFlag)
	{
		if (cTrack > 1)	//* track a planet
		{
			iii		=	(cTrack / 2) - 1;	//* extract the planet #
			cRa		=	cPlanets[iii].ra;
			cDecl	=	cPlanets[iii].decl;
		}
		else
		{
			cRa		=	cRa0 + kEPSILON;
			cDecl	=	cDecl0 + kEPSILON;
		}
		FindAltAz(&cCurrLatLon, &gCurrentSkyTime);
		cAz0		=	cAz;
		cElev0		=	cElev;
		cCsrx		=	cWind_x0;
		cCsry		=	cWind_y0;
		cFindFlag	=	false;	//* always reset
	}


	ConvertLatLonToRaDec(&cCurrLatLon, &gCurrentSkyTime);

//	DRAW HERE

	//*--------------------------------------------------------------------------------
	//*	these are outlines for the milkyway
	if (gMilkyWay_outlines != NULL)
	{
		//*	draw the filed areas first
		//*	if we are zoomed in too far, dont draw the polygons
		if (cDispOptions.dispMilkyWayFilled && (cView_angle_Rads > 0.25))
		{
			DrawPolygons(gMilkyWay_outlines, gMilkyWay_outlineCnt);
		}

		if (cDispOptions.dispMilkyWayOutline)
		{
			LLG_PenSize(gST_DispOptions.LineWidth_MilkyWay);
			DrawOutlines(gMilkyWay_outlines, gMilkyWay_outlineCnt, W_DARKGRAY);
		}
	}

	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispNGC)
	{
		//*	these are outlines from OpenNGC
		DrawOpenNGC_Outlines();
	}

	//*--------------------------------------------------------------------------------
	//*--------------------------------------------------------------------------------
	//*--------------------------------------------------------------------------------
	//*	the first group of things should NOT be drawn when we are mouse dragging
	if ((cMouseDragInProgress == false) && (cLeftButtonDown == false))
	{
	#ifdef _ENABLE_REMOTE_GAIA_
		if (cDispOptions.dispGaia)
		{
		double		distance_Deg;
		int			drawnCnt;
		int			notDrawnCnt;
		bool		foundValid;

//			SETUP_TIMING();

			drawnCnt	=	0;
			notDrawnCnt	=	0;
			foundValid	=	false;
			//*	step through the array of remote GAIA data and plot what ever is there.
	//		CONSOLE_DEBUG("Checking remote GAIA data");
			for (iii=0; iii<kMaxGaiaDataSets; iii++)
			{
				//*	check to see if this entry is valid
				if (gGaiaDataList[iii].validData && (gGaiaDataList[iii].gaiaDataCnt > 0))
				{
					foundValid	=	true;
				//*	we are going to check to see if any of the region is on the screen
					//*	to help speed up drawing
	//				CONSOLE_DEBUG("--------------------------------------------------------------");
					distance_Deg	=   CalcRA_DEC_Distance_Deg(DEGREES(cRa0),
																DEGREES(cDecl0),
																gGaiaDataList[iii].block_RA_deg,
																gGaiaDataList[iii].block_DEC_deg);
					gGaiaDataList[iii].distanceCtrScrn	=	distance_Deg;
					if ((distance_Deg <= (DEGREES(cView_angle_Rads) * 0.75)) ||
						(distance_Deg < 2.0))
					{
	//					CONSOLE_DEBUG("Drawn:YES");
						drawnCnt++;
						PlotObjectsByDataSource(	cDispOptions.dispGaia,
													gGaiaDataList[iii].gaiaData,
													gGaiaDataList[iii].gaiaDataCnt);
					}
					else
					{
	//					CONSOLE_DEBUG("Drawn:No");
						notDrawnCnt++;
					}
	//				CONSOLE_DEBUG_W_NUM("iii\t\t\t=", iii);
	//				CONSOLE_DEBUG_W_DBL("distance_Deg\t\t=", distance_Deg);
	//				CONSOLE_DEBUG_W_DBL("DEGREES(cView_angle_Rads)\t=", DEGREES(cView_angle_Rads));
				}
			}
			if (foundValid)
			{
		//		CONSOLE_DEBUG("GAIA Done");
		//		CONSOLE_DEBUG_W_NUM("drawnCnt   \t=", drawnCnt);
		//		CONSOLE_DEBUG_W_NUM("notDrawnCnt\t=", notDrawnCnt);
		//		DEBUG_TIMING("Time to draw remote GAIA:");
			}
		}
	#endif // _ENABLE_REMOTE_GAIA_

	#ifdef _ENABLE_HYG_
		//*	draw the dense stuff first so the other stuff is on top
		//*--------------------------------------------------------------------------------
		PlotObjectsByDataSource(cDispOptions.dispHYG_all,	gHYGObjectPtr,		gHYGObjectCount);
	#endif
		//*--------------------------------------------------------------------------------
		//*	draw the faint Hipparcos stuff first
		PlotObjectsByDataSource(cDispOptions.dispHipparcos,	gHipObjectPtr,		gHipObjectCount);
		PlotObjectsByDataSource(cDispOptions.dispDraper,	gDraperObjectPtr,	gDraperObjectCount);

	#ifdef _ENABLE_ASTEROIDS_
		//*--------------------------------------------------------------------------------
		//*	check to see if the asteroids are loaded
//		if (cDispOptions.dispAsteroids && ((cView_angle_Rads < 1.0) || (gST_DispOptions.MagnitudeMode == kMagnitudeMode_All)))
		if (cDispOptions.dispAsteroids)
		{
			DrawAsteroids();
		}
	#endif // _ENABLE_ASTEROIDS_

		PlotObjectsByDataSource(cDispOptions.dispNGC,	gNGCobjectPtr,		gNGCobjectCount);
		PlotObjectsByDataSource(cDispOptions.dispSAO,	gSAOobjectPtr,		gSAOobjectCount);
	}


	//*--------------------------------------------------------------------------------
	//*	if we are to much zoomed in, dont bother with the outlines
	if (cDispOptions.dispConstOutlines && (cView_angle_Rads > 0.15))
	{
		LLG_PenSize(gST_DispOptions.LineWidth_ConstOutlines);
		DrawConstellationOutLines();
		LLG_PenSize(1);
	}
	//*--------------------------------------------------------------------------------
	//*	this is my new constellation vectors, far better than the original ones
	if (cDispOptions.dispConstellations)
	{
		LLG_PenSize(gST_DispOptions.LineWidth_Constellations);
		DrawConstellationVectors();
		LLG_PenSize(1);
	}

	//*--------------------------------------------------------------------------------
	//*	common star names are added to the Hipparcos data, we plot it separately
	if (cDispOptions.dispCommonStarNames && (cView_angle_Rads < 3.0))
	{
		DrawCommonStarNames();
	}


	//*--------------------------------------------------------------------------------
	if (cDispOptions.dispLines && (cView_angle_Rads > 0.15))
	{
		DrawConstellationLines();
	}


	//*--------------------------------------------------------------------------------
	PlotObjectsByDataSource(cDispOptions.dispDefaultData,		gStarDataPtr,		gStarCount);
	PlotObjectsByDataSource(cDispOptions.dispYale,				gYaleStarDataPtr,	gYaleStarCount);
	PlotObjectsByDataSource(cDispOptions.dispMessier,			gMessierObjectPtr,	gMessierObjectCount);
	PlotObjectsByDataSource(cDispOptions.dispSpecialObjects,	gSpecialObjectPtr,	gSpecialObjectCount);


	//*--------------------------------------------------------------------------------
	if (gZodiacPtr != NULL)
	{
//		CONSOLE_DEBUG("Plotting zodiac");
		PlotSkyObjects(gZodiacPtr, gZodiac_names, zodiac_shapes, kZodiacCount);
	}


	//*--------------------------------------------------------------------------------
	//*	only display the alignment objects if we are zoomed
	if (cView_angle_Rads < 0.6)
	{
		PlotObjectsByDataSource(cDispOptions.dispSpecialObjects, gPolarAlignObjectPtr, gPolarAlignObjectCount);

		//*	figure out which display option we are doing.
		switch(cDispOptions.dispSpecialObjects)
		{
			case kSpecialDisp_Off:
				//*	do nothing
				break;

			case kSpecialDisp_All:
			case kSpecialDisp_Arcs_w_CentVect:
			case kSpecialDisp_Arcs_noLabels:
				//*	Draw a line connecting the centers
				LLG_PenSize(gST_DispOptions.LineWidth_ConstOutlines);
				LLG_SetColor(W_YELLOW);
				DrawPolarAlignmentCenterVector(gPolarAlignObjectPtr, gPolarAlignObjectCount);
				LLG_PenSize(1);
				//*	FALL THROUGH

			case kSpecialDisp_ArcsOnly:
				LLG_PenSize(gST_DispOptions.LineWidth_ConstOutlines);
				DrawPolarAlignmentCircles(gSpecialObjectPtr, gSpecialObjectCount);
				break;
		}
	}

	//*--------------------------------------------------------------------------------
	if (cView_angle_Rads < gAAVSO_maxViewAngle)
	{
		PlotObjectsByDataSource(cDispOptions.dispAAVSOalerts , gAAVSOalertsPtr, gAAVSOalertsCnt);
	}



	PlotSkyObjects(cPlanets, gPlanet_names, gPlanet_shapes, kPlanetObjectCnt);	//* planets

	LLG_SetColor(W_BLACK);
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cDisplayedStarCount\t=", cDisplayedStarCount);

	SetWidgetNumber(kSkyTravel_DisplayedStarCnt, cDisplayedStarCount);

	//*	if we are in chart mode, draw the "scale of miles"
	if (cChartMode)
	{
		DrawScale();
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::SetView_Angle(const double newViewAngle_radians)
{
double	myViewAngle;

//	CONSOLE_DEBUG(__FUNCTION__);
	cView_angle_Rads	=	newViewAngle_radians;
	cLN_view_angle		=	log(cView_angle_Rads);		//*	this is to speed up drawing
	cView_index			=	0;
	myViewAngle			=		RADIANS(kMinViewAngle_Degrees);
	while (myViewAngle < cView_angle_Rads)
	{
		cView_index++;
		myViewAngle	=	myViewAngle * kViewAngleMultipler;
	}

	SetWidgetNumber(kSkyTravel_Btn_ZoomLevel,	cView_index);
}

//*****************************************************************************
void	WindowTabSkyTravel::ZoomViewAngle(const int direction)
{
int		iii;
int		myDirection;
double	newView_angle;

	newView_angle	=	cView_angle_Rads;
	if (direction > 0)
	{
		for (iii=0; iii<direction; iii++)
		{
			newView_angle	=	newView_angle * kViewAngleMultipler;
		}
	}
	else if (direction < 0)
	{
		myDirection	=	- direction;

		for (iii=0; iii<myDirection; iii++)
		{
			newView_angle	=	newView_angle / kViewAngleMultipler;
		}
	}

	if (newView_angle < RADIANS(kMinViewAngle_Degrees))
	{
		newView_angle	=	RADIANS(kMinViewAngle_Degrees);
	}
	if (newView_angle > RADIANS(kMaxViewAngle_Degrees))
	{
		newView_angle	=	RADIANS(kMaxViewAngle_Degrees);
	}
	SetView_Angle(newView_angle);
}

//*****************************************************************************
void	WindowTabSkyTravel::SetMinimumViewAngle(const double minimumViewAngle)
{
int	loopCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_DBL("minimumViewAngle=", minimumViewAngle);
	loopCnt	=	0;
	while ((cView_angle_Rads < minimumViewAngle) && (loopCnt < 40))
	{
		CONSOLE_DEBUG_W_DBL("cView_angle_Rads (radians)=", cView_angle_Rads);
		ZoomViewAngle(+1);
		loopCnt++;
	}
	if (loopCnt > 0)
	{
		UpdateViewAngleDisplay();
//		ForceReDrawSky();
	}
	CONSOLE_DEBUG_W_NUM("loopCnt\t=",	loopCnt);
}

//*****************************************************************************
void	WindowTabSkyTravel::SetMaximumViewAngle(const double maximumViewAngle)
{
int	loopCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	loopCnt	=	0;
	while ((cView_angle_Rads > maximumViewAngle) && (loopCnt < 40))
	{
		ZoomViewAngle(-1);
		loopCnt++;
	}
	if (loopCnt > 0)
	{
		UpdateViewAngleDisplay();
//		ForceReDrawSky();
	}
	CONSOLE_DEBUG_W_NUM("loopCnt\t=",	loopCnt);
}


//*****************************************************************************
void	WindowTabSkyTravel::ResetView(void)
{
	SetView_Angle(kViewAngle_Default);

	cElev0									=	kHALFPI / 2;		//* 45 degrees
	cAz0									=	0.0;				//* north
	cRa0									=	0.0;
	cDecl0									=	0.0;

	//*	set auto advance time to on
	gAutoAdvanceTime						=	true;
	SetCurrentTime();
	cForceClockUpdate						=	true;		//*	force clock on screen to update
	cPreviousClockUpdateTVsecs				=	0;

	cChartMode								=	false;

	cDispOptions.dispAAVSOalerts			=	true;
	cDispOptions.dispAsteroids				=	false;
	cDispOptions.dispCommonStarNames		=	true;
 	cDispOptions.dispConstOutlines			=	true;
 	cDispOptions.dispConstellations			=	true;
	cDispOptions.dispDeep					=	true;
	cDispOptions.dispDefaultData			=	true;
	cDispOptions.dispDraper					=	false;
	cDispOptions.dispEarth					=	true;
	cDispOptions.dispGaia					=	true;
	cDispOptions.dispGrid					=	true;
	cDispOptions.dispHipparcos				=	false;
	cDispOptions.dispHYG_all				=	false;
	cDispOptions.dispHorizon_line			=	true;
 	cDispOptions.dispMessier				=	true;
	cDispOptions.dispNames					=	true;
	cDispOptions.dispNGC					=	true;
	cDispOptions.dispMilkyWayOutline		=	true;
	cDispOptions.dispMilkyWayFilled			=	false;
	cDispOptions.dispSAO					=	true;
	cDispOptions.dispSpecialObjects			=	kSpecialDisp_All;
 	cDispOptions.dispSymbols				=	true;
	cDispOptions.dispYale					=	false;

	gST_DispOptions.DayNightSkyColor		=	false;
	gST_DispOptions.DispMagnitude			=	true;
	gST_DispOptions.DispSpectralType		=	true;
	gST_DispOptions.DashedLines				=	false;
	gST_DispOptions.MagnitudeMode			=	kMagnitudeMode_Dynamic;
	gST_DispOptions.DisplayedMagnitudeLimit	=	15.0;

	cFaintLimit_B							=	4.5;
	cMaxRadius_D							=	20;

	if (gConstVecotrPtr != NULL)
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
int			skycolor;
int			skycolor1;		//* skycolor1 from sun elevation, 2 from solar eclipse
int			skycolor2;
double		r1,r2,d,d1,d2,theta1,theta2,a0,a1,a2;	//* 1 = sun 2 = moon
double		frac;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("earthFlag\t=",	earthFlag);

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
					frac	=	PI * ((r1 + r2)*(r1 - r2)) / a0;	//* annular total eclipse
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

		skycolor1			=	(int)(63. * (sunmonptr->sun_elev + 0.174)/ 0.244);
		if (skycolor1 < 0)
		{
			skycolor1	=	0;
		}
		if (skycolor > 63)
		{
			skycolor	=	63;
		}

		//* use the darker of the two sky colors
		skycolor	=	((skycolor1 > skycolor2) ? skycolor2 : skycolor1);
//		CONSOLE_DEBUG_W_NUM("skycolor\t=",	skycolor);
	}

	cSkyRGBvalue.red	=	0;
	cSkyRGBvalue.grn	=	0;
//	cSkyRGBvalue.blu	=	(skycolor * 4) << 8;
	cSkyRGBvalue.blu	=	(skycolor * 4);
//	CONSOLE_DEBUG_W_NUM("blu\t=",	cSkyRGBvalue.blu);


	SetWidgetBGColor(kSkyTravel_NightSky, CV_RGB(	cSkyRGBvalue.red,
													cSkyRGBvalue.grn,
													cSkyRGBvalue.blu));
	return(skycolor);
}



//*****************************************************************************
void	WindowTabSkyTravel::BuildConstellationData(void)
{
int		ii;
int		jj;
int		kk;
int		sCount;
int		vectorTabIdex;
long	totalStars;
long	starID;
long	ss;
long	cc;
int		maxVectorIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("sizeof(TYPE_Constelation)\t=",	sizeof(TYPE_Constelation));
	maxVectorIdx	=	0;
	ii				=	0;
	while (gConstel_names[ii][0] != '-')
	{
		ii++;
	}
	gConstelationCount	=	ii;

	if (gConstelationCount > 0)
	{
		totalStars		=	0;
		gConstelations	=	(TYPE_Constelation *)calloc(gConstelationCount, sizeof(TYPE_Constelation));
		if (gConstelations != NULL)
		{
			for (ii=0; ii < gConstelationCount; ii++)
			{
				sCount			=	0;
				vectorTabIdex	=	gValid_name[ii] & 0x0ff;

				if (vectorTabIdex != 99)
				{
					if (vectorTabIdex > maxVectorIdx)
					{
						maxVectorIdx	=	vectorTabIdex;
					}
//					CONSOLE_DEBUG("=========================================================");
//					CONSOLE_DEBUG_W_STR("gConstel_LongNames\t=",	gConstel_LongNames[ii]);
//					CONSOLE_DEBUG_W_NUM("vectorTabIdex\t\t=",	vectorTabIdex);

					//*	count the stars in the vector index
					jj	=	0;
					while ((gConstellationVecor[vectorTabIdex][jj] != 0) && (jj <= kConstVectCount))
					{
						jj++;
					}
					sCount		=	jj;
					totalStars	+=	sCount;

				}

//				strcpy(gConstelations[ii].name,				gConstel_names[ii]);
				strcpy(gConstelations[ii].name,				gConstel_LongNames[ii]);

				strncpy(gConstelations[ii].shortName,		gConstel_names[ii], 3);
				gConstelations[ii].shortName[4]				=	0;
				gConstelations[ii].indexIntoConstStarTable	=	vectorTabIdex | 0x8000;
				gConstelations[ii].starsInConstelation		=	sCount;
			}

			if (gStarDataPtr != NULL)
			{
				//*	now allocate an array for the constellation stars and FIND them
				gConstStarPtr	=	(TYPE_CelestData *)calloc((totalStars + 5), sizeof(TYPE_CelestData));
				if (gConstStarPtr != NULL)
				{
					gConstStarCount	=	totalStars;

					kk	=	0;
					for (ii=0; ii<kConstVectCount; ii++)
					{
						//*	find the constelation that points to this one
						for (cc=0; cc < gConstelationCount; cc++)
						{
							vectorTabIdex	=	gConstelations[cc].indexIntoConstStarTable;
							if (vectorTabIdex < 0)
							{
								if (ii == (vectorTabIdex & 0x7fff))
								{
									gConstelations[cc].indexIntoConstStarTable	=	kk;
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
									gConstStarPtr[kk].id	=	gConstellationVecor[ii][jj];
									//*	now find this id in the master list
									starID	=	gConstellationVecor[ii][jj] & 0x3fff;
									for (ss=0; ss<gStarCount; ss++)
									{
										if (starID == gStarDataPtr[ss].id)
										{
											//*	copy the entire star over
											gConstStarPtr[kk]	=	gStarDataPtr[ss];

											//*	but use the ID from the vector table
											gConstStarPtr[kk].id	=	gConstellationVecor[ii][jj];
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
//*	this is the original SkyTravel constellation lines
//*	there are much better lines and this is disabled by default if the better data is available
//*****************************************************************************
void	WindowTabSkyTravel::DrawConstellationLines(void)
{
int				ii;
int				jj;
int				kk;
double			xangle,yangle,rangle;
double			temp,angle;
double			alpha,aside,cside,gamma;
double			sin_bside,cos_bside;
double			sinRatio;
int				xPos, yPos;
int				firstStarInCurConstellation;
int				starsInCurConstellation;
unsigned int	idword;
bool			firstMove;
//int				linesDrawn;

//	CONSOLE_DEBUG(__FUNCTION__);
//	linesDrawn	=	0;

	if ((gConstelations != NULL) && (gConstStarPtr != NULL))
	{
		xangle		=	cView_angle_Rads / 2.0;
		yangle		=	xangle * cWind_height / cWind_width;
		rangle		=	sqrt((xangle * xangle) + (yangle * yangle));	//* the diagonal
		cRadmax		=	rangle;

		sin_bside	=	sin(kHALFPI - cDecl0);
		cos_bside	=	cos(kHALFPI - cDecl0);

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

			sinRatio=	sin(rangle)/sin(temp);
			if (sinRatio < 1.0)
			{
				cRamax	=	asin(sinRatio);
			}
		}
		cWind_x0	=	wind_ulx + (cWind_width / 2);
		cWind_y0	=	wind_uly + (cWind_height / 2);

		//*	compute pixels per radian
		cXfactor	=	cWind_width / cView_angle_Rads;
		cYfactor	=	cXfactor;	//* 1:1 aspect ratio

		LLG_SetColor(W_RED);
		for (jj=0; jj < gConstelationCount; jj++)
		{
			if ((gConstelations[jj].indexIntoConstStarTable >= 0) && (gConstelations[jj].starsInConstelation > 0))
			{

				firstStarInCurConstellation	=	gConstelations[jj].indexIntoConstStarTable;
				starsInCurConstellation		=	gConstelations[jj].starsInConstelation;
				firstMove					=	true;
				for (kk=0; kk<starsInCurConstellation; kk++)
				{
					ii		=	firstStarInCurConstellation + kk;
					alpha	=	cRa0 - gConstStarPtr[ii].ra;
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
						cside	=	kHALFPI - gConstStarPtr[ii].decl;
						aside	=	acos((cos_bside * cos(cside)) + (sin_bside * sin(cside) * cos(alpha)));
						if (aside < cRadmax)	//* within bounding circle?
						{
							if (aside > kEPSILON)
							{
								gamma	=	asin(sin(cside)*sin(alpha)/sin(aside));
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

							//*	compute x and y coordinates
							//* x	=	x0 + cXfactor * aside * cos(angle)
							//* y	=	y0 - cYfactor * aside * sin(angle) (minus sign is because plus y is down)

							xPos		=	cWind_x0 + (int)(cXfactor * aside*sin(angle));
							yPos		=	cWind_y0 - (int)(cYfactor * aside*cos(angle));
							idword		=	gConstStarPtr[ii].id;

							if (firstMove || ((idword & 0x4000) == 0x4000))
							{
								LLG_MoveTo(xPos, yPos);
								firstMove	=	false;
							}
							else
							{
								LLG_LineTo(xPos, yPos);
							//	linesDrawn++;
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
		LLG_SetColor(W_BLACK);
	}
//	CONSOLE_DEBUG_W_NUM("linesDrawn\t=",	linesDrawn);
//	CONSOLE_DEBUG("=========================================================");
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
		LLG_SetColor(W_DARKRED);
	}
	else
	{
		LLG_SetColor(W_DARKGREEN);
	}
	if ((gConstOutlinePtr != NULL) && (gConstOutlineCount > 0))
	{
		for (iii=0; iii<gConstOutlineCount; iii++)
		{
			//*	this makes it easier to follow the code
			myOutLineObj	=	&gConstOutlinePtr[iii];
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
						LLG_LineTo(pt_XX, pt_YY);
					}
					else
					{
						LLG_MoveTo(pt_XX, pt_YY);
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

			//------------------------------------------------------------------------------------
			//*	only draw the names of the outlines if the constellations are NOT displayed
			if (cDispOptions.dispNames && (cDispOptions.dispConstellations == false))
			{
				//*	only draw the name if we drew at least 1/4 of the points
				if (pointsDrawnCnt >= (totalPts / 4))
				{
					pt_XX	=	(minPixelX + maxPixelX) / 2;
					pt_YY	=	(minPixelY + maxPixelY) / 2;
					nameLen	=	strlen(myOutLineObj->constOutlineName);
					if (nameLen > 0)
					{
						DrawConstellationNameByViewAngle(	(pt_XX - (nameLen * 6)),
															pt_YY,
															myOutLineObj->constOutlineName);
					}
					else
					{
						LLG_DrawCString(pt_XX, pt_YY, myOutLineObj->shortName);
					}
				}
			}
			ptInView		=	GetXYfromRA_Decl(	myOutLineObj->rtAscension,
													myOutLineObj->declination,
													&pt_XX,
													&pt_YY);
			if (ptInView)
			{
				LLG_SetColor(W_PINK);
				LLG_DrawCString(pt_XX, pt_YY, myOutLineObj->shortName);
				LLG_SetColor(W_DARKGREEN);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("gConstOutlinePtr is null");
	}
}

//**************************************************************************************
void	WindowTabSkyTravel::DrawConstellationNameByViewAngle(	const int pt_XX,
																const int pt_YY,
																const char *theString)
{
	if (cView_angle_Rads < 2.5)
	{
		LLG_DrawCString(	pt_XX,
							pt_YY,
							theString,
							kFont_Triplex_Large);
	}
	else if (cView_angle_Rads < 9.0)
	{
		LLG_DrawCString(	pt_XX,
							pt_YY,
							theString,
							kFont_Triplex_Small);
	}
	else  if (cView_angle_Rads < 10.5)
	{
		LLG_DrawCString(	pt_XX,
						pt_YY,
						theString,
						kFont_Medium);
	}
}

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
int					nameLen;
short				deltaPixels;
//int					longestDelta	=	0;
//	CONSOLE_DEBUG(__FUNCTION__);

	if (gConstVecotrPtr != NULL)
	{
		if (cNightMode)
		{
			LLG_SetColor(W_RED);
		}
		else
		{
			LLG_SetColor(W_PINK);
		}
		ptsOnScreenCnt	=	0;
//		CONSOLE_DEBUG_W_NUM("gConstVectorCnt\t=",gConstVectorCnt);
		for (iii=0; iii<gConstVectorCnt; iii++)
		{

			myConstPtr		=	&gConstVecotrPtr[iii];

			LLG_SetColor(W_PINK);
//			if (strncasecmp(myConstPtr->constellationName, "CEN", 3) == 0)
//			{
//				LLG_SetColor(W_CYAN);
//			}

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
					if ((cView_angle_Rads > 2.6) && (prev_XX >= 0))
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
						LLG_MoveTo(pt_XX, pt_YY);
					}
					else
					{
						LLG_LineTo(pt_XX, pt_YY);
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

			//---------------------------------------------------------------------
			//*	Now draw the constellation name
			//CONSOLE_DEBUG_W_STR("myConstPtr->constellationName\t=",myConstPtr->constellationName);
			if (cDispOptions.dispNames)
			{
				nameLen	=	strlen(myConstPtr->constellationName);
				if (nameLen > 0)
				{
					if (fabs(myConstPtr->rtAscension) > 0.0)
					{
						ptInView		=	GetXYfromRA_Decl(	myConstPtr->rtAscension,
																myConstPtr->declination,
																&pt_XX,
																&pt_YY);
						if (ptInView)
						{
							//*	check the view angle to see how big to make the font
							DrawConstellationNameByViewAngle(	(pt_XX - (nameLen * 6)),
																pt_YY,
																myConstPtr->constellationName);
						}
					}
					else
					{
						CONSOLE_DEBUG_W_STR("No center point for", myConstPtr->constellationName);
						CONSOLE_ABORT(__FUNCTION__);
					}
				}
			}
		}
//		CONSOLE_DEBUG_W_NUM("ptsOnScreenCnt\t=",ptsOnScreenCnt);

	}
	else
	{
		CONSOLE_DEBUG("gConstVecotrPtr is null");
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
void	WindowTabSkyTravel::DrawCommonStarNames(void)
{
int		iii;
short	pt_XX, pt_YY;
bool	ptInView;

//	CONSOLE_DEBUG("-------------------------------------------------------");
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

	if ((gHipObjectPtr != NULL) && (gHipObjectCount > 0))
	{
		if (cNightMode)
		{
			LLG_SetColor(W_RED);
		}
		else
		{
			LLG_SetColor(W_WHITE);
		}
		for (iii = 0; iii < gHipObjectCount; iii++)
		{
			//*	most of them don't have names, so check for that first
			if (gHipObjectPtr[iii].longName[0] != 0)
			{
				ptInView		=	GetXYfromRA_Decl(	gHipObjectPtr[iii].ra,
														gHipObjectPtr[iii].decl,
														&pt_XX,
														&pt_YY);
				if (ptInView)
				{
					DrawStar_shape(pt_XX, pt_YY, 0);	//*	many of these stars are not drawn
//					CONSOLE_DEBUG(gHipObjectPtr[iii].longName);
					if (cDispOptions.dispHipparcos && (cView_angle_Rads <= 0.2))
					{
						pt_YY	+=	12;
					}
					LLG_DrawCString(pt_XX + 10, pt_YY, gHipObjectPtr[iii].longName);
				}
			}
		}
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTabSkyTravel::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabSkyTravel::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat		image_roi;
#endif // _USE_OPENCV_CPP_
cv::Rect	myCVrect;
TYPE_WIDGET	*theWidget;

//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;
	theWidget		=	&cWidgetList[widgetIdx];

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	cCursorOffsetY	=	theWidget->top;

	switch(widgetIdx)
	{
		case kSkyTravel_NightSky:
			if (cOpenCV_Image != NULL)
			{
				cCurrentColor	=	CV_RGB(	cSkyRGBvalue.red,
											cSkyRGBvalue.grn,
											cSkyRGBvalue.blu);
//				CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.red\t=", cSkyRGBvalue.red);
//				CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.grn\t=", cSkyRGBvalue.grn);
//				CONSOLE_DEBUG_W_NUM("cSkyRGBvalue.blu\t=", cSkyRGBvalue.blu);
				LLG_FillRect(theWidget->left, theWidget->top, theWidget->width, theWidget->height);

			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
				image_roi		=	cv::Mat(*cOpenCV_Image, myCVrect);
				cOpenCV_Image	=	&image_roi;
				DrawSkyAll();
				DrawWindowOverlays();
				cOpenCV_Image	=	openCV_Image;
			#elif (CV_MAJOR_VERSION <= 3)
				cvSetImageROI(cOpenCV_Image,  myCVrect);
				DrawSkyAll();
				DrawWindowOverlays();
				cvResetImageROI(cOpenCV_Image);
			#else
				#error "Not able to complete this operation"
			#endif // _USE_OPENCV_CPP_

				//*	this has to be done AFTER roi is reset
				Compute_cursor(&gCurrentSkyTime, &cCurrLatLon);
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
//***********************************************************************
int CelestObjDeclinationQsortProc(const void *e1, const void *e2)
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
		if (!gCurrentSkyTime.precflag && !forcePrecession)
		{
			if ((fabs(gCurrentSkyTime.fJulianDay - gCurrentSkyTime.timeOfLastPrec)) < gCurrentSkyTime.delprc)
			{
				CONSOLE_DEBUG("precess not necessary");
				return(false);	//*	precess not necessary
			}


			if ((fabs(gCurrentSkyTime.fJulianDay - JD2000)) < gCurrentSkyTime.delprc)
			{
				CONSOLE_DEBUG("precess not necessary");
				return(false);	//* return if precession not needed
			}
		}

		gCurrentSkyTime.precflag			=	false;			//*	force off
		gCurrentSkyTime.strflag				=	false;			//*	say year 2000 data not present
		gCurrentSkyTime.starDataModified	=	true;			//*	say year 2000 data not present
		gCurrentSkyTime.timeOfLastPrec		=	gCurrentSkyTime.fJulianDay;	//*	set new lst prc

		//* precession math starts
		epoch		=	gCurrentSkyTime.cent - 1.0;			//*!!!68000 source code says use 1-cent !!!

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

			aside	=	acos((cos_bside * cos(cside)) + (sin_bside * sin(cside) * cos(alpha)));

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

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

//	startTicks		=	TickCount();
	pressesOccurred	=	Precess(gStarDataPtr, gStarCount, kSortData, kPressionIfNeeded);

	//*	if precess occurred for the stars, we want to force it for the constellations
	if (pressesOccurred)
	{
//		CONSOLE_DEBUG("precess occurred");

		//*	NGC objects
		if ((gNGCobjectPtr != NULL) && (gNGCobjectCount > 0))
		{
			Precess(gNGCobjectPtr, gNGCobjectCount, kSortData, kForcePression);
		}

		//*	Yale start catalog
		if ((gYaleStarDataPtr != NULL) && (gYaleStarCount > 0))
		{
			Precess(gYaleStarDataPtr, gYaleStarCount, kSortData, kForcePression);
		}

		//*	Hippacos database
		if ((gHipObjectPtr != NULL) && (gHipObjectCount > 0))
		{
			Precess(gHipObjectPtr, gHipObjectCount, kSortData, kForcePression);
		}

//*	It appears that the Messier objects do not need to be precessed.
//*	Jan 23,	2022	<MLS> No longer precessing Messier objects data
		//*	Messier objects
		if ((gMessierObjectPtr != NULL) && (gMessierObjectCount > 0))
		{
			Precess(gMessierObjectPtr, gMessierObjectCount, kDoNotSort, kForcePression);
		}

		//*	Henry Draper catalog
		if ((gDraperObjectPtr != NULL) && (gDraperObjectCount > 0))
		{
			Precess(gDraperObjectPtr, gDraperObjectCount, kSortData, kForcePression);
		}


#ifdef _ENABLE_HYG_
		if ((gHYGObjectPtr != NULL) && (gHYGObjectCount > 0))
		{
			Precess(gHYGObjectPtr, gHYGObjectCount, kSortData, kForcePression);
		}
#endif


//*	special objects should not be precessed
		if ((gSpecialObjectPtr != NULL) && (gSpecialObjectCount > 0))
		{
			Precess(gSpecialObjectPtr, gSpecialObjectCount, kSortData, kForcePression);
		}

		if ((gConstStarPtr != NULL) && (gConstStarCount > 0))
		{
			Precess(gConstStarPtr, gConstStarCount, kDoNotSort, kForcePression);
		}

		if ((gAAVSOalertsPtr != NULL) && (gAAVSOalertsCnt > 0))
		{
			Precess(gAAVSOalertsPtr, gAAVSOalertsCnt, kDoNotSort, kForcePression);
		}

		if ((gSAOobjectPtr != NULL) && (gSAOobjectCount > 0))
		{
			Precess(gSAOobjectPtr, gSAOobjectCount, kSortData, kForcePression);
		}

//		endTicks		=	TickCount();
//		elapsedTicks	=	endTicks - startTicks;

//		sprintf(ticksMsg, "Ticks = %ld", elapsedTicks);
//		DisplayHelpMessage(ticksMsg);


#ifdef _ENABLE_ASTEROIDS_
		//----------------------------------------------------------
		//*	now do the asteroids
		if (gAsteroidPtr != NULL)
		{
		double	targetJulian;
//		double	degrees;
		double	solar_RA;
		double	solar_DEC;
		double	solar_Distance;
//		int		iii;


//			//*	do the precession on each asteroid
//			for (iii=0; iii<gAsteroidCnt; iii++)
//			{
//
//			}
			targetJulian	=	Julian_CalcFromDate(	gCurrentSkyTime.month,
														gCurrentSkyTime.day,
														gCurrentSkyTime.year);
			//http://astropixels.com/ephemeris/sun/sun2022.html
			//http://people.tamu.edu/~kevinkrisciunas/ra_dec_sun_2022.html
			solar_RA		=	cPlanets[SUN].ra;
			solar_DEC		=	cPlanets[SUN].decl;
			solar_Distance	=	cPlanetStruct[SUN].dist;
//					cPlanetStruct[SUN].ra,		//*	radians
//					cPlanetStruct[SUN].decl,	//*	radians
//					cPlanetStruct[SUN].dist);	//*	AU


			UpdateAsteroidEphemeris(	gAsteroidPtr,
										gAsteroidCnt,
										targetJulian,
										solar_RA,			//*	radians
										solar_DEC,			//*	radians
										solar_Distance);	//*	AU
//			PrintAsteroidEphemeris(		gAsteroidPtr,
//										targetJulian,
//										solar_RA,			//*	radians
//										solar_DEC,			//*	radians
//										solar_Distance);	//*	AU
		}
#endif // _ENABLE_ASTEROIDS_
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
				LLG_SetColor(W_RED);
			//	strcpy(symb, gConstel_names[constelNameIdx]);
			//	LLG_DrawCString(xcoord, ycoord, symb);
				LLG_DrawCString(xcoord, ycoord, gConstel_LongNames[constelNameIdx]);
			}
			else
			{
				SysBeep(1);
			}
			break;

		case ST_STAR:
			if (cNightMode)
			{
				LLG_SetColor(W_RED);
			}
		#ifdef _ENBABLE_WHITE_CHART_
			else if (cChartMode)
			{
				LLG_SetColor(W_BLACK);
			}
		#endif
			else
			{
				LLG_SetColor(W_WHITE);
			}
			DrawStar_shape(xcoord, ycoord, magn & 0x07);
			break;

		case ST_DEEP:
			{
				//*	new calculations, used to depend on view index
				scale	=	1 + (8 / (cView_angle_Rads * 30.0));
				switch(magn & 0xf0)	//*	isolate hi 4 bits
				{
					case 0x10:	//*	globular
						DrawVector(W_LIGHTGRAY, xcoord, ycoord, scale, globular_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x20:	//*	elliptical
						DrawVector(W_BLUE, xcoord, ycoord, scale, elliptical_shapes[magn & 0x0f]);		//*	draw the object
						break;

					case 0x30:	//*	spiral
						DrawVector(W_GREEN, xcoord, ycoord, scale, spiral_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x40:	//*	planetary
						DrawVector(W_CYAN, xcoord, ycoord, scale, planetary_shapes[magn & 0x0f]);	//*	draw the object
						break;

					case 0x50:	//	*bright diffuse
						DrawVector(W_RED, xcoord, ycoord, scale, bdn_shapes[magn & 0x0f]);			//*	draw the object
						break;

					case 0x60:	//*	open galactic
						DrawVector(W_MAGENTA, xcoord, ycoord, scale, ogc_shapes[magn & 0x0f]);		//*	draw the object
						break;

					case 0x70:	//*	smc/lmc outline
						LLG_SetColor(W_GREEN);
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
							LLG_SetColor(W_LIGHTGRAY);
						}
						else
						{
							CONSOLE_ABORT("Magellanic Cloud");
						}
						LLG_DrawCString(xcoord, ycoord, symb);
						break;
				}
			}
			break;

		case ST_ALWAYS:
			CONSOLE_ABORT("ST_ALWAYS");
			break;
	}
	cDisplayedStarCount++;

}

//*****************************************************************************
static int	GetColorFromChar(const char theChar)
{
int	theColor;

	switch(theChar)
	{
		case 'C':	theColor	=	W_CYAN;		break;
		case 'D':	theColor	=	W_RED;		break;
		case 'E':	theColor	=	W_BLUE;		break;
		case 'F':	theColor	=	W_GREEN;	break;
		case 'G':	theColor	=	W_GREEN;	break;
		case 'H':	theColor	=	W_MAGENTA;	break;
		case 'I':	theColor	=	W_MAGENTA;	break;
		case 'J':	theColor	=	W_RED;		break;
		case 'K':	theColor	=	W_RED;		break;

		default:	theColor	=	W_WHITE;	break;
	}
	return(theColor);
}

//*****************************************************************************
//*	in order to speed up the drawing of the stars, use a table for the colors
//*	the index starts at 0
//*****************************************************************************
static char	gOBAFGKM_colorTable[128]	=
{
		//	00-0f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	10-1f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	20-2f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	30-3f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	40-4f
		W_WHITE,		//*	@
		W_STAR_A,		//*	A
		W_STAR_B,		//*	B
		W_WHITE,		//*	C
		W_WHITE,		//*	D
		W_WHITE,		//*	E
		W_STAR_F,		//*	F
		W_STAR_G,		//*	G
		W_WHITE,		//*	H
		W_WHITE,		//*	I
		W_WHITE,		//*	J
		W_STAR_K,		//*	K
		W_WHITE,		//*	L
		W_STAR_M,		//*	M
		W_WHITE,		//*	N
		W_STAR_O,		//*	O

		//*	50-5f
		W_WHITE,		//*	P
		W_WHITE,		//*	Q
		W_WHITE,		//*	R
		W_WHITE,		//*	S
		W_WHITE,		//*	T
		W_WHITE,		//*	U
		W_WHITE,		//*	V
		W_WHITE,		//*	W
		W_WHITE,		//*	X
		W_WHITE,		//*	Y
		W_WHITE,		//*	z
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	60-6f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,

		//*	70-7f
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE,
		W_WHITE
};


//*****************************************************************************
void	WindowTabSkyTravel::AdjustFaintLimit(const double adjustmentAmount)
{
char	statusMsg[128];

	cFaintLimit_B	+=	adjustmentAmount;
	if (cFaintLimit_B < 4.0)
	{
		cFaintLimit_B	=	4.0;
	}
	if (cFaintLimit_B > 13.5)
	{
		cFaintLimit_B	=	13.5;
	}
	sprintf(statusMsg, "Faint limit set to %2.2f", cFaintLimit_B);
	SetWidgetText(		kSkyTravel_MsgTextBox, statusMsg);
}


//*****************************************************************************
void	WindowTabSkyTravel::DrawStarFancy(	const int		xcoord,
											const int		ycoord,
											TYPE_CelestData	*theStar,
											int				textColor,
											double			viewAngle_LabelDisplay,
											double			viewAngle_InfoDisplay)
{
int		theStarColor;
int		starRadiusPixels;
int		restoreColor;
bool	starWasDrawn;
double	majAxis_Rad;		//*	these are for OpenNGC drawing
double	minAxis_Rad;
int		majAxisPixels;
int		minAxisPixels;

//	CONSOLE_DEBUG(__FUNCTION__);
	majAxisPixels	=	0;
	minAxisPixels	=	0;

	//*	get the spectral color
	if (cNightMode)
	{
		theStarColor	=	W_RED;
		restoreColor	=	W_RED;
	}
	else
	{
		//*	look up the spectral color of the star
		//*	it is done with a table to make it faster
		//*	the defaults in the table are all white
		theStarColor	=	gOBAFGKM_colorTable[theStar->spectralClass & 0x6F];
		restoreColor	=	W_WHITE;
	}
	LLG_SetColor(theStarColor);

	starRadiusPixels	=	0;
	starWasDrawn		=	false;


	if (theStar->realMagnitude > -10.0)	//*	check for valid magnitude
	{
	#define		kSlope_A				(-1.93)
	#define		kBrightLimit_C		(-3.6769)
	double		b_minus_c;


		//*	from Clif 1/23/2022
		//*		starRadius = D * (A * LN(field) + B - mag) / (B - C)
		//*		A	=	Slope		=	-1.93
		//*		B	=	FaintLimit	=	13.60708
		//*		C	=	BrightLimit	=	-3.6769
		//*		D	=	MaxRadius	=	20

		//*	cView_angle_Rads is in radians
		//*	20 is the limiting value (max size)
		b_minus_c	=	(cFaintLimit_B - kBrightLimit_C);

		//*	cLN_view_angle is the natural log of view angle, it is updated whenever cView_angle_Rads is changed
		//*	It is done this way so we dont have to compute log(cView_angle_Rads) for every star
		//*		see SetView_Angle()
		switch(theStar->dataSrc)
		{
			case kDataSrc_Messier:
				starRadiusPixels	=	cMaxRadius_D *
										(-1.93 * cLN_view_angle + 4.5224 - theStar->realMagnitude) / 8.1993;
				break;

			case kDataSrc_OpenNGC:
				starRadiusPixels		=	0;
				majAxis_Rad				=	RADIANS(theStar->oNGC_MajAxis_arcmin / 60.0);
				minAxis_Rad				=	RADIANS(theStar->oNGC_MinAxis_arcmin / 60.0);
				if ((majAxis_Rad > 0.0) && (minAxis_Rad > 0.0))
				{
					//*	the values for ellipse are RADIUS, divide by 2
					majAxisPixels		=	(cXfactor * majAxis_Rad) / 2;
					minAxisPixels		=	(cXfactor * minAxis_Rad) / 2;
					if (minAxisPixels > 1)
					{
						starRadiusPixels	=	1;
					}
					else
					{
						starRadiusPixels	=	cMaxRadius_D *
												((kSlope_A * cLN_view_angle) + cFaintLimit_B - theStar->realMagnitude) /
												(b_minus_c);
					}
				}
				break;

			default:
				//*		starRadius = D * (A * LN(field) + B - mag) / (B - C)
				starRadiusPixels	=	cMaxRadius_D * ((kSlope_A * cLN_view_angle) + cFaintLimit_B - theStar->realMagnitude) /
										(b_minus_c);
				break;
		}
	}
#ifdef _USE_LARGE_FONT_FOR_PUBLICATION_
	//*	this is temporary code to generate pictures for publication
	//*	July 21, 2022
	if (theStar->dataSrc == kDataSrc_GAIA_SQL)
	{
		if (starRadiusPixels > 0)
		{
			LLG_FillEllipse(xcoord, ycoord, 2, 2);
			starWasDrawn	=	true;
		}
	}
	else
#endif // _USE_LARGE_FONT_FOR_PUBLICATION_

//	if (strncasecmp(theStar->longName, "NGC4406", 7) == 0)
//	{
//		CONSOLE_DEBUG_W_STR("name         \t=",	theStar->longName);
//		CONSOLE_DEBUG_W_NUM("majAxPix     \t=",	majAxisPixels);
//		CONSOLE_DEBUG_W_NUM("minAxPix     \t=",	minAxisPixels);
//		CONSOLE_DEBUG_W_NUM("starRadiusPix\t=",	starRadiusPixels);
//	}

	if (starRadiusPixels > 0)
	{
		switch(theStar->dataSrc)
		{
			case kDataSrc_Messier:
				LLG_SetColor(textColor);
				LLG_FrameEllipse(xcoord, ycoord, starRadiusPixels, starRadiusPixels);
				break;

			case kDataSrc_OpenNGC:
				if ((majAxisPixels > 1) && (minAxisPixels > 1))
				{
				double	rotationAngle_degs;
				double	northAngle_degs;
				bool	valid2ndPt;
				short	xxx1, yyy1;
				double	decOffset_rads;
				int		loopCnt;

					//*	create a new point that is NORTH of the original point to determine
					//*	the drawing angle
					northAngle_degs	=	0.0;
					valid2ndPt		=	false;
					//*	we need a valid point to determine NORTH
					decOffset_rads	=	majAxis_Rad;
					loopCnt			=	0;
					while ((valid2ndPt == false) && (loopCnt < 10))
					{
						//*	if the point was not on the screen, try the minor axis
						valid2ndPt		=	GetXYfromRA_Decl(	theStar->ra,
																theStar->decl + decOffset_rads,
																&xxx1,
																&yyy1);
						decOffset_rads	=	decOffset_rads / 2.0;
						loopCnt++;
					}
					LLG_PenSize(gST_DispOptions.LineWidth_NGCoutlines);
					if (valid2ndPt)
					{
						northAngle_degs	=	DEGREES(atan2((yyy1 - ycoord), (xxx1 - xcoord)));
						//*	DEBUGGING:	Draw the NORTH vector
						LLG_SetColor(W_GREEN);
						LLG_MoveTo(xcoord, ycoord);
						LLG_LineTo(xxx1, yyy1);
//						LLG_DrawCString(xxx1, yyy1 + 10, "N", kFont_Medium);

						rotationAngle_degs	=	northAngle_degs - 90.0 + theStar->oNGC_PosAng_degs;
//						if (strncasecmp(theStar->longName, "NGC0224", 7) == 0)
//						{
//							CONSOLE_DEBUG_W_STR("name    \t=",	theStar->longName);
//							CONSOLE_DEBUG_W_DBL("tilt    \t=",	theStar->oNGC_PosAng_degs);
//							CONSOLE_DEBUG_W_DBL("north   \t=",	northAngle_degs);
//							CONSOLE_DEBUG_W_DBL("rotate  \t=",	rotationAngle_degs);
//							CONSOLE_DEBUG_W_NUM("majAxPix\t=",	majAxisPixels);
//							CONSOLE_DEBUG_W_NUM("minAxPix\t=",	minAxisPixels);
//						}
						LLG_SetColor(W_RED);
	//					LLG_FrameEllipse(xcoord, ycoord, majAxisPixels, minAxisPixels, 0.0);
						LLG_FrameEllipse(xcoord, ycoord, majAxisPixels, minAxisPixels, rotationAngle_degs);
						LLG_PenSize(1);
					}
					else if (starRadiusPixels > 0)
					{
						LLG_FillEllipse(xcoord, ycoord, starRadiusPixels, starRadiusPixels);
					}
				}
				else if (starRadiusPixels > 0)
				{
					LLG_FillEllipse(xcoord, ycoord, starRadiusPixels, starRadiusPixels);
				}
				break;

			default:
				LLG_FillEllipse(xcoord, ycoord, starRadiusPixels, starRadiusPixels);
				break;
		}
		starWasDrawn	=	true;
	}
	else if (starRadiusPixels == 0)
	{
		LLG_Putpixel(xcoord,	ycoord,		theStarColor);
		starWasDrawn	=	true;
	}
	else
	{
		//*	figure out what to do based on the display mode
		switch (gST_DispOptions.MagnitudeMode)
		{
			case kMagnitudeMode_Dynamic:
				//*	do nothing, already taken care of
				break;

			case kMagnitudeMode_Specified:
				if (theStar->realMagnitude <= gST_DispOptions.DisplayedMagnitudeLimit)
				{
					//*	Clif ???????????????????????????????????????????????
					LLG_Putpixel(xcoord,	ycoord,		theStarColor);
					starWasDrawn	=	true;
				}
				break;

			case kMagnitudeMode_All:
				LLG_Putpixel(xcoord,	ycoord,		theStarColor);
				starWasDrawn	=	true;
				break;
		}
	}

	LLG_SetColor(restoreColor);


	if (starWasDrawn)
	{
		cDisplayedStarCount++;
		DrawStarFancy_Label(	xcoord,
								ycoord,
								starRadiusPixels,
								theStar,
								textColor,
								viewAngle_LabelDisplay,
								viewAngle_InfoDisplay);
	}
}

//*****************************************************************************
void	WindowTabSkyTravel::DrawAsteroidFancy(	const int		xcoord,
												const int		ycoord,
												TYPE_CelestData	*theStar,
												int				textColor,
												double			viewAngle_LabelDisplay,
												double			viewAngle_InfoDisplay)
{
int		theStarColor;
int		starRadiusPixels;
int		asteroidHt;
int		asteroidWd;
int		restoreColor;
bool	starWasDrawn;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cNightMode)
	{
		theStarColor	=	W_RED;
		restoreColor	=	W_RED;
	}
	else
	{
		theStarColor	=	W_CYAN;
		restoreColor	=	W_WHITE;
	}
	LLG_SetColor(theStarColor);

	starRadiusPixels	=	0;
	asteroidWd			=	0;
	starWasDrawn		=	false;


	if (theStar->realMagnitude > -10)	//*	check for valid magnitude
	{
		starRadiusPixels	=	20 * (-1.93 * cLN_view_angle + 4.5224 - theStar->realMagnitude) / 8.1993;
	}
	if (starRadiusPixels > 0)
	{
		asteroidWd	=	starRadiusPixels * 3;
		asteroidHt	=	starRadiusPixels / 2;
		if (asteroidHt < 1)
		{
			asteroidHt	=	1;
		}
		LLG_FillEllipse(xcoord, ycoord, asteroidWd, asteroidHt);
		starWasDrawn	=	true;
	}
	else if (starRadiusPixels == 0)
	{
		LLG_Putpixel(xcoord,	ycoord,		theStarColor);
		LLG_Putpixel(xcoord+1,	ycoord,		theStarColor);
		LLG_Putpixel(xcoord-1,	ycoord,		theStarColor);
		LLG_Putpixel(xcoord+2,	ycoord,		theStarColor);
		LLG_Putpixel(xcoord-2,	ycoord,		theStarColor);
		starWasDrawn	=	true;
	}
	else
	{
		switch (gST_DispOptions.MagnitudeMode)
		{
			case kMagnitudeMode_Dynamic:
				//*	do nothing, already taken care of
				break;

			case kMagnitudeMode_Specified:
				if (theStar->realMagnitude <= gST_DispOptions.DisplayedMagnitudeLimit)
				{
					LLG_Putpixel(xcoord,	ycoord,		theStarColor);
					starWasDrawn	=	true;
				}
				break;

			case kMagnitudeMode_All:
				if (starRadiusPixels == 0)
				{
					//*	make the above star a bit bigger
					LLG_Putpixel(xcoord,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord+1,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord-1,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord+2,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord-2,	ycoord,		theStarColor);
					starWasDrawn	=	true;
				}
				else
				{
					LLG_Putpixel(xcoord,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord+1,	ycoord,		theStarColor);
					LLG_Putpixel(xcoord-1,	ycoord,		theStarColor);
					starWasDrawn	=	true;
				}
				break;
		}
	}

	LLG_SetColor(restoreColor);


	if (starWasDrawn)
	{
		cDisplayedStarCount++;
		DrawStarFancy_Label(	xcoord,
								ycoord,
								asteroidWd,
								theStar,
								textColor,
								viewAngle_LabelDisplay,
								viewAngle_InfoDisplay);
	}
}

//#define	kLineSpacingPixels	11
#define	kLineSpacingPixels	13


//*****************************************************************************
void	WindowTabSkyTravel::DrawStarFancy_Label(	const int		xcoord,
													const int		ycoord,
													const int		starRadiusPixels,
													TYPE_CelestData	*theStar,
													int				textColor,
													double			viewAngle_LabelDisplay,
													double			viewAngle_InfoDisplay)

{
char	labelString[32];
int		myXcoord;
int		myYcoord;
bool	starHasMag_and_Spectral;
int		fontIndex;
int		myLineSpacing;

//	fontIndex		=   kFont_RadioBtn;
	fontIndex		=   kFont_TextList;

	myLineSpacing	=	kLineSpacingPixels;
	myXcoord		=	xcoord;
	myYcoord		=	ycoord;
	starHasMag_and_Spectral	=	true;
	if (starRadiusPixels > 0)
	{
		myXcoord	+=	starRadiusPixels;
	}

	if (theStar->dataSrc == kDataSrc_Messier)
	{
		starHasMag_and_Spectral	=	false;
	}

#ifdef _USE_LARGE_FONT_FOR_PUBLICATION_
	if ((theStar->dataSrc == kDataSrc_GAIA_SQL) || (theStar->dataSrc == kDataSrc_AAVSOalert))
	{
		fontIndex		=   kFont_Large;
		myLineSpacing	=	2 * kLineSpacingPixels;
	}
#endif // _USE_LARGE_FONT_FOR_PUBLICATION_
	//------------------------------------------------------
	//*	draw the name if needed
	if (cDispOptions.dispNames && (cView_angle_Rads < viewAngle_LabelDisplay))
	{
		labelString[0]	=	0;
		switch(theStar->dataSrc)
		{
			case kDataSrc_Messier:
				strcpy(labelString, theStar->shortName);
				if (cView_angle_Rads < 0.4)
				{
					strcat(labelString, " - ");
					strcat(labelString, theStar->longName);
				}
				break;

			case kDataSrc_Hipparcos:
				//*	draw common H numbers and names if present
				sprintf(labelString, "H%ld", theStar->id);

				if (theStar->longName[0] > 0x20)
				{
					strcat(labelString, "-");
					strcat(labelString, theStar->longName);
				}
				break;

			//debugging
//				case kDataSrc_AAVSOalert:
//					strcpy(labelString, "foo");
//					break;
//
			case kDataSrc_NGC2000IC:
				//*	this is a special case, NGC and IC are in the same database.
				textColor	=	W_CYAN;
				//*	fall through to default
			default:
				strcpy(labelString, theStar->longName);
				break;
		}
		LLG_SetColor(textColor);


		if (labelString[0] > 0x20)
		{
			LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
			myYcoord	+=	myLineSpacing;
		}
		else if (theStar->shortName[0] > 0x20)
		{
			LLG_DrawCString(myXcoord + 10, myYcoord, theStar->shortName, fontIndex);
			myYcoord	+=	myLineSpacing;
		}

		//----------------------------------------------
		//*	AAVSO alerts
		if (theStar->dataSrc == kDataSrc_AAVSOalert)
		{
//			CONSOLE_DEBUG_W_STR("labelString\t=",		labelString);
			if (cView_angle_Rads < 0.4)
			{
				if (theStar->id > 0)
				{
					sprintf(labelString, "Alert#%ld", theStar->id);
					LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
					myYcoord	+=	myLineSpacing;
				}
			}
		}
	}

	//------------------------------------------------------
	//*	check for magnitude and spectral type
	if (starHasMag_and_Spectral && (cView_angle_Rads < viewAngle_InfoDisplay))
	{
		LLG_SetColor(textColor);
		//*	Are we are drawing the magnitude of the star
		if (gST_DispOptions.DispMagnitude)
		{
			if (theStar->realMagnitude > -10.0)
			{
				sprintf(labelString, "%3.1f", theStar->realMagnitude);
				LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
				myYcoord	+=	myLineSpacing;
			}
		}

		//*	Are we are drawing the spectral class identifier
		if (gST_DispOptions.DispSpectralType)
		{
			//*	see if there is spectral data
			if (theStar->spectralClass > 0)
			{
				labelString[0]	=	theStar->spectralClass;
				labelString[1]	=	0;
				LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
				myYcoord		+=	myLineSpacing;
				if (!isalpha(theStar->spectralClass))
				{
//					DumpCelestDataStruct(__FUNCTION__, theStar);
				}
			}
		}
	}

	//----------------------------------------------
	//*	Parallax
	if (gST_DispOptions.DispParallax && (theStar->parallax > 0.00000000000000001) && (cView_angle_Rads < viewAngle_InfoDisplay))
	{
		LLG_SetColor(W_GREEN);
		sprintf(labelString, "pX=%+8.5f", theStar->parallax);
		LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
		myYcoord	+=	myLineSpacing;
	}

	//----------------------------------------------
	//*	proper motion
	if (gST_DispOptions.DispProperMotion && theStar->propMotionValid && (cView_angle_Rads < viewAngle_InfoDisplay))
	{
		LLG_SetColor(W_PINK);
		sprintf(labelString, "pmR=%+8.4f", theStar->propMotion_RA_mas_yr);
		LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
		myYcoord	+=	myLineSpacing;

		sprintf(labelString, "pmD=%+8.4f", theStar->propMotion_DEC_mas_yr);
		LLG_DrawCString(myXcoord + 10, myYcoord, labelString, fontIndex);
		myYcoord	+=	myLineSpacing;
	}

	//*	debugging to see the number of stars on the screen
//	if ((gST_DispOptions.DispMagnitude == false) && (gST_DispOptions.DispSpectralType == false))
//	{
//		sprintf(labelString, "%d", cDisplayedStarCount);
//		LLG_DrawCString(myXcoord + 10, myYcoord, labelString);
//	}
}


//*****************************************************************************
int	WindowTabSkyTravel::SetStarTextColorAndViewAngle(int dataSource)
{
int		textColor;

	//-----------------------------------------------------
	//*	we have the data source, do this one time so we dont have to do it for each star
	//*		determine:
	//*			font color
	//*			view angle limit for name display

	//*	cViewAngle_LabelDisplay		==>	name
	//*	cViewAngle_InfoDisplay		==> Magnitude and spectral info

	cViewAngle_InfoDisplay	=	0.7;
	switch(dataSource)
	{
		case kDataSrc_Messier:
			textColor				=	W_CYAN;
			cViewAngle_LabelDisplay	=	2.0;
			break;

		case kDataSrc_Hipparcos:
			textColor				=	W_RED;
			cViewAngle_LabelDisplay	=	0.2;
			break;

		case kDataSrc_OpenNGC:
			textColor				=	W_ORANGE;
			cViewAngle_LabelDisplay	=	0.35;
			break;

		case kDataSrc_NGC2000:
			textColor				=	W_YELLOW;
			cViewAngle_LabelDisplay	=	0.35;
			break;

		case kDataSrc_NGC2000IC:
			textColor				=	W_CYAN;
			cViewAngle_LabelDisplay	=	0.35;
			break;

		case kDataSrc_HYG:
			textColor				=	W_RED;
			cViewAngle_LabelDisplay	=	0.2;
			break;

		case kDataSrc_Draper:
			textColor				=	W_GREEN;
			cViewAngle_LabelDisplay	=	0.1;
			break;

		case kDataSrc_YaleBrightStar:
			textColor				=	W_WHITE;
			cViewAngle_LabelDisplay	=	1.0;
			break;

		case kDataSrc_Special:
			textColor				=	W_WHITE;
			cViewAngle_LabelDisplay	=	1.0;
			break;

		case kDataSrc_PolarAlignCenter:
			textColor				=	W_WHITE;
			cViewAngle_LabelDisplay	=	1.0;
			break;

		case kDataSrc_AAVSOalert:
			textColor				=	W_YELLOW;
			cViewAngle_LabelDisplay	=	2.5;
			break;

		case kDataSrc_GAIA_SQL:
			textColor				=	W_ORANGE;
			cViewAngle_LabelDisplay	=	0.0005;
			cViewAngle_InfoDisplay	=	0.001;
			break;

		case kDataSrc_SAO:
			textColor				=	W_LIGHTMAGENTA;
			cViewAngle_LabelDisplay	=	0.05;
			cViewAngle_InfoDisplay	=	0.02;
			break;


		default:
			textColor				=	W_WHITE;
			cViewAngle_LabelDisplay	=	1.0;
			break;
	}
	if (cNightMode)
	{
		//*	if night mode, override the text color
		textColor				=	W_RED;
	}
	return(textColor);
}


//*****************************************************************************
long	WindowTabSkyTravel::Search_and_plot(TYPE_CelestData	*objectptr,
											long			maxObjects,
											bool			dataIsSorted)
{
bool				goflag;
int					iii;
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
int					myFontIdx;
int					textColor;
int					myColor;


//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("cDecl0\t\t=",		cDecl0);
//	CONSOLE_DEBUG_W_NUM("gST_DispOptions.MagnitudeMode\t=",		gST_DispOptions.MagnitudeMode);

#ifdef _ENBABLE_WHITE_CHART_
	if (cChartMode)
	{
		LLG_SetColor(W_BLACK);
	}
	else
#endif // _ENBABLE_WHITE_CHART_
	{
		LLG_SetColor(W_WHITE);
	}

	myCount			=	0;
	xangle			=	cView_angle_Rads / 2.0;
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

	cXfactor	=	cWind_width / cView_angle_Rads;
	cYfactor	=	cXfactor;	//* 1:1 aspect ratio

	//*	the database is sorted by declination, from high value (north pole) to low
	//*	value (south pole)
	//*	skip quickly until we find the first star within our current display

	//* continue scan until decl < decmin
	iii		=	0;
	if (dataIsSorted)
	{
		while ((objectptr[iii].decl > cDecmax) && (iii < maxObjects))	//* skip down to where decl < cDecmax
		{
			objectptr[iii].curXX	=	-100;
			objectptr[iii].curYY	=	-100;
			iii++;
		}
	}
	//*	debuging code
	if (iii > maxObjects)
	{
		CONSOLE_DEBUG_W_LONG("maxObjects\t=", maxObjects);
	}

	//-----------------------------------------------------
	//*	we have the data source, do this one time so we dont have to do it for each star
	//*		determine:
	//*			font color
	//*			view angle limit for name display
	textColor	=	SetStarTextColorAndViewAngle(dataSource);


	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//*	main loop through the stars
	while (iii < maxObjects)
	{
		goflag	=	true;
		objectptr[iii].curXX	=	-100;
		objectptr[iii].curYY	=	-100;

		magn		=	0;
		shape		=	ST_ALWAYS;
		if (dataSource == kDataSrc_Orginal)
		{
			magn	=	objectptr[iii].magn & 0x00ff;

			//*	this is part of the original database processing
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

			switch(shape)
			{
				case ST_NAME: if (!cDispOptions.dispNames)	goflag	=	false; break;
				case ST_DEEP: if (!cDispOptions.dispDeep)	goflag	=	false; break;
	//?			case ST_STAR: if (magn < cMagmin)			goflag	=	false; break;
			}
		}

		//*	check for displayed magnitude limits
//		if ((gST_DispOptions.MagnitudeMode != kMagnitudeMode_All) &&
//			(objectptr[iii].realMagnitude > gST_DispOptions.DisplayedMagnitudeLimit))
//		{
//			goflag	=	false;
//		}


		if (goflag)	//* try to plot it
		{
			//*	calculate the screen coordinates
			alpha	=	cRa0 - objectptr[iii].ra;
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
				cside	=	kHALFPI - objectptr[iii].decl;

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
						objectptr[iii].curXX	=	xcoord;
						ycoord					=	cWind_y0 - (cYfactor * aside * cos(angle));
						if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
						{
							objectptr[iii].curYY	=	ycoord;

							if (dataSource == kDataSrc_Orginal)
							{
								DrawObjectByShape(xcoord, ycoord, shape, magn);

//								char		idNameString[64];
//								sprintf(idNameString, "#%d", objectptr[iii].id);
//								LLG_DrawCString(xcoord + 3, ycoord + 10, idNameString, 1);

							}
							else
							{
								DrawStarFancy(	xcoord,
												ycoord,
												&objectptr[iii],
												textColor,
												cViewAngle_LabelDisplay,
												cViewAngle_InfoDisplay);
							}

							switch(dataSource)
							{
								case kDataSrc_Special:
									if (cView_angle_Rads < 0.2)
									{
										myFontIdx	=	kFont_Large;
									}
									else if (cView_angle_Rads < 0.7)
									{
										myFontIdx	=	kFont_Medium;
									}
									else
									{
										myFontIdx	=	1;
									}
									switch(cDispOptions.dispSpecialObjects)
									{
										case kSpecialDisp_All:
											LLG_DrawCString(xcoord + 3, ycoord + 10, objectptr[iii].longName, myFontIdx);
											break;
									}
									break;

								case kDataSrc_PolarAlignCenter:
									myColor	=	GetColorFromChar(objectptr[iii].longName[0]);
									LLG_SetColor(myColor);
									if (cView_angle_Rads < 0.1)
									{
										myFontIdx	=	kFont_Large;
									}
									else if (cView_angle_Rads < 0.4)
									{
										myFontIdx	=	kFont_Medium;
									}
									else
									{
										myFontIdx	=	1;
									}
									switch(cDispOptions.dispSpecialObjects)
									{
										case kSpecialDisp_All:
										case kSpecialDisp_Arcs_w_CentVect:
											LLG_DrawCString(xcoord + 3, ycoord + 10, objectptr[iii].longName, myFontIdx);
											break;
									}
									break;

							}
							myCount++;
						}
					}
				}
			}
		}
		iii++;

		if (dataIsSorted && (objectptr[iii].decl < cDecmin))
		{
			break;
		}
	}

	//*	make sure any remaining stars are set back to not on screen
	while (iii < maxObjects)
	{
		objectptr[iii].curXX	=	-100;
		objectptr[iii].curYY	=	-100;
		iii++;
	}

//	CONSOLE_DEBUG_W_NUM("myCount\t\t=",		myCount);
	return(myCount);
}
//magn

//*****************************************************************************
//* draw an object described by vectors at position x,y and color
void	WindowTabSkyTravel::DrawVector(	short		color,
										short		xx,
										short		yy,
										short		scale,
										const char	*shape_data)
{
unsigned char	movedata;
unsigned int	deltaMoves;
unsigned int	movetype;
unsigned int	nn;
short			myScale;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("scale\t=", scale);

	if (cNightMode)
	{
		LLG_SetColor(W_RED);
	}
	else
	{
		LLG_SetColor(color);
	}

	myScale	=	scale;
	if (myScale < 1)
	{
		myScale	=	1;
	}
	LLG_MoveTo(xx, yy);

	nn	=	0;
	while ((movedata = shape_data[nn++]) != 0)	//* loop until a zero byte is reached
	{
		deltaMoves	=	(movedata & 0x0f) * myScale;
		movetype	=	((movedata) & 0x00f0) >> 4;	//* extract move type
		switch(movetype & 0x0f)
		{
			case 0x00:
			case 0x01:
				xx	+=	deltaMoves;
				break;

			case 0x02:
			case 0x03:
				xx	+=	deltaMoves;
				yy	-=	deltaMoves;
				break;

			case 0x04:
			case 0x05:
				yy	-=	deltaMoves;
				break;

			case 0x06:
			case 0x07:
				xx	-=	deltaMoves;
				yy	-=	deltaMoves;
				break;

			case 0x08:
			case 0x09:
				xx	-=	deltaMoves;
				break;

			case 0x0a:
			case 0x0b:
				xx	-=	deltaMoves;
				yy	+=	deltaMoves;
				break;

			case 0x0c:
			case 0x0d:
				yy	+=	deltaMoves;
				break;

			case 0x0e:
			case 0x0f:
				xx	+=	deltaMoves;
				yy	+=	deltaMoves;
				break;
		}
		if (movetype & 0x01)
		{
			LLG_MoveTo(xx, yy);
		}
		else
		{
			LLG_LineTo(xx, yy);
		}
	}
}

//*****************************************************************************
//*	draw a star-like shape at xcoord, ycoord based on index(=6 brightest)
//*****************************************************************************
void WindowTabSkyTravel::DrawStar_shape(short xcoord, short ycoord, short index)
{
int 	ii;
int		color1;
int		color2;
//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENBABLE_WHITE_CHART_
	if (cChartMode)
	{
		color1	=	W_BLACK;
		color2	=	W_BLACK;
	}
	else
#endif // _ENBABLE_WHITE_CHART_
	{
		color1	=	W_WHITE;
		color2	=	W_LIGHTGRAY;
	}

	switch(index)
	{
		case 0:
	//		LLG_Putpixel(xcoord,	ycoord,		color2);
			LLG_Putpixel(xcoord,	ycoord,		color1);
			break;

		case 1:
			LLG_Putpixel(xcoord,	ycoord,		color1);
			break;

		case 2:
			LLG_Putpixel(xcoord,	ycoord,		color1);
			LLG_Putpixel(xcoord +1,	ycoord,		color2);
			LLG_Putpixel(xcoord-1,	ycoord,		color2);
			LLG_Putpixel(xcoord,	ycoord+1,	color2);
			LLG_Putpixel(xcoord,	ycoord-1,	color2);
			break;

		case 3:
			LLG_Putpixel(xcoord+1,	ycoord,		color1);
			LLG_Putpixel(xcoord-1,	ycoord,		color1);
			LLG_Putpixel(xcoord,	ycoord,		color1);
			LLG_Putpixel(xcoord,	ycoord+1,	color1);
			LLG_Putpixel(xcoord,	ycoord-1,	color1);
			break;

		case 4:
			LLG_Putpixel(xcoord-2,	ycoord,		color2);

			LLG_Putpixel(xcoord-1,	ycoord,		color1);
			LLG_Putpixel(xcoord,	ycoord,		color1);
			LLG_Putpixel(xcoord+1,	ycoord,		color1);

			LLG_Putpixel(xcoord+2,	ycoord,		color1);

			LLG_Putpixel(xcoord-1,	ycoord+1,	color1);
			LLG_Putpixel(xcoord,	ycoord+1,	color1);
			LLG_Putpixel(xcoord+1,	ycoord+1,	color1);
			LLG_Putpixel(xcoord-1,	ycoord-1,	color1);
			LLG_Putpixel(xcoord,	ycoord-1,	color1);
			LLG_Putpixel(xcoord+1,	ycoord-1,	color1);

			LLG_Putpixel(xcoord,	ycoord-2,	color2);
			break;

		case 5:
			for (ii = -1; ii < 2; ii++)
			{
				LLG_Putpixel(xcoord-2,	ycoord+ii,	color1);
				LLG_Putpixel(xcoord-1,	ycoord+ii,	color1);
				LLG_Putpixel(xcoord,	ycoord+ii,	color1);
				LLG_Putpixel(xcoord+1,	ycoord+ii,	color1);
				LLG_Putpixel(xcoord+2,	ycoord+ii,	color1);
			}

			LLG_Putpixel(xcoord-1,	ycoord+2,	color1);
			LLG_Putpixel(xcoord,	ycoord+2,	color1);
			LLG_Putpixel(xcoord+1,	ycoord+2,	color1);

			LLG_Putpixel(xcoord-1,	ycoord-2,	color1);
			LLG_Putpixel(xcoord,	ycoord-2,	color1);
			LLG_Putpixel(xcoord+1,	ycoord-2,	color1);

			break;

		case 6:
			LLG_SetColor(color1);
			LLG_DrawCString(xcoord,	ycoord,	"*");
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
	//	if ((fabs(cDecl0) < (cView_angle_Rads + ECLIPTIC_ANGLE)))
		{
			DrawEcliptic();
		}
	}

	if (!cChartMode && (cDispOptions.dispEarth || cDispOptions.dispHorizon_line))	//* horizon line or earth
	{
		//* are we zoomed in enough and is horizon in field?
		if ((cView_angle_Rads < 5.5) && (fabs(cElev0) < (cView_angle_Rads / 2.0)))
		{
			DrawHorizon();								//* plot the horizon
		#ifdef _DISPLAY_MAP_TOKENS_
			MapTokens(&gCurrentSkyTime, &cCurrLatLon);		//* look for map tokens
		#endif // _DISPLAY_MAP_TOKENS_
		}
		else if (cDispOptions.dispEarth && (cElev0 < 0.0))
		{
//			FillWithEarth();	//* fill whole screen
		}
	}

	//*	are we supposed to draw the telescope F.O.V. information
	if (cDispOptions.dispTelescope)
	{
//	bool	telescopeIsInView;
	short	telescopeXX, telescopeYY;

//		telescopeIsInView	=
								GetXYfromRA_Decl(	gTelescopeRA_Radians,
													gTelescopeDecl_Radians,
													&telescopeXX,
													&telescopeYY);
//		CONSOLE_DEBUG_W_NUM("telescopeXX           \t=",	telescopeXX);
//		CONSOLE_DEBUG_W_NUM("telescopeYY           \t=",	telescopeYY);
//		//*	do we display the telescope cross hairs
//		if (telescopeIsInView)
		{
		int	fovCount;

			fovCount	=	DrawTelescopeFOV();
			if (fovCount == 0)
			{
				telescopeXX	+=	cWorkSpaceTopOffset;
				telescopeYY	+=	cWorkSpaceLeftOffset;
				DrawTelescopeReticle(telescopeXX, telescopeYY);
			}
		}
//		else
//		{
//			CONSOLE_DEBUG("-------------------------");
//			CONSOLE_DEBUG_W_DBL("gTelescopeRA_Radians  \t=",	DEGREES(gTelescopeRA_Radians / 15.0));
//			CONSOLE_DEBUG_W_DBL("gTelescopeDecl_Radians\t=",	DEGREES(gTelescopeDecl_Radians));
//		}
	}
#if 0
		//*	are we keeping the display centered on the telescope position
		if (cTrackTelescope)
		{
		cv::Rect	centerRect;
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
		if (fabs(cDecl0) < cView_angle_Rads)
		{
			LLG_SetColor(W_MAGENTA);
			DrawGreatCircle(0.0, gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, false);
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
	LLG_SetColor(W_BLUE);
	if (cTelescopeDisplayOptions.dispFindScopeOutline)
	{
		LLG_FrameEllipse(screenXX, screenYY, kFinderRadius, kFinderRadius);
	}

	if (cTelescopeDisplayOptions.dispFindScopeCrossHairs)
	{
		LLG_MoveTo(screenXX - kFinderRadius, screenYY - kFinderRadius);
		LLG_LineTo(screenXX + kFinderRadius, screenYY + kFinderRadius);

		LLG_MoveTo(screenXX + kFinderRadius, screenYY - kFinderRadius);
		LLG_LineTo(screenXX - kFinderRadius, screenYY + kFinderRadius);
	}

	//*	now draw the actual telescope

	LLG_SetColor(W_GREEN);

	if (cTelescopeDisplayOptions.dispTeleScopeOutline)
	{
		LLG_FrameEllipse(screenXX, screenYY, kTlescopeRadius, kTlescopeRadius);
	}

	if (cTelescopeDisplayOptions.dispTeleScopeCrossHairs)
	{
		LLG_MoveTo(screenXX - kTlescopeRadius, screenYY - kTlescopeRadius);
		LLG_LineTo(screenXX + kTlescopeRadius, screenYY + kTlescopeRadius);

		LLG_MoveTo(screenXX + kTlescopeRadius, screenYY - kTlescopeRadius);
		LLG_LineTo(screenXX - kTlescopeRadius, screenYY + kTlescopeRadius);
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
char	cameraDisplayName[128];

	LLG_SetColor(W_RED);
//	LLG_SetColor(W_WHITE);
	LLG_SetColor(fovPtr->OutLineColor);

	fovWasDrawn	=	false;
	if (fovPtr->IsValid && fovPtr->FOVenabled)
	{
		//*	get FOV width and height in radians
		fovWidth_RAD		=	RADIANS(fovPtr->FOV_X_arcSeconds / 3600.0);
		fovHeight_RAD		=	RADIANS(fovPtr->FOV_Y_arcSeconds / 3600.0);

		//	cXfactor is	pixels per radian
		pixelsWide	=	fovWidth_RAD * cXfactor;
		pixelsTall	=	fovHeight_RAD * cYfactor;

		//*	check the width of the box
		//*	dont bother drawing it if its less than 10
		if (pixelsWide >= 10)
		{
			fovWasDrawn	=	true;

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
			LLG_MoveTo(topLeft_XX,		topLeft_YY);
			LLG_LineTo(topRight_XX,	topRight_YY);

			//*	bottom line
			LLG_MoveTo(btmLeft_XX,		btmLeft_YY);
			LLG_LineTo(btmRight_XX,	btmRight_YY);

			//*	left line
			LLG_MoveTo(topLeft_XX,		topLeft_YY);
			LLG_LineTo(btmLeft_XX,		btmLeft_YY);

			//*	right line
			LLG_MoveTo(topRight_XX,	topRight_YY);
			LLG_LineTo(btmRight_XX,	btmRight_YY);

			//*	check the width of the box, only draw the label if its is big enough
			if (pixelsWide > 50)
			{
				strcpy(cameraDisplayName, fovPtr->HostName);
				if (pixelsWide > 200)
				{
					//*	if we have enough room, add the camera name as well
					strcat(cameraDisplayName, "-");
					strcat(cameraDisplayName, fovPtr->CameraName);
				}
				//*	now do the label
				if ((btmLeft_XX > 0) && (btmLeft_YY > 0))
				{
					LLG_DrawCString(btmLeft_XX, btmLeft_YY, cameraDisplayName);
				}
				else if ((topLeft_XX > 0) && (topLeft_YY > 0))
				{
					LLG_DrawCString(topLeft_XX, topLeft_YY, cameraDisplayName);
				}
				else if ((topRight_XX > 0) && (topRight_YY > 0))
				{
					LLG_DrawCString(topRight_XX, topRight_YY, cameraDisplayName);
				}
				else if ((btmRight_XX > 0) && (btmRight_YY > 0))
				{
					LLG_DrawCString(btmRight_XX, btmRight_YY, cameraDisplayName);
				}
			}
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

	LLG_SetColor(W_RED);
//	LLG_SetColor(W_WHITE);
	telescopeIsInView	=	GetXYfromRA_Decl(	gTelescopeRA_Radians,
												gTelescopeDecl_Radians,
												&telescopeXX,
												&telescopeYY);
	fovCount	=	0;
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


#if 0
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
#endif // 0




#include	"skytravel_radec.cpp"
//*****************************************************************************
void		WindowTabSkyTravel::CovertAzEl_to_RA_DEC(	double	latitude_rad,		//*	phi
														double	azimuth_rad,		//*	x
														double	elev_rad,			//*	y
														double	*ra_rad,		//*	P
														double	*dec_rad)		//	Q
{
	CONSOLE_DEBUG_W_DBL("azimuth\t=",		DEGREES(azimuth_rad));
	CONSOLE_DEBUG_W_DBL("elevation\t=",		DEGREES(elev_rad));
	CONSOLE_DEBUG_W_DBL("latitude\t=",		DEGREES(latitude_rad));
	CONSOLE_DEBUG_W_DBL("fSiderealTime\t=", (gCurrentSkyTime.fSiderealTime));
	CONSOLE_DEBUG_W_DBL("sidereal(deg)\t=", DEGREES(gCurrentSkyTime.fSiderealTime));
	CONSOLE_DEBUG_W_DBL("sidereal(deg)\t=", DEGREES(gCurrentSkyTime.fSiderealTime) / 15);

	GetRADEC_fromAzEl(	azimuth_rad,
						elev_rad,
						latitude_rad,
						gCurrentSkyTime.fSiderealTime,
						ra_rad,
						dec_rad);

	CONSOLE_DEBUG_W_DBL("returned RA\t=", DEGREES(*ra_rad));
	CONSOLE_DEBUG_W_DBL("returned DEC\t=", DEGREES(*dec_rad));

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// write code in the language of C to transform alt-az to right ascension declination
//
//
//	As an AI language model, I can provide you with the steps to convert alt-az coordinates to right ascension declination:
//
//	1. Convert the altitude (alt) and azimuth (az) angles from degrees to radians.
//	2. Calculate the declination (dec) angle using the following formula:
//			dec = asin(sin(alt) * sin(latitude) + cos(alt) * cos(az) * cos(latitude)),
//			where latitude is the observer's latitude in radians.
//	3. Calculate the hour angle (ha) using the following formula:
//			ha = acos((sin(alt) - sin(dec) * sin(latitude)) / (cos(dec) * cos(latitude))).
//			If the azimuth angle is greater than 180 degrees, subtract the hour angle from 360 degrees.
//			Otherwise, add the hour angle to 180 degrees.
//	4. Calculate the right ascension (ra) using the following formula:
//			ra = local_sidereal_time - ha,
//			where local_sidereal_time is the observer's local sidereal time in hours (0 to 24).
//	5. If the calculated right ascension is less than zero, add 24 hours to it.
//
//Here is the C code to perform these calculations:


#define deg2rad(x) ((x) * M_PI / 180.0)
#define rad2deg(x) ((x) * 180.0 / M_PI)

//*****************************************************************************
//*	this routine is based on code generated by AI
//*****************************************************************************
void	WindowTabSkyTravel::ConvertAzEl_to_RaDec(	const double	azimuth_deg,
													const double	elevation_deg,
													double			*output_RA_deg,
													double			*output_DEC_deg)
{
double	az_rad;
double	elv_rad;
double	lat_rad;
double	lst;
double	dec_rad;
double	hourAngle;
double	ra_rad;

	// input altitude, azimuth, latitude, and local sidereal time
//	printf("Enter altitude (degrees): ");
//	scanf("%lf", &altitude);
//
//	printf("Enter azimuth (degrees): ");
//	scanf("%lf", &azimuth);

//	printf("Enter observer's latitude (degrees): ");
//	scanf("%lf", &lat);
//	lat_rad	=	deg2rad(lat);

//	printf("Enter local sidereal time (hours): ");
//	scanf("%lf", &lst);

//	CONSOLE_DEBUG_W_DBL("gObseratorySettings.Latitude_deg\t=", gObseratorySettings.Latitude_deg);
	lat_rad	=	deg2rad(gObseratorySettings.Latitude_deg);
	lst		=	gCurrentSkyTime.fSiderealTime;
	lst		=	8.0;
//	CONSOLE_DEBUG_W_DBL("lst                  \t=",	lst);

	// convert altitude and azimuth to radians
	az_rad	=	deg2rad(azimuth_deg);
	elv_rad	=	deg2rad(elevation_deg);

	// calculate declination
	dec_rad	=	asin(sin(elv_rad) * sin(lat_rad) + cos(elv_rad) * cos(az_rad) * cos(lat_rad));

	// calculate hour angle
	hourAngle	=	acos((sin(elv_rad) - sin(dec_rad) * sin(lat_rad)) / (cos(dec_rad) * cos(lat_rad)));
	if (az_rad > deg2rad(180))
	{
		hourAngle	=	(2 * M_PI) - hourAngle;
	}

	// calculate right ascension
	ra_rad	=	lst - hourAngle;
	if (ra_rad < 0)
	{
		ra_rad	+=	24.0;
	}

	// output right ascension and declination in degrees
	if (ra_rad > (2 * M_PI))
	{
		ra_rad	-=	(2 * M_PI);
	}
	else if (ra_rad < 0.0)
	{
		ra_rad	+=	(2 * M_PI);
	}

	*output_RA_deg	=	rad2deg(ra_rad);
	*output_DEC_deg	=	rad2deg(dec_rad);
//	printf("Right ascension: %.2f\n",	rad2deg(ra_rad));
//	printf("Declination: %.2f\n",		rad2deg(dec_rad));
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define	ADJUST360(x)		\
	if (x > 360.0)			\
	{						\
		x	=	x - 360.0;	\
	}						\
	else if (x < 0.0)		\
	{						\
		x	=	x + 360.0;	\
	}						\


#define _USE_AI_ROUTINE_
#ifdef _USE_AI_ROUTINE_
//*****************************************************************************
void	WindowTabSkyTravel::DrawDomeSlit(void)
{
double	slitWidth_Radians;
double	slitWidth_Degrees;
//double	slitHeight_Radians;
double	slitCenter_Degrees;
double	slitLeft_Degrees;
double	slitRight_Degrees;
double	skyTravelDomeAzimuth;	//*	Skytravel is reversed coordinates
double	btmLeftRA_deg;
double	btmLeftDEC_deg;
double	btmRghtRA_deg;
double	btmRghtDEC_deg;

double	topLeftRA_deg;
double	topLeftDEC_deg;
double	topRghtRA_deg;
double	topRghtDEC_deg;
short	x1, y1;
short	x2, y2;
bool	pt1InView;
bool	pt2InView;

	skyTravelDomeAzimuth	=	360.0 - gDomeAzimuth_degrees;
//	skyTravelDomeAzimuth	=	gDomeAzimuth_degrees;

	CONSOLE_DEBUG("------------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	CONSOLE_DEBUG_W_DBL("gSlitWidth_inches   \t=",	gSlitWidth_inches);
	CONSOLE_DEBUG_W_DBL("gDomeDiameter_inches\t=",	gDomeDiameter_inches);

	slitWidth_Radians	=	2.0 * atan2((gSlitWidth_inches / 2.0), (gDomeDiameter_inches / 2.0));
	slitWidth_Degrees	=	DEGREES(slitWidth_Radians);
//	slitHeight_Radians	=	RADIANS(gSlitTop_degrees) - RADIANS(gSlitBottom_degrees);

//	CONSOLE_DEBUG_W_DBL("slitHeight_Degrees\t\t=",	slitHeight_Degrees);
	CONSOLE_DEBUG_W_DBL("Slit width degrees\t\t=",	DEGREES(slitWidth_Radians));
//	CONSOLE_DEBUG_W_DBL("Slit height degrees\t=",	DEGREES(slitHeight_Radians));

	slitCenter_Degrees	=	skyTravelDomeAzimuth;
	slitLeft_Degrees	=	slitCenter_Degrees - (slitWidth_Degrees / 2);
	slitRight_Degrees	=	slitCenter_Degrees + (slitWidth_Degrees / 2);
	ADJUST360(slitLeft_Degrees);
	ADJUST360(slitRight_Degrees);

	CONSOLE_DEBUG_W_DBL("gDomeAzimuth_degrees\t=",	gDomeAzimuth_degrees);
	CONSOLE_DEBUG_W_DBL("slitWidth           \t=",	slitWidth_Degrees);
	CONSOLE_DEBUG_W_DBL("slitLeft            \t=",	slitLeft_Degrees);
	CONSOLE_DEBUG_W_DBL("slitRight           \t=",	slitRight_Degrees);


	ConvertAzEl_to_RaDec(slitLeft_Degrees,	25.0, &btmLeftRA_deg, &btmLeftDEC_deg);
	ConvertAzEl_to_RaDec(slitRight_Degrees,	25.0, &btmRghtRA_deg, &btmRghtDEC_deg);
	CONSOLE_DEBUG_W_DBL("btmLeftRA_deg       \t=",	btmLeftRA_deg);
	CONSOLE_DEBUG_W_DBL("btmRghtRA_deg       \t=",	btmRghtRA_deg);

	CONSOLE_DEBUG_W_DBL("btmLeftDEC_deg      \t=",	btmLeftDEC_deg);
	CONSOLE_DEBUG_W_DBL("btmRghtDEC_deg      \t=",	btmRghtDEC_deg);


	ConvertAzEl_to_RaDec(slitLeft_Degrees,	85.0, &topLeftRA_deg, &topLeftDEC_deg);
	ConvertAzEl_to_RaDec(slitRight_Degrees, 85.0, &topRghtRA_deg, &topRghtDEC_deg);

	CONSOLE_DEBUG_W_DBL("topLeftRA_deg       \t=",	topLeftRA_deg);
	CONSOLE_DEBUG_W_DBL("topRghtRA_deg       \t=",	topRghtRA_deg);

	CONSOLE_DEBUG_W_DBL("topLeftDEC_deg       \t=",	topLeftDEC_deg);
	CONSOLE_DEBUG_W_DBL("topRghtDEC_deg       \t=",	topRghtDEC_deg);

	//---------------------------------------------------------------
	//*	do the left line
	pt1InView	=	GetXYfromRA_Decl(	RADIANS(topLeftRA_deg),
										RADIANS(topLeftDEC_deg),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	RADIANS(btmLeftRA_deg),
										RADIANS(btmLeftDEC_deg),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_GREEN);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);
	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
	//---------------------------------------------------------------
	//*	do the Right line
	pt1InView	=	GetXYfromRA_Decl(	RADIANS(topRghtRA_deg),
										RADIANS(topRghtDEC_deg),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	RADIANS(btmRghtRA_deg),
										RADIANS(btmRghtDEC_deg),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_GREEN);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);
	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
	//---------------------------------------------------------------
	//*	do the bottom line
	pt1InView	=	GetXYfromRA_Decl(	RADIANS(btmLeftRA_deg),
										RADIANS(btmLeftDEC_deg),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	RADIANS(btmRghtRA_deg),
										RADIANS(btmRghtDEC_deg),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_GREEN);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);
	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
	//---------------------------------------------------------------
	//*	do the top line
	pt1InView	=	GetXYfromRA_Decl(	RADIANS(topLeftRA_deg),
										RADIANS(topLeftDEC_deg),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	RADIANS(topRghtRA_deg),
										RADIANS(topRghtDEC_deg),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_GREEN);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);
	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
}

#else

//*****************************************************************************
void	WindowTabSkyTravel::DrawDomeSlit(void)
{
double	slitWidth_Radians;
//double	slitHeight_Radians;
double	slitCenter_Radians;
double	slitLeft_Radians;
double	slitRight_Radians;
double	skyTravelDomeAzimuth;	//*	Skytravel is reversed coordinates
double	obsLatitude_rad;

	skyTravelDomeAzimuth	=	360.0 - gDomeAzimuth_degrees;

	CONSOLE_DEBUG("------------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	LLG_SetColor(W_CYAN);

	obsLatitude_rad		=	RADIANS(gObseratorySettings.Latitude_deg);
	slitWidth_Radians	=	2.0 * atan2((gSlitWidth_inches / 2.0), (gDomeDiameter_inches / 2.0));
//	slitHeight_Radians	=	RADIANS(gSlitTop_degrees) - RADIANS(gSlitBottom_degrees);

//	CONSOLE_DEBUG_W_DBL("slitHeight_Degrees\t\t=",	slitHeight_Degrees);
//	CONSOLE_DEBUG_W_DBL("Slit width degrees\t\t=",	DEGREES(slitWidth_Radians));
//	CONSOLE_DEBUG_W_DBL("Slit height degrees\t=",	DEGREES(slitHeight_Radians));

	slitCenter_Radians	=	RADIANS(skyTravelDomeAzimuth);
	slitLeft_Radians	=	slitCenter_Radians - (slitWidth_Radians / 2);
	slitRight_Radians	=	slitCenter_Radians + (slitWidth_Radians / 2);
	CONSOLE_DEBUG_W_DBL("slitWidth\t=",		DEGREES(slitWidth_Radians));
	CONSOLE_DEBUG_W_DBL("slitLeft\t=",		DEGREES(slitLeft_Radians));
	CONSOLE_DEBUG_W_DBL("slitRight\t=",		DEGREES(slitRight_Radians));
#if 0
#else
	DrawHorizontalArc(RADIANS(gSlitBottom_degrees), slitLeft_Radians, slitRight_Radians);
	DrawHorizontalArc(RADIANS(gSlitTop_degrees),	slitLeft_Radians, slitRight_Radians);

	DrawVerticalArc(slitLeft_Radians, RADIANS(gSlitBottom_degrees), RADIANS(gSlitTop_degrees));
	DrawVerticalArc(slitRight_Radians, RADIANS(gSlitBottom_degrees), RADIANS(gSlitTop_degrees));
#endif

#if 1
double		slitBtm_RA_rad;
double		slitBtm_Dec_rad;
double		slitTop_RA_rad;
double		slitTop_Dec_rad;
short		x1, y1;
short		x2, y2;
bool		pt1InView;
bool		pt2InView;


	CONSOLE_DEBUG_W_DBL("gDomeAzimuth_degrees\t=",	gDomeAzimuth_degrees);
	CONSOLE_DEBUG_W_DBL("skyTravelDomeAzimuth\t=",	skyTravelDomeAzimuth);


	CONSOLE_DEBUG_W_DBL("slitCenter (degrees)\t=",	DEGREES(slitCenter_Radians));

	//*	Draw a line down the center
	CovertAzEl_to_RA_DEC(	obsLatitude_rad,
							slitCenter_Radians,
							RADIANS(gSlitTop_degrees),
							&slitTop_RA_rad,
							&slitTop_Dec_rad);

	CovertAzEl_to_RA_DEC(	obsLatitude_rad,
							slitCenter_Radians,
							RADIANS(gSlitBottom_degrees),
							&slitBtm_RA_rad,
							&slitBtm_Dec_rad);


	pt1InView	=	GetXYfromRA_Decl(	(slitTop_RA_rad),
										(slitTop_Dec_rad),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	(slitBtm_RA_rad),
										(slitBtm_Dec_rad),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_GREEN);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);

	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
#if 0
	//*	now the right side
	CovertAzEl_to_RA_DEC(	obsLatitude_rad,
							slitRight_Radians,
							RADIANS(gSlitTop_degrees),
							&slitTop_RA_rad,
							&slitTop_Dec_rad);

	CovertAzEl_to_RA_DEC(	obsLatitude_rad,
							slitRight_Radians,
							RADIANS(gSlitBottom_degrees),
							&slitBtm_RA_rad,
							&slitBtm_Dec_rad);


	pt1InView	=	GetXYfromRA_Decl(	(slitTop_RA_rad),
										(slitTop_Dec_rad),
										&x1,
										&y1);

	pt2InView	=	GetXYfromRA_Decl(	(slitBtm_RA_rad),
										(slitBtm_Dec_rad),
										&x2,
										&y2);
	if (pt1InView && pt2InView)
	{
		LLG_SetColor(W_PINK);
		LLG_MoveTo(x1,	y1);
		LLG_LineTo(x2,	y2);

	}
	else
	{
		CONSOLE_DEBUG("Slit not in view")
	}
#endif
#endif

}
#endif // _USE_AI_ROUTINE_

//*****************************************************************************
void	WindowTabSkyTravel::CenterOnDomeSlit(void)
{
double	skyTravelDomeAzimuth;	//*	Skytravel is reversed coordinates

	skyTravelDomeAzimuth	=	360.0 - gDomeAzimuth_degrees;

	cAz0	=	RADIANS(skyTravelDomeAzimuth);
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

	LLG_SetColor(W_BLUE);
	LLG_PenSize(2);
//	for (alpha = (cRa0 - cRamax); alpha < (cRa0 + cRamax); alpha += (cRamax / 10.0))
	//*	<MLS> 1/4/2021, this allows the Ecliptic to be drawn at all zoom levels
	myRamax		=	cRamax;
	if ((cView_angle_Rads >= 1.7) || (myRamax < 0.1))
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
			LLG_LineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
		else
		{
			LLG_MoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
			drawLineFlag	=	true;
		}
	}
	LLG_PenSize(1);
}

//*********************************************************************
void	WindowTabSkyTravel::DrawGrid(short theSkyColor)
{
double		degrees;
int			northColor;
int			southColor;
bool		forceNumberDrawFlag	=	false;

	LLG_PenSize(gST_DispOptions.LineWidth_GridLines);

//setlinestyle(USERBIT_LINE,0x0f0f,NORM_WIDTH);
	if (cNightMode)
	{
		northColor	=	W_DARKRED;
		southColor	=	W_DARKRED;
	}
	else if (theSkyColor > 60)
	{
		northColor	=	W_DARKGRAY;
		southColor	=	W_DARKBLUE;
	}
	else
	{
		northColor	=	W_DARKGRAY;
		southColor	=	W_DARKBLUE;
	}

	//-------------------------------------------------------------------------
	//*	draw 1 degree grid lines
	if ((cView_angle_Rads <= 1.0) && (cDispOptions.dispGaia))
	{
	bool	enableGreatCircleNumbers;

		if (cView_angle_Rads < 0.2)
		{
			enableGreatCircleNumbers	=	kEnableGreatCircleNumbers;
		}
		else
		{
			enableGreatCircleNumbers	=	kDisableGreatCircleNumbers;
		}
		degrees	=	-80.0;
		while (degrees < 81.0)
		{
			LLG_SetColor(W_DARKRED);
			DrawGreatCircle(RADIANS(degrees), true, enableGreatCircleNumbers, false);
			degrees	+=	1.0;
		}
	}

	LLG_SetColor(northColor);

	//-------------------------------------------------------------------------
	//*	draw the great circles
	degrees	=	-80.0;
	while (degrees < 81.0)
	{
		LLG_SetColor((degrees >= 0.0) ? northColor : southColor);
		DrawGreatCircle(RADIANS(degrees), gST_DispOptions.DashedLines, true, true);
		degrees	+=	10.0;
	}

	//*	if we are zoomed in real far, draw the circles around the poles.
	if (cView_angle_Rads <= 0.7)
	{
		if (cView_angle_Rads < 0.4)
		{
			forceNumberDrawFlag	=	true;
		}
		LLG_SetColor(northColor);

		DrawGreatCircle(RADIANS(87.0), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(88.0), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(89.0), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);

		DrawGreatCircle(RADIANS(89.1), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(89.3), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(89.5), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(89.7), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);

		//--------------------------------------------------
		//*	now do the south pole
		LLG_SetColor(southColor);
		DrawGreatCircle(RADIANS(-89.1), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(-89.3), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(-89.5), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
		DrawGreatCircle(RADIANS(-89.7), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, forceNumberDrawFlag);
	}

	if (cView_angle_Rads < 0.18)
	{
		LLG_SetColor(northColor);
		DrawGreatCircle(RADIANS(89.9), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, true);

		LLG_SetColor(southColor);
		DrawGreatCircle(RADIANS(-89.9), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, true);
	}

	if (cView_angle_Rads < 0.04)
	{
		LLG_SetColor(northColor);
		DrawGreatCircle(RADIANS(89.95), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, true);

		LLG_SetColor(southColor);
		DrawGreatCircle(RADIANS(-89.95), gST_DispOptions.DashedLines, kEnableGreatCircleNumbers, true);
	}

	//-------------------------------------------------------------------------
	//*	draw the north/south lines
	degrees	=	0.0;
	while (degrees < 360.0)
	{
		LLG_SetColor(northColor);
		DrawNorthSouthLine(RADIANS(degrees));
		degrees	+=	15.0;
//		degrees	+=	15.0;	//*	put it in twice to match K-Stars
	}
	LLG_PenSize(1);

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);
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

	LLG_SetColor(W_BROWN);	//* make horizon brown

	gam			=	0.0;
	codecl		=	kHALFPI - cElev0;

	rtasc		=	cView_angle_Rads / 2.0;	//* starting point
	sin_bside	=	sin(codecl);
	cos_bside	=	cos(codecl);

	switch(gST_DispOptions.EarthDispMode)
	{
		case 0:
			delta_ra	=	rtasc / 100.0;		//*increment
			break;

		case 1:
			delta_ra	=	rtasc / 20.0;		//*increment
			LLG_PenSize(2);
			break;

		case 2:
			delta_ra	=	rtasc / 900.0;		//*increment
			break;

		case 3:
			LLG_SetColor(W_DARKGREEN);
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
			LLG_LineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
		else
		{
			LLG_MoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
			ftflag++;
		}

		if (cDispOptions.dispEarth)
		{
			//*	this fills with earth
			LLG_LineTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + wind_uly + cWind_height);
			LLG_MoveTo(cWorkSpaceLeftOffset + xcoord, cWorkSpaceTopOffset + ycoord);
		}
	}
	LLG_PenSize(1);
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
	azDelta		=	GetRA_DEC_delta(cView_angle_Rads);
	CONSOLE_DEBUG_W_DBL("viewangle/delta ratio\t=", (cView_angle_Rads / azDelta))
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
			LLG_MoveTo(pt1_XX,	pt1_YY);
			LLG_LineTo(pt2_XX,	pt2_YY);
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
	elevDelta		=	GetRA_DEC_delta(cView_angle_Rads);

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
			LLG_MoveTo(pt1_XX,	pt1_YY);
			LLG_LineTo(pt2_XX,	pt2_YY);
		}
		myELEVvalue1	+=	elevDelta;
	}
}

//*****************************************************************************
static double	GetRA_DEC_delta(const double viewAngle)
{
double	angleDelta;

//	if (viewAngle < 0.2)
//	{
//		angleDelta	=	viewAngle / 4.0;
//	}
//	else
//	{
//		angleDelta	=	M_PI / 200.0;
//	}

	if (viewAngle > 1.0)
	{
		angleDelta	=	viewAngle / 100.0;
	}
	else
	{
		angleDelta	=	viewAngle / 50.0;
	}

	if (angleDelta < RADIANS(0.75))
	{
		angleDelta	=	RADIANS(0.75);
	}
//	return(angleDelta * 10);
	return(angleDelta);
}


//*****************************************************************************
//*	draw a great circle at the specified dec angle
//*	declinationAngle is in radians
//*
//*	returns the number of points draw, 0 means nothing was drawn
//*****************************************************************************
int	WindowTabSkyTravel::DrawGreatCircle(	const double	declinationAngle_rad,
											const bool		useDashedLines,
											const bool		enableNumbers,
											const bool		forceNumberDraw)
{
double	rtAscen1;
double	rtAscen2;
double	rtAscenDelta_Radians;
double	minDeltaRA;
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;
int		segmentsDrnCnt;
int		adjustmentCnt	=	0;	//*	for adjusting angle delta table in GetRA_DEC_delta()
int		leftMost_X;
int		leftMost_Y;
int		rightMost_X;
int		rightMost_Y;
int		topMost_X;
int		topMost_Y;
int		btmMost_X;
int		btmMost_Y;
char	numberStr[32];
bool	numberWasDrawn;
int		segmentLength;
int		segment_deltaX;
int		segment_deltaY;
int		minSegLength;
int		maxSegLength;

//	CONSOLE_DEBUG(__FUNCTION__);

//	SETUP_TIMING();

//	if (declinationAngle_rad < -1.0)
//	{
//		LLG_SetColor(W_RED);
//	}

	leftMost_X		=	10000;
	leftMost_Y		=	10000;
	topMost_X		=	10000;
	topMost_Y		=	10000;
	rightMost_X		=	0;
	rightMost_Y		=	0;
	btmMost_X		=	0;
	btmMost_Y		=	0;
	numberWasDrawn	=	false;

	minSegLength			=	9999;
	maxSegLength			=	0;
	segmentsDrnCnt			=	0;
	minDeltaRA				=	GetRA_DEC_delta(kMinViewAngle);
	rtAscenDelta_Radians	=	GetRA_DEC_delta(cView_angle_Rads);

	if (cMouseDragInProgress && (rtAscenDelta_Radians < RADIANS(2.0)))
	{
		rtAscenDelta_Radians	=	RADIANS(2.0);
	}

	//*	setup the start of the circle
	rtAscen1		=	0.0;
	while (rtAscen1 < ((2 * M_PI)))
	{
//		if ((segmentsDrnCnt % 2) == 1)
//		{
//			LLG_SetColor(W_RED);
//		}
//		else
//		{
//			LLG_SetColor(W_WHITE);
//		}

		//*	we are going draw a line between 2 points
		//*	calculate the 2nd pt
		rtAscen2	=	rtAscen1 + rtAscenDelta_Radians;

		pt1inView	=	GetXYfromRA_Decl(	rtAscen1,
											declinationAngle_rad,
											&pt1_XX, &pt1_YY);

		pt2inView	=	GetXYfromRA_Decl(	rtAscen2,
											declinationAngle_rad,
											&pt2_XX, &pt2_YY);

		//*	if both pints are on the screen, draw it
		if (pt1inView && pt2inView)
		{
			//*	check the step size, we want to make sure nothing bigger than 20 pixels
			//*	I tried to do this in a separate routine but it slowed down way too much
			while (((abs(pt2_XX - pt1_XX) + abs(pt2_YY - pt1_YY)) > 20) && (rtAscenDelta_Radians > minDeltaRA))
			{
				rtAscenDelta_Radians	-=	0.001;
				//*	check to make sure we didnt go too far
				if (rtAscenDelta_Radians < minDeltaRA)
				{
					rtAscenDelta_Radians	=	minDeltaRA;
				}

				//*	re-calculate point 2
				rtAscen2	=	rtAscen1 + rtAscenDelta_Radians;
				pt2inView	=	GetXYfromRA_Decl(	rtAscen2,
													declinationAngle_rad,
													&pt2_XX, &pt2_YY);
				adjustmentCnt++;
			}
			//*	now we can go ahead and draw the line
			LLG_MoveTo(pt1_XX,	pt1_YY);
			LLG_LineTo(pt2_XX,	pt2_YY);
			segmentsDrnCnt++;


			segment_deltaX	=	pt2_XX - pt1_XX;
			segment_deltaY	=	pt2_YY - pt1_YY;

			segmentLength	=	((segment_deltaX * segment_deltaX) + (segment_deltaY * segment_deltaY));
			if (segmentLength > maxSegLength)
			{
				maxSegLength	=	segmentLength;
			}
			if (segmentLength < minSegLength)
			{
				minSegLength	=	segmentLength;
			}
		}
		rtAscen1	+=	rtAscenDelta_Radians;

		//*	if the line is dashed, skip a segment

//-		if (gST_DispOptions.DashedLines)
		if (useDashedLines)
		{
			rtAscen1	+=	rtAscenDelta_Radians;
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

	if (enableNumbers)
	{
		//----------------------------------------------------
		//*	draw numbers along the edges
		if (declinationAngle_rad == 0.0)
		{
			strcpy(numberStr, "EQ");
		}
		else if (declinationAngle_rad > 1.553343)
		{
			sprintf(numberStr, "%1.2fN", DEGREES(declinationAngle_rad));
		}
		else if (declinationAngle_rad < -1.553343)
		{
			sprintf(numberStr, "%1.2fS", -DEGREES(declinationAngle_rad));
		}
		else if (declinationAngle_rad > 1.51)
		{
			sprintf(numberStr, "%1.1fN", DEGREES(declinationAngle_rad));
		}
		else if (declinationAngle_rad > 0.0)
		{
			sprintf(numberStr, "%1.0fN", DEGREES(declinationAngle_rad));
		}
		else
		{
			sprintf(numberStr, "%1.0fS", -DEGREES(declinationAngle_rad));
		}
		LLG_SetColor(W_DARKGREEN);
		if (leftMost_X < 30)
		{
			LLG_DrawCString(3, leftMost_Y, numberStr);
			numberWasDrawn	=	true;
		}
		else if (rightMost_X > (cWind_width - 30))
		{
			LLG_DrawCString((cWind_width - 30), rightMost_Y, numberStr);
			numberWasDrawn	=	true;
		}
		else if (topMost_Y < 15)
		{
			LLG_DrawCString(topMost_X, 15, numberStr);
			numberWasDrawn	=	true;
		}
		else if (btmMost_Y > (cWind_height - 15))
		{
			LLG_DrawCString(btmMost_X, (cWind_height - 4), numberStr);
			numberWasDrawn	=	true;
		}

		//*	in some case we want the number anyway
		if (forceNumberDraw && (numberWasDrawn == false))
		{
			LLG_DrawCString(topMost_X, topMost_Y, numberStr);
		}
	}

//	DEBUG_TIMING(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("segmentsDrnCnt\t=", segmentsDrnCnt);
//	if (segmentsDrnCnt > 0)
//	{
//		CONSOLE_DEBUG("==============================================");
//	//	CONSOLE_DEBUG_W_DBL("minDeltaRA (degrees)  \t=", DEGREES(minDeltaRA));
//		CONSOLE_DEBUG_W_DBL("viewangle/delta ratio \t=", (cView_angle_Rads / rtAscenDelta_Radians));
//	//	CONSOLE_DEBUG_W_DBL("rtAscenDelta (radians)\t=", rtAscenDelta_Radians);
//		CONSOLE_DEBUG_W_DBL("rtAscenDelta (degrees)\t=", DEGREES(rtAscenDelta_Radians));
//		CONSOLE_DEBUG_W_DBL("minSegLength\t=", sqrt(minSegLength));
//		CONSOLE_DEBUG_W_DBL("maxSegLength\t=", sqrt(maxSegLength));
//		if (adjustmentCnt > 0)
//		{
//			CONSOLE_DEBUG_W_NUM("adjustmentCnt\t=", adjustmentCnt);
//		}
//	}

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
	minDeltaDec		=	GetRA_DEC_delta(kMinViewAngle) / 10.0;
	declDelta		=	GetRA_DEC_delta(cView_angle_Rads);
	declDelta		=	declDelta / 15.0;

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


			LLG_MoveTo(pt1_XX,	pt1_YY);
			LLG_LineTo(pt2_XX,	pt2_YY);
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
		if (gST_DispOptions.DashedLines)
		{
			declination1	-=	declDelta;
		}
	}
	//----------------------------------------------------
	//*	draw numbers along the edges
	sprintf(numberStr, "%1.0fh", DEGREES(rightAscen_Rad / 15.0));
	if (leftMost_X < 30)
	{
		LLG_DrawCString(3, leftMost_Y, numberStr);
	}
	if (rightMost_X > (cWind_width - 30))
	{
		LLG_DrawCString((cWind_width - 30), rightMost_Y, numberStr);
	}
	if (topMost_Y < 15)
	{
		LLG_DrawCString(topMost_X, 15, numberStr);
	}
	if (btmMost_Y > (cWind_height - 15))
	{
		LLG_DrawCString(btmMost_X, (cWind_height - 4), numberStr);
	}


//	CONSOLE_DEBUG_W_NUM("segmentsDrnCnt\t=", segmentsDrnCnt);
	return(segmentsDrnCnt);

}

//*****************************************************************************
//*	"scale of miles" indicator, chart mode only
//*****************************************************************************
void	WindowTabSkyTravel::DrawScale(void)
{
//bool	pt1inView;
//bool	pt2inView;
//short	pt1_XX, pt1_YY;
//short	pt2_XX, pt2_YY;
//double	deltaRA;
//double	myCursor_RA;
//double	myCursor_DEC;
//
//CONSOLE_DEBUG(__FUNCTION__);
//
//
//#if 1
//double	leftSideOfScreen_rad;
//double	leftSideOfSCreen_deg;
//double	leftSideOfSCreen_hrs;
//
//	leftSideOfScreen_rad	=	cRa0 - (cView_angle_Rads / 2.0);
//	leftSideOfSCreen_deg	=	DEGREES(leftSideOfScreen_rad);
//	leftSideOfSCreen_hrs	=	leftSideOfSCreen_deg / 15.0;
//	if (leftSideOfSCreen_hrs < 0.0)
//	{
//		leftSideOfSCreen_hrs	+=	24.0;
//	}
//	CONSOLE_DEBUG_W_DBL("leftSideOfScreen_rad\t=",	leftSideOfScreen_rad);
//	CONSOLE_DEBUG_W_DBL("leftSideOfSCreen_deg\t=",	leftSideOfSCreen_deg);
//	CONSOLE_DEBUG_W_DBL("leftSideOfSCreen_hrs\t=",	leftSideOfSCreen_hrs);
//
//
//#else
//double		aa, bb, cc;
//TYPE_SpherTrig	sph;
//int		myTopLeftX;
//int		myTopLeftY;
//
//	//*	code copies from Compute_Cursor
//	myTopLeftX	=	10;
//	myTopLeftY	=	200;
//	myTopLeftX	-=	cWorkSpaceLeftOffset;
//	myTopLeftY	-=	cWorkSpaceTopOffset;
//
//	//*	still need the offset for doing cursor calculations
//	myTopLeftY	-=	cCursorOffsetY;
//
//	aa	=	(myTopLeftX - cWind_x0) / cXfactor;
//	bb	=	-(myTopLeftY - cWind_y0) / cYfactor;
//
//	//*	we will never be close to the center, so skip that part
//
//	sph.cside			=	sqrt((aa * aa) + (bb * bb));
//	cc					=	sph.cside;
//	sph.alpha			=	acos(aa / cc);
//	if (bb < 0.)
//	{
//		sph.alpha	*=	-1.0;			//*	negate
//	}
//	sph.alpha			+=	1.5 * PI;	//*	always add 270 degrees
//	if (sph.alpha > kTWOPI)
//	{
//		sph.alpha	-=	kTWOPI;
//	}
//	//*	always in chart mode
//	if (cChartMode)
//	{
//		//*	chart mode calculations
//		sph.bside		=	kHALFPI - cDecl0;
//		sphsas(&sph);
//		myCursor_DEC	=	kHALFPI - sph.aside;
//		sphsss(&sph);
//		myCursor_RA		=	sph.gamma + cRa0;
//
//		sph.bside		=	kHALFPI - cElev0;
//		sph.alpha		=	sph.alpha - cChart_gamma;
//		sphsas(&sph);
////		cCursor_elev	=	kHALFPI - sph.aside;
//		sphsss(&sph);
////		cCursor_az		=	sph.gamma + cAz0;
//	}
//
//	if (myCursor_RA < 0.0)
//	{
////			myCursor_RA	+=	2 * M_PI;
//	}
//	deltaRA	=	RADIANS(2.0);
//
//	pt1inView	=	GetXYfromRA_Decl(	myCursor_RA,
//										myCursor_DEC,
//										&pt1_XX, &pt1_YY);
//
//	pt2inView	=	GetXYfromRA_Decl(	myCursor_RA + deltaRA,
//										myCursor_DEC,
//										&pt2_XX, &pt2_YY);
//
//	//*	if both pints are on the screen, draw it
//	if (pt1inView && pt2inView)
//	{
//		LLG_SetColor(W_ORANGE);
//		LLG_MoveTo(pt1_XX,	pt1_YY);
//		LLG_LineTo(pt2_XX,	pt2_YY);
//
//		LLG_MoveTo(pt1_XX,	pt1_YY+1);
//		LLG_LineTo(pt2_XX,	pt2_YY+1);
//	}
//	else
//	{
//		CONSOLE_DEBUG("Off screen");
//	}
//#endif // 1
}


//*****************************************************************************
void	WindowTabSkyTravel::DrawCompass(void)
{
double	dtemp,angle,left_edge,right_edge;
int		ii;
int		xpos;
int		ypos;

//	CONSOLE_DEBUG(__FUNCTION__);

	LLG_SetColor(W_YELLOW);
	left_edge	=	-cAz0 - (cView_angle_Rads / 2.0);

	while (left_edge < -kTWOPI)
	{
		left_edge	+=	kTWOPI;
	}
	while (left_edge > kTWOPI)
	{
		left_edge	-=	kTWOPI;
	}

	right_edge	=	left_edge + cView_angle_Rads;

	for (angle = -kTWOPI,ii = 0;angle < kTWOPI; angle += (kTWOPI/16.), ii++)
	{
		if ((angle >= left_edge) && (angle <= right_edge))
		{
			dtemp	=	angle - left_edge;
			if (dtemp >= 0.0)
			{
				xpos	=	wind_ulx + (dtemp * (1.0 * cWind_width) / cView_angle_Rads);
				ypos	=	wind_uly + cWind_height - (THGT / 2);
				LLG_DrawCString(cWorkSpaceTopOffset + xpos,
							cWorkSpaceLeftOffset  + ypos, gCompass_text[ii & 0x0f]);
			}
		}
	}
}

//*****************************************************************************
//*	Blood moon
//	May 26, 2021 – North and South America, Asia and Australasia.
//	May 16, 2022 – North and South America, Europe and Africa.
//*****************************************************************************
//enum
//{
//	kMoonType_Normal	=	0,
//	kMoonType_Blue,
//	kMoonType_Blood,
//	kMoonType_Last
//};
//
////*****************************************************************************
//static int	GetMoonType(double phasang, TYPE_SkyTime *skyTravelTime)
//{
//int	moonType;
//
//	moonType	=	kMoonType_Normal;
//	if (phasang >= 3.0)
//	{
//		//*	we only have to figure things out on full moons
//		//*	default is normal
//		moonType	=	kMoonType_Normal;
//		if (skyTravelTime->local_day >= 29)
//		{
//			moonType	=	kMoonType_Blue;
//		}
//	}
//	return(moonType);
//}

//*****************************************************************************
//* Moon, filled crescent
//*****************************************************************************
void	WindowTabSkyTravel::DrawMoon(	TYPE_SpherTrig	sphptr,
										TYPE_CelestData	*objectptr,
										const char		**shapes,
										short			xcoord,
										short			ycoord
)
{
int				jjj;
short			xxx;
short			yyy;
double			angle;
double			theta;
double			phasang;
double			posang;
double			minang;
double			radx;
double			rady;
double			dx;
double			dy;
double			deldecl, delra;
bool			drawInFillFlag;

//	CONSOLE_DEBUG_W_NUM("---------------------------------", cDebugCounter++);
//	minang	=	(cView _ index + 1) * (7.0 * PI / 180.0);	//* to avoid floodfill problems
	minang	=	((8.0 * (cView_angle_Rads / kMaxViewAngleIndex) ) * 7.0 * PI / 180.0);	//* to avoid floodfill problems
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
	radx	=	cMoon_radius * cXfactor;
	rady	=	cMoon_radius * cYfactor;

	if ((radx < 3.0) || (cView_angle_Rads > 1.2))	//* don't draw if too small
	{
		DrawVector(W_LIGHTGRAY, xcoord, ycoord, 1, shapes[MON]);
	}
	else
	{
		LLG_SetColor(W_DARKGRAY);	//* complete circle
		LLG_FillEllipse(xcoord,ycoord, radx, rady);

//		CONSOLE_DEBUG_W_DBL("phasang-rad\t=",		phasang);
//		CONSOLE_DEBUG_W_DBL("phasang-deg\t=",		DEGREES(phasang));
		if (phasang != 0.0)
		{
			LLG_SetColor(W_YELLOW);	//* semi-ellipse
			theta	=	posang;
			for (jjj=0; theta < (posang + PI); jjj++)
			{
			//	xxx	=	xcoord + (radx * cos(theta));
			//	yyy	=	ycoord + (rady * sin(theta));
				//*	make the outside a tiny bit smaller so we dont have any black leaking through
				xxx	=	xcoord + ((radx - 0.3) * cos(theta));
				yyy	=	ycoord + ((rady - 0.3) * sin(theta));
				if (jjj)
				{
					LLG_LineTo(xxx, yyy);
				}
				else
				{
					LLG_MoveTo(xxx, yyy);
				}
				theta	+=	PI / 64.0;
			}

			//* 64 points in a semi-ellipse
	//		for (jjj=0,theta = -PI/2.0; theta < (33.0 * PI / 64.0); jjj++, theta += PI /64.0)
			jjj		=	0;
			theta	=	-PI/2.0;
			while (theta < (33.0 * PI / 64.0))
			{
				dx	=	radx * cos(theta) * cos(phasang);
				dy	=	rady * sin(theta);

				//* coordinate rotation by posang
				yyy	=	ycoord + ((dx * cos(posang)) + (dy * sin(posang)));
				xxx	=	xcoord + ((dy * cos(posang)) - (dx * sin(posang)));

				LLG_LineTo(xxx, yyy);
				jjj++;
				theta	+=	PI / 64.0;
			}

			//* fill the crescent part
//			CONSOLE_DEBUG_W_DBL("cPhase_angle\t=",		cPhase_angle);
//			CONSOLE_DEBUG_W_DBL("minang      \t=",		minang);
			//*	there are a number of cases were the infill isnt needed or doesnt work.
			//*	go through them and turn off infill for these cases

			drawInFillFlag	=	true;


			if (fabs(cPhase_angle) < minang)	//*	this was the original SkyTravel check
			{
				drawInFillFlag	=	false;
			}
			else if (phasang < 0.08)
			{
				drawInFillFlag	=	false;
			}
			else if ((phasang < 0.15) && (cView_angle_Rads > 0.03))
			{
				drawInFillFlag	=	false;
			}
			else if ((phasang < 0.2) && (cView_angle_Rads > 0.3))
			{
				drawInFillFlag	=	false;
			}
			else if ((phasang < 0.26) && (cView_angle_Rads > 0.1))
			{
				drawInFillFlag	=	false;
			}
			else if ((phasang < 1.0) && (cView_angle_Rads > 1.2))
			{
				drawInFillFlag	=	false;
			}


			if (drawInFillFlag)
			{
			double	radAdjustmentValue;

//				CONSOLE_DEBUG("Calling FloodFill()");
				theta	=	posang + (PI / 2.0);
		//		xxx		=	xcoord + ((radx - 1.5) * cos(theta));	//* make sure it is inside
		//		yyy		=	ycoord + ((rady - 1.5) * sin(theta));

				//*	the radius adjustment value needs some tweeking when zoomed in real far
				if (cView_angle_Rads < 0.06)
				{
					radAdjustmentValue	=	2.0;
				}
				else
				{
					radAdjustmentValue	=	1.5;
				}
				xxx		=	xcoord + ((radx - radAdjustmentValue) * cos(theta));	//* make sure it is inside
				yyy		=	ycoord + ((rady - radAdjustmentValue) * sin(theta));

//			int	moonType;
//			int	moonColor;
//				//*	determine the color of the moon
//				moonType	=	GetMoonType(phasang, &gCurrentSkyTime);
//				switch(moonType)
//				{
//					case kMoonType_Blue:
//						moonColor	=	W_BLUE;
//						break;
//
//					case kMoonType_Blood:
//						moonColor	=	W_RED;
//						break;
//
//					default:
//						moonColor	=	W_YELLOW;
//						break;
//				}
//				LLG_FloodFill(xxx, yyy, moonColor);
				LLG_FloodFill(xxx, yyy, W_YELLOW);
				//*	debug code
//			#define		kCrossSize	25
//				for (jjj=-kCrossSize; jjj<kCrossSize; jjj++)
//				{
//					LLG_Putpixel(xxx + jjj, yyy, W_GREEN);
//
//					LLG_Putpixel(xxx, yyy+jjj, W_GREEN);
//				}
//				for (jjj=-kCrossSize; jjj<kCrossSize; jjj++)
//				{
//					LLG_Putpixel(xcoord + jjj, ycoord, W_RED);
//
//					LLG_Putpixel(xcoord, ycoord + jjj, W_RED);
//				}
			}
		}

		//*	Lunar eclipse display processing
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
			radx	=	cEarth_shadow_radius * cXfactor;
			rady	=	cEarth_shadow_radius * cYfactor;
			LLG_SetColor(W_BLACK);
			LLG_FillEllipse(xcoord, ycoord, radx, rady);
		}
	}
}

//*****************************************************************************
//*	must be in this order
//*	enum{MON,SUN,MER,VEN,MAR,JUP,SAT,URA,NEP,PLU,GEOMON};
//*	values are in arc seconds
//*	obtained from
//*		https://en.wikipedia.org/wiki/Angular_diameter
//*	value for Pluto  https://en.wikipedia.org/wiki/Pluto
//*****************************************************************************
static double	gPlanetDiameter[]	=
{
	0.0,	//*	MON
	0.0,	//*	SUN
	4.5,	//*	MER
	9.7,	//*	VEN
	3.5,	//*	MAR
	29.8,	//*	JUP
	14.5,	//*	SAT
	3.3,	//*	URA
	2.2,	//*	NEP
	0.11,	//*	PLU
	0.0	//*	GEOMON

};

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
int				iii;
short			xcoord;
short			ycoord;
double			angle;
double			radius;
double			radx,rady;
double			rad0[3];
int				myColor;

//int				jjj;
//short				xxx;
//short				yyy;
//double			theta;
//double			phasang;
//double			posang;
//double			minang;
//double			dx,dy;
//double			deldecl, delra;

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
		case 10:			myColor	=	W_CYAN;			break;	//* planet names
		case kZodiacCount:	myColor	=	W_LIGHTMAGENTA;	break;	//* zodiac sign names
		default:			myColor	=	W_WHITE;		break;
	}

	for (iii=objCnt-1; iii>=0; iii--)
	{
		LLG_SetColor(myColor);
		objectptr[iii].curXX	=	-100;
		objectptr[iii].curYY	=	-100;

		//*	do all of the spherical trip stuff
		sphptr.alpha	=	cRa0 - objectptr[iii].ra;

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
			sphptr.cside	=	kHALFPI - objectptr[iii].decl;
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
					objectptr[iii].curXX	=	xcoord;

					ycoord	=	cWind_y0 - (cYfactor * sphptr.aside * cos(angle));
					if ((ycoord >= wind_uly) && (ycoord <= wind_uly + cWind_height))
					{
						objectptr[iii].curYY	=	ycoord;

						if ((objCnt == kPlanetObjectCnt) && (iii > 1) && !cDispOptions.dispSymbols && !cDispOptions.dispNames)
						{
							DrawStar_shape(xcoord, ycoord, objectptr[iii].magn);
						}

						if ((objCnt == kPlanetObjectCnt) && (iii < 2))	//* Moon(0) or Sun(1)
						{
							if (iii == 1)	//* Sun, filled ellipse
							{
								LLG_SetColor(W_WHITE);
								radius	=	rad0[iii];
								radx	=	radius * cXfactor;
								rady	=	radius * cYfactor;
								if (radx < 3.0)	//* don't draw if too small
								{
									DrawVector(W_WHITE, xcoord, ycoord, 1, shapes[iii]);
//									goto next1;
								}
								else
								{
									LLG_FillEllipse(xcoord, ycoord, radx, rady);
								}
							}
							//-------------------------------------------------
							if (iii == 0)	//* Moon, filled crescent
							{
								DrawMoon(	sphptr,
											objectptr,
											shapes,
											xcoord,
											ycoord);
							}
						}
						else
						{
							//*	no symbols or names for sun and moon
							if (cDispOptions.dispSymbols)	//* symbols except if moon or sun
							{
							short	planetScale;

								if (cView_angle_Rads <= 0.1)
								{
									planetScale	=	3;
								}
								else if (cView_angle_Rads <= 0.7)
								{
									planetScale	=	2;
								}
								else
								{
									planetScale	=	1;
								}
								if (objectptr[iii].dataSrc == kDataSrc_Planets)
								{
								double	planetDiam_arc_secs;
								int		planetColor;

									planetColor			=	myColor;
									planetDiam_arc_secs	=	gPlanetDiameter[iii];
									if (planetDiam_arc_secs > 0.0)
									{
									double	planetDiam_degrees;
									double	planetDiam_Radians;
									int		diameterPixels;
									int		radiusPixels;

										planetDiam_degrees	=	planetDiam_arc_secs / 3600.0;
										planetDiam_Radians	=	RADIANS(planetDiam_degrees);
										diameterPixels		=	(planetDiam_Radians / cView_angle_Rads) * cWind_width;
										radiusPixels		=	diameterPixels / 2;
										if (radiusPixels < 1)
										{
											radiusPixels	=	1;
										}

										LLG_FillEllipse(xcoord, ycoord, radiusPixels, radiusPixels);

										if (radiusPixels > 30)
										{
											planetColor	=	W_BLACK;
										}
										LLG_SetColor(planetColor);
									}
									DrawVector(planetColor, xcoord, ycoord, planetScale, shapes[iii]);
								}
								else
								{
									DrawVector(myColor, xcoord, ycoord, planetScale, shapes[iii]);
								}

								if (cView_angle_Rads < 0.4)
								{
									//*	if we are this far zoomed in, show the name as well
									LLG_DrawCString(xcoord + 18, ycoord, name[iii]);
								}
							}
							else if (cDispOptions.dispNames)	//* names except if moon or sun
							{
								LLG_DrawCString(xcoord, ycoord, name[iii]);
							}
						}
					}
				}
			}
		}
//	next1:;
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
int		informStrLen;

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
		informStrLen	=	strlen(cInform_name);
		if (informStrLen > 0)
		{
			if ((informStrLen + strlen(cursorString)) < sizeof(cursorString))
			{
				strcat(cursorString, " - ");
				strcat(cursorString, cInform_name);
			}
			else
			{
				CONSOLE_DEBUG_W_STR("cursorString\t=", cursorString);
				CONSOLE_DEBUG_W_STR("cInform_name\t=", cInform_name);
				CONSOLE_ABORT(__FUNCTION__);
			}
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

//	CONSOLE_DEBUG(__FUNCTION__);
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

//aavso
//*********************************************************************
void	WindowTabSkyTravel::FindObjectNearCursor(TYPE_CelestData *returnObject)
{
TYPE_CelestData	foundObject;
TYPE_CelestData	closestObject;
long			closestDistance;
long			pixDist;
char			closestObjSrc[32];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);
//	CONSOLE_DEBUG_W_LHEX("this        \t=", this);
//	CONSOLE_DEBUG_W_LHEX("returnObject\t=", returnObject);

	memset(&foundObject,	0,	sizeof(TYPE_CelestData));
	memset(&closestObject,	0,	sizeof(TYPE_CelestData));

	closestDistance	=	9999;

	//--------------------------------------------------------------------------
	//*	kDataSrc_Messier
	if (cDispOptions.dispMessier && (gMessierObjectPtr != NULL) && (gMessierObjectCount > 0))
	{
//		CONSOLE_DEBUG("gMessierObjectPtr");
		pixDist	=	FindXX_YYinObjectList(	gMessierObjectPtr,
											gMessierObjectCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "Messier");
		}
	}

	//--------------------------------------------------------------------------
	if (cDispOptions.dispDefaultData && (gStarDataPtr != NULL) && (gStarCount > 0))
	{
//		CONSOLE_DEBUG("gStarDataPtr");
		pixDist	=	FindXX_YYinObjectList(	gStarDataPtr,
											gStarCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		//*	since this is the first one, it will always be true
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "Default");
		}
	}
	pixDist	=	FindXX_YYinObjectList(	cPlanets,
										kPlanetObjectCnt,
										cCsrx,				//*	current x location of cursor,
										cCsry,				//*	current y location of cursor,
										&foundObject);		//*	closestObject
	if (pixDist < closestDistance)
	{
		closestDistance	=	pixDist;
		closestObject	=	foundObject;
		strcpy(closestObjSrc, "Planets");
	}

	//--------------------------------------------------------------------------
	if (cDispOptions.dispNGC && (gNGCobjectPtr != NULL) && (gNGCobjectCount > 0))
	{
//		CONSOLE_DEBUG("gNGCobjectPtr");
		pixDist	=	FindXX_YYinObjectList(	gNGCobjectPtr,
											gNGCobjectCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "NGC");

		}
	}

	//--------------------------------------------------------------------------
	if (gZodiacPtr != NULL)
	{
//		CONSOLE_DEBUG("gZodiacPtr");
		pixDist	=	FindXX_YYinObjectList(	gZodiacPtr,
											kZodiacCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
//			CONSOLE_DEBUG("Zodiac");
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "Zodiac");
		}
	}

	//--------------------------------------------------------------------------
	//*	kDataSrc_YaleBrightStar
	if (cDispOptions.dispYale && (gYaleStarDataPtr != NULL) && (gYaleStarCount > 0))
	{
//		CONSOLE_DEBUG("gYaleStarDataPtr");
		pixDist	=	FindXX_YYinObjectList(	gYaleStarDataPtr,
											gYaleStarCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "Yale");
		}
	}


	//--------------------------------------------------------------------------
	//*	kDataSrc_Hipparcos
	if (cDispOptions.dispHipparcos && (gHipObjectPtr != NULL) && (gHipObjectCount > 0))
	{
//		CONSOLE_DEBUG("gHipObjectPtr");
		pixDist	=	FindXX_YYinObjectList(	gHipObjectPtr,
											gHipObjectCount,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "Hipparcos");
		}
	}

	//--------------------------------------------------------------------------
	//*	kDataSrc_AAVSOalert
	if (cDispOptions.dispAAVSOalerts && (gAAVSOalertsPtr != NULL) && (gAAVSOalertsCnt > 0))
	{
		pixDist	=	FindXX_YYinObjectList(	gAAVSOalertsPtr,
											gAAVSOalertsCnt,
											cCsrx,				//*	current x location of cursor,
											cCsry,				//*	current y location of cursor,
											&foundObject);		//*	closestObject
		if (pixDist < closestDistance)
		{
			closestDistance	=	pixDist;
			closestObject	=	foundObject;
			strcpy(closestObjSrc, "AAVSO");
		}
	}

	cInform_dist	=	0x7fff;

	//*	lets make sure we have something semi valid
	if (closestDistance < kMinInformDist)
	{
	short	ii;
	bool	foundMoreInfo;
	char	tmpString[8]	=	"";
	char	moreInfo[128]	=	"";
	double	distance_LY;

//		CONSOLE_DEBUG("Valid close object");

		cInform_dist		=	closestDistance;
		strcpy(cInform_name, "Unknown");

		cInform_dist	=	1;
		switch(closestObject.dataSrc)
		{
			case kDataSrc_Orginal:
				sprintf(cInform_name, "Original DB = %4ld", closestObject.id);
				break;

			case kDataSrc_Planets:
				//*	figure out which planet it is
				short	planetIndex;
				planetIndex	=	-1;
				for (ii=0; ii<kPlanetObjectCnt; ii++)
				{
					if (closestObject.id == cPlanets[ii].id)
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
						if (closestObject.id == gZodiacPtr[ii].id)
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
				sprintf(cInform_name, "Yale = %04ld mag=%4.2f", closestObject.id, closestObject.realMagnitude);
				break;


			case kDataSrc_NGC2000:
			case kDataSrc_NGC2000IC:
				tmpString[0]	=	(closestObject.NGCtype >> 24) & 0x7f;
				tmpString[1]	=	(closestObject.NGCtype >> 16) & 0x7f;
				tmpString[2]	=	(closestObject.NGCtype >> 8) & 0x7f;
				tmpString[3]	=	(closestObject.NGCtype) & 0x7f;
				tmpString[4]	=	0;

				sprintf(cInform_name, "NGC = %04ld %s mag=%4.2f", closestObject.id, tmpString, closestObject.realMagnitude);

				foundMoreInfo	=	GetObjectDescription(&closestObject, moreInfo, 128);
				if (foundMoreInfo)
				{
					strcat(cInform_name, " ");
					strcat(cInform_name, moreInfo);
				}
				break;

			case kDataSrc_Hipparcos:
				sprintf(cInform_name, "Hipparcos = H%04ld mag=%4.2f", closestObject.id, closestObject.realMagnitude);
				if (closestObject.spectralClass > 0x40)
				{
					sprintf(moreInfo, " Spectral Class=%c", closestObject.spectralClass);
					strcat(cInform_name, moreInfo);
				}
				break;

			case kDataSrc_Messier:
				sprintf(cInform_name, "Messier %s in %s", closestObject.shortName, closestObject.longName);
				break;

			case kDataSrc_OpenNGC:
				sprintf(cInform_name, "OpenNGC = %s mag=%4.2f", closestObject.longName, closestObject.realMagnitude);
				break;

			case kDataSrc_AAVSOalert:
				sprintf(cInform_name, "AAVSO alert = %s mag=%4.2f", closestObject.longName, closestObject.realMagnitude);
				break;

			default:
				CONSOLE_DEBUG_W_STR("cInform_name\t=", cInform_name);
				CONSOLE_DEBUG_W_NUM("dataSrc      \t=", closestObject.dataSrc);
				strcpy(cInform_name, "default");
				break;
		}

		//*	check for parallax and display in light years if available
		if (closestObject.parallax > 0.0000000000000000001)
//		if (closestObject.parallax > 0.0)
		{
//			CONSOLE_DEBUG_W_DBL("closestObject.parallax\t=", closestObject.parallax);
			//*	1 / parallax = # of parsecs
			//*	1 parsec = 3.26 light years

			distance_LY	=	(1.0 / closestObject.parallax) * 3.26;

			//*	make sure we have a valid number
			if (distance_LY < 1E30)
			{
				sprintf(moreInfo, " parallax = %5.4f distance=%4.2f light years", closestObject.parallax, distance_LY);
				if ((strlen(cInform_name) + strlen(moreInfo)) < sizeof(cInform_name))
				{
					strcat(cInform_name, moreInfo);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_STR("cInform_name\t=", cInform_name);
				CONSOLE_DEBUG_W_DBL("distance_LY \t=", distance_LY);
				DumpCelestDataStruct(__FUNCTION__, &closestObject);
				CONSOLE_ABORT(__FUNCTION__);
			}
		}

//		CONSOLE_DEBUG_W_STR("cInform_name\t=", cInform_name);
//		CONSOLE_DEBUG_W_STR("moreInfo     \t=", moreInfo);
	}
	if (strlen(cInform_name) >= 255)
	{
		CONSOLE_DEBUG_W_STR("cInform_name\t=", cInform_name);
		CONSOLE_DEBUG_W_SIZE("Length      \t=", strlen(cInform_name));
		CONSOLE_ABORT(__FUNCTION__);
	}

	if (returnObject != NULL)
	{
		*returnObject	=	closestObject;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}
//aavso

//*****************************************************************************
bool	WindowTabSkyTravel::SearchSkyObjectsDataListByNumber(	TYPE_CelestData *starDataPtr,
																long			starCount,
																int				dataSource,
																const char		*namePrefix,
																char			*searchString)
{
int		objectIDnum;
int		iii;
char	*argPtr;
bool	foundIt;
int		prefixLen;

	foundIt		=	false;
	prefixLen	=	0;
	if ((starDataPtr != NULL) && (starCount > 0))
	{
		argPtr	=	searchString;
		if (namePrefix != NULL)
		{
			prefixLen	=	strlen(namePrefix);
			argPtr		+=	prefixLen;
			//*	now skip any white space
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
		}
		//*	strip off any non-numeric chars
		while (isdigit(*argPtr) == false)
		{
			argPtr++;
		}
		objectIDnum	=	atoi(argPtr);
		CONSOLE_DEBUG_W_NUM("looking for objectIDnum", objectIDnum);
		iii			=	0;
		while ((cFoundSomething == false) && (iii < starCount))
		{
			if ((objectIDnum == starDataPtr[iii].id) && (starDataPtr[iii].dataSrc == dataSource))
			{
				cFound_newRA	=	starDataPtr[iii].ra;
				cFound_newDec	=	starDataPtr[iii].decl;

				if ((namePrefix != NULL) && (strlen(starDataPtr[iii].longName) > 0))
				{
					//*	a prefix specified, make sure it matches
					if (strncasecmp(starDataPtr[iii].longName, namePrefix, prefixLen) == 0)
					{
						cFoundSomething		=	true;
						foundIt				=	true;
						sprintf(cFoundName, "%s-%d mag=%2.1f", namePrefix, objectIDnum, starDataPtr[iii].realMagnitude);
					}
				}
				else
				{
					sprintf(cFoundName, "#%d mag=%2.1f", objectIDnum, starDataPtr[iii].realMagnitude);
					cFoundSomething		=	true;
					foundIt				=	true;
				}
			}
			iii++;
		}
	}
	return(foundIt);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SearchSkyObjectsDataListByShortName(TYPE_CelestData *starDataPtr,
																long			starCount,
																int				dataSource,
																char			*searchString)
{
int		iii;
bool	foundIt;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Looking for:", searchString);

	foundIt	=	false;
	if ((starDataPtr != NULL) && (starCount > 0))
	{
		iii			=	0;
		while ((cFoundSomething == false) && (iii < starCount))
		{
			if (strcasecmp(searchString, starDataPtr[iii].shortName) == 0)
			{
				cFound_newRA	=	starDataPtr[iii].ra;
				cFound_newDec	=	starDataPtr[iii].decl;

				sprintf(cFoundName, "%s mag=%2.1f", starDataPtr[iii].shortName, starDataPtr[iii].realMagnitude);
				cFoundSomething		=	true;
				foundIt				=	true;
			}
			iii++;
		}
	}
	else
	{
		CONSOLE_DEBUG("Database pointer is NULL");
	}
	return(foundIt);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SearchSkyObjectsDataListByLongName(TYPE_CelestData *starDataPtr,
																long			starCount,
																int				dataSource,
																char			*searchString)
{
int		iii;
bool	foundIt;
int		searchStrLen;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Looking for:", searchString);

	searchStrLen	=	strlen(searchString);
	foundIt			=	false;
	if ((starDataPtr != NULL) && (starCount > 0))
	{
		iii			=	0;
		while ((cFoundSomething == false) && (iii < starCount))
		{
			if (strncasecmp(searchString, starDataPtr[iii].longName, searchStrLen) == 0)
			{
				cFound_newRA	=	starDataPtr[iii].ra;
				cFound_newDec	=	starDataPtr[iii].decl;

				sprintf(cFoundName, "%s mag=%2.1f", starDataPtr[iii].longName, starDataPtr[iii].realMagnitude);
				cFoundSomething		=	true;
				foundIt				=	true;
			}
			iii++;
		}
	}
	else
	{
		CONSOLE_DEBUG("Database pointer is NULL");
	}
	return(foundIt);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SearchSkyObjectsConstellations(const char *searchString)
{
int		iii;
int		searchStrLen;
bool	foundIt;

	foundIt			=	false;
	if ((gConstVecotrPtr != NULL) && (gConstVectorCnt > 0))
	{
		searchStrLen	=	strlen(searchString);
		iii				=	0;
		while ((cFoundSomething == false) && (iii < gConstOutlineCount))
		{
			if (strncasecmp(searchString, gConstVecotrPtr[iii].constellationName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG("found in Constellations");

				cFound_newRA	=	gConstVecotrPtr[iii].rtAscension;
				cFound_newDec	=	gConstVecotrPtr[iii].declination;
				strcpy(cFoundDatabase, "Constellations");
				strcpy(cFoundName, gConstVecotrPtr[iii].constellationName);
				cFoundSomething	=	true;
				foundIt			=	true;
			}
			iii++;
		}
	}
	return(foundIt);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SearchSkyObjectsConstOutlines(const char *searchString)
{
int		iii;
int		searchStrLen;
bool	foundIt;

	foundIt			=	false;
	if ((gConstOutlinePtr != NULL) && (gConstOutlineCount > 0))
	{
		searchStrLen	=	strlen(searchString);
		//*	lets look through all of the long names first
		iii	=	0;
		while ((cFoundSomething == false) && (iii < gConstOutlineCount))
		{
			if (strncasecmp(searchString, gConstOutlinePtr[iii].constOutlineName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG("found in Constellation outlines");

				cFound_newRA	=	gConstOutlinePtr[iii].rtAscension;
				cFound_newDec	=	gConstOutlinePtr[iii].declination;
				strcpy(cFoundDatabase, "Constellation outlines");
				strcpy(cFoundName, gConstOutlinePtr[iii].constOutlineName);
				cFoundSomething	=	true;
				foundIt			=	true;
			}
			iii++;
		}

		//*	now if we didnt find anything, look at the short names
		iii	=	0;
		while ((cFoundSomething == false) && (iii < gConstOutlineCount))
		{
			if (strncasecmp(searchString, gConstOutlinePtr[iii].shortName, searchStrLen) == 0)
			{
				CONSOLE_DEBUG_W_STR("Found", gConstOutlinePtr[iii].shortName);
				cFound_newRA	=	gConstOutlinePtr[iii].rtAscension;
				cFound_newDec	=	gConstOutlinePtr[iii].declination;
				strcpy(cFoundDatabase, "Constellation outlines");
				strcpy(cFoundName, gConstOutlinePtr[iii].shortName);
				cFoundSomething	=	true;
				foundIt			=	true;
			}
			iii++;
		}
	}

	return(foundIt);
}

//*****************************************************************************
bool	WindowTabSkyTravel::SearchAsteroids(const char *searchString)
{
int		iii;
bool	foundIt;
char	*stringPtr;

	foundIt			=	false;
	if ((gAsteroidPtr != NULL) && (gAsteroidCnt > 0))
	{
		//*	lets look through all of the long names first
		iii	=	0;
		while ((cFoundSomething == false) && (iii < gAsteroidCnt))
		{
			stringPtr	=	strstr(gAsteroidPtr[iii].AsteroidName, searchString);

		//	if (strncasecmp(searchString, gAsteroidPtr[iii].AsteroidName, searchStrLen) == 0)
			if (stringPtr != NULL)
			{
				CONSOLE_DEBUG_W_STR("Found", gAsteroidPtr[iii].AsteroidName);
				cFound_newRA	=	gAsteroidPtr[iii].StarData.ra;
				cFound_newDec	=	gAsteroidPtr[iii].StarData.decl;
				cFoundMagnitude	=	gAsteroidPtr[iii].AbsoluteMagnitude;
				strcpy(cFoundDatabase, "Asteroids");
				strcpy(cFoundName, gAsteroidPtr[iii].AsteroidName);
				cFoundSomething	=	true;
				foundIt			=	true;
			}
			iii++;
		}
	}
	return(foundIt);
}

//*****************************************************************************
void	WindowTabSkyTravel::SearchSkyObjects(char *objectName)
{
int		iii;
char	firstChar;
char	msgString[256];
int		objectIDnum;
char	*argPtr;
bool	foundIt;

//	CONSOLE_DEBUG("-----------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Searching for", objectName);

	cFoundSomething	=	false;
	cFoundMagnitude	=	-99.9;

	firstChar		=	toupper(objectName[0]);
	strcpy(cFoundDatabase, "");
	strcpy(cFoundName, "");
	//-------------------------------------------------------------------------------
	//*	check to see if they specified original DB #
	if (objectName[0] == '#')
	{
		CONSOLE_DEBUG_W_STR("Looking in NGC for", objectName);
		foundIt	=	SearchSkyObjectsDataListByNumber(	gStarDataPtr,
														gStarCount,
														kDataSrc_Orginal,
														NULL,
														&objectName[1]);
		if (foundIt)
		{
			cDispOptions.dispNGC	=	true;
			strcpy(cFoundDatabase, "OriginalDB");
		}
	}


	//-------------------------------------------------------------------------------
	//*	check to see if they specified an NGC object
	if (strncasecmp(objectName, "NGC", 3) == 0)
	{
		CONSOLE_DEBUG_W_STR("Looking in NGC for", objectName);
		foundIt	=	SearchSkyObjectsDataListByNumber(	gNGCobjectPtr,
														gNGCobjectCount,
														kDataSrc_NGC2000,
														"NGC",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispNGC	=	true;
			strcpy(cFoundDatabase, "NGC");
		}
		else
		{
			//*	lets try OpenNGC
			foundIt	=	SearchSkyObjectsDataListByNumber(	gNGCobjectPtr,
															gNGCobjectCount,
															kDataSrc_OpenNGC,
															"NGC",
															objectName);
			if (foundIt)
			{
				cDispOptions.dispNGC	=	true;
				strcpy(cFoundDatabase, "OpenNGC");
			}
		}
	}

	//-------------------------------------------------------------------------------
	//*	check to see if they specified an IC object
	if (strncasecmp(objectName, "IC", 2) == 0)
	{
		foundIt	=	SearchSkyObjectsDataListByNumber(	gNGCobjectPtr,
														gNGCobjectCount,
														kDataSrc_NGC2000IC,
														"IC",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispNGC	=	true;
			strcpy(cFoundDatabase, "IC");
		}
		else
		{
			//*	lets try OpenNGC
			foundIt	=	SearchSkyObjectsDataListByNumber(	gNGCobjectPtr,
															gNGCobjectCount,
															kDataSrc_OpenNGC,
															"IC",
															objectName);
			if (foundIt)
			{
				cDispOptions.dispNGC	=	true;
				strcpy(cFoundDatabase, "OpenNGC");
			}
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for planets
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in planets for", objectName);
		foundIt	=	SearchSkyObjectsDataListByShortName(	cPlanets,
															kPlanetObjectCnt,
															kDataSrc_Planets,
															objectName);
		if (foundIt)
		{
			strcpy(cFoundDatabase, "Planets");
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for messier objects
	if ((firstChar == 'M') && isdigit(objectName[1]))
	{
	//	CONSOLE_DEBUG("Looking for messier objects");
		foundIt	=	SearchSkyObjectsDataListByShortName(	gMessierObjectPtr,
															gMessierObjectCount,
															kDataSrc_Messier,
															objectName);
		if (foundIt)
		{
			cDispOptions.dispMessier	=	true;
			strcpy(cFoundDatabase, "Messier");
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for Hipparcos numbers
	if ((firstChar == 'H') && isdigit(objectName[1]))
	{
		CONSOLE_DEBUG_W_STR("Looking in Hipparcos for", objectName);
		foundIt	=	SearchSkyObjectsDataListByNumber(	gHipObjectPtr,
														gHipObjectCount,
														kDataSrc_Hipparcos,
														"H",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispHipparcos	=	true;
			strcpy(cFoundDatabase, "Hipparcos catalog");
			//*	make sure we can see the data base
			SetMaximumViewAngle(0.03);
		}
	}


	//-------------------------------------------------------------------------------
	//*	look for SAO numbers
	if ((firstChar == 'S') && isdigit(objectName[1]))
	{
		CONSOLE_DEBUG_W_STR("Looking in SAO for", objectName);
		foundIt	=	SearchSkyObjectsDataListByNumber(	gSAOobjectPtr,
														gSAOobjectCount,
														kDataSrc_SAO,
														"S",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispSAO	=	true;
			strcpy(cFoundDatabase, "SAO catalog");
			//*	make sure we can see the data base
			SetMaximumViewAngle(0.03);
		}
	}
	//*	alternate way of specifying an SAO object
	if (strncasecmp(objectName, "SAO", 3) == 0)
	{
		CONSOLE_DEBUG_W_STR("Looking in SAO for", objectName);
		foundIt	=	SearchSkyObjectsDataListByNumber(	gSAOobjectPtr,
														gSAOobjectCount,
														kDataSrc_SAO,
														"SAO",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispSAO	=	true;
			strcpy(cFoundDatabase, "SAO catalog");
			//*	make sure we can see the data base
			SetMaximumViewAngle(0.03);
		}
	}

	//-------------------------------------------------------------------------------
	//*	look in the default star list
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in default for", objectName);
		foundIt	=	SearchSkyObjectsDataListByShortName(gStarDataPtr,
														gStarCount,
														kDataSrc_Orginal,
														objectName);
		if (foundIt)
		{
			cDispOptions.dispDefaultData	=	true;
			strcpy(cFoundDatabase, "Default catalog");
		}

	}



	//-------------------------------------------------------------------------------
	//*	look in the yale catalog
	if (cFoundSomething == false)
	{
		foundIt	=	SearchSkyObjectsDataListByLongName(	gYaleStarDataPtr,
														gYaleStarCount,
														kDataSrc_YaleBrightStar,
														objectName);
		if (foundIt)
		{
			cDispOptions.dispYale	=	true;
			strcpy(cFoundDatabase, "Yale catalog");
		}

	}

	//-------------------------------------------------------------------------------
	//*	look for constellations
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in constellations for", objectName);

		foundIt	=	SearchSkyObjectsConstellations(objectName);
		if (foundIt)
		{
			cDispOptions.dispConstellations	=	true;
			SetMinimumViewAngle(0.7);
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for constellation outlines
	if ((cFoundSomething == false) && cDispOptions.dispConstOutlines)
	{
		CONSOLE_DEBUG_W_STR("Looking in constellation outlines for", objectName);

		foundIt	=	SearchSkyObjectsConstOutlines(objectName);
		//*	outlines dont get displayed unless we the view angle is at least 0.7
		if (foundIt)
		{
			SetMinimumViewAngle(0.7);
		}
	}


	//-------------------------------------------------------------------------------
	//*	look for for common star names in the Hipparcos list
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in Common star names for", objectName);

		foundIt	=	SearchSkyObjectsDataListByLongName(	gHipObjectPtr,
														gHipObjectCount,
														kDataSrc_Hipparcos,
														objectName);
		if (foundIt)
		{
			cDispOptions.dispHipparcos	=	true;
			strcpy(cFoundDatabase, "Hipparcos catalog");
		}
	}



//	if ((cFoundSomething == false) && (gHipObjectPtr != NULL) && (gHipObjectCount > 0))
//	{
//	//	CONSOLE_DEBUG_W_STR("Searching Hipparcosfor \t=", objectName);
//
//		iii	=	0;
//		while ((cFoundSomething == false) && (iii < gHipObjectCount))
//		{
//			if (strncasecmp(objectName, gHipObjectPtr[iii].longName, searchStrLen) == 0)
//			{
//				CONSOLE_DEBUG_W_STR("Found", gHipObjectPtr[iii].longName);
//				cFound_newRA	=	gHipObjectPtr[iii].ra;
//				cFound_newDec	=	gHipObjectPtr[iii].decl;
//
//				strcpy(cFoundDatabase, "Hipparcos catalog");
//				sprintf(cFoundName, "H%ld-%s", gHipObjectPtr[iii].id, gHipObjectPtr[iii].longName);
//				cFoundSomething				=	true;
//				cDispOptions.dispHYG_all	=	true;
//			}
//			iii++;
//		}
//	}

	//-------------------------------------------------------------------------------
	//*	look for for common star names in the AAVSO alert list
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in AAVSO for", objectName);

		foundIt	=	SearchSkyObjectsDataListByLongName(	gAAVSOalertsPtr,
														gAAVSOalertsCnt,
														kDataSrc_AAVSOalert,
														objectName);
		CONSOLE_DEBUG_W_BOOL("foundIt\t=", foundIt);
		if (foundIt)
		{
		//*	make sure the AAVSO alerts are displayed
			SetAAVSOdisplayFlag(true);
			strcpy(cFoundDatabase, "AAVSO Alerts");
			SetMaximumViewAngle(0.15);
			CONSOLE_DEBUG("Found it in AAVSO alerts")
		}
	}

	//-------------------------------------------------------------------------------
	//*	look for for alert number in the AAVSO alert list
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in AAVSO for number", objectName);

		foundIt	=	SearchSkyObjectsDataListByNumber(	gAAVSOalertsPtr,
														gAAVSOalertsCnt,
														kDataSrc_AAVSOalert,
														NULL,
														objectName);
		if (foundIt)
		{
		//*	make sure the AAVSO alerts are displayed
			SetAAVSOdisplayFlag(true);
			strcpy(cFoundDatabase, "AAVSO Alerts");
			SetMaximumViewAngle(0.15);
			CONSOLE_DEBUG("Found it in AAVSO alerts")
		}
	}

	//-------------------------------------------------------------------------------
	//*	check to see if they specified an Henry Draper lists
	if (strncasecmp(objectName, "HD", 2) == 0)
	{
		CONSOLE_DEBUG_W_STR("Looking in HD for", objectName);

		foundIt	=	SearchSkyObjectsDataListByNumber(	gDraperObjectPtr,
														gDraperObjectCount,
														kDataSrc_Draper,
														"HD",
														objectName);
		if (foundIt)
		{
			cDispOptions.dispDraper	=	true;
			strcpy(cFoundDatabase, "Henry Draper");
			CONSOLE_DEBUG("Found it in Henry Draper")
		}
	}


#ifdef _ENABLE_HYG_
	//-------------------------------------------------------------------------------
	//*	check to see if they specified an Henry Draper lists
	if ((cFoundSomething == false) && (strncasecmp(objectName, "HD", 2) == 0))
	{
		CONSOLE_DEBUG("looking for Henry draper objects")
		if ((gHYGObjectPtr != NULL) && (gHYGObjectCount > 0))
		{
			argPtr	=	objectName;
			argPtr	+=	2;
			while (*argPtr == 0x20)
			{
				argPtr++;
			}
			objectIDnum	=	atoi(argPtr);
			iii			=	0;
			while ((cFoundSomething == false) && (iii < gHYGObjectCount))
			{
				if ((objectIDnum == gHYGObjectPtr[iii].id))
				{
					cFound_newRA	=	gHYGObjectPtr[iii].org_ra;
					cFound_newDec	=	gHYGObjectPtr[iii].org_decl;

					strcpy(cFoundDatabase, "IC");
					sprintf(cFoundName, "IC-%d mag=%2.1f", objectIDnum, gHYGObjectPtr[iii].realMagnitude);
					cDispOptions.dispHYG_all	=	true;
					cFoundSomething				=	true;
			//		DumpCelestDataStruct(__FUNCTION__, &gHYGObjectPtr[iii]);
				}
				iii++;
			}
		}
	}
#endif // _ENABLE_HYG_

	//-------------------------------------------------------------------------------
	//*	check the special list
	if (cFoundSomething == false)
	{
		foundIt	=	SearchSkyObjectsDataListByLongName(	gSpecialObjectPtr,
														gSpecialObjectCount,
														kDataSrc_Special,
														objectName);
		if (foundIt)
		{
			cDispOptions.dispSpecialObjects	=	kSpecialDisp_All;
			strcpy(cFoundDatabase, "Special Objects");
		}
	}

//	if ((cFoundSomething == false) && (gSpecialObjectPtr != NULL) && (gSpecialObjectCount > 0))
//	{
//
//		iii	=	0;
//		while ((cFoundSomething == false) && (iii < gSpecialObjectCount))
//		{
//			if (strncasecmp(objectName, gSpecialObjectPtr[iii].longName, searchStrLen) == 0)
//			{
//				cFound_newRA	=	gSpecialObjectPtr[iii].org_ra;
//				cFound_newDec	=	gSpecialObjectPtr[iii].org_decl;
//
//				strcpy(cFoundDatabase, "Special Objects");
//				sprintf(cFoundName, "%s", gSpecialObjectPtr[iii].longName);
//				cFoundSomething					=	true;
//				cDispOptions.dispSpecialObjects	=	kSpecialDisp_All;
//			}
//			iii++;
//		}
//	}

	//-------------------------------------------------------------------------------
	//*	check the asteroids list
	if (cFoundSomething == false)
	{
		CONSOLE_DEBUG_W_STR("Looking in Asteroids for", objectName);
		foundIt	=	SearchAsteroids(objectName);
		if (foundIt)
		{
			cDispOptions.dispSpecialObjects	=	kSpecialDisp_All;
			strcpy(cFoundDatabase, "Asteroids");
			cDispOptions.dispAsteroids	=	true;
		}
	}

	if (cFoundSomething)
	{
	char	raDecString[64];

		if (cFound_newRA < 0.0)
		{
			cFound_newRA	+=	(2 * M_PI);
		}

		FromatRa_Dec_toString(cFound_newRA, cFound_newDec, raDecString);
		sprintf(msgString, "Found %s in %s at %s", cFoundName, cFoundDatabase, raDecString);

//		CONSOLE_DEBUG_W_STR("Found      \t",	cFoundName);
//		CONSOLE_DEBUG_W_DBL("cFound_newRA(deg) \t=",	DEGREES(cFound_newRA / 15.0));
//		CONSOLE_DEBUG_W_DBL("cFound_newDec(deg)\t=",	DEGREES(cFound_newDec));
//		CONSOLE_DEBUG_W_STR("located at \t=",	raDecString);

		cRa0		=	cFound_newRA;
		cDecl0		=	cFound_newDec;
		cFindFlag	=	true;
		ForceReDrawSky();
	}
	else
	{
		sprintf(msgString, "Nothing Found for %s", objectName);
	}
//	CONSOLE_DEBUG(__FUNCTION__);
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
//	CalanendarTime(&gCurrentSkyTime);
//	Local_Time(&gCurrentSkyTime);		//* compute local time from gmt and timezone
//	ConvertLatLonToRaDec(&cCurrLatLon, &gCurrentSkyTime);
//+++


	inCurrentWindow	=	false;
	xcoord			=	0.0;
	ycoord			=	0.0;
	xangle			=	cView_angle_Rads / 2.0;
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

	cXfactor	=	cWind_width / cView_angle_Rads;
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
//*	center the map on the specified RA/DEC
//*****************************************************************************
void	WindowTabSkyTravel::Center_RA_DEC(double argRA_radians, double argDecl_radians)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cRa0		=	argRA_radians;
	cDecl0		=	argDecl_radians;
	cFindFlag	=	true;
	ForceWindowUpdate();

}

//*****************************************************************************
//*	center the map on the specified RA/DEC
//*****************************************************************************
void	WindowTabSkyTravel::Center_CelestralObject(TYPE_CelestData *starObject)
{
double	maxViewAngle;

	CONSOLE_DEBUG(__FUNCTION__);

	maxViewAngle	=	RADIANS(10.0);
	if (starObject != NULL)
	{
		cRa0		=	starObject->ra;
		cDecl0		=	starObject->decl;
		cFindFlag	=	true;

		//*	make sure that the particular data type display is enabled
		switch(starObject->dataSrc)
		{
			case kDataSrc_Orginal:
				cDispOptions.dispDefaultData	=	true;
				break;

//			case kDataSrc_Planets:
//			case kDataSrc_Zodiac:
//				break;

			case kDataSrc_YaleBrightStar:
				cDispOptions.dispYale	=	true;
				break;

			case kDataSrc_NGC2000:
			case kDataSrc_NGC2000IC:
			case kDataSrc_OpenNGC:
				CONSOLE_DEBUG("NGC object");
				cDispOptions.dispNGC	=	true;
				break;

			case kDataSrc_HubbleGSC:
				break;

			case kDataSrc_Hipparcos:
				cDispOptions.dispHipparcos	=	true;
				break;

			case kDataSrc_Messier:
				cDispOptions.dispMessier	=	true;
				break;

			case kDataSrc_Draper:
			case kDataSrc_HYG:
			case kDataSrc_Special:
			case kDataSrc_PolarAlignCenter:
				break;

			case kDataSrc_AAVSOalert:
				//*	make sure AAVSO alerts are enabled
				SetAAVSOdisplayFlag(true);
				break;

			case kDataSrc_GAIA_SQL:
				cDispOptions.dispGaia	=	true;
				break;
		}
		SetCurrentTab(kTab_SkyTravel);
		UpdateButtonStatus();
		if (cView_angle_Rads > maxViewAngle)
		{
			SetView_Angle(maxViewAngle);
		}
		ForceWindowUpdate();
	}
}

//*****************************************************************************
//*	put it in a separate routine so that all of the zoom settings can be consistent
//*****************************************************************************
void	WindowTabSkyTravel::SetAAVSOdisplayFlag(const bool newAAVSOdisplayState)
{
	cDispOptions.dispAAVSOalerts	=	newAAVSOdisplayState;
	SetWidgetChecked(kSkyTravel_Btn_AAVSOalerts, cDispOptions.dispAAVSOalerts);

	if (cDispOptions.dispAAVSOalerts)
	{
		if (cView_angle_Rads >= gAAVSO_maxViewAngle)
		{
			//*	make sure that we are zoomed in enough to allow the display of the AAVSO alerts
			SetMaximumViewAngle(gAAVSO_maxViewAngle);
		}
	}
}

//*****************************************************************************
void	Center_RA_DEC(double argRA_radians, double argDecl_radians)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (gSkyTravelWindow != NULL)
	{
		gSkyTravelWindow->Center_RA_DEC(argRA_radians, argDecl_radians);
	}
}

//*****************************************************************************
void	Center_CelestralObject(TYPE_CelestData *starObject)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (gSkyTravelWindow != NULL)
	{
		gSkyTravelWindow->Center_CelestralObject(starObject);
	}
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
		DrawVector(	W_BROWN,
					cWorkSpaceLeftOffset + cWind_x0,
					cWorkSpaceTopOffset + yy,
					1,
					feet_shape);
	}
}

//******************************************************
void	WindowTabSkyTravel::DrawPolarAlignmentCircles(TYPE_CelestData *polarAlignCenters, long polarAlignCnt)
{
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;
int		iii;
TYPE_CelestData	point1;
TYPE_CelestData	point2;
bool	pt1Valid;
char	theChar;
int		myColor;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, polarAlignCnt);

//	for (iii=0; iii<polarAlignCnt; iii++)
//	{
//		printf("%2d\t%s\r\n", iii, polarAlignCenters[iii].longName);
//	}

	for (theChar = 'A'; theChar <= 'Z'; theChar++)
	{
		myColor	=	GetColorFromChar(theChar);
		LLG_SetColor(myColor);

		//*	find the first CENTER
		pt1Valid	=	false;
		iii			=	0;
		while ((pt1Valid == false) && (iii < polarAlignCnt))
		{
			if ((polarAlignCenters[iii].longName[0] == theChar) && (polarAlignCenters[iii].longName[1] == '-'))
			{
//				CONSOLE_DEBUG_W_HEX("Found first point", theChar);
//				CONSOLE_DEBUG_W_NUM("Index           =", iii);
				point1		=	polarAlignCenters[iii];
				pt1Valid	=	true;
			}
			iii++;
		}
		//*	now find the next one
		while ((pt1Valid == true) && (iii < polarAlignCnt))
		{
			if ((polarAlignCenters[iii].longName[0] == theChar) && (polarAlignCenters[iii].longName[1] == '-'))
			{
//				CONSOLE_DEBUG_W_NUM("Found 2nd point at", iii);
				point2		=	polarAlignCenters[iii];

				//*	we have to points, lets see if we can plot them
				pt1inView	=	GetXYfromRA_Decl(	point1.ra,
													point1.decl,
													&pt1_XX, &pt1_YY);

				pt2inView	=	GetXYfromRA_Decl(	point2.ra,
													point2.decl,
													&pt2_XX, &pt2_YY);
				//*	if both pints are on the screen, draw it
				if (pt1inView && pt2inView)
				{
					LLG_MoveTo(pt1_XX,	pt1_YY);
					LLG_LineTo(pt2_XX,	pt2_YY);
				}
				//*	set the end point to the new start point
				point1	=	point2;
			}
			iii++;
		}
	}
}


//******************************************************
void	WindowTabSkyTravel::DrawPolarAlignmentCenterVector(TYPE_CelestData *polarAlignCenters, long polarAlignCnt)
{
bool	pt1inView;
bool	pt2inView;
short	pt1_XX, pt1_YY;
short	pt2_XX, pt2_YY;
int		iii;
TYPE_CelestData	point1;
TYPE_CelestData	point2;
bool	pt1Valid;

	//*	find the first CENTER
	pt1Valid	=	false;
	iii	=	0;
	while ((pt1Valid == false) && (iii < polarAlignCnt))
	{
		if (strstr(polarAlignCenters[iii].longName, "center") != NULL)
		{
			point1		=	polarAlignCenters[iii];
			pt1Valid	=	true;
		}
		iii++;
	}
	//*	now find the next one
	while ((pt1Valid == true) && (iii < polarAlignCnt))
	{
		if (strstr(polarAlignCenters[iii].longName, "center") != NULL)
		{
			point2		=	polarAlignCenters[iii];
			pt1Valid	=	true;

			//*	we have to points, lets see if we can plot them
			pt1inView	=	GetXYfromRA_Decl(	point1.ra,
												point1.decl,
												&pt1_XX, &pt1_YY);

			pt2inView	=	GetXYfromRA_Decl(	point2.ra,
												point2.decl,
												&pt2_XX, &pt2_YY);
			//*	if both pints are on the screen, draw it
			if (pt1inView && pt2inView)
			{
				LLG_MoveTo(pt1_XX,	pt1_YY);
				LLG_LineTo(pt2_XX,	pt2_YY);
			}
			//*	set the end point to the new start point
			point1	=	point2;
		}
		iii++;
	}

}

//********************************************************************
//*	returns the number of asteroids drawn
//********************************************************************
int	WindowTabSkyTravel::DrawAsteroids(void)
{
int		numDrawn;
int		iii;
short	pt_XX, pt_YY;
bool	ptInView;
int		theAsteroidColor;

//	SETUP_TIMING();

	numDrawn	=	0;

	theAsteroidColor	=	W_GREEN;
	if (gAsteroidPtr != NULL)
	{
		for (iii=0; iii<gAsteroidCnt; iii++)
		{
			ptInView		=	GetXYfromRA_Decl(	gAsteroidPtr[iii].StarData.ra,
													gAsteroidPtr[iii].StarData.decl,
													&pt_XX,
													&pt_YY);
			if (ptInView)
			{
				DrawAsteroidFancy(	pt_XX,
									pt_YY,
									&gAsteroidPtr[iii].StarData,
									theAsteroidColor,
									0.05,
									0.05);
				numDrawn++;
			}
		}
	}
//	DEBUG_TIMING("Time to draw asteroids");
	return(numDrawn);
}

//#define	_USE_COLORS_FOR_MILKYWAY_

#ifdef _USE_COLORS_FOR_MILKYWAY_
	int		gColorsByLevel[]	=	{ W_WHITE, W_RED, W_GREEN, W_BLUE, W_CYAN, W_MAGENTA, W_YELLOW};
#endif


//********************************************************************
int	WindowTabSkyTravel::DrawPolygons(TYPE_OutlineData *outLineArray, long outLineCount)
{
#define	kMaxPolyCount	5500
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
cv::Point	pt1;
cv::Point	ptList[kMaxPolyCount];
cv::Point	pointLoc;
int			iii;
int			pointCntr;
short		pt_XX;
short		pt_YY;
bool		ptInView;
//bool		polyIsOnScreen;
bool		drawPolygon;
cv::Scalar	milkyWayColor;
int			levelID;
int			colorLevel;
int			polygonsDrawn;
//	CONSOLE_DEBUG(__FUNCTION__);

	pointCntr		=	0;
	polygonsDrawn	=	0;
//	polyIsOnScreen	=	true;
	for (iii=0; iii<outLineCount; iii++)
	{
		levelID		=	outLineArray[iii].level;
		ptInView	=	GetXYfromRA_Decl(	outLineArray[iii].ra_rad,
											outLineArray[iii].decl_rad,
											&pt_XX,
											&pt_YY);
//		if (ptInView != true)
//		{
//			polyIsOnScreen	=	false;
//		}
		//*	look for the start of a new polygon
		if (outLineArray[iii].flag == 0)
		{
			//*	if we have accumulated a polygon, time to draw it
			if (pointCntr > 0)
			{
				drawPolygon	=	true;
				if ((pointCntr > 500) && (levelID == 1))
				{
					drawPolygon	=	false;
				}
//				if (pointCntr > 4000)
//				{
//					drawPolygon	=	false;
//				}

				if (drawPolygon)
				{
				#ifdef _USE_COLORS_FOR_MILKYWAY_
					milkyWayColor	=	LLG_GetColor(gColorsByLevel[levelID]);
				#else
					colorLevel		=	levelID * 40;
					milkyWayColor	=	CV_RGB(colorLevel, colorLevel, colorLevel);
				#endif
					cv::fillConvexPoly(	*cOpenCV_Image,
										ptList,						//	const CvPoint* pts,
										pointCntr,					//	pointCntrint npts,
										milkyWayColor,				//	color
										8,							//	int line_type CV_DEFAULT(8),
										0);							//	int shift CV_DEFAULT(0));
					polygonsDrawn++;
				}
			}
			//*	reset for the next one
			pointCntr		=	0;
//			polyIsOnScreen	=	true;
		}
		else if ((pt_XX > 0) || (pt_YY > 0))
//		else if ((pt_XX != 0) && (pt_YY != 0))
		{
			if (pointCntr < kMaxPolyCount)
			{
				ptList[pointCntr].x	=	pt_XX;
				ptList[pointCntr].y	=	pt_YY;
				pointCntr++;
			}
		}
	}

#else
	#warning "DrawPolygons() not implemented"
#endif // defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	return(polygonsDrawn);
}

//********************************************************************
int	WindowTabSkyTravel::DrawOutlines(TYPE_OutlineData *outLineArray, long outLineCount, const int color)
{
int		numDrawn;
int		iii;
short	pt_XX;
short	pt_YY;
bool	drawFlag;
bool	ptInView;
int		myColorID;

//	CONSOLE_DEBUG(__FUNCTION__);

	numDrawn	=	0;

	if (outLineArray != NULL)
	{
		if (cNightMode)
		{
			LLG_SetColor(W_DARKRED);
			myColorID	=	W_DARKRED;
		}
		else
		{
			LLG_SetColor(color);
			myColorID	=	color;
		}
		drawFlag	=	false;
		for (iii=0; iii<outLineCount; iii++)
		{
			ptInView		=	GetXYfromRA_Decl(	outLineArray[iii].ra_rad,
													outLineArray[iii].decl_rad,
													&pt_XX,
													&pt_YY);
			if (outLineArray[iii].flag == 0)
			{
				drawFlag	=	false;
			}

			if (ptInView)
			{
				if (drawFlag)
				{
				#ifdef _USE_COLORS_FOR_MILKYWAY_
					if (outLineArray[iii].level > 0)
					{
					int	levelID;
					int	colorNum;

						levelID	=	outLineArray[iii].level;
						if ((levelID >= 0) && (levelID <= 5))
						{
							colorNum	=	gColorsByLevel[levelID];
							LLG_SetColor(colorNum);
						}
						else
						{
							CONSOLE_DEBUG_W_NUM("levelID\t=", levelID);
							LLG_SetColor(W_WHITE);
						}
					}
					else
					{
						LLG_SetColor(myColorID);
					}
				#else
					if (outLineArray[iii].level == 1)
					{
						LLG_SetColor(W_WHITE);
					}
					else
					{
						LLG_SetColor(myColorID);
					}
				#endif // _USE_COLORS_FOR_MILKYWAY_
					LLG_LineTo(pt_XX, pt_YY);
				}
				else
				{
					LLG_MoveTo(pt_XX, pt_YY);
					drawFlag	=	true;
				}
				numDrawn++;
			}
			else
			{
				drawFlag	=	false;
			}
		}
		LLG_PenSize(1);
	}
	else
	{
//		CONSOLE_DEBUG("Nothing to draw");
	}
//	CONSOLE_DEBUG_W_NUM("numDrawn\t=", numDrawn);
	return(numDrawn);
}



//********************************************************************
int	WindowTabSkyTravel::DrawOpenNGC_Outlines(void)
{
int		numDrawn;

//	CONSOLE_DEBUG(__FUNCTION__);

	LLG_PenSize(gST_DispOptions.LineWidth_NGCoutlines);
//	numDrawn	=	DrawOutlines(gOpenNGC_outlines, gOpenNGC_outlineCnt, W_VDARKGRAY);
	numDrawn	=	DrawOutlines(gOpenNGC_outlines, gOpenNGC_outlineCnt, W_DARKGREEN);

	return(numDrawn);
}



#ifdef _DISPLAY_MAP_TOKENS_

#include	"MAPTOK.DTA"


//******************************************************
//* map_tokens - draw map tokens on horizon, if present
void	WindowTabSkyTravel::MapTokens(TYPE_SkyTime *timeptr, TYPE_LatLon *locptr)
{
int				xcoord,ycoord;
int				ii;
TYPE_SpherTrig	sphptr;
int				scale;

	for (ii=0; ii<kNMAPOBJS; ii++)
	{
	//	if (fabs(mapptr[ii].map_lat-locptr->latitude) > mapptr[ii].map_tol)		continue;	//* not close enough
	//	if (fabs(mapptr[ii].map_long-locptr->longitude) > mapptr[ii].map_tol)	continue;	//* not close enough
	//	if (timeptr->fJulianDay < mapptr[ii].jd_begin)							continue;	//* too early
	//	if (timeptr->fJulianDay > mapptr[ii].jd_end)							continue;	//* too late
	//	if (fabs(mapptr[ii].map_az - cAz0) > cView_angle_Rads)					continue;	//* outside the azimuth range

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
				scale	=	1 + (8 / (cView_angle_Rads * 30.0));
				DrawVector(	W_WHITE,
							cWorkSpaceLeftOffset + xcoord,
							cWorkSpaceTopOffset + ycoord,
							scale,
							map_shape[ii]);	//* draw the object
			}
		}
	}
}
#endif // _DISPLAY_MAP_TOKENS_


#endif // _ENABLE_SKYTRAVEL_
