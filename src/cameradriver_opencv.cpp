//**************************************************************************
//*	Name:			cameradriver_opencv.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Usage notes:
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 19,	2020	<MLS> Started opencv mouse handling in image window
//*	Feb 19,	2020	<MLS> Added SetOpenCVcallbackFunction()
//*	Feb 19,	2020	<MLS> Added ProcessMouseEvent() & DrawCrossHairs()
//*	Feb 22,	2020	<MLS> Added image number to live view to verify image update
//*	Mar  7,	2020	<MLS> Fixed text display bug in 16 bit mode
//*	Mar 27,	2020	<MLS> OpenCV V3.3.1 installed and working on jetson nano
//*	Apr  8,	2020	<MLS> Added CreateHistogramGraph()
//*	Apr 10,	2020	<MLS> Added DisplayLiveImage_wSideBar() & DrawSidebar()
//*	Apr 11,	2020	<MLS> Added frames saved to sidebar
//*	Apr 16,	2020	<MLS> Switched to using commoncolor for background color selection
//*	Apr 19,	2020	<MLS> Fixed cross hair location when using sidebar
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_USE_OPENCV_)
#include	<stdio.h>
#include	<string.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#ifdef _ENABLE_STAR_SEARCH_
	#include	"imageprocess_orb.h"
#endif


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"
#define _DEBUG_TIMING_


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"commoncolor.h"

#define	FC_BLUE()	CV_RGB(0x64, 0x8c, 0xff)


IplImage	*gAlpacaImgPtr	=	NULL;

//*****************************************************************************
void	LoadAlpacaImage(void)
{
	if (gAlpacaImgPtr == NULL)
	{
		gAlpacaImgPtr	=	cvLoadImage("logos/AlpacaLogo-small.png", CV_LOAD_IMAGE_COLOR);
	}
}


//*****************************************************************************
void	CameraDriver::ProcessMouseEvent(int event, int xxx, int yyy, int flags)
{
int	deltaX;
int	deltaY;
int	deltaMouse;

	switch(event)
	{
		case CV_EVENT_MOUSEMOVE:
		//	CONSOLE_DEBUG("CV_EVENT_MOUSEMOVE");
			cCurrentMouseX	=	xxx;
			cCurrentMouseY	=	yyy;
			if (cLeftButtonDown)
			{
				deltaX		=	cCurrentMouseX - cLastLClickX;
				deltaY		=	cCurrentMouseY - cLastLClickY;
				deltaMouse	=	sqrt((deltaX * deltaX) + (deltaY * deltaY));
//				CONSOLE_DEBUG_W_NUM("deltaMouse\t=", deltaMouse);
				if (deltaMouse > 50)
				{
					cDrawRectangle	=	true;
				}
				else
				{
					cDrawRectangle	=	false;
				}
			}
			break;

		case CV_EVENT_LBUTTONDOWN:
			cLeftButtonDown	=	true;
//			CONSOLE_DEBUG("CV_EVENT_LBUTTONDOWN");
			if ((flags & CV_EVENT_FLAG_CTRLKEY) && (flags & CV_EVENT_FLAG_SHIFTKEY))
			{
				cDisplayCrossHairs	=	false;
			}
			else if (flags & CV_EVENT_FLAG_CTRLKEY)
			{
				cDisplayCrossHairs	=	true;
				cCrossHairX			=	xxx;
				cCrossHairY			=	yyy;
				if (cDisplaySideBar)
				{
					//*	adjust the cross hair point
					cCrossHairX		-=	cSideBarWidth + cSideFrameWidth;
					cCrossHairY		-=	cSideFrameWidth;
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("flags\t=", flags);
				cLastLClickX	=	xxx;
				cLastLClickY	=	yyy;
			}
			break;

		case CV_EVENT_RBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDOWN");
			break;

		case CV_EVENT_MBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDOWN");
			break;

		case CV_EVENT_LBUTTONUP:
			cLeftButtonDown	=	false;
			cDrawRectangle	=	false;
			CONSOLE_DEBUG("CV_EVENT_LBUTTONUP");
			break;

		case CV_EVENT_RBUTTONUP:
			cRightButtonDown	=	false;
			CONSOLE_DEBUG("CV_EVENT_RBUTTONUP");
			break;

		case CV_EVENT_MBUTTONUP:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONUP");
			break;

		case CV_EVENT_LBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_LBUTTONDBLCLK");
			break;

		case CV_EVENT_RBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
			break;

		case CV_EVENT_MBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDBLCLK");
			break;
	}
}

//*****************************************************************************
void	CameraDriver::DrawOpenCVoverlay(void)
{
CvPoint			point1;
CvPoint			point2;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cOpenCV_LiveDisplay != NULL)
	{
		if (cDisplayCrossHairs)
		{
			point1.x		=	0;
			point1.y		=	cCrossHairY;

			point2.x		=	1024;
			point2.y		=	cCrossHairY;

			cvLine(	cOpenCV_LiveDisplay,
					point1,
					point2,
					cCrossHairColor,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0						//	int shift CV_DEFAULT(0)
					);

			point1.x	=	cCrossHairX;
			point1.y	=	0;

			point2.x	=	cCrossHairX;
			point2.y	=	1024;

			cvLine(	cOpenCV_LiveDisplay,
					point1,
					point2,
					cCrossHairColor,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0						//	int shift CV_DEFAULT(0)
					);
		}
		if (cDrawRectangle)
		{
			point1.x	=	cLastLClickX;
			point1.y	=	cLastLClickY;

			point2.x	=	cCurrentMouseX;
			point2.y	=	cCurrentMouseY;


			cvRectangle(	cOpenCV_LiveDisplay,
							point1,
							point2,
							cCrossHairColor,	//	CvScalar color,
							1,				//	int thickness CV_DEFAULT(1),
							8,				//	int line_type CV_DEFAULT(8),
							0);				//	int shift CV_DEFAULT(0));

		}
	}
}


