//*****************************************************************************
//*		controller_remoteview.cpp		(c) 2023 by Mark Sproul
//*				Controller base class
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Sep  9,	2023	<MLS> Created controller_remoteview.cpp
//*****************************************************************************

#ifdef _ENABLE_CTRL_IMAGE_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>



#include	"alpaca_defs.h"
#include	"discovery_lib.h"
#include	"helper_functions.h"
#include	"sendrequest_lib.h"
#include	"opencv_utils.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"widget.h"
#include	"controller.h"
#include	"controller_remoteview.h"


#include	"fits_opencv.h"


#include	"controller.h"
#include	"controller_image.h"
#include	"windowtab_imageinfo.h"




extern char	gFullVersionString[];

#define	kWindowWidth	1100
#define	kWindowHeight	800

//**************************************************************************************
enum
{
	kTab_Image	=	1,
	kTab_ImageInfo,
	kTab_About,

	kTab_Count

};


////#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//////**************************************************************************************
////ControllerRemote::ControllerRemote(	const char			*argWindowName,
////											TYPE_REMOTE_DEV		*alpacaDevice)
////			:Controller(	argWindowName,
////							kWindowWidth,
////							kWindowHeight)
////#else
//////**************************************************************************************
////ControllerRemote::ControllerRemote(	const char			*argWindowName,
////									IplImage			*downloadedImage,
////									TYPE_BinaryImageHdr	*binaryImageHdr)
////			:Controller(	argWindowName,
////							kWindowWidth,
////							kWindowHeight)
////#endif // _USE_OPENCV_CPP_
//**************************************************************************************
ControllerRemote::ControllerRemote(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice)
	:ControllerCamera(argWindowName, alpacaDevice, kWindowWidth,  kWindowHeight)
{

	CONSOLE_DEBUG(__FUNCTION__);

	InitClassVariables();

//	//*	deal with the binary image header if it was supplied
//	if (binaryImageHdr != NULL)
//	{
//	char	elementTypeStr[32];
//	char	transmitTypeStr[32];
//
//		CONSOLE_DEBUG("Setting binary image header data");
//		cBinaryImageHdr	=	*binaryImageHdr;
//
//		GetBinaryElementTypeString(cBinaryImageHdr.ImageElementType,		elementTypeStr);
//		GetBinaryElementTypeString(cBinaryImageHdr.TransmissionElementType,	transmitTypeStr);
//
//		CONSOLE_DEBUG_W_NUM("MetadataVersion        \t=",	cBinaryImageHdr.MetadataVersion);
//		CONSOLE_DEBUG_W_NUM("ErrorNumber            \t=",	cBinaryImageHdr.ErrorNumber);
//		CONSOLE_DEBUG_W_NUM("ClientTransactionID    \t=",	cBinaryImageHdr.ClientTransactionID);
//		CONSOLE_DEBUG_W_NUM("ServerTransactionID    \t=",	cBinaryImageHdr.ServerTransactionID);
//		CONSOLE_DEBUG_W_NUM("DataStart              \t=",	cBinaryImageHdr.DataStart);
//		CONSOLE_DEBUG_W_STR("ImageElementType       \t=",	elementTypeStr);
//		CONSOLE_DEBUG_W_STR("TransmissionElementType\t=",	transmitTypeStr);
//		CONSOLE_DEBUG_W_NUM("Rank                   \t=",	cBinaryImageHdr.Rank);
//		CONSOLE_DEBUG_W_NUM("Dimension1             \t=",	cBinaryImageHdr.Dimension1);
//		CONSOLE_DEBUG_W_NUM("Dimension2             \t=",	cBinaryImageHdr.Dimension2);
//		CONSOLE_DEBUG_W_NUM("Dimension3             \t=",	cBinaryImageHdr.Dimension3);
//
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_MetadataVersionVal,			cBinaryImageHdr.MetadataVersion);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ErrorNumberVal,				cBinaryImageHdr.ErrorNumber);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ClientTransactionIDVal,		(int32_t)cBinaryImageHdr.ClientTransactionID);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_ServerTransactionIDVal,		(int32_t)cBinaryImageHdr.ServerTransactionID);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_DataStartVal,				cBinaryImageHdr.DataStart);
//		SetWidgetText(	kTab_ImageInfo, kImageInfo_ImageElementTypeVal,			elementTypeStr);
//		SetWidgetText(	kTab_ImageInfo, kImageInfo_TransmissionElementTypeVal,	transmitTypeStr);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_RankVal,						cBinaryImageHdr.Rank);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension1Val,				cBinaryImageHdr.Dimension1);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension2Val,				cBinaryImageHdr.Dimension2);
//		SetWidgetNumber(kTab_ImageInfo, kImageInfo_Dimension3Val,				cBinaryImageHdr.Dimension3);
//
//
//		SetWidgetText(		kTab_Image, kImageDisplay_LiveOrDownLoad,	"Downloaded image");
//		SetWidgetBGColor(	kTab_Image, kImageDisplay_LiveOrDownLoad,	CV_RGB(0,	200, 0));
//	}
//	else
//	{
//		CONSOLE_DEBUG("Setting Live image");
//		SetWidgetText(		kTab_Image, kImageDisplay_LiveOrDownLoad,	"LIVE image!!!!!!!");
//		SetWidgetBGColor(	kTab_Image, kImageDisplay_LiveOrDownLoad,	CV_RGB(255,	255, 77));
//	}

	SetWidgetText(		kTab_Image, kImageDisplay_LiveOrDownLoad,	"LIVE image!!!!!!!");
	SetWidgetBGColor(	kTab_Image, kImageDisplay_LiveOrDownLoad,	CV_RGB(255,	255, 77));

