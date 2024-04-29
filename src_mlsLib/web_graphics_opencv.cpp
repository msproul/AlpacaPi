//**************************************************************************************
//*	Web Graphics library (opencv version)
//*
//*		web_graphics_opencv.cpp
//*
//*		(C) 2024 by Mark Sproul
//*
//*
//**************************************************************************************
//*	Edit History
//**************************************************************************************
//*	Apr 26,	2024	<MLS> Created web_graphics_opencv.cpp
//*	Apr 26,	2024	<MLS> Started on opencv version of web_graphics librar
//**************************************************************************************

#include	<opencv2/opencv.hpp>
#include	<opencv2/core.hpp>

#include	"web_graphics_opencv.h"

#define	_ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define	kLegendLineHeight	25
#define kXdotLen			4
#define kXcircleDiam		7
#define kXcircleDiamLarge	15

cv::Scalar	_gdColorBlack	=	CV_RGB(0,		0,		0);
cv::Scalar	_gdColorGrey	=	CV_RGB(128,		128,	128);
cv::Scalar	_gdColorLtGrey	=	CV_RGB(200,		200,	200);
cv::Scalar	_gdColorWhite	=	CV_RGB(255,		255,	255);
cv::Scalar	_gdColorRed		=	CV_RGB(255,		0,		0);
cv::Scalar	_gdColorPink	=	CV_RGB(255,		0,		230);
cv::Scalar	_gdColorGreen	=	CV_RGB(88,		255,	88);
cv::Scalar	_gdColorDkGreen	=	CV_RGB(0,		196,	0);
cv::Scalar	_gdColorBlue	=	CV_RGB(0,		0,		255);
cv::Scalar	_gdColorCyan	=	CV_RGB(0,		255,	255);
cv::Scalar	_gdColorMagenta	=	CV_RGB(255,		0,		255);
cv::Scalar	_gdColorYellow	=	CV_RGB(255,		255,	0);
cv::Scalar	_gdColorOrange	=	CV_RGB(255,		128,	0);
cv::Scalar	_gdColorCopper	=	CV_RGB(0xC9,	0x63,	0x33);


int			_xCenter;
int			_yCenter;

static int			_currentXsize;
static int			_currentYsize;
static double		_cosZangle;
static double		_sinZangle;

static cv::Mat		*gWebGrapicsImage	=	NULL;
static int			gCurrentLineWidth	=	1;
static cv::Scalar	gCurrentColor		=	CV_RGB(255,	0,	0);

#define	kMaxColors	100
static cv::Scalar	gWebGraphicsColorTable[kMaxColors];
static bool			gWebGrapicsInitColorTable	=	true;
//*****************************************************************************
static void	WebGraph_InitColorTable(void)
{
int	iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<kMaxColors; iii++)
	{
		gWebGraphicsColorTable[iii]	=	CV_RGB(0,	0,	0);
	}

	gWebGraphicsColorTable[kGraphPointMode_RedLine]				=	CV_RGB(255,	0,		0);
	gWebGraphicsColorTable[kGraphPointMode_GreenLine]			=	CV_RGB(0,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlueLine]			=	CV_RGB(0,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_OrangeLine]			=	CV_RGB(255,	128,	0);
	gWebGraphicsColorTable[kGraphPointMode_CyanLine]			=	CV_RGB(0,	255,	255);
	gWebGraphicsColorTable[kGraphPointMode_MagentaLine]			=	CV_RGB(255,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_YellowLine]			=	CV_RGB(255,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlackLine]			=	CV_RGB(0,	0,		0);

	gWebGraphicsColorTable[kGraphPointMode_RedLineDashed]		=	CV_RGB(255,	0,		0);
	gWebGraphicsColorTable[kGraphPointMode_GreenLineDashed]		=	CV_RGB(0,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlueLineDashed]		=	CV_RGB(0,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_OrangeLineDashed]	=	CV_RGB(255,	128,	0);
	gWebGraphicsColorTable[kGraphPointMode_CyanLineDashed]		=	CV_RGB(0,	255,	255);
	gWebGraphicsColorTable[kGraphPointMode_MagentaLineDashed]	=	CV_RGB(255,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_YellowLineDashed]	=	CV_RGB(255,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlackLineDashed]		=	CV_RGB(0,	0,		0);

	gWebGraphicsColorTable[kGraphPointMode_RedDot]				=	CV_RGB(255,	0,		0);
	gWebGraphicsColorTable[kGraphPointMode_GreenDot]			=	CV_RGB(0,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlueDot]				=	CV_RGB(0,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_OrangeDot]			=	CV_RGB(255,	128,	0);
	gWebGraphicsColorTable[kGraphPointMode_CyanDot]				=	CV_RGB(0,	255,	255);
	gWebGraphicsColorTable[kGraphPointMode_MagentaDot]			=	CV_RGB(255,	0,		255);
	gWebGraphicsColorTable[kGraphPointMode_YellowDot]			=	CV_RGB(255,	255,	0);
	gWebGraphicsColorTable[kGraphPointMode_BlackDot]			=	CV_RGB(0,	0,		0);

	gWebGrapicsInitColorTable	=	false;
}


//*****************************************************************************
void	WebGraph_CreateImage(int xSize, int ySize)
{
double	myZangle;

//	CONSOLE_DEBUG(__FUNCTION__);


	if (gWebGrapicsInitColorTable)
	{
		WebGraph_InitColorTable();
	}

	_currentXsize	=	xSize;
	_currentYsize	=	ySize;
	_xCenter		=	_currentXsize / 2;
	_yCenter		=	_currentYsize / 2;

	myZangle	=	135.0 * (M_PI / 180.0);
	_cosZangle	=	cos(myZangle);
	_sinZangle	=	sin(myZangle);

	gWebGrapicsImage	=	new cv::Mat(cv::Size(	xSize,
													ySize),
													CV_8UC3);
	if (gWebGrapicsImage != NULL)
	{
	cv::Rect	myCVrect;

		myCVrect.x		=	0;
		myCVrect.y		=	0;
		myCVrect.width	=	xSize;
		myCVrect.height	=	ySize;

		cv::rectangle(	*gWebGrapicsImage,
						myCVrect,
						CV_RGB(255,	255,	255),
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED
					#endif
						);

	}
}

//*****************************************************************************
bool	WebGraph_CheckImage(const char *routineName, const int lineNum)
{

	return(false);
}

