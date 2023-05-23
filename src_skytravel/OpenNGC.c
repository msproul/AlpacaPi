//*****************************************************************************
//*		OpenNGC.c		(c) 2022 by Mark Sproul
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
//*	Feb  3,	2022	<MLS> Created OpenNGC.c
//*	Feb  3,	2022	<MLS> OpenNGC star data working
//*	Feb  4,	2022	<MLS> Added Read_OpenNGC_Outline_catgen()
//*	Jun 17,	2022	<MLS> Added error checking to ParseOneLine_OpenNGC()
//*****************************************************************************
//*	git clone https://github.com/mattiaverga/OpenNGC
//*****************************************************************************


#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>
#include	<pthread.h>
#include	<unistd.h>
#include	<stdbool.h>

#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"OpenNGC.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#ifdef _INCLUDE_OPENNGC_MAIN_
	char	gNGCDatbase[32];
#endif // _INCLUDE_OPENNGC_MAIN_

//Name	Type	RA	Dec	Const	MajAx	MinAx	PosAng	B-Mag	V-Mag	J-Mag	H-Mag	K-Mag	SurfBr	Hubble	Pax	Pm-RA	Pm-Dec	RadVel	Redshift	Cstar U-Mag	Cstar B-Mag	Cstar V-Mag	M	NGC	IC	Cstar Names	Identifiers	Common names	NED notes	OpenNGC notes
//IC0001	**	00:08:27.05	+27:43:03.6	Peg
//IC0002	G	00:11:00.88	-12:49:22.3	Cet	0.98	0.32	142	15.46		12.26	11.48	11.17	23.45	Sb				6775	0.02286								2MASX J00110081-1249206	IRAS 00084-1306	MCG -02-01-031	PGC 000778

//************************************************************************
enum
{
	kOpenNGC_Name	=	0,
	kOpenNGC_Type,
	kOpenNGC_RA,
	kOpenNGC_Dec,
	kOpenNGC_Const,
	kOpenNGC_MajAx,
	kOpenNGC_MinAx,
	kOpenNGC_PosAng,
	kOpenNGC_B_Mag,
	kOpenNGC_V_Mag,
	kOpenNGC_J_Mag,
	kOpenNGC_H_Mag,
	kOpenNGC_K_Mag,
	kOpenNGC_SurfBr,
	kOpenNGC_Hubble,
	kOpenNGC_Pax,
	kOpenNGC_Pm_RA,
	kOpenNGC_Pm_Dec,
	kOpenNGC_RadVel,
	kOpenNGC_Redshift,
	kOpenNGC_Cstar_U_Mag,
	kOpenNGC_Cstar_B_Mag,
	kOpenNGC_Cstar_V_Mag,
	kOpenNGC_M,
	kOpenNGC_NGC,
	kOpenNGC_IC,
	kOpenNGC_Cstar_Names,
	kOpenNGC_Identifiers,
	kOpenNGC_Common_names,
	kOpenNGC_NED_notes,
	kOpenNGC_OpenNGC_notes

};


#define		CHECK_MAG(name, x)				\
		if (x < 90.0)						\
		{									\
			CONSOLE_DEBUG_W_DBL(name, x);	\
		}

