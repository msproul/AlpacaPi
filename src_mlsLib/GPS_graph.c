//**************************************************************************************
//*	GPS graphing library
//*		uses data accumulated by the ParseNMEA library to generate html outputs
//**************************************************************************************
//*	Edit History
//*		MLS	=	Mark Sproul
//**************************************************************************************
//*	Apr 20,	2017	<MLS> Added almanac graphing
//*	Apr 21,	2017	<MLS> Added DrawGPS_AlmanacGrid()
//*	Apr 21,	2017	<MLS> Added CreateSNRdistrbutionPlot()
//*	Apr 24,	2017	<MLS> Added NMEA sentence list
//*	Apr 25,	2017	<MLS> Added Legend to satellite position plot
//*	May  3,	2017	<MLS> Added plotting of altitude over time
//*	May  9,	2017	<MLS> Added sat numbers on almanac
//*	May 15,	2017	<MLS> Added CreateSatsInUseHistoryPlot()
//*	Jun  8,	2017	<MLS> Moved graphing routines to separate file
//*	Jun  9,	2017	<MLS> Added vertical line at the current time in the history plots
//*	Jun 12,	2017	<MLS> Added CreateSatelliteTrailsGraph()
//*	Jun 13,	2017	<MLS> Added CreateSatelliteElevationGraph()
//*	Apr 27,	2024	<MLS> Added _ENABLE_HTML_OUTPUT_
//**************************************************************************************

#ifdef _ENABLE_GPS_GRAPHS_


#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<stdbool.h>
#include	<math.h>

//#define	_ENABLE_HTML_OUTPUT_

//*	MLS Librarires
#define	_ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#ifdef _USE_OPENCV_
	#include	"web_graphics_opencv.h"
#else
	#include	"web_graphics.h"
	#include	"HTMLoutput.h"
	#include	"Statistics.h"
#endif


#include	"ParseNMEA.h"

#include	"GPS_graph.h"

#include	"Statistics.c"

#define	kScreen_Width	800
#define	kScreen_Height	512
//#define	kDotSize		8
#define	kDotSize		5
#define	kSatSize		20

//#define	kDeg2Rad		(57.2957795130823)	//	360 / (2 * pi)
#define	degrees(r)						(r * 180.0 / M_PI)
#define	radians(deg)					((deg) * (M_PI / 180.0))

////*****************************************************************************
//const char	elevationGraphFileName[]	=	"gps-elevation.jpg";
//const char	snrGraphFileName[]			=	"gps-snrplot.jpg";
//const char	pdopGraphFileName[]			=	"gps-pdop.jpg";
//const char	altGraphFileName[]			=	"gps-alt.jpg";
//const char	latlonGraphFileName[]		=	"gps-latlon.jpg";
//const char	satsInUseGraphFileName[]	=	"gps-satsInUse.jpg";
//const char	posErrGraphFileName[]		=	"gps-posErr.jpg";
//const char	magVarGraphFileName[]		=	"gps-magvar.jpg";

//*****************************************************************************
static short	ReturnCompAngFromMapAng(short theMapAngle_Degrees)
{
short	computerAngle;

	computerAngle	=	360 - theMapAngle_Degrees - 270;
	while (computerAngle < 0)
	{
		computerAngle	=	360	+ computerAngle;
	}
	return(computerAngle);
}


//**************************************************************************************
static int	gSnrDivions[]	=	{	10,	20,	30,	40,	50, 100	};
static int	gSnrColors[]	=
{
	kGraphPointMode_RedDot,
	kGraphPointMode_OrangeDot,
	kGraphPointMode_YellowDot,
	kGraphPointMode_BlueDot,
	kGraphPointMode_GreenDot,
	kGraphPointMode_DkGreenDot,

	-1
};

//**************************************************************************************
void	DrawGPS_AlmanacGrid(void)
{
int		xCenter, yCenter;
int		outsideRadius;
int		circleRadius;
int		circleDiameter;
double	degrees;
double	percent;
int		x1, y1;
int		x2, y2;
int		iii;
char	legendText[32];
int		previousSNR;

//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _T6963_GRAPHICS_
	LCD.createCircle(xCenter, yCenter, circleRadius);
#endif

#define	kCircleStepDegrees	30.0

	xCenter			=	kScreen_Width / 2;
	yCenter			=	kScreen_Height / 2;
	outsideRadius	=	(kScreen_Height / 2) - 10;
	circleRadius	=	outsideRadius;


	circleDiameter	=	2 * circleRadius;
	WebGraph_Draw2DCircleFilled(xCenter, yCenter, circleDiameter, circleDiameter, _gdColorLtGrey);

	degrees	=	kCircleStepDegrees;
	while (degrees <= 90)
	{
		percent			=	degrees / 90.0;
		circleRadius	=	percent * outsideRadius;
		circleDiameter	=	2 * circleRadius;

		WebGraph_Draw2DCircle(xCenter, yCenter, circleDiameter, circleDiameter, _gdColorBlack);

		degrees			+=	kCircleStepDegrees;
	}

	//*	now draw horizontal and vertical lines
	x1	=	0;
	x2	=	kScreen_Width;
	y1	=	yCenter;
	y2	=	yCenter;
	WebGraph_Draw3Dline(x1, y1, 0, x2, y2, 0, _gdColorBlack);


	x1	=	xCenter;
	x2	=	xCenter;
	y1	=	0;
	y2	=	kScreen_Height;
	WebGraph_Draw3Dline(x1, y1, 0, x2, y2, 0, _gdColorBlack);


#define	kLegendWidth	125
//	CONSOLE_DEBUG_W_NUM("_currentYsize", _currentYsize);
	//*	draw the legend
	WebGraph_DrawLegend(0, -1,		"SNR",	kLegendWidth);
	iii			=	0;
	previousSNR	=	0;
	while (gSnrColors[iii] >= 0)
	{
		sprintf(legendText, "%2d - %2d", previousSNR, gSnrDivions[iii]);
		WebGraph_DrawLegend((iii + 1), gSnrColors[iii],	legendText,	kLegendWidth);

		previousSNR	=	gSnrDivions[iii] + 1;
		iii++;
	}
}

//**************************************************************************************
static cv::Scalar	GetColorFromSNRvalue(int snrValue)
{
int			iii;
cv::Scalar	snrColor;

	iii	=	0;
	while ((gSnrColors[iii] >= 0) && (iii < 10))
	{
		if (snrValue <= gSnrDivions[iii])
		{
			snrColor	=	WebGraph_GetColorByIndex(gSnrColors[iii]);
			break;
		}
		iii++;
	}

	return(snrColor);
}