//*****************************************************************************
int		WebGraph_SaveImage(const char *pathName, const char *fileName)
{
int			openCVerr;
char		outputFileName[256];

	if (gWebGrapicsImage != NULL)
	{
		strcpy(outputFileName, pathName);
		strcat(outputFileName, "/");
		strcat(outputFileName, fileName);

		openCVerr	=	cv::imwrite(outputFileName, *gWebGrapicsImage);
		if (openCVerr == 0)
		{
			CONSOLE_DEBUG_W_NUM("cv::imwrite returned error\t=", openCVerr);
		}
		//---try------try------try------try------try------try---
		try
		{
		//	CONSOLE_DEBUG("try delete cOpenCV_matImage");
			delete gWebGrapicsImage;
			gWebGrapicsImage	=	NULL;
		}
		catch(cv::Exception& ex)
		{
			CONSOLE_DEBUG("delete gWebGrapicsImage; had an exception");
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}

	return(0);
}


//**************************************************************************************
int	WebGraph_TranslateXvalue(int xCordinate, int zCordinate)
{
int		screenX;
int		zXoffset;

	zXoffset	=	zCordinate * _cosZangle;
	screenX		=	_xCenter + xCordinate + zXoffset;
	return(screenX);
}

//**************************************************************************************
int	WebGraph_TranslateYvalue(int yCordinate, int zCordinate)
{
int		screenY;
int		zYoffset;

	zYoffset	=	zCordinate * _sinZangle;
	screenY		=	_yCenter - yCordinate + zYoffset;
	return(screenY);
}

//*****************************************************************************
void	WebGraph_DrawScaleX2(double scaleX, cv::Scalar color)
{
	CONSOLE_DEBUG("Not finished");
}

//*****************************************************************************
void	WebGraph_DrawAxis(int style, cv::Scalar color)
{
int	vectorLen;

#ifdef _GRAPH_DEBUG_AXIS_
	CONSOLE_DEBUG_W_NUM("style\t=", style);
#endif
	switch(style)
	{
		case kAxisStyle_3D:
			_xCenter		=	_currentXsize / 4;
			_yCenter		=	(_currentYsize / 2) + (_currentYsize / 10);
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 2),	0,						0,						color, "e1");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 2),	0,						color, "e2");
			WebGraph_DrawVector(0,	0,	0,		0,						0,						(_currentYsize / 5),	color, "e3");
			break;

		case kAxisStyle_XYZ_abs:
			_xCenter		=	_currentXsize / 4;
			_yCenter		=	(_currentYsize / 2) + (_currentYsize / 10);
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 2),	0,						0,						color, "X");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 2),	0,						color, "Y");
			WebGraph_DrawVector(0,	0,	0,		0,						0,						(_currentYsize / 5),	color, "Z");
			break;


		case kAxisStyle_3D_abs:
			_xCenter		=	_currentXsize / 4;
			_yCenter		=	(_currentYsize / 2) + (_currentYsize / 10);
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 2),	0,						0,						color, "S");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 2),	0,						color, "alpha");
			WebGraph_DrawVector(0,	0,	0,		0,						0,						(_currentYsize / 5),	color, "i-beta");
			break;

		case kAxisStyle_XY:
			_xCenter		=	75;
			_yCenter		=	_currentYsize - 50;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "X");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,						color, "Y");
			break;

		case kAxisStyle_XY_NoLabels:
			_xCenter		=	75;
			_yCenter		=	_currentYsize - 50;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,						color, "");
			break;



		case kAxisStyle_XY_640x800:
		#ifdef _GRAPH_DEBUG_AXIS_
			CONSOLE_DEBUG("kAxisStyle_XY_640x800");
		#endif
			_xCenter		=	50;
			_yCenter		=	_currentYsize - 150;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "X");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,						color, "Y");
			break;


		case kAxisStyle_XT:
			_xCenter		=	50;
			_yCenter		=	_currentYsize - 50;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "x");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,						color, "t");
			break;

		case kAxisStyle_UP:
			_xCenter		=	50;
			_yCenter		=	_currentYsize - 50;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "u");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,						color, "p (GPa)");
			break;

		case kAxisStyle_UP_centered:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize - 50;
			vectorLen		=	(_currentXsize / 20) * 9;
//			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,			color, "u");
			WebGraph_DrawVector(0,	0,	0,		vectorLen,	0,						0,						color, "u");
			WebGraph_DrawVector(0,	0,	0,		-vectorLen,	0,						0,						color, "-u");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 60),	0,			color, "p (GPa)");
			break;


		case kAxisStyle_UP_4Quads:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			vectorLen		=	(_currentXsize / 20) * 9;
			WebGraph_DrawVector(0,	0,	0,		vectorLen,	0,						0,						color, "u");
			WebGraph_DrawVector(0,	0,	0,		-vectorLen,	0,						0,						color, "-u");
			WebGraph_DrawVector(0,	0,	0,		0,						((_currentYsize / 2) - 40),	0,		color, "p");
			WebGraph_DrawVector(0,	0,	0,		0,						-((_currentYsize / 2) - 40),	0,	color, "-p");
			break;

		case kAxisStyle_XY_abs:
			_xCenter		=	50;
			_yCenter		=	_currentYsize - 30;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize - 65),	0,						0,						color, "S");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize - 40),	0,						color, "sqrt(alpha^2 + beta^2)");
			break;


		case kAxisStyle_XY_centered:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			vectorLen		=	(_currentXsize / 20) * 9;
			WebGraph_DrawVector(0,	0,	0,		vectorLen,	0,						0,						color, "");
			WebGraph_DrawVector(0,	0,	0,		-vectorLen,	0,						0,						color, "");
			WebGraph_DrawVector(0,	0,	0,		0,						vectorLen,	0,						color, "");
			WebGraph_DrawVector(0,	0,	0,		0,						-vectorLen,	0,						color, "");
			break;

		case kAxisStyle_XY_compass:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			vectorLen		=	(_currentXsize / 20) * 9;
			WebGraph_DrawVector(0,	0,	0,		vectorLen,	0,						0,						color, "E (90)");
			WebGraph_DrawVector(0,	0,	0,		-vectorLen,	0,						0,						color, "W (270)");
			WebGraph_DrawVector(0,	0,	0,		0,						vectorLen,	0,						color, "N (0)");
			WebGraph_DrawVector(0,	0,	0,		0,						-vectorLen,	0,						color, "S (180)");
			break;


		case kAxisStyle_PolarXY:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 3),	0,						0,			color, "x");
			WebGraph_DrawVector(0,	0,	0,		-(_currentXsize / 3),	0,						0,			color, "-x");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 3),	0,			color, "y");
			WebGraph_DrawVector(0,	0,	0,		0,						-(_currentYsize / 3),	0,			color, "-y");
			break;



		case kAxisStyle_e3e1:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 3),	0,						0,						color, "e3");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 3),	0,						color, "e1");
			WebGraph_Draw2DCircle(0, 0, 20, 20, color);
			break;

		case kAxisStyle_e1e2:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			WebGraph_DrawVector(0,	0,	0,		(_currentXsize / 3),	0,						0,						color, "e1");
			WebGraph_DrawVector(0,	0,	0,		0,						(_currentYsize / 3),	0,						color, "e2");
			WebGraph_Draw2DCircle(0, 0, 20, 20, color);
			break;

		case kAxisStyle_AlhpaBeta:
			_xCenter		=	_currentXsize / 2;
			_yCenter		=	_currentYsize / 2;
			vectorLen		=	(_currentXsize / 20) * 9;

			WebGraph_DrawVector(0,	0,	0,		vectorLen,	0,				0,			color, "i-beta");
			WebGraph_DrawVector(0,	0,	0,		-vectorLen,	0,				0,			color, "");
			WebGraph_DrawVector(0,	0,	0,		0,				vectorLen,	0,			color, "alpha");
			WebGraph_DrawVector(0,	0,	0,		0,				-vectorLen,	0,			color, "");
			break;


	}

#ifdef _GRAPH_DEBUG_AXIS_
	CONSOLE_DEBUG_W_NUM("_currentXsize\t=", _currentXsize);
	CONSOLE_DEBUG_W_NUM("_currentYsize\t=", _currentYsize);
	CONSOLE_DEBUG_W_NUM("_xCenter\t=", _xCenter);
	CONSOLE_DEBUG_W_NUM("_yCenter\t=", _yCenter);
#endif

//	WebGraph_DrawText("e1",	(_currentXsize / 3),	0,						0,		color);
//	WebGraph_DrawText("e2",	5,						(_currentXsize / 3),	0,		color);

}

