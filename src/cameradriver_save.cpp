//**************************************************************************
//*	Name:			cameradriver_save.cpp
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan 30,	2020	<MLS> Created cameradriver_save.cpp
//*	Jan 30,	2020	<MLS> Added SaveImageData(), AddToDataProductsList()
//*	Jan 30,	2020	<MLS> Added SaveOpenCVImage()
//*	Jan 30,	2020	<MLS> Separated saving of opencv image from the creation part
//*	Apr 10,	2022	<MLS> CreateOpenCVImage() now can use openCV++ calls
//*	Apr 10,	2022	<MLS> SaveOpenCVImage() now can use openCV++ calls
//*	May  3,	2022	<MLS> Added WriteIMUtextFile()
//*	Jul  7,	2022	<MLS> Added Quaternion data to WriteIMUtextFile()
//*	Jul 25,	2022	<MLS> Increased # of decimal points in WriteIMUtextFile()
//*	Oct  5,	2022	<MLS> Added ReadIMUdata()
//*	Jun 13,	2023	<MLS> Added checking for valid IMU
//*****************************************************************************

#ifdef _ENABLE_CAMERA_

//*	OpenCV png file creation takes WAY too long, don't use it.
//#define	_ENABLE_PNG_

#include	<stdio.h>
#include	<string.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#if defined(_JETSON_) && defined(_FIND_STARS_)
	#include	"imageprocess_orb.h"
#endif // _JETSON_


//#define	_DEBUG_TIMING_
#include	"linuxerrors.h"
#include	"helper_functions.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"

#ifdef _ENABLE_STAR_SEARCH_
	//*	this is totally experimental and is not part of the normal release
	#include	"imageprocess_orb.h"
#endif
#ifdef _ENABLE_IMU_
	#include "imu_lib.h"
	#include "imu_lib_bno055.h"
#endif

//*****************************************************************************
void	CameraDriver::SaveImageData(void)
{
int		iii;


	CONSOLE_DEBUG_W_NUM("cSaveNextImage\t=", cSaveNextImage);
	CONSOLE_DEBUG_W_NUM("cSaveAllImages\t=", cSaveAllImages);
	cCameraProp.SavedImageCnt++;
	cTotalFramesSaved++;
	CONSOLE_DEBUG_W_NUM("cCameraProp.SavedImageCnt=", cCameraProp.SavedImageCnt);


	for (iii=0; iii<kMaxDataProducts; iii++)
	{
		memset(&cOtherDataProducts[iii], 0, sizeof(TYPE_FILENAME));
	}
	cOtherDataCnt	=	0;

	if (cCameraDataBuffer != NULL)
	{
	#ifdef _ENABLE_IMU_
		//*	we want to do this first so the readings are closest to the time we took the picture
		if (IMU_IsAvailable())
		{
			ReadIMUdata();
			WriteIMUtextFile();
		}
	#endif

	#ifdef _INCLUDE_HISTOGRAM_
		CalculateHistogramArray();
		//*	Apr 15,	2022	<MLS> Disabled Histogram to speed up saving files
		//	SaveHistogramFile();
	#endif // _INCLUDE_HISTOGRAM_


	#ifdef _USE_OPENCV_
		if (cSaveAsJPEG || cSaveAsPNG)
		{
			SaveOpenCVImage();
		}
	#endif	//	_USE_OPENCV_


	#if defined(_ENABLE_JPEGLIB_)
int		bytesPerPixel;
		bytesPerPixel		=	cOpenCV_ImagePtr->step[1];
		if (cSaveAsJPEG && (bytesPerPixel!= 2))
		{
			SaveUsingJpegLib();
		}
	#endif	//	_ENABLE_JPEGLIB_


	//*	we want FITS to be last so it can include info about other save data products
	#ifdef _ENABLE_FITS_
		if (cSaveAsFITS)
		{
			SaveImageAsFITS();
		}
	#endif // _ENABLE_FITS_
	#if defined(_JETSON_) && defined(_FIND_STARS_)
		long	keyPointCnt;
		char	imageFilePath[128];
		int		quality[3] = {16, 200, 0};
		int		openCVerr;

		CONSOLE_DEBUG("Calling ProcessORB_Image!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		SETUP_TIMING();

		keyPointCnt	=	ProcessORB_Image(cOpenCV_Image);

		DEBUG_TIMING("Time to complete ORB");
		CONSOLE_DEBUG_W_LONG("keyPointCnt\t=", keyPointCnt);

		CONSOLE_DEBUG("Saving ORB Image *****************************************");
		strcpy(imageFilePath, gImageDataDir);
		strcat(imageFilePath, "/");
		strcat(imageFilePath, cFileNameRoot);
		strcat(imageFilePath, "-orb.jpg");
		openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_Image, quality);
		if (openCVerr != 0)
		{
			CONSOLE_DEBUG_W_NUM("cvSaveImage returned\t=", openCVerr);
		}
	#endif // _JETSON_
	}
	else
	{
		CONSOLE_DEBUG("cCameraDataBuffer is NULL");
	}
	cSaveNextImage	=	false;

}

