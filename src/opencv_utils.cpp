//*****************************************************************************
//*		opencv_utils.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:	Basic openCV utility functions
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	May  5,	2022	<MLS> Added ConvertImageToRGB()
//*	Sep 19,	2022	<MLS> Created opencv_utils.cpp
//*	Sep 19,	2022	<MLS> Moved DumpCVMatStruct() from controller.cpp
//*****************************************************************************

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"opencv2/opencv.hpp"
#include	"opencv2/core.hpp"

#include	"opencv_utils.h"


#if (CV_MAJOR_VERSION == 2)
	#warning "OpenCV is version 2"
#endif


#if (CV_MAJOR_VERSION >= 2)

//**************************************************************************************
cv::Mat *ConvertImageToRGB(cv::Mat *openCVImage)
{
int		oldImgWidth;
int		oldImgHeight;
int		oldImgRowStepSize;
int		oldImgBytesPerPixel;
int		rgbImgRowStepSize;
cv::Mat	*rgbOpenCVImage;
size_t	byteCount;
int		row;
int		clm;
int		pixIdx;
int		pixValue;
uint8_t	*oldRowPtr;
uint8_t	*rgbRowPtr;

	CONSOLE_DEBUG(__FUNCTION__);

	oldImgWidth			=	openCVImage->cols;
	oldImgHeight		=	openCVImage->rows;
	oldImgRowStepSize	=	openCVImage->step[0];
	oldImgBytesPerPixel	=	openCVImage->step[1];

	//*	create an RGB image of the same size
	rgbOpenCVImage		=	new cv::Mat(cv::Size(	oldImgWidth,
													oldImgHeight),
													CV_8UC3);
	if (rgbOpenCVImage != NULL)
	{
		//*	double check the data buffers
		if ((rgbOpenCVImage->data != NULL) && (openCVImage->data != NULL))
		{
			rgbImgRowStepSize	=	rgbOpenCVImage->step[0];

			CONSOLE_DEBUG_W_NUM("oldImgBytesPerPixel\t=", oldImgBytesPerPixel);
			CONSOLE_DEBUG_W_NUM("rgbImgRowStepSize\t=", rgbImgRowStepSize);

			//*	copy the image data to OUR image
			switch(oldImgBytesPerPixel)
			{
				case 1:
							//		#if (CV_MAJOR_VERSION <= 3)
							//			cv::cvtColor(*newOpenCVImage, *cDownLoadedImage, CV_GRAY2RGB);
							//		#else
							//			#warning "OpenCV convert from 8 bit to RGB not finished"
							//		#endif
					//*	copy the data over, pixel at a time
					for (row = 0; row < oldImgHeight; row++)
					{
						oldRowPtr	=	openCVImage->data;
						oldRowPtr	+=	row * oldImgRowStepSize;

						rgbRowPtr	=	rgbOpenCVImage->data;
						rgbRowPtr	+=	row * rgbImgRowStepSize;

						pixIdx		=	0;
						for (clm = 0; clm < oldImgWidth; clm++)
						{
							pixValue			=	oldRowPtr[clm];	//*	get the 8 bit pixel value
							rgbRowPtr[pixIdx++]	=	pixValue;		//*	put it in R,G,B
							rgbRowPtr[pixIdx++]	=	pixValue;
							rgbRowPtr[pixIdx++]	=	pixValue;
						}
					}
					break;

				case 2:
					//*	copy the data over, pixel at a time
					for (row = 0; row < oldImgHeight; row++)
					{
						oldRowPtr	=	openCVImage->data;
						oldRowPtr	+=	row * oldImgRowStepSize;

						rgbRowPtr	=	rgbOpenCVImage->data;
						rgbRowPtr	+=	row * rgbImgRowStepSize;

						pixIdx		=	0;
						for (clm = 0; clm < oldImgWidth; clm++)
						{
							//*	get the most significant byte of the 16 bit value
						#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
							pixValue			=	oldRowPtr[(clm * 2) + 1];
						#else
							pixValue			=	oldRowPtr[(clm * 2)];
						#endif
							rgbRowPtr[pixIdx++]	=	pixValue;	//*	put it in R,G,B
							rgbRowPtr[pixIdx++]	=	pixValue;
							rgbRowPtr[pixIdx++]	=	pixValue;
						}
					}
					break;

				case 3:
					if (rgbImgRowStepSize == oldImgRowStepSize)
					{
						//*	the data is the same, use memcpy
						byteCount	=	openCVImage->rows * oldImgRowStepSize;
						memcpy(rgbOpenCVImage->data, openCVImage->data, byteCount);
					}
					else
					{
						CONSOLE_DEBUG("Failed to copy data from newOpenCVImage to cDownLoadedImage");
					}
					break;
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to create new RGB image")
	}

//	openCVerr	=	cv::imwrite("16bitdownload.png", *cDownLoadedImage);
//
//	newOpenCVImage->convertTo(*cDownLoadedImage, CV_8UC3);
//	openCVerr	=	cv::imwrite("16bitdownload-8bit.png", *cDownLoadedImage);

	return(rgbOpenCVImage);
}


//*****************************************************************************
void	DumpCVMatStruct(cv::Mat *theImageMat, const char *message)
{
//		oldImgRowStepSize	=	myOpenCVimage->step[0];
//		oldImgChannels		=	myOpenCVimage->step[1];

	if (message != NULL)
	{
		CONSOLE_DEBUG(message);
	}

	CONSOLE_DEBUG_W_NUM("theImageMat->cols\t\t=",		theImageMat->cols);
	CONSOLE_DEBUG_W_NUM("theImageMat->rows\t\t=",		theImageMat->rows);
	CONSOLE_DEBUG_W_NUM("theImageMat->dims\t\t=",		theImageMat->dims);
	if (theImageMat->dims >= 1)
	{
		CONSOLE_DEBUG_W_LONG("step[0] (rowStepSize)\t=",	theImageMat->step[0]);
	}
	if (theImageMat->dims >= 2)
	{
		CONSOLE_DEBUG_W_LONG("step[1] (nChannels)\t=",		theImageMat->step[1]);
	}
	if (theImageMat->dims >= 3)
	{
		CONSOLE_DEBUG_W_LONG("theImageMat->step[2]\t=",		theImageMat->step[2]);
	}

}

#endif	//	(CV_MAJOR_VERSION >= 3)