//**************************************************************************************
static double	CalcLabelStepValueY(const double maxYvalue)
{
double	labelStepY;

	if (maxYvalue > 200000)
	{
		labelStepY	=	50000;
	}
	else if (maxYvalue > 100000)
	{
		labelStepY	=	20000;
	}
	else if (maxYvalue > 20000)
	{
		labelStepY	=	5000;
	}
	else if (maxYvalue > 10000)
	{
		labelStepY	=	2000;
	}
	else if (maxYvalue > 5000)
	{
		labelStepY	=	1000;
	}
	else if (maxYvalue > 2500)
	{
		labelStepY	=	500;
	}
	else if (maxYvalue > 1000)
	{
		labelStepY	=	200;
	}
	else
	{
		labelStepY	=	100;
	}

	if (maxYvalue < 10)
	{
		labelStepY	=	1;
	}
	else if (maxYvalue < 50)
	{
		labelStepY	=	5;
	}
	else if (maxYvalue < 100)
	{
		labelStepY	=	10;
	}

	return(labelStepY);
}

//**************************************************************************************
static void	DrawCurrentPointTimeOnGraph(double xValue, double scaleFactorX, double scaleFactorY, double yMax)
{
	WebGraph_Draw2DlineScaledXY(xValue,			0,	xValue,			20000,	scaleFactorX,	scaleFactorY,	_gdColorMagenta);
	WebGraph_Draw2DlineScaledXY(xValue + 0.03,	0,	xValue + 0.03,	20000,	scaleFactorX,	scaleFactorY,	_gdColorMagenta);
	WebGraph_Draw2DlineScaledXY(xValue + 0.06,	0,	xValue + 0.06,	20000,	scaleFactorX,	scaleFactorY,	_gdColorMagenta);
}


#ifdef _ENABLE_SATELLITE_ALMANAC_
//**************************************************************************************
void	DisplayGPSalmanac(TYPE_SatStatsStruct *theSatData, bool showSatNum)
{
int			xCenter, yCenter;
int			circleRadius;
//int			circleDiameter;
int			iii;
double		dhy;	//	Hypotenuse
double		daj;	//	Adjacent
double		dop;	//	Oposite
int			azm;
int			azimuth;
int			elevation;
//int			signal2Noise;
double		azm_radians;
double		elv_radians;
//int			satCenterX, satCenterY;
cv::Scalar	snrColor;

//	CONSOLE_DEBUG(__FUNCTION__);

	xCenter			=	kScreen_Width / 2;
	yCenter			=	kScreen_Height / 2;
	circleRadius	=	(kScreen_Height / 2) - 10;
//	circleDiameter	=	circleRadius * 2;

//	iii	=	9;
	for (iii=0; iii<kMaxNumOfSatallites; iii++)
	{
		elevation		=	theSatData[iii].elvevation;

		if (elevation > 0)
		{
			azimuth			=	theSatData[iii].azimuth;
//			signal2Noise	=	theSatData[iii].signal2Noise;

			azm				=	ReturnCompAngFromMapAng(azimuth);

			elv_radians	=	radians(elevation);
			azm_radians	=	radians(azm);

			//*	compute the distance out from the center
			dhy			=	cos(elv_radians) * (circleRadius - 3);
			daj			=	cos(azm_radians) * dhy;
			dop			=	sin(azm_radians) * dhy;

//			satCenterX	=	xCenter + daj;
//			satCenterY	=	yCenter + dop;

		#if defined(_GENERATE_GRAPHICS_)
		#error foo
			snrColor	=	GetColorFromSNRvalue(signal2Noise);

			if (iii > 32)
			{
				snrColor	=	_gdColorMagenta;
			}
			if (iii >= 64)
			{
				snrColor	=	_gdColorCyan;
			}

			WebGraph_Draw2DCircleFilled(satCenterX, satCenterY, kDotSize, kDotSize, snrColor);

			//*	this option should only be called at the very end to show the current location of the sat
			if (showSatNum)
			{
			char	satNumText[32];
			int		xOffset;

				sprintf(satNumText, "%d", theSatData[iii].satellitePRN);
		#if 1
			#define	kSatSizeOuter	31
			#define	kSatSizeIner	21
				if (theSatData[iii].satellitePRN < 10)
				{
					xOffset	=	4;
				}
				else
				{
					xOffset	=	8;
				}
				WebGraph_Draw2DCircleFilled(	satCenterX,	satCenterY,	kSatSizeOuter,	kSatSizeOuter,	snrColor);
				WebGraph_Draw2DCircle(			satCenterX,	satCenterY,	kSatSizeOuter,	kSatSizeOuter,	_gdColorBlack);
				WebGraph_Draw2DCircleFilled(	satCenterX,	satCenterY,	kSatSizeIner,	kSatSizeIner,	_gdColorWhite);
				WebGraph_DrawScreenTextLg_translated(	satNumText,
														satCenterX - xOffset,
														satCenterY + 8,
														_gdColorBlack);

		#else
				WebGraph_Draw2DCircleFilled(satCenterX, satCenterY, kSatSize, kSatSize, snrColor);
				WebGraph_Draw2DCircle(satCenterX, satCenterY, kSatSize, kSatSize, _gdColorBlack);
				WebGraph_DrawScreenTextLg_translated(	satNumText,
														satCenterX + 12,
														satCenterY + 8,
														_gdColorBlack);
		#endif
			}

		#elif defined(_T6963_GRAPHICS_)
			LCD.createLine(satCenterX - 2,	satCenterY,		satCenterX + 2,	satCenterY);
			LCD.createLine(satCenterX,		satCenterY - 2,	satCenterX,		satCenterY + 2);

		//	LCD.createCircle(satCenterX, satCenterY, 2);
		#endif
		}
	}
}


//**************************************************************************************
void	CreateSatsInUseHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *satsInUseGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kSatsInUseTracking_ArraySize];
double		satsInUse[kSatsInUseTracking_ArraySize];
double		satMode[kSatsInUseTracking_ArraySize];
int			iii;
int			maxSatsInUse;
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;
int			saveImgRetCode;
//char		legendText[32];

	WebGraph_CreateImage(800, 400);

	maxSatsInUse	=	0;
	for (iii=0; iii<kSatsInUseTracking_ArraySize; iii++)
	{
		hourValues[iii]	=	(iii * kSatsInUseTracking_deltaTime) / 3600.0;
		satsInUse[iii]	=	gNMEAdata.satsInUse[iii];	//*	we need this array as a double for plotting
		satMode[iii]	=	gNMEAdata.satMode[iii];	//*	we need this array as a double for plotting
		if (gNMEAdata.satsInUse[iii] > maxSatsInUse)
		{
			maxSatsInUse	=	gNMEAdata.satsInUse[iii];
		}
	}

	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	labelStepX	=	2;
	labelStepY	=	1;
	if (maxSatsInUse > 8)
	{
	//	endY		=	11.8;
		endY		=	15;
	}
	else
	{
	//	endY		=	9.8;
		endY		=	10;
	}
	endY		=	15;
	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kSatsInUseTracking_ArraySize, hourValues, satMode,	kGraphPointMode_DkGreenLine, &scaleFactorX, &scaleFactorY, true, true, true);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kSatsInUseTracking_ArraySize, hourValues, satsInUse,	kGraphPointMode_RedLine, NULL, NULL, true, true, true);


	//*****************************************************
	//*	figure out where the "current" time on the graph is and draw a vertical line
	currentPtOnGraph	=	gNMEAdata.gpsTime / kSatsInUseTracking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);


	WebGraph_DrawTitle("Sats in use History (Count vs Time (24-hrs))", _gdColorBlack);

	WebGraph_DrawLegend(0, kGraphPointMode_RedLine,		"Sats in use",				300);
	WebGraph_DrawLegend(1, kGraphPointMode_DkGreenLine,	"Mode, 1=none,2=2D,3=3D",	300);

	saveImgRetCode	=	WebGraph_SaveImage(imageFolderName, satsInUseGraphFileName);
	if (saveImgRetCode)
	{
		CONSOLE_DEBUG("Saving image failed");
	}
	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<CENTER>\n");
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	satsInUseGraphFileName);
		fprintf(htmlFile, "<BR>\n");
		fprintf(htmlFile, "</CENTER>\n");
		fprintf(htmlFile, "<P>\n");
	}
}