//	//*	the downloaded image needs to be copied and/or resized to the displayed image
//	if (downloadedImage != NULL)
//	{
//		SetLiveWindowImage(downloadedImage);
//	}
//	else
//	{
//		CONSOLE_DEBUG("No image was specfified");
//	}
}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerRemote::~ControllerRemote(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, NULL);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//++	#warning "OpenCV++ not tested"
	CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//*	free up the image memory
//	if (cDownLoadedImage != NULL)
//	{
//		CONSOLE_DEBUG("Deleting cDownLoadedImage!!!!!!!!!!!!!!!!!!!!!!!!");
//		delete cDownLoadedImage;
//		cDownLoadedImage	=	NULL;
//	}
//	if (cDisplayedImage != NULL)
//	{
//		CONSOLE_DEBUG("Deleting cDisplayedImage!!!!!!!!!!!!!!!!!!!!!!!!");
//		delete cDisplayedImage;
//		cDisplayedImage	=	NULL;
//	}
#else
	//--------------------------------------------
	//*	free up the image memory
//	if (cDownLoadedImage != NULL)
//	{
////		CONSOLE_DEBUG_W_LHEX("Release cDownLoadedImage", (unsigned long)cDownLoadedImage);
//		cvReleaseImage(&cDownLoadedImage);
//		cDownLoadedImage	=	NULL;
//	}
//	if (cDisplayedImage != NULL)
//	{
////		CONSOLE_DEBUG_W_LHEX("Release cDisplayedImage", (unsigned long)cDisplayedImage);
//		cvReleaseImage(&cDisplayedImage);
//		cDisplayedImage	=	NULL;
//	}
#endif // _USE_OPENCV_CPP_

	//--------------------------------------------
	//*	delete the window tab objects
	DELETE_OBJ_IF_VALID(cImageTabObjPtr);
	DELETE_OBJ_IF_VALID(cImageInfoTabObjcPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerRemote::InitClassVariables(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
//	cDownLoadedImage	=	NULL;
//	cDisplayedImage		=	NULL;
//	cColorImage			=	NULL;

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
void	ControllerRemote::SetupWindowControls(void)
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

//	//---------------------------------------------------------------------
//	SetTabText(kTab_ImageInfo,		"Image Info");
//	CONSOLE_DEBUG(__FUNCTION__);
//	cImageInfoTabObjcPtr		=	new WindowTabImageInfo(	cWidth, cHeight, cBackGrndColor, cWindowName, &cBinaryImageHdr);
//	CONSOLE_DEBUG(__FUNCTION__);
//	if (cImageInfoTabObjcPtr != NULL)
//	{
//		SetTabWindow(kTab_ImageInfo,	cImageInfoTabObjcPtr);
//		cImageInfoTabObjcPtr->SetParentObjectPtr(this);
//	}

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


#endif // _ENABLE_CTRL_IMAGE_
