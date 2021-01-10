//*****************************************************************************
//*		controller_image.cpp		(c) 2020 by Mark Sproul
//*
//*
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 27,	2020	<MLS> Created controller_image.cpp
//*****************************************************************************


#ifdef _ENABLE_CTRL_IMAGE_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	1100
#define	kWindowHeight	800

#include	"alpaca_defs.h"
#include	"windowtab_image.h"
#ifdef _ENABLE_SKYTRAVEL_
	#include	"windowtab_skytravel.h"
	#include	"lx200_com.h"
#endif // _ENABLE_SKYTRAVEL_

#include	"windowtab_about.h"


#include	"controller.h"
#include	"controller_image.h"


extern char	gFullVersionString[];


//**************************************************************************************
enum
{
	kTab_Image	=	0,
#ifdef _ENABLE_SKYTRAVEL_
	kTab_SkyTravel,
#endif
	kTab_About,

	kTab_Count

};



//**************************************************************************************
ControllerImage::ControllerImage(	const char *argWindowName, IplImage *downloadedImage)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
int					liveDispalyWidth;
int					liveDisplayHeight;
int					reduceFactor;

	CONSOLE_DEBUG(__FUNCTION__);

	cDownLoadedImage	=	downloadedImage;
	cDisplayedImage		=	NULL;

	cImageTabObjPtr		=	NULL;
#ifdef _ENABLE_SKYTRAVEL_
	cSkyTravelTabOjbPtr	=	NULL;
#endif // _ENABLE_SKYTRAVEL_
	cAboutBoxTabObjPtr	=	NULL;

	SetupWindowControls();

	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,		gFullVersionString);

//	SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, cDownLoadedImage);

	//*	the downloaded image needs to be copied and/or resized to the displayed image
	if (cDownLoadedImage != NULL)
	{
//			CONSOLE_DEBUG("Creating small image");
		reduceFactor		=	1;
		liveDispalyWidth	=	cDownLoadedImage->width;
		liveDisplayHeight	=	cDownLoadedImage->height;

		CONSOLE_DEBUG_W_NUM("cDownLoadedImage->width\t=",	cDownLoadedImage->width);
		CONSOLE_DEBUG_W_NUM("cDownLoadedImage->height\t=",	cDownLoadedImage->height);

		CONSOLE_DEBUG_W_NUM("kWindowHeight\t=", kWindowHeight);
		while ((liveDispalyWidth > kWindowWidth) || (liveDisplayHeight > (kWindowHeight - 50)))
		{
			CONSOLE_DEBUG_W_NUM("liveDisplayHeight\t=", liveDisplayHeight);
			reduceFactor++;
			liveDispalyWidth	=	cDownLoadedImage->width / reduceFactor;
			liveDisplayHeight	=	cDownLoadedImage->height / reduceFactor;
		}
		CONSOLE_DEBUG_W_NUM("reduceFactor\t=", reduceFactor);
		CONSOLE_DEBUG_W_NUM("liveDispalyWidth\t=", liveDispalyWidth);
		CONSOLE_DEBUG_W_NUM("liveDisplayHeight\t=", liveDisplayHeight);

		cDisplayedImage	=	cvCreateImage(cvSize(	liveDispalyWidth,
													liveDisplayHeight),
													IPL_DEPTH_8U,
													3);
		if (cDisplayedImage != NULL)
		{
			CONSOLE_DEBUG("Resizing image");

			//*	Check to see if the original is color
			if ((cOpenCV_Image->nChannels == 3) && (cOpenCV_Image->depth == 8))
			{
				CONSOLE_DEBUG("Original is 8 bit color");
				cvResize(cDownLoadedImage, cDisplayedImage, CV_INTER_LINEAR);
			}
			else if ((cOpenCV_Image->nChannels == 1) && (cOpenCV_Image->depth == 8))
			{
				CONSOLE_DEBUG("Original is8 bit B/W");
				cvCvtColor(cDownLoadedImage, cDisplayedImage, CV_GRAY2RGB);
			}

			SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, cDisplayedImage);
		}
		else
		{
			CONSOLE_DEBUG("Failed to create new image");
		}
	}
	else
	{
#ifdef _ENABLE_SKYTRAVEL_
		ProcessTabClick(kTab_SkyTravel);
#endif // _ENABLE_SKYTRAVEL_
	}
}



//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerImage::~ControllerImage(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetImage(kTab_Image, kImageDisplay_ImageDisplay, NULL);
	if (cDownLoadedImage != NULL)
	{
		CONSOLE_DEBUG("destroy downloaded image");
		cvReleaseImage(&cDownLoadedImage);
		cDownLoadedImage	=	NULL;
	}
	if (cDisplayedImage != NULL)
	{
		CONSOLE_DEBUG("destroy display image");
		cvReleaseImage(&cDisplayedImage);
		cDisplayedImage	=	NULL;
	}

	if (cImageTabObjPtr != NULL)
	{
		delete cImageTabObjPtr;
		cImageTabObjPtr	=	NULL;
	}
#ifdef _ENABLE_SKYTRAVEL_
	if (cSkyTravelTabOjbPtr != NULL)
	{
		delete cSkyTravelTabOjbPtr;
		cSkyTravelTabOjbPtr	=	NULL;
	}
#endif // _ENABLE_SKYTRAVEL_
	if (cAboutBoxTabObjPtr != NULL)
	{
		delete cAboutBoxTabObjPtr;
		cAboutBoxTabObjPtr	=	NULL;
	}
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

#ifdef _ENABLE_SKYTRAVEL_
	SetTabText(kTab_SkyTravel,	"SkyTravel");
	cSkyTravelTabOjbPtr		=	new WindowTabSkyTravel(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSkyTravelTabOjbPtr != NULL)
	{
		SetTabWindow(kTab_SkyTravel,	cSkyTravelTabOjbPtr);
		cSkyTravelTabOjbPtr->SetParentObjectPtr(this);
	}
#endif // _ENABLE_SKYTRAVEL_

	SetTabText(kTab_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

//	SetWidgetFont(kTab_Video,	kUSBselect_IPaddr, kFont_Medium);

//	SetWidgetText(kTab_Video,	kUSBselect_IPaddr,	cUSBpath);
//	SetWidgetText(kTab_About,	kAboutBox_IPaddr,	cUSBpath);
}

//**************************************************************************************
void	ControllerImage::RunBackgroundTasks(void)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		needToUpdate;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
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
#ifdef _ENABLE_SKYTRAVEL_
	if (cSkyTravelTabOjbPtr != NULL)
	{
		cSkyTravelTabOjbPtr->RunBackgroundTasks();
	}
#endif

}



//**************************************************************************************
void	ControllerImage::RefreshWindow(void)
{
	HandleWindowUpdate();
	cvWaitKey(100);
}


#endif // _ENABLE_CTRL_IMAGE_