//*****************************************************************************
static void	LiveWindowMouseCallbac(int event, int x, int y, int flags, void* param)
{
CameraDriver	*myCameraDriver;

	myCameraDriver	=	(CameraDriver *)param;
	if (myCameraDriver != NULL)
	{
		myCameraDriver->ProcessMouseEvent(event, x, y, flags);

	}
}

//*****************************************************************************
void	CameraDriver::SetOpenCVcallbackFunction(const char *windowName)
{
	cvSetMouseCallback( windowName,
						LiveWindowMouseCallbac,
						(void *)this);

}


//*****************************************************************************
void	CameraDriver::DisplayLiveImage(void)
{
int			keyPressed;

	if (cNewImageReadyToDisplay)
	{
		if (cCreateOpenCVwindow)
		{
			cvNamedWindow(	cOpenCV_ImgWindowName,
						//	(CV_WINDOW_AUTOSIZE)
						//	(CV_WINDOW_NORMAL)
							(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						//	(CV_WINDOW_AUTOSIZE)
							);
			SetOpenCVcallbackFunction(cOpenCV_ImgWindowName);
			cCreateOpenCVwindow		=	false;
			cOpenCV_ImgWindowValid	=	true;
		}
#define	kLimitWidth	1000

		//**************************************************************
		//*	make sure the existing live display is compatible
		//*	if the format of the image is changed (i.e. RAW8 -> RGB24
		//*	while live image is being displayed, opencv aborts on resize
		//*	Feb 18,	2020	<MLS> Fixed bug in live view when image format gets changed
		if (cOpenCV_LiveDisplay != NULL)
		{
			if ((cOpenCV_LiveDisplay->nChannels != cOpenCV_Image->nChannels) ||
				(cOpenCV_LiveDisplay->depth != cOpenCV_Image->depth))
			{
				CONSOLE_DEBUG("Image format has changed!!! re-creating live view image");
				cvReleaseImage(&cOpenCV_LiveDisplay);
				cOpenCV_LiveDisplay	=	NULL;
			}

		}
		//*	we have to create a liveDisp image to display
		if (cOpenCV_LiveDisplay == NULL)
		{
			//*	create the display image (reduced size)
			cLiveDisplayWidth	=	cOpenCV_Image->width;
			cLiveDisplayHeight	=	cOpenCV_Image->height;
			while (cLiveDisplayWidth > kLimitWidth)
			{
				cLiveDisplayWidth	=	cLiveDisplayWidth / 2;
				cLiveDisplayHeight	=	cLiveDisplayHeight / 2;
			}
			switch(cROIinfo.currentROIimageType)
			{
				case kImageType_RAW8:
				case kImageType_Y8:
					cOpenCV_LiveDisplay	=	cvCreateImage(cvSize(cLiveDisplayWidth, cLiveDisplayHeight), IPL_DEPTH_8U, 1);
					break;

				case kImageType_RAW16:
					cOpenCV_LiveDisplay	=	cvCreateImage(cvSize(cLiveDisplayWidth, cLiveDisplayHeight), IPL_DEPTH_16U, 1);
					break;

				case kImageType_RGB24:
					cOpenCV_LiveDisplay	=	cvCreateImage(cvSize(cLiveDisplayWidth, cLiveDisplayHeight), IPL_DEPTH_8U, 3);
					break;

				default:
					cOpenCV_LiveDisplay	=	NULL;
					break;

			}
			SetOpenCVcolors(cOpenCV_LiveDisplay);
		}
		if (cOpenCV_LiveDisplay != NULL)
		{
		CvPoint			point1;
		char			imageNumBuff[32];

			cvResize(cOpenCV_Image, cOpenCV_LiveDisplay, CV_INTER_LINEAR);
			if (cDisplayCrossHairs || cDrawRectangle)
			{
				DrawOpenCVoverlay();
			}

			point1.x	=	20;
			point1.y	=	20;
			sprintf(imageNumBuff, "#%ld %d-bit, EXP=%f",
											cFramesRead,
											cOpenCV_LiveDisplay->depth,
											(cCurrentExposure_us / 1000000.0));
//				CONSOLE_DEBUG_W_STR("imageNumBuff\t=", imageNumBuff);
			cvPutText(	cOpenCV_LiveDisplay,
						imageNumBuff,
						point1,
						&cTextFont,
						cSideBarTXTcolor
						);

			cvShowImage(cOpenCV_ImgWindowName, cOpenCV_LiveDisplay);
		}

		cNewImageReadyToDisplay	=	false;
	}

	keyPressed	=	cvWaitKey(10);	//*	required to allow opencv to update the window
	if (keyPressed > 0)
	{
	//	CONSOLE_DEBUG_W_HEX("Key pressed =", keyPressed);
		switch(tolower(keyPressed & 0x007f))
		{
			case 'r':
				CONSOLE_DEBUG_W_STR("Reseting window:", cOpenCV_ImgWindowName);
				cvMoveWindow(cOpenCV_ImgWindowName, 25, 100);
				cvResizeWindow(cOpenCV_ImgWindowName, 500, 500);
				keyPressed	=	cvWaitKey(5000);
				break;

		}
	}
}

//*****************************************************************************
void	CameraDriver::DisplayLiveImage_wSideBar(void)
{
int			keyPressed;
int			windowWidth;
int			windowHeight;
CvRect		roiRect;
CvRect		myCVrect;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cNewImageReadyToDisplay)
	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, "--------start-----------");
		if (cCreateOpenCVwindow)
		{
			cvNamedWindow(	cOpenCV_ImgWindowName,
						//	(CV_WINDOW_AUTOSIZE)
						//	(CV_WINDOW_NORMAL)
							(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
						//	(CV_WINDOW_AUTOSIZE)
							);
			SetOpenCVcallbackFunction(cOpenCV_ImgWindowName);
			cCreateOpenCVwindow		=	false;
			cOpenCV_ImgWindowValid	=	true;
		}
#define	kLimitWidth	1000

		if (cOpenCV_Image != NULL)
		{
			//**************************************************************
			//*	make sure the existing live display is compatible
			//*	if the format of the image is changed (i.e. RAW8 -> RGB24
			//*	while live image is being displayed, opencv aborts on resize
			//*	Feb 18,	2020	<MLS> Fixed bug in live view when image format gets changed
			if (cOpenCV_LiveDisplay != NULL)
			{
				if (cOpenCV_LiveDisplay->depth != cOpenCV_Image->depth)
				{
					CONSOLE_DEBUG("Image format has changed!!! re-creating live view image");
					cvReleaseImage(&cOpenCV_LiveDisplay);
					cOpenCV_LiveDisplay	=	NULL;
				}

			}
			//*	we have to create a liveDisp image to display
			if (cOpenCV_LiveDisplay == NULL)
			{
				CONSOLE_DEBUG("Creating new Live Display image");
				//*	create the display image (reduced size)
				cLiveDisplayWidth	=	cOpenCV_Image->width;
				cLiveDisplayHeight	=	cOpenCV_Image->height;
				while (cLiveDisplayWidth > kLimitWidth)
				{
					cLiveDisplayWidth	=	cLiveDisplayWidth / 2;
					cLiveDisplayHeight	=	cLiveDisplayHeight / 2;
				}
				windowWidth		=	cLiveDisplayWidth;
				windowHeight	=	cLiveDisplayHeight;
				if (cDisplaySideBar)
				{
					//*	add the room for the side bar
					windowWidth		+=	cSideBarWidth + (2 * cSideFrameWidth);
					windowHeight	+=	(2 * cSideFrameWidth);
				}
				switch (cOpenCV_Image->depth)
				{
					case 8:
						cOpenCV_LiveDisplay	=	cvCreateImage(cvSize(windowWidth, windowHeight), IPL_DEPTH_8U, 3);
						break;

					case 16:
						cOpenCV_LiveDisplay	=	cvCreateImage(cvSize(windowWidth, windowHeight), IPL_DEPTH_16U, 3);
						break;

					default:
						CONSOLE_DEBUG("UNKNOWN PIXEL DEPTH!!!!!!!!!!!!!");
						cOpenCV_LiveDisplay	=	NULL;
						exit(0);
						break;

				}
				CONSOLE_DEBUG("New display image created");
				SetOpenCVcolors(cOpenCV_LiveDisplay);
			}

			if (cOpenCV_LiveDisplay != NULL)
			{
				//*	check to see if the image sizes are different
				if ((cLiveDisplayWidth != cOpenCV_LiveDisplay->width ) ||
					(cLiveDisplayHeight != cOpenCV_LiveDisplay->height ))
				{
					//*	set the entire background color
					myCVrect.x		=	0;
					myCVrect.y		=	0;
					myCVrect.width	=	cOpenCV_LiveDisplay->width;
					myCVrect.height	=	cOpenCV_LiveDisplay->height;

					cvRectangleR(	cOpenCV_LiveDisplay,
									myCVrect,
									cSideBarBGcolor,			//	CvScalar color,
									CV_FILLED,					//	int thickness CV_DEFAULT(1),
									8,							//	int line_type CV_DEFAULT(8),
									0);							//	int shift CV_DEFAULT(0));

					//*	we have to set the ROI
//					CONSOLE_DEBUG("we have to set the ROI");
					roiRect.x		=	cSideBarWidth + cSideFrameWidth;
					roiRect.y		=	cSideFrameWidth;
					roiRect.width	=	cLiveDisplayWidth;
					roiRect.height	=	cLiveDisplayHeight;
					cvSetImageROI(cOpenCV_LiveDisplay,  roiRect);
				}
				else
				{
					CONSOLE_DEBUG("SAME");
				}
			#ifdef _ENABLE_STAR_SEARCH_
				long	keyPointCnt;
				//*	this is an attempt at finding the locations of all of the stars in an image.

//				CONSOLE_DEBUG("Calling ProcessORB_Image!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				SETUP_TIMING();

				keyPointCnt	=	ProcessORB_Image(cOpenCV_Image);

				DEBUG_TIMING("Time to complete ORB");

				CONSOLE_DEBUG_W_NUM("keyPointCnt\t=", keyPointCnt);
			#endif // _JETSON_
				//*	lets try to display gray scale on a color screen
				if ((cOpenCV_Image->nChannels == 1) && (cOpenCV_Image->depth == 8))
				{
				IplImage	*smallImg;

					//CONSOLE_DEBUG_W_STR(__FUNCTION__, "chan = 1 and depth = 8");

					//*	create a small image (8 bit)
					smallImg	=	cvCreateImage(cvSize(cLiveDisplayWidth, cLiveDisplayHeight), IPL_DEPTH_8U, 1);
					if (smallImg != NULL)
					{
						cvResize(cOpenCV_Image, smallImg, CV_INTER_LINEAR);
					#ifdef _ENABLE_STAR_SEARCH_
						CONSOLE_DEBUG("Calling ProcessORB_Image!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						SETUP_TIMING();

						ProcessORB_Image(smallImg);

						DEBUG_TIMING("Time to complete ORB");
						CONSOLE_DEBUG_W_NUM("Image width\t=", smallImg->width);
					#endif // _JETSON_

						cvCvtColor(smallImg, cOpenCV_LiveDisplay, CV_GRAY2RGB);
						cvReleaseImage(&smallImg);
					}
					else
					{
						CONSOLE_DEBUG("Failed to create small image");
					}
				}
				else if ((cOpenCV_Image->nChannels == 1) && (cOpenCV_Image->depth == 16))
				{
				IplImage	*smallImg;

					CONSOLE_DEBUG_W_STR(__FUNCTION__, "chan = 1 and depth = 16");

					//*	create a small image (16 bit)
					smallImg	=	cvCreateImage(cvSize(cLiveDisplayWidth, cLiveDisplayHeight), IPL_DEPTH_16U, 1);
					if (smallImg != NULL)
					{
						cvResize(cOpenCV_Image, smallImg, CV_INTER_LINEAR);

						cvCvtColor(smallImg, cOpenCV_LiveDisplay, CV_GRAY2RGB);
						cvReleaseImage(&smallImg);
					}
					else
					{
						CONSOLE_DEBUG("Failed to create small image");
					}
				}
				else
				{
					CONSOLE_DEBUG_W_STR(__FUNCTION__, "default");
					cvResize(cOpenCV_Image, cOpenCV_LiveDisplay, CV_INTER_LINEAR);
				}
				if (cDisplayCrossHairs || cDrawRectangle)
				{
					DrawOpenCVoverlay();
				}
				cvResetImageROI(cOpenCV_LiveDisplay);

				//************************************************************
				//*	we are now going to start drawing the sidebar
				DrawSidebar(cOpenCV_LiveDisplay);

				cvShowImage(cOpenCV_ImgWindowName, cOpenCV_LiveDisplay);
			}
			else
			{
				CONSOLE_DEBUG("Failed to create live window image");
			}
		}
		else
		{
			CONSOLE_DEBUG("cOpenCV_Image is null");
			exit(0);
		}
		cNewImageReadyToDisplay	=	false;
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, "--------exit-----------");
	}
	else
	{
	//	CONSOLE_DEBUG("Nothing to display");
	}

	keyPressed	=	cvWaitKey(10);	//*	required to allow opencv to update the window
	if (keyPressed > 0)
	{
	//	CONSOLE_DEBUG_W_HEX("Key pressed =", keyPressed);
		switch(tolower(keyPressed & 0x007f))
		{
			case 'r':
				CONSOLE_DEBUG_W_STR("Reseting window:", cOpenCV_ImgWindowName);
				cvMoveWindow(cOpenCV_ImgWindowName, 25, 100);
				cvResizeWindow(cOpenCV_ImgWindowName, 500, 500);
				keyPressed	=	cvWaitKey(5000);
				break;

		}
	}
}

