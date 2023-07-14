//*****************************************************************************
//*****************************************************************************
//*	Edit History
//*****************************************************************************

#include	<stdio.h>
#include	<string.h>
#include	<fitsio.h>
#include	<stdbool.h>
#include	<stdlib.h>
#include	<unistd.h>

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


//#define	_DEBUG_DISPLAY_MANDLE_

#define		kQmax				3000
#define		kStartColor			1
//#define		kMaxItterations		512
#define		kMaxItterations		512

#define		kMandel_FarLeft		-2.01
#define		kMandel_FarRight	1.0
#define		kMandel_FarTop		-1.2
#define		kMandel_FarBottom	1.2

#define		kImageWidth			1024
#define		kImageHeight		750

int		gLastClickX	=	kImageWidth / 2;
int		gLastClickY	=	kImageHeight / 2;

int		gItteration	=	0;

char	gWindowName[]	=	"Mandlbrot";

uint32_t	gColorTable[kMaxItterations];

//*******************************************************************************
static void InitColorTable(void)
{
int		ii;
int		redValue;
int		grnValue;
int		bluValue;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("RAND_MAX\t=", RAND_MAX);

	for (ii=0; ii<kMaxItterations; ii++)
	{
	#if 0
		redValue	=	ii * 8;
		grnValue	=	0;
		bluValue	=	0;
	#elif 0
		redValue	=	(rand() * rand()) & 0x00ff;
		grnValue	=	(rand() * rand()) & 0x00ff;
		bluValue	=	(rand() * rand()) & 0x00ff;
	#else
		redValue	=	rand() & 0x00ff;
		grnValue	=	rand() & 0x00ff;
		bluValue	=	rand() & 0x00ff;
	#endif
//		printf("%3d\t%3d\t%3d\r\n", redValue, grnValue, bluValue);
		gColorTable[ii]	=	(redValue << 16) + (grnValue << 8) + bluValue;
//		CONSOLE_DEBUG_W_HEX("gColorTable[ii]=", gColorTable[ii]);
	}


	gColorTable[kMaxItterations - 1]	=	0;
}

//*******************************************************************************
static void SetGrayScale(void)
{
int		ii;
int		redValue;
int		grnValue;
int		bluValue;
int		divideValue;


	divideValue	=	kMaxItterations / 256;
	for (ii=0; ii<kMaxItterations; ii++)
	{
		redValue		=	(ii / divideValue) & 0x00ff;
		grnValue		=	(ii / divideValue) & 0x00ff;
		bluValue		=	(ii / divideValue) & 0x00ff;
		gColorTable[ii]	=	(redValue << 16) + (grnValue << 8) + bluValue;
	}


	gColorTable[kMaxItterations - 1]	=	0;
}