#endif	//	_ENABLE_SATELLITE_ALMANAC_


#ifdef _ENABLE_SATELLITE_TRAILS_
//**************************************************************************************
void	CreateSatelliteTrailsGraph(FILE *htmlFile, const char *imageFolderName, const char *gpsGraphFileName)
{
int			xCenter, yCenter;
int			circleRadius;
int			iii;
int			jjj;
double		dhy;	//	Hypotenuse
double		daj;	//	Adjacent
double		dop;	//	Oposite
int			azm;
int			azimuth;
int			elevation;
int			signal2Noise;
double		azm_radians;
double		elv_radians;
int			satCenterX, satCenterY;
cv::Scalar	snrColor;

#define	kSatSizeOuter	31
#define	kSatSizeIner	21

	CONSOLE_DEBUG(__FUNCTION__);

	xCenter			=	kScreen_Width / 2;
	yCenter			=	kScreen_Height / 2;
	circleRadius	=	(kScreen_Height / 2) - 10;

	WebGraph_CreateImage(kScreen_Width, kScreen_Height);
	_xCenter	=	0;
	_yCenter	=	kScreen_Height;

	DrawGPS_AlmanacGrid();

	//*	step thru the history and draw the track for each satellite
	for (iii=0; iii<kSatTrails_ArraySize; iii++)
	{
		for (jjj=0; jjj<kMaxNumOfSatallites; jjj++)
		{
			elevation		=	gSatTrails[jjj].elvevation[iii];
//			CONSOLE_DEBUG_W_NUM("elevation", elevation);
			if (elevation > 0)
			{
				azimuth			=	gSatTrails[jjj].azimuth[iii];
				signal2Noise	=	gSatTrails[jjj].signal2Noise[iii];

				azm				=	ReturnCompAngFromMapAng(azimuth);

				elv_radians	=	radians(elevation);
				azm_radians	=	radians(azm);

				//*	compute the distance out from the center
				dhy			=	cos(elv_radians) * (circleRadius - 3);
				daj			=	cos(azm_radians) * dhy;
				dop			=	sin(azm_radians) * dhy;

				satCenterX	=	xCenter + daj;
				satCenterY	=	yCenter + dop;

				snrColor	=	GetColorFromSNRvalue(signal2Noise);

				if (jjj > 32)
				{
					snrColor	=	_gdColorMagenta;
				}
				if (jjj >= 64)
				{
					snrColor	=	_gdColorCyan;
				}

				WebGraph_Draw2DCircleFilled(satCenterX, satCenterY, kDotSize, kDotSize, snrColor);
			}
		}
	}

	//*	now draw the sat in its last postion
	if ((gSatTrailsLastIdx >= 0) && (gSatTrailsLastIdx < kSatTrails_ArraySize))
	{
//		CONSOLE_DEBUG_W_NUM("gSatTrailsLastIdx", gSatTrailsLastIdx);
		for (jjj=0; jjj<kMaxNumOfSatallites; jjj++)
		{
		char	satNumText[32];
		int		xOffset;

			elevation		=	gSatTrails[jjj].elvevation[gSatTrailsLastIdx];
//			CONSOLE_DEBUG_W_NUM("elevation", elevation);
			if (elevation > 0)
			{
				azimuth			=	gSatTrails[jjj].azimuth[gSatTrailsLastIdx];
				signal2Noise	=	gSatTrails[jjj].signal2Noise[gSatTrailsLastIdx];

				azm				=	ReturnCompAngFromMapAng(azimuth);

				elv_radians	=	radians(elevation);
				azm_radians	=	radians(azm);

				//*	compute the distance out from the center
				dhy			=	cos(elv_radians) * (circleRadius - 3);
				daj			=	cos(azm_radians) * dhy;
				dop			=	sin(azm_radians) * dhy;

				satCenterX	=	xCenter + daj;
				satCenterY	=	yCenter + dop;

				snrColor	=	GetColorFromSNRvalue(signal2Noise);


				sprintf(satNumText, "%d", gSatTrails[jjj].satellitePRN);

				if (gSatTrails[jjj].satellitePRN < 10)
				{
					xOffset	=	4;
				}
				else
				{
					xOffset	=	8;
				}
				WebGraph_Draw2DCircleFilled(satCenterX, satCenterY, kSatSizeOuter, kSatSizeOuter, snrColor);
				WebGraph_Draw2DCircle(satCenterX, satCenterY, kSatSizeOuter, kSatSizeOuter, _gdColorBlack);
				WebGraph_Draw2DCircleFilled(satCenterX, satCenterY, kSatSizeIner, kSatSizeIner, _gdColorWhite);
				WebGraph_DrawScreenTextLg(				satNumText,
														satCenterX - xOffset,
														satCenterY + 8,
														_gdColorBlack);

			}
		}
	}

	WebGraph_SaveImage(imageFolderName, gpsGraphFileName);

	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	gpsGraphFileName);
		fprintf(htmlFile, "<P>\n");
	}
}


