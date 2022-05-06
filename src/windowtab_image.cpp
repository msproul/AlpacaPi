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
//*	Feb 26,	2022	<MLS> Image zooming working under opencv C++
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
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("xSize        \t=",	xSize);
	CONSOLE_DEBUG_W_NUM("ySize        \t=",	ySize);

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
int			boxWidth;
int			boxHeight;
int			histLblWidth;
int			hist_xLoc;
char		buttonName[16];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cWidth        \t=",	cWidth);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kImageDisplay_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kImageDisplay_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kImageDisplay_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//------------------------------------------
	//*	do the buttons
	xLoc			=	5;
	buttonName[0]	=	'A';
	buttonName[1]	=	0;
	for (iii=kImageDisplay_Btn_1; iii<kImageDisplay_Btn_N; iii++)
	{
		SetWidget(				iii,	xLoc,	yLoc,		cTitleHeight,		cTitleHeight);
		SetWidgetType(			iii, 	kWidgetType_Button);
		SetWidgetText(			iii,	buttonName);

		xLoc	+=	cTitleHeight;
		xLoc	+=	2;

		buttonName[0]++;
	}
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	save_yLoc		=	yLoc;

	//------------------------------------------
	xLoc		=	5;
	labelWidth	=	155;
	dataWidth	=	110;
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

	//-------------------------------------------------------------------------
	//*	set up the histogram
	//*	first 3 text boxes for the R,G,B percent values
	boxWidth		=	labelWidth + dataWidth;
	boxHeight		=	cSmallBtnHt * 4;
	histLblWidth	=	(boxWidth - 8) / 3;
	hist_xLoc		=	xLoc +2;
	iii		=	kImageDisplay_HistRedPerct;
	while (iii <= kImageDisplay_HistBluPerct)
	{
		SetWidget(				iii,	hist_xLoc,	yLoc,		histLblWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Center);
		SetWidgetBorder(		iii,	true);

		hist_xLoc	+=	histLblWidth;
		hist_xLoc	+=	2;
		iii++;
	}
	SetWidgetText(kImageDisplay_HistRedPerct,			"R=---%");
	SetWidgetText(kImageDisplay_HistGrnPerct,			"G=---%");
	SetWidgetText(kImageDisplay_HistBluPerct,			"B=---%");

	SetWidgetTextColor(kImageDisplay_HistRedPerct,		CV_RGB(255,		0,		0));
	SetWidgetTextColor(kImageDisplay_HistGrnPerct,		CV_RGB(0,		255,	0));
	SetWidgetTextColor(kImageDisplay_HistBluPerct,		CV_RGB(0x64,	0x8c,	0xff));


	yLoc	+=	cSmallBtnHt;
	yLoc	+=	2;
	SetWidget(		kImageDisplay_Histogram,	xLoc + 2,	yLoc,		boxWidth,		boxHeight);
	SetWidgetType(	kImageDisplay_Histogram, 	kWidgetType_CustomGraphic);
	SetWidgetText(	kImageDisplay_Histogram,	"Histogram");
	CONSOLE_DEBUG_W_NUM("boxWidth\t=",	boxWidth);
	CONSOLE_DEBUG_W_NUM("boxHeight\t=",	boxHeight);

	SetWidgetOutlineBox(kImageDisplay_HistogramOutline, kImageDisplay_HistRedPerct, (kImageDisplay_HistogramOutline -1));
