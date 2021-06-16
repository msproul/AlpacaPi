//*****************************************************************************
//#include	"nitecrawler_image.h"

#ifndef _NITECRAWLER_IMAGE_H_
#define	_NITECRAWLER_IMAGE_H_
#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#ifdef __cplusplus
	extern "C" {
#endif

IplImage	*GetNiteCrawlerImage(void);
IplImage	*GetMoonLiteImage(void);

void		WriteOutImageAsCode(IplImage *theImage, const char *codeFileName);
void		WriteOutImageFileAsCode(const char *fileName);

extern	IplImage	*gNiteCrawlerImgPtr;

#ifdef __cplusplus
}
#endif


#endif	//	_NITECRAWLER_IMAGE_H_