//**************************************************************************************
void	CreateSatelliteElevationGraph(FILE *htmlFile, const char *imageFolderName, const char *elevationGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kSatTrails_ArraySize];
double		satElevation[kSatTrails_ArraySize];
int			iii;
int			jjj;
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;
int			lineColor;
bool		satHasData;

	CONSOLE_DEBUG(__FUNCTION__);
	WebGraph_CreateImage(800, 400);
	WebGraph_DrawTitle("Satellite elevation vs Time (24 hrs)", _gdColorBlack);

	for (iii=0; iii<kSatTrails_ArraySize; iii++)
	{
		hourValues[iii]	=	((iii * kSatTrails_deltaTime) / 3600.0);
//		CONSOLE_DEBUG_W_DBL("hourValues\t=", hourValues[iii]);
	}

	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	endY		=	95.0;		//*	degrees
	labelStepX	=	2;
	labelStepY	=	15;
	lineColor	=	kGraphPointMode_RedLine;

	//*	step through all of the satellites
	for (jjj=0; jjj<kMaxNumOfSatallites; jjj++)
	{
		satHasData	=	false;
		//*	copy the elevation to an array of doubles
		for (iii=0; iii<kSatTrails_ArraySize; iii++)
		{
			satElevation[iii]	=	gSatTrails[jjj].elvevation[iii];
			if (satElevation[iii] > 0.0)
			{
				satHasData	=	true;
			}
		}
		if (satHasData)
		{
			WebGraph_GraphDataArray(startX, endX, labelStepX,
									startY, endY, labelStepY,
									kSatTrails_ArraySize, hourValues, satElevation,	lineColor, &scaleFactorX, &scaleFactorY, true, true, true);

			lineColor++;
			if (lineColor > kGraphPointMode_BlackLine)
			{
				lineColor	=	kGraphPointMode_RedLine;
			}
		}
	}

	//*****************************************************
	//*	figure out where the "current" time on the graph is and draw a vertical line
	currentPtOnGraph	=	gNMEAdata.gpsTime / kSatTrails_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);


	WebGraph_SaveImage(imageFolderName, elevationGraphFileName);
	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<CENTER>\n");
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	elevationGraphFileName);
		fprintf(htmlFile, "<BR>\n");
		fprintf(htmlFile, "</CENTER>\n");
		fprintf(htmlFile, "<P>\n");
	}
}

#endif	//	_ENABLE_SATELLITE_TRAILS_



#ifdef _ENABLE_SATELLITE_ALMANAC_
//**************************************************************************************
void	CreateSNRdistrbutionPlot(FILE *htmlFile, const char *imageFolderName, const char *snrGraphFileName)
{
#define	kMaxSNRcount	100
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		xValues[kMaxSNRcount];
double		snrCounts[kMaxSNRcount];		//*	we use doubles because the graph function wants doubles
double		maxSNRcount;
int			iii;

	WebGraph_CreateImage(800, 400);

	maxSNRcount	=	0;
	for (iii=0; iii<kMaxSNRcount; iii++)
	{
		xValues[iii]		=	iii;

		//*	its stored as an int, we need it as a double for the graph routine
		snrCounts[iii]	=	gNMEAdata.snrDistribution[iii];
		if (snrCounts[iii] > maxSNRcount)
		{
			maxSNRcount	=	snrCounts[iii];
		}
	}
//	CONSOLE_DEBUG_W_DBL("maxSNRcount", maxSNRcount);

	startX		=	0;
	endX		=	109;
	startY		=	0.0;
	endY		=	maxSNRcount * 1.2;
	labelStepX	=	10;
	labelStepY	=	CalcLabelStepValueY(maxSNRcount);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kMaxSNRcount, xValues, snrCounts,	kGraphPointMode_DkGreenLine, NULL, NULL, true, true, true);

	WebGraph_DrawTitle("SNR distribution (count vs SNR)", _gdColorBlack);
	WebGraph_SaveImage(imageFolderName, snrGraphFileName);
	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	snrGraphFileName);
		fprintf(htmlFile, "<P>\n");
	}
}
#endif	//	_ENABLE_SATELLITE_ALMANAC_


#ifdef _ENABLE_PDOP_TRACKING_
//**************************************************************************************
void	CreatePDOPhistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *pdopGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kPDOPtacking_ArraySize];
double		maxPDOP;
int			iii;
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

	WebGraph_CreateImage(800, 400);

	maxPDOP	=	0;
	for (iii=0; iii<kPDOPtacking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kPDOPtacking_deltaTime) / 3600.0;

		if (gNMEAdata.pdopHistory[iii] > maxPDOP)
		{
			maxPDOP	=	gNMEAdata.pdopHistory[iii];
		}
	}

	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	endY		=	maxPDOP * 1.2;
	labelStepX	=	2;
	labelStepY	=	5;
	if (maxPDOP < 5)
	{
		labelStepY	=	1;
	}
	else if (maxPDOP < 10)
	{
		labelStepY	=	2;
	}

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPDOPtacking_ArraySize, hourValues, gNMEAdata.pdopHistory,	kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);


	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPDOPtacking_ArraySize, hourValues, gNMEAdata.vdopHistory,	kGraphPointMode_GreenLine, NULL, NULL, true, true, true);


	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPDOPtacking_ArraySize, hourValues, gNMEAdata.hdopHistory,	kGraphPointMode_BlueLine, NULL, NULL, true, true, true);

	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kPDOPtacking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);



//	CONSOLE_DEBUG_W_NUM("_currentYsize", _currentYsize);
	WebGraph_DrawLegend(0, kGraphPointMode_RedLine,		"PDOP",	150);
	WebGraph_DrawLegend(1, kGraphPointMode_GreenLine,	"VDOP",	150);
	WebGraph_DrawLegend(2, kGraphPointMode_BlueLine,	"HDOP",	150);

	WebGraph_DrawTitle("PDOP History (PDOP vs Time (24-hrs)) lower is better", _gdColorBlack);
	WebGraph_SaveImage(imageFolderName, pdopGraphFileName);
	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	pdopGraphFileName);
		fprintf(htmlFile, "<P>\n");
	}

}
#endif	//	_ENABLE_PDOP_TRACKING_


#ifdef _ENABLE_LAT_LON_TRACKING_
//**************************************************************************************
void	CreateLatLonHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *latlonGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kLatLonTacking_ArraySize];
double		validLatValues[kLatLonTacking_ArraySize];
double		validLonValues[kLatLonTacking_ArraySize];
int			validLatLonIdx;
//double		latitude_avg;
//double		latitude_std;
//double		longitude_avg;
//double		longitude_std;
double		maxLatitude;
double		minLatitude;
double		minLongitude;
double		maxLongitude;
int			iii;
int			ccc;
char		legendText[32];
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