//*****************************************************************************
void	CameraDriver::DrawSidebar(IplImage *imageDisplay)
{
CvRect		roiRect;
int			yLoc;
CvPoint		point1;
CvPoint		point2;
char		textBuffer[64];
const int	deltaY	=	16;

//	CONSOLE_DEBUG(__FUNCTION__);

	yLoc		=	cSideFrameWidth;
	point1.x	=	cSideBarWidth + cSideFrameWidth;
	point1.y	=	yLoc -2 ;
	cvPutText(	imageDisplay,	cFileNameRoot,		point1,	&cTextFont,	cSideBarTXTcolor);



	//=======================================
	//*	Frame count
	point1.x	=	cSideFrameWidth;
	point1.y	=	yLoc;
	point2.x	=	cSideFrameWidth + (cSideBarWidth/2);
	point2.y	=	yLoc;
	sprintf(textBuffer, "#%ld %d-bit", cFramesRead, cOpenCV_LiveDisplay->depth);
	cvPutText(	imageDisplay,	textBuffer,				point1,	&cTextFont,	cSideBarTXTcolor);
	yLoc	+=	deltaY;

	//=======================================
	//*	Exposure
	point1.y	=	yLoc;
	point2.y	=	yLoc;
	sprintf(textBuffer, "%f", (cCurrentExposure_us / 1000000.0));
	if (cAutoAdjustExposure)
	{
		strcat(textBuffer, " (auto)");
	}
	cvPutText(	imageDisplay,	"Exposure:",			point1,	&cTextFont,	cSideBarTXTcolor);
	cvPutText(	imageDisplay,	textBuffer,				point2,	&cTextFont,	cSideBarTXTcolor);
	yLoc	+=	deltaY;

	//=======================================
	//*	Gain
	point1.y	=	yLoc;
	point2.y	=	yLoc;
	sprintf(textBuffer, "%d", cCameraProp.Gain);
	cvPutText(	imageDisplay,	"Gain:",				point1,	&cTextFont,	cSideBarTXTcolor);
	cvPutText(	imageDisplay,	textBuffer,				point2,	&cTextFont,	cSideBarTXTcolor);
	yLoc	+=	deltaY;

	if (cTempReadSupported)
	{
		point1.y	=	yLoc;
		point2.y	=	yLoc;
		sprintf(textBuffer, "%2.1f C", cCameraTemp_Dbl);
		cvPutText(	imageDisplay,	"Camera Temp:",			point1,	&cTextFont,	cSideBarTXTcolor);
		cvPutText(	imageDisplay,	textBuffer,				point2,	&cTextFont,	cSideBarTXTcolor);
		yLoc	+=	deltaY;
	}


#ifdef _ENABLE_FILTERWHEEL_
	if (strlen(cFilterWheelCurrName) > 0)
	{
		point1.y	=	yLoc;
		point2.y	=	yLoc;
		cvPutText(	imageDisplay,	"Filter:",				point1,	&cTextFont,	cSideBarTXTcolor);
		cvPutText(	imageDisplay,	cFilterWheelCurrName,	point2,	&cTextFont,	cSideBarTXTcolor);
		yLoc	+=	deltaY;
	}
#endif // _ENABLE_FILTERWHEEL_

	point1.y	=	yLoc;
	point2.y	=	yLoc;
	cvPutText(	imageDisplay,	"Object:",				point1,	&cTextFont,	cSideBarTXTcolor);
	cvPutText(	imageDisplay,	cObjectName,			point2,	&cTextFont,	cSideBarTXTcolor);
	yLoc	+=	deltaY;

	if (cFrameRate > 0)
	{
		point1.y	=	yLoc;
		point2.y	=	yLoc;
		sprintf(textBuffer, "%2.1f fps", cFrameRate);
		cvPutText(	imageDisplay,	"Frame Rate:",			point1,	&cTextFont,	cSideBarTXTcolor);
		cvPutText(	imageDisplay,	textBuffer,				point2,	&cTextFont,	cSideBarTXTcolor);
		yLoc	+=	deltaY;
	}

	//=======================================
	//*	total fames saved
	point1.y	=	yLoc;
	point2.y	=	yLoc;
	sprintf(textBuffer, "%d", cTotalFramesSaved);
	cvPutText(	imageDisplay,	"Frames Saved:",		point1,	&cTextFont,	cSideBarTXTcolor);
	cvPutText(	imageDisplay,	textBuffer,				point2,	&cTextFont,	cSideBarTXTcolor);
	yLoc	+=	deltaY;



	yLoc	+=	50;
	//*************************************
	//*	Histogram
	//*	we have to set the ROI
	roiRect.x		=	cSideFrameWidth / 2;
	roiRect.y		=	yLoc;
	roiRect.width	=	cSideBarWidth;
	roiRect.height	=	100;
	cvSetImageROI(imageDisplay,  roiRect);

	CalculateHistogramArray();
	CreateHistogramGraph(imageDisplay);

	cvResetImageROI(imageDisplay);
	//*	put a boarder around it
	cvRectangleR(	imageDisplay,
					roiRect,
					cSideBarTXTcolor,	//	CvScalar color,
					1,					//	int thickness CV_DEFAULT(1),
					8,					//	int line_type CV_DEFAULT(8),
					0);					//	int shift CV_DEFAULT(0));
	yLoc	+=	100;
	yLoc	+=	5;

	//=============================================================
	//*	put the alpaca logo in the bottom left corner
	if (gAlpacaImgPtr != NULL)
	{
		roiRect.x		=	0;
		roiRect.y		=	imageDisplay->height - gAlpacaImgPtr->height;
		roiRect.width	=	gAlpacaImgPtr->width;
		roiRect.height	=	gAlpacaImgPtr->height;
		cvSetImageROI(imageDisplay,  roiRect);

		cvCopy(gAlpacaImgPtr, imageDisplay);

		cvResetImageROI(imageDisplay);

		point1.x	=	2;
		point1.y	=	roiRect.y - 1;
		cvPutText(	imageDisplay,	"Compatible with:",		point1,	&cTextFont,	cSideBarTXTcolor);
	}
}


