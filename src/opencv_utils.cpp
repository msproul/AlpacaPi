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
//*	Sep  6,	2023	<MLS> Added LLG_FillRect() to opencv_utils
//*	Sep  6,	2023	<MLS> Added LLG_DrawCString() to opencv_utils
//*****************************************************************************

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//#include	"opencv2/opencv.hpp"
//#include	"opencv2/core.hpp"
#include	<opencv2/opencv.hpp>
#include	<opencv2/core.hpp>

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

//	CONSOLE_DEBUG(__FUNCTION__);

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

//			CONSOLE_DEBUG_W_NUM("oldImgBytesPerPixel\t=", oldImgBytesPerPixel);
//			CONSOLE_DEBUG_W_NUM("rgbImgRowStepSize  \t=", rgbImgRowStepSize);

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

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(rgbOpenCVImage);
}


//*****************************************************************************
void	DumpCVMatStruct(const char *calledFrom, cv::Mat *theImageMat, const char *message)
{
//		oldImgRowStepSize	=	myOpenCVimage->step[0];
//		oldImgChannels		=	myOpenCVimage->step[1];

	CONSOLE_DEBUG("-------------------------------------------");
	CONSOLE_DEBUG_W_STR("Called from", calledFrom);

	if (message != NULL)
	{
		CONSOLE_DEBUG(message);
	}

	CONSOLE_DEBUG_W_NUM("theImageMat->cols\t\t=",		theImageMat->cols);
	CONSOLE_DEBUG_W_NUM("theImageMat->rows\t\t=",		theImageMat->rows);
	CONSOLE_DEBUG_W_NUM("theImageMat->dims\t\t=",		theImageMat->dims);
	if (theImageMat->dims >= 1)
	{
		CONSOLE_DEBUG_W_SIZE("step[0] (rowStepSize)\t=",	theImageMat->step[0]);
	}
	if (theImageMat->dims >= 2)
	{
		CONSOLE_DEBUG_W_SIZE("step[1] (nChannels)\t=",		theImageMat->step[1]);
	}
	if (theImageMat->dims >= 3)
	{
		CONSOLE_DEBUG_W_SIZE("theImageMat->step[2]\t=",		theImageMat->step[2]);
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)

//**************************************************************************************
//*	Low Level FillRect
//**************************************************************************************
void	LLG_FillRect(cv::Mat *openCV_Image, int left, int top, int width, int height, cv::Scalar fillColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("left  \t=",	left);
//	CONSOLE_DEBUG_W_NUM("width \t=",	width);
//	CONSOLE_DEBUG_W_NUM("top   \t=",	top);
//	CONSOLE_DEBUG_W_NUM("height\t=",	height);

	if (openCV_Image != NULL)
	{
	cv::Rect	myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cv::rectangle(	*openCV_Image,
						myCVrect,
						fillColor,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED
					#else
						CV_FILLED
					#endif
						);

	}
	else
	{
		CONSOLE_DEBUG("openCV_Image is NULL");
	}
}
#else

//**************************************************************************************
void	LLG_FillRect(IplImage *openCV_Image, int left, int top, int width, int height, CvScalar fillColor)
{
	if (openCV_Image != NULL)
	{
	CvRect		myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cvRectangleR(	openCV_Image,
						myCVrect,
						fillColor,				//	cv::Scalar color,
						CV_FILLED,					//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
	else
	{
		CONSOLE_DEBUG("openCV_Image is NULL");
	}
}
#endif // _USE_OPENCV_CPP_

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
void	LLG_DrawCString(	cv::Mat		*openCV_Image,
							const int	xxLoc,
							const int	yyLoc,
							const char	*theString,
							const int	fontIndex,
							cv::Scalar	fillColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xxLoc    \t=",	xxLoc);
//	CONSOLE_DEBUG_W_NUM("yyLoc    \t=",	yyLoc);
//	CONSOLE_DEBUG_W_STR("theString\t=",	theString);

	if (openCV_Image != NULL)
	{
	cv::Point		textLoc;
//	int				curFontNum;
//		curFontNum	=	1;
//		if ((curFontNum >= 0) && (curFontNum < kFont_last))
//		{
//			curFontNum	=	fontIndex;
//		}
		textLoc.x	=	xxLoc;
		textLoc.y	=	yyLoc;
		cv::putText(	*openCV_Image,
						theString,
						textLoc,
						cv::FONT_HERSHEY_SIMPLEX,	//	gFontInfo[curFontNum].fontID,
						1.0,						//	gFontInfo[curFontNum].scale,
						fillColor,
						1							//	gFontInfo[curFontNum].thickness
						);

	}
	else
	{
		CONSOLE_DEBUG("openCV_Image is NULL");
	}
}
#else
//*****************************************************************************
void	LLG_DrawCString(	IplImage	*openCV_Image,
							const int	xx,
							const int	yy,
							const char	*theString,
							const int	fontIndex)
{
	//	CONSOLE_DEBUG(theString);
	if (openCV_Image != NULL)
	{
	CvPoint		textLoc;
//	int			myFontIdx;
//		myFontIdx	=	1;
//		if ((fontIndex >= 0) && (fontIndex < 10))
//		{
//			myFontIdx	=	fontIndex;
//		}
//		else
//		{
//			myFontIdx	=	1;
//		}
		textLoc.x	=	xx;
		textLoc.y	=	yy;
//		cvPutText(	openCV_Image,
//					theString,
//					textLoc,
//					&gTextFont[myFontIdx],
//					cCurrentColor
//				);

	}
	else
	{
		CONSOLE_ABORT("openCV_Image is NULL");
	}
}
#endif // defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)

#endif	//	(CV_MAJOR_VERSION >= 2)