//*****************************************************************************
void	WebGraph_DrawScaleTickMarks_X(double deltaPixelsX, double startValueX, double deltaValueX, cv::Scalar color)
{
double	screenX_DBL;
int		screenX, screenY;
double	scaleValue;
char	scaleText[32];
bool	useFractionFormat;
int		tickCount;

//	CONSOLE_DEBUG(__FUNCTION__);

	screenY				=	-5;
	screenX				=	0;
	scaleValue			=	startValueX;
	screenX_DBL			=	screenX;
	tickCount			=	0;
	if (deltaValueX < 1.0)
	{
		useFractionFormat	=	true;
	}
	else
	{
		useFractionFormat	=	false;
	}
	if (deltaPixelsX > 0)
	{
		while ((screenX < _currentXsize) && (tickCount < 1000))
		{
			WebGraph_Draw3Dline(screenX, -3, 0, screenX, 3, 0, color);

			if (useFractionFormat || ((scaleValue > 0.0) && (scaleValue < 0.9)))
			{
				useFractionFormat	=	true;
				if (deltaValueX >= 1.0)
				{
					sprintf(scaleText, "%3.1f", scaleValue);
				}
				else if (deltaValueX >= 0.1)
				{
					sprintf(scaleText, "%3.2f", scaleValue);
				}
				else if (deltaValueX >= 0.01)
				{
					sprintf(scaleText, "%3.2f", scaleValue);
				}
				else
				{
					sprintf(scaleText, "%3.3f", scaleValue);
				}
			}
			else
			{
				sprintf(scaleText, "%1.0f", scaleValue);
			}

			WebGraph_DrawText(scaleText, screenX, screenY, 0, color);

			scaleValue	+=	deltaValueX;
			screenX_DBL	+=	deltaPixelsX;
			screenX		=	screenX_DBL;
			tickCount++;
		}
	}
	else if (deltaPixelsX < 0)
	{
		while (-screenX < _currentXsize)
		{
			WebGraph_Draw3Dline(screenX, -3, 0, screenX, 3, 0, color);

			sprintf(scaleText, "%1.0f", scaleValue);

			WebGraph_DrawText(scaleText, screenX, screenY, 0, color);

			scaleValue	+=	deltaValueX;
			screenX		+=	deltaPixelsX;
		}
	}
	else
	{
		CONSOLE_DEBUG("Program error: deltaPixelsX is zero");
	}
}

//*****************************************************************************
void	WebGraph_DrawScaleTickMarks_Y(double deltaPixelsY, double startValueY, double deltaValueY, cv::Scalar color)
{
double	screenY_DBL;
int		screenX, screenY;
double	scaleValue;
char	scaleText[32];
bool	useFractionFormat;
int		loopCntr;

#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG("WebGraph_DrawScaleTickMarks_Y");
	CONSOLE_DEBUG_W_DBL("deltaPixelsY =", deltaPixelsY);
	CONSOLE_DEBUG_W_DBL("deltaValueY =", deltaValueY);
#endif

	screenX		=	-5;
	screenY		=	0;
	scaleValue	=	startValueY;
	screenY_DBL	=	screenY;

	if (deltaValueY < 1.0)
	{
		useFractionFormat	=	true;
	}
	else
	{
		useFractionFormat	=	false;
	}
	if (deltaPixelsY > 0.0)
	{
		loopCntr	=	0;
		while ((screenY < _currentYsize) && (loopCntr < 1000))
		{
		#ifdef _GRAPH_VERBOSE_
			CONSOLE_DEBUG_W_NUM("screenY =", screenY);
			CONSOLE_DEBUG_W_DBL("deltaPixelsY =", deltaPixelsY);
		#endif
			WebGraph_Draw3DlineDashed(-3, screenY, 0, +3, screenY, 0, color);

			if (useFractionFormat || ((scaleValue > 0.0) && (scaleValue < 0.9)))
			{
				useFractionFormat	=	true;
				if (deltaValueY >= 0.1)
				{
					sprintf(scaleText, "%3.1f", scaleValue);
				}
				else if (deltaValueY >= 0.01)
				{
					sprintf(scaleText, "%3.2f", scaleValue);
				}
				else if (deltaValueY < 0.001)
				{
					sprintf(scaleText, "%3.5f", scaleValue);
				}
				else
				{
					sprintf(scaleText, "%3.3f", scaleValue);
				}
			}
			else
			{
				sprintf(scaleText, "%1.0f", scaleValue);
			}
			WebGraph_DrawText(scaleText, (screenX - 25), (screenY + 7), 0, color);

			scaleValue	+=	deltaValueY;
			screenY_DBL	+=	deltaPixelsY;
			screenY		=	screenY_DBL;

			loopCntr++;
		}
		if (loopCntr > 900)
		{
			CONSOLE_DEBUG_W_NUM("too many loops; loopCntr =", loopCntr);
		}
	}
	else if (deltaPixelsY < 0.0)
	{
		screenX		=	-5;
		screenY		=	0;
		scaleValue	=	startValueY;
		while (-screenY < _currentYsize)
		{
		#ifdef _GRAPH_VERBOSE_
			CONSOLE_DEBUG_W_NUM("screenY =", screenY);
		#endif
			WebGraph_Draw3DlineDashed(-3, screenY, 0, +3, screenY, 0, color);

			if ((scaleValue > 0.0) && (scaleValue < 0.9))
			{
				sprintf(scaleText, "%3.3f", scaleValue);
			}
			else
			{
				sprintf(scaleText, "%1.0f", scaleValue);
			}
			WebGraph_DrawText(scaleText, (screenX - 25), (screenY + 7), 0, color);

			scaleValue	+=	deltaValueY;
			screenY		+=	deltaPixelsY;
		}
	}
	else
	{
		CONSOLE_DEBUG("Program error: deltaPixelsY is zero");
	}

//	CONSOLE_DEBUG("WebGraph_DrawScaleTickMarks_Y  EXIT");


}

