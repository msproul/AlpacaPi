//*****************************************************************************
//*		windowtab_image.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:	C++ Client to talk to Alpaca devices
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
//*	Edit History
//*****************************************************************************
//*	Dec 29,	2020	<MLS> Created windowtab_image.cpp
//*	Feb  2,	2021	<MLS> Added the ability to zoom and scroll image by draging the mouse.
//*	Feb 28,	2021	<MLS> Working on image scrolling/dragging
//*	Feb 28,	2021	<MLS> Finally got image dragging to work properly
//*	Apr  5,	2021	<MLS> Zoomed in live view now working
//*****************************************************************************

#ifdef _ENABLE_CTRL_IMAGE_


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_image.h"

#include	"windowtab.h"
#include	"windowtab_image.h"




//**************************************************************************************
WindowTabImage::WindowTabImage(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCVdownLoadedImage	=	NULL;
	cOpenCVdisplayedImage	=	NULL;
	cImageZoomState			=	0;			//*	more state to be defined later

	cMouseDragInProgress	=	false;

	cImageCenterX			=	500;
	cImageCenterY			=	500;

	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabImage::~WindowTabImage(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCVdownLoadedImage != NULL)
	{
//		CONSOLE_DEBUG("destroy old image");
//		SetWidgetImage(kImageDisplay_ImageDisplay, NULL);
//		cvReleaseImage(&cOpenCVdownLoadedImage);
//		cOpenCVdownLoadedImage	=	NULL;
	}
}


//**************************************************************************************
void	WindowTabImage::SetupWindowControls(void)
{
int			xLoc;
int			yLoc;
int			save_yLoc;
int			iii;
int			imageBoxWidth;
int			imageBoxHeight;
int			labelWidth;
int			dataWidth;
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kImageDisplay_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kImageDisplay_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kImageDisplay_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//------------------------------------------
	xLoc	=	5;
	for (iii=kImageDisplay_Btn_1; iii<kImageDisplay_Btn_N; iii++)
	{
		SetWidget(				iii,	xLoc,	yLoc,		cTitleHeight,		cTitleHeight);
		SetWidgetType(			iii, 	kWidgetType_Button);

		xLoc	+=	cTitleHeight;
		xLoc	+=	2;
	}
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	save_yLoc		=	yLoc;

	//------------------------------------------
	xLoc		=	5;
	labelWidth	=	150;
	dataWidth	=	100;
	iii		=	kImageDisplay_FrameCnt_Lbl;
	while (iii < kImageDisplay_FramesSaved)
	{
		SetWidget(				iii,	xLoc,	yLoc,		labelWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Left);
		iii++;

		SetWidget(				iii,	(xLoc + labelWidth + 2),	yLoc,		dataWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Left);



		yLoc	+=	cSmallBtnHt;
		yLoc	+=	2;

		iii++;
	}
	SetWidgetJustification(	kImageDisplay_FrameCnt, 	kJustification_Center);

	SetWidgetText(kImageDisplay_FrameCnt_Lbl,		"Frames");
	SetWidgetText(kImageDisplay_Exposure_Lbl,		"Exposure");
	SetWidgetText(kImageDisplay_Gain_Lbl,			"Gain");
	SetWidgetText(kImageDisplay_CameraTemp_Lbl,		"CameraTemp");
	SetWidgetText(kImageDisplay_Filter_Lbl,			"Filter");
	SetWidgetText(kImageDisplay_Object_Lbl,			"Object");
	SetWidgetText(kImageDisplay_FramesSaved_Lbl,	"FramesSaved");


	SetWidgetText(kImageDisplay_Histogram,		"Histogram");

	yLoc			=	save_yLoc;
	xLoc			+=	labelWidth + dataWidth;
	xLoc			+=	2;
	imageBoxWidth	=	cWidth - xLoc;
	imageBoxHeight	=	cHeight - yLoc;
	imageBoxWidth	-=	2;
	imageBoxHeight	-=	2;
	SetWidget(				kImageDisplay_ImageDisplay,	xLoc,	yLoc,		imageBoxWidth,		imageBoxHeight);
	SetWidgetBGColor(		kImageDisplay_ImageDisplay,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kImageDisplay_ImageDisplay,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kImageDisplay_ImageDisplay,	true);

//	CONSOLE_DEBUG_W_NUM("imageBoxWidth\t=",		imageBoxWidth);
//	CONSOLE_DEBUG_W_NUM("imageBoxHeight\t=",	imageBoxHeight);

}