//*****************************************************************************
enum
{
	kGraphTYpe_Filled	=	0,
	kGraphTYpe_Line,
	kGraphTYpe_Log,
};

//*****************************************************************************
static void	DrawGraph256(	IplImage	*theImage,
							int32_t		*theArray,
							int			xStart,
							int			windowHeight,
							int			graphHeight,
							int			yDivideFactor,
							int			graphType,
							CvScalar	lineColor)
{
int			iii;
int32_t		histYvalue;
CvPoint		point1;
CvPoint		point2;
double		logOf255;
double		logOfPixlCnt;

	switch(graphType)
	{
		case kGraphTYpe_Filled:
			for (iii=0; iii<256; iii++)
			{
				histYvalue	=	theArray[iii] / yDivideFactor;
				if (histYvalue > 0)
				{
					point1.x	=	xStart + iii;
					point1.y	=	windowHeight;

					point2.x	=	xStart + iii;
					point2.y	=	windowHeight - histYvalue;

					cvLine(	theImage,
							point1,
							point2,
							lineColor,
							1,						//	int thickness CV_DEFAULT(1),
							8,						//	int line_type CV_DEFAULT(8),
							0						//	int shift CV_DEFAULT(0)
							);
				}
			}
			break;

		case kGraphTYpe_Line:
			histYvalue	=	theArray[0] / yDivideFactor;
			point1.x	=	xStart;
			point1.y	=	windowHeight - histYvalue;
			for (iii=0; iii<256; iii++)
			{
				histYvalue	=	theArray[iii] / yDivideFactor;

				point2.x	=	xStart + iii;
				point2.y	=	windowHeight - histYvalue;

				cvLine(	theImage,
						point1,
						point2,
						lineColor,
						1,						//	int thickness CV_DEFAULT(1),
						8,						//	int line_type CV_DEFAULT(8),
						0						//	int shift CV_DEFAULT(0)
						);
				point1	=	point2;
			}
			break;

		case kGraphTYpe_Log:
			logOf255	=	log(255);

			for (iii=0; iii<256; iii++)
			{
				histYvalue	=	theArray[iii] / yDivideFactor;
				if (histYvalue > 0)
				{
					logOfPixlCnt	=	log(histYvalue);
					histYvalue		=	(logOfPixlCnt * graphHeight) / logOf255;
					point1.x		=	xStart + iii;
					point1.y		=	windowHeight;

					point2.x		=	xStart + iii;
					point2.y		=	windowHeight - histYvalue;

					cvLine(	theImage,
							point1,
							point2,
							lineColor,
							1,						//	int thickness CV_DEFAULT(1),
							8,						//	int line_type CV_DEFAULT(8),
							0						//	int shift CV_DEFAULT(0)
							);
				}
			}
			break;
	}

}

