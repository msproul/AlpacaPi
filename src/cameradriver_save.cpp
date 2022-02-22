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
//*	Redistributions of this source code must retain this copyright notice.
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
//*****************************************************************************

#ifdef _ENABLE_CAMERA_

//*	OpenCV png file creation takes WAY too long, don't use it.
//#define	_ENABLE_PNG_

#include	<stdio.h>
#include	<string.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#ifdef _USE_OPENCV_
	#ifdef _USE_OPENCV_CPP_
		#include	<opencv2/opencv.hpp>
	#else
		#include "opencv/highgui.h"
		#include "opencv2/highgui/highgui_c.h"
		#include "opencv2/imgproc/imgproc_c.h"
		#include "opencv2/core/version.hpp"
	#endif // _USE_OPENCV_CPP_
#endif

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

//*****************************************************************************
void	CameraDriver::SaveImageData(void)
{
int	ii;

	CONSOLE_DEBUG_W_NUM("cSaveNextImage\t=", cSaveNextImage);
	CONSOLE_DEBUG_W_NUM("cSaveAllImages\t=", cSaveAllImages);
	cNumFramesSaved++;
	cTotalFramesSaved++;
	CONSOLE_DEBUG_W_NUM("cNumFramesSaved=", cNumFramesSaved);

	for (ii=0; ii<kMaxDataProducts; ii++)
	{
		memset(&cOtherDataProducts[ii], 0, sizeof(TYPE_FILENAME));
	}
	cOtherDataCnt	=	0;

	if (cCameraDataBuffer != NULL)
	{
	#ifdef _INCLUDE_HISTOGRAM_
		CalculateHistogramArray();
		SaveHistogramFile();
	#endif // _INCLUDE_HISTOGRAM_


	#ifdef _USE_OPENCV_
		SaveOpenCVImage();
	#endif	//	_USE_OPENCV_

	#if defined(_ENABLE_JPEGLIB_) && !defined(_USE_OPENCV_)
		SaveUsingJpegLib();
	#endif	//	_ENABLE_JPEGLIB_


	//*	we want FITS to be last so it can include info about other save data products
	#ifdef _ENABLE_FITS_
		SaveImageAsFITS();
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
		strcpy(imageFilePath, kImageDataDir);
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


//*****************************************************************************
//*	wget http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure%20Content-Type:%20-dDuration=0.011&Light=true
//*	wget http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/imagearray
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


	SETUP_TIMING();

//	CONSOLE_DEBUG(__FUNCTION__);
	GenerateFileNameRoot();

	if (cOpenCV_Image != NULL)
	{
		cvReleaseImage(&cOpenCV_Image);
		cOpenCV_Image	=	NULL;
	}
	if (cOpenCV_Image != NULL)
	{
		cvReleaseImage(&cOpenCV_LiveDisplay);
		cOpenCV_LiveDisplay	=	NULL;
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
			cOpenCV_Image	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
			imageDataLen	=	width * height;
			break;

		case kImageType_RAW16:
		//	CONSOLE_DEBUG("kImageType_RAW16");
			cOpenCV_Image	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
			imageDataLen	=	width * height * 2;
			break;


		case kImageType_RGB24:
		//	CONSOLE_DEBUG("kImageType_RGB24");
			cOpenCV_Image	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
			imageDataLen	=	width * height * 3;
			break;

		case kImageType_Y8:
		default:
			cOpenCV_Image	=	NULL;
			break;

	}
	DEBUG_TIMING("Stop point 1 (milliseconds)\t=");

	if (imageDataPtr != NULL)
	{
		if (cOpenCV_Image != NULL)
		{
			//*	what size did openCV make the image
	//		cvShowImage(cOpenCV_ImgWindowName, cOpenCV_Image);
	//		cvWaitKey(1);

//			CONSOLE_DEBUG_W_STR("Image SUCCESS!!!!!!!!!!!!!!-", cameraSerialNum);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_Image->imageSize\t=", cOpenCV_Image->imageSize);
//			CONSOLE_DEBUG_W_LONG("imageDataLen\t\t=",			imageDataLen);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_Image->nChannels\t=",	cOpenCV_Image->nChannels);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_Image->depth\t=",		cOpenCV_Image->depth);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_Image->width\t=",		cOpenCV_Image->width);
//			CONSOLE_DEBUG_W_NUM("cOpenCV_Image->widthStep\t=",	cOpenCV_Image->widthStep);

			bytesPerPixel		=	(cOpenCV_Image->depth / 8) * cOpenCV_Image->nChannels;
			bytesPerPixel2		=	cOpenCV_Image->widthStep / cOpenCV_Image->width;
			openCVimageWidth	=	cOpenCV_Image->widthStep / bytesPerPixel;
//			CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",		bytesPerPixel);
//			CONSOLE_DEBUG_W_NUM("bytesPerPixel2\t=",	bytesPerPixel2);
//			CONSOLE_DEBUG_W_NUM("openCVimageWidth\t=",	openCVimageWidth);
			if (bytesPerPixel != bytesPerPixel2)
			{
				CONSOLE_DEBUG("Houston, we have a problem");
				CONSOLE_DEBUG("bytes per pixel is messed up");
				CONSOLE_ABORT(__FUNCTION__);
			}
			if (openCVimageWidth == cOpenCV_Image->width)
			{
			//	CONSOLE_DEBUG("Normal image data !!!!!!!!!!!!!!");
				memcpy(cOpenCV_Image->imageData, imageDataPtr, imageDataLen);
			}
			else
			{
			char	*ocv_rowPtr;
			char	*img_rowPtr;
			int		rowLength;

				//*	we have to copy over the image a line at a time
//				CONSOLE_DEBUG("Miss match image data !!!!!!!!!!!!!!");
				ocv_rowPtr	=	cOpenCV_Image->imageData;
				img_rowPtr	=	(char *)imageDataPtr;
				rowLength	=	cOpenCV_Image->width * bytesPerPixel;
				for (ii=0; ii<cOpenCV_Image->height; ii++)
				{
					memcpy(ocv_rowPtr, img_rowPtr, rowLength);

					ocv_rowPtr	+=	cOpenCV_Image->widthStep;
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
int	CameraDriver::SaveOpenCVImage(void)
{
int			bytesPerPixel;
int			openCVerr;
char		imageFileName[64];
char		imageFilePath[128];
//int		quality[3] = {CV_IMWRITE_PNG_COMPRESSION, 200, 0};
int			quality[3] = {16, 200, 0};

	SETUP_TIMING();

	if (cOpenCV_Image != NULL)
	{
		bytesPerPixel		=	(cOpenCV_Image->depth / 8) * cOpenCV_Image->nChannels;
		if (bytesPerPixel != 2)
		{
			//*	save as JPEG
			strcpy(imageFileName, cFileNameRoot);
			strcat(imageFileName, ".jpg");

			strcpy(imageFilePath, kImageDataDir);
			strcat(imageFilePath, "/");
			strcat(imageFilePath, imageFileName);

			strcpy(cLastJpegImageName, imageFilePath);	//*	save the full image path for the web server
			openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_Image, quality);
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
		if (cOpenCV_Image->depth == 16)
		{
			//*	OpenCV png file creation takes WAY too long, use caution
			START_TIMING();
			//*	save as PNG
			strcpy(imageFileName, cFileNameRoot);
			strcat(imageFileName, ".png");

			strcpy(imageFilePath, kImageDataDir);
			strcat(imageFilePath, "/");
			strcat(imageFilePath, imageFileName);

			strcpy(cLastJpegImageName, imageFilePath);	//*	save the full image path for the web server
			openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_Image, quality);
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

		keyPointCnt	=	ProcessORB_Image(cOpenCV_Image, cFileNameRoot);

	#endif // _ENABLE_STAR_SEARCH_
	}
	return(0);
}


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
	strcpy(filePath, kImageDataDir);
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
		fprintf(filePointer, "Frames captured=%d\r\n",			cNumFramesSaved);
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



#endif	//	_ENABLE_CAMERA_
