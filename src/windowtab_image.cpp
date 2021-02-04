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
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCVdownLoadedImage	=	NULL;
	cOpenCVdisplayedImage	=	NULL;
	cImageZoomState			=	0;			//*	more state to be defined later
	//*	save these for image dragging.
	cZoomTopLeft.x			=	0;
	cZoomTopLeft.y			=	0;
	cMouseDragInProgress	=	false;


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
int			yLocSave;
int			iii;



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


	SetWidget(				kImageDisplay_ImageDisplay,	0,	yLoc,		cWidth,		((3 * cWidth) / 7));
	SetWidgetBGColor(		kImageDisplay_ImageDisplay,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kImageDisplay_ImageDisplay,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kImageDisplay_ImageDisplay,	true);
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
void	WindowTabImage::ProcessDoubleClick(const int buttonIdx)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(buttonIdx)
	{
		case kImageDisplay_ImageDisplay:
			CONSOLE_DEBUG(__FUNCTION__);
			ZoomImage();
			cMouseDragInProgress	=	false;
			break;

		default:
			if (cOpenCVdisplayedImage != NULL)
			{
				for (iii= 0; iii< cOpenCVdisplayedImage->imageSize; iii+=3)
				{
					cOpenCVdisplayedImage->imageData[iii]	+=	75;
				}
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
void	WindowTabImage::ProcessMouseLeftButtonDown(	const int	widgitIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{

	if (widgitIdx == kImageDisplay_ImageDisplay)
	{
		cSavedMouseClick_X	=	xxx;
		cSavedMouseClick_Y	=	yyy;

	}
}


//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonUp(	const int	widgitIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cMouseDragInProgress	=	false;
}



//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonDragged(	const int	widgitIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
double		deltaXX;
double		deltaYY;
double		moveAmount;
CvRect		sourceImgROIrect;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;

	CONSOLE_DEBUG(__FUNCTION__);

//	if (cMouseDragInProgress == false)
	{
		cMouseDragInProgress	=	true;
//		CONSOLE_DEBUG("--------------------------------------------------------");
		CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);

//		if (widgitIdx == kImageDisplay_ImageDisplay)
		{

			deltaXX	=	xxx - cSavedMouseClick_X;
			deltaYY	=	yyy - cSavedMouseClick_Y;

			cZoomTopLeft.x	-=	deltaXX;
			cZoomTopLeft.y	-=	deltaYY;


			//*	get the size of the destination image
			displayedWidth			=	cOpenCVdisplayedImage->width;
			displayedHeight			=	cOpenCVdisplayedImage->height;

			//*	get the size of the source image
			sourceImageWidth		=	cOpenCVdownLoadedImage->width;
			sourceImageHeight		=	cOpenCVdownLoadedImage->height;


			//*	check the boundaries
			if (cZoomTopLeft.x < 0)
			{
				CONSOLE_DEBUG("Too far left");
				cZoomTopLeft.x	=	0;
			}
			if (cZoomTopLeft.y < 0)
			{
				CONSOLE_DEBUG("Too far up");
				cZoomTopLeft.y	=	0;
			}

			if (cZoomTopLeft.x > (sourceImageWidth - displayedWidth))
			{
				CONSOLE_DEBUG("Too far right");
				cZoomTopLeft.x	=	sourceImageWidth - displayedWidth;
			}
			if (cZoomTopLeft.y > (sourceImageHeight - displayedHeight))
			{
				CONSOLE_DEBUG("Too far down");
				cZoomTopLeft.y	=	sourceImageHeight - displayedHeight;
			}

//			CONSOLE_DEBUG_W_NUM("cZoomTopLeft.x\t=", cZoomTopLeft.x);

			sourceImgROIrect.x		=	cZoomTopLeft.x;
			sourceImgROIrect.y		=	cZoomTopLeft.y;
			sourceImgROIrect.width	=	cOpenCVdisplayedImage->width;
			sourceImgROIrect.height	=	cOpenCVdisplayedImage->height;



			cvSetImageROI(cOpenCVdownLoadedImage,  sourceImgROIrect);
			cvCopy(cOpenCVdownLoadedImage, cOpenCVdisplayedImage);
			cvResetImageROI(cOpenCVdownLoadedImage);


			cSavedMouseClick_X	=	xxx;
			cSavedMouseClick_Y	=	yyy;

			ForceUpdate();
			cMouseDragInProgress	=	false;

		}
	}
}



//*****************************************************************************
void	WindowTabImage::SetImagePtrs(IplImage *originalImage, IplImage *displayedImage)
{
	cOpenCVdownLoadedImage	=	originalImage;
	cOpenCVdisplayedImage	=	displayedImage;

}



//*****************************************************************************
void	WindowTabImage::ZoomImage(void)
{
CvRect		displayedImgRect;

	CONSOLE_DEBUG(__FUNCTION__);

	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
		if (cImageZoomState != 0)
		{
			//*	set it back to fit on the screen

			//*	Check to see if the original is color
			if ((cOpenCVdownLoadedImage->nChannels == 3) && (cOpenCVdownLoadedImage->depth == 8))
			{
				cvResize(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_INTER_LINEAR);
			}
			else if ((cOpenCVdownLoadedImage->nChannels == 1) && (cOpenCVdownLoadedImage->depth == 8))
			{
				cvCvtColor(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_GRAY2RGB);
			}
			cImageZoomState	=	0;
		}
		else
		{
			//*	first get the size of the displayed image
			displayedImgRect.x		=	0;
			displayedImgRect.y		=	0;
			displayedImgRect.width	=	cOpenCVdisplayedImage->width;
			displayedImgRect.height	=	cOpenCVdisplayedImage->height;

			cvSetImageROI(cOpenCVdownLoadedImage,  displayedImgRect);
			cvCopy(cOpenCVdownLoadedImage, cOpenCVdisplayedImage);
			cvResetImageROI(cOpenCVdownLoadedImage);

			cImageZoomState	=	1;

		}

		ForceUpdate();
	}
}


#endif // _ENABLE_CTRL_IMAGE_
