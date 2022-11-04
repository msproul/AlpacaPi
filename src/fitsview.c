//*****************************************************************************
//*	FITS image viewer
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan 25,	2020	<MLS> Created fitsview.c
//*	Jan 26,	2020	<MLS> fitsview displaying B/W images
//*	Jan 28,	2020	<MLS> fitsview displaying RGB images
//*	Jan 31,	2020	<MLS> (CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
//*	Jan 31,	2020	<MLS> 	works correctly on Ubunto 16.04LTS
//*	Nov 13,	2021	<MLS> Added cmd 'a' to toggle between automatic and manual advance
//*	Nov 13,	2021	<MLS> Added -t title option
//*	Sep 19,	2022	<MLS> fitsview now working with openCV C++ version
//*	Oct  7,	2022	<MLS> Image scaling for large image display working w/ openCV C++
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<fitsio.h>
#include	<stdbool.h>

#ifdef _USE_OPENCV_CPP_
	#include	"opencv2/opencv.hpp"
	#include	"opencv2/core.hpp"
#else
	#include	"opencv/highgui.h"
	#include	"opencv2/core.hpp"
	#include	"opencv2/highgui/highgui_c.h"
	#include	"opencv2/imgproc/imgproc_c.h"
#endif // _USE_OPENCV_CPP_

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include	"fits_opencv.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#if 0
//*****************************************************************************
void	AdjustImageData(void)
{
	if (bitpix == 16)
	{
	uint16_t	*shortPtr;
	uint32_t	curPixValue;
	uint32_t	minPixValue;
	uint32_t	maxPixValue;
	uint32_t	newPixValue;

		shortPtr	=	(uint16_t *)imageDataPtr;
		minPixValue	=	70000;
		maxPixValue	=	0;
		for (ii=0; ii<imgSize_Pixels; ii++)
		{
			curPixValue		=	shortPtr[ii] & 0x00ffff;
			if (curPixValue < minPixValue)
			{
				minPixValue	=	curPixValue;
			}
			if (curPixValue > maxPixValue)
			{
				maxPixValue	=	curPixValue;
			}
			if (curPixValue < 10000)
			{
				newPixValue		=	4 * curPixValue;
				if (newPixValue > 65535)
				{
					newPixValue	=	65535;
				}
				shortPtr[ii]	=	newPixValue;
			}
		}
		CONSOLE_DEBUG_W_NUM("minPixValue", minPixValue);
		CONSOLE_DEBUG_W_NUM("maxPixValue", maxPixValue);
	}
}
#endif // 0


int	gTranslationMap[1 << 16];
int	gHistogram16bit[1 << 16];

char	gNormalWindowName[]	=	"Normalized";
bool	gCreateNormalWindow	=	true;
bool	gAutomatic			=	false;

#ifndef _USE_OPENCV_CPP_
//*****************************************************************************
#ifdef _USE_OPENCV_CPP_
void	NormalizeImage(cv::Mat *openCV_Image)
#else
void	NormalizeImage(IplImage *openCV_Image)
#endif
{
int			newWidth;
int			newHeight;
int			keyPressed;
double		a	=	1.0;
double		b	=	0.0;
#ifdef _USE_OPENCV_CPP_
	cv::Mat		*adjusted_Image;
	cv::Mat		*normalized_Image;
#else
	IplImage	*adjusted_Image;
	IplImage	*normalized_Image;
#endif


#ifdef _USE_OPENCV_CPP_
	newWidth			=	openCV_Image->cols / 4;
	newHeight			=	openCV_Image->rows / 4;
	adjusted_Image		=	cv::createImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
//	adjusted_Image		=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
	normalized_Image	=	cv::createImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
	cvResize(openCV_Image, adjusted_Image, CV_INTER_LINEAR);
//	cvShowImage(myWindowName, smallCV_Image);
#else

	newWidth			=	openCV_Image->width / 4;
	newHeight			=	openCV_Image->height / 4;
	adjusted_Image		=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
//	adjusted_Image		=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
	normalized_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
	cvResize(openCV_Image, adjusted_Image, CV_INTER_LINEAR);
//	cvShowImage(myWindowName, smallCV_Image);
#endif

	a	=	0.0;
	b	=	3000.0;

	cvNormalize(	adjusted_Image,
					normalized_Image,
					a,					//	CV_DEFAULT(1.)
					b,					//	double b CV_DEFAULT(0.),
					CV_MINMAX,			//	int norm_type CV_DEFAULT(CV_L2),
					NULL);				//	const CvArr* mask CV_DEFAULT(NULL) );
	if (gCreateNormalWindow)
	{
#ifdef _USE_OPENCV_CPP_
		cv::namedWindow(	gNormalWindowName,
							(cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO)
							);
#else
		cvNamedWindow(	gNormalWindowName,
					//	(CV_WINDOW_NORMAL)
					//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
					//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
					//	(CV_WINDOW_AUTOSIZE)
						);
#endif
		gCreateNormalWindow	=	false;
	}
	cvShowImage(gNormalWindowName, normalized_Image);
	keyPressed	=	cvWaitKey(0);

	cvReleaseImage(&adjusted_Image);
	cvReleaseImage(&normalized_Image);

}