//*******************************************************************************
static uint32_t	GetMacCLUT(int color8Bit)
{
int			red;
int			green;
int			blue;
int			redDbl;
int			greenDbl;
int			blueDbl;
double		values[16];
int			which;
int			iii;
uint32_t	rgbColor;
double		colorDbl;


	if (color8Bit < 215)
	{
		// Component-based colors, with RGB values in the range 0...5,
		// stored in reverse order (i.e. color #0 is white, (1.0, 1.0, 1.0)).
		// Note that x == 215 would normally produce black, (0.0, 0.0, 0.0),
		// but the palette deliberately puts that at the end.
		colorDbl	=	color8Bit;
		redDbl		=	(5 - (colorDbl / 36.0))	/ 5.0;
		greenDbl	=	(5 - ((color8Bit / 6) % 6))	/ 5.0;
		blueDbl		=	(5 - (color8Bit % 6))		/ 5.0;

	}
	else if (color8Bit == 255)
	{
		// Special case: black is last.
		redDbl		=	0;
		greenDbl	=	0;
		blueDbl		=	0;
	}
	else
	{
		// Extra shades of "primary" colors: red, green, blue, and grey.
		//	values = (0...15).reversed()
		//				.filter { $0 % 3 != 0 }
		//				.map { Double($0) / 15.0 }
		//assert(values.count == 10)
		for (iii=0; iii<16; iii++)
		{
			values[iii]	=	15 - iii;
			values[iii]	=	iii;
		}
		which	=	((color8Bit - 215) % 10);
		switch ((color8Bit - 215) / 10)
		{
			case 0:
				redDbl		=	values[which];
				greenDbl	=	0;
				blueDbl		=	0;
				break;
			case 1:
				redDbl		=	0;
				greenDbl	=	values[which];
				blueDbl		=	0;
				break;

			case 2:
				redDbl		=	0;
				greenDbl	=	0;
				blueDbl		=	values[which];
				break;

			case 3:
				redDbl		=	values[which];
				greenDbl	=	values[which];
				blueDbl		=	values[which];
				break;

			default:
				break;
		}
	}
	red			=	255 - (redDbl * 42.6666);
	green		=	255 - (greenDbl * 42.6666);
	blue		=	255 - (blueDbl * 42.6666);

	red			=	red & 0x00ff;
	green		=	green & 0x00ff;
	blue		=	blue & 0x00ff;

	rgbColor	=	(red << 16) + (green << 8) + blue;
	CONSOLE_DEBUG_W_HEX("rgbColor=", rgbColor);
	return(rgbColor);
}


//*******************************************************************************
uint32_t	clut8ColorComponents(int x)

