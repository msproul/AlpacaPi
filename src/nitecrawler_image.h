//*****************************************************************************
//#include	"nitecrawler_image.h"

#ifndef _NITECRAWLER_IMAGE_H_
#define	_NITECRAWLER_IMAGE_H_

#include	<opencv2/opencv.hpp>
#include	<opencv2/core.hpp>

#ifndef _USE_OPENCV_CPP_
	#include "opencv2/highgui/highgui_c.h"
	#include "opencv2/imgproc/imgproc_c.h"
	#include "opencv2/core/version.hpp"
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat			*GetNiteCrawlerImage(void);
	cv::Mat			*GetMoonLiteImage(void);
	extern	cv::Mat	*gNiteCrawlerImgPtr;
#else
	IplImage		*GetNiteCrawlerImage(void);
	IplImage		*GetMoonLiteImage(void);
	extern IplImage	*gNiteCrawlerImgPtr;
	void			WriteOutImageAsCode(IplImage *theImage, const char *codeFileName);
#endif
void		WriteOutImageFileAsCode(const char *fileName);


#ifdef __cplusplus
}
#endif


#endif	//	_NITECRAWLER_IMAGE_H_