//*****************************************************************************
static void	DrawHorizBar256(	IplImage	*theImage,
								int			xStart,
								int			yLoc,
								int			maxValue,
								CvScalar	lineColor)
{
CvPoint		point1;
CvPoint		point2;

	point1.x	=	xStart;
	point1.y	=	yLoc;

	point2.x	=	xStart + maxValue;
	point2.y	=	yLoc;

	cvLine(	theImage,
			point1,
			point2,
			lineColor,
			3,						//	int thickness CV_DEFAULT(1),
			8,						//	int line_type CV_DEFAULT(8),
			0						//	int shift CV_DEFAULT(0)
			);

}


//*****************************************************************************
void	CameraDriver::Draw3TextStrings(	IplImage	*theImage,
										const char	*textStr1,
										const char	*textStr2,
										const char	*textStr3)
{
CvPoint		point1;
int			imgWidth;
CvSize		textSize;
int			baseLine;

	if (theImage->roi != NULL)
	{
		imgWidth	=	theImage->roi->width;
	}
	else
	{
		imgWidth	=	theImage->width;
	}
	if (textStr1 != NULL)
	{
		point1.x	=	5;
		point1.y	=	15;

		cvPutText(	theImage,
					textStr1,
					point1,
					&cTextFont,
					cSideBarRed);
	}
	if (textStr2 != NULL)
	{
		cvGetTextSize(	textStr2,
						&cTextFont,
						&textSize,
						&baseLine);


		point1.x	=	(imgWidth - textSize.width) / 2;
		point1.y	=	15;

		cvPutText(	theImage,
					textStr2,
					point1,
					&cTextFont,
					cSideBarGrn);
	}
	if (textStr3 != NULL)
	{
		cvGetTextSize(	textStr3,
						&cTextFont,
						&textSize,
						&baseLine);

		point1.x	=	imgWidth - textSize.width;
		point1.x	-=	8;
		point1.y	=	15;

		cvPutText(	theImage,
					textStr3,
					point1,
					&cTextFont,
					cSideBarFCblue);
	}
}