//************************************************************************
static bool	ParseOneLine_OpenNGC(char *lineBuff, TYPE_CelestData *starRec)
{
bool	validData;
int		iii;
int		ccc;
int		sLen;
int		argNum;
char	theChar;
char	argString[64];
double	magnitude;
double	b_mag	=	99.0;
double	v_mag	=	99.0;
double	j_mag	=	99.0;
double	h_mag	=	99.0;
//double	k_mag	=	99.0;
double	parallax;
double	rightAscen_Deg;
char	decl_Sign;
double	declination_Deg;
char	designation[64];
double	hours;
double	degrees;
double	minutes;
double	seconds;
char	*idPtr;

//	CONSOLE_DEBUG("-------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(lineBuff);

	validData		=	false;
	sLen			=	strlen(lineBuff);
	argNum			=	0;
	ccc				=	0;
	magnitude		=	15;
	parallax		=	0;
	rightAscen_Deg	=	0.0;
	declination_Deg	=	0.0;
	strcpy(designation, "");


	for (iii=0; iii <= sLen; iii++)
	{
		theChar	=	lineBuff[iii];
		if ((theChar == ';') || (theChar == 0))
		{
			//*	end of argument, do something with it
			//IC0001	**	00:08:27.05	+27:43:03.6	Peg

//			CONSOLE_DEBUG_W_STR("argString:", argString);
			switch(argNum)
			{
				case kOpenNGC_Name:
					strcpy(designation, argString);
					break;

				case kOpenNGC_Type:
					strcat(designation, "-");
					strcat(designation, argString);
					break;

				case kOpenNGC_RA:
					if (strlen(argString) > 0)
					{
						hours			=	atoi(argString);
						minutes			=	atoi(&argString[3]);
						seconds			=	AsciiToDouble(&argString[6]);
						rightAscen_Deg	=	hours;
						rightAscen_Deg	+=	minutes / 60.0;
						rightAscen_Deg	+=	seconds / 3600.0;
						rightAscen_Deg	=	rightAscen_Deg * 15.0;
					}
//					else
//					{
//						CONSOLE_DEBUG(lineBuff);
//					}
					break;

				case kOpenNGC_Dec:
					if (strlen(argString) > 0)
					{
						decl_Sign		=	argString[0];
						degrees			=	atoi(&argString[1]);
						minutes			=	atoi(&argString[4]);
						seconds			=	AsciiToDouble(&argString[7]);
						declination_Deg	=	degrees;
						declination_Deg	+=	minutes / 60.0;
						declination_Deg	+=	seconds / 3600.0;
						if (decl_Sign == '-')
						{
							declination_Deg	=	-declination_Deg;
						}
					}
//					else
//					{
//						CONSOLE_DEBUG(lineBuff);
//					}
					break;

//				case kOpenNGC_Const:
//				case kOpenNGC_MajAx:
//				case kOpenNGC_MinAx:
//				case kOpenNGC_PosAng:

				case kOpenNGC_B_Mag:
					if (strlen(argString) > 0)
					{
						b_mag	=	AsciiToDouble(argString);
					}
					break;

				case kOpenNGC_V_Mag:
					if (strlen(argString) > 0)
					{
						v_mag	=	AsciiToDouble(argString);
					}
					break;

				case kOpenNGC_J_Mag:
					if (strlen(argString) > 0)
					{
						j_mag	=	AsciiToDouble(argString);
					}
					break;

				case kOpenNGC_H_Mag:
					if (strlen(argString) > 0)
					{
						h_mag	=	AsciiToDouble(argString);
					}
					break;

//				case kOpenNGC_K_Mag:
//					if (strlen(argString) > 0)
//					{
//						k_mag	=	AsciiToDouble(argString);
//					}
//					break;


//				case kOpenNGC_SurfBr:
//				case kOpenNGC_Hubble:
				case kOpenNGC_Pax:
					if (strlen(argString) > 0)
					{
						parallax	=	AsciiToDouble(argString);
					}
					break;

//				case kOpenNGC_Pm_RA:
//				case kOpenNGC_Pm_Dec:
//				case kOpenNGC_RadVel:
//				case kOpenNGC_Redshift:
//				case kOpenNGC_Cstar_U_Mag:
//				case kOpenNGC_Cstar_B_Mag:
//				case kOpenNGC_Cstar_V_Mag:
//				case kOpenNGC_M:
//				case kOpenNGC_NGC:
//				case kOpenNGC_IC:
//				case kOpenNGC_Cstar_Names:
//				case kOpenNGC_Identifiers:
//				case kOpenNGC_Common_names:
//				case kOpenNGC_NED_notes:
//				case kOpenNGC_OpenNGC_notes:
					break;


			}
			argNum++;
			ccc				=	0;
			argString[0]	=	0;
		}
		else
		{
			if (ccc < 60)
			{
				argString[ccc++]	=	theChar;
				argString[ccc]		=	0;
			}
		}
	}
	if (argNum >= kOpenNGC_K_Mag)
	{
		//*	figure out which magnitude to use
		if (v_mag < 50.0)
		{
			magnitude	=	v_mag;
		}
		else if ((b_mag < 50.0) && (j_mag < 50.0))
		{
			//*	take the average of these 2
			magnitude	=	(b_mag + j_mag) / 2.0;
		}
		else if (b_mag < 50.0)
		{
			magnitude	=	b_mag;
		}
		else if (h_mag < 50.0)
		{
			magnitude	=	h_mag;
		}
		else
		{
//			CHECK_MAG("b_mag:", b_mag);
//			CHECK_MAG("v_mag:", v_mag);
//			CHECK_MAG("j_mag:", j_mag);
//			CHECK_MAG("h_mag:", h_mag);
//			CHECK_MAG("k_mag:", k_mag);
//			CONSOLE_DEBUG(lineBuff);
			magnitude	=	15.0;
//			CONSOLE_ABORT(__FUNCTION__)
		}
		strcpy(starRec->longName, designation);
		idPtr	=	designation;
		while (!isdigit(*idPtr))
		{
			idPtr++;
		}
		starRec->id				=	atoi(idPtr);
		starRec->dataSrc		=	kDataSrc_OpenNGC;
		starRec->decl			=	RADIANS(declination_Deg);
		starRec->org_decl		=	RADIANS(declination_Deg);
		starRec->org_ra			=	RADIANS(rightAscen_Deg);
		starRec->ra				=	RADIANS(rightAscen_Deg);
		starRec->realMagnitude	=	magnitude;
		starRec->parallax		=	parallax;

		validData	=	true;
	}
	else
	{
		validData	=	false;
	}

	return(validData);
}


