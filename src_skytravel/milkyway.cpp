//*****************************************************************************
//*		milkyway.cpp		(c) 2024 by Mark Sproul
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
//*	May 29,	2024	<MLS> Created milkyway.cpp
//*	May 29,	2024	<MLS> Added Milkyway_ReadOutlines() & MilkyWay_ParsePointData()
//*****************************************************************************
//*		https://github.com/ofrohn/d3-celestial
//*		git clone https://github.com/ofrohn/d3-celestial.git
//*****************************************************************************


#ifndef _ENABLE_SKYTRAVEL_
//	#define _INCLUDE_MILKYWAY_MAIN_
#endif // _ENABLE_SKYTRAVEL_


#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"outlinedata.h"
#include	"milkyway.h"

#ifdef _ENABLE_SKYTRAVEL_
	#include	"controller_startup.h"
#endif // _ENABLE_SKYTRAVEL_


#define	kMilkyWayMaxPoints	32000

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

static char	gOutLineLevel;
static int	gPolyGonPointCount;
static int	gPolyGonPointCountMax;

//************************************************************************
//		{"type":"FeatureCollection",
//		"features":[{"type":"Feature",
//		"id":"ol1",
//		"properties":{},
//		"geometry":{"type":"Polygon",
//		"coordinates":[[[97.754,34.659],
//		[97.854,34.483],
//		[97.904,34.394],
//		[98.067,34.348],
//************************************************************************
//*	returns true if the data point was valid
//************************************************************************
bool	MilkyWay_ParsePointData(char *dataPointString, TYPE_OutlineData *dataPoint)
{
bool	pointIsValid;
bool	newPolygon;
double	raValue_deg;
double	decValue_deg;
char	*tripleBrktPtr;
char	*dataValuePtr;
int		ccc;

	pointIsValid	=	false;
	newPolygon		=	false;
	raValue_deg		=	0.0;
	decValue_deg	=	0.0;

	if (dataPointString[0] == '[')
	{
		gPolyGonPointCount++;
		dataValuePtr	=	dataPointString;
		dataValuePtr++;
		if (dataPointString[1] == '[')
		{
			newPolygon		=	true;
			dataValuePtr++;

//			CONSOLE_DEBUG_W_NUM("gPolyGonPointCount\t=",	gPolyGonPointCount);
			if (gPolyGonPointCount > gPolyGonPointCountMax)
			{
				gPolyGonPointCountMax	=	gPolyGonPointCount;
			}
			gPolyGonPointCount	=	0;
		}
		raValue_deg		=	atof(dataValuePtr);
		ccc				=	0;
		while ((*dataValuePtr != ',') && (ccc < 12))
		{
			dataValuePtr++;
		}
		dataValuePtr++;
		decValue_deg	=	atof(dataValuePtr);
		pointIsValid	=	true;
	}
	else if (dataPointString[0] == '"')
	{
		if (dataPointString[1] == 'c')
		{
			//*	we have the first of a new polygon
			newPolygon		=	true;
			tripleBrktPtr	=	strstr(dataPointString, "[[[");
			if (tripleBrktPtr != NULL)
			{
				tripleBrktPtr	+=	3;
				raValue_deg		=	atof(tripleBrktPtr);
				ccc				=	0;
				while ((*tripleBrktPtr != ',') && (ccc < 12))
				{
					tripleBrktPtr++;
				}
				tripleBrktPtr++;
				decValue_deg	=	atof(tripleBrktPtr);
				pointIsValid	=	true;
			}
		}
		else if ((dataPointString[1] == 'i') && (dataPointString[2] == 'd'))
		{
			CONSOLE_DEBUG(dataPointString);
			//*	this is the level ID, values 1 -> 5
			//	"id":"ol1",
			//	"id":"ol2",
			//	"id":"ol3",
			//	"id":"ol4",
			//	"id":"ol5",
			gOutLineLevel	=	dataPointString[8] & 0x0f;
		}
	}
	if (pointIsValid)
	{
	#ifdef _INCLUDE_MILKYWAY_MAIN_
//		printf("%-32s\t%f\t%f\r\n",	dataPointString, raValue_deg, decValue_deg);
	#endif // _INCLUDE_MILKYWAY_MAIN_

		dataPoint->ra_rad	=	RADIANS(raValue_deg);	//*	do NOT divide by 15
		dataPoint->decl_rad	=	RADIANS(decValue_deg);
		dataPoint->flag		=	(newPolygon ? 0 : 1);
		dataPoint->level	=	gOutLineLevel;
	}
	return(pointIsValid);
}