//	CONSOLE_ABORT(__FUNCTION__);
//	kImageDisplay_HistRedPerct,
//	kImageDisplay_HistGrnPerct,
//	kImageDisplay_HistBluPerct,
//
//	kImageDisplay_Histogram,
//	kImageDisplay_HistogramOutline,

	//-------------------------------------------------------------------------
	//*	set up the image display area
	xLoc			+=	labelWidth + dataWidth;
	xLoc			+=	2;
	yLoc			=	save_yLoc;
	imageBoxWidth	=	cWidth - xLoc;
	imageBoxHeight	=	cHeight - yLoc;
	imageBoxWidth	-=	2;
	imageBoxHeight	-=	2;
	SetWidget(				kImageDisplay_ImageDisplay,	xLoc,	yLoc,		imageBoxWidth,		imageBoxHeight);
	SetWidgetBGColor(		kImageDisplay_ImageDisplay,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kImageDisplay_ImageDisplay,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kImageDisplay_ImageDisplay,	true);

	CONSOLE_DEBUG_W_NUM("cWidth        \t=",	cWidth);
	CONSOLE_DEBUG_W_NUM("xLoc          \t=",	xLoc);
	CONSOLE_DEBUG_W_NUM("yLoc          \t=",	yLoc);
	CONSOLE_DEBUG_W_NUM("imageBoxWidth \t=",	imageBoxWidth);
	CONSOLE_DEBUG_W_NUM("imageBoxHeight\t=",	imageBoxHeight);

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
		CONSOLE_DEBUG(__FUNCTION__);
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
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

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
			CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		#else
			int		iii;
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

	CONSOLE_DEBUG("--------------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("xxx\t=", xxx);
	CONSOLE_DEBUG_W_NUM("yyy\t=", yyy);

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

			CONSOLE_DEBUG(__FUNCTION__);
			DrawFullScaleIamge(cImageCenterX, cImageCenterY);

			cSavedMouseClick_X	=	cursorXXoffset;
			cSavedMouseClick_Y	=	cursorYYoffset;

			ForceUpdate();
		}
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabImage::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget)
{
//int		xLoc;
//int		yLoc;
//int		boxWidth;
//char	textStr1[32];
//char	textStr2[32];
//char	textStr3[32];

	cOpenCV_Image	=	openCV_Image;		//*	set the image data ptr so the window tabs can use it

//	xLoc		=	theWidget->left;
//	yLoc		=	theWidget->top;
//	boxWidth	=	theWidget->width;

//	sprintf(textStr1, "R=%d%%",	((66 * 100) / 255));
//	sprintf(textStr2, "G=%d%%",	((190 * 100) / 255));
//	sprintf(textStr3, "B=%d%%",	((15 * 100) / 255));
//
//	yLoc	+=	20;
//	LLD_SetColor(W_RED);
//	LLD_DrawCString(xLoc, yLoc, textStr1, kFont_Medium);
//	LLD_SetColor(W_GREEN);
//	LLD_DrawCString(xLoc + (boxWidth /2 ), yLoc, textStr2, kFont_Medium);
//	LLD_SetColor(W_BLUE);
//	LLD_DrawCString(xLoc, yLoc, textStr3, kFont_Medium);


//	if (cROIinfo.currentROIimageType == kImageType_RGB24)
//	{
//
//
//	}

}
#else
//**************************************************************************************
void	WindowTabImage::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
{
	//*	this routine should be overloaded
}
#endif // _USE_OPENCV_CPP_


#ifdef _USE_OPENCV_CPP_
//*****************************************************************************
void	WindowTabImage::SetImagePtrs(cv::Mat *originalImage, cv::Mat *displayedImage)
{
	CONSOLE_DEBUG(__FUNCTION__);
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
	//*	Check to see if the original is color
//	if ((cOpenCVdownLoadedImage->nChannels == 3) && (cOpenCVdownLoadedImage->depth == 8))
	if ((cOpenCVdownLoadedImage->step[1] == 3))
	{
	//	cvResize(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_INTER_LINEAR);
		cv::resize(	*cOpenCVdownLoadedImage,
					*cOpenCVdisplayedImage,
					cOpenCVdisplayedImage->size(),
					0,
					0,
					cv::INTER_LINEAR);
	}
//	else if ((cOpenCVdownLoadedImage->nChannels == 1) && (cOpenCVdownLoadedImage->depth == 8))
	else if ((cOpenCVdownLoadedImage->step[1] == 1))
	{
		CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//		cvCvtColor(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_GRAY2RGB);
	}
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
int			cursorXXoffset;
int			cursorYYoffset;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;			//*	get the size of the destination image
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
			//*	get the size of the displayed image
			displayedWidth		=	cOpenCVdisplayedImage->cols;
			displayedHeight		=	cOpenCVdisplayedImage->rows;

			//*	get the size of the source image
			sourceImageWidth	=	cOpenCVdownLoadedImage->cols;
			sourceImageHeight	=	cOpenCVdownLoadedImage->rows;
		#else
			//*	get the size of the displayed image
			displayedWidth		=	cOpenCVdisplayedImage->width;
			displayedHeight		=	cOpenCVdisplayedImage->height;

			//*	get the size of the source image
			sourceImageWidth	=	cOpenCVdownLoadedImage->width;
			sourceImageHeight	=	cOpenCVdownLoadedImage->height;
		#endif // _USE_OPENCV_CPP_


			//*	calculate the relative position of the cursor WRT the full image
			imageCursorXX		=	sourceImageWidth * cursorXXoffset / displayedWidth;
			imageCursorYY		=	sourceImageHeight * cursorYYoffset / displayedHeight;

			CONSOLE_DEBUG(__FUNCTION__);
			DrawFullScaleIamge(imageCursorXX, imageCursorYY);

			cImageZoomState	=	1;
		}

		//ForceUpdate();
	}
}