//*****************************************************************************
void	CameraDriver::CreateHistogramGraph(IplImage *imageDisplay)
{
int			windowWidth;
int			windowHeight;
int			graphHeight;
int			xStart;
int			yDivideFactor;
CvRect		myCVrect;
int			yLoc;
char		textStr1[32];
char		textStr2[32];
char		textStr3[32];

//	CONSOLE_DEBUG(__FUNCTION__);
	if (imageDisplay != NULL)
	{
		if (imageDisplay->roi != NULL)
		{
			windowWidth		=	imageDisplay->roi->width;
			windowHeight	=	imageDisplay->roi->height;
		}
		else
		{
			windowWidth		=	imageDisplay->width;
			windowHeight	=	imageDisplay->height;
		}

		//*	first erase the image
		myCVrect.x		=	0;
		myCVrect.y		=	0;
		myCVrect.width	=	windowWidth;
		myCVrect.height	=	windowHeight;


		cvRectangleR(	imageDisplay,
						myCVrect,
						cSideBarBlk,				//	CvScalar color,
						CV_FILLED,					//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

		xStart			=	(windowWidth - 256) / 2;
		graphHeight		=	windowHeight - 40;
		yDivideFactor	=	cMaxHistogramPixCnt / graphHeight;
		yDivideFactor	+=	1;

//		CONSOLE_DEBUG_W_NUM("yDivideFactor\t\t=", yDivideFactor);

		yLoc	=	25;
		if (cROIinfo.currentROIimageType == kImageType_RGB24)
		{
		int	graphType;
			//*	draw the horizontal lines showing the max pixel value of that color

			graphType	=	kGraphTYpe_Log;
		//	graphType	=	kGraphTYpe_Line;

			//=======================================================
			//*	do the blue array
			DrawGraph256(	imageDisplay,
							cHistogramBlu,
							xStart,
							windowHeight,
							graphHeight,
							yDivideFactor,
							graphType,
							cSideBarFCblue);
			DrawHorizBar256(imageDisplay, xStart, yLoc, cMaxBluValue, cSideBarFCblue);
			yLoc	+=	5;

			//=======================================================
			//*	do the green array
			DrawGraph256(	imageDisplay,
							cHistogramGrn,
							xStart,
							windowHeight,
							graphHeight,
							yDivideFactor,
							graphType,
							cSideBarGrn);
			DrawHorizBar256(imageDisplay, xStart, yLoc, cMaxGrnValue, cSideBarGrn);
			yLoc	+=	5;

			//=======================================================
			//*	do the red array
			DrawGraph256(	imageDisplay,
							cHistogramRed,
							xStart,
							windowHeight,
							graphHeight,
							yDivideFactor,
							graphType,
							cSideBarRed);
			DrawHorizBar256(imageDisplay, xStart, yLoc, cMaxRedValue, cSideBarRed);
			yLoc	+=	5;

			sprintf(textStr1, "R=%d%%",	((cMaxRedValue * 100) / 255));
			sprintf(textStr2, "G=%d%%",	((cMaxGrnValue * 100) / 255));
			sprintf(textStr3, "B=%d%%",	((cMaxBluValue * 100) / 255));
			Draw3TextStrings(imageDisplay, textStr1, textStr2, textStr3);
		}
		else
		{
			DrawHorizBar256(imageDisplay, xStart, yLoc, cMaxGryValue, cSideBarGry);

			//=======================================================
			//*	do the overall luminance array
			DrawGraph256(	imageDisplay,
							cHistogramLum,
							xStart,
							windowHeight,
							graphHeight,
							yDivideFactor,
							kGraphTYpe_Log,
							cSideBarGry);

			sprintf(textStr1, "Gray=%d%%",	((cMaxGryValue * 100) / 255));
			Draw3TextStrings(imageDisplay, textStr1, NULL, NULL);
		}

	}
	else
	{
		CONSOLE_DEBUG("imageDisplay is NULL");
	}
}


//*****************************************************************************
void	CameraDriver::SetOpenCVcolors(IplImage *imageDisplay)
{
RGBcolor	bgc;	//*	background color
RGBcolor	txc;	//*	text color

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(cTS_info.refID);
	GetDefaultColors(0, cTS_info.refID, &bgc, &txc);

	if (imageDisplay != NULL)
	{
		if ((imageDisplay->depth == 8) && (imageDisplay->nChannels == 3))
		{
			cSideBarBGcolor		=	CV_RGB(bgc.red,	bgc.grn,	bgc.blu);
			cSideBarTXTcolor	=	CV_RGB(txc.red,	txc.grn,	txc.blu);
			cSideBarBlk			=	CV_RGB(0,		0,		0);
			cSideBarRed			=	CV_RGB(255,		0,		0);
			cSideBarGrn			=	CV_RGB(0,		255,	0);
			cSideBarBlu			=	CV_RGB(0,		0,		255);
			cSideBarFCblue		=	FC_BLUE();

			cSideBarGry			=	CV_RGB(128,		128,	128);
			cCrossHairColor		=	CV_RGB(255,		0,		0);
		}
		else if ((imageDisplay->depth == 16) && (imageDisplay->nChannels == 3))
		{
			cSideBarBGcolor		=	CV_RGB((bgc.red<<8),	(bgc.grn<<8),	(bgc.blu<<8));
			cSideBarTXTcolor	=	CV_RGB((txc.red<<8),	(txc.grn<<8),	(txc.blu<<8));
			cSideBarRed			=	CV_RGB(65535,	0,		0);
			cSideBarGrn			=	CV_RGB(0,		65535,	0);
			cSideBarBlu			=	CV_RGB(0,		0,		65535);

			cSideBarGry			=	CV_RGB(32000,	32000,	32000);
			cCrossHairColor		=	CV_RGB(65535,	0,		0);
		}
		else if (imageDisplay->depth == 16)
		{
			cSideBarBGcolor		=	CV_RGB((bgc.red<<8),	(bgc.grn<<8),	(bgc.blu<<8));
			cSideBarTXTcolor	=	CV_RGB((txc.red<<8),	(txc.grn<<8),	(txc.blu<<8));
			cSideBarRed			=	CV_RGB(65535,	0,		0);
			cSideBarGrn			=	CV_RGB(0,		65535,	0);
			cSideBarBlu			=	CV_RGB(0,		0,		65535);

			cSideBarGry			=	CV_RGB(32000,	32000,	32000);
			cCrossHairColor		=	CV_RGB(65535,	65535,	65535);
		}
		else
		{
			cSideBarBGcolor		=	CV_RGB(bgc.red,	bgc.grn,	bgc.blu);
			cSideBarTXTcolor	=	CV_RGB(txc.red,	txc.grn,	txc.blu);
			cSideBarBlk			=	CV_RGB(0,		0,		0);
			cSideBarRed			=	CV_RGB(255,		0,		0);
			cSideBarGrn			=	CV_RGB(0,		255,	0);
			cSideBarBlu			=	CV_RGB(0,		0,		255);
			cSideBarFCblue		=	FC_BLUE();

			cSideBarBGcolor		=	CV_RGB(128,		128,	128);
			cCrossHairColor		=	CV_RGB(255,		255,	255);
		}
	}
	else
	{
		CONSOLE_DEBUG("Image display is null, setting to RGB 8 bit values")
		//*	defaults
		cSideBarBGcolor		=	CV_RGB(bgc.red,	bgc.grn,	bgc.blu);
		cSideBarTXTcolor	=	CV_RGB(txc.red,	txc.grn,	txc.blu);
		cSideBarBlk			=	CV_RGB(0,		0,		0);
		cSideBarRed			=	CV_RGB(255,		0,		0);
		cSideBarGrn			=	CV_RGB(0,		255,	0);
		cSideBarBlu			=	CV_RGB(0,		0,		255);
		cSideBarFCblue		=	FC_BLUE();

		cSideBarGry			=	CV_RGB(128,		128,	128);
		cCrossHairColor		=	CV_RGB(255,		0,		0);
	}
	CONSOLE_DEBUG(__FUNCTION__);
}

#endif	//	defined(_ENABLE_CAMERA_) && defined(_USE_OPENCV_)