//*****************************************************************************
void	WindowTabImage::HandleKeyDown(const int keyPressed)
{
bool	updateFlag;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t",	keyPressed);

	updateFlag	=	true;
	switch(keyPressed & 0x7f)
	{
		case '0':
			ResetImage();
			updateFlag	=	false;
			break;

		case '4':
			cImageCenterX	-=	100;
			break;

		case '6':
			cImageCenterX	+=	100;
			break;

		case '2':
			cImageCenterY	+=	100;
			break;

		case '8':
			cImageCenterY	-=	100;
			break;

		case '7':
			cImageCenterX	-=	100;
			cImageCenterY	-=	100;
			break;

		case '9':
			cImageCenterX	+=	100;
			cImageCenterY	-=	100;
			break;

		case '1':
			cImageCenterX	-=	100;
			cImageCenterY	+=	100;
			break;

		case '3':
			cImageCenterX	+=	100;
			cImageCenterY	+=	100;
			break;

		default:
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
		DrawFullScaleIamge(cImageCenterX, cImageCenterY);
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessButtonClick(const int buttonIdx)
{

	switch(buttonIdx)
	{

		default:
//			CONSOLE_DEBUG(__FUNCTION__);
//			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kImageDisplay_ImageDisplay:
			CONSOLE_DEBUG("kImageDisplay_ImageDisplay");
			ZoomImage(event, xxx, yyy, flags);
			cMouseDragInProgress	=	false;
			break;

		default:
			//*	this adjusts the blue part of the image, just for testing.
			if (cOpenCVdisplayedImage != NULL)
			{
		#ifdef _USE_OPENCV_CPP_
			#warning "OpenCV++ not finished"
		#else
				for (iii= 0; iii< cOpenCVdisplayedImage->imageSize; iii+=3)
				{
					cOpenCVdisplayedImage->imageData[iii]	+=	75;
				}
		#endif // _USE_OPENCV_CPP_
				ForceUpdate();
			}
			else
			{
				CONSOLE_DEBUG("cOpenCVdisplayedImage is NULL");
			}
			break;
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonDown(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
int		cursorXXoffset;
int		cursorYYoffset;

//	CONSOLE_DEBUG("--------------------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xxx\t=", xxx);
//	CONSOLE_DEBUG_W_NUM("yyy\t=", yyy);

	if (widgetIdx == kImageDisplay_ImageDisplay)
	{
		cMouseDragInProgress	=	true;
		cursorXXoffset			=	xxx - cWidgetList[widgetIdx].roiRect.x;
		cursorYYoffset			=	yyy - cWidgetList[widgetIdx].roiRect.y;
//		CONSOLE_DEBUG_W_NUM("cursorXXoffset\t=", cursorXXoffset);
//		CONSOLE_DEBUG_W_NUM("cursorYYoffset\t=", cursorYYoffset);

		cSavedMouseClick_X		=	cursorXXoffset;
		cSavedMouseClick_Y		=	cursorYYoffset;

//		CONSOLE_DEBUG_W_NUM("cSavedMouseClick_X\t=", cSavedMouseClick_X);
//		CONSOLE_DEBUG_W_NUM("cSavedMouseClick_Y\t=", cSavedMouseClick_Y);
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonUp(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cMouseDragInProgress	=	false;
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonDragged(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
int		deltaXX;
int		deltaYY;
int		cursorXXoffset;
int		cursorYYoffset;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cMouseDragInProgress)
	{
		cImageZoomState			=	1;
//-		cMouseDragInProgress	=	true;
//		CONSOLE_DEBUG("--------------------------------------------------------");
//		CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);

//		if (widgetIdx == kImageDisplay_ImageDisplay)
		{

			cursorXXoffset	=	xxx - cWidgetList[kImageDisplay_ImageDisplay].roiRect.x;
			cursorYYoffset	=	yyy - cWidgetList[kImageDisplay_ImageDisplay].roiRect.y;

			deltaXX			=	cursorXXoffset - cSavedMouseClick_X;
			deltaYY			=	cursorYYoffset - cSavedMouseClick_Y;

//			CONSOLE_DEBUG_W_NUM("deltaXX\t=", deltaXX);
//			CONSOLE_DEBUG_W_NUM("deltaYY\t=", deltaYY);

			cImageCenterX	-=	deltaXX;
			cImageCenterY	-=	deltaYY;

			DrawFullScaleIamge(cImageCenterX, cImageCenterY);

			cSavedMouseClick_X	=	cursorXXoffset;
			cSavedMouseClick_Y	=	cursorYYoffset;

			ForceUpdate();

//-			cMouseDragInProgress	=	false;

		}
	}
}

#ifdef _USE_OPENCV_CPP_
//*****************************************************************************
void	WindowTabImage::SetImagePtrs(cv::Mat *originalImage, cv::Mat *displayedImage)
{
	cOpenCVdownLoadedImage	=	originalImage;
	cOpenCVdisplayedImage	=	displayedImage;
}
#else
//*****************************************************************************
void	WindowTabImage::SetImagePtrs(IplImage *originalImage, IplImage *displayedImage)
{
	cOpenCVdownLoadedImage	=	originalImage;
	cOpenCVdisplayedImage	=	displayedImage;
}
#endif // _USE_OPENCV_CPP_

//*****************************************************************************
void	WindowTabImage::ResetImage(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _USE_OPENCV_CPP_
	#warning "OpenCV++ not finished"
#else
	//*	Check to see if the original is color
	if ((cOpenCVdownLoadedImage->nChannels == 3) && (cOpenCVdownLoadedImage->depth == 8))
	{
		cvResize(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_INTER_LINEAR);
	}
	else if ((cOpenCVdownLoadedImage->nChannels == 1) && (cOpenCVdownLoadedImage->depth == 8))
	{
		cvCvtColor(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_GRAY2RGB);
	}
#endif // _USE_OPENCV_CPP_
	cImageZoomState	=	0;
	ForceUpdate();
}


//*****************************************************************************
void	WindowTabImage::ZoomImage(	const int	event,
									const int	xxx,
									const int	yyy,
									const int	flags)
{
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;
int			cursorXXoffset;
int			cursorYYoffset;
int			imageCursorXX;
int			imageCursorYY;

	CONSOLE_DEBUG(__FUNCTION__);

	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
		if (cImageZoomState != 0)
		{
			//*	set it back to fit on the screen
			CONSOLE_DEBUG("Calling ResetImage()");
			ResetImage();
		}
		else
		{
			cursorXXoffset		=	xxx - cWidgetList[kImageDisplay_ImageDisplay].roiRect.x;
			cursorYYoffset		=	yyy - cWidgetList[kImageDisplay_ImageDisplay].roiRect.y;

		#ifdef _USE_OPENCV_CPP_
			#warning "OpenCV++ not finished"
		#else
			//*	get the size of the destination image
			displayedWidth		=	cOpenCVdisplayedImage->width;
			displayedHeight		=	cOpenCVdisplayedImage->height;

			//*	get the size of the source image
			sourceImageWidth	=	cOpenCVdownLoadedImage->width;
			sourceImageHeight	=	cOpenCVdownLoadedImage->height;


			//*	calculate the relative position of the cursor WRT the full image
			imageCursorXX		=	sourceImageWidth * cursorXXoffset / displayedWidth;
			imageCursorYY		=	sourceImageHeight * cursorYYoffset / displayedHeight;

			DrawFullScaleIamge(imageCursorXX, imageCursorYY);
		#endif // _USE_OPENCV_CPP_

			cImageZoomState	=	1;
		}

		//ForceUpdate();
	}
}


//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(void)
{
	DrawFullScaleIamge(cImageCenterX, cImageCenterY);
}


//*****************************************************************************
//*	draw the image at full scale centered on these image coordinates
//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(const int image_X, const int	image_Y)
{
CvRect		displayedImgRect;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("image_X\t=", image_X);
//	CONSOLE_DEBUG_W_NUM("image_Y\t=", image_Y);

	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
	#ifdef _USE_OPENCV_CPP_
		#warning "OpenCV++ not finished"
	#else
		//*	get the size of the source image
		sourceImageWidth		=	cOpenCVdownLoadedImage->width;
		sourceImageHeight		=	cOpenCVdownLoadedImage->height;
//		CONSOLE_DEBUG_W_NUM("sourceImageWidth\t=", sourceImageWidth);
//		CONSOLE_DEBUG_W_NUM("sourceImageHeight\t=", sourceImageHeight);

		//*	get the size of the destination image
		displayedWidth			=	cOpenCVdisplayedImage->width;
		displayedHeight			=	cOpenCVdisplayedImage->height;

//		CONSOLE_DEBUG_W_NUM("displayedWidth\t=", displayedWidth);
//		CONSOLE_DEBUG_W_NUM("displayedHeight\t=", displayedHeight);

		if ((displayedWidth > 0) && (displayedWidth < 10000)
			&& (displayedHeight > 0) && (displayedHeight < 10000))
		{
			displayedImgRect.width	=	displayedWidth;
			displayedImgRect.height	=	displayedHeight;

			//*	now set the top left of the image
			displayedImgRect.x		=	image_X - (displayedWidth / 2);
			displayedImgRect.y		=	image_Y - (displayedHeight / 2);
			//*	check minimums
			if (displayedImgRect.x < 0)
			{
				displayedImgRect.x	=	0;
			}
			if (displayedImgRect.y < 0)
			{
				displayedImgRect.y	=	0;
			}

			//*	check maximums
			if (displayedImgRect.x > (sourceImageWidth - displayedWidth))
			{
				displayedImgRect.x	=	(sourceImageWidth - displayedWidth);
			}

			if (displayedImgRect.y > (sourceImageHeight - displayedHeight))
			{
				displayedImgRect.y	=	(sourceImageHeight - displayedHeight);
			}
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.x\t\t=", displayedImgRect.x);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.y\t\t=", displayedImgRect.y);

//			CONSOLE_DEBUG_W_NUM("displayedImgRect.width\t=", displayedImgRect.width);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.height\t=", displayedImgRect.height);

			//*	set the area we want to look at
			cvSetImageROI(cOpenCVdownLoadedImage,  displayedImgRect);
			//*	copy that part from the original to the displayed view
			cvCopy(cOpenCVdownLoadedImage, cOpenCVdisplayedImage);
			cvResetImageROI(cOpenCVdownLoadedImage);				//*	reset ROI

			//*	now update the CURRENT center of the displayed image
			cImageCenterX	=	displayedImgRect.x + (displayedWidth / 2);
			cImageCenterY	=	displayedImgRect.y + (displayedHeight / 2);
			ForceUpdate();
		}
		else
		{
			CONSOLE_DEBUG("Somethings not right!!!!!!!!!!!!");
		}
	#endif // _USE_OPENCV_CPP_
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}


#endif // _ENABLE_CTRL_IMAGE_
