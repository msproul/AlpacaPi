//*****************************************************************************
//*		controller_image.cpp		(c) 2020 by Mark Sproul
//*
//*
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 27,	2020	<MLS> Created controller_image.cpp
//*	Apr  2,	2021	<MLS> Added SetLiveWindowImage()
//*	Apr  2,	2021	<MLS> Added UpdateLiveWindowImage()
//*	Apr  3,	2021	<MLS> Added UpdateLiveWindowInfo()
//*	Apr  8,	2021	<MLS> Added CopyImageToLiveImage()
//*	Feb 25,	2022	<MLS> SetLiveWindowImage() working under C++ opencv
//*	Mar 12,	2022	<MLS> Added SetImageWindowInfo()
//*	Mar 12,	2022	<MLS> Added TYPE_BinaryImageHdr struct to Image controller
//*****************************************************************************

#ifdef _ENABLE_CTRL_IMAGE_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"helper_functions.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	1100
#define	kWindowHeight	800

#include	"alpaca_defs.h"
#include	"windowtab_image.h"
#include	"windowtab_about.h"


#include	"controller.h"
#include	"controller_image.h"




extern char	gFullVersionString[];


//**************************************************************************************
enum
{
	kTab_Image	=	1,
	kTab_About,

	kTab_Count

};


#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
ControllerImage::ControllerImage(	const char	*argWindowName,
									cv::Mat		*downloadedImage,
									TYPE_BinaryImageHdr	*binaryImageHdr)
			:Controller(	argWindowName,
							kWindowWidth,
							kWindowHeight)
#else
//**************************************************************************************
ControllerImage::ControllerImage(	const char	*argWindowName,
									IplImage	*downloadedImage,
									TYPE_BinaryImageHdr	*binaryImageHdr)
			:Controller(	argWindowName,
							kWindowWidth,
							kWindowHeight)
#endif // _USE_OPENCV_CPP_
{

	CONSOLE_DEBUG(__FUNCTION__);

	cDownLoadedImage	=	NULL;
	cDisplayedImage		=	NULL;
	cColorImage			=	NULL;

	cImageTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr	=	NULL;

	//*	deal with the binary image header if it was supplied
	memset((void *)&cBinaryImageHdr, 0, sizeof(TYPE_BinaryImageHdr));
	if (binaryImageHdr != NULL)
	{
	char	elementTypeStr[32];
	char	transmitTypeStr[32];

		cBinaryImageHdr	=	*binaryImageHdr;

		GetBinaryElementTypeString(cBinaryImageHdr.ImageElementType,		elementTypeStr);
		GetBinaryElementTypeString(cBinaryImageHdr.TransmissionElementType,	transmitTypeStr);

		CONSOLE_DEBUG_W_NUM("MetadataVersion        \t=",	cBinaryImageHdr.MetadataVersion);
		CONSOLE_DEBUG_W_NUM("ErrorNumber            \t=",	cBinaryImageHdr.ErrorNumber);
		CONSOLE_DEBUG_W_NUM("ClientTransactionID    \t=",	cBinaryImageHdr.ClientTransactionID);
		CONSOLE_DEBUG_W_NUM("ServerTransactionID    \t=",	cBinaryImageHdr.ServerTransactionID);
		CONSOLE_DEBUG_W_NUM("DataStart              \t=",	cBinaryImageHdr.DataStart);
		CONSOLE_DEBUG_W_STR("ImageElementType       \t=",	elementTypeStr);
		CONSOLE_DEBUG_W_STR("TransmissionElementType\t=",	transmitTypeStr);
		CONSOLE_DEBUG_W_NUM("Rank                   \t=",	cBinaryImageHdr.Rank);
		CONSOLE_DEBUG_W_NUM("Dimension1             \t=",	cBinaryImageHdr.Dimension1);
		CONSOLE_DEBUG_W_NUM("Dimension2             \t=",	cBinaryImageHdr.Dimension2);
		CONSOLE_DEBUG_W_NUM("Dimension3             \t=",	cBinaryImageHdr.Dimension3);
	}


	SetupWindowControls();

	//*	the downloaded image needs to be copied and/or resized to the displayed image
	if (downloadedImage != NULL)
	{
		SetLiveWindowImage(downloadedImage);
	}
	else
	{
		CONSOLE_DEBUG("No image was specfified");
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerImage::~ControllerImage(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, NULL);
#ifdef _USE_OPENCV_CPP_
//++	#warning "OpenCV++ not tested"
	CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//*	free up the image memory
	if (cDownLoadedImage != NULL)
	{
		delete cDownLoadedImage;
		cDownLoadedImage	=	NULL;
	}
	if (cDisplayedImage != NULL)
	{
		delete cDisplayedImage;
		cDisplayedImage	=	NULL;
	}
#else
	//--------------------------------------------
	//*	free up the image memory
	if (cDownLoadedImage != NULL)
	{
//		CONSOLE_DEBUG_W_LHEX("Release cDownLoadedImage", (unsigned long)cDownLoadedImage);
		cvReleaseImage(&cDownLoadedImage);
		cDownLoadedImage	=	NULL;
	}
	if (cDisplayedImage != NULL)
	{
//		CONSOLE_DEBUG_W_LHEX("Release cDisplayedImage", (unsigned long)cDisplayedImage);
		cvReleaseImage(&cDisplayedImage);
		cDisplayedImage	=	NULL;
	}
#endif // _USE_OPENCV_CPP_

	//--------------------------------------------
	//*	delete the window tab objects
	DELETE_OBJ_IF_VALID(cImageTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerImage::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);

	SetTabText(kTab_Image,		"Image");
	cImageTabObjPtr		=	new WindowTabImage(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cImageTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Image,	cImageTabObjPtr);
		cImageTabObjPtr->SetParentObjectPtr(this);
	}

	SetTabText(kTab_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//**************************************************************************************
void	ControllerImage::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		needToUpdate;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_STR("Called from:", callingFunction);
	if (cReadStartup)
	{
//		AlpacaGetStartupData();
		cReadStartup	=	false;
	}

	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;


	if (cForceAlpacaUpdate)
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}
	else if (deltaSeconds >= 2)
	{
		needToUpdate	=	true;
	}

	if (needToUpdate)
	{
		cLastUpdate_milliSecs	=	millis();
	}

	if (cImageTabObjPtr != NULL)
	{
		cImageTabObjPtr->RunBackgroundTasks();
	}
}

