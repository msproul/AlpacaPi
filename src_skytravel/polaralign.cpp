//*****************************************************************************
//*		polaralign.cpp		(c) 2021 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Aug 24,	2021	<MLS> Created polaralign.cpp
//*	Aug 26,	2021	<MLS> Found 3pt equations http://ambrsoft.com/TrigoCalc/Circle3D.htm
//*	Aug 26,	2021	<MLS> Added ComputeCenterFrom3Pts()
//*	AUg 27,	2021	<MLS> Center of circle from PolarAlignment working
//*****************************************************************************

#include	<math.h>
#include	<string.h>
#include	<stdio.h>

//*	MLS Libraries
//#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"SkyStruc.h"
//#include	"VectorMath.h"

#include	"polaralign.h"

//#define	TEST_DETERMINATE

#ifdef TEST_DETERMINATE
	#include	"VectorMath.c"
#endif // TEST_DETERMINATE

//**************************************************************************************
typedef	struct
	{
		char	longName[kLongNameMax];
		double	xCoord;
		double	yCoord;
	} TYPE_Cartisian;



//*****************************************************************************
//*	http://ambrsoft.com/TrigoCalc/Circle3D.htm
//*****************************************************************************
static TYPE_Cartisian	ComputeCenterFrom3Pts(	TYPE_Cartisian	*pt1,
												TYPE_Cartisian	*pt2,
												TYPE_Cartisian	*pt3)
{
double	AAA;
double	BBB;
double	CCC;
double	DDD;
double	x1Sqrd;
double	y1Sqrd;
double	x2Sqrd;
double	y2Sqrd;
double	x3Sqrd;
double	y3Sqrd;
double	x1, y1;
double	x2, y2;
double	x3, y3;

TYPE_Cartisian	returnVal;
#ifdef TEST_DETERMINATE
	Matrix			testMatrix;
	double			testDeterminate;
#endif // TEST_DETERMINATE

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Pt 1=", pt1->longName);
//	CONSOLE_DEBUG_W_STR("Pt 2=", pt2->longName);
//	CONSOLE_DEBUG_W_STR("Pt 3=", pt3->longName);

	x1		=	pt1->xCoord;
	x2		=	pt2->xCoord;
	x3		=	pt3->xCoord;

	y1		=	pt1->yCoord;
	y2		=	pt2->yCoord;
	y3		=	pt3->yCoord;

#ifdef TEST_DETERMINATE
	testMatrix.colCnt			=	3;
	testMatrix.rowCnt			=	3;
	testMatrix.row[0].col[0]	=	x1;
	testMatrix.row[0].col[1]	=	y1;
	testMatrix.row[0].col[2]	=	1;

	testMatrix.row[1].col[0]	=	x2;
	testMatrix.row[1].col[1]	=	y2;
	testMatrix.row[1].col[2]	=	1;

	testMatrix.row[2].col[0]	=	x3;
	testMatrix.row[2].col[1]	=	y3;
	testMatrix.row[2].col[2]	=	1;

	testDeterminate	=   Matrix_Determinant(&testMatrix);
	CONSOLE_DEBUG_W_DBL("testDeterminate-A\t=", testDeterminate);
#endif // TEST_DETERMINATE

	x1Sqrd	=	x1  * x1;
	y1Sqrd	=	y1  * y1;
	x2Sqrd	=	x2  * x2;
	y2Sqrd	=	y2  * y2;
	x3Sqrd	=	x3  * x3;
	y3Sqrd	=	y3  * y3;

	AAA		=	x1 * (y2 - y3);
	AAA		-=	y1 * (x2 - x3);
	AAA		+=	(x2 * y3) - (x3 * y2);
//	CONSOLE_DEBUG_W_DBL("AAA\t\t=", AAA);


	BBB		=	(x1Sqrd + y1Sqrd) * (y3 - y2);
	BBB		+=	(x2Sqrd + y2Sqrd) * (y1 - y3);
	BBB		+=	(x3Sqrd + y3Sqrd) * (y2 - y1);
//	CONSOLE_DEBUG_W_DBL("BBB\t\t=", BBB);

#ifdef TEST_DETERMINATE
	testMatrix.row[0].col[0]	=	x1Sqrd + y1Sqrd;
	testMatrix.row[1].col[0]	=	x2Sqrd + y2Sqrd;
	testMatrix.row[2].col[0]	=	x3Sqrd + y3Sqrd;
	testDeterminate	=   -Matrix_Determinant(&testMatrix);
	CONSOLE_DEBUG_W_DBL("testDeterminate-B\t=", testDeterminate);
	if (BBB != testDeterminate)
	{
	//	CONSOLE_ABORT(__FUNCTION__);
	}
#endif // TEST_DETERMINATE



	CCC		=	(x1Sqrd + y1Sqrd) * (x2 - x3);
	CCC		+=	(x2Sqrd + y2Sqrd) * (x3 - x1);
	CCC		+=	(x3Sqrd + y3Sqrd) * (x1 - x2);

	DDD		=	(x1Sqrd + y1Sqrd) * ((x3 * y2) - (x2 * y3));
	DDD		+=	(x2Sqrd + y2Sqrd) * ((x1 * y3) - (x3 * y1));
	DDD		+=	(x3Sqrd + y3Sqrd) * ((x2 * y1) - (x1 * y2));


	returnVal.xCoord	=	-BBB / (2.0 * AAA);
	returnVal.yCoord	=	-CCC / (2.0 * AAA);
//	radius				=	sqrt((BBB * BBB) + (CCC * CCC) - (4.0 * AAA * DDD));

	return(returnVal);
}