//*****************************************************************************
void	CameraDriver::AddToDataProductsList(const char *newDataProductName, const char *newDatacomment)
{
int		fileNameLen;

	if (cOtherDataCnt < kMaxDataProducts)
	{
		fileNameLen	=	strlen(newDataProductName);
		if (fileNameLen < kMaxFileNameLen)
		{
			strcpy(cOtherDataProducts[cOtherDataCnt].filename, newDataProductName);
			if (newDatacomment != NULL)
			{
				strcpy(cOtherDataProducts[cOtherDataCnt].comment, newDatacomment);
			}
			cOtherDataCnt++;
		}
	}
	else
	{
		CONSOLE_DEBUG("cOtherDataProducts list is full");
	}
}


#if defined(_USE_OPENCV_)
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
//*	using "C++" interface
//*****************************************************************************
int	CameraDriver::CreateOpenCVImage(const unsigned char *imageDataPtr)
{
int				returnCode	=	0;
int				width;
int				height;
int				imageDataLen;

//	CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
//	CONSOLE_DEBUG("+++++           OpenCV++ not finished              +++++");
//	CONSOLE_DEBUG("+++++           Finish this NEXT!!!!!!             +++++");
//	CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");

	CONSOLE_DEBUG(__FUNCTION__);

	GenerateFileNameRoot();
	if (cOpenCV_ImagePtr != NULL)
	{
		delete cOpenCV_ImagePtr;
		cOpenCV_ImagePtr	=	NULL;
	}
	if (cOpenCV_LiveDisplayPtr != NULL)
	{
		delete cOpenCV_LiveDisplayPtr;
		cOpenCV_LiveDisplayPtr	=	NULL;
	}
	width			=	cCameraProp.CameraXsize;
	height			=	cCameraProp.CameraYsize;
	GetImage_ROI_info();

	switch(cROIinfo.currentROIimageType)
	{
		case kImageType_RAW8:
		case kImageType_MONO8:
			CONSOLE_DEBUG("kImageType_RAW8");
			//	9/20/2022
			//cOpenCV_ImagePtr	=	new cv::Mat(height, width, CV_8UC3);
			cOpenCV_ImagePtr	=	new cv::Mat(height, width, CV_8UC1);
			imageDataLen		=	width * height;
			break;

		case kImageType_RAW16:
			CONSOLE_DEBUG("kImageType_RAW16");
			cOpenCV_ImagePtr	=	new cv::Mat(height, width, CV_16UC1);
			imageDataLen		=	width * height * 2;
			break;


		case kImageType_RGB24:
			CONSOLE_DEBUG("kImageType_RGB24");
			cOpenCV_ImagePtr	=	new cv::Mat(height, width, CV_8UC3);
			imageDataLen		=	width * height * 3;
			break;

		case kImageType_Y8:
		default:
			cOpenCV_ImagePtr	=	new cv::Mat(height, width, CV_8UC1);
			imageDataLen		=	width * height;
			break;
	}

	if (imageDataPtr != NULL)
	{
		if (cOpenCV_ImagePtr != NULL)
		{
			//*	what size did openCV make the image
//			CONSOLE_DEBUG_W_NUM("width \t=",	cOpenCV_ImagePtr->cols);
//			CONSOLE_DEBUG_W_NUM("height\t=",	cOpenCV_ImagePtr->rows);
//			CONSOLE_DEBUG_W_LONG("step[0]\t=",	cOpenCV_ImagePtr->step[0]);
//			CONSOLE_DEBUG_W_LONG("step[1]\t=",	cOpenCV_ImagePtr->step[1]);
//			CONSOLE_DEBUG_W_LONG("step[2]\t=",	cOpenCV_ImagePtr->step[2]);

			if (cOpenCV_ImagePtr->data != NULL)
			{
				memcpy(cOpenCV_ImagePtr->data, imageDataPtr, imageDataLen);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("did NOT copy image data into cOpenCV_ImagePtr, imageDataLen\t\t=",	imageDataLen);
				CONSOLE_ABORT(__FUNCTION__);
			}

//			CONSOLE_DEBUG_W_LONG("imageDataLen\t\t=",			imageDataLen);

//			if (openCVimageWidth == cOpenCV_ImagePtr->width)
//			{
//			//	CONSOLE_DEBUG("Normal image data !!!!!!!!!!!!!!");
//				memcpy(cOpenCV_ImagePtr->imageData, imageDataPtr, imageDataLen);
//			}
//			else
//			{
//			char	*ocv_rowPtr;
//			char	*img_rowPtr;
//			int		rowLength;
//
//				//*	we have to copy over the image a line at a time
////				CONSOLE_DEBUG("Miss match image data !!!!!!!!!!!!!!");
//				ocv_rowPtr	=	cOpenCV_ImagePtr->imageData;
//				img_rowPtr	=	(char *)imageDataPtr;
//				rowLength	=	cOpenCV_ImagePtr->width * bytesPerPixel;
//				for (ii=0; ii<cOpenCV_ImagePtr->height; ii++)
//				{
//					memcpy(ocv_rowPtr, img_rowPtr, rowLength);
//
//					ocv_rowPtr	+=	cOpenCV_ImagePtr->widthStep;
//					img_rowPtr	+=	rowLength;
//				}
//
//			}
//			//CONSOLE_DEBUG("Copy SUCCESS!!!!!!!!!!!!!!");
//			DEBUG_TIMING("Stop point 2 (milliseconds)\t=");
//
		}
		else
		{
			CONSOLE_DEBUG("Failed to allocate openCV image");
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG("Image data is NULL (imageDataPtr)");
//		CONSOLE_ABORT(__FUNCTION__);
	}
//
	return(returnCode);
}
//*****************************************************************************
//*	using "C++" interface
//*****************************************************************************
int	CameraDriver::SaveOpenCVImage(void)
{
int			bytesPerPixel;
int			openCVerr;
char		imageFileName[64];
char		imageFilePath[128];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Using C++ openCV calls");
	SETUP_TIMING();

	if (cOpenCV_ImagePtr != NULL)
	{

		bytesPerPixel		=	cOpenCV_ImagePtr->step[1];
		CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",	bytesPerPixel);
		if (bytesPerPixel != 0)
		{
			//--------------------------------------------------------------------------------------------
			//*	JPEG does not work on 16 bit images
			if (cSaveAsJPEG && (bytesPerPixel != 2))
			{
				//*	save as JPEG
				strcpy(imageFileName, cFileNameRoot);
				strcat(imageFileName, ".jpg");

				strcpy(imageFilePath, gImageDataDir);
				strcat(imageFilePath, "/");
				strcat(imageFilePath, imageFileName);

				strcpy(cLastJpegImageName, imageFilePath);	//*	save the full image path for the web server

				openCVerr	=	cv::imwrite(imageFilePath, *cOpenCV_ImagePtr);
				if (openCVerr == 1)
				{
					AddToDataProductsList(imageFileName, "JPEG image-openCV");
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("cvSaveImage (jpg) failed, returned\t=", openCVerr);
				}
			}

			//--------------------------------------------------------------------------------------------
			if (cSaveAsPNG)
			{
//				SETUP_TIMING();
//				//*	OpenCV png file creation takes WAY too long, use caution
//				START_TIMING();

				//*	save as png
				strcpy(imageFileName, cFileNameRoot);
				strcat(imageFileName, ".png");

				strcpy(imageFilePath, gImageDataDir);
				strcat(imageFilePath, "/");
				strcat(imageFilePath, imageFileName);

				openCVerr	=	cv::imwrite(imageFilePath, *cOpenCV_ImagePtr);
				if (openCVerr == 1)
				{
					AddToDataProductsList(imageFileName, "PNG image-openCV");
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("cvSaveImage (PNG) failed, returned\t=", openCVerr);
				}
//				DEBUG_TIMING("Time to create PNG file=");
			}
		}
		else
		{
			CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
			CONSOLE_DEBUG("+++++           OpenCV++ not finished              +++++");
			CONSOLE_DEBUG("+++++   Saving JPEG only implemented for RGB       +++++");
			CONSOLE_DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_ImagePtr is NULL!!!!!!");
	}
	return(0);
}

#else
//*****************************************************************************
//*	wget http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure%20Content-Type:%20-dDuration=0.011&Light=true
//*	wget http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/imagearray
//*****************************************************************************
//*	using "C" interface
//*****************************************************************************
int	CameraDriver::CreateOpenCVImage(const unsigned char *imageDataPtr)
{
int				ii;
int				returnCode	=	0;
int				width;
int				height;
int				imageDataLen;
int				openCVimageWidth;
int				bytesPerPixel;
int				bytesPerPixel2;	//*	calculated 2 different ways

	CONSOLE_DEBUG(__FUNCTION__);

	SETUP_TIMING();

	GenerateFileNameRoot();

	if (cOpenCV_ImagePtr != NULL)
	{
		cvReleaseImage(&cOpenCV_ImagePtr);
		cOpenCV_ImagePtr	=	NULL;
	}
	if (cOpenCV_LiveDisplayPtr != NULL)
	{
		cvReleaseImage(&cOpenCV_LiveDisplayPtr);
		cOpenCV_LiveDisplayPtr	=	NULL;
	}
	width			=	cCameraProp.CameraXsize;
	height			=	cCameraProp.CameraYsize;
	GetImage_ROI_info();

//	CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);
//	CONSOLE_DEBUG_W_NUM("width\t=",		width);
//	CONSOLE_DEBUG_W_NUM("height\t=",	height);
//	CONSOLE_DEBUG_W_NUM("w * h\t=",		(width * height));

	switch(cROIinfo.currentROIimageType)
	{
		case kImageType_RAW8:
		//	CONSOLE_DEBUG("kImageType_RAW8");
			cOpenCV_ImagePtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
			imageDataLen		=	width * height;
			break;

		case kImageType_RAW16:
		//	CONSOLE_DEBUG("kImageType_RAW16");
			cOpenCV_ImagePtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
			imageDataLen		=	width * height * 2;
			break;


		case kImageType_RGB24:
		//	CONSOLE_DEBUG("kImageType_RGB24");
			cOpenCV_ImagePtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
			imageDataLen		=	width * height * 3;
			break;

		case kImageType_Y8:
		default:
			cOpenCV_ImagePtr	=	NULL;
			break;

	}
	DEBUG_TIMING("Stop point 1 (milliseconds)\t=");

	if (imageDataPtr != NULL)
	{
		if (cOpenCV_ImagePtr != NULL)
		{
			//*	what size did openCV make the image
	//		cvShowImage(cOpenCV_ImgWindowName, cOpenCV_ImagePtr);
	//		cv::waitKey(1);

//			CONSOLE_DEBUG_W_STR("Image SUCCESS!!!!!!!!!!!!!!-", cameraSerialNum);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_ImagePtr->imageSize\t=", cOpenCV_ImagePtr->imageSize);
//			CONSOLE_DEBUG_W_LONG("imageDataLen\t\t=",			imageDataLen);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_ImagePtr->nChannels\t=",	cOpenCV_ImagePtr->nChannels);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_ImagePtr->depth\t=",		cOpenCV_ImagePtr->depth);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_ImagePtr->width\t=",		cOpenCV_ImagePtr->width);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_ImagePtr->widthStep\t=",	cOpenCV_ImagePtr->widthStep);

			bytesPerPixel		=	(cOpenCV_ImagePtr->depth / 8) * cOpenCV_ImagePtr->nChannels;
			bytesPerPixel2		=	cOpenCV_ImagePtr->widthStep / cOpenCV_ImagePtr->width;
			openCVimageWidth	=	cOpenCV_ImagePtr->widthStep / bytesPerPixel;
//			CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",		bytesPerPixel);
//			CONSOLE_DEBUG_W_NUM("bytesPerPixel2\t=",	bytesPerPixel2);
//			CONSOLE_DEBUG_W_NUM("openCVimageWidth\t=",	openCVimageWidth);
			if (bytesPerPixel != bytesPerPixel2)
			{
				CONSOLE_DEBUG("Houston, we have a problem");
				CONSOLE_DEBUG("bytes per pixel is messed up");
				CONSOLE_ABORT(__FUNCTION__);
			}
			if (openCVimageWidth == cOpenCV_ImagePtr->width)
			{
			//	CONSOLE_DEBUG("Normal image data !!!!!!!!!!!!!!");
				memcpy(cOpenCV_ImagePtr->imageData, imageDataPtr, imageDataLen);
			}
			else
			{
			char	*ocv_rowPtr;
			char	*img_rowPtr;
			int		rowLength;

				//*	we have to copy over the image a line at a time
//				CONSOLE_DEBUG("Miss match image data !!!!!!!!!!!!!!");
				ocv_rowPtr	=	cOpenCV_ImagePtr->imageData;
				img_rowPtr	=	(char *)imageDataPtr;
				rowLength	=	cOpenCV_ImagePtr->width * bytesPerPixel;
				for (ii=0; ii<cOpenCV_ImagePtr->height; ii++)
				{
					memcpy(ocv_rowPtr, img_rowPtr, rowLength);

					ocv_rowPtr	+=	cOpenCV_ImagePtr->widthStep;
					img_rowPtr	+=	rowLength;
				}
			}
			//CONSOLE_DEBUG("Copy SUCCESS!!!!!!!!!!!!!!");
			DEBUG_TIMING("Stop point 2 (milliseconds)\t=");
		}
		else
		{
			CONSOLE_DEBUG("Failed to allocate openCV image");
		}
	}
	else
	{
		CONSOLE_DEBUG("Image data is NULL");
	}
	DEBUG_TIMING("Stop point 5 (milliseconds)\t=");

	return(returnCode);
}

//*****************************************************************************
//*	using "C" interface
//*****************************************************************************
int	CameraDriver::SaveOpenCVImage(void)
{
int			bytesPerPixel;
int			openCVerr;
char		imageFileName[64];
char		imageFilePath[128];
//int		quality[3] = {CV_IMWRITE_PNG_COMPRESSION, 200, 0};
int			quality[3] = {16, 200, 0};

	CONSOLE_DEBUG(__FUNCTION__);
	SETUP_TIMING();

	if (cOpenCV_ImagePtr != NULL)
	{
		bytesPerPixel		=	(cOpenCV_ImagePtr->depth / 8) * cOpenCV_ImagePtr->nChannels;
		if (bytesPerPixel != 2)
		{
			//*	save as JPEG
			strcpy(imageFileName, cFileNameRoot);
			strcat(imageFileName, ".jpg");

			strcpy(imageFilePath, gImageDataDir);
			strcat(imageFilePath, "/");
			strcat(imageFilePath, imageFileName);

			strcpy(cLastJpegImageName, imageFilePath);	//*	save the full image path for the web server
			openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_ImagePtr, quality);
			if (openCVerr == 1)
			{
				AddToDataProductsList(imageFileName, "JPEG image-openCV");
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("cvSaveImage (jpg) failed, returned\t=", openCVerr);
			}
		}
	#ifdef _ENABLE_PNG_
		if (cOpenCV_ImagePtr->depth == 16)
		{
			SETUP_TIMING();
			//*	OpenCV png file creation takes WAY too long, use caution
			START_TIMING();
			//*	save as PNG
			strcpy(imageFileName, cFileNameRoot);
			strcat(imageFileName, ".png");

			strcpy(imageFilePath, gImageDataDir);
			strcat(imageFilePath, "/");
			strcat(imageFilePath, imageFileName);

			strcpy(cLastJpegImageName, imageFilePath);	//*	save the full image path for the web server
			openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_ImagePtr, quality);
			DEBUG_TIMING("Time to create PNG file=");
			if (openCVerr == 1)
			{
				AddToDataProductsList(imageFileName, "PNG image-openCV");
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("cvSaveImage (png) returned\t=", openCVerr);
			}
		}
	#endif	//	_ENABLE_PNG_
	#ifdef _ENABLE_STAR_SEARCH_
		long	keyPointCnt;
		//*	this is an attempt at finding the locations of all of the stars in an image.

		keyPointCnt	=	ProcessORB_Image(cOpenCV_ImagePtr, cFileNameRoot);

	#endif // _ENABLE_STAR_SEARCH_
	}
	return(0);
}
#endif // _USE_OPENCV_CPP_