//**************************************************************************************
void	ControllerImage::RefreshWindow(void)
{
	HandleWindowUpdate();
	cv::waitKey(100);
}

//**************************************************************************************
void	ControllerImage::DrawWidgetImage(TYPE_WIDGET *theWidget)
{
	if (cImageTabObjPtr != NULL)
	{
		if (cImageTabObjPtr->cImageZoomState)
		{
			CONSOLE_DEBUG("Zoomed");
			cImageTabObjPtr->DrawFullScaleIamge();
			Controller::DrawWidgetImage(theWidget, cImageTabObjPtr->cOpenCVdisplayedImage);

		}
		else
		{
			CONSOLE_DEBUG("Normal");
			Controller::DrawWidgetImage(theWidget);
		}
	}
	else
	{
		CONSOLE_DEBUG("cImageTabObjPtr is NULL");
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	ControllerImage::SetLiveWindowImage(cv::Mat *newOpenCVImage)
{
int		smallDispalyWidth;
int		smallDisplayHeight;
int		reduceFactor;
int		newImgWidth;
int		newImgHeight;
int		newImgChannels;
int		newImgRowStepSize;
bool	validImg;
size_t	byteCount;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cDownLoadedImage != NULL)
	{
	//	cvReleaseImage(&cDownLoadedImage);
		delete cDownLoadedImage;
		cDownLoadedImage	=	NULL;
	}
	if (cDisplayedImage != NULL)
	{
	//	cvReleaseImage(&cDisplayedImage);
		delete cDisplayedImage;
		cDisplayedImage	=	NULL;
	}

	if (newOpenCVImage != NULL)
	{
		//*	ok, now its time to CREATE our own image, we are going to make it the same as the
		//*	supplied image
		//	https://docs.opencv.org/3.4/d3/d63/classcv_1_1Mat.html
		newImgWidth			=	newOpenCVImage->cols;
		newImgHeight		=	newOpenCVImage->rows;
		newImgRowStepSize	=	newOpenCVImage->step[0];
		newImgChannels		=	newOpenCVImage->step[1];
		validImg			=	true;
		if ((newImgWidth < 100) || (newImgWidth > 10000))
		{
			validImg		=	false;
		}
		if ((newImgHeight < 100) || (newImgHeight > 10000))
		{
			validImg		=	false;
		}
		if ((newImgChannels != 1) && (newImgChannels != 3))
		{
			validImg		=	false;
		}
		if (validImg)
		{
			CONSOLE_DEBUG_W_NUM("newImgChannels\t=", newImgChannels);
			newImgChannels		=	3;
			cDownLoadedImage	=	new cv::Mat(cv::Size(	newImgWidth,
															newImgHeight),
															CV_8UC3);
		}
		else
		{
			CONSOLE_DEBUG("Image parameters invalid !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			CONSOLE_DEBUG_W_NUM("newImgWidth   \t=",	newImgWidth);
			CONSOLE_DEBUG_W_NUM("newImgHeight  \t=",	newImgHeight);
			CONSOLE_DEBUG_W_NUM("newImgChannels\t=",	newImgChannels);
			CONSOLE_ABORT(__FUNCTION__);
		}

		//*	the downloaded image needs to be copied and/or resized to the displayed image
		if (cDownLoadedImage != NULL)
		{
			//*	copy the image data to OUR image
			byteCount	=	newOpenCVImage->rows * newImgRowStepSize;
			if ((cDownLoadedImage->data != NULL) && (newOpenCVImage->data != NULL))
			{
				memcpy(cDownLoadedImage->data, newOpenCVImage->data, byteCount);
			}
			CONSOLE_DEBUG("Creating small image");
			reduceFactor		=	1;
			smallDispalyWidth	=	cDownLoadedImage->cols;
			smallDisplayHeight	=	cDownLoadedImage->rows;

			CONSOLE_DEBUG_W_NUM("cDownLoadedImage->cols\t=",	cDownLoadedImage->cols);
			CONSOLE_DEBUG_W_NUM("cDownLoadedImage->rows\t=",	cDownLoadedImage->rows);


			int		maxWindowWidth	=	800;
			int		maxWindowHeight	=	700;

			while ((smallDispalyWidth > maxWindowWidth) || (smallDisplayHeight > (maxWindowHeight - 50)))
			{
				CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);
				reduceFactor++;
				smallDispalyWidth	=	cDownLoadedImage->cols / reduceFactor;
				smallDisplayHeight	=	cDownLoadedImage->rows / reduceFactor;
			}
			CONSOLE_DEBUG_W_NUM("reduceFactor\t=", reduceFactor);
			CONSOLE_DEBUG_W_NUM("smallDispalyWidth\t=", smallDispalyWidth);
			CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);

			cDisplayedImage	=	new cv::Mat(cv::Size(	smallDispalyWidth,
														smallDisplayHeight),
														CV_8UC3);

// 300 [DumpCVMatStruct     ] theImageMat->cols		= 1200
// 301 [DumpCVMatStruct     ] theImageMat->rows		= 800
// 302 [DumpCVMatStruct     ] theImageMat->dims		= 2
// 303 [DumpCVMatStruct     ] theImageMat->step[0]	= 3600
// 304 [DumpCVMatStruct     ] theImageMat->step[1]	= 3
// 305 [DumpCVMatStruct     ] theImageMat->step[2]	= 0

			if (cDisplayedImage != NULL)
			{
				CONSOLE_DEBUG("Resizing image");

				//*	Check to see if the original is color
				if ((cDownLoadedImage->step[1] == 3))
				{
					CONSOLE_DEBUG("Original is 8 bit color (3 channels)");
//					cvResize(cDownLoadedImage, cDisplayedImage, CV_INTER_LINEAR);
					cv::resize(	*cDownLoadedImage,
								*cDisplayedImage,
								cDisplayedImage->size(),
								0,
								0,
								cv::INTER_LINEAR);
				}
//				else if ((cDownLoadedImage->nChannels == 1) && (cDownLoadedImage->depth == 8))
				else if ((cDownLoadedImage->step[1] == 1))
				{
					CONSOLE_DEBUG("Original is 8 bit B/W");
					CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//					cvCvtColor(cDownLoadedImage, cDisplayedImage, CV_GRAY2RGB);
				}
//
				SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, cDisplayedImage);
			}
			else
			{
				CONSOLE_DEBUG("Failed to create new image");
			}

			if (cImageTabObjPtr != NULL)
			{
				cImageTabObjPtr->SetImagePtrs(cDownLoadedImage,	cDisplayedImage);
			}
		}
		else
		{
			CONSOLE_DEBUG("Error creating image (new cv::Mat)");
		}
		//*	Update the image size on the screen
		SetImageWindowInfo();
	}
}
#else
//**************************************************************************************
void	ControllerImage::SetLiveWindowImage(IplImage *newOpenCVImage)
{
int		smallDispalyWidth;
int		smallDisplayHeight;
int		reduceFactor;
int		newImgWidth;
int		newImgHeight;
int		newImgChannels;
bool	validImg;
size_t	byteCount;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cDownLoadedImage != NULL)
	{
		cvReleaseImage(&cDownLoadedImage);
		cDownLoadedImage	=	NULL;
	}
	if (cDisplayedImage != NULL)
	{
		cvReleaseImage(&cDisplayedImage);
		cDisplayedImage	=	NULL;
	}

	if (newOpenCVImage != NULL)
	{
		//*	ok, now its time to CREATE our own image, we are going to make it the same as the
		//*	supplied image
		newImgWidth			=	newOpenCVImage->width;
		newImgHeight		=	newOpenCVImage->height;
		newImgChannels		=	newOpenCVImage->nChannels;

		validImg			=	true;
		if ((newImgWidth < 100) || (newImgWidth > 10000))
		{
			validImg		=	false;
		}
		if ((newImgHeight < 100) || (newImgHeight > 10000))
		{
			validImg		=	false;
		}
		if ((newImgChannels != 1) && (newImgChannels != 3))
		{
			validImg		=	false;
		}
		if (validImg)
		{
			CONSOLE_DEBUG_W_NUM("newImgChannels\t=", newImgChannels);
			newImgChannels		=	3;
			cDownLoadedImage	=	cvCreateImage(cvSize(	newImgWidth,
															newImgHeight),
															IPL_DEPTH_8U,
															newImgChannels);
		}

		//*	the downloaded image needs to be copied and/or resized to the displayed image
		if (cDownLoadedImage != NULL)
		{
			//*	copy the image data to OUR image
			byteCount	=	newOpenCVImage->height * newOpenCVImage->widthStep;
			memcpy(cDownLoadedImage->imageData, newOpenCVImage->imageData, byteCount);


//			CONSOLE_DEBUG("Creating small image");
			reduceFactor		=	1;
			smallDispalyWidth	=	cDownLoadedImage->width;
			smallDisplayHeight	=	cDownLoadedImage->height;

			CONSOLE_DEBUG_W_NUM("cDownLoadedImage->width\t=",	cDownLoadedImage->width);
			CONSOLE_DEBUG_W_NUM("cDownLoadedImage->height\t=",	cDownLoadedImage->height);


			int		maxWindowWidth	=	800;
			int		maxWindowHeight	=	700;

			while ((smallDispalyWidth > maxWindowWidth) || (smallDisplayHeight > (maxWindowHeight - 50)))
			{
				CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);
				reduceFactor++;
				smallDispalyWidth	=	cDownLoadedImage->width / reduceFactor;
				smallDisplayHeight	=	cDownLoadedImage->height / reduceFactor;
			}
			CONSOLE_DEBUG_W_NUM("reduceFactor\t=", reduceFactor);
			CONSOLE_DEBUG_W_NUM("smallDispalyWidth\t=", smallDispalyWidth);
			CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);

			cDisplayedImage	=	cvCreateImage(cvSize(	smallDispalyWidth,
														smallDisplayHeight),
														IPL_DEPTH_8U,
														3);
			if (cDisplayedImage != NULL)
			{
				CONSOLE_DEBUG("Resizing image");

				//*	Check to see if the original is color
				if ((cDownLoadedImage->nChannels == 3) && (cDownLoadedImage->depth == 8))
				{
					CONSOLE_DEBUG("Original is 8 bit color (3 channels)");
					cvResize(cDownLoadedImage, cDisplayedImage, CV_INTER_LINEAR);
				}
				else if ((cDownLoadedImage->nChannels == 1) && (cDownLoadedImage->depth == 8))
				{
					CONSOLE_DEBUG("Original is 8 bit B/W");
					cvCvtColor(cDownLoadedImage, cDisplayedImage, CV_GRAY2RGB);
				}

				SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, cDisplayedImage);
			}
			else
			{
				CONSOLE_DEBUG("Failed to create new image");
			}

			if (cImageTabObjPtr != NULL)
			{
				cImageTabObjPtr->SetImagePtrs(cDownLoadedImage,	cDisplayedImage);
			}
		}
		else
		{

		}
		//*	Update the image size on the screen
		SetImageWindowInfo();
	}
}
#endif // _USE_OPENCV_CPP_