//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	DrawFullScaleIamge(cImageCenterX, cImageCenterY);
}


//*****************************************************************************
//*	draw the image at full scale centered on these image coordinates
//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(const int image_X, const int	image_Y)
{
cv::Rect	displayedImgRect;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;

//	CONSOLE_DEBUG("--------------------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("image_X\t=", image_X);
//	CONSOLE_DEBUG_W_NUM("image_Y\t=", image_Y);

	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
	#ifdef _USE_OPENCV_CPP_

		//*	get the size of the source image
		sourceImageWidth		=	cOpenCVdownLoadedImage->cols;
		sourceImageHeight		=	cOpenCVdownLoadedImage->rows;

		//*	get the size of the destination image
		displayedWidth			=	cOpenCVdisplayedImage->cols;
		displayedHeight			=	cOpenCVdisplayedImage->rows;
	#else
		//*	get the size of the source image
		sourceImageWidth		=	cOpenCVdownLoadedImage->width;
		sourceImageHeight		=	cOpenCVdownLoadedImage->height;

		//*	get the size of the destination image
		displayedWidth			=	cOpenCVdisplayedImage->width;
		displayedHeight			=	cOpenCVdisplayedImage->height;
	#endif // _USE_OPENCV_CPP_

//		CONSOLE_DEBUG_W_NUM("sourceImageWidth\t=",	sourceImageWidth);
//		CONSOLE_DEBUG_W_NUM("sourceImageHeight\t=",	sourceImageHeight);
//		CONSOLE_DEBUG_W_NUM("displayedWidth\t=",	displayedWidth);
//		CONSOLE_DEBUG_W_NUM("displayedHeight\t=",	displayedHeight);

		if ((displayedWidth > 0) && (displayedWidth < 10000)
			&& (displayedHeight > 0) && (displayedHeight < 10000))
		{
			displayedImgRect.width	=	displayedWidth;
			displayedImgRect.height	=	displayedHeight;

			//*	now set the top left of the image
			displayedImgRect.x		=	image_X - (displayedWidth / 2);
			displayedImgRect.y		=	image_Y - (displayedHeight / 2);

//			CONSOLE_DEBUG("First pass");
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.x\t\t=",		displayedImgRect.x);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.y\t\t=",		displayedImgRect.y);

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
//			CONSOLE_DEBUG("Adjusted");
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.x\t\t=",		displayedImgRect.x);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.y\t\t=",		displayedImgRect.y);
//
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.width\t=",	displayedImgRect.width);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.height\t=",	displayedImgRect.height);

		#ifdef _USE_OPENCV_CPP_
cv::Mat		image_roi;
			CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

			image_roi	=	cv::Mat(*cOpenCVdownLoadedImage, displayedImgRect);

//			cvSetImageROI(cOpenCVdownLoadedImage,  theWidget->roiRect);
//			cvCopy(theWidget->openCVimagePtr, cOpenCVdownLoadedImage);
//			cvResetImageROI(cOpenCVdownLoadedImage);

			//---try------try------try------try------try------try---
			try
			{
				image_roi.copyTo(*cOpenCVdisplayedImage);
//				cv::waitKey(100);
			}
			catch(cv::Exception& ex)
			{
				//*	this catch prevents opencv from crashing
				CONSOLE_DEBUG("????????????????????????????????????????????????????");
				CONSOLE_DEBUG("copyTo() had an exception");
				CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
			//	CONSOLE_ABORT(__FUNCTION__);
			}

		#else
			//*	set the area we want to look at
			cvSetImageROI(cOpenCVdownLoadedImage,  displayedImgRect);
			//*	copy that part from the original to the displayed view
			cvCopy(cOpenCVdownLoadedImage, cOpenCVdisplayedImage);
			cvResetImageROI(cOpenCVdownLoadedImage);				//*	reset ROI
		#endif
			//*	now update the CURRENT center of the displayed image
			cImageCenterX	=	displayedImgRect.x + (displayedWidth / 2);
			cImageCenterY	=	displayedImgRect.y + (displayedHeight / 2);
			ForceUpdate();
		}
		else
		{
			CONSOLE_DEBUG("Somethings not right!!!!!!!!!!!!");
			CONSOLE_DEBUG("cOpenCVdownLoadedImage is NULL or cOpenCVdisplayedImage is NULL");
		}
	}
	else
	{
		CONSOLE_DEBUG("Not finished");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}


#endif // _ENABLE_CTRL_IMAGE_
