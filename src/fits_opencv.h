//*****************************************************************************
//#include	"fits_opencv.h"

#if defined(_USE_OPENCV_CPP_) &&  (CV_MAJOR_VERSION >= 4)
#else
	IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName);
	IplImage	*ReadImageIntoOpenCVimage(const char *imageFileName);
#endif
