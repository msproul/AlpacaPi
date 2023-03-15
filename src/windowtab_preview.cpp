//*****************************************************************************
//*		windowtab_preview.cpp		(c) 2020 by Mark Sproul
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 23,	2020	<MLS> Created windowtab_preview.cpp
//*	Jun 25,	2020	<MLS> Added DownloadImage()
//*	Jun 25,	2020	<MLS> Downloading image via Alpaca protocol working (rgbarray)
//*	Jun 29,	2020	<MLS> Added saving the downloaded image locally
//*	Aug 11,	2020	<MLS> Added autoexposure radio button to preview window
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_preview.h"

#include	"windowtab.h"
#include	"windowtab_preview.h"


#define	kMaxReadOutModes		5


//**************************************************************************************
WindowTabPreview::WindowTabPreview(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCVdownLoadedImage	=	NULL;
	strcpy(cAlpacaDeviceName, "");
	memset(&cAlpacaDevInfo, 0, sizeof(TYPE_REMOTE_DEV));

	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabPreview::~WindowTabPreview(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cOpenCVdownLoadedImage != NULL)
	{
		CONSOLE_DEBUG("destroy old image");
		SetWidgetImage(kPreviewBox_ImageDisplay, NULL);
		cvReleaseImage(&cOpenCVdownLoadedImage);
		cOpenCVdownLoadedImage	=	NULL;
	}
}


