//*****************************************************************************
//#include	"fits_opencv.h"

#ifdef _USE_OPENCV_CPP_
	#include	<opencv2/opencv.hpp>
	#include	<opencv2/core.hpp>
#else
	#include	"opencv2/highgui/highgui_c.h"
	#include	"opencv2/imgproc/imgproc_c.h"
	#include	"opencv2/core/version.hpp"

	#if (CV_MAJOR_VERSION >= 3)
		#include	"opencv2/imgproc/imgproc.hpp"
	#endif
#endif


#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat		*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	cv::Mat		*ReadImageIntoOpenCVimage(const char *imageFileName);
	cv::Mat		*ReadPDSimageIntoOpenCVimage(const char *imageFileName);
#else
	IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	IplImage	*ReadImageIntoOpenCVimage(const char *imageFileName);
#endif