//************************************************************************
TYPE_CelestData	*Read_OpenNGC_StarCatalog(long *starCount)
{
FILE			*filePointer;
long			ngcLineCount;
long			recordCount;
TYPE_CelestData	*ngcStarData;
TYPE_CelestData	starObject;
bool			validObject;
char			lineBuff[512];
char			filePath[128];
char			*firstLinePtr;
int				startupWidgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	startupWidgetIdx	=	SetStartupText("Open-NGC catalog:");

	ngcStarData	=	NULL;

	strcpy(filePath, "OpenNGC/NGC.csv");

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		ngcLineCount	=	13969;

//		bufferSize	=	(ngcLineCount + 2) * sizeof(TYPE_CelestData);
		ngcStarData	=	(TYPE_CelestData *)calloc((ngcLineCount + 2), sizeof(TYPE_CelestData));
		if (ngcStarData != NULL)
		{
			//*	read the first line to get rid of it
			firstLinePtr	=	fgets(lineBuff, 500, filePointer);
			if (firstLinePtr == NULL)
			{
				CONSOLE_DEBUG("Error reading file");
			}
			recordCount	=	0;
			while (fgets(lineBuff, 500, filePointer) && (recordCount < ngcLineCount))
			{
				validObject	=	ParseOneLine_OpenNGC(lineBuff, &starObject);
				if (validObject)
				{
					ngcStarData[recordCount]	=	starObject;
					recordCount++;
				}
			}

			*starCount	=	recordCount;

			//*	make note that we are using OpenNGC
			strcpy(gNGCDatbase, "OpenNGC");
		}
		fclose(filePointer);
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("File not found\t=", filePath);
		SetStartupTextStatus(startupWidgetIdx, "Not found");
	}
	return(ngcStarData);
}

//97.92134  9.93024   2
//97.92182  9.90016   2
//97.90899  9.86813   2
//97.89076  9.85067   2
//97.88731  9.84339   2
//97.88829  9.82640   2
//97.89813  9.80262   2
//97.89368  9.77642   2
//97.86757  9.73615   2
//97.83507  9.72548   1
//85.23945  -2.25265  0 3 IC0434_lv3
//85.23542  -2.23353  2
//85.22636  -2.22246  2
//85.22535  -2.21541  2

//************************************************************************
static bool	ParseOneLine_OpenNGC_Outlines(char *lineBuff, TYPE_OpenNGCoutline *outlinePt)
{
double				ra_degrees;
double				dec_degrees;
int					flag;


	ra_degrees	=	AsciiToDouble(lineBuff);
	dec_degrees	=	AsciiToDouble(&lineBuff[10]);
	flag		=	atoi(&lineBuff[20]);

	outlinePt->ra_rad	=	RADIANS(ra_degrees);
	outlinePt->decl_rad	=	RADIANS(dec_degrees);
	outlinePt->flag		=	flag;
	return(true);
}

//************************************************************************
TYPE_OpenNGCoutline	*Read_OpenNGC_Outline_catgen(long *pointCount)
{
FILE				*filePointer;
TYPE_OpenNGCoutline	*outlineArray;
TYPE_OpenNGCoutline	outlinePoint;
long				linesInFile;
long				recordCount;
char				lineBuff[512];
char				filePath[128];
bool				validObject;

//	CONSOLE_DEBUG(__FUNCTION__);

	outlineArray	=	NULL;

	strcpy(filePath, "OpenNGC/outlines/outlines_catgen.dat");

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		linesInFile		=	CountLinesInFile(filePointer);
		outlineArray	=	(TYPE_OpenNGCoutline *)calloc((linesInFile + 2), sizeof(TYPE_OpenNGCoutline));
		if (outlineArray != NULL)
		{
			recordCount	=	0;
			while (fgets(lineBuff, 500, filePointer) && (recordCount < linesInFile))
			{
				validObject	=	ParseOneLine_OpenNGC_Outlines(lineBuff, &outlinePoint);
				if (validObject)
				{
					outlineArray[recordCount]	=	outlinePoint;
					recordCount++;
				}
			}
			*pointCount	=	recordCount;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("File not found\t=", filePath);
	}
	return(outlineArray);
}


#ifdef _INCLUDE_OPENNGC_MAIN_


//*****************************************************************************
int main(int argc, char *argv[])
{
TYPE_CelestData		*ngcData;
long				starCount;
TYPE_OpenNGCoutline	*outlineArray;
long				outlinePtCount;

	CONSOLE_DEBUG(__FUNCTION__);

	ngcData	=	Read_OpenNGC_StarCatalog(&starCount);
	CONSOLE_DEBUG_W_LONG("starCount\t=", starCount);


	outlineArray	=	Read_OpenNGC_Outline_catgen(&outlinePtCount);
	CONSOLE_DEBUG_W_LONG("outlinePtCount\t=", outlinePtCount);

}
#endif // _INCLUDE_OPENNGC_MAIN_
