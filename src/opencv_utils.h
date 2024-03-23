//**************************************************************************************
//#include	"opencv_utils.h"

#ifndef _OPENCV_UTILS_H_
#define _OPENCV_UTILS_H_


//#include	"opencv2/opencv.hpp"
//#include	"opencv2/core.hpp"
#include	<opencv2/opencv.hpp>
#include	<opencv2/core.hpp>


cv::Mat *ConvertImageToRGB(cv::Mat *openCVImage);
void	DumpCVMatStruct(const char *calledFrom, cv::Mat *theImageMat, const char *message= NULL);

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)

void	LLG_FillRect(cv::Mat *openCV_Image, int left, int top, int width, int height, cv::Scalar fillColor);
void	LLG_DrawCString(	cv::Mat		*openCV_Image,
							const int	xx,
							const int	yy,
							const char	*theString,
							const int	fontIndex,
							cv::Scalar	fillColor);
#else

void	LLG_FillRect(	IplImage	*openCV_Image,
						int			left,
						int			top,
						int			width,
						int			height,
						CvScalar	fillColor);
#endif // _USE_OPENCV_CPP_

#endif // _OPENCV_UTILS_H_