//**************************************************************************************
static void	WebGraph_DrawVectorArrowHead(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color)
{
int		deltaX, deltaY, deltaZ;
double	angle;
double	angle180;
int		myX1;
int		myY1;
int		myX2;
int		myY2;
int		arrowDx;
int		arrowDy;
double	pointLen	=	12.0;
double	arrowPtAngle;

#ifdef _DEBUG_IMAGE_
	CONSOLE_DEBUG_W_HEX("_gWebimGD\t=", _gWebimGD);
#endif
	if (gWebGrapicsImage != NULL)
	{
		myX1		=	WebGraph_TranslateXvalue(x1, z1);
		myY1		=	WebGraph_TranslateYvalue(y1, z1);
		myX2		=	WebGraph_TranslateXvalue(x2, z2);
		myY2		=	WebGraph_TranslateYvalue(y2, z2);

		deltaX		=	myX2 - myX1;
		deltaY		=	-(myY2 - myY1);
		deltaZ		=	z2 - z1;

		if ((deltaX == 0) && (myY1 > myY2) && (deltaZ == 0))
		{
			angle	=	90.0 * (M_PI / 180.0);
		}
		else if ((deltaX == 0) && (myY1 < myY2) && (deltaZ == 0))
		{
			angle	=	-90.0 * (M_PI / 180.0);
		}
		else if ((deltaY == 0) && (myX1 < myX2) && (deltaZ == 0))
		{
			angle	=	0;
		}
		else if ((deltaY == 0) && (myX1 > myX2) && (deltaZ == 0))
		{
			angle	=	180.0 * (M_PI / 180.0);
		}
		else
		{
			angle	=	atan2((1.0 * deltaY), (1.0 * deltaX));
		}
		angle180	=	angle + M_PI;

		//*	we now have the angle pointing from the end, pointing BACK along the vector
		arrowPtAngle	=	18.0 * (M_PI / 180.0);


		//*	draw 2 lines back along this line
		arrowDx	=	pointLen * cos(angle180 + arrowPtAngle);
		arrowDy	=	-pointLen * sin(angle180 + arrowPtAngle);

		WebGraph_Draw2Dline(	myX2,
								myY2,
								myX2 + arrowDx,
								myY2 + arrowDy,
								color);

		arrowDx	=	pointLen * cos(angle180 - arrowPtAngle);
		arrowDy	=	-pointLen * sin(angle180 - arrowPtAngle);
		WebGraph_Draw2Dline(	myX2,
								myY2,
								myX2 + arrowDx,
								myY2 + arrowDy,
								color);

	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//*****************************************************************************
void	WebGraph_DrawVector(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color, const char *labelText)
{
int			myX1;
int			myY1;
int			myLabel_Y;
cv::Point	textLoc;

	if (gWebGrapicsImage != NULL)
	{
		//*	draw the line
		WebGraph_Draw3Dline(x1, y1, z1, x2, y2, z2, color);

		//*	now draw an arrow head
		WebGraph_DrawVectorArrowHead(x1, y1, z1, x2, y2, z2, color);


		if (strlen(labelText) > 0)
		{
			myX1		=	WebGraph_TranslateXvalue(x2, z2);
			myY1		=	WebGraph_TranslateYvalue(y2, z2);

#define	kXlabelSpace	100

			if (myX1 > (_currentXsize - kXlabelSpace))
			{
				myX1	=	_currentXsize - kXlabelSpace;
			}
			myLabel_Y	=	myY1 + 13;
			if (myLabel_Y > (_currentYsize - 10))
			{
				myLabel_Y	=	_currentYsize - 10;
			}
//			gdImageString(_gWebimGD, gdFontGetGiant(), (myX1 + 5), myLabel_Y, (unsigned char *)labelText, color);
			textLoc.x	=	myX1 + 5;
			textLoc.y	=	myLabel_Y;
			//cv::FONT_HERSHEY_PLAIN,	0.7, 1);
			cv::putText(	*gWebGrapicsImage,
							labelText,
							textLoc,
							cv::FONT_HERSHEY_PLAIN,
							1.0,
							color,
							1
							);
		}
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//*****************************************************************************
void	WebGraph_Draw2Dline(int x1, int y1, int x2, int y2, cv::Scalar color)
{
cv::Point	pt1;
cv::Point	pt2;


	if (gWebGrapicsImage != NULL)
	{
		pt1.x	=	x1;
		pt1.y	=	y1;

		pt2.x	=	x2;
		pt2.y	=	y2;
		cv::line(	*gWebGrapicsImage,
				pt1,
				pt2,
				color,				//	color,
				gCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));
	}
}


//*****************************************************************************
void	WebGraph_Draw3Dline(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color)
{
int			myX1;
int			myY1;
int			myX2;
int			myY2;
cv::Point	pt1;
cv::Point	pt2;


	myX1		=	WebGraph_TranslateXvalue(x1, z1);
	myY1		=	WebGraph_TranslateYvalue(y1, z1);

	myX2		=	WebGraph_TranslateXvalue(x2, z2);
	myY2		=	WebGraph_TranslateYvalue(y2, z2);
	if (gWebGrapicsImage != NULL)
	{
		pt1.x	=	myX1;
		pt1.y	=	myY1;

		pt2.x	=	myX2;
		pt2.y	=	myY2;
		cv::line(	*gWebGrapicsImage,
				pt1,
				pt2,
				color,				//	color,
				gCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));
	}
}

//*****************************************************************************
void	WebGraph_Draw3DlineDashed(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color)
{
int			myX1;
int			myY1;
int			myX2;
int			myY2;
cv::Point	pt1;
cv::Point	pt2;

#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG("__FUNCTION__");
#endif

	myX1		=	WebGraph_TranslateXvalue(x1, z1);
	myY1		=	WebGraph_TranslateYvalue(y1, z1);

	myX2		=	WebGraph_TranslateXvalue(x2, z2);
	myY2		=	WebGraph_TranslateYvalue(y2, z2);


	if ((myX1 > -100000) && (myX2 > -100000) && (myY1 > -100000) && (myY2 > -100000))
	{
		if (gWebGrapicsImage != NULL)
		{
			pt1.x	=	myX1;
			pt1.y	=	myY1;

			pt2.x	=	myX2;
			pt2.y	=	myY2;
			cv::line(	*gWebGrapicsImage,
					pt1,
					pt2,
					color,		//	color,
					gCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
					8,					//	int line_type CV_DEFAULT(8),
					0);					//	int shift CV_DEFAULT(0));

		}
		else
		{
			CONSOLE_DEBUG("_gWebimGD is NULL");
		}
	}
	else
	{
#ifdef _GRAPH_SHOW_ERRORS_
		CONSOLE_DEBUG("---------------------------------");
		CONSOLE_DEBUG_W_NUM("Parameters out of range x1", x1);
		CONSOLE_DEBUG_W_NUM("Parameters out of range y1", y1);
		CONSOLE_DEBUG_W_NUM("Parameters out of range x2", x2);
		CONSOLE_DEBUG_W_NUM("Parameters out of range y2", y2);


		CONSOLE_DEBUG_W_NUM("Parameters out of range myX1", myX1);
		CONSOLE_DEBUG_W_NUM("Parameters out of range myX2", myX2);
		CONSOLE_DEBUG_W_NUM("Parameters out of range myY1", myY1);
		CONSOLE_DEBUG_W_NUM("Parameters out of range myY2", myY2);
#endif
	}
#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG("Done drawing dashed line");
#endif

}

//*****************************************************************************
void	WebGraph_DrawScreenTextLg(const char *theText, int x1, int y1, cv::Scalar color)
{
cv::Point	textLoc;

	if (gWebGrapicsImage != NULL)
	{
		textLoc.x	=	x1;
		textLoc.y	=	y1;
		//cv::FONT_HERSHEY_PLAIN,	0.7, 1);
		cv::putText(	*gWebGrapicsImage,
						theText,
						textLoc,
						cv::FONT_HERSHEY_PLAIN,
						1.0,
						color,
						1
						);
	}
}

//*****************************************************************************
void	WebGraph_DrawScreenTextLg_translated(const char *theText, int x1, int y1, cv::Scalar color)
{
int			myX1;
int			myY1;
cv::Point	textLoc;

	myX1		=	WebGraph_TranslateXvalue(x1, 0);
	myY1		=	WebGraph_TranslateYvalue(y1, 0);
	if (gWebGrapicsImage != NULL)
	{
		textLoc.x	=	myX1;
		textLoc.y	=	myY1;

		//cv::FONT_HERSHEY_PLAIN,	0.7, 1);
		cv::putText(	*gWebGrapicsImage,
						theText,
						textLoc,
						cv::FONT_HERSHEY_PLAIN,
						1.0,
						color,
						1
						);
	}
}

//*****************************************************************************
void	WebGraph_DrawScale(double scaleX, double scaleY, cv::Scalar color)
{
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_Draw2DCircle(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (gWebGrapicsImage != NULL)
	{
	cv::Point	center;
	cv::Size	axes;

		if ((sizeX > 0) && (sizeY > 0))
		{
			center.x	=	centerX;
			center.y	=	centerY;
//			axes.width	=	1 * xRadius;
//			axes.height	=	1 * yRadius;
			axes.width	=	sizeX / 2;
			axes.height	=	sizeY / 2;

			cv::ellipse(	*gWebGrapicsImage,
							center,
							axes,
							0.0,			//*	angle
							0.0,			//*	start_angle
							360.0,			//*	end_angle
							color,			//	color,
							gCurrentLineWidth
							);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", centerX);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", centerY);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", sizeX);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", sizeY);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//*****************************************************************************
void	WebGraph_Draw2DCircleDashed(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color)
{
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_Draw2DCircleFilled(int centerX, int centerY, int xRadius, int yRadius, cv::Scalar color)
{
cv::Point	center;
cv::Size	axes;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (gWebGrapicsImage != NULL)
	{
		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	centerX;
			center.y	=	centerY;
			axes.width	=	xRadius / 2;
			axes.height	=	yRadius / 2;

			cv::ellipse(	*gWebGrapicsImage,
							center,
							axes,
							0.0,			//*	angle
							0.0,			//*	start_angle
							360.0,			//*	end_angle
							color,			//	color,
						#if (CV_MAJOR_VERSION >= 3)
							cv::FILLED				//	int thickness CV_DEFAULT(1),
						#else
							CV_FILLED
						#endif
							);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", centerX);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", centerY);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}


//*****************************************************************************
void	WebGraph_Draw2DCircleFilled_translated(int centerX, int centerY, int xRadius, int yRadius, cv::Scalar color)
{
int			myX1;
int			myY1;
cv::Point	center;
cv::Size	axes;

//	CONSOLE_DEBUG(__FUNCTION__);

	myX1		=	WebGraph_TranslateXvalue(centerX, 0);
	myY1		=	WebGraph_TranslateYvalue(centerY, 0);

	if (gWebGrapicsImage != NULL)
	{
		if ((xRadius > 0) && (yRadius > 0))
		{
//			center.x	=	centerX;
//			center.y	=	centerY;
			center.x	=	myX1;
			center.y	=	myY1;
			axes.width	=	xRadius / 2;
			axes.height	=	yRadius / 2;

			cv::ellipse(	*gWebGrapicsImage,
							center,
							axes,
							0.0,			//*	angle
							0.0,			//*	start_angle
							360.0,			//*	end_angle
							color,			//	color,
						#if (CV_MAJOR_VERSION >= 3)
							cv::FILLED				//	int thickness CV_DEFAULT(1),
						#else
							CV_FILLED
						#endif
							);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", centerX);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", centerY);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//*****************************************************************************
void	WebGraph_Draw2D_RegMark(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color)
{
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
cv::Scalar		WebGraph_GetColorByIndex(int colorIndex)
{
cv::Scalar	lineColor;

	//*	make sure the color index is in range
	while (colorIndex >= kGraphPointMode_Last)
	{
		colorIndex	-=	kGraphPointMode_Last;
	}
	switch (colorIndex)
	{
		case kGraphPointMode_RedLine:
		case kGraphPointMode_RedLineDashed:
		case kGraphPointMode_RedDot:
		case kGraphPointMode_Red_X:
		case kGraphPointMode_RedDot_Large:
												lineColor	=	_gdColorRed;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_GreenLine:
		case kGraphPointMode_GreenLineDashed:
		case kGraphPointMode_GreenDot:
		case kGraphPointMode_Green_X:
		case kGraphPointMode_GreenDot_Large:
												lineColor	=	_gdColorGreen;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_BlueLine:
		case kGraphPointMode_BlueLineDashed:
		case kGraphPointMode_BlueDot:
		case kGraphPointMode_Blue_X:
		case kGraphPointMode_BlueDot_Large:
												lineColor	=	_gdColorBlue;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_OrangeLine:
		case kGraphPointMode_OrangeLineDashed:
		case kGraphPointMode_OrangeDot:
		case kGraphPointMode_Orange_X:
		case kGraphPointMode_OrangeDot_Large:
												lineColor	=	_gdColorOrange;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_CyanLine:
		case kGraphPointMode_CyanLineDashed:
		case kGraphPointMode_CyanDot:
		case kGraphPointMode_Cyan_X:
		case kGraphPointMode_CyanDot_Large:
												lineColor	=	_gdColorCyan;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_MagentaLine:
		case kGraphPointMode_MagentaLineDashed:
		case kGraphPointMode_MagentaDot:
		case kGraphPointMode_Magenta_X:
		case kGraphPointMode_MagentaDot_Large:
												lineColor	=	_gdColorMagenta;
												break;

		//----------------------------------------------------------------------
		case kGraphPointMode_YellowLine:
		case kGraphPointMode_YellowLineDashed:
		case kGraphPointMode_YellowDot:
		case kGraphPointMode_Yellow_X:
		case kGraphPointMode_YellowDot_Large:
												lineColor	=	_gdColorYellow;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_GreyLine:
												lineColor	=	_gdColorGrey;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_DkGreenLine:
		case kGraphPointMode_DkGreenDot:
												lineColor	=	_gdColorDkGreen;
												break;
		//----------------------------------------------------------------------
		case kGraphPointMode_none:
		case kGraphPointMode_BlackLine:
		case kGraphPointMode_BlackLineDashed:
		case kGraphPointMode_BlackDot:
		case kGraphPointMode_Black_X:
		case kGraphPointMode_BlackDot_Large:
												lineColor	=	_gdColorBlack;
												break;
		//----------------------------------------------------------------------
		default:
												lineColor	=	_gdColorBlack;
												break;
	}
	return(lineColor);
}

//*****************************************************************************
//*	scaled routines
//*****************************************************************************
void	WebGraph_Draw2DlineScaled(double x1, double y1, double x2, double y2, double scaleFactor, cv::Scalar color)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_Draw2DlineScaledXY(double x1, double y1, double x2, double y2, double scaleFactorX, double scaleFactorY, cv::Scalar color)
{
//	CONSOLE_DEBUG(__FUNCTION__);
int	myX1, myY1;
int	myX2, myY2;

	myX1	=	scaleFactorX * x1;
	myY1	=	scaleFactorY * y1;
	myX2	=	scaleFactorX * x2;
	myY2	=	scaleFactorY * y2;
	WebGraph_Draw3Dline(myX1, myY1, 0, myX2, myY2, 0, color);

}

//*****************************************************************************
void	WebGraph_Draw2DlineScaledXY_dashed(double x1, double y1, double x2, double y2, double scaleFactorX, double scaleFactorY, cv::Scalar color)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");
int	myX1, myY1;
int	myX2, myY2;

	myX1	=	scaleFactorX * x1;
	myY1	=	scaleFactorY * y1;
	myX2	=	scaleFactorX * x2;
	myY2	=	scaleFactorY * y2;
	WebGraph_Draw3Dline(myX1, myY1, 0, myX2, myY2, 0, color);
}

//*****************************************************************************
void	WebGraph_Draw2DlineDashedScaled(double x1, double y1, double x2, double y2, double scaleFactor, cv::Scalar color)
{
int	myX1, myY1;
int	myX2, myY2;

//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_DBL("scaleFactor =", scaleFactor);
#endif

	myX1	=	scaleFactor * x1;
	myY1	=	scaleFactor * y1;
	myX2	=	scaleFactor * x2;
	myY2	=	scaleFactor * y2;
	WebGraph_Draw3DlineDashed(myX1, myY1, 0, myX2, myY2, 0, color);

}

//*****************************************************************************
void	WebGraph_DrawTextScaled(const char *theText, double x1, double y1, double z1, cv::Scalar color, double scaleFactor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_DrawDimensionScaled(double x1, double y1, double z1, double x2, double y2, double z2, cv::Scalar color, const char *labelText, double scaleFactor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_DrawVectorScaled(double x1, double y1, double z1, double x2, double y2, double z2, cv::Scalar color, const char *labelText, double scaleFactor)
{
int	my_x1;
int	my_y1;
int	my_z1;
int	my_x2;
int	my_y2;
int	my_z2;

//	CONSOLE_DEBUG(__FUNCTION__);

	my_x1	=	x1 * scaleFactor;
	my_y1	=	y1 * scaleFactor;
	my_z1	=	z1 * scaleFactor;
	my_x2	=	x2 * scaleFactor;
	my_y2	=	y2 * scaleFactor;
	my_z2	=	z2 * scaleFactor;

	WebGraph_DrawVector(my_x1, my_y1, my_z1, my_x2, my_y2, my_z2, color, labelText);

}

//*****************************************************************************
void	WebGraph_Draw2DArcScaled(	double	centerX,
									double	centerY,
									double	sizeX,
									double	sizeY,
									double	startAngle,
									double	endAngle,
									int		color,
									double	scaleFactor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_SetPixel(const int xLoc, const int yLoc, const cv::Scalar color)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_SetPixelScaled(const int xLoc, const int yLoc, const cv::Scalar color, double scaleFactor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_DrawText(const char *theText, int x1, int y1, int z1, cv::Scalar color)
{
int			myX1;
int			myY1;
cv::Point	textLoc;

//	CONSOLE_DEBUG(__FUNCTION__);

	myX1		=	WebGraph_TranslateXvalue(x1, z1);
	myY1		=	WebGraph_TranslateYvalue(y1, z1);

	if (gWebGrapicsImage != NULL)
	{
		textLoc.x	=	myX1;
		textLoc.y	=	myY1;

		cv::putText(	*gWebGrapicsImage,
						theText,
						textLoc,
						cv::FONT_HERSHEY_PLAIN,
						1.0,
						color,
						1
						);
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//*****************************************************************************
void	WebGraph_DrawTextLg_translated(const char *theText, int x1, int y1, int z1, cv::Scalar color)
{
int			myX1;
int			myY1;
cv::Point	textLoc;

	myX1		=	WebGraph_TranslateXvalue(x1, z1);
	myY1		=	WebGraph_TranslateYvalue(y1, z1);

	if (gWebGrapicsImage != NULL)
	{
		textLoc.x	=	myX1;
		textLoc.y	=	myY1;

		cv::putText(	*gWebGrapicsImage,
						theText,
						textLoc,
						cv::FONT_HERSHEY_PLAIN,
						1.0,
						color,
						1
						);
	}
	else
	{
		CONSOLE_DEBUG("gWebGrapicsImage is NULL");
	}
}

//**************************************************************************************
static void	WebGraph_DrawMarker(int			myScreenX,
								int			myScreenY,
								int			pointMode,
								cv::Scalar	lineColor)
{
	switch (pointMode)
	{
		case kGraphPointMode_none:			//*	do nothing
			break;

		case kGraphPointMode_RedDot:		//*	draw a little circle on the spot
		case kGraphPointMode_GreenDot:
		case kGraphPointMode_BlueDot:
		case kGraphPointMode_OrangeDot:
		case kGraphPointMode_CyanDot:
		case kGraphPointMode_MagentaDot:
		case kGraphPointMode_YellowDot:
		case kGraphPointMode_BlackDot:
		case kGraphPointMode_DkGreenDot:
			WebGraph_Draw2DCircleFilled(myScreenX, myScreenY, kXcircleDiam, kXcircleDiam, lineColor);
			break;

		case kGraphPointMode_RedDot_Large:
		case kGraphPointMode_GreenDot_Large:
		case kGraphPointMode_BlueDot_Large:
		case kGraphPointMode_OrangeDot_Large:
		case kGraphPointMode_CyanDot_Large:
		case kGraphPointMode_MagentaDot_Large:
		case kGraphPointMode_YellowDot_Large:
		case kGraphPointMode_BlackDot_Large:
			WebGraph_Draw2DCircleFilled(myScreenX, myScreenY, kXcircleDiamLarge, kXcircleDiamLarge, lineColor);
			break;


		//*	draw a little x on the spot
		case kGraphPointMode_Red_X:
		case kGraphPointMode_Green_X:
		case kGraphPointMode_Blue_X:
		case kGraphPointMode_Orange_X:
		case kGraphPointMode_Cyan_X:
		case kGraphPointMode_Magenta_X:
		case kGraphPointMode_Yellow_X:
		case kGraphPointMode_Black_X:
			WebGraph_Draw3Dline((myScreenX - kXdotLen), (myScreenY - kXdotLen), 0, (myScreenX + kXdotLen), (myScreenY + kXdotLen), 0, lineColor);
			WebGraph_Draw3Dline((myScreenX - kXdotLen), (myScreenY + kXdotLen), 0, (myScreenX + kXdotLen), (myScreenY - kXdotLen), 0, lineColor);
			break;

		default:						//*	do nothing
			break;

	}
}

//*****************************************************************************
void	WebGraph_GraphDataArray(double	startX,
								double	endX,
								double	labelStepX,
								double	startY,
								double	endY,
								double	labelStepY,
								int		numberCnt,
								double	*xValues,
								double	*yValues,
								int		pointColorMode,
								double	*rtnScaleX,
								double	*rtnScaleY,
								bool	drawAxis,
								bool	drawFullLengthVerticalTickMarks,
								bool	drawFullLengthHorizontalTickMarks)
{
double		xScaleFactor;
double		yScaleFactor;
int			iii;
double		myX1;
double		myY1;
double		myX2;
double		myY2;
int			myScreenX;
int			myScreenY;
double		usableWidth;
double		deltaX;
double		deltaY;
cv::Scalar	lineColor;

//	CONSOLE_DEBUG_W_DBL("web-endY",		endY);

#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_NUM("numberCnt =", numberCnt);
	CONSOLE_DEBUG_W_DBL("startX =", startX);
	CONSOLE_DEBUG_W_DBL("endX =", endX);
	CONSOLE_DEBUG_W_DBL("startY =", startY);
	CONSOLE_DEBUG_W_DBL("endY =", endY);
#endif
	if (drawAxis)
	{
		//*	adjust this later
		if ((_currentXsize == 640) && (_currentYsize == 800))
		{
			WebGraph_DrawAxis(kAxisStyle_XY_640x800, _gdColorBlack);
		}
		else
		{
		//	WebGraph_DrawAxis(kAxisStyle_XY, _gdColorBlack);
			WebGraph_DrawAxis(kAxisStyle_XY_NoLabels, _gdColorBlack);

		}
	}

	usableWidth		=	(1.0 * _currentXsize) - _xCenter;

	deltaX			=	endX - startX;
	if (fabs(deltaX) > 0.0000001)
	{
		xScaleFactor	=	usableWidth / (endX - startX);
	}
	else
	{
		xScaleFactor	=	1;
	}
#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_DBL("usableWidth =", usableWidth);
	CONSOLE_DEBUG_W_DBL("endX - startX =", (endX - startX));
	CONSOLE_DEBUG_W_DBL("xScaleFactor =", xScaleFactor);
	CONSOLE_DEBUG_W_DBL("labelStepX", labelStepX);
#endif

	deltaY			=	endY - startY;
	if (fabs(deltaY) > 0.000000001)
	{
//		CONSOLE_DEBUG("deltaY != 0");
		yScaleFactor	=	(_yCenter) / (deltaY);
	}
	else
	{
		yScaleFactor	=	1;
	}
#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_DBL("deltaY =", deltaY);
	CONSOLE_DEBUG_W_DBL("endY - startY =", (endY - startY));
	CONSOLE_DEBUG_W_DBL("yScaleFactor =", yScaleFactor);
	CONSOLE_DEBUG_W_DBL("labelStepY", labelStepY);
#endif

	lineColor	=	WebGraph_GetColorByIndex(pointColorMode);


	if (xScaleFactor > 0.0)
	{
		if (labelStepX != 0.0)
		{
			WebGraph_DrawScaleTickMarks_X((xScaleFactor * labelStepX), startX, labelStepX, _gdColorBlack);
		}
	}

#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_DBL("yScaleFactor =", yScaleFactor);
#endif

	if (yScaleFactor > 0.0)
	{
		if (labelStepY != 0.0)
		{
			WebGraph_DrawScaleTickMarks_Y((yScaleFactor * labelStepY), startY, labelStepY, _gdColorBlack);
		}
	}
	else
	{
	#ifdef _GRAPH_SHOW_ERRORS_
		CONSOLE_DEBUG_W_DBL("yScaleFactor < 0.0", yScaleFactor);
	#endif
	}
#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG("debugging1");
#endif

	if (drawFullLengthVerticalTickMarks)
	{
		//*	draw vertical dashed lines at each of the tick marks
		if ((startX < endX) && (labelStepX > 0.0))
		{
		#ifdef _GRAPH_VERBOSE_
			CONSOLE_DEBUG_W_DBL("drawing verticl dashed lines; labelStepX=", labelStepX);
		#endif
			//*	put vertical dash lines at every tick mark
			myX1	=	startX;
			while (myX1 <= endX)
			{
			double	myAdjustedX1;

			#ifdef _GRAPH_VERBOSE_
				CONSOLE_DEBUG_W_DBL("myX1=", myX1);
			#endif

				myX1			+=	labelStepX;
				myAdjustedX1	=	myX1 - startX;
	//			WebGraph_Draw2DlineDashedScaled(myAdjustedX1, 0, myAdjustedX1, (_currentYsize * 5), xScaleFactor, _gdColorRed);
				WebGraph_Draw2DlineDashedScaled(myAdjustedX1, 0, myAdjustedX1, ((_currentYsize / xScaleFactor) * 5), xScaleFactor, _gdColorGrey);
			}
		}
	}

	if (drawFullLengthHorizontalTickMarks)
	{
		//*	draw Horizontal dashed lines at each of the tick marks
		if ((startY < endY) && (labelStepY > 0.0))
		{
		#ifdef _DEBUG_HORIZONTAL_GRIDS_
			CONSOLE_DEBUG_W_DBL("drawing Horizontal dashed lines; labelStepY=", labelStepY);
			CONSOLE_DEBUG_W_DBL("startY", startY);
			CONSOLE_DEBUG_W_DBL("endY", endY);
		#endif
			//*	put vertical dash lines at every tick mark
			myY1	=	startY;
			while (myY1 <= endY)
			{
			double	myAdjustedY1;

			#ifdef _DEBUG_HORIZONTAL_GRIDS_
				CONSOLE_DEBUG_W_DBL("myY1=", myY1);
			#endif
				myX1			=	0;
				myX2			=	((_currentXsize / yScaleFactor) * 5);
				myY1			+=	labelStepY;
				myAdjustedY1	=	myY1 - startY;

				WebGraph_Draw2DlineDashedScaled(myX1, myAdjustedY1, myX2, myAdjustedY1, yScaleFactor, _gdColorGrey);
			}
		}
	}

	if (startY < 0)
	{
		WebGraph_Draw2DlineDashedScaled(0, -startY, _currentXsize, -startY, yScaleFactor, _gdColorGreen);
	}

	for (iii=0; iii < (numberCnt - 1); iii++)
	{
		myX1	=	xValues[iii] - startX;
		myY1	=	yValues[iii] - startY;

		if (iii == 0)
		{
			//*	draw a marker at the first point
			myScreenX	=	xScaleFactor * myX1;
			myScreenY	=	yScaleFactor * myY1;
			WebGraph_DrawMarker(myScreenX, myScreenY, pointColorMode, lineColor);
		}

		myX2	=	xValues[iii + 1] - startX;
		myY2	=	yValues[iii + 1] - startY;

//		CONSOLE_DEBUG_W_DBL("myX1 =", myX1);
//		CONSOLE_DEBUG_W_DBL("myY1 =", myY1);
		switch (pointColorMode)
		{
			case kGraphPointMode_RedLineDashed:
			case kGraphPointMode_GreenLineDashed:
			case kGraphPointMode_BlueLineDashed:
			case kGraphPointMode_OrangeLineDashed:
			case kGraphPointMode_CyanLineDashed:
			case kGraphPointMode_MagentaLineDashed:
			case kGraphPointMode_BlackLineDashed:
				WebGraph_Draw2DlineScaledXY_dashed(myX1, myY1, myX2, myY2, xScaleFactor, yScaleFactor, lineColor);
				break;

			default:
				WebGraph_Draw2DlineScaledXY(myX1, myY1, myX2, myY2, xScaleFactor, yScaleFactor, lineColor);
				break;
		}
		myScreenX	=	xScaleFactor * myX2;
		myScreenY	=	yScaleFactor * myY2;
		WebGraph_DrawMarker(myScreenX, myScreenY, pointColorMode, lineColor);

	}
#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG_W_DBL("startX=", startX);
	CONSOLE_DEBUG_W_DBL("endX=", endX);
	CONSOLE_DEBUG_W_DBL("labelStepX=", labelStepX);
#endif


#ifdef _GRAPH_VERBOSE_
	CONSOLE_DEBUG("exit");
#endif
	if (rtnScaleX != NULL)
	{
		*rtnScaleX	=	xScaleFactor;
	}
	if (rtnScaleY != NULL)
	{
		*rtnScaleY	=	yScaleFactor;
	}
}

//*****************************************************************************
void	WebGraph_DrawTitle(const char *titleText, const cv::Scalar color)
{
int			myScreenX;
int			middleScreenX;
int			myScreenY;
int			textLength;
cv::Point	textLoc;

	textLength		=	strlen(titleText);
	middleScreenX	=	_currentXsize / 2;
	myScreenX		=	middleScreenX - ((textLength / 2) * 10);
	myScreenY		=	_currentYsize - 25;

	if (gWebGrapicsImage != NULL)
	{
		textLoc.x	=	myScreenX;
		textLoc.y	=	myScreenY;

		cv::putText(	*gWebGrapicsImage,
						titleText,
						textLoc,
						cv::FONT_HERSHEY_PLAIN,
						1.0,
						color,
						1
						);
	}
}

//*****************************************************************************
void	WebGraph_DrawLegend(const int index, const int pointColorMode, const char *legendText, const int lengendWidth)
{
int			myScreenX;
int			myScreenY;
cv::Scalar	pointColor;
cv::Rect	myCVrect;
int			myX1;
int			myX2;
int			myY1;

	myScreenX	=	_currentXsize - lengendWidth;
	myScreenY	=	_currentYsize - 64;
	myScreenY	-=	(index * kLegendLineHeight);

	pointColor	=	WebGraph_GetColorByIndex(pointColorMode);

	myX1		=	WebGraph_TranslateXvalue(myScreenX, 0);
	myY1		=	WebGraph_TranslateYvalue(myScreenY, 0);
	myX2		=	myX1 + lengendWidth;
	if (myX2 > (_currentXsize - 2))
	{
		myX2	=	_currentXsize - 2;
	}

	if (gWebGrapicsImage != NULL)
	{
//		gdImageFilledRectangle(_gWebimGD,	(myX1 - 25),
//											(myY1 - (kLegendLineHeight / 2)),
//											(myX2),
//											(myY1 + (kLegendLineHeight / 2)),
//											0xe0e0e0);

			myCVrect.x		=	(myX1 - 25);
			myCVrect.y		=	(myY1 - (kLegendLineHeight / 2));
//			myCVrect.width	=	myX2 - myX1;
			myCVrect.width	=	_currentXsize - myCVrect.x;
			myCVrect.height	=	kLegendLineHeight;

			cv::rectangle(	*gWebGrapicsImage,
							myCVrect,
							_gdColorLtGrey,
						#if (CV_MAJOR_VERSION >= 3)
							cv::FILLED				//	int thickness CV_DEFAULT(1),
						#else
							CV_FILLED
						#endif
							);
	}
	else
	{
	#ifdef _DEBUG_LEGEND_
		CONSOLE_DEBUG("myY1 is NULL");
	#endif
	}

	switch(pointColorMode)
	{
		case kGraphPointMode_RedLine:
		case kGraphPointMode_GreenLine:
		case kGraphPointMode_BlueLine:
		case kGraphPointMode_OrangeLine:
		case kGraphPointMode_CyanLine:
		case kGraphPointMode_MagentaLine:
		case kGraphPointMode_YellowLine:
		case kGraphPointMode_GreyLine:
		case kGraphPointMode_DkGreenLine:
		case kGraphPointMode_BlackLine:
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY, 0,
								(myScreenX + 15), myScreenY, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY+1, 0,
								(myScreenX + 15), myScreenY+1, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY-1, 0,
								(myScreenX + 15), myScreenY-1, 0, pointColor);
			break;

		case kGraphPointMode_RedDot:
		case kGraphPointMode_GreenDot:
		case kGraphPointMode_BlueDot:
		case kGraphPointMode_OrangeDot:
		case kGraphPointMode_CyanDot:
		case kGraphPointMode_MagentaDot:
		case kGraphPointMode_YellowDot:
		case kGraphPointMode_BlackDot:
		case kGraphPointMode_DkGreenDot:
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY, 0,
								(myScreenX + 15), myScreenY, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY+1, 0,
								(myScreenX + 15), myScreenY+1, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY-1, 0,
								(myScreenX + 15), myScreenY-1, 0, pointColor);
			WebGraph_Draw2DCircleFilled_translated(myScreenX, myScreenY, 10, 10, pointColor);
			break;

		case kGraphPointMode_RedLineDashed:
		case kGraphPointMode_GreenLineDashed:
		case kGraphPointMode_BlueLineDashed:
		case kGraphPointMode_OrangeLineDashed:
		case kGraphPointMode_CyanLineDashed:
		case kGraphPointMode_MagentaLineDashed:
		case kGraphPointMode_YellowLineDashed:
		case kGraphPointMode_BlackLineDashed:
			WebGraph_Draw3DlineDashed(	(myScreenX - 15), myScreenY, 0,
										(myScreenX + 15), myScreenY, 0, pointColor);
			WebGraph_Draw3DlineDashed(	(myScreenX - 15), myScreenY+1, 0,
										(myScreenX + 15), myScreenY+1, 0, pointColor);
			WebGraph_Draw3DlineDashed(	(myScreenX - 15), myScreenY-1, 0,
										(myScreenX + 15), myScreenY-1, 0, pointColor);
			break;

		case kGraphPointMode_Red_X:
		case kGraphPointMode_Green_X:
		case kGraphPointMode_Blue_X:
		case kGraphPointMode_Orange_X:
		case kGraphPointMode_Cyan_X:
		case kGraphPointMode_Magenta_X:
		case kGraphPointMode_Yellow_X:
		case kGraphPointMode_Black_X:
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY, 0,
								(myScreenX + 15), myScreenY, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY+1, 0,
								(myScreenX + 15), myScreenY+1, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY-1, 0,
								(myScreenX + 15), myScreenY-1, 0, pointColor);

			WebGraph_Draw3Dline((myScreenX - (2 * kXdotLen)), (myScreenY - (2 * kXdotLen)), 0, (myScreenX + (2 * kXdotLen)), (myScreenY + (2 * kXdotLen)), 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - (2 * kXdotLen)), (myScreenY + (2 * kXdotLen)), 0, (myScreenX + (2 * kXdotLen)), (myScreenY - (2 * kXdotLen)), 0, pointColor);
			break;

		case kGraphPointMode_RedDot_Large:
		case kGraphPointMode_GreenDot_Large:
		case kGraphPointMode_BlueDot_Large:
		case kGraphPointMode_OrangeDot_Large:
		case kGraphPointMode_CyanDot_Large:
		case kGraphPointMode_MagentaDot_Large:
		case kGraphPointMode_YellowDot_Large:
		case kGraphPointMode_BlackDot_Large:
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY, 0,
								(myScreenX + 15), myScreenY, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY+1, 0,
								(myScreenX + 15), myScreenY+1, 0, pointColor);
			WebGraph_Draw3Dline((myScreenX - 15), myScreenY-1, 0,
								(myScreenX + 15), myScreenY-1, 0, pointColor);
			WebGraph_Draw2DCircleFilled_translated(myScreenX, myScreenY, kXcircleDiamLarge, kXcircleDiamLarge, pointColor);
			break;


		default:
			//*	if its negative, dont do anything
			if (pointColorMode >= 0)
			{

				WebGraph_Draw3Dline((myScreenX - 15), myScreenY, 0,
									(myScreenX + 15), myScreenY, 0, pointColor);
				WebGraph_Draw3Dline((myScreenX - 15), myScreenY+1, 0,
									(myScreenX + 15), myScreenY+1, 0, pointColor);
				WebGraph_Draw3Dline((myScreenX - 15), myScreenY-1, 0,
									(myScreenX + 15), myScreenY-1, 0, pointColor);
			}
			break;
	}
	//*	this adjustment moves the text lines up with the line graphic
#ifdef	_USE_HERSHEY_FONTS_
	myScreenX	+=	30;
#else
	myScreenX	+=	20;
	myScreenY	-=	10;
#endif
	WebGraph_DrawTextLg_translated(legendText, myScreenX, myScreenY, 0, _gdColorBlack);
}

//*****************************************************************************
void	WebGraph_DrawAirplane(const int centerX, const int centerY, const int airplaneStyle, const double angle)
{
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
void	WebGraph_DrawTarget(const int centerX, const int centerY, const int targetType, double scale)
{
	CONSOLE_DEBUG("Not finished");

}

//*****************************************************************************
int		WebGraph_CheckTargetHit(const int targetType, const double hitXX_inches, const double hitYY_inches)
{
	CONSOLE_DEBUG("Not finished");

	return(0);
}

//*****************************************************************************
void	WebGraph_GraphDistribution(	double			*dataArray,
									const int		numValues,
									const int		numBins,
									const double	meanValue,
									const double	halfWidthValue,
									bool			includeStdDev,
									char			*jpgFileName,
									char			*legend0text,
									char			*graphTitle)
{
	CONSOLE_DEBUG("Not finished");

}


