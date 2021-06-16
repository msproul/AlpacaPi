//*****************************************************************************
//*	rgb Merge
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 25,	2020	<MLS> Started on rgbmerge.cpp
//*	Apr  9,	2020	<MLS> Now works with 8 bit images
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<stdbool.h>

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include	"fits_opencv.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

int	gTranslationMap8bit[256];
int	gHistogram16bit[1 << 16];
int	gTranslationMap16bit[1 << 16];

int	gPixelWidth		=	0;
int	gPixelHeight	=	0;
int	gPixelDepth		=	0;

//**************************************************************************************
static void	InitTranisitionMap(void)
{
int			iii;
uint32_t	myTranslationValue;
double		xValue;
double		radianValue;

	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<256; iii++)
	{
		gTranslationMap8bit[iii]	=	iii*16;
		if (gTranslationMap8bit[iii] > 255)
		{
			gTranslationMap8bit[iii]	=	255;
		}
	}
	iii	=	0;
	while (iii<65535)
	{
		myTranslationValue	=	(iii*32);
		if (myTranslationValue > 65535)
		{
			myTranslationValue	=	65535;
		}

		gTranslationMap16bit[iii]	=	myTranslationValue;
		iii++;
	}
#if 1
	iii	=	0;
	while (iii<=65535)
	{
		xValue				=	(16.0 * iii);
		if (xValue <  65536.0)
		{
			xValue	=	xValue / 65536.0;
		}
		else
		{
			xValue	=	1.0;
		}
		radianValue			=	xValue * (M_PI / 2);
		myTranslationValue	=	sin(radianValue) * 65535;
		if (myTranslationValue > 65535)
		{
			myTranslationValue	=	65535;
		}

		gTranslationMap16bit[iii]	=	myTranslationValue;
		iii++;
	}
#endif
}

//**************************************************************************************
class FitsImage
{
	public:

		//
		// Construction
		//
					FitsImage(const char *filePath, int whichColor);
		virtual		~FitsImage(void);
			void	Adjust8bitImage(void);
			void	Adjust16bitImage(void);
			void	Adjust16bitImageLinear(void);

			void	CopyIntoColorPlane(IplImage	*colorCV_Image);

			int			cWhichColor;	//	0=red, 1=green, 2=blue
			char		cFileName[64];
			IplImage	*cOpenCV_Image;
			IplImage	*cSmallCV_Image;
			int			cXoffset;			//*	these are the offsets for aligning the image
			int			cYoffset;


};

//*****************************************************************************
enum
{
	kColorRed	=	0,
	kColorGrn,
	kColorBlu
};