//	CONSOLE_DEBUG(__FUNCTION__);
	WebGraph_CreateImage(800, 400);

	maxLatitude		=	0;
	minLatitude		=	0;
	minLongitude	=	0;
	maxLongitude	=	0;
	validLatLonIdx	=	0;
	for (iii=0; iii<kLatLonTacking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kLatLonTacking_deltaTime) / 3600.0;
		if (gNMEAdata.latitudeHistory[iii] > maxLatitude)
		{
			maxLatitude		=	gNMEAdata.latitudeHistory[iii];
		}
		if (gNMEAdata.latitudeHistory[iii] < minLatitude)
		{
			minLatitude		=	gNMEAdata.latitudeHistory[iii];
		}

		if (gNMEAdata.longitudeHistory[iii] > maxLongitude)
		{
			maxLongitude	=	gNMEAdata.longitudeHistory[iii];
		}
		if (gNMEAdata.longitudeHistory[iii] < minLongitude)
		{
			minLongitude	=	gNMEAdata.longitudeHistory[iii];
		}

		if ((gNMEAdata.latitudeHistory[iii] != 0.0) && (gNMEAdata.longitudeHistory[iii] != 0.0))
		{
			validLatValues[validLatLonIdx]	=	gNMEAdata.latitudeHistory[iii];
			validLonValues[validLatLonIdx]	=	gNMEAdata.longitudeHistory[iii];

			validLatLonIdx++;
		}
	}

	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	endY		=	95.0;
	labelStepX	=	2;
	labelStepY	=	10;

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kLatLonTacking_ArraySize, hourValues, gNMEAdata.latitudeHistory,	kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kLatLonTacking_ArraySize, hourValues, gNMEAdata.longitudeHistory,	kGraphPointMode_GreenLine, NULL, NULL, true, true, true);


	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kLatLonTacking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);


	gNMEAdata.latitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLatValues);
	gNMEAdata.latitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLatValues);

	gNMEAdata.longitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLonValues);
	gNMEAdata.longitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLonValues);

#define	kLegendWidth_Lat	260

	ccc	=	0;
	WebGraph_DrawLegend(ccc++, kGraphPointMode_RedLine,		"Latitude",		kLegendWidth_Lat);
	WebGraph_DrawLegend(ccc++, kGraphPointMode_GreenLine,	"Longitude",	kLegendWidth_Lat);

	sprintf(legendText, "avg Lat = %7.3f", gNMEAdata.latitude_avg);
	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);
	sprintf(legendText, "std Lat  = %7.3f", gNMEAdata.latitude_std);
	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);

	sprintf(legendText, "avg Lon = %7.3f", gNMEAdata.longitude_avg);
	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);
	sprintf(legendText, "std Lon  = %7.3f", gNMEAdata.longitude_std);
	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);

	WebGraph_DrawTitle("Lat/Lon History (Lat/Lon vs Time (24-hrs))", _gdColorBlack);

	WebGraph_SaveImage(imageFolderName, latlonGraphFileName);

#ifdef _NOT_SURE_YET_
	if (htmlFile != NULL)
	{
	double	delataMiles;
	double	delataFeet;

		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	latlonGraphFileName);
		fprintf(htmlFile, "<BR>\n");
		fprintf(htmlFile, "<CENTER>\n");
		fprintf(htmlFile, "<TABLE BORDER=1>\n");
		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "");
			PrintHTMLtableCell(htmlFile, "<CENTER>Avg");
			PrintHTMLtableCell(htmlFile, "<CENTER>Std Dev");
			PrintHTMLtableCell(htmlFile, "<CENTER>&sigma; Miles");
			PrintHTMLtableCell(htmlFile, "<CENTER>&sigma; Feet");
		fprintf(htmlFile, "		</TR>\n");


		delataMiles	=	(latitude_std / 360.0) * 24901.0;
		delataFeet	=	delataMiles * 5280;

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Latitude");
			PrintHTMLtableCellDouble(htmlFile, latitude_avg, kFormat_6_4, kUnits_degrees);
			PrintHTMLtableCellDouble(htmlFile, latitude_std, kFormat_6_4, kUnits_degrees);
			PrintHTMLtableCellDouble(htmlFile, delataMiles, kFormat_6_4, kUnits_none);
			PrintHTMLtableCellDouble(htmlFile, delataFeet, kFormat_6_4, kUnits_feet);
		fprintf(htmlFile, "		</TR>\n");

		delataMiles	=	(longitude_std / 360.0) * 24901.0;
		delataFeet	=	delataMiles * 5280;

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Longitude");
			PrintHTMLtableCellDouble(htmlFile, longitude_avg, kFormat_6_4, kUnits_degrees);
			PrintHTMLtableCellDouble(htmlFile, longitude_std, kFormat_6_4, kUnits_degrees);
			PrintHTMLtableCellDouble(htmlFile, delataMiles, kFormat_6_4, kUnits_none);
			PrintHTMLtableCellDouble(htmlFile, delataFeet, kFormat_6_4, kUnits_feet);
		fprintf(htmlFile, "		</TR>\n");


		fprintf(htmlFile, "</TABLE>\n");
		fprintf(htmlFile, "</CENTER>\n");
		fprintf(htmlFile, "<P>\n");
	}
#endif // _NOT_SURE_YET_
}


//**************************************************************************************
void	CreateLatDetailHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *latlonGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kLatLonTacking_ArraySize];
double		validLatValues[kLatLonTacking_ArraySize];
double		validLonValues[kLatLonTacking_ArraySize];
int			validLatLonIdx;
double		maxLatitude;
double		minLatitude;
double		minLongitude;
double		maxLongitude;
int			iii;
int			ccc;
char		legendText[32];
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

	CONSOLE_DEBUG(__FUNCTION__);
	WebGraph_CreateImage(800, 400);

	maxLatitude		=	0;
	minLatitude		=	0;
	minLongitude	=	0;
	maxLongitude	=	0;
	validLatLonIdx	=	0;
	for (iii=0; iii<kLatLonTacking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kLatLonTacking_deltaTime) / 3600.0;
		if (gNMEAdata.latitudeHistory[iii] > maxLatitude)
		{
			maxLatitude		=	gNMEAdata.latitudeHistory[iii];
		}
		if (gNMEAdata.latitudeHistory[iii] < minLatitude)
		{
			minLatitude		=	gNMEAdata.latitudeHistory[iii];
		}

		if (gNMEAdata.longitudeHistory[iii] > maxLongitude)
		{
			maxLongitude	=	gNMEAdata.longitudeHistory[iii];
		}
		if (gNMEAdata.longitudeHistory[iii] < minLongitude)
		{
			minLongitude	=	gNMEAdata.longitudeHistory[iii];
		}

		if ((gNMEAdata.latitudeHistory[iii] != 0.0) && (gNMEAdata.longitudeHistory[iii] != 0.0))
		{
			validLatValues[validLatLonIdx]	=	gNMEAdata.latitudeHistory[iii];
			validLonValues[validLatLonIdx]	=	gNMEAdata.longitudeHistory[iii];

			validLatLonIdx++;
		}
	}

	gNMEAdata.latitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLatValues);
	gNMEAdata.latitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLatValues);

	gNMEAdata.longitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLonValues);
	gNMEAdata.longitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLonValues);

	startX		=	0;
	endX		=	25;
	startY		=	gNMEAdata.latitude_avg - 0.1;
	endY		=	gNMEAdata.latitude_avg + 0.1;
	labelStepX	=	2;
	labelStepY	=	0.025;

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kLatLonTacking_ArraySize, hourValues, gNMEAdata.latitudeHistory,	kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);



	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kLatLonTacking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);


	gNMEAdata.latitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLatValues);
	gNMEAdata.latitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLatValues);

	gNMEAdata.longitude_avg	=	STAT_CalcMean(validLatLonIdx,			validLonValues);
	gNMEAdata.longitude_std	=	STAT_CalcStdDeviation(validLatLonIdx,	validLonValues);