#endif	//	_USE_OPENCV_

#pragma mark -

//*****************************************************************************
//*	FireCapture v2.6  Settings
//*	------------------------------------
//*	Camera=ZWO ASI1600MM Pro
//*	Filter=L
//*	Profile=Sun
//*	Filename=Sun_134141.avi
//*	Date=080619
//*	Start=134126.410
//*	Mid=134141.422
//*	End=134156.435
//*	Start(UT)=174126.410
//*	Mid(UT)=174141.422
//*	End(UT)=174156.435
//*	Duration=30.025s
//*	Date_format=ddMMyy
//*	Time_format=HHmmss
//*	LT=UT -5h
//*	Frames captured=64
//*	File type=AVI
//*	Extended AVI mode=false
//*	Compressed AVI=false
//*	Binning=no
//*	ROI=4656x3520
//*	ROI(Offset)=0x0
//*	FPS (avg.)=2
//*	Shutter=0.060ms
//*	Gain=298 (49%)
//*	Gamma=50
//*	FPS=100 (off)
//*	HardwareBin=off
//*	AutoHisto=75 (off)
//*	Brightness=10 (off)
//*	HighSpeed=off
//*	TargetTemp=0 (off)
//*	AutoGain=off
//*	USBTraffic=80 (off)
//*	AutoExposure=off
//*	FixPattern=0 (off)
//*	SoftwareGain=10 (off)
//*	Histogramm(min)=0
//*	Histogramm(max)=217
//*	Histogramm=85%
//*	Noise(avg.deviation)=n/a
//*	Limit=30 Seconds
//*	Sensor temperature=27.0 C
//**************************************************************************
void	CameraDriver::WriteFireCaptureTextFile(void)
{
char	filePath[128];
char	codecString[8];
char	timeStampString[256];
FILE	*filePointer;
double	exposureTim_ms;
double	imageDuration_secs;
int		gainPercent;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cCameraProp.GainMax > 0)
	{
		gainPercent	=	(cCameraProp.Gain * 100) / cCameraProp.GainMax;
	}
	else
	{
		gainPercent	=	0;
	}

	codecString[0]	=	(cAVIfourCC) & 0x007f;
	codecString[1]	=	(cAVIfourCC >> 8) & 0x007f;
	codecString[2]	=	(cAVIfourCC >> 16) & 0x007f;
	codecString[3]	=	(cAVIfourCC >> 24) & 0x007f;
	codecString[4]	=	0;
	CONSOLE_DEBUG_W_HEX("cAVIfourCC\t=", cAVIfourCC);
	CONSOLE_DEBUG_W_STR("codecString\t=", codecString);

	GenerateFileNameRoot();
	strcpy(filePath, gImageDataDir);
	strcat(filePath, "/");
	strcat(filePath, cFileNameRoot);
	strcat(filePath, ".txt");
	filePointer	=	fopen(filePath, "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "FireCapture v2.6  Settings\r\n");
		fprintf(filePointer, "------------------------------------\r\n");

		exposureTim_ms	=	(cCameraProp.Lastexposure_duration_us * 1.0) / 1000.0;

		fprintf(filePointer, "Camera=%s\r\n",					cCommonProp.Name);
//		fprintf(filePointer, "Filter=%s\r\n",					foo);L
		fprintf(filePointer, "Profile=%s\r\n",					cFileNamePrefix);
		fprintf(filePointer, "Filename=%s\r\n",					cFileNameRoot);
//		fprintf(filePointer, "Date=%s\r\n",						foo);080619
//		fprintf(filePointer, "Start=%s\r\n",					foo);134126.410
//		fprintf(filePointer, "Mid=%s\r\n",						foo);134141.422
//		fprintf(filePointer, "End=%s\r\n",						foo);134156.435

		FormatTimeStringISO8601(&cCameraProp.Lastexposure_StartTime, timeStampString);
		fprintf(filePointer, "Start(UT)=%s\r\n",				timeStampString);	//	174126.410

//		fprintf(filePointer, "Mid(UT)=%s\r\n",					foo);174141.422

		FormatTimeStringISO8601(&cCameraProp.Lastexposure_EndTime, timeStampString);
		fprintf(filePointer, "End(UT)=%s\r\n",					timeStampString);	//	174156.435

		imageDuration_secs	=	cCameraProp.Lastexposure_EndTime.tv_sec - cCameraProp.Lastexposure_StartTime.tv_sec;
		fprintf(filePointer, "Duration=%3.3fs\r\n",				imageDuration_secs);	//	30.025s

//		fprintf(filePointer, "Date_format=%s\r\n",				foo);ddMMyy
//		fprintf(filePointer, "Time_format=%s\r\n",				foo);HHmmss
		fprintf(filePointer, "LT=UT %d\r\n",					gObseratorySettings.UTCoffset);
		fprintf(filePointer, "Frames captured=%d\r\n",			cCameraProp.SavedImageCnt);
		fprintf(filePointer, "Video Frames captured=%d\r\n",	cNumVideoFramesSaved);

		fprintf(filePointer, "File type=%s\r\n",				"AVI");
		fprintf(filePointer, "Extended AVI mode=%s\r\n",		"false");
		fprintf(filePointer, "CODEC=%s\r\n",					codecString);

//		fprintf(filePointer, "Compressed AVI=%s\r\n",			foo);false
		fprintf(filePointer, "Binning=%s\r\n",					"no");
		fprintf(filePointer, "ROI=%dx%d\r\n",					cCameraProp.CameraXsize, cCameraProp.CameraYsize);
		fprintf(filePointer, "ROI(Offset)=%dx%d\r\n",			0, 0);
		fprintf(filePointer, "FPS (avg.)=%1.1f\r\n",			cFrameRate);
		fprintf(filePointer, "Shutter=%1.3fms\r\n",				exposureTim_ms);
		fprintf(filePointer, "Gain=%d (%d)\r\n",				cCameraProp.Gain, gainPercent);	//	298 (49%)
//		fprintf(filePointer, "Gamma=%s\r\n",					foo);50
//		fprintf(filePointer, "FPS=%s\r\n",						foo);100 (off)
//		fprintf(filePointer, "HardwareBin=%s\r\n",				foo);off
//		fprintf(filePointer, "AutoHisto=%s\r\n",				foo);75 (off)
//		fprintf(filePointer, "Brightness=%s\r\n",				foo);10 (off)
//		fprintf(filePointer, "HighSpeed=%s\r\n",				foo);off
//		fprintf(filePointer, "TargetTemp=%s\r\n",				foo);0 (off)
		fprintf(filePointer, "AutoGain=%s\r\n",					"off");
//		fprintf(filePointer, "USBTraffic=%s\r\n",				foo);80 (off)
//		fprintf(filePointer, "AutoExposure=%s\r\n",				foo);off
//		fprintf(filePointer, "FixPattern=%s\r\n",				foo);0 (off)
		fprintf(filePointer, "SoftwareGain=%s\r\n",				"(off)");
//		fprintf(filePointer, "Histogramm(min)=%s\r\n",			foo);0
//		fprintf(filePointer, "Histogramm(max)=%s\r\n",			foo);217
//		fprintf(filePointer, "Histogramm=%s\r\n",				foo);85%
//		fprintf(filePointer, "Noise(avg.deviation)=%s\r\n",		foo);n/a
		fprintf(filePointer, "Limit=%3.0f Seconds\r\n",			cVideoDuration_secs);	//	30 Seconds

		Read_SensorTemp();
		fprintf(filePointer, "Sensor temperature=%1.1f C\r\n",	cCameraProp.CCDtemperature);

		fprintf(filePointer, "Object=%s\r\n",					cObjectName);
		if (strlen(cAuxTextTag) > 0)
		{
			fprintf(filePointer, "AuxiliaryText=%s\r\n",		cAuxTextTag);

		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to create file:", filePath);
	}
}

