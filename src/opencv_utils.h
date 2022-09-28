//#include	"opencv_utils.h"

#ifndef _OPENCV_UTILS_H_
#define _OPENCV_UTILS_H_


#include	"opencv2/opencv.hpp"
#include	"opencv2/core.hpp"


cv::Mat *ConvertImageToRGB(cv::Mat *openCVImage);
void	DumpCVMatStruct(cv::Mat *theImageMat, const char *message=NULL);

#endif // _OPENCV_UTILS_H_