//*****************************************************************************
FitsImage::FitsImage(const char *filePath, int whichColor)
{
int		newWidth;
int		newHeight;

	CONSOLE_DEBUG(__FUNCTION__);
	cWhichColor	=	whichColor;
	cYoffset	=	0;
	switch(whichColor)
	{
//		case kColorRed:	cXoffset	=	0;	break;
//		case kColorGrn:	cXoffset	=	20;	break;
//		case kColorBlu:	cXoffset	=	40;	break;
	}
	cXoffset	=	0;

	strcpy(cFileName, filePath);
	cOpenCV_Image	=	ReadImageIntoOpenCVimage(filePath);
	if (cOpenCV_Image != NULL)
	{
		//*	save for the color image creation
		gPixelWidth		=	cOpenCV_Image->width;
		gPixelHeight	=	cOpenCV_Image->height;
		gPixelDepth		=	cOpenCV_Image->depth;

		if (cOpenCV_Image->depth == 16)
		{
		//	Adjust16bitImage();
			Adjust16bitImageLinear();
		}
		else
		{
			Adjust8bitImage();
		}
		CONSOLE_DEBUG(__FUNCTION__);

		newWidth		=	cOpenCV_Image->width / 4;
		newHeight		=	cOpenCV_Image->height / 4;
		if (cOpenCV_Image->depth == 16)
		{
			cSmallCV_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
		}
		else
		{
			cSmallCV_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
		}
		CONSOLE_DEBUG(__FUNCTION__);
		cvResize(cOpenCV_Image, cSmallCV_Image, CV_INTER_LINEAR);
		cvNamedWindow(	filePath,
					//	(CV_WINDOW_NORMAL)
					//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
					//	(CV_WINDOW_AUTOSIZE)
						);

		cvShowImage(cFileName, cSmallCV_Image);
		CONSOLE_DEBUG(__FUNCTION__);
	}
		CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FitsImage::~FitsImage( void )
{


}

//**************************************************************************************
void FitsImage::Adjust8bitImage(void)
{
int			myWidth;
int			myHeight;
int			pixelCount;
int			ii;
uint8_t		*myBytePtr;
uint8_t		myPixelValue;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cOpenCV_Image != NULL)
	{
		myWidth			=	cOpenCV_Image->width;
		myHeight		=	cOpenCV_Image->height;
		pixelCount		=	myWidth * myHeight;
		CONSOLE_DEBUG_W_NUM("myWidth\t=", myWidth);
		CONSOLE_DEBUG_W_NUM("myHeight\t=", myHeight);
		CONSOLE_DEBUG_W_NUM("pixelCount\t=", pixelCount);
		myBytePtr		=	(uint8_t *)cOpenCV_Image->imageData;
		if (myBytePtr != NULL)
		{
			for (ii=0; ii<pixelCount; ii++)
			{
				myPixelValue	=	myBytePtr[ii] & 0x00ff;
				myBytePtr[ii]	=	gTranslationMap8bit[myPixelValue];
			}
		}
		else
		{
			CONSOLE_DEBUG("myBytePtr is NULL");
		}
	}
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void FitsImage::Adjust16bitImageLinear(void)
{
int			myWidth;
int			myHeight;
int			pixelCount;
int			ii;
uint16_t	*myShortPtr;
int			myPixelValue;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cOpenCV_Image != NULL)
	{
		myWidth			=	cOpenCV_Image->width;
		myHeight		=	cOpenCV_Image->height;
		pixelCount		=	myWidth * myHeight;
		myShortPtr		=	(uint16_t *)cOpenCV_Image->imageData;
		for (ii=0; ii<pixelCount; ii++)
		{
			myPixelValue	=	myShortPtr[ii] & 0x00ffff;
			myShortPtr[ii]	=	gTranslationMap16bit[myPixelValue];
		}
	}
}

//*****************************************************************************
void FitsImage::Adjust16bitImage(void)
{
int			myWidth;
int			myHeight;
int			pixelCount;
int			ii;
uint16_t	*myShortPtr;
int			pixelValue16;
int			peakPixelValue;
int			peakPixelIdx;
int			newPixValue;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cOpenCV_Image != NULL)
	{

		for (ii=0; ii<65536; ii++)
		{
			gHistogram16bit[ii]	=	0;
		}
		myWidth			=	cOpenCV_Image->width;
		myHeight		=	cOpenCV_Image->height;
		pixelCount		=	myWidth * myHeight;
		myShortPtr		=	(uint16_t *)cOpenCV_Image->imageData;
		for (ii=0; ii<pixelCount; ii++)
		{
			pixelValue16		=	myShortPtr[ii] & 0x00ffff;
			gHistogram16bit[pixelValue16]++;
		}
		//*	now find the peak value
		peakPixelValue	=	0;
		peakPixelIdx	=	0;
		for (ii=0; ii<65536; ii++)
		{
			if (gHistogram16bit[ii] > peakPixelValue)
			{
				peakPixelValue	=	gHistogram16bit[ii];
				peakPixelIdx	=	ii;
			}
		}
		//*	now go through and set every thing below that peak to 0
		for (ii=0; ii<pixelCount; ii++)
		{
			pixelValue16		=	myShortPtr[ii] & 0x00ffff;
			if (pixelValue16 <= peakPixelIdx)
			{
				myShortPtr[ii]	=	0;
			}
		}
		for (ii=0; ii<pixelCount; ii++)
		{
			pixelValue16		=	myShortPtr[ii] & 0x00ffff;
			newPixValue			=	pixelValue16 * 3;
			if (newPixValue > 0x0ffff)
			{
				newPixValue		=	0x0ffff;
			}
			myShortPtr[ii]	=	newPixValue;
		}

	}
}

