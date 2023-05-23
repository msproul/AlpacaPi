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
//*	Nov 15,	2022	<MLS> Added ControllerImage(filePath);
//*	Nov 15,	2022	<MLS> Added InitClassVariables()
//*	Feb 25,	2023	<MLS> Added SetDownloadInfo()
//*	Feb 26,	2023	<MLS> Added histogram to image window
//*	Feb 26,	2023	<MLS> Added CalculateHistogramArray()
//*	Feb 28,	2023	<MLS> Added SaveHistogram()
//*****************************************************************************

#ifdef _ENABLE_CTRL_IMAGE_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"helper_functions.h"

#include	"opencv_utils.h"

#define _ENABLE_CONSOLE_DEBUG_
#define	_DEBUG_TIMING_
#include	"ConsoleDebug.h"


#define	kWindowWidth	1100
#define	kWindowHeight	800

#include	"alpaca_defs.h"
#include	"windowtab_image.h"
#include	"windowtab_about.h"

#include	"fits_opencv.h"


#include	"controller.h"
#include	"controller_image.h"
#include	"windowtab_imageinfo.h"




extern char	gFullVersionString[];


//**************************************************************************************
enum
{
	kTab_Image	=	1,
	kTab_ImageInfo,
	kTab_About,

	kTab_Count

};


#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
ControllerImage::ControllerImage(	const char			*argWindowName,
									cv::Mat				*downloadedImage,
									TYPE_BinaryImageHdr	*binaryImageHdr)
			:Controller(	argWindowName,
							kWindowWidth,
							kWindowHeight)
#else
//**************************************************************************************
ControllerImage::ControllerImage(	const char			*argWindowName,
									IplImage			*downloadedImage,
									TYPE_BinaryImageHdr	*binaryImageHdr)
			:Controller(	argWindowName,
							kWindowWidth,
							kWindowHeight)