#ifdef _ENABLE_IMU_
//**************************************************************************
//*	we only want to read the data ONCE for each frame.
//*	Reading multiple times proved to give different answers
//**************************************************************************
void	CameraDriver::ReadIMUdata(void)
{
double	imuHeading;
double	imuRoll;
double	imuPitch;
double	imuwww;
double	imuxxx;
double	imuyyy;
double	imuzzz;
int		imuRetCode;

	//*	set default values
	cIMU_EulerValid	=	false;
	cIMU_Heading	=	0.0;
	cIMU_Roll		=	0.0;
	cIMU_Pitch		=	0.0;
	cIMU_QuatValid	=	false;
	cIMU_www		=	0.0;
	cIMU_xxx		=	0.0;
	cIMU_yyy		=	0.0;
	cIMU_zzz		=	0.0;
	imuRetCode		=	IMU_BNO055_Read_Euler(&imuHeading, &imuRoll, &imuPitch);
	if (imuRetCode == 0)
	{
		cIMU_EulerValid	=	true;
		cIMU_Heading	=	imuHeading;
		cIMU_Roll		=	imuRoll;
		cIMU_Pitch		=	imuPitch;
	}
	imuRetCode	=	IMU_BNO055_Read_Quaternion(&imuwww, &imuxxx, &imuyyy, &imuzzz);
	if (imuRetCode == 0)
	{
		cIMU_QuatValid	=	true;
		cIMU_www		=	imuwww;
		cIMU_xxx		=	imuxxx;
		cIMU_yyy		=	imuyyy;
		cIMU_zzz		=	imuzzz;
	}
	cIMU_Cal_Gyro	=	IMU_BNO055_Get_Calibration(kIMU_Gyro);
	cIMU_Cal_Acce	=	IMU_BNO055_Get_Calibration(kIMU_Accelerometer);
	cIMU_Cal_Magn	=	IMU_BNO055_Get_Calibration(kIMU_Magnetometer);
	cIMU_Cal_Syst	=	IMU_BNO055_Get_Calibration(kIMU_System);
}