//*****************************************************************************
void FitsImage::CopyIntoColorPlane(IplImage	*colorCV_Image)
{
int			myWidth;
int			myHeight;
int			myDepth;
int			pixelCount;
int			iii;
int			jjj;
uint16_t	*myShortPtr;
uint16_t	*colorShortPtr;
uint8_t		*myBytePtr;
uint8_t		*colorBytePtr;
int			colorWidth;
int			colorHeight;
int			colorDepth;
int			ccc;
int			myPixelValue16;
int			myPixelValue8;
int			myPixIdx;
int			clrPixIdx;

	CONSOLE_DEBUG(__FUNCTION__);

	myWidth			=	cOpenCV_Image->width;
	myHeight		=	cOpenCV_Image->height;
	myDepth			=	cOpenCV_Image->depth;

	colorWidth		=	colorCV_Image->width;
	colorHeight		=	colorCV_Image->height;
	colorDepth		=	colorCV_Image->depth;
	CONSOLE_DEBUG_W_NUM("myDepth\t=", myDepth);
	CONSOLE_DEBUG_W_NUM("colorDepth\t=", colorDepth);



	if ((myWidth == colorWidth) && (myHeight == colorHeight) && (myDepth == colorDepth))
	{
		if (myDepth == 8)
		{
			CONSOLE_DEBUG("Processing 8 bit image");
			pixelCount		=	myWidth * myHeight;
	//		CONSOLE_DEBUG_W_NUM("pixelCount\t=", pixelCount);

			myBytePtr		=	(uint8_t *)cOpenCV_Image->imageData;
			colorBytePtr	=	(uint8_t *)colorCV_Image->imageData;
			ccc				=	0;

			//*	this has to be done a row at a time so that we can shift an image
	//		CONSOLE_DEBUG_W_NUM("cOpenCV_Image->widthStep\t=", cOpenCV_Image->widthStep);
	//		CONSOLE_DEBUG_W_NUM("colorCV_Image->widthStep\t=", colorCV_Image->widthStep);

			//*	jjj is which row (Y directon)
			for (jjj=0; jjj<myHeight; jjj++)
			{
				if ((jjj + cYoffset) >= 0)
				{
					//*	iii is which colum (X direction)
					for (iii=0; iii<myWidth; iii++)
					{
						clrPixIdx			=	(jjj * colorCV_Image->widthStep) + iii;
						myPixIdx			=	((jjj + cYoffset) * myWidth) + (iii + cXoffset);
						myPixelValue8		=	myBytePtr[myPixIdx] & 0x00ff;
						switch(cWhichColor)
						{
							//*	openCV is GBR instead of RBG
							case kColorRed:
								colorBytePtr[ccc + 2]	=	myPixelValue8;
								break;

							case kColorGrn:
								colorBytePtr[ccc + 1]	=	myPixelValue8;
								break;

							case kColorBlu:
								colorBytePtr[ccc + 0]	=	myPixelValue8;
								break;
						}
						ccc	+=	3;
					}
				}
				else
				{
					ccc	+=	3 * myWidth;
				}
			}
		}
		else
		{
			CONSOLE_DEBUG("Processing 16 bit image");
			pixelCount		=	myWidth * myHeight;
	//		CONSOLE_DEBUG_W_NUM("pixelCount\t=", pixelCount);

			myShortPtr		=	(uint16_t *)cOpenCV_Image->imageData;
			colorShortPtr	=	(uint16_t *)colorCV_Image->imageData;
			ccc				=	0;

			//*	this has to be done a row at a time so that we can shift an image
	//		CONSOLE_DEBUG_W_NUM("cOpenCV_Image->widthStep\t=", cOpenCV_Image->widthStep);
	//		CONSOLE_DEBUG_W_NUM("colorCV_Image->widthStep\t=", colorCV_Image->widthStep);

			//*	jjj is which row (Y directon)
			for (jjj=0; jjj<myHeight; jjj++)
			{
				if ((jjj + cYoffset) >= 0)
				{
					//*	iii is which colum (X direction)
					for (iii=0; iii<myWidth; iii++)
					{
						clrPixIdx			=	(jjj * colorCV_Image->widthStep) + iii;
						myPixIdx			=	((jjj + cYoffset) * myWidth) + (iii + cXoffset);
						myPixelValue16		=	myShortPtr[myPixIdx] & 0x00ffff;
						switch(cWhichColor)
						{
							//*	openCV is GBR instead of RBG
							case kColorRed:
								colorShortPtr[ccc + 2]	=	myPixelValue16;
								break;

							case kColorGrn:
								colorShortPtr[ccc + 1]	=	myPixelValue16;
								break;

							case kColorBlu:
								colorShortPtr[ccc + 0]	=	myPixelValue16;
								break;
						}
						ccc	+=	3;
					}
				}
				else
				{
					ccc	+=	3 * myWidth;
				}
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Image mis match");
	}
}

//*****************************************************************************
void	SaveImage(IplImage	*colorCV_Image)
{
int			openCVerr;
char		imageFileName[64];
char		imageFilePath[128];
//int		quality[3] = {CV_IMWRITE_PNG_COMPRESSION, 200, 0};
int			quality[3] = {16, 200, 0};

	openCVerr	=	cvSaveImage("color.png", colorCV_Image, quality);
//	openCVerr	=	cvSaveImage("color.jpg", colorCV_Image, quality);

}

//*****************************************************************************
int	HandleKeyDownEvents(void)
{
int				keyPressed;
bool			keepGoing;

//	CONSOLE_DEBUG(__FUNCTION__);
	keyPressed	=	cvWaitKey(0);

	return(keyPressed & 0x07f);


	keepGoing		=	true;
	while (keepGoing)
	{
		keyPressed	=	cvWaitKey(0);
		CONSOLE_DEBUG_W_HEX("keyPressed=", keyPressed);
		switch(keyPressed & 0x07f)
		{
			case 'r':
			case 'g':
			case 'b':
			case 'q':
				keepGoing	=	false;
				break;

			case 'n':
				break;

		}
	}
	return(keyPressed);
}


//*****************************************************************************
int main(int argc, char *argv[])
{
int				ii;
int				fileIdx;
FitsImage		*fitsImageRed;
FitsImage		*fitsImageGrn;
FitsImage		*fitsImageBlu;
IplImage		*colorCV_Image;
int				keyPressed;
char			myWindowName[128];
bool			createWindow;
bool			keepGoing;
char			firstChar;
char			argChar;
bool			keepLooping;
bool			fileIsFits;
int				imgCnt;
char			colorWindowName[]	=	"color";
char			currentPlane	=	'g';
bool	updateFlag;


	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("argc\t\t=", argc);
	InitTranisitionMap();

	if (argc < 3)
	{
		CONSOLE_DEBUG("Must specify 3 files");
		exit(0);
	}

	fitsImageRed	=	NULL;
	fitsImageGrn	=	NULL;
	fitsImageBlu	=	NULL;

	//*	check for cmd line parameters
	for (ii=1; ii<argc; ii++)
	{
		firstChar	=	argv[ii][0];
		argChar		=	argv[ii][1];
		if (firstChar == '-')
		{
			switch(argChar)
			{
				case 'a':
					break;

				case 'l':
					keepLooping	=	true;
					break;

			}
		}
	}

	//*	there should be 3 files specified

	createWindow	=	true;
	keepGoing		=	true;
	fileIdx			=	1;
	imgCnt			=	0;
	while ((fileIdx < argc) && (imgCnt < 3))
	{
		CONSOLE_DEBUG(__FUNCTION__);
		if (strstr(argv[fileIdx], "fits") != NULL)
		{
			fileIsFits	=	true;
			switch(imgCnt)
			{
				case 0:
					fitsImageRed	=	new FitsImage(argv[fileIdx], kColorRed);
					break;

				case 1:
					fitsImageGrn	=	new FitsImage(argv[fileIdx], kColorGrn);
					break;

				case 2:
					fitsImageBlu	=	new FitsImage(argv[fileIdx], kColorBlu);
					break;
			}
			imgCnt++;
		}
		else
		{
			fileIsFits	=	false;
		}


		fileIdx++;
	}
	CONSOLE_DEBUG(__FUNCTION__);

	//*	now create the merge image
	if (gPixelDepth == 16)
	{
		CONSOLE_DEBUG("16 bit color image");
		colorCV_Image	=	cvCreateImage(cvSize(gPixelWidth, gPixelHeight), IPL_DEPTH_16U, 3);
	}
	else
	{
		CONSOLE_DEBUG("8 bit color image");
		colorCV_Image	=	cvCreateImage(cvSize(gPixelWidth, gPixelHeight), IPL_DEPTH_8U, 3);
	}
	if (colorCV_Image != NULL)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		fitsImageRed->CopyIntoColorPlane(colorCV_Image);
		fitsImageGrn->CopyIntoColorPlane(colorCV_Image);
		fitsImageBlu->CopyIntoColorPlane(colorCV_Image);


		cvNamedWindow(	colorWindowName,
					//	(CV_WINDOW_NORMAL)
					//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
					//	(CV_WINDOW_AUTOSIZE)
						);

		cvShowImage(colorWindowName, colorCV_Image);
	}

	while (keepGoing)
	{
		keyPressed	=	HandleKeyDownEvents();
		if (keyPressed > 0)
		{
			switch(keyPressed & 0x07f)
			{
				case 'r':
				case 'g':
				case 'b':
					currentPlane	=	keyPressed & 0x07f;
					break;

				case '4':
					CONSOLE_DEBUG("4");
					updateFlag	=	true;
					switch (currentPlane)
					{
						case 'r':
							fitsImageRed->cXoffset++;
							break;
						case 'g':
							fitsImageGrn->cXoffset++;
							break;
						case 'b':
							fitsImageBlu->cXoffset++;
							break;
					}
					break;

				case '6':
					CONSOLE_DEBUG("6");
					updateFlag	=	true;
					switch (currentPlane)
					{
						case 'r':
							fitsImageRed->cXoffset--;
							break;
						case 'g':
							fitsImageGrn->cXoffset--;
							break;
						case 'b':
							fitsImageBlu->cXoffset--;
							break;
					}
					break;

				case '8':
					CONSOLE_DEBUG("8");
					updateFlag	=	true;
					switch (currentPlane)
					{
						case 'r':
							fitsImageRed->cYoffset++;
							break;
						case 'g':
							fitsImageGrn->cYoffset++;
							break;
						case 'b':
							fitsImageBlu->cYoffset++;
							break;
					}
					break;

				case '2':
					CONSOLE_DEBUG("2");
					updateFlag	=	true;
					switch (currentPlane)
					{
						case 'r':
							fitsImageRed->cYoffset--;
							break;
						case 'g':
							fitsImageGrn->cYoffset--;
							break;
						case 'b':
							fitsImageBlu->cYoffset--;
							break;
					}
					break;

				case 'q':
					keepGoing	=	false;
					break;

				case 's':
					SaveImage(colorCV_Image);
					break;
			}
		}
		if (updateFlag)
		{
//			CONSOLE_DEBUG_W_NUM("fitsImageRed->cXoffset\t=", fitsImageRed->cXoffset);
			CONSOLE_DEBUG_W_NUM("fitsImageGrn->cXoffset\t=", fitsImageGrn->cXoffset);
			CONSOLE_DEBUG_W_NUM("fitsImageBlu->cXoffset\t=", fitsImageBlu->cXoffset);

			CONSOLE_DEBUG_W_NUM("fitsImageGrn->cXoffset\t=", fitsImageGrn->cYoffset);
			CONSOLE_DEBUG_W_NUM("fitsImageBlu->cXoffset\t=", fitsImageBlu->cYoffset);

			switch (currentPlane)
			{
				case 'r':
					fitsImageRed->CopyIntoColorPlane(colorCV_Image);
					break;
				case 'g':
					fitsImageGrn->CopyIntoColorPlane(colorCV_Image);
					break;
				case 'b':
					fitsImageBlu->CopyIntoColorPlane(colorCV_Image);
					break;
			}


			cvShowImage(colorWindowName, colorCV_Image);
			updateFlag	=	false;
		}
	}
	return(0);
}