#endif // _USE_OPENCV_CPP_
{

	CONSOLE_DEBUG(__FUNCTION__);

	InitClassVariables();

	//*	deal with the binary image header if it was supplied
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

		SetWidgetNumber(kTab_ImageInfo, kImageInfo_MetadataVersionVal,			cBinaryImageHdr.MetadataVersion);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ErrorNumberVal,				cBinaryImageHdr.ErrorNumber);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ClientTransactionIDVal,		(int32_t)cBinaryImageHdr.ClientTransactionID);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ServerTransactionIDVal,		(int32_t)cBinaryImageHdr.ServerTransactionID);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_DataStartVal,				cBinaryImageHdr.DataStart);
		SetWidgetText(	kTab_ImageInfo, kImageInfo_ImageElementTypeVal,			elementTypeStr);
		SetWidgetText(	kTab_ImageInfo, kImageInfo_TransmissionElementTypeVal,	transmitTypeStr);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_RankVal,						cBinaryImageHdr.Rank);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension1Val,				cBinaryImageHdr.Dimension1);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension2Val,				cBinaryImageHdr.Dimension2);
		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension3Val,				cBinaryImageHdr.Dimension3);
	}


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
//*	this version reads a file from disk into the display window
//**************************************************************************************
ControllerImage::ControllerImage(	const char	*argWindowName,
									const char *imageFilePath)
			:Controller(	argWindowName,
							kWindowWidth,
							kWindowHeight)
{
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat		*imageFromDisk;
#else
	IplImage	*imageFromDisk;
#endif // _USE_OPENCV_CPP_

	CONSOLE_DEBUG(__FUNCTION__);
	InitClassVariables();

	imageFromDisk	=	ReadImageIntoOpenCVimage(imageFilePath);
	if (imageFromDisk != NULL)
	{
		SetLiveWindowImage(imageFromDisk);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read image from disk:", imageFilePath);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerImage::~ControllerImage(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, NULL);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
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
	DELETE_OBJ_IF_VALID(cImageInfoTabObjcPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerImage::InitClassVariables(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cDownLoadedImage	=	NULL;
	cDisplayedImage		=	NULL;
	cColorImage			=	NULL;

	cImageTabObjPtr			=	NULL;
	cImageInfoTabObjcPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	strcpy(cImageFileName, "unkownimage");

	CONSOLE_DEBUG_W_SIZE("sizeof(TYPE_BinaryImageHdr)", sizeof(TYPE_BinaryImageHdr));
	memset((void *)&cBinaryImageHdr, 0, sizeof(TYPE_BinaryImageHdr));

	SetupWindowControls();
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}

//**************************************************************************************
void	ControllerImage::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cWidth        \t=",	cWidth);

	SetTabCount(kTab_Count);

	//---------------------------------------------------------------------
	SetTabText(kTab_Image,		"Image");
	CONSOLE_DEBUG(__FUNCTION__);
	cImageTabObjPtr		=	new WindowTabImage(	cWidth, cHeight, cBackGrndColor, cWindowName);
	CONSOLE_DEBUG(__FUNCTION__);
	if (cImageTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Image,	cImageTabObjPtr);
		cImageTabObjPtr->SetParentObjectPtr(this);
	}

	//---------------------------------------------------------------------
	SetTabText(kTab_ImageInfo,		"Image Info");
	CONSOLE_DEBUG(__FUNCTION__);
	cImageInfoTabObjcPtr		=	new WindowTabImageInfo(	cWidth, cHeight, cBackGrndColor, cWindowName, &cBinaryImageHdr);
	CONSOLE_DEBUG(__FUNCTION__);
	if (cImageInfoTabObjcPtr != NULL)
	{
		SetTabWindow(kTab_ImageInfo,	cImageInfoTabObjcPtr);
		cImageInfoTabObjcPtr->SetParentObjectPtr(this);
	}

	//---------------------------------------------------------------------
	SetTabText(kTab_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
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
	else if (deltaSeconds >= 1)
	{
		needToUpdate	=	true;
	}

	if (needToUpdate)
	{
		cLastUpdate_milliSecs	=	millis();
	}

	if (cImageTabObjPtr != NULL)
	{
		cImageTabObjPtr->RunWindowBackgroundTasks();
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
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_NUM("theWidget->left\t=",	theWidget->left);
//	CONSOLE_DEBUG_W_NUM("theWidget->top\t=",	theWidget->top);
//
//	//*	this is hard coded to debug an image display problem
//	if (theWidget->left == 272)
//	{
//		DumpWidget(theWidget);
//	}
//
//	CONSOLE_DEBUG_W_NUM("theWidget->width\t=",	theWidget->width);
//	CONSOLE_DEBUG_W_NUM("theWidget->height\t=",	theWidget->height);
	if (cImageTabObjPtr != NULL)
	{
		if (cImageTabObjPtr->cImageZoomState)
		{
			CONSOLE_DEBUG(__FUNCTION__);
//			CONSOLE_DEBUG("Zoomed");
			cImageTabObjPtr->DrawFullScaleIamge();
			Controller::DrawWidgetImage(theWidget, cImageTabObjPtr->cOpenCVdisplayedImage);
		}
		else
		{
//			CONSOLE_DEBUG("Normal");
			Controller::DrawWidgetImage(theWidget);
		}
	}
	else
	{
		CONSOLE_DEBUG("cImageTabObjPtr is NULL");
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	ControllerImage::SetLiveWindowImage(cv::Mat *newOpenCVImage)
{
int		smallDispalyWidth;
int		smallDisplayHeight;
int		reduceFactor;
int		newImgWidth;
int		newImgHeight;
//int		newImgBytesPerPixel;
int		openCVerr;
bool	validImg;

//	CONSOLE_DEBUG(__FUNCTION__);

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
//		newImgBytesPerPixel	=	newOpenCVImage->step[1];
		validImg			=	true;
		if ((newImgWidth < 100) || (newImgWidth > 10000))
		{
			validImg		=	false;
		}
		if ((newImgHeight < 100) || (newImgHeight > 10000))
		{
			validImg		=	false;
		}
//		if ((newImgBytesPerPixel != 1) && (newImgBytesPerPixel != 3))
//		{
//			validImg		=	false;
//		}

		//--------------------------------------------------------------
		if (validImg)
		{
//			CONSOLE_DEBUG_W_NUM("newImgBytesPerPixel\t=", newImgBytesPerPixel);
			cDownLoadedImage	=	ConvertImageToRGB(newOpenCVImage);

			//*	the downloaded image needs to be copied and/or resized to the displayed image
			if (cDownLoadedImage != NULL)
			{
//				CONSOLE_DEBUG_W_LONG("cDownLoadedImage->step[0]\t=",	cDownLoadedImage->step[0]);
//				CONSOLE_DEBUG_W_LONG("cDownLoadedImage->step[1]\t=",	cDownLoadedImage->step[1]);

//				CONSOLE_DEBUG("Creating small image");
				reduceFactor		=	1;
				smallDispalyWidth	=	cDownLoadedImage->cols;
				smallDisplayHeight	=	cDownLoadedImage->rows;

//				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->cols\t=",	cDownLoadedImage->cols);
//				CONSOLE_DEBUG_W_NUM("cDownLoadedImage->rows\t=",	cDownLoadedImage->rows);


				int		maxWindowWidth	=	800;
				int		maxWindowHeight	=	700;

//				CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);
				while ((smallDispalyWidth > maxWindowWidth) || (smallDisplayHeight > (maxWindowHeight - 50)))
				{
					reduceFactor++;
					smallDispalyWidth	=	cDownLoadedImage->cols / reduceFactor;
					smallDisplayHeight	=	cDownLoadedImage->rows / reduceFactor;
//					CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=", smallDisplayHeight);
				}
//				CONSOLE_DEBUG_W_NUM("reduceFactor\t=", reduceFactor);
//				CONSOLE_DEBUG_W_NUM("smallDispalyWidth \t=",	smallDispalyWidth);
//				CONSOLE_DEBUG_W_NUM("smallDisplayHeight\t=",	smallDisplayHeight);

				cDisplayedImage	=	new cv::Mat(cv::Size(	smallDispalyWidth,
															smallDisplayHeight),
															CV_8UC3);

				if (cDisplayedImage != NULL)
				{
//					CONSOLE_DEBUG_W_LONG("cDisplayedImage->step[0]\t=",	cDisplayedImage->step[0]);
//					CONSOLE_DEBUG_W_LONG("cDisplayedImage->step[1]\t=",	cDisplayedImage->step[1]);
//					CONSOLE_DEBUG("Resizing image");

//					CONSOLE_DEBUG("Original is 8 bit color (3 channels)");
					cv::resize(	*cDownLoadedImage,
								*cDisplayedImage,
								cDisplayedImage->size(),
								0,
								0,
								cv::INTER_LINEAR);

					openCVerr	=	cv::imwrite("displayed-resized.png", *cDisplayedImage);
					if (openCVerr != 0)
					{
						CONSOLE_DEBUG_W_NUM("openCVerr               \t=",	openCVerr);
						CONSOLE_DEBUG_W_NUM("cDisplayedImage->cols   \t=",	cDisplayedImage->cols);
						CONSOLE_DEBUG_W_NUM("cDisplayedImage->rows   \t=",	cDisplayedImage->rows);
						CONSOLE_DEBUG_W_LONG("cDisplayedImage->step[0]\t=",	cDisplayedImage->step[0]);
						CONSOLE_DEBUG_W_LONG("cDisplayedImage->step[1]\t=",	cDisplayedImage->step[1]);
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
				CONSOLE_DEBUG("Error creating image (new cv::Mat)");
			}
			//*	Update the image size on the screen
			SetImageWindowInfo();
		}
	}
	else
	{
		CONSOLE_DEBUG("Image parameters invalid !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//		CONSOLE_ABORT(__FUNCTION__);
	}
	CalculateHistogramArray();
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

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
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

//**************************************************************************************
void	ControllerImage::SetDownloadInfo(double download_MBytes, double download_seconds)
{
double	download_MB_per_sec;

//	CONSOLE_DEBUG(__FUNCTION__);
	download_MB_per_sec	=	0.0;
	if (download_seconds > 0.0)
	{
		download_MB_per_sec	=	download_MBytes / download_seconds;
	}
	SetWidgetNumber(kTab_ImageInfo,	kImageInfo_DownLoadMBytesVal,	download_MBytes);
	SetWidgetNumber(kTab_ImageInfo,	kImageInfo_DownLoadSecondsVal,	download_seconds);
	SetWidgetNumber(kTab_ImageInfo,	kImageInfo_DownLoadSpeedVal,	download_MB_per_sec);

}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	ControllerImage::CopyImageToLiveImage(cv::Mat *newOpenCVImage)
{
size_t	byteCount_src;
size_t	byteCount_old;
//int		newImgWidth;
int		newImgHeight;
int		newImgRowStepSize;
//int		newImgChannels;

int		oldImgHeight;
int		oldImgRowStepSize;

//*	displayed image info
int		dspImgHeight;
int		dspImgRowStepSize;
size_t	byteCount_dsp;

	CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	CONSOLE_DEBUG("+++++           OpenCV++ not finished              +++++");
	CONSOLE_DEBUG("+++++           Finish this NEXT!!!!!!             +++++");
	CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this is just an extra check, it was crashing on testing
	if ((cDownLoadedImage != NULL) && (newOpenCVImage != NULL))
	{
//		DumpCVMatStruct(__FUNCTION__, newOpenCVImage,	"newOpenCVImage");
//		DumpCVMatStruct(__FUNCTION__, cDownLoadedImage,	"cDownLoadedImage");
//		newImgWidth			=	newOpenCVImage->cols;
		newImgHeight		=	newOpenCVImage->rows;
		newImgRowStepSize	=	newOpenCVImage->step[0];
//		newImgChannels		=	newOpenCVImage->step[1];

		byteCount_src		=	newImgHeight * newImgRowStepSize;

		oldImgHeight		=	cDownLoadedImage->rows;
		oldImgRowStepSize	=	cDownLoadedImage->step[0];

		byteCount_old		=	oldImgHeight * oldImgRowStepSize;
//		CONSOLE_DEBUG_W_LONG("byteCount_src\t=",	byteCount_src);
//		CONSOLE_DEBUG_W_LONG("byteCount_old\t=",	byteCount_old);

		if (byteCount_src == byteCount_old)
		{
//			CONSOLE_DEBUG("memcpy to cDownLoadedImage");
			memcpy(cDownLoadedImage->data, newOpenCVImage->data, byteCount_src);
		}

		//*	double check the displayed image
		if (cDisplayedImage != NULL)
		{
//			DumpCVMatStruct(__FUNCTION__, cDisplayedImage, "cDisplayedImage");
			dspImgHeight		=	cDisplayedImage->rows;
			dspImgRowStepSize	=	cDisplayedImage->step[0];
			byteCount_dsp		=	dspImgHeight * dspImgRowStepSize;

			if (byteCount_dsp == byteCount_old)
			{
				CONSOLE_DEBUG("memcpy to cDisplayedImage");
				memcpy(cDisplayedImage->data, cDownLoadedImage->data, byteCount_dsp);
			}
			else
			{
				//*	now make a small copy that will fit on the screen.
				CONSOLE_DEBUG("byteCount_dsp != byteCount_old");
				CONSOLE_DEBUG_W_SIZE("byteCount_dsp\t=",	byteCount_dsp);
				CONSOLE_DEBUG_W_SIZE("byteCount_old\t=",	byteCount_old);
				cv::resize(	*newOpenCVImage,
							*cDisplayedImage,
							cDisplayedImage->size(),
							0,
							0,
							cv::INTER_LINEAR);
			}
		}
		else
		{
			CONSOLE_DEBUG("cDisplayedImage is NULL");
		}
	}
	else
	{
		CONSOLE_DEBUG("((cDownLoadedImage != NULL) && (newOpenCVImage != NULL))");
	}
	CalculateHistogramArray();
}
#else
//**************************************************************************************
void	ControllerImage::CopyImageToLiveImage(IplImage *newOpenCVImage)
{
size_t				byteCount_src;
size_t				byteCount_dst;

//	CONSOLE_DEBUG(__FUNCTION__);
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



#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
//*	this routine updates the existing image by copying the new image to the old image buffer
//*	it checks to make sure they are compatible
//**************************************************************************************
void	ControllerImage::UpdateLiveWindowImage(cv::Mat *newOpenCVImage, const char *imageFileName)
#else
void	ControllerImage::UpdateLiveWindowImage(IplImage *newOpenCVImage, const char *imageFileName)
#endif // _USE_OPENCV_CPP_
{
bool			imagesAreTheSame;
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	unsigned int	rowStepSize;
	unsigned int	nChannels;
#endif

	CONSOLE_DEBUG("-------------------Start");
	CONSOLE_DEBUG(__FUNCTION__);
	if (newOpenCVImage  != NULL)
	{
		CONSOLE_DEBUG("newOpenCVImage is OK!!!!");
		if ((cDownLoadedImage == NULL) || (cDisplayedImage == NULL))
		{
			CONSOLE_DEBUG("Setting image");
			SetLiveWindowImage(newOpenCVImage);
		}
		else
		{
//			CONSOLE_DEBUG("Updating image");
			imagesAreTheSame	=	true;
	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		#warning "OpenCV++ not finished"
			//*	check if width are the same
			if (newOpenCVImage->cols != cDownLoadedImage->cols)
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on width");
				CONSOLE_DEBUG_W_NUM("newOpenCVImage->cols  \t=",	newOpenCVImage->cols);
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
			rowStepSize	=	newOpenCVImage->step[0];
			nChannels	=	newOpenCVImage->step[1];
			if (rowStepSize != cDownLoadedImage->step[0])
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on rowStepSize");
			}

			if (nChannels != cDownLoadedImage->step[1])
			{
				imagesAreTheSame	=	false;
				CONSOLE_DEBUG("Failed on nChannels");
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
				CONSOLE_DEBUG("imagesAreTheSame");
				CopyImageToLiveImage(newOpenCVImage);
			}
			else
			{
				CONSOLE_DEBUG("images are different !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				//*	check to see if our temporary image exists
				if (cColorImage == NULL)
				{
				int		newImgWidth;
				int		newImgHeight;

			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
					newImgWidth		=	newOpenCVImage->cols;
					newImgHeight	=	newOpenCVImage->rows;
					cColorImage		=	new cv::Mat(cv::Size(	newImgWidth,
																newImgHeight),
																CV_8UC3);
			#else
					newImgWidth		=	newOpenCVImage->width;
					newImgHeight	=	newOpenCVImage->height;
					cColorImage		=	cvCreateImage(cvSize(	newImgWidth,
																newImgHeight),
																IPL_DEPTH_8U,
																3);
			#endif // _USE_OPENCV_CPP_
				}
				if (cColorImage != NULL)
				{
				#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
					//*	convert gray scale to color
					cv::cvtColor(*newOpenCVImage, *cColorImage, cv::COLOR_GRAY2BGR);
				#else
					cvCvtColor(newOpenCVImage, cColorImage, CV_GRAY2RGB);
				#endif // _USE_OPENCV_CPP_
					CopyImageToLiveImage(cColorImage);
				}
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
		strcpy(cImageFileName, imageFileName);
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


//*****************************************************************************
void	ControllerImage::CalculateHistogramArray(void)
{
int32_t			imageDataLen;
int32_t			currPixValue;
int32_t			iii;
int32_t			ccc;
//uint16_t		*imageDataPtr16bit;
uint8_t			*imageDataPtr8bit;
int32_t			redValue;
int32_t			grnValue;
int32_t			bluValue;
int32_t			lumValue;		//*	luminance value
int32_t			peakPixelIdx;
int32_t			peakPixelCount;
bool			lookingForMin;
int				imgWidth;
int				imgHeight;
int				imgChannels;
//int				imgRowStepSize;
uint8_t			*imgDataPtr;
char			textString[32];

//	SETUP_TIMING();
//
//	CONSOLE_DEBUG(__FUNCTION__);
//	START_TIMING();

	cPeakHistogramValue	=	0;
	cMaxHistogramValue	=	0;
	cMaxHistogramPixCnt	=	0;
	cMaxRedValue		=	0;
	cMaxGrnValue		=	0;
	cMaxBluValue		=	0;
	cMaxGryValue		=	0;

	//*	clear out the histogram data array
	memset(cHistogramLum,	0,	sizeof(cHistogramLum));
	memset(cHistogramRed,	0,	sizeof(cHistogramRed));
	memset(cHistogramGrn,	0,	sizeof(cHistogramGrn));
	memset(cHistogramBlu,	0,	sizeof(cHistogramBlu));

	if (cDownLoadedImage != NULL)
	{

	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		imgWidth		=	cDownLoadedImage->cols;
		imgHeight		=	cDownLoadedImage->rows;
//		imgRowStepSize	=	cDownLoadedImage->step[0];
		imgChannels		=	cDownLoadedImage->step[1];
		imgDataPtr		=	cDownLoadedImage->data;
//		CONSOLE_DEBUG_W_LONG("cDownLoadedImage->step[0]\t=",	cDownLoadedImage->step[0]);
//		CONSOLE_DEBUG_W_LONG("cDownLoadedImage->step[1]\t=",	cDownLoadedImage->step[1]);

	#else
		imgWidth		=	cDownLoadedImage->width;
		imgHeight		=	cDownLoadedImage->height;
		imgChannels		=	cDownLoadedImage->nChannels;
		imgDataPtr		=	NULL;
	#endif // _USE_OPENCV_CPP_
		//*	calculate the size of the image
		imageDataLen	=	imgWidth * imgHeight;

//		CONSOLE_DEBUG_W_NUM("imgWidth      \t=",	imgWidth);
//		CONSOLE_DEBUG_W_NUM("imgHeight     \t=",	imgHeight);
//		CONSOLE_DEBUG_W_NUM("imgChannels   \t=",	imgChannels);
//		CONSOLE_DEBUG_W_NUM("imgRowStepSize\t=",	imgRowStepSize);
//		CONSOLE_DEBUG_W_NUM("imageDataLen  \t=",	imageDataLen);

		if (imgDataPtr != NULL)
		{
			//*	figure out what type of image it is
			switch(imgChannels)
			{
				case 1:
					imageDataPtr8bit	=	(uint8_t *)imgDataPtr;
					for (iii=0; iii<imageDataLen; iii++)
					{
						currPixValue	=	imageDataPtr8bit[iii] & 0x00ff;

						cHistogramLum[currPixValue]++;

						if (currPixValue > cMaxGryValue)
						{
							cMaxGryValue	=	currPixValue;
						}
					}
					break;

//				case kImageType_RAW16:
//					imageDataPtr16bit	=	(uint16_t *)cCameraDataBuffer;
//					for (iii=0; iii<imageDataLen; iii++)
//					{
//						//*	for 16 bit data, we shift it right 8 bits
//						currPixValue	=	imageDataPtr16bit[iii] & 0x00ffff;
//						currPixValue	=	(currPixValue >> 8) & 0x00ff;
//						cHistogramLum[currPixValue]++;
//					}
//					break;


				case 3:
//					CONSOLE_DEBUG("Processing histogram for RGB image");
					imageDataPtr8bit	=	(uint8_t *)imgDataPtr;
					ccc					=	0;
					for (iii=0; iii<imageDataLen; iii++)
					{
						//*	openCV uses BGR instead of RGB
						//*	https://docs.opencv.org/master/df/d24/tutorial_js_image_display.html
						bluValue	=	imageDataPtr8bit[ccc + 0] & 0x00ff;
						grnValue	=	imageDataPtr8bit[ccc + 1] & 0x00ff;
						redValue	=	imageDataPtr8bit[ccc + 2] & 0x00ff;
						cHistogramRed[redValue]++;
						cHistogramGrn[grnValue]++;
						cHistogramBlu[bluValue]++;

						if (redValue > cMaxRedValue)
						{
							cMaxRedValue	=	redValue;
						}
						if (grnValue > cMaxGrnValue)
						{
							cMaxGrnValue	=	grnValue;
						}
						if (bluValue > cMaxBluValue)
						{
							cMaxBluValue	=	bluValue;
						}
						lumValue	=	(redValue + grnValue + bluValue) / 3;
						if (lumValue > cMaxGryValue)
						{
							cMaxGryValue	=	lumValue;
						}
						ccc	+=	3;
					}
					//*	now calculate the luminance
					for (iii=0; iii<256; iii++)
					{
						lumValue	=	cHistogramRed[iii];
						lumValue	+=	cHistogramGrn[iii];
						lumValue	+=	cHistogramBlu[iii];
						lumValue	=	(lumValue / 3);

						cHistogramLum[iii]	=	lumValue;
					}

					break;

				default:
					break;

			}
		}
		//*	now go through the array and find the peak value and max value
		peakPixelIdx		=	-1;
		peakPixelCount		=	0;
		lookingForMin		=	true;
		for (iii=0; iii<256; iii++)
		{
			//*	find the minimum value
			if (lookingForMin && (cHistogramLum[iii] > 0))
			{
				cMinHistogramValue	=	iii;
				lookingForMin		=	false;
			}
			//*	find the maximum value
			if (cHistogramLum[iii] > 0)
			{
				cMaxHistogramValue	=	iii;
			}
			//*	find the peak value
			if (cHistogramLum[iii] > peakPixelCount)
			{
				peakPixelIdx	=	iii;
				peakPixelCount	=	cHistogramLum[iii];
			}
		}
		cPeakHistogramValue	=	peakPixelIdx;

		//*	look for maximum pixel counts
		cMaxHistogramPixCnt	=	0;

		//*	purposely skip the first and last
//		for (iii=1; iii<255; iii++)
//		for (iii=5; iii<250; iii++)
		for (iii=0; iii<256; iii++)
		{
			if (cHistogramLum[iii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramLum[iii];
			}
			if (cHistogramRed[iii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramRed[iii];
			}
			if (cHistogramGrn[iii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramGrn[iii];
			}
			if (cHistogramBlu[iii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramBlu[iii];
			}
		}

//		DEBUG_TIMING("Time to calculate histogram data (milliseconds)\t=");

//		CONSOLE_DEBUG_W_NUM("cMinHistogramValue\t=",	cMinHistogramValue);
//		CONSOLE_DEBUG_W_NUM("cMaxHistogramValue\t=",	cMaxHistogramValue);
//		CONSOLE_DEBUG_W_NUM("cPeakHistogramValue\t=",	cPeakHistogramValue);
//		CONSOLE_DEBUG_W_NUM("cMaxHistogramPixCnt\t=",	cMaxHistogramPixCnt);
//
//		CONSOLE_DEBUG_W_NUM("cMaxRedValue\t=",	cMaxRedValue);
//		CONSOLE_DEBUG_W_NUM("cMaxGrnValue\t=",	cMaxGrnValue);
//		CONSOLE_DEBUG_W_NUM("cMaxBluValue\t=",	cMaxBluValue);
//		CONSOLE_DEBUG_W_NUM("cMaxGryValue\t=",	cMaxGryValue);

		sprintf(textString,	"R=%3.0f%%", ((100.0 * cMaxRedValue) / 255));
		SetWidgetText(		kTab_Image, kImageDisplay_HistRedPerct, textString);

		sprintf(textString,	"G=%3.0f%%", ((100.0 * cMaxGrnValue) / 255));
		SetWidgetText(		kTab_Image, kImageDisplay_HistGrnPerct, textString);

		sprintf(textString,	"B=%3.0f%%", ((100.0 * cMaxBluValue) / 255));
		SetWidgetText(		kTab_Image, kImageDisplay_HistBluPerct, textString);

		SetWidgetProgress(	kTab_Image, kImageDisplay_LumBar, cMaxGryValue, 255);
		SetWidgetProgress(	kTab_Image, kImageDisplay_RedBar, cMaxRedValue, 255);
		SetWidgetProgress(	kTab_Image, kImageDisplay_GrnBar, cMaxGrnValue, 255);
		SetWidgetProgress(	kTab_Image, kImageDisplay_BluBar, cMaxBluValue, 255);
	}
	else
	{
		CONSOLE_DEBUG("cDownLoadedImage is NULL");
	}
}

//*****************************************************************************
void	ControllerImage::SaveHistogram(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

#endif // _ENABLE_CTRL_IMAGE_