#define	kLegendWidth_Lat	260

	ccc	=	0;
	WebGraph_DrawLegend(ccc++, kGraphPointMode_RedLine,		"Latitude",		kLegendWidth_Lat);
//	WebGraph_DrawLegend(ccc++, kGraphPointMode_GreenLine,	"Longitude",	kLegendWidth_Lat);

//	sprintf(legendText, "avg Lat = %7.3f", gNMEAdata.latitude_avg);
//	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);
//	sprintf(legendText, "std Lat  = %7.3f", gNMEAdata.latitude_std);
//	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);
//
//	sprintf(legendText, "avg Lon = %7.3f", gNMEAdata.longitude_avg);
//	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);
//	sprintf(legendText, "std Lon  = %7.3f", gNMEAdata.longitude_std);
//	WebGraph_DrawLegend(ccc++, -1,		legendText,	kLegendWidth_Lat);

	WebGraph_DrawTitle("Lat/ History (Lat/Lon vs Time (24-hrs))", _gdColorBlack);

	WebGraph_SaveImage(imageFolderName, latlonGraphFileName);

}

#endif	//	_ENABLE_LAT_LON_TRACKING_

#ifdef _ENABLE_ALTITUDE_TRACKING_
//**************************************************************************************
void	CreateAltitudeHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *altGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kAltTacking_ArraySize];
double		validAltValues[kAltTacking_ArraySize];
int			validAltIdx;
double		maxAltitude;
double		minAltitude;
int			iii;
int			jjj;
double		altitude_avg;
double		altitude_std;
char		legendText[32];
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

	WebGraph_CreateImage(800, 400);

	maxAltitude	=	0;
	minAltitude	=	0;
	validAltIdx	=	0;
	for (iii=0; iii<kAltTacking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kAltTacking_deltaTime) / 3600.0;

		if (gNMEAdata.altitudeHistory[iii] > maxAltitude)
		{
			maxAltitude	=	gNMEAdata.altitudeHistory[iii];
		}
		if (gNMEAdata.altitudeHistory[iii] < minAltitude)
		{
			minAltitude	=	gNMEAdata.altitudeHistory[iii];
		}

		if (gNMEAdata.altitudeHistory[iii] != 0.0)
		{
			validAltValues[validAltIdx]	=	gNMEAdata.altitudeHistory[iii];

			validAltIdx++;
		}
	}
	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	endY		=	maxAltitude * 1.6;
	if (minAltitude < 0)
	{
		startY	=	-600;
	}
	labelStepX	=	2;
	labelStepY	=	CalcLabelStepValueY(maxAltitude);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kAltTacking_ArraySize, hourValues, gNMEAdata.altitudeHistory,	kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);

	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kAltTacking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, (maxAltitude * 2));


	altitude_avg	=	STAT_CalcMean(validAltIdx, validAltValues);
	altitude_std	=	STAT_CalcStdDeviation(validAltIdx, validAltValues);

#define	kLegendWidth_Alt	255

	jjj	=	0;
	WebGraph_DrawLegend(jjj++, kGraphPointMode_RedLine,		"Altitude (ft)",	kLegendWidth_Alt);

	sprintf(legendText, "avgerage = %5.1f", altitude_avg);
	WebGraph_DrawLegend(jjj++, -1,		legendText,	kLegendWidth_Alt);

	sprintf(legendText, "min Alt  = %5.1f", gNMEAdata.minAltitude);
	WebGraph_DrawLegend(jjj++, -1,		legendText,	kLegendWidth_Alt);

	sprintf(legendText, "max Alt  = %5.1f", gNMEAdata.maxAltitude);
	WebGraph_DrawLegend(jjj++, -1,		legendText,	kLegendWidth_Alt);

	sprintf(legendText, "std dev  = %5.1f", altitude_std);
	WebGraph_DrawLegend(jjj++, -1,		legendText,	kLegendWidth_Alt);

	WebGraph_DrawTitle("Altitude History (Alt vs Time (24-hrs))", _gdColorBlack);

	WebGraph_SaveImage(imageFolderName, altGraphFileName);

	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	altGraphFileName);
		fprintf(htmlFile, "<P>\n");
	}
}
#endif	//	_ENABLE_ALTITUDE_TRACKING_

#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
//**************************************************************************************
void	CreatePositionErrorHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *posErrGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kPosErrTacking_ArraySize];
double		maxError;
int			iii;
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

//	CONSOLE_DEBUG(__FUNCTION__);

	WebGraph_CreateImage(800, 400);

	maxError	=	0;
	for (iii=0; iii<kPosErrTacking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kPosErrTacking_deltaTime) / 3600.0;

		if (gNMEAdata.horzPosErrArry[iii] > maxError)
		{
			maxError	=	gNMEAdata.horzPosErrArry[iii];
		}
		if (gNMEAdata.vertPosErrArry[iii] > maxError)
		{
			maxError	=	gNMEAdata.vertPosErrArry[iii];
		}
		if (gNMEAdata.sphrPosErrArry[iii] > maxError)
		{
			maxError	=	gNMEAdata.sphrPosErrArry[iii];
		}
	}

	startX		=	0;
	endX		=	25;
	startY		=	0.0;
	endY		=	maxError * 1.2;
	labelStepX	=	2;
	labelStepY	=	10;

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPosErrTacking_ArraySize, hourValues, gNMEAdata.horzPosErrArry,	kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPosErrTacking_ArraySize, hourValues, gNMEAdata.vertPosErrArry,	kGraphPointMode_GreenLine, NULL, NULL, true, true, true);

	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,	kPosErrTacking_ArraySize, hourValues, gNMEAdata.sphrPosErrArry,	kGraphPointMode_BlueLine, NULL, NULL, true, true, true);


	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kPosErrTacking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);


	WebGraph_DrawLegend(0, kGraphPointMode_RedLine,		"Horz Error (M)",	230);
	WebGraph_DrawLegend(1, kGraphPointMode_GreenLine,	"Vert Error (M)",	230);
	WebGraph_DrawLegend(2, kGraphPointMode_BlueLine,	"Spher Error (M)",	230);

	WebGraph_DrawTitle("Position Error History vs Time (24-hrs) (lower is better)", _gdColorBlack);
	WebGraph_SaveImage(imageFolderName, posErrGraphFileName);

	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	posErrGraphFileName);
		fprintf(htmlFile, "<P>\n");
	}
}
#endif


#ifdef _ENABLE_MAGNETIC_VARIATION_TRACKING_