//**************************************************************************
void	CameraDriver::WriteIMUtextFile(void)
{
char	imageFileName[64];
char	imageFilePath[128];
FILE	*filePointer;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(imageFileName, cFileNameRoot);
	strcat(imageFileName, "-imu.txt");


	strcpy(imageFilePath, gImageDataDir);
	strcat(imageFilePath, "/");
	strcat(imageFilePath, imageFileName);
	filePointer	=	fopen(imageFilePath, "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "#using bno055 sensor\r\n");
		fprintf(filePointer, "Image   =%s\r\n",		cFileNameRoot);
		if (cIMU_EulerValid)
		{
			fprintf(filePointer, "Heading =%3.5f\r\n",	cIMU_Heading);
			fprintf(filePointer, "Roll    =%3.5f\r\n",	cIMU_Roll);
			fprintf(filePointer, "Pitch   =%3.5f\r\n",	cIMU_Pitch);
		}
		else
		{
			fprintf(filePointer, "Error getting IMU data\r\n");
		}

		if (cIMU_QuatValid)
		{
			fprintf(filePointer, "www   =%3.5f\r\n",	cIMU_www);
			fprintf(filePointer, "xxx   =%3.5f\r\n",	cIMU_xxx);
			fprintf(filePointer, "yyy   =%3.5f\r\n",	cIMU_yyy);
			fprintf(filePointer, "zzz   =%3.5f\r\n",	cIMU_zzz);
		}
		else
		{
			fprintf(filePointer, "Error getting IMU data\r\n");
		}
		fprintf(filePointer,	"IMU-Calibration values 0=uncalibrated 3 = fully calibrated\r\n");
		fprintf(filePointer,	"IMU-Cal-Gyro   =%d\r\n",	cIMU_Cal_Gyro);
		fprintf(filePointer,	"IMU-Cal-Acce   =%d\r\n",	cIMU_Cal_Acce);
		fprintf(filePointer,	"IMU-Cal-Magn   =%d\r\n",	cIMU_Cal_Magn);
		fprintf(filePointer,	"IMU-Cal-Syst   =%d\r\n",	cIMU_Cal_Syst);

		fclose(filePointer);

		AddToDataProductsList(imageFileName, "IMU data");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to create file:", imageFilePath);
	}

}
#endif


#endif	//	_ENABLE_CAMERA_
