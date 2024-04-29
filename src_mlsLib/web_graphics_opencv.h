//**************************************************************************************
//*	Web Graphics library (opencv version
//*		(C) 2024 by Mark Sproul
//*
//*
//**************************************************************************************
//*	Edit History
//**************************************************************************************
//*	Apr 26,	2024	<MLS> Created web_graphics_opencv.h
//**************************************************************************************
//#include	"web_graphics_opencv.h"

#ifndef _WEB_GRAPHICS_H_
#define	_WEB_GRAPHICS_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#include	<opencv2/opencv.hpp>
#include	<opencv2/core.hpp>



//**************************************************************************************
enum
{
	kAxisStyle_3D	=	0,
	kAxisStyle_3D_abs,
//	kAxisStyle_XYZ,
	kAxisStyle_XYZ_abs,
	kAxisStyle_XY,
	kAxisStyle_XY_NoLabels,
	kAxisStyle_XY_640x800,
	kAxisStyle_XT,
	kAxisStyle_UP,
	kAxisStyle_UP_centered,
	kAxisStyle_UP_4Quads,
	kAxisStyle_XY_abs,
	kAxisStyle_XY_centered,
	kAxisStyle_XY_compass,
	kAxisStyle_PolarXY,
	kAxisStyle_e3e1,
	kAxisStyle_e1e2,
	kAxisStyle_AlhpaBeta,

	kAxisStyle_last
};

//extern	gdImagePtr	_gWebimGD;		//* gdLibrary image
extern	cv::Scalar			_gdColorWhite;
extern	cv::Scalar			_gdColorBlack;
extern	cv::Scalar			_gdColorGrey;
extern	cv::Scalar			_gdColorLtGrey;
extern	cv::Scalar			_gdColorRed;
extern	cv::Scalar			_gdColorPink;
extern	cv::Scalar			_gdColorGreen;
extern	cv::Scalar			_gdColorDkGreen;
extern	cv::Scalar			_gdColorBlue;
extern	cv::Scalar			_gdColorCyan;
extern	cv::Scalar			_gdColorMagenta;
extern	cv::Scalar			_gdColorYellow;
extern	cv::Scalar			_gdColorOrange;
extern	cv::Scalar			_gdColorCopper;


//extern	int			_currentXsize;
//extern	int			_currentYsize;
extern	int			_xCenter;
extern	int			_yCenter;
//extern	double		_cosZangle;
//extern	double		_sinZangle;

#ifdef __cplusplus
	extern "C" {
#endif

void		WebGraph_CreateImage(int xSize, int ySize);
bool		WebGraph_CheckImage(const char *routineName, const int lineNum);

int			WebGraph_SaveImage(const char *pathName, const char *fileName);
int			WebGraph_TranslateXvalue(int xCordinate, int zCordinate);
int			WebGraph_TranslateYvalue(int yCordinate, int zCordinate);
void		WebGraph_DrawScaleX2(double scaleX, cv::Scalar color);

void		WebGraph_DrawAxis(int style, cv::Scalar color);
void		WebGraph_DrawScaleTickMarks_X(double deltaPixelsX, double startValueX, double deltaValueX, cv::Scalar color);
void		WebGraph_DrawScaleTickMarks_Y(double deltaPixelsY, double startValueY, double deltaValueY, cv::Scalar color);
void		WebGraph_DrawVector(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color, const char *labelText);
void		WebGraph_Draw3Dline(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color);
void		WebGraph_Draw3DlineDashed(int x1, int y1, int z1, int x2, int y2, int z2, cv::Scalar color);

void		WebGraph_DrawScreenTextLg(const char *theText, int x1, int y1, cv::Scalar color);
void		WebGraph_DrawScreenTextLg_translated(const char *theText, int x1, int y1, cv::Scalar color);
void		WebGraph_DrawScale(double scaleX, double scaleY, cv::Scalar color);

void		WebGraph_Draw2Dline(int x1, int y1, int x2, int y2, cv::Scalar color);
void		WebGraph_Draw2DCircle(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color);
void		WebGraph_Draw2DCircleDashed(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color);
void		WebGraph_Draw2DCircleFilled(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color);
void		WebGraph_Draw2D_RegMark(int centerX, int centerY, int sizeX, int sizeY, cv::Scalar color);
cv::Scalar	WebGraph_GetColorByIndex(int colorIndex);

//*	scaled routines
void		WebGraph_Draw2DlineScaled(double x1, double y1, double x2, double y2, double scaleFactor, cv::Scalar color);
void		WebGraph_Draw2DlineScaledXY(double x1, double y1, double x2, double y2, double scaleFactorX, double scaleFactorY, cv::Scalar color);
void		WebGraph_Draw2DlineScaledXY_dashed(double x1, double y1, double x2, double y2, double scaleFactorX, double scaleFactorY, cv::Scalar color);

void		WebGraph_Draw2DlineDashedScaled(double x1, double y1, double x2, double y2, double scaleFactor, cv::Scalar color);
void		WebGraph_DrawTextScaled(const char *theText, double x1, double y1, double z1, cv::Scalar color, double scaleFactor);
void		WebGraph_DrawDimensionScaled(double x1, double y1, double z1, double x2, double y2, double z2, cv::Scalar color, const char *labelText, double scaleFactor);
void		WebGraph_DrawVectorScaled(double x1, double y1, double z1, double x2, double y2, double z2, cv::Scalar color, const char *labelText, double scaleFactor);

void		WebGraph_Draw2DArcScaled(	double	centerX,
										double	centerY,
										double	sizeX,
										double	sizeY,
										double	startAngle,
										double	endAngle,
										int		color,
										double	scaleFactor);


void		WebGraph_SetPixel(const int xLoc, const int yLoc, const cv::Scalar color);
void		WebGraph_SetPixelScaled(const int xLoc, const int yLoc, const cv::Scalar color, double scaleFactor);


void	WebGraph_DrawText(const char *theText, int x1, int y1, int z1, cv::Scalar color);
void	WebGraph_DrawTextLg(const char *theText, int x1, int y1, int z1, cv::Scalar color);
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
								bool	drawFullLengthHorizontalTickMarks);