//*****************************************************************************
void	Adjust16bitImge(IplImage *openCV_Image)
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

#ifdef _DISPLAY_2ND_WINDOW_
	if (gCreateNormalWindow)
	{
		cvNamedWindow(	gNormalWindowName,
						(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
						);

		gCreateNormalWindow	=	false;
	}
#endif // _DISPLAY_2ND_WINDOW_

	for (ii=0; ii<65536; ii++)
	{
		gHistogram16bit[ii]	=	0;
	}
	myWidth			=	openCV_Image->width;
	myHeight		=	openCV_Image->height;
	pixelCount		=	myWidth * myHeight;
	myShortPtr		=	(uint16_t *)openCV_Image->imageData;
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
	//*	now go through and set everything below that peak to 0
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

#ifdef _DISPLAY_2ND_WINDOW_
	cvShowImage(gNormalWindowName, openCV_Image);
#endif // _DISPLAY_2ND_WINDOW_

//	CONSOLE_DEBUG_W_NUM("peakPixelValue\t=", peakPixelValue);
//	CONSOLE_DEBUG_W_NUM("peakPixelIdx\t=", peakPixelIdx);
//	CONSOLE_DEBUG_W_NUM("peakPixelIdx/256\t=", peakPixelIdx / 256);
}

//*****************************************************************************
void	WriteOutColumnOne(IplImage *openCV_Image)
{
int		xxx;
int		yyy;
char	*pixelPtr;
int		pixelIdx;
int		jjj;
int		quality[3] = {16, 200, 0};
int		openCVerr;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("openCV_Image->widthStep\t=", openCV_Image->widthStep);
	if (openCV_Image != NULL)
	{
		//*	set every 50th line to white
		pixelPtr	=	openCV_Image->imageData;
		yyy			=	50;
		while (yyy < openCV_Image->height)
		{
			CONSOLE_DEBUG_W_NUM("yyy\t=", yyy);
			pixelIdx	=	yyy * openCV_Image->widthStep;
			for (jjj=0; jjj<openCV_Image->width; jjj++)
			{
				pixelPtr[pixelIdx + jjj]	=	0x00ff;
			}

			yyy	+=	50;
		}


		xxx	=	0;
		while (xxx < 100)
		{

			printf("%4d\t", xxx);
			for (jjj=0; jjj<16; jjj++)
			{
				printf("%02X-", pixelPtr[xxx] & 0x00ff);
				if ((jjj % 2) == 1)
				{
					printf("  ");
				}
				xxx++;
			}

			printf("\r\n");
		}

//		for (yyy=0; yyy<101; yyy++)
//		{
//			pixelIdx	=	yyy * openCV_Image->widthStep;
//
//			printf("%4d", yyy);
//			printf("\t%02X", pixelPtr[pixelIdx] & 0x00ff);
//			printf("-%02X",  pixelPtr[pixelIdx + 1] & 0x00ff);
//
//			printf("\t\t%02X", pixelPtr[pixelIdx + 2] & 0x00ff);
//			printf("-%02X",  pixelPtr[pixelIdx + 3] & 0x00ff);
//
//			printf("\t\t%02X", pixelPtr[pixelIdx + 4] & 0x00ff);
//			printf("-%02X",  pixelPtr[pixelIdx + 5] & 0x00ff);
//
//			printf("\r\n");
//
//		}
//++		openCVerr	=	cvSaveImage("striped.png", openCV_Image, quality);
	}
}

#endif // _USE_OPENCV_CPP_


//*****************************************************************************
void	DeleteFileWithExension(const char *fileNameRoot, const char *extenstion)
{
char	deleteFileName[256];
int		retCode;

	strcpy(deleteFileName, fileNameRoot);
	strcat(deleteFileName, extenstion);
	retCode	=	remove(deleteFileName);
	if (retCode == 0)
	{
		CONSOLE_DEBUG_W_STR("File deleted:", deleteFileName)
	}
	else
	{
//		CONSOLE_DEBUG_W_STR("FAILED TO DELETE:", deleteFileName)
	}
}