//	-> (red: Double, green: Double, blue: Double)
{
int			red;
int			green;
int			blue;
int			which;
int			values[16];
int			iii;
uint32_t	resultColor;
#define	Double(x) (1.0 * x)

	if (x < 215)
	{
		// Component-based colors, with RGB values in the range 0...5,
		// stored in reverse order (i.e. color #0 is white, (1.0, 1.0, 1.0)).
		// Note that x == 215 would normally produce black, (0.0, 0.0, 0.0),
		// but the palette deliberately puts that at the end.
		red		=	Double(5 - (x / 36))	/ 5.0;
		green	=	Double(5 - (x / 6 % 6)) / 5.0;
		blue	=	Double(5 - (x % 6))		/ 5.0;
	//	return (red: red, green: green, blue: blue)

	}
	else if (x == 255
	{
		// Special case: black is last.
		red		=	0;
		green	=	0;
		blue	=	0;
	//	return (red: 0.0, green: 0.0, blue: 0.0)
	}
	else
	{
		// Extra shades of "primary" colors: red, green, blue, and grey.
//		let values = (0...15).reversed()
//					 .filter { $0 % 3 != 0 }
//					 .map { Double($0) / 15.0 }
//		assert(values.count == 10)
		for (iii= 0; iii<15; iii++)
		{
			values[iii]	=	15 -iii;
		}
		which	=	(x - 215) % 10;
		switch ((x - 215) / 10)
		{
			case 0:		//return (red: values[which], green: 0.0, blue: 0.0)
				red		=	values[which];
				green	=	0;
				blue	=	0;
				break;

			case 1:		//return (red: 0.0, green: values[which], blue: 0.0)
				red		=	0;
				green	=	values[which];
				blue	=	0;
				break;

			case 2:		//return (red: 0.0, green: 0.0, blue: values[which])
				red		=	0;
				green	=	0;
				blue	=	values[which];
				break;
			case 3:		//return (red:   values[which],
						//green: values[which],
						//blue:  values[which])
				red		=	values[which];
				green	=	0;
				blue	=	0;
				break;

//			default: fatalError("x must be out of range")
		}
	}
	resultColor	=	red << 16;
	resultColor	+=	green << 8;
	resultColor	+=	blue;
	return(resultColor);
}

//*******************************************************************************
static void	SetMacColorTable(void)
{
int	iii;

	for (iii=0; iii<kMaxItterations; iii++)
	{
//		gColorTable[ii]	=	GetMacCLUT(iii & 0x00ff);
		gColorTable[ii]	=	clut8ColorComponents(iii & 0x00ff);
	}
	gColorTable[kMaxItterations - 1]	=	0;

}


//*******************************************************************************
static void SetGrayScale2(void)
{
int		ii;
int		redValue;
int		grnValue;
int		bluValue;
int		divideValue;

	divideValue	=	kMaxItterations / 256;

	for (ii=0; ii<kMaxItterations; ii++)
	{
		redValue	=	(ii / divideValue) & 0x00ff;
		grnValue	=	(ii /4 ) & 0x00ff;
		bluValue	=	(ii /8 ) & 0x00ff;
		gColorTable[ii]	=	(redValue << 16) + (grnValue << 8) + bluValue;
	}


	gColorTable[kMaxItterations - 1]	=	0;
}


//*******************************************************************************
static int GetMaxX(IplImage	*openCV_Image)
{
	return(openCV_Image->width);
}
//*******************************************************************************
static int GetMaxY(IplImage	*openCV_Image)
{
	return(openCV_Image->height);
}

//*******************************************************************************
static void	SetMandelbrotPixel(IplImage	*openCV_Image, int color, int xLocation, int yLocation)
{
unsigned char	*imgDataPtr;
int				myWidth;
int				myHeight;
int				myStride;
int				pixelIndex;
int				myColorIdx;
long			myRGBcolor;

//	SetColor(RGB565CONVERT(( (255 - color) & 0xe0), (((255 - color) << 3) & 0xe0), (((255 - color) << 6) & 0xe0)));
//	PutPixel(xLocation, yLocation);

	imgDataPtr				=	(unsigned char	*)openCV_Image->imageData;
	myWidth					=	openCV_Image->width;
	myHeight				=	openCV_Image->height;
	myStride				=	openCV_Image->widthStep;

//	CONSOLE_DEBUG_W_NUM("myWidth\t=", myWidth);
//	CONSOLE_DEBUG_W_NUM("myStride\t=", myStride);

//	CONSOLE_DEBUG_W_NUM("nChannels\t=", openCV_Image->nChannels);

	if (openCV_Image->nChannels == 1)
	{
		pixelIndex				=	(yLocation * myStride) + xLocation;
		imgDataPtr[pixelIndex]	=	color;
	}
	else if (openCV_Image->nChannels == 3)
	{
		pixelIndex				=	(yLocation * myStride) + (xLocation * 3);

		myColorIdx	=	color;
		if (myColorIdx >= kMaxItterations)
		{
			myColorIdx	=	kMaxItterations - 1;
		}
		myRGBcolor	=	gColorTable[myColorIdx];

		imgDataPtr[pixelIndex + 2]	=	(myRGBcolor >> 16) & 0x00ff;
		imgDataPtr[pixelIndex + 1]	=	(myRGBcolor >> 8) & 0x00ff;
		imgDataPtr[pixelIndex + 0]	=	(myRGBcolor) & 0x00ff;
	}


#ifdef _DEBUG_DISPLAY_MANDLE_
	if (color > 254)
	{
		printf(" ");
	}
	else
	{
		printf("%c", (char) 0x20 + (color / 3));
	}
#endif
}



#define	_USE_STAGGERED_
//*******************************************************************************
void	DoMandelBrot(	IplImage	*openCV_Image,
						double		XMin,
						double		XMax,
						double		YMin,
						double		YMax
						)
{
double		X, Y, Xsquare, Ysquare;
double		QQQtemp;
double		QQQ[kQmax + 10];
double 		PP, deltaP, deltaQ;

long		color;
int			row, col;
int			maxcol;
int			maxrow;
bool		keepGoing;
int			deltaColumn;
int			iii;
int			keyPressed;
#ifdef _USE_STAGGERED_
	int			startOffset[]	=	{0,  16, 8,  4, 2, 1, 3, 0, 0, 0, 0};
	int			columnSteps[]	=	{32, 32, 16, 8, 4, 4, 4, 0, 0, 0, 0};
#else
	int			startOffset[]	=	{0,  16, 8,  4, 2, 1, 3, 0, 0, 0, 0};
	int			columnSteps[]	=	{1, 0, 16, 8, 4, 4, 4, 0, 0, 0, 0};
#endif

//	CONSOLE_DEBUG(__FUNCTION__);

	maxcol	=	GetMaxX(openCV_Image);
	maxrow	=	GetMaxY(openCV_Image);


	deltaP	=	(XMax - XMin) / (maxcol);
	deltaQ	=	(YMax - YMin) / (maxrow);

	QQQ[0]	=	YMin;
	for (row=1; row<=maxrow; row++)
	{
		QQQ[row]	=	QQQ[row-1] + deltaQ;
	}

	iii			=	0;
	keepGoing	=	true;
	while ((columnSteps[iii] > 0) && keepGoing)
	{
		deltaColumn	=	columnSteps[iii];
		for (col=startOffset[iii]; col < maxcol; col += deltaColumn)
		{
			PP	=	XMin + (deltaP * col);

			for (row=0; row < maxrow; row++)
			{
				X		=	Y	=	Xsquare	=	Ysquare	=	0.0;
				color	=	kStartColor;
				QQQtemp	=	QQQ[row];
				while ((color < kMaxItterations)  && ((Xsquare + Ysquare) < 4))
				{
					Xsquare =	X * X;
					Ysquare =	Y * Y;
					Y		*=	X;
					Y		+=	Y + QQQtemp;
					X		=	Xsquare - Ysquare + PP;
					color++;
				}

				SetMandelbrotPixel(openCV_Image, color, col, row);

			}
		#ifdef _DEBUG_DISPLAY_MANDLE_
			printf("\r\n");
		#endif
		//	cvShowImage(gWindowName, openCV_Image);
		//	keyPressed	=	cvWaitKey(10);

		}
		iii++;
		cvShowImage(gWindowName, openCV_Image);
		keyPressed	=	cvWaitKey(5);
	}
	gItteration++;

}

#define		kMandel_FarLeft		-2.01
#define		kMandel_FarRight	1.0
#define		kMandel_FarTop		-1.2
#define		kMandel_FarBottom	1.2
#define		kMandel_Width		(kMandel_FarRight - kMandel_FarLeft)
#define		kMandel_Height		(kMandel_FarBottom - kMandel_FarTop)

double		gMandelCenterX	=	kMandel_FarLeft + (kMandel_Width / 2);
double		gMandelCenterY	=	0;
double		gMandleWidth	=	kMandel_Width;
double		gMandleHeight	=	kMandel_Height;

double		gXmin;
double		gXmax;
double		gYmin;
double		gYmax;

//*****************************************************************************
void	CalcNewBoundries(const int newImgCenterX, const int newImgCenterY, double magFactor)
{
double	unitsPerPixelX;
double	unitsPerPixelY;
double	magnifcation;


	gLastClickX	=	newImgCenterX;
	gLastClickY	=	newImgCenterY;

	unitsPerPixelX	=	gMandleWidth / kImageWidth;
	unitsPerPixelY	=	gMandleHeight / kImageHeight;

	gMandelCenterX	=	gXmin + (newImgCenterX * unitsPerPixelX);
	gMandelCenterY	=	gYmin + (newImgCenterY * unitsPerPixelY);

	gMandleWidth	=	gMandleWidth / magFactor;
	gMandleHeight	=	gMandleHeight / magFactor;

	gXmin	=	gMandelCenterX - (gMandleWidth / 2);
	gXmax	=	gMandelCenterX + (gMandleWidth / 2);
	gYmin	=	gMandelCenterY - (gMandleHeight / 2);
	gYmax	=	gMandelCenterY + (gMandleHeight / 2);

	magnifcation	=	kMandel_Width / gMandleWidth;

	CONSOLE_DEBUG_W_DBL("magnifcation\t=", magnifcation);

}

//*****************************************************************************
void	ResetToOrigin(void)
{
	gMandelCenterX	=	kMandel_FarLeft + (kMandel_Width / 2);
	gMandelCenterY	=	0;
	gMandleWidth	=	kMandel_Width;
	gMandleHeight	=	kMandel_Height;
	gXmin			=	kMandel_FarLeft;
	gXmax			=	kMandel_FarRight;
	gYmin			=	kMandel_FarTop;
	gYmax			=	kMandel_FarBottom;
}

//*****************************************************************************
void	LiveWindowMouseCallbac(int event, int x, int y, int flags, void* param)
{
	switch(event)
	{
		case CV_EVENT_MOUSEMOVE:
			break;

		case CV_EVENT_LBUTTONDOWN:
		//	CONSOLE_DEBUG("CV_EVENT_LBUTTONDOWN");
			break;

		case CV_EVENT_RBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDOWN");
			break;

		case CV_EVENT_MBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDOWN");
			break;

		case CV_EVENT_LBUTTONUP:
		//	CONSOLE_DEBUG("CV_EVENT_LBUTTONUP");
			break;

		case CV_EVENT_RBUTTONUP:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONUP");
			break;

		case CV_EVENT_MBUTTONUP:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONUP");
			break;

		case CV_EVENT_LBUTTONDBLCLK:
		//	CONSOLE_DEBUG("CV_EVENT_LBUTTONDBLCLK");
			CalcNewBoundries(x, y, 2.0);
			DoMandelBrot((IplImage *)param, gXmin, gXmax, gYmin, gYmax);

			break;

		case CV_EVENT_RBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
			break;

		case CV_EVENT_MBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDBLCLK");
			break;
	}
}


//*****************************************************************************
void	RunMandlebrotSequence(IplImage	*openCV_Image)
{
double		XMax, YMax, XMin, YMin;
double		mandelWidth;
double		mandelStep;
int			keyPressed;
bool		keepGoing;

	XMin	=	-2.00001;
	XMax	=	XMin + 0.0002;
	YMin	=	-0.000075;
	YMax	=	 0.000075;

	mandelWidth	=	XMax - XMin;
	mandelStep	=	(mandelWidth / 6) * 5;
	keepGoing	=	true;
	CONSOLE_DEBUG_W_DBL("mandelWidth\t=", mandelWidth);
	CONSOLE_DEBUG_W_DBL("mandelStep\t=", mandelStep);

	while (keepGoing && (XMin < -1.7))
	{
		CONSOLE_DEBUG_W_DBL("XMin\t=", XMin);
		DoMandelBrot(openCV_Image, XMin, XMax, YMin, YMax);

	#if 1
		XMin		+=	mandelWidth / 5;
		XMax		=	XMin + 0.0002;
		keyPressed	=	cvWaitKey(50);
		switch(keyPressed & 0x07f)
		{
			case 0x20:
				sleep(3);
				break;

			case 'q':
				keepGoing	=	false;
				break;

			case '-':
				XMin		-=	mandelWidth;
				XMax		=	XMin + 0.0002;
				break;

		}
	#else
		printf("Hit any key to continue\r\n");
		keyPressed	=	cvWaitKey(0);
		switch(keyPressed & 0x0ff)
		{
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				CONSOLE_DEBUG("digit");
				XMin	+=	mandelStep * (keyPressed & 0x0f);
				XMax	+=	mandelStep * (keyPressed & 0x0f);
				break;

			case '0':
				CONSOLE_DEBUG("x 10");
				XMin	+=	(mandelStep * 10);
				XMax	+=	(mandelStep * 10);
				break;

			case '/':
			case '+':
				CONSOLE_DEBUG("x 100");
				XMin	+=	(mandelStep * 100);
				XMax	+=	(mandelStep * 100);
				break;


			default:
				CONSOLE_DEBUG("default");
				XMin	+=	mandelStep;
				XMax	+=	mandelStep;
				break;
		}
	#endif
	}

}

//*****************************************************************************
int main(int argc, char *argv[])
{
IplImage	*openCV_Image;
double		XMax, YMax, XMin, YMin;
int			keyPressed;
bool		keepGoing;

	CONSOLE_DEBUG(__FUNCTION__);

	InitColorTable();
	ResetToOrigin();

	XMin		=	kMandel_FarLeft;
	XMax		=	kMandel_FarRight;
	YMin		=	kMandel_FarTop;
	YMax		=	kMandel_FarBottom;

	cvNamedWindow(	gWindowName,
				//	(CV_WINDOW_NORMAL)
				//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
					(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
				//	(CV_WINDOW_AUTOSIZE)
					);


	openCV_Image	=	cvCreateImage(cvSize(kImageWidth, kImageHeight), IPL_DEPTH_8U, 3);

	cvSetMouseCallback( gWindowName,
						LiveWindowMouseCallbac,
						(void *)openCV_Image);

//	DoMandelBrot(openCV_Image, XMin, XMax, YMin, YMax);
	keepGoing	=	true;
	while (keepGoing)
	{
		DoMandelBrot(openCV_Image, gXmin, gXmax, gYmin, gYmax);
		keyPressed	=	cvWaitKey(0);
		switch(keyPressed & 0x07f)
		{
			case 'c':
				InitColorTable();
				DoMandelBrot(openCV_Image, gXmin, gXmax, gYmin, gYmax);
				break;

			case 'g':
				SetGrayScale();
				DoMandelBrot(openCV_Image, gXmin, gXmax, gYmin, gYmax);
				break;

			case 'h':
				SetGrayScale2();
				DoMandelBrot(openCV_Image, gXmin, gXmax, gYmin, gYmax);
				break;

			case 'm':
				SetMacColorTable();
				DoMandelBrot(openCV_Image, gXmin, gXmax, gYmin, gYmax);
				break;

			case 'q':
				keepGoing	=	false;
				break;

			case 'o':
				CalcNewBoundries(gLastClickX, gLastClickY, 0.5);
				break;

			case 'r':
				ResetToOrigin();
				break;

			default:
				break;
		}

	}

#if 0
	for (ii=0; ii<5; ii++)
	{
		switch(ii)
		{
			case 0:
				XMin	=	kMandel_FarLeft;
				XMax	=	kMandel_FarRight;
				YMin	=	kMandel_FarTop;
				YMax	=	kMandel_FarBottom;
				break;

			case 1:
				XMin	=	-1.7018;
				XMax	=	-1.7016;
				YMin	=	-0.000075;
				YMax	=	 0.000075;
				break;

			case 2:
				XMin	=	-0.197272275682447305;
				XMax	=	-0.196537412401197305;
				YMin	=	-0.67495234375;
				YMax	=	-0.67436640625;
				break;

			case 3:
				XMin	=	-1.796176666666689;
				XMax	=	XMin + 0.0002;
				YMin	=	-0.000075;
				YMax	=	 0.000075;
				break;

			case 4:
				XMin	=	-1.789343333333357;
				XMax	=	XMin + 0.0002;
				YMin	=	-0.000075;
				YMax	=	 0.000075;
				break;


		}
		mandelWidth	=	XMax - XMin;
		CONSOLE_DEBUG_W_DBL("mandelWidth\t=", mandelWidth);
		DoMandelBrot(openCV_Image, XMin, XMax, YMin, YMax);

//		printf("Hit any key to continue\r\n");
		keyPressed	=	cvWaitKey(4000);
	}
#endif

//	RunMandlebrotSequence(openCV_Image);

	return(0);
}