//**************************************************************************************
void	WindowTabPreview::SetupWindowControls(void)
{
int			yLoc;
int			yLocSave;
int			iii;
IplImage	*logoImage;

//*	create our own set of column offsets
int		myClm1_offset;
int		myClm2_offset;
int		myClm3_offset;
int		myClm4_offset;
int		myClm5_offset;
int		myClm6_offset;
//int		myClm7_offset;
//int		myClm8_offset;
//int		myClm9_offset;
//int		myClm10_offset;
int		myClm11_offset;
//int		myClm12_offset;
int		myClmWidth;

	myClmWidth		=	cWidth / 12;
	myClmWidth		-=	2;
	myClm1_offset	=	3;
	myClm2_offset	=	1 * myClmWidth;
	myClm3_offset	=	2 * myClmWidth;
	myClm4_offset	=	3 * myClmWidth;
	myClm5_offset	=	4 * myClmWidth;
	myClm6_offset	=	5 * myClmWidth;
//	myClm7_offset	=	6 * myClmWidth;
//	myClm8_offset	=	7 * myClmWidth;
//	myClm9_offset	=	8 * myClmWidth;
//	myClm10_offset	=	9 * myClmWidth;
	myClm11_offset	=	10 * myClmWidth;
//	myClm12_offset	=	11 * myClmWidth;


//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kPreviewBox_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kPreviewBox_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kPreviewBox_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	//------------------------------------------
	SetWidget(			kPreviewBox_Size,		0,			yLoc,	myClmWidth,	cSmallBtnHt	);
	SetWidgetFont(		kPreviewBox_Size, 	kFont_Medium);
	SetWidgetType(		kPreviewBox_Size,	kWidgetType_RadioButton);
	SetWidgetText(		kPreviewBox_Size,	"image size");
	SetWidgetChecked(	kPreviewBox_Size, true);

	//*	the logo goes the same vertical placement as the size button
	SetWidget(		kPreviewBox_Logo,	myClm5_offset,	yLoc,	cLogoWidth,	cLogoHeight	);
	SetWidgetType(	kPreviewBox_Logo,	kWidgetType_Image);
	//*	now figure out which logo belongs
	logoImage	=	NULL;
	CONSOLE_DEBUG_W_STR("cAlpacaDeviceName=", cAlpacaDeviceName);
	if (strcasestr(cAlpacaDeviceName, "ZWO") != NULL)
	{
		logoImage		=	cvLoadImage("logos/zwo-logo.png",		CV_LOAD_IMAGE_COLOR);
	}
	else if (strcasestr(cAlpacaDeviceName, "Atik") != NULL)
	{
		logoImage		=	cvLoadImage("logos/atik-logo.png",	CV_LOAD_IMAGE_COLOR);
	}
	else if ((strcasestr(cAlpacaDeviceName, "toup") != NULL) || (strcasestr(cAlpacaDeviceName, "GCMOS") != NULL))
	{
		logoImage		=	cvLoadImage("logos/touptek-logo.png",	CV_LOAD_IMAGE_COLOR);
	}
	else if (strcasestr(cAlpacaDeviceName, "QHY") != NULL)
	{
		logoImage		=	cvLoadImage("logos/qhy-logo.png",		CV_LOAD_IMAGE_COLOR);
	}
	if (logoImage != NULL)
	{
		SetWidgetImage(kPreviewBox_Logo, logoImage);
	}

	yLoc			+=	cLogoHeight;
	yLoc			+=	2;

	//=======================================================
	//*	exposure with slider
	SetWidget(		kPreviewBox_Exposure_Label,		myClm1_offset,	yLoc,		myClmWidth,			cRadioBtnHt	);
	SetWidget(		kPreviewBox_Exposure_Slider,	myClm2_offset,	yLoc,		(3 * myClmWidth),	cRadioBtnHt	);
	SetWidget(		kPreviewBox_Exposure,			myClm5_offset,	yLoc,		myClmWidth,			cRadioBtnHt	);

	SetWidget(				kPreviewBox_Exposure_Up,	myClm6_offset+5,	yLoc -2,	cSmIconSize,		cSmIconSize);
	SetWidget(				kPreviewBox_Exposure_Down,	myClm6_offset+5,	yLoc + cSmIconSize,	cSmIconSize,cSmIconSize);
	SetWidgetBGColor(		kPreviewBox_Exposure_Up,	CV_RGB(255,	255,	255));
	SetWidgetBGColor(		kPreviewBox_Exposure_Down,	CV_RGB(255,	255,	255));
	SetWidgetBorderColor(	kPreviewBox_Exposure_Up,	CV_RGB(0,	0,	0));
	SetWidgetBorderColor(	kPreviewBox_Exposure_Down,	CV_RGB(0,	0,	0));
	SetWidgetIcon(			kPreviewBox_Exposure_Up,	kIcon_UpArrow);
	SetWidgetIcon(			kPreviewBox_Exposure_Down,	kIcon_DownArrow);
	SetWidgetTextColor(		kPreviewBox_Exposure_Up,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(		kPreviewBox_Exposure_Down,	CV_RGB(255,	0,	0));



	SetWidgetType(	kPreviewBox_Exposure_Slider,	kWidgetType_Slider);
	SetWidgetFont(	kPreviewBox_Exposure_Label,		kFont_Small);
	SetWidgetFont(	kPreviewBox_Exposure,			kFont_Small);
	SetWidgetFont(	kPreviewBox_Exposure_Slider,	kFont_Small);
	SetWidgetText(	kPreviewBox_Exposure_Label,	"Exp");
	SetWidgetSliderLimits(	kPreviewBox_Exposure_Slider,	0.001, 1000);
	SetWidgetSliderValue(	kPreviewBox_Exposure_Slider,	123.0);
	yLoc			+=	cSmIconSize * 2;
	yLoc			+=	2;

	yLoc			+=	8;

	//=======================================================
	//*	gain with slider
	SetWidget(		kPreviewBox_Gain_Label,		myClm1_offset,		yLoc,		myClmWidth,			cRadioBtnHt	);
	SetWidget(		kPreviewBox_Gain_Slider,	myClm2_offset,		yLoc,		(3 * myClmWidth),	cRadioBtnHt	);
	SetWidget(		kPreviewBox_Gain,			myClm5_offset,		yLoc,		myClmWidth,			cRadioBtnHt	);
	SetWidget(		kPreviewBox_Gain_Up,		myClm6_offset+5,	yLoc -2,	cSmIconSize,		cSmIconSize);
	SetWidget(		kPreviewBox_Gain_Down,		myClm6_offset+5,	yLoc + cSmIconSize,	cSmIconSize,		cSmIconSize);
	SetWidgetBGColor(kPreviewBox_Gain_Up,		CV_RGB(255,	255,	255));
	SetWidgetBGColor(kPreviewBox_Gain_Down,		CV_RGB(255,	255,	255));

	SetWidgetBorderColor(kPreviewBox_Gain_Up,	CV_RGB(0,	0,	0));
	SetWidgetBorderColor(kPreviewBox_Gain_Down,	CV_RGB(0,	0,	0));

	SetWidgetIcon(		kPreviewBox_Gain_Up,		kIcon_UpArrow);
	SetWidgetIcon(		kPreviewBox_Gain_Down,		kIcon_DownArrow);
	SetWidgetTextColor(	kPreviewBox_Gain_Up,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kPreviewBox_Gain_Down,		CV_RGB(255,	0,	0));


	SetWidgetType(	kPreviewBox_Gain_Slider,	kWidgetType_Slider);
	SetWidgetFont(	kPreviewBox_Gain_Label,		kFont_Small);
	SetWidgetFont(	kPreviewBox_Gain_Slider,	kFont_Small);
	SetWidgetFont(	kPreviewBox_Gain,			kFont_Small);
	SetWidgetText(	kPreviewBox_Gain_Label,	"Gain");
	yLoc			+=	cSmIconSize * 2;
	yLoc			+=	5;

	//=======================================================
	//*	Camera state (idle, waiting, etc)
	SetWidget(					kPreviewBox_State,	myClm1_offset,	yLoc,	myClmWidth * 2,		cRadioBtnHt	);
	SetWidgetFont(				kPreviewBox_State,	kFont_Small);
	SetWidgetJustification  (	kPreviewBox_State,	kJustification_Left);

	//=======================================================
	//*	Auto Exposure radio button
	SetWidget(		kPreviewBox_AutoExposure,	myClm4_offset,	yLoc,	myClmWidth * 2,		cRadioBtnHt	);
	SetWidgetFont(	kPreviewBox_AutoExposure,	kFont_Small);
	SetWidgetType(	kPreviewBox_AutoExposure,	kWidgetType_RadioButton);
	SetWidgetText(	kPreviewBox_AutoExposure,	"Auto Exposure");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	5;

	//=======================================================
	//*	Readout modes
	yLocSave	=	yLoc;
	yLoc		=	cTabVertOffset + cTitleHeight + 3;
	for (iii=0; iii<kMaxReadOutModes; iii++)
	{
		SetWidget(		(kPreviewBox_ReadMode0 + iii),	cWidth/2,	yLoc,	cWidth/4,		cRadioBtnHt	);
		SetWidgetFont(	(kPreviewBox_ReadMode0 + iii),	kFont_RadioBtn);
		SetWidgetType(	(kPreviewBox_ReadMode0 + iii),	kWidgetType_RadioButton);
//		SetWidgetValid(	(kPreviewBox_ReadMode0 + iii),	false);


		yLoc			+=	cRadioBtnHt;
		yLoc			+=	1;
	}
	SetWidgetText(	kPreviewBox_ReadMode0,	"opt1");
	SetWidgetText(	kPreviewBox_ReadMode1,	"opt2");
	SetWidgetText(	kPreviewBox_ReadMode2,	"opt3");
	SetWidgetText(	kPreviewBox_ReadMode3,	"opt4");

	SetWidgetOutlineBox(kPreviewBox_ReadModeOutline, kPreviewBox_ReadMode0, kPreviewBox_ReadMode4);
	yLoc			+=	4;


	SetWidget(		kPreviewBox_RefID,			cWidth/2,	yLoc,	cWidth/4,	cBtnHeight);
	SetWidgetText(	kPreviewBox_RefID,	"---");


	//=======================================================
	yLoc		=	cTabVertOffset + cTitleHeight + 3;
	SetWidget(			kPreviewBox_Reset,			myClm11_offset,	yLoc,	myClmWidth * 2,	cBtnHeight);
	SetWidgetType(		kPreviewBox_Reset,			kWidgetType_Button);
	SetWidgetText(		kPreviewBox_Reset,			"Reset");
	SetWidgetFont(		kPreviewBox_Reset,			kFont_Medium);
	SetWidgetBGColor(	kPreviewBox_Reset,			CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kPreviewBox_Reset,			CV_RGB(255,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	4;

	SetWidget(			kPreviewBox_StartExposure,	myClm11_offset,	yLoc,	myClmWidth * 2,cBtnHeight);
	SetWidgetType(		kPreviewBox_StartExposure,	kWidgetType_Button);
	SetWidgetText(		kPreviewBox_StartExposure,	"Take Picture");
	SetWidgetFont(		kPreviewBox_StartExposure,	kFont_Medium);
	SetWidgetBGColor(	kPreviewBox_StartExposure,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kPreviewBox_StartExposure,	CV_RGB(255,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	4;

	SetWidget(			kPreviewBox_GetPicture,	myClm11_offset,	yLoc,	myClmWidth * 2,cBtnHeight);
	SetWidgetType(		kPreviewBox_GetPicture,	kWidgetType_Button);
	SetWidgetText(		kPreviewBox_GetPicture,	"Get Picture");
	SetWidgetFont(		kPreviewBox_GetPicture,	kFont_Medium);
	SetWidgetBGColor(	kPreviewBox_GetPicture,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kPreviewBox_GetPicture,	CV_RGB(255,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	4;


	yLoc	=	yLocSave;
	SetWidget(			kPreviewBox_FileName,	0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(		kPreviewBox_FileName,	kFont_Medium);
	SetWidgetText(		kPreviewBox_FileName,	gFullVersionString);
	SetWidgetTextColor(	kPreviewBox_FileName,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetWidget(			kPreviewBox_ImageDisplay,	0,			yLoc,		cWidth,		((3 * cWidth) / 7));
	SetWidgetBGColor(	kPreviewBox_ImageDisplay,	CV_RGB(128,	128,	128));



	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kPreviewBox_IPaddr,
							kPreviewBox_Readall,
							kPreviewBox_ProgressBar,	//	kPreviewBox_AlpacaErrorMsg,
							-1,							//	kPreviewBox_LastCmdString,
							-1,							//	kPreviewBox_AlpacaLogo,
							-1);

	SetWidgetBGColor(		kPreviewBox_ProgressBar,	CV_RGB(0,	0,	0));
	SetWidgetTextColor(		kPreviewBox_ProgressBar,	CV_RGB(0,	255,	0));
	SetWidgetBorderColor(	kPreviewBox_ProgressBar,	CV_RGB(255,	255,	255));

}

//*****************************************************************************
void	WindowTabPreview::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool		validData;
char		dataString[64];

	switch(buttonIdx)
	{
		case kPreviewBox_Gain_Up:
			BumpGain(1);
			break;

		case kPreviewBox_Gain_Down:
			BumpGain(-1);
			break;

		case kPreviewBox_Exposure_Up:
			BumpExposure(0.002);
			break;

		case kPreviewBox_Exposure_Down:
			BumpExposure(-0.002);
			break;


		case kPreviewBox_AutoExposure:
			ToggleAutoExposure();
			break;

		case kPreviewBox_ReadMode0:
		case kPreviewBox_ReadMode1:
		case kPreviewBox_ReadMode2:
		case kPreviewBox_ReadMode3:
		case kPreviewBox_ReadMode4:
			sprintf(dataString, "ReadoutMode=%d", (buttonIdx - kPreviewBox_ReadMode0));
			validData	=	AlpacaSendPutCmd(	"camera", "readoutmode",	dataString);
			ForceUpdate();
			break;

//		case kPreviewBox_LiveMode:
//			ToggleLiveMode();
			break;

//		case kPreviewBox_SideBar:
//			ToggleSideBar();
			break;

//		case kPreviewBox_AutoExposure:
//			ToggleAutoExposure();
			break;

//		case kPreviewBox_DisplayImage:
//			ToggleDisplayImage();
			break;

//		case kPreviewBox_CoolerChkBox:
//			ToggleCooler();
			break;

		case kPreviewBox_Reset:
			validData	=	AlpacaSendPutCmd(	"camera", "abortexposure",	NULL);
			if (validData == false)
			{
				CONSOLE_DEBUG("abortexposure failed");
			}
			break;

		case kPreviewBox_StartExposure:
			StartExposure();
			break;

		case kPreviewBox_GetPicture:
			if ((cAlpacaCameraState == kALPACA_CameraState_Idle) ||
				(cAlpacaCameraState == kALPACA_CameraState_Error))
			{
				DownloadImage();
			}
			else
			{
				SetWidgetType(kPreviewBox_ProgressBar, kWidgetType_TextBox);
				SetWidgetText(kPreviewBox_ProgressBar, "Camera is busy");
			}
			break;

		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;

	}
	DisplayLastAlpacaCommand();
}


//*****************************************************************************
void	WindowTabPreview::SetDeviceInfo(TYPE_REMOTE_DEV *alpacaDevice)
{
IplImage	*logoImage;

	cAlpacaDevInfo	=	*alpacaDevice;
	CONSOLE_DEBUG_W_STR("hostName\t\t=",	alpacaDevice->hostName);
	CONSOLE_DEBUG_W_STR("deviceType\t\t=",	alpacaDevice->deviceTypeStr);
	CONSOLE_DEBUG_W_STR("deviceName\t\t=",	alpacaDevice->deviceNameStr);
	CONSOLE_DEBUG_W_STR("versionString\t=",	alpacaDevice->versionString);

	strcpy(cAlpacaDeviceName, alpacaDevice->deviceNameStr);
	logoImage	=	NULL;
	CONSOLE_DEBUG_W_STR("cAlpacaDeviceName=", cAlpacaDeviceName);
	if (strcasestr(cAlpacaDeviceName, "ZWO") != NULL)
	{
		logoImage		=	cvLoadImage("logos/zwo-logo.png",		CV_LOAD_IMAGE_COLOR);
	}
	else if (strcasestr(cAlpacaDeviceName, "Atik") != NULL)
	{
		logoImage		=	cvLoadImage("logos/atik-logo.png",	CV_LOAD_IMAGE_COLOR);
	}
	else if ((strcasestr(cAlpacaDeviceName, "toup") != NULL) || (strcasestr(cAlpacaDeviceName, "GCMOS") != NULL))
	{
		logoImage		=	cvLoadImage("logos/touptek-logo.png",	CV_LOAD_IMAGE_COLOR);
	}
	else if (strcasestr(cAlpacaDeviceName, "QHY") != NULL)
	{
		logoImage		=	cvLoadImage("logos/qhy-logo.png",		CV_LOAD_IMAGE_COLOR);
	}
	if (logoImage != NULL)
	{
		SetWidgetImage(kPreviewBox_Logo, logoImage);
	}
}

//*****************************************************************************
void	WindowTabPreview::SetReceivedFileName(const char *newFileName)
{
	strcpy(cDownLoadedFileNameRoot, newFileName);
}

//*****************************************************************************
void	WindowTabPreview::ForceUpdate(void)
{
ControllerPreview	*myPreviewController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;

	if (myPreviewController != NULL)
	{
		myPreviewController->cForceAlpacaUpdate	=	true;
	}
	else
	{
		CONSOLE_DEBUG("myPreviewController is NULL");
	}
}


//*****************************************************************************
void	WindowTabPreview::BumpGain(const int howMuch)
{
ControllerPreview	*myPreviewController;

	CONSOLE_DEBUG(__FUNCTION__);
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;

	if (myPreviewController != NULL)
	{
		myPreviewController->BumpGain(howMuch);
	}
	else
	{
		CONSOLE_DEBUG("myPreviewController is NULL");
	}
}

//*****************************************************************************
void	WindowTabPreview::BumpExposure(const double howMuch)
{
ControllerPreview	*myPreviewController;

	CONSOLE_DEBUG(__FUNCTION__);
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;

	if (myPreviewController != NULL)
	{
		myPreviewController->BumpExposure(howMuch);
	}
	else
	{
		CONSOLE_DEBUG("myPreviewController is NULL");
	}
}

//*****************************************************************************
void	WindowTabPreview::ToggleAutoExposure(void)
{
ControllerPreview	*myPreviewController;

	CONSOLE_DEBUG(__FUNCTION__);
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;

	if (myPreviewController != NULL)
	{
		myPreviewController->ToggleAutoExposure();
	}
	else
	{
		CONSOLE_DEBUG("myPreviewController is NULL");
	}
}

//*****************************************************************************
void	WindowTabPreview::StartExposure(void)
{
ControllerPreview	*myPreviewController;

	CONSOLE_DEBUG(__FUNCTION__);
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;

	if (myPreviewController != NULL)
	{
		myPreviewController->StartExposure();
	}
	else
	{
		CONSOLE_DEBUG("myPreviewController is NULL");
	}
}

//*****************************************************************************
void	WindowTabPreview::DownloadImage(void)
{
ControllerPreview	*myPreviewController;
IplImage			*originalImage;
int					liveDispalyWidth;
int					liveDisplayHeight;
int					reduceFactor;
char				textBuf[128];
double				download_MBytes;
double				download_MB_per_sec;
double				download_seconds;
char				fileName[256];
int					quality[3] = {16, 200, 0};
int					openCVerr;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cOpenCVdownLoadedImage != NULL)
	{
		CONSOLE_DEBUG("destroy old image");
		SetWidgetImage(kPreviewBox_ImageDisplay, NULL);
		cvReleaseImage(&cOpenCVdownLoadedImage);
		cOpenCVdownLoadedImage	=	NULL;
	}

	originalImage		=	NULL;
	myPreviewController	=	(ControllerPreview *)cParentObjPtr;
	if (myPreviewController != NULL)
	{
//		CONSOLE_DEBUG("Starting download");

		originalImage	=	myPreviewController->DownloadImage();
		if (originalImage != NULL)
		{
			CONSOLE_DEBUG("Download complete");
//			CONSOLE_DEBUG("Creating small image");
			reduceFactor		=	1;
			liveDispalyWidth	=	originalImage->width;
			liveDisplayHeight	=	originalImage->height;
			while (liveDispalyWidth > 700)
			{
				reduceFactor++;
				liveDispalyWidth	=	originalImage->width / reduceFactor;
				liveDisplayHeight	=	originalImage->height / reduceFactor;
			}
			CONSOLE_DEBUG_W_NUM("reduceFactor\t=", reduceFactor);
			CONSOLE_DEBUG_W_NUM("liveDispalyWidth\t=", liveDispalyWidth);
			CONSOLE_DEBUG_W_NUM("liveDisplayHeight\t=", liveDisplayHeight);
			cOpenCVdownLoadedImage	=	cvCreateImage(cvSize(	liveDispalyWidth,
																liveDisplayHeight),
																IPL_DEPTH_8U,
																3);
			if (cOpenCVdownLoadedImage != NULL)
			{
//				CONSOLE_DEBUG("Resizing image");
				cvResize(originalImage, cOpenCVdownLoadedImage, CV_INTER_LINEAR);
				SetWidgetImage(kPreviewBox_ImageDisplay, cOpenCVdownLoadedImage);
			}
			//======================================
			//*	save the image
			strcpy(fileName, cDownLoadedFileNameRoot);
			strcat(fileName, ".jpg");

			CONSOLE_DEBUG_W_STR("Saving image as", fileName);
			openCVerr	=	cvSaveImage(fileName, originalImage, quality);
			if (openCVerr == 0)
			{
			int		openCVerrorCode;
			char	*errorMsgPtr;

				CONSOLE_DEBUG_W_NUM("Error saving file\t=", openCVerr);
				openCVerrorCode	=	cvGetErrStatus();
				CONSOLE_DEBUG_W_NUM("openCVerrorCode\t=", openCVerrorCode);
				errorMsgPtr	=	(char *)cvErrorStr(openCVerrorCode);
				CONSOLE_DEBUG_W_STR("errorMsgPtr\t=", errorMsgPtr);
			}
			cvReleaseImage(&originalImage);


			download_MBytes		=	1.0 * myPreviewController->cLastDownload_Bytes / (1024.0 * 1024.0);
			download_seconds	=	1.0 * myPreviewController->cLastDownload_Millisecs / 1000.0;
			download_MB_per_sec	=	download_MBytes / download_seconds;

			sprintf(textBuf,	"%2.2f megabytes in %2.2f seconds =%2.2f mbytes/sec",
								download_MBytes,
								download_seconds,
								download_MB_per_sec
								);


			SetWidgetType(kPreviewBox_ProgressBar, kWidgetType_TextBox);
			SetWidgetText(kPreviewBox_ProgressBar, textBuf);
		}
		else
		{
			SetWidgetType(kPreviewBox_ProgressBar, kWidgetType_TextBox);
			SetWidgetText(kPreviewBox_ProgressBar, "Failed to download image, no image exists");
			CONSOLE_DEBUG("Failed to download image");
		}
	}
}


//*****************************************************************************
void	WindowTabPreview::UpdateCameraState(TYPE_ALPACA_CAMERASTATE newCameraState)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cAlpacaCameraState	=	newCameraState;
//	CONSOLE_DEBUG_W_NUM("cAlpacaCameraState\t=", cAlpacaCameraState);
}