//*****************************************************************************
void	DeleteFiles(const char *fileName)
{
char			fileNameRoot[256];
int				slen;
int				iii;

	strcpy(fileNameRoot, fileName);
	slen	=	strlen(fileNameRoot);
	iii		=	slen-1;
	while ((fileNameRoot[iii] != '.') && (iii > 0))
	{
		fileNameRoot[iii]	=	0;
		iii--;
	}
	CONSOLE_DEBUG_W_STR("fileNameRoot\t=", fileNameRoot);
	DeleteFileWithExension(fileNameRoot, "fits");
	DeleteFileWithExension(fileNameRoot, "csv");
	DeleteFileWithExension(fileNameRoot, "png");
	DeleteFileWithExension(fileNameRoot, "jpg");
}


//*****************************************************************************
int	HandleKeyDownEvents(	const char	*fileName,
							const char	*windowName,
						#ifdef _USE_OPENCV_CPP_
							cv::Mat		*openCV_Image
						#else
							IplImage	*openCV_Image
						#endif
							)
{
int				keyPressed;
bool			keepGoing;
#ifdef _USE_OPENCV_CPP_
	cv::Scalar		mean;
	cv::Scalar		std_dev;
	cv::Mat			*adjusted_Image;
	cv::Mat			*normalized_Image;
#else
	IplImage		*adjusted_Image;
	IplImage		*normalized_Image;
	CvScalar		mean;
	CvScalar		std_dev;
#endif

//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _USE_OPENCV_CPP_
//?	cv::avgSdv(openCV_Image, &mean, &std_dev, NULL);
#else
	cvAvgSdv(openCV_Image, &mean, &std_dev, NULL);
#endif

//	if ((mean.val[0] < 0.001) && (std_dev.val[0] < 0.001))
//	{
//		CONSOLE_DEBUG("No image");
//	}

	keepGoing		=	true;
	while (keepGoing)
	{
	#ifdef _USE_OPENCV_CPP_
		keyPressed	=	cv::waitKey(0);
	#else
		keyPressed	=	cvWaitKey(0);
	#endif
		switch(keyPressed & 0x07f)
		{
			case 'a':
				keepGoing	=	false;
				break;

			case 'd':
				DeleteFiles(fileName);
				keepGoing	=	false;
				break;


			case ' ':
			case 'q':
				keepGoing	=	false;
				break;

			case 'n':
				break;

#ifndef _USE_OPENCV_CPP_
			case 'z':
				//*	write out the first vertical column
				WriteOutColumnOne(openCV_Image);
				cvShowImage(windowName, openCV_Image);
				break;
#endif // _USE_OPENCV_CPP_
		}
	}
	return(keyPressed);
}