//**************************************************************************************
static void	CreateMagneticVariationHistoryPlot(FILE *htmlFile, const char *imageFolderName, const char *magVarGraphFileName)
{
double		startX;
double		endX;
double		labelStepX;
double		startY;
double		endY;
double		labelStepY;
double		hourValues[kMagVariationTracking_ArraySize];
int			iii;
int			currentPtOnGraph;
double		scaleFactorX;
double		scaleFactorY;

	WebGraph_CreateImage(800, 400);

	for (iii=0; iii<kMagVariationTracking_ArraySize; iii++)
	{
		hourValues[iii]		=	(iii * kMagVariationTracking_deltaTime) / 3600.0;

	}

	startX		=	0;
	endX		=	25;
	startY		=	-25.0;
	endY		=	25.0;
	labelStepX	=	2;
	labelStepY	=	5;


	WebGraph_GraphDataArray(startX, endX, labelStepX,
							startY, endY, labelStepY,
							kMagVariationTracking_ArraySize,
							hourValues,
							gNMEAdata.magVariationArray,
							kGraphPointMode_RedLine, &scaleFactorX, &scaleFactorY, true, true, true);


	//*****************************************************
	//*	figure out where the "current" time on the graph is
	currentPtOnGraph	=	gNMEAdata.gpsTime / kMagVariationTracking_deltaTime;
	DrawCurrentPointTimeOnGraph(hourValues[currentPtOnGraph], scaleFactorX, scaleFactorY, 20000);

	WebGraph_DrawTitle("Magnetic Variation History (Degrees vs Time (24-hrs))", _gdColorBlack);

//	WebGraph_DrawLegend(0, kGraphPointMode_RedLine,		"Sats in use",				300);
//	WebGraph_DrawLegend(1, kGraphPointMode_DkGreenLine,	"Mode, 1=none,2=2D,3=3D",	300);

	WebGraph_SaveImage(imageFolderName, magVarGraphFileName);

	if (htmlFile != NULL)
	{
		fprintf(htmlFile, "<CENTER>\n");
		fprintf(htmlFile, "<IMG SRC=%s%s>\n",	imageFolderName,	magVarGraphFileName);
		fprintf(htmlFile, "<BR>\n");
		fprintf(htmlFile, "</CENTER>\n");
		fprintf(htmlFile, "<P>\n");
	}
}
#endif	//	_ENABLE_MAGNETIC_VARIATION_TRACKING_


#ifdef _ENABLE_HTML_OUTPUT_

//**************************************************************************************
int	SaveGPS_HTMLandGRAPHS(const char *imageFolderName, const char *htmlFileName, bool dataIsLive)
{
int		iii;
int		totalSentanceCnt;
int		activeSatCnt;
FILE	*htmlFile;
#ifdef _LIVE_DATA_
	char	gpsGraphFileName[]		=	"gps-graph.jpg";
#else
	char	gpsGraphFileName[]		=	"gps-graph1.jpg";
#endif
char	timeString[32];


	htmlFile	=	fopen(htmlFileName, "w");
	if (htmlFile != NULL)
	{

		sprintf(timeString, "%02d:%02d:%02d",	gNMEAdata.gpsTimeHHMMSS.hours,
												gNMEAdata.gpsTimeHHMMSS.minutes,
												gNMEAdata.gpsTimeHHMMSS.seconds);

		PrintHTMLheader(htmlFile, "GPS 24 hour performance", "Mark Sproul");
		fprintf(htmlFile, "<CENTER>\n");

		fprintf(htmlFile, "<TABLE BORDER=1>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Data source");
			if (dataIsLive)
			{
				PrintHTMLtableCell(htmlFile, "LIVE");
			}
			else
			{
				PrintHTMLtableCell(htmlFile, "Logfile");
			}
		fprintf(htmlFile, "		</TR>\n");


		if (strlen(gNMEAdata.gpsModel) > 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "GPS Model");
				PrintHTMLtableCell(htmlFile, gNMEAdata.gpsModel);
			fprintf(htmlFile, "		</TR>\n");
		}

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Valid Date");
			PrintHTMLtableCellBool(htmlFile, gNMEAdata.validDate);
		fprintf(htmlFile, "		</TR>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Valid Time");
			PrintHTMLtableCellBool(htmlFile, gNMEAdata.validTime);
		fprintf(htmlFile, "		</TR>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Valid Lat/Lon");
			PrintHTMLtableCellBool(htmlFile, gNMEAdata.validLatLon);
		fprintf(htmlFile, "		</TR>\n");


		if (strlen(gNMEAdata.mapDatum) > 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "Map Datum");
				PrintHTMLtableCell(htmlFile, gNMEAdata.mapDatum);
			fprintf(htmlFile, "		</TR>\n");
		}

//		fprintf(htmlFile, "		<TR>\n");
//			PrintHTMLtableCell(htmlFile, "Time");
//			PrintHTMLtableCell(htmlFile, timeString);
//		fprintf(htmlFile, "		</TR>\n");


		if (gNMEAdata.gpsSensorTemp != 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "GPS Sensor Temp");
				PrintHTMLtableCell_INT(htmlFile, gNMEAdata.gpsSensorTemp, kFormat_6_0, kUnits_degreesC);
			fprintf(htmlFile, "		</TR>\n");
		}

		if (gNMEAdata.zzAlt != 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "Altitude");
				PrintHTMLtableCell_INT(htmlFile, gNMEAdata.zzAlt, kFormat_6_0, kUnits_feet);
			fprintf(htmlFile, "		</TR>\n");
		}
		if (gNMEAdata.altitudeMeters != 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "Altitude");
				PrintHTMLtableCell_INT(htmlFile, gNMEAdata.altitudeMeters, kFormat_6_0, kUnits_meters);
			fprintf(htmlFile, "		</TR>\n");
		}

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Latest Date string");
			PrintHTMLtableCell(htmlFile, gNMEAdata.theNN.Date);
		fprintf(htmlFile, "		</TR>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Latest Time string");
			PrintHTMLtableCell(htmlFile, gNMEAdata.theNN.Time);
		fprintf(htmlFile, "		</TR>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Latest Lat string");
			PrintHTMLtableCell(htmlFile, gNMEAdata.theNN.Lat);
		fprintf(htmlFile, "		</TR>\n");

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "Latest Lon string");
			PrintHTMLtableCell(htmlFile, gNMEAdata.theNN.Lon);
		fprintf(htmlFile, "		</TR>\n");


		if (gNMEAdata.magneticVariation != 0)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile, "Magnetic Variation");
				PrintHTMLtableCellDouble(htmlFile, gNMEAdata.magneticVariation, kFormat_6_1, kUnits_degrees);
			fprintf(htmlFile, "		</TR>\n");
		}


		fprintf(htmlFile, "</TABLE>\n");


#ifdef _ENABLE_SATELLITE_TRAILS_
		CreateSatelliteTrailsGraph(htmlFile, imageFolderName,		gpsGraphFileName);
		CreateSatelliteElevationGraph(htmlFile, imageFolderName,	elevationGraphFileName);