void	WebGraph_DrawTitle(const char *titleText, const cv::Scalar color);
void	WebGraph_DrawLegend(const int index, const int pointColorMode, const char *legendText, const int lengendWidth);


void	WebGraph_DrawAirplane(const int centerX, const int centerY, const int airplaneStyle, const double angle);
void	WebGraph_DrawTarget(const int centerX, const int centerY, const int targetType, double scale);
int		WebGraph_CheckTargetHit(const int targetType, const double hitXX_inches, const double hitYY_inches);

void	WebGraph_GraphDistribution(	double			*dataArray,
									const int		numValues,
									const int		numBins,
									const double	meanValue,
									const double	halfWidthValue,
									bool			includeStdDev,
									char			*jpgFileName,
									char			*legend0text,
									char			*graphTitle);


//**************************************************************************************
enum
{
	kTargetType_Esilhouete	=	0,
	kTargetType_IPSC,
	kTargetType_NRA600Yard,
	kTargetType_NRA1000Yard,

	kTargetType_last
};

//*	the number of colors per set in the list below
#define	kGraphPoint_ColorsPerSet	8

//**************************************************************************************
enum
{
	kGraphPointMode_none	=	0,

	kGraphPointMode_RedLine,			//*	this must be 1
	kGraphPointMode_GreenLine,			//*	there must be 10 lines in a row
	kGraphPointMode_BlueLine,			//*	these must be in the same color order as below
	kGraphPointMode_OrangeLine,
	kGraphPointMode_CyanLine,
	kGraphPointMode_MagentaLine,
	kGraphPointMode_YellowLine,
	kGraphPointMode_BlackLine,

	kGraphPointMode_RedLineDashed,		//*	these must be in the same color order as above
	kGraphPointMode_GreenLineDashed,
	kGraphPointMode_BlueLineDashed,
	kGraphPointMode_OrangeLineDashed,
	kGraphPointMode_CyanLineDashed,
	kGraphPointMode_MagentaLineDashed,
	kGraphPointMode_YellowLineDashed,
	kGraphPointMode_BlackLineDashed,

	kGraphPointMode_RedDot,				//*	these must be in the same color order as above
	kGraphPointMode_GreenDot,
	kGraphPointMode_BlueDot,
	kGraphPointMode_OrangeDot,
	kGraphPointMode_CyanDot,
	kGraphPointMode_MagentaDot,
	kGraphPointMode_YellowDot,
	kGraphPointMode_BlackDot,

	kGraphPointMode_Red_X,				//*	these must be in the same color order as above
	kGraphPointMode_Green_X,
	kGraphPointMode_Blue_X,
	kGraphPointMode_Orange_X,
	kGraphPointMode_Cyan_X,
	kGraphPointMode_Magenta_X,
	kGraphPointMode_Yellow_X,
	kGraphPointMode_Black_X,


	kGraphPointMode_DkGreenLine,
	kGraphPointMode_DkGreenDot,
	kGraphPointMode_GreyLine,

	kGraphPointMode_RedDot_Large,				//*	these must be in the same color order as above
	kGraphPointMode_GreenDot_Large,
	kGraphPointMode_BlueDot_Large,
	kGraphPointMode_OrangeDot_Large,
	kGraphPointMode_CyanDot_Large,
	kGraphPointMode_MagentaDot_Large,
	kGraphPointMode_YellowDot_Large,
	kGraphPointMode_BlackDot_Large,


	kGraphPointMode_Last
};


#ifdef __cplusplus
}
#endif

#endif	//	_WEB_GRAPHICS_H_

