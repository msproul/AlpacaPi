//*****************************************************************************
//#include	"fits_opencv.h"

#if defined(_USE_OPENCV_CPP_)
	cv::Mat		*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	cv::Mat		*ReadImageIntoOpenCVimage(const char *imageFileName);
#else
	IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	IplImage	*ReadImageIntoOpenCVimage(const char *imageFileName);
#endif
