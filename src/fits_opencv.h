//*****************************************************************************
//#include	"fits_opencv.h"

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat		*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	cv::Mat		*ReadImageIntoOpenCVimage(const char *imageFileName);
	cv::Mat		*ReadPDSimageIntoOpenCVimage(const char *imageFileName);
#else
	IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	IplImage	*ReadImageIntoOpenCVimage(const char *imageFileName);
#endif