//*****************************************************************************
//*	This will take a list of stars, look at ONLY those with matching first char
//*	and compute the center of that list of stars.
//*	This is INTENDED to be use for polar alignment
//*****************************************************************************
int		ComputeCenterFromStarList(	TYPE_CelestData	*starList,
									int				totalStars,
									char			fistCharOfName,
									TYPE_CelestData	*centerPointArray,
									int				maxCenterPtCnt)
{
int				iii;
int				jjj;
int				qqq;
int				myStarCount;
double			polar_Radius;
TYPE_Cartisian	*cartesianCoordArray;
int				pointDelta;
int				firstPoint;
TYPE_Cartisian	centerPtCart;
TYPE_CelestData	myCenterPoint;
double			centerRadius;
double			centerRA;
double			centerDEC;
int				idx1, idx2, idx3;
bool			seriesOK;
int				seriesIdx;
char			centerPtName[16];
double			xxx;
double			yyy;
int				newCenterPtCnt;

	xxx				=	0.0;
	yyy				=	0.0;
	newCenterPtCnt	=	0;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("fistCharOfName\t=", fistCharOfName);

	myStarCount	=	0;
	seriesIdx	=	1;
	//*	step through the star list
	for (iii=0; iii<totalStars; iii++)
	{
		if (starList[iii].longName[0] == fistCharOfName)
		{
			myStarCount++;
		}
	}

	if (myStarCount >= 3)
	{
		cartesianCoordArray	=	(TYPE_Cartisian *)malloc(myStarCount * sizeof(TYPE_Cartisian));
		if (cartesianCoordArray != NULL)
		{
			jjj	=	0;
			for (iii=0; iii<totalStars; iii++)
			{
				if (starList[iii].longName[0] == fistCharOfName)
				{
					//*	convert to polar coordinates
					polar_Radius					=	(M_PI / 2.0) - starList[iii].org_decl;
					cartesianCoordArray[jjj].xCoord	=	polar_Radius * cos(starList[iii].org_ra);
					cartesianCoordArray[jjj].yCoord	=	polar_Radius * sin(starList[iii].org_ra);
					strcpy(cartesianCoordArray[jjj].longName, starList[iii].longName);


					jjj++;
				}
			}

			//*	now we have the Cartesian coords,
			//*	pick out groups of 3 to find the radial center
			if (jjj != myStarCount)
			{
				//*	something is wrong
				CONSOLE_ABORT("something is wrong");
			}
			firstPoint	=	0;
			pointDelta	=	myStarCount / 2;
			if ((myStarCount % 2) == 0)
			{
				pointDelta--;
			}
//			CONSOLE_DEBUG_W_NUM("myStarCount\t=", myStarCount);
//			CONSOLE_DEBUG_W_NUM("pointDelta\t=", pointDelta);
			while (pointDelta >= 2)
			{
				//*	figure out which 3 points to use
				idx1	=	firstPoint;
				idx2	=	firstPoint + pointDelta;
				idx3	=	firstPoint + (2 * pointDelta);
//				CONSOLE_DEBUG_W_NUM("idx1\t=", idx1);
//				CONSOLE_DEBUG_W_NUM("idx2\t=", idx2);
//				CONSOLE_DEBUG_W_NUM("idx3\t=", idx3);

				//*	make sure they are all in bounds
				seriesOK	=	true;
				if ((idx1 >= myStarCount) || (idx2 >= myStarCount) || (idx3 >= myStarCount))
				{
					//*	out of bounds, dont process
					seriesOK	=	false;
				}
				if ((idx1 >= idx2) || (idx2 >= idx3))
				{
					//*	the points are not in order, dont process
					seriesOK	=	false;
				}
				if (seriesOK)
				{
					centerPtCart	=	ComputeCenterFrom3Pts(	&cartesianCoordArray[idx1],
																&cartesianCoordArray[idx2],
																&cartesianCoordArray[idx3]);
					//*	this is for the average point
					xxx		+=	centerPtCart.xCoord;
					yyy		+=	centerPtCart.yCoord;

					//*	now we have to turn this back into polar coordinates
					centerRadius	=	sqrt((centerPtCart.xCoord * centerPtCart.xCoord) +
											(centerPtCart.yCoord * centerPtCart.yCoord));
					centerDEC		=	(M_PI / 2) - centerRadius;
					centerRA		=	atan2(centerPtCart.yCoord, centerPtCart.xCoord);

					memset(&myCenterPoint, 0, sizeof(TYPE_CelestData));
					sprintf(centerPtName, "%c%d", fistCharOfName, seriesIdx);
					myCenterPoint.org_decl			=	centerDEC;
					myCenterPoint.org_ra			=	centerRA;
					myCenterPoint.decl				=	centerDEC;
					myCenterPoint.ra				=	centerRA;
					myCenterPoint.dataSrc			=	kDataSrc_PolarAlignCenter;
					strcpy(myCenterPoint.shortName,	centerPtName);
					strcpy(myCenterPoint.longName,	centerPtName);
					myCenterPoint.realMagnitude		=	1;
					myCenterPoint.magn				=	ST_STAR;

//					CONSOLE_DEBUG_W_STR("longName\t=", myCenterPoint.longName);

					seriesIdx++;

					//*	now find a place in the array to put the new entry
					qqq	=	0;
					while (qqq < maxCenterPtCnt)
					{
						if (centerPointArray[qqq].dataSrc == 0)
						{
							centerPointArray[qqq]	=	myCenterPoint;
							newCenterPtCnt++;
							break;
						}
						qqq++;
					}
				}
				else
				{
					CONSOLE_DEBUG("3 Point series not valid");
					CONSOLE_DEBUG_W_NUM("idx1\t=", idx1);
					CONSOLE_DEBUG_W_NUM("idx2\t=", idx2);
					CONSOLE_DEBUG_W_NUM("idx3\t=", idx3);
					CONSOLE_ABORT(__FUNCTION__);
				}
				//*	decrease the point delta, however we do not want to do points right next to each other
				pointDelta--;
				if ((pointDelta < 2) && (firstPoint == 0))
				{
//					CONSOLE_DEBUG("set up for the 2nd half");
					//*	set up for the 2nd half
					firstPoint	=	myStarCount / 2;
					pointDelta	=	myStarCount / 4;
					if ((myStarCount % 2) == 0)
					{
						pointDelta--;
					}
				}
			}


			//---------------------------------------------------
			//*	we are done with the list, determine the average
			xxx	=	xxx / newCenterPtCnt;
			yyy	=	yyy / newCenterPtCnt;

			//*	now we have to turn this back into polar coordinates
			centerRadius	=	sqrt((xxx * xxx) +
									(yyy * yyy));
			centerDEC		=	(M_PI / 2) - centerRadius;
			centerRA		=	atan2(yyy, xxx);

			memset(&myCenterPoint, 0, sizeof(TYPE_CelestData));
			sprintf(centerPtName, "%c-center", fistCharOfName);
			myCenterPoint.org_decl			=	centerDEC;
			myCenterPoint.org_ra			=	centerRA;
			myCenterPoint.decl				=	centerDEC;
			myCenterPoint.ra				=	centerRA;
			myCenterPoint.dataSrc			=	kDataSrc_PolarAlignCenter;
			strcpy(myCenterPoint.shortName,	centerPtName);
			strcpy(myCenterPoint.longName,	centerPtName);
			myCenterPoint.realMagnitude		=	1;
			myCenterPoint.magn				=	ST_STAR;

			//*	now find a place in the array to put the new entry
			qqq	=	0;
			while (qqq < maxCenterPtCnt)
			{
				if (centerPointArray[qqq].dataSrc == 0)
				{
					centerPointArray[qqq]	=	myCenterPoint;
					break;
				}
				qqq++;
			}

			free(cartesianCoordArray);
		}
	}
	return(myStarCount);
}