//**************************************************************************************
void	ControllerImage::SetImageWindowInfo(void)
{
char	textString[64];
int		imgWidth;
int		imgHeight;
int		imgChannels;

#ifdef _USE_OPENCV_CPP_
	imgWidth	=	cDownLoadedImage->cols;
	imgHeight	=	cDownLoadedImage->rows;
	imgChannels	=	cDownLoadedImage->step[1];

#else
	imgWidth	=	cDownLoadedImage->width;
	imgHeight	=	cDownLoadedImage->height;
	imgChannels	=	cDownLoadedImage->nChannels;
#endif // _USE_OPENCV_CPP_

	sprintf(textString, "%4d x %4d", imgWidth, imgHeight);
	SetWidgetText(kTab_Image, kImageDisplay_FrameCnt, textString);

	sprintf(textString, "ch = %d", imgChannels);
	SetWidgetText(kTab_Image, kImageDisplay_Exposure, textString);
}
#ifdef _USE_OPENCV_CPP_
#warning "OpenCV++ not finished"
//**************************************************************************************
void	ControllerImage::CopyImageToLiveImage(cv::Mat *newOpenCVImage)
{
	CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this is just an extra check, it was crashing on testing
	if ((cDownLoadedImage != NULL) && (newOpenCVImage != NULL))
	{
	}
}
#else
//**************************************************************************************
void	ControllerImage::CopyImageToLiveImage(IplImage *newOpenCVImage)
{
size_t				byteCount_src;
size_t				byteCount_dst;

	CONSOLE_DEBUG(__FUNCTION__);
	//*	this is just an extra check, it was crashing on testing
	if ((cDownLoadedImage != NULL) && (newOpenCVImage != NULL))
	{
		byteCount_src	=	newOpenCVImage->height * newOpenCVImage->widthStep;
		byteCount_dst	=	cDownLoadedImage->height * cDownLoadedImage->widthStep;

		if (byteCount_src == byteCount_dst)
		{
			//*	copy the new data to the existing buffer
//			CONSOLE_DEBUG_W_NUM("Copying image:byteCount\t=",	byteCount_src);
			if ((cDownLoadedImage->imageData != NULL) && (newOpenCVImage->imageData != NULL))
			{
				memcpy(cDownLoadedImage->imageData, newOpenCVImage->imageData, byteCount_src);

				//*	double check the displayed image
				if (cDisplayedImage != NULL)
				{
					//*	now make a small copy that will fit on the screen.

//					CONSOLE_DEBUG("Resizing image");
					//*	Check to see if the original is color
					if ((cDownLoadedImage->nChannels == 3) && (cDownLoadedImage->depth == 8))
					{
//						CONSOLE_DEBUG("Original is 8 bit color (3 channels)");
						cvResize(cDownLoadedImage, cDisplayedImage, CV_INTER_LINEAR);
					}
					else if ((cDownLoadedImage->nChannels == 1) && (cDownLoadedImage->depth == 8))
					{
//						CONSOLE_DEBUG("Original is 8 bit B/W");
						cvCvtColor(cDownLoadedImage, cDisplayedImage, CV_GRAY2RGB);
					}
				}
				else
				{
					CONSOLE_DEBUG("cDisplayedImage is NULL");
				}
			}
			else
			{
				CONSOLE_DEBUG("Data ptr is NULL!!!!!!!!!!!!!!!!!!!!!!!!");
			//	CONSOLE_ABORT(__FUNCTION__);
			}
		}
		else
		{
			CONSOLE_DEBUG("Byte counts dont match");
		}
	}
}
#endif // _USE_OPENCV_CPP_