//*****************************************************************************
int main(int argc, char *argv[])
{
int				ii;
int				fileIdx;
int				keyPressed;
char			myWindowName[128];
bool			createWindow;
bool			keepGoing;
char			firstChar;
char			argChar;
bool			keepLooping;
bool			fileIsFits;
#ifdef _USE_OPENCV_CPP_
	cv::Mat			*openCV_Image;
	cv::Mat			*smallCV_Image;
	unsigned int	nChannels;

#else
	IplImage		*openCV_Image;
	IplImage		*smallCV_Image;
#endif

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_LONG("sizeof(gTranslationMap)\t=", sizeof(gTranslationMap));

	strcpy(myWindowName, "fits file");
	gAutomatic	=	false;
	keepLooping	=	false;
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
					gAutomatic	=	true;
					break;

				case 'l':
					keepLooping	=	true;
					break;

				case 't':
					strcpy(myWindowName, argv[ii+1]);
					ii++;
					break;

			}
		}
	}
	createWindow	=	true;
	keepGoing		=	true;
	fileIdx		=	1;
	while ((fileIdx < argc) && keepGoing)
	{
		if (strstr(argv[fileIdx], "fits") != NULL)
		{
			fileIsFits	=	true;
		}
		else
		{
			fileIsFits	=	false;
		}
		smallCV_Image	=	NULL;
		openCV_Image	=	ReadImageIntoOpenCVimage(argv[fileIdx]);
		if (openCV_Image != NULL)
		{
//			CONSOLE_DEBUG(__FUNCTION__);
			if (createWindow)
			{
				CONSOLE_DEBUG("Create Window");
			#ifdef _USE_OPENCV_CPP_
				cv::namedWindow(	myWindowName,
									(cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO)
									);
			#else
				cvNamedWindow(	myWindowName,
							//	(CV_WINDOW_NORMAL)
							//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
							//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
								(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
							//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
							//	(CV_WINDOW_AUTOSIZE)
								);
			#endif
			//	cvMoveWindow(myWindowName, -1025, 100);
				createWindow	=	false;
				CONSOLE_DEBUG(__FUNCTION__);
			}
//			if (fileIsFits && (openCV_Image->depth == 16))
//			{
//			//	CONSOLE_DEBUG("Calling Adjust16bitImge()");
//			//	Adjust16bitImge(openCV_Image);
//			}
//			CONSOLE_DEBUG(__FUNCTION__);

		#ifdef _USE_OPENCV_CPP_
			if (openCV_Image->cols > 2000)
			{
			int	newImgWidth;
			int	newImgHeight;
			int	divideFactor;

				//*	make it fit on the screen
				divideFactor	=	1;
				newImgWidth		=	openCV_Image->cols / divideFactor;
				newImgHeight	=	openCV_Image->rows / divideFactor;
				while (newImgWidth > 2000)
				{
					divideFactor++;
					newImgWidth		=	openCV_Image->cols / divideFactor;
					newImgHeight	=	openCV_Image->rows / divideFactor;
				}
//				CONSOLE_DEBUG_W_NUM("divideFactor\t=",	divideFactor);
//				CONSOLE_DEBUG_W_NUM("newImgWidth\t=",	newImgWidth);

				//*	create the new smaller image
				smallCV_Image		=	new cv::Mat(cv::Size(	newImgWidth,
																newImgHeight),
																CV_8UC3);

				nChannels		=	openCV_Image->step[1];
				if (nChannels == 3)
				{
					cv::resize(	*openCV_Image,
								*smallCV_Image,
								smallCV_Image->size(),
								0,
								0,
								cv::INTER_LINEAR);
				}
				else
				{
					CONSOLE_DEBUG("convert gray scale to color");
					//*	convert gray scale to color
					cv::cvtColor(*openCV_Image, *smallCV_Image, cv::COLOR_GRAY2BGR);
				}
				cv::imshow(myWindowName, *smallCV_Image);
			}
			else
			{
				cv::imshow(myWindowName, *openCV_Image);
			}
		#else
			if (openCV_Image->width > 2000)
			{
			int	newWidth;
			int	newHeight;

				CONSOLE_DEBUG(__FUNCTION__);

				newWidth		=	openCV_Image->width / 4;
				newHeight		=	openCV_Image->height / 4;
				if (openCV_Image->depth == 16)
				{
					smallCV_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_16U, 1);
				}
				else
				{
					if (openCV_Image->nChannels == 3)
					{
						smallCV_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 3);
					}
					else
					{
						smallCV_Image	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
					}
				}
				cvResize(openCV_Image, smallCV_Image, CV_INTER_LINEAR);
				cvShowImage(myWindowName, smallCV_Image);
			}
			else
			{
				cvShowImage(myWindowName, openCV_Image);
			}
		#endif
//			CONSOLE_DEBUG(__FUNCTION__);

			if (gAutomatic)
			{
			#ifdef _USE_OPENCV_CPP_
				keyPressed	=	cv::waitKey(3000);
			#else
				keyPressed	=	cvWaitKey(3000);
			#endif // _USE_OPENCV_CPP_
			}
			else
			{
				keyPressed	=	HandleKeyDownEvents(argv[fileIdx],
													myWindowName,
													openCV_Image);
			}
			if (keyPressed > 0)
			{
				switch(keyPressed & 0x07f)
				{
					case 'a':
						gAutomatic		=	!gAutomatic;
						if (gAutomatic == false)
						{
							fileIdx	-=	1;
						}
						break;

					case 'q':
						keepGoing	=	false;
						break;

				}
			}
		#ifdef _USE_OPENCV_CPP_
			delete openCV_Image;
			openCV_Image	=	NULL;
			if (smallCV_Image != NULL)
			{
				delete smallCV_Image;
				smallCV_Image	=	NULL;
			}
		#else
			cvReleaseImage(&openCV_Image);
			openCV_Image	=	NULL;
			if (smallCV_Image != NULL)
			{
				cvReleaseImage(&smallCV_Image);
				smallCV_Image	=	NULL;
			}
		#endif
		}

		fileIdx++;
		if (keepLooping)
		{
			if (fileIdx >= argc)
			{
				fileIdx	=	1;
			}
		}
	}
	return(0);
}