#endif

#ifdef _ENABLE_SATELLITE_ALMANAC_
		CreateSNRdistrbutionPlot(htmlFile, imageFolderName,	snrGraphFileName);

		CreateSatsInUseHistoryPlot(htmlFile, imageFolderName,		satsInUseGraphFileName);
#endif



#ifdef _ENABLE_LAT_LON_TRACKING_
		CreateLatLonHistoryPlot(htmlFile, imageFolderName,			latlonGraphFileName);
		CreateLatDetailHistoryPlot(htmlFile, imageFolderName,		"latitudeDetail.jpg");
#endif

#ifdef _ENABLE_ALTITUDE_TRACKING_
		CreateAltitudeHistoryPlot(htmlFile, imageFolderName,	altGraphFileName);
#endif


#ifdef _ENABLE_PDOP_TRACKING_
		CreatePDOPhistoryPlot(htmlFile, imageFolderName,		pdopGraphFileName);
#endif

#ifdef _ENABLE_NMEA_POSITION_ERROR_TRACKING_
		if (gNMEAdata.PGRME_exists)
		{
			CONSOLE_DEBUG(__FUNCTION__);
			CreatePositionErrorHistoryPlot(htmlFile, imageFolderName,	posErrGraphFileName);
		}
#endif

#ifdef _ENABLE_MAGNETIC_VARIATION_TRACKING_
		if (gNMEAdata.magneticVariation != 0)
		{
			CreateMagneticVariationHistoryPlot(htmlFile, imageFolderName,		magVarGraphFileName);
		}
#endif

#ifdef _ENABLE_SATELLITE_ALMANAC_
		//*********************************************************************************
		fprintf(htmlFile, "<TABLE BORDER=1>\n");
		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "<CENTER>idx");
			PrintHTMLtableCell(htmlFile, "<CENTER>Sat PRN");
			PrintHTMLtableCell(htmlFile, "<CENTER>cnt");
			PrintHTMLtableCell(htmlFile, "<CENTER>elev");
			PrintHTMLtableCell(htmlFile, "<CENTER>az");
			PrintHTMLtableCell(htmlFile, "<CENTER>SNR");
			PrintHTMLtableCell(htmlFile, "<CENTER>max SNR");
		fprintf(htmlFile, "		</TR>\n");

		activeSatCnt	=	0;

		for (iii=1; iii<kMaxNumOfSatallites; iii++)
		{
			if (gNMEAdata.theSats[iii].reportCnt > 0)
			{
				fprintf(htmlFile, "		<TR>\n");
					PrintHTMLtableCell_INT(htmlFile, iii,									kFormat_6_0, kUnits_none);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].satellitePRN,	kFormat_6_0, kUnits_none);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].reportCnt,		kFormat_6_0, kUnits_none);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].elvevation,		kFormat_6_0, kUnits_degrees);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].azimuth,		kFormat_6_0, kUnits_degrees);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].signal2Noise,	kFormat_6_0, kUnits_none);
					PrintHTMLtableCell_INT(htmlFile, gNMEAdata.theSats[iii].maxSNR,			kFormat_6_0, kUnits_none);
				fprintf(htmlFile, "		</TR>\n");

				activeSatCnt++;
			}
		}
		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile,		"<CENTER>total");
			PrintHTMLtableCell_INT(htmlFile,	activeSatCnt,	kFormat_6_0, kUnits_none);
		fprintf(htmlFile, "		</TR>\n");
		fprintf(htmlFile, "</TABLE>\n");
#endif


#ifdef _ENABLE_NMEA_SENTANCE_TRACKING_

		//*********************************************************************************
		fprintf(htmlFile, "<TABLE BORDER=1>\n");
		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile, "<CENTER>NMEA ID");
			PrintHTMLtableCell(htmlFile, "<CENTER>Count");
			PrintHTMLtableCell(htmlFile, "<CENTER>Last data");
		fprintf(htmlFile, "		</TR>\n");

		totalSentanceCnt	=	0;
		for (iii=0; iii<kMaxNMEAsentances; iii++)
		{
			if (gNMEAsentances[iii].count > 0)
			{
				totalSentanceCnt	+=	gNMEAsentances[iii].count;
				fprintf(htmlFile, "		<TR>\n");
					PrintHTMLtableCell(htmlFile,		gNMEAsentances[iii].nmeaID);
					PrintHTMLtableCell_INT(htmlFile,	gNMEAsentances[iii].count,	kFormat_6_0, kUnits_none);
					PrintHTMLtableCell(htmlFile,		gNMEAsentances[iii].lastData);
				fprintf(htmlFile, "		</TR>\n");
			}
		}

		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile,		"<CENTER>total");
			PrintHTMLtableCell_INT(htmlFile,	totalSentanceCnt,	kFormat_6_0, kUnits_none);
		fprintf(htmlFile, "		</TR>\n");

//		fprintf(htmlFile, "		<TR>\n");
//			PrintHTMLtableCell(htmlFile,		"<CENTER>total");
//			PrintHTMLtableCell_INT(htmlFile,	gNMEAdataRecorded,	kFormat_6_0, kUnits_none);
//		fprintf(htmlFile, "		</TR>\n");



		fprintf(htmlFile, "</TABLE>\n");
#endif

		//**********************************************************
		fprintf(htmlFile, "<TABLE BORDER=1>\n");
		if (dataIsLive)
		{
			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile,		"Total time adjustments");
				PrintHTMLtableCell_INT(htmlFile,	gTimeAdjustmentCount,	kFormat_6_0, kUnits_none);
			fprintf(htmlFile, "		</TR>\n");

			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile,		"Clock was ahead");
				PrintHTMLtableCell_INT(htmlFile,	gTimeAdjustmentAhead,	kFormat_6_0, kUnits_none);
			fprintf(htmlFile, "		</TR>\n");

			fprintf(htmlFile, "		<TR>\n");
				PrintHTMLtableCell(htmlFile,		"Clock was behind");
				PrintHTMLtableCell_INT(htmlFile,	gTimeAdjustmentBehind,	kFormat_6_0, kUnits_none);
			fprintf(htmlFile, "		</TR>\n");
		}
		fprintf(htmlFile, "		<TR>\n");
			PrintHTMLtableCell(htmlFile,		"Invalid Time reports");
			PrintHTMLtableCell_INT(htmlFile,	gNMEAdata.invalidTimeCount,	kFormat_6_0, kUnits_none);
		fprintf(htmlFile, "		</TR>\n");


		fprintf(htmlFile, "</TABLE>\n");


		fprintf(htmlFile, "</CENTER>\n");
		fprintf(htmlFile, "Compiled %s %s\n", __DATE__, __TIME__);


		fclose(htmlFile);
		htmlFile	=	NULL;
	}
	return 0;
}
#endif // _ENABLE_HTML_OUTPUT_


#endif // _ENABLE_GPS_GRAPHS_