#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
//*	this routine updates the existing image by copying the new image to the old image buffer
//*	it checks to make sure they are compatible
//**************************************************************************************
void	ControllerImage::UpdateLiveWindowImage(cv::Mat *newOpenCVImage, const char *imageFileName)
#else
void	ControllerImage::UpdateLiveWindowImage(IplImage *newOpenCVImage, const char *imageFileName)
#endif // _USE_OPENCV_CPP_
{
bool	imagesAreTheSame;

//	CONSOLE_DEBUG("-------------------Start");
	CONSOLE_DEBUG(__FUNCTION__);
	if (newOpenCVImage  != NULL)
	{
		if ((cDownLoadedImage == NULL) || (cDisplayedImage == NULL))
		{
//			CONSOLE_DEBUG("Setting image");
			SetLiveWindowImage(newOpenCVImage);
		}
		else
		{
//			CONSOLE_DEBUG("Updating image");
			imagesAreTheSame	=	true;
	#ifdef _USE_OPENCV_CPP_
		#warning "OpenCV++ not finished"
			//*	check if width are the same
			if (newOpenCVImage->cols != cDownLoadedImage->cols)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on width");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->cols  \t=",		newOpenCVImage->cols);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->cols\t=",	cDownLoadedImage->cols);
			}
			//*	check if height are the same
			if (newOpenCVImage->rows != cDownLoadedImage->rows)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on height");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->rows  \t=",	newOpenCVImage->rows);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->rows\t=",	cDownLoadedImage->rows);
			}
	#else
			//*	check if width are the same
			if (newOpenCVImage->width != cDownLoadedImage->width)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on width");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->width  \t=",		newOpenCVImage->width);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->width\t=",	cDownLoadedImage->width);

			}
			//*	check if height are the same
			if (newOpenCVImage->height != cDownLoadedImage->height)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on height");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->height  \t=",	newOpenCVImage->height);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->height\t=",	cDownLoadedImage->height);
			}
			//*	check if nChannels are the same
			if (newOpenCVImage->nChannels != cDownLoadedImage->nChannels)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on nChannels");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->nChannels  \t=",	newOpenCVImage->nChannels);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->nChannels\t=",	cDownLoadedImage->nChannels);
			}
			//*	check if depth are the same
			if (newOpenCVImage->depth != cDownLoadedImage->depth)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on depth");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->depth  \t=",	newOpenCVImage->depth);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->depth\t=",	cDownLoadedImage->depth);
			}
			//*	check if widthStep are the same
			if (newOpenCVImage->widthStep != cDownLoadedImage->widthStep)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on widthStep");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->widthStep  \t=",	newOpenCVImage->widthStep);
				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->widthStep\t=",	cDownLoadedImage->widthStep);
			}
	#endif // _USE_OPENCV_CPP_

			if (imagesAreTheSame)
			{
				CopyImageToLiveImage(newOpenCVImage);
			}
			else
			{
				CONSOLE_DEBUG("images are different !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	#ifdef _USE_OPENCV_CPP_
		#warning "OpenCV++ not finished"
	#else
				//*	check to see if our temporary image exists
				if (cColorImage == NULL)
				{
				int		newImgWidth;
				int		newImgHeight;

					newImgWidth		=	newOpenCVImage->width;
					newImgHeight	=	newOpenCVImage->height;
					cColorImage		=	cvCreateImage(cvSize(	newImgWidth,
																newImgHeight),
																IPL_DEPTH_8U,
																3);
				}
				if (cColorImage != NULL)
				{
					cvCvtColor(newOpenCVImage, cColorImage, CV_GRAY2RGB);
					CopyImageToLiveImage(cColorImage);
				}
	#endif // _USE_OPENCV_CPP_
			}
			cUpdateWindow	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG("newOpenCVImage is NULL");
	}

	//*	was a file name supplied
	if (imageFileName != NULL)
	{
		SetWidgetText(kTab_Image, kImageDisplay_Title, imageFileName);
	}


//	CONSOLE_DEBUG("-------------------exit");
}

//**************************************************************************************
void	ControllerImage::UpdateLiveWindowInfo(	TYPE_CameraProperties	*cameraProp,
												const int				framesRead,
												const double			exposure_Secs,
												const char				*filterName,
												const char				*objectName
												)
{
char	lineBuff[64];

	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetNumber(kTab_Image, kImageDisplay_Gain,			cameraProp->Gain);

	sprintf(lineBuff, "%1.1f F",			(cameraProp->CCDtemperature * 9.0/5.0) +32.0);
	SetWidgetText(	kTab_Image, kImageDisplay_CameraTemp,	lineBuff);


	SetWidgetNumber(kTab_Image, kImageDisplay_FrameCnt,		framesRead);
	SetWidgetNumber(kTab_Image, kImageDisplay_Exposure,		exposure_Secs);
	SetWidgetText(	kTab_Image, kImageDisplay_Object,		objectName);

	if (filterName != NULL)
	{
		SetWidgetText(	kTab_Image, kImageDisplay_Filter,	filterName);
	}

	SetWidgetNumber(kTab_Image, kImageDisplay_FramesSaved,	0);

	cUpdateWindow	=	true;
}

#endif // _ENABLE_CTRL_IMAGE_