//************************************************************************
TYPE_OutlineData	*Milkyway_ReadOutlines(const char *filePath, long *milkyWayPtCnt)
{
FILE				*filePointer;
char				lineBuff[512];
char				dataPointString[128];
int					bytesRead;
int					iii;
int					ccc;
long				myDataPtCnt;
bool				keepGoing;
bool				pointIsValid;
char				theChar;
char				prevChar;
TYPE_OutlineData	*milkyWayOutlines;
TYPE_OutlineData	outlinePoint;
#ifdef _ENABLE_SKYTRAVEL_
	int				startupWidgetIdx;

	startupWidgetIdx	=	SetStartupText("MilkyWay outlines:");
#endif // _ENABLE_SKYTRAVEL_


//	CONSOLE_DEBUG(__FUNCTION__);

 	gOutLineLevel			=	0;
 	gPolyGonPointCount		=	0;
 	gPolyGonPointCountMax	=	0;

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		milkyWayOutlines	=	(TYPE_OutlineData *)calloc(kMilkyWayMaxPoints, sizeof(TYPE_OutlineData));
		myDataPtCnt	=	0;
		keepGoing	=	true;
		prevChar	=	0;
		ccc			=	0;
		while (keepGoing)
		{
			bytesRead	=	fread(lineBuff, 1, 500, filePointer);
			if (bytesRead > 0)
			{
				for (iii = 0; iii < bytesRead; iii++)
				{
					theChar					=	lineBuff[iii];
					dataPointString[ccc]	=	theChar;
					ccc++;
//					printf("%c", theChar);
					if (theChar == ',')
					{
						if ((prevChar == ']') || (prevChar == '}') || (prevChar == '"'))
						{
							dataPointString[ccc]	=	0;
//							printf("%s\r\n", dataPointString);

							pointIsValid	=	MilkyWay_ParsePointData(dataPointString, &outlinePoint);
							if (pointIsValid)
							{
								if ((milkyWayOutlines != NULL) && (myDataPtCnt < kMilkyWayMaxPoints))
								{
									milkyWayOutlines[myDataPtCnt]	=	outlinePoint;
									myDataPtCnt++;
								}
								else
								{
									CONSOLE_DEBUG("Not enough room in milkyway array");
								}
							}
							else
							{
							#ifdef _INCLUDE_MILKYWAY_MAIN_
								printf("%s\r\n", dataPointString);
							#endif // _INCLUDE_MILKYWAY_MAIN_

							}
							ccc	=	0;
						}
					}
					prevChar	=	theChar;
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		fclose(filePointer);
		*milkyWayPtCnt	=	myDataPtCnt;

	#ifdef _ENABLE_SKYTRAVEL_
		SetStartupTextStatus(startupWidgetIdx, "OK");
	#endif // _ENABLE_SKYTRAVEL_
	}
	else
	{
//		CONSOLE_DEBUG_W_STR("File not found\t=", filePath);
	#ifdef _ENABLE_SKYTRAVEL_
		SetStartupTextStatus(startupWidgetIdx, "Not found");
	#endif // _ENABLE_SKYTRAVEL_
	}
	return(milkyWayOutlines);
}

#ifdef _INCLUDE_MILKYWAY_MAIN_
//*****************************************************************************
int main(int argc, char *argv[])
{
long 	milkyWayPtCnt;

	printf("Reading MilkyWay data\r\n");
	milkyWayPtCnt	=	0;
	Milkyway_ReadOutlines("d3-celestial/data/milkyway.json", &milkyWayPtCnt);
	printf("MilkyWay has %ld data points\r\n", milkyWayPtCnt);

	CONSOLE_DEBUG_W_NUM("gPolyGonPointCountMax\t=",	gPolyGonPointCountMax);

}
#endif // _INCLUDE_MILKYWAY_MAIN_

