//*****************************************************************************
//*		windowtab_camera.cpp		(c) 2020 by Mark Sproul
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar  1,	2020	<MLS> Started on windowtab_camera.cpp
//*	Mar  6,	2020	<MLS> Added alpaca camera state to window
//*	Mar  6,	2020	<MLS> Added filter wheel display
//*	Mar  9,	2020	<MLS> Added SetTempartueEnable()
//*	Mar  9,	2020	<MLS> Added "R" indicator in IP field if "readall" is available
//*	Mar 16,	2020	<MLS> Added error message box to camera display
//*	Apr  5,	2020	<MLS> Added ToggleDisplayImage()
//*	Dec 26,	2020	<MLS> Started on image download for camera controller
//*	Dec 27,	2020	<MLS> Added DownloadImage()
//*	Jan 22,	2021	<MLS> Moved all logo png files to "logos" directory
//*	Feb 14,	2021	<MLS> Added UpdateSliderValue() to windowtab_camera
//*	Feb 14,	2021	<MLS> Slider now working for exposure
//*	Feb 14,	2021	<MLS> Slider now working for gain
//*	Mar 13,	2021	<MLS> Added support for savel all images ToggleSaveAll()
//*	Mar 27,	2021	<MLS> Added BumpOffset()
//*	Mar 27,	2021	<MLS> Offset slider fully working
//*	Dec 18,	2021	<MLS> Double click in the title bar causes connect to be sent
//*	Dec 22,	2021	<MLS> Added support for flipmode
//*	Feb 18,	2022	<MLS> Added SetCameraLogo()
//*	Feb 25,	2022	<MLS> Downloading image working with C++ opencv
//*	Apr 16,	2022	<MLS> Saving dowloaded image working with C++ opencv
//*	Jul  2,	2022	<MLS> Added FLIR logo to camera display
//*	Sep 18,	2022	<MLS> Added DisableFilterWheel()
//*	Sep 21,	2022	<MLS> Added display of remote IMU data
//*	Sep 21,	2022	<MLS> Added SetRemoteIMUdisplay()
//*****************************************************************************

#ifdef _ENABLE_CTRL_CAMERA_

#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"controller.h"
#include	"controller_camera.h"
#include	"windowtab.h"
#include	"windowtab_camera.h"
#include	"controller_image.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabCamera::WindowTabCamera(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName,
									const char	*deviceName,
									const bool	hasFilterWheel)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	CONSOLE_DEBUG(hasFilterWheel ? "Has FilterWheel" : "No Filterwheel");

	strcpy(cAlpacaDeviceName, "");
 //	memset(&cAlpacaDevInfo, 0, sizeof(TYPE_REMOTE_DEV));

	cForce8BitRead			=	false;
	cAllowBinaryDownload	=	true;
	cHasFilterWheel			=	hasFilterWheel;
	strcpy(cAlpacaDeviceName, deviceName);

	strcpy(cDownLoadedFileNameRoot, "unknown");

	cLastExposureUpdate_Millis	=	0;
	cLastGainUpdate_Millis		=	0;
	cLastOffsetUpdate_Millis	=	0;

	SetupWindowControls();
	ForceAlpacaUpdate();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabCamera::~WindowTabCamera(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabCamera::SetupWindowControls(void)
{
int			xLoc;
int			yLoc;
int			yLocSave;
int			yLocClm4;
int			yLocRest;
int			yLocExposure;
int			iii;
int			valueXloc;
int			updownXloc;
int			sliderWidth;
int			valueWidth;
char		textBuff[32];

//	CONSOLE_DEBUG(__FUNCTION__);


	for (iii=0; iii<kCameraBox_last; iii++)
	{
		SetWidgetTextColor(iii, CV_RGB(255,	255,	255));
	}

	//------------------------------------------
	yLoc	=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kCameraBox_Title, kCameraBox_Connected, yLoc, "AlpacaPi project");

	//------------------------------------------
	SetWidget(			kCameraBox_Size,	0,	yLoc,	cFullWidthBtn,	cSmallBtnHt	);
	SetWidgetFont(		kCameraBox_Size, 	kFont_Medium);
	SetWidgetType(		kCameraBox_Size,	kWidgetType_RadioButton);
	SetWidgetChecked(	kCameraBox_Size,	true);

	//*	the logo goes the same vertical placement as the size button
	SetWidget(		kCameraBox_Logo,	(cWidth - cLogoWidth),	yLoc,	cLogoWidth,	cLogoHeight	);
	SetWidgetType(	kCameraBox_Logo,	kWidgetType_Image);

	//*	now figure out which logo belongs
	SetCameraLogo();

	yLoc			+=	cLogoHeight;
	yLoc			+=	2;

	//=======================================================
	//*	exposure with slider
	valueWidth		=	cClmWidth;
	updownXloc		=	cWidth - cSmIconSize - 5;
	valueXloc		=	cWidth - valueWidth - cSmIconSize - 8;
	sliderWidth		=	valueXloc - cClm2_offset - 2;

	SetWidget(		kCameraBox_Exposure_Label,	cClm1_offset,	yLoc,		cClmWidth,		cRadioBtnHt	);
	SetWidget(		kCameraBox_Exposure_Slider,	cClm2_offset,	yLoc,		sliderWidth,	cRadioBtnHt	);
	SetWidget(		kCameraBox_Exposure,		valueXloc,		yLoc,		valueWidth,		cRadioBtnHt	);

	SetWidget(				kCameraBox_Exposure_Up,		updownXloc,	yLoc -2,	cSmIconSize,		cSmIconSize);
	SetWidget(				kCameraBox_Exposure_Down,	updownXloc,	yLoc + cSmIconSize,	cSmIconSize,cSmIconSize);
	SetWidgetBGColor(		kCameraBox_Exposure_Up,		CV_RGB(255,	255,	255));
	SetWidgetBGColor(		kCameraBox_Exposure_Down,	CV_RGB(255,	255,	255));

	SetWidgetBorderColor(	kCameraBox_Exposure_Up,		CV_RGB(0,	0,	0));
	SetWidgetBorderColor(	kCameraBox_Exposure_Down,	CV_RGB(0,	0,	0));
	SetWidgetIcon(			kCameraBox_Exposure_Up,		kIcon_UpArrow);
	SetWidgetIcon(			kCameraBox_Exposure_Down,	kIcon_DownArrow);
	SetWidgetTextColor(		kCameraBox_Exposure_Up,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(		kCameraBox_Exposure_Down,	CV_RGB(255,	0,	0));



	SetWidgetType(	kCameraBox_Exposure_Slider,	kWidgetType_Slider);
	SetWidgetFont(	kCameraBox_Exposure_Label,	kFont_Small);
	SetWidgetFont(	kCameraBox_Exposure,		kFont_Small);
	SetWidgetFont(	kCameraBox_Exposure_Slider,	kFont_Small);
	SetWidgetText(	kCameraBox_Exposure_Label,	"Exp");
	SetWidgetSliderLimits(	kCameraBox_Exposure_Slider,	0.001, 1000);
	SetWidgetSliderValue(	kCameraBox_Exposure_Slider,	123.0);
	yLoc			+=	cSmIconSize * 2;
	yLoc			+=	2;
	yLoc			+=	8;

	//=======================================================
	//*	gain with slider
	SetWidget(		kCameraBox_Gain_Label,	cClm1_offset,	yLoc,		cClmWidth,		cRadioBtnHt	);
	SetWidget(		kCameraBox_Gain_Slider,	cClm2_offset,	yLoc,		sliderWidth,	cRadioBtnHt	);
	SetWidget(		kCameraBox_Gain,		valueXloc,		yLoc,		valueWidth,		cRadioBtnHt	);
	SetWidget(		kCameraBox_Gain_Up,		updownXloc,		yLoc -2,			cSmIconSize,	cSmIconSize);
	SetWidget(		kCameraBox_Gain_Down,	updownXloc,		yLoc + cSmIconSize,	cSmIconSize,	cSmIconSize);
	SetWidgetBGColor(kCameraBox_Gain_Up,	CV_RGB(255,	255,	255));
	SetWidgetBGColor(kCameraBox_Gain_Down,	CV_RGB(255,	255,	255));

	SetWidgetBorderColor(kCameraBox_Gain_Up,	CV_RGB(0,	0,	0));
	SetWidgetBorderColor(kCameraBox_Gain_Down,	CV_RGB(0,	0,	0));

	SetWidgetIcon(		kCameraBox_Gain_Up,		kIcon_UpArrow);
	SetWidgetIcon(		kCameraBox_Gain_Down,	kIcon_DownArrow);
	SetWidgetTextColor(	kCameraBox_Gain_Up,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kCameraBox_Gain_Down,	CV_RGB(255,	0,	0));


	SetWidgetType(	kCameraBox_Gain_Slider,	kWidgetType_Slider);
	SetWidgetFont(	kCameraBox_Gain_Label,	kFont_Small);
	SetWidgetFont(	kCameraBox_Gain_Slider,	kFont_Small);
	SetWidgetFont(	kCameraBox_Gain,		kFont_Small);
	SetWidgetText(	kCameraBox_Gain_Label,	"Gain");
	yLoc			+=	cSmIconSize * 2;
	yLoc			+=	5;


	//=======================================================
	//*	Offset with slider
	SetWidget(			kCameraBox_Offset_Label,	cClm1_offset,	yLoc,		cClmWidth,		cRadioBtnHt	);
	SetWidget(			kCameraBox_Offset_Slider,	cClm2_offset,	yLoc,		sliderWidth,	cRadioBtnHt	);
	SetWidget(			kCameraBox_Offset,			valueXloc,		yLoc,		valueWidth,		cRadioBtnHt	);
	SetWidget(			kCameraBox_Offset_Up,		updownXloc,	yLoc -2,			cSmIconSize,	cSmIconSize);
	SetWidget(			kCameraBox_Offset_Down,		updownXloc,	yLoc + cSmIconSize,	cSmIconSize,	cSmIconSize);
	SetWidgetBGColor(	kCameraBox_Offset_Up,		CV_RGB(255,	255,	255));
	SetWidgetBGColor(	kCameraBox_Offset_Down,		CV_RGB(255,	255,	255));

	SetWidgetBorderColor(kCameraBox_Offset_Up,		CV_RGB(0,	0,	0));
	SetWidgetBorderColor(kCameraBox_Offset_Down,	CV_RGB(0,	0,	0));

	SetWidgetIcon(		kCameraBox_Offset_Up,		kIcon_UpArrow);
	SetWidgetIcon(		kCameraBox_Offset_Down,		kIcon_DownArrow);
	SetWidgetTextColor(	kCameraBox_Offset_Up,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kCameraBox_Offset_Down,		CV_RGB(255,	0,	0));


	SetWidgetType(		kCameraBox_Offset_Slider,	kWidgetType_Slider);
	SetWidgetFont(		kCameraBox_Offset_Label,	kFont_Small);
	SetWidgetFont(		kCameraBox_Offset_Slider,	kFont_Small);
	SetWidgetFont(		kCameraBox_Offset,			kFont_Small);
	SetWidgetText(		kCameraBox_Offset_Label,	"Offset");
	yLoc			+=	cSmIconSize * 2;
	yLoc			+=	5;



	//=======================================================
	//*	Camera state (idle, waiting, etc)
	SetWidget(					kCameraBox_State,	cClm1_offset,	yLoc,		cClmWidth * 2,		cRadioBtnHt	);
	SetWidgetFont(				kCameraBox_State,	kFont_Small);
	SetWidgetJustification  (	kCameraBox_State,	kJustification_Left);

	SetWidget(					kCameraBox_PercentCompleted,	cClm3_offset + 5,	yLoc,	cClmWidth * 4,		cRadioBtnHt	);
	SetWidgetFont(				kCameraBox_PercentCompleted,	kFont_Small);
	SetWidgetJustification  (	kCameraBox_PercentCompleted,	kJustification_Center);
	SetWidgetText(				kCameraBox_PercentCompleted,	"Percent complete");
	SetWidgetBGColor(			kCameraBox_PercentCompleted,	CV_RGB(0,	0,	0));
	SetWidgetTextColor(			kCameraBox_PercentCompleted,	CV_RGB(255,	0,	0));
	SetWidgetAltText(			kCameraBox_PercentCompleted,	"Exposure");


	yLoc			+=	cRadioBtnHt;
	yLoc			+=	5;

	yLocRest	=	yLoc;
	//=======================================================
	//*	Readout modes
	xLoc		=	5;
	yLocSave	=	yLoc;
	for (iii=0; iii<kMaxReadOutModes; iii++)
	{
//		SetWidget(		(kCameraBox_ReadMode0 + iii),	xLoc,	yLoc,	(cWidth / 3),	cRadioBtnHt);
		SetWidget(		(kCameraBox_ReadMode0 + iii),	xLoc,	yLoc,	(cClmWidth * 2),	cRadioBtnHt);
		SetWidgetFont(	(kCameraBox_ReadMode0 + iii),	kFont_RadioBtn);
		SetWidgetType(	(kCameraBox_ReadMode0 + iii),	kWidgetType_RadioButton);
		SetWidgetValid(	(kCameraBox_ReadMode0 + iii),	false);

		yLoc			+=	cRadioBtnHt;
		yLoc			+=	1;
	}
	SetWidgetText(	kCameraBox_ReadMode0,	"opt1");
	SetWidgetText(	kCameraBox_ReadMode1,	"opt2");
	SetWidgetText(	kCameraBox_ReadMode2,	"opt3");
	SetWidgetText(	kCameraBox_ReadMode3,	"opt4");

	SetWidgetOutlineBox(	kCameraBox_ReadModeOutline, kCameraBox_ReadMode0, kCameraBox_ReadMode4);
	//*	we are setting these now because the widget type gets changed depending on live mode
	SetWidgetTextColor(		kCameraBox_ReadModeOutline,	CV_RGB(255,	0,	0));
	SetWidgetJustification(	kCameraBox_ReadModeOutline, kJustification_Left);
	yLoc			+=	4;

	//=======================================================
	//*	Live mode
	SetWidget(		kCameraBox_LiveMode,	cClm4_offset,	yLocSave,		cWidth/4,		cRadioBtnHt	);
	SetWidgetType(	kCameraBox_LiveMode,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_LiveMode,	"LIVE");
	SetWidgetFont(	kCameraBox_LiveMode,	kFont_Medium);
	yLocSave	+=	cRadioBtnHt;
	yLocSave	+=	2;

	//=======================================================
	//*	Live mode
	SetWidget(		kCameraBox_SideBar,	cClm4_offset,	yLocSave,		cWidth/4,		cRadioBtnHt	);
	SetWidgetType(	kCameraBox_SideBar,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_SideBar,	"SideBar");
	SetWidgetFont(	kCameraBox_SideBar,	kFont_Medium);
	yLocSave	+=	cRadioBtnHt;
	yLocSave	+=	2;


	//=======================================================
	//*	Auto exposure
	SetWidget(		kCameraBox_AutoExposure,	cClm4_offset,	yLocSave,	cWidth/4,		cRadioBtnHt	);
	SetWidgetType(	kCameraBox_AutoExposure,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_AutoExposure,	"AUTO EXP");
	SetWidgetFont(	kCameraBox_AutoExposure,	kFont_Medium);
	yLocSave	+=	cRadioBtnHt;
	yLocSave	+=	2;

	//=======================================================
	//*	Display Image
	SetWidget(		kCameraBox_DisplayImage,	cClm4_offset,	yLocSave,	cWidth/4,		cRadioBtnHt	);
	SetWidgetType(	kCameraBox_DisplayImage,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_DisplayImage,	"Display Image");
	SetWidgetFont(	kCameraBox_DisplayImage,	kFont_Medium);

	yLocSave	+=	cRadioBtnHt;
	yLocSave	+=	2;
	//=======================================================
	//*	Save All Images
	SetWidget(		kCameraBox_SaveAll,	cClm4_offset,	yLocSave,	cWidth/4,		cRadioBtnHt	);
	SetWidgetType(	kCameraBox_SaveAll,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_SaveAll,	"Save All");
	SetWidgetFont(	kCameraBox_SaveAll,	kFont_Medium);
	yLocSave	+=	cRadioBtnHt;
	yLocSave	+=	2;

	//=======================================================
	SetWidget(			kCameraBox_Reset,			cClm6_offset,	yLocRest,	cLrgBtnWidth,	cLrgBtnHeight);
	SetWidgetType(		kCameraBox_Reset,			kWidgetType_Button);
	SetWidgetText(		kCameraBox_Reset,			"Reset");
	SetWidgetFont(		kCameraBox_Reset,			kFont_Medium);
	SetWidgetBGColor(	kCameraBox_Reset,			CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kCameraBox_Reset,			CV_RGB(255,	0,	0));
	yLocRest	+=	cLrgBtnHeight;
	yLocRest	+=	2;

	//=======================================================
	//*	Flip stuff
	SetWidget(			kCameraBox_FlipText,		cClm6_offset,	yLocRest,	40,	cRadioBtnHt);
	SetWidgetType(		kCameraBox_FlipText,		kWidgetType_TextBox);
	SetWidgetFont(		kCameraBox_FlipText,		kFont_RadioBtn);
	SetWidgetText(		kCameraBox_FlipText,		"Flip");

	SetWidget(			kCameraBox_FlipValue,		cClm6_offset + 42,	yLocRest,	40,	cRadioBtnHt);
	SetWidgetType(		kCameraBox_FlipValue,		kWidgetType_TextBox);
	SetWidgetFont(		kCameraBox_FlipValue,		kFont_RadioBtn);
	SetWidgetNumber(	kCameraBox_FlipValue,		0);
	SetWidgetJustification(	kCameraBox_FlipValue,	kJustification_Center);
	yLocRest	+=	cRadioBtnHt;
	yLocRest	+=	2;

	SetWidget(		kCameraBox_FlipHorzCheckBox,	cClm6_offset,	yLocRest,	cLrgBtnWidth,	cRadioBtnHt);
	SetWidgetType(	kCameraBox_FlipHorzCheckBox,	kWidgetType_CheckBox);
	SetWidgetFont(	kCameraBox_FlipHorzCheckBox,	kFont_RadioBtn);
	SetWidgetText(	kCameraBox_FlipHorzCheckBox,	"Flip Hor");

	yLocRest	+=	cRadioBtnHt;
	yLocRest	+=	2;

	SetWidget(		kCameraBox_FlipVertCheckBox,	cClm6_offset,	yLocRest,	cLrgBtnWidth,	cRadioBtnHt);
	SetWidgetType(	kCameraBox_FlipVertCheckBox,	kWidgetType_CheckBox);
	SetWidgetFont(	kCameraBox_FlipVertCheckBox,	kFont_RadioBtn);
	SetWidgetText(	kCameraBox_FlipVertCheckBox,	"Flip Ver");


	yLoc			+=	10;
	//=======================================================
	//*	Filename
	SetWidget(		kCameraBox_Filename,	cClm1_offset,	yLoc,		cFullWidthBtn,		cRadioBtnHt	);
	SetWidgetFont(	kCameraBox_Filename,	kFont_Medium);
	SetWidgetText(	kCameraBox_Filename,	"-----");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	//*	Error messages
	SetWidget(			kCameraBox_ErrorMsg,	cClm1_offset,	yLoc,	cFullWidthBtn,		cRadioBtnHt * 2	);
	SetWidgetType(		kCameraBox_ErrorMsg, 	kWidgetType_MultiLineText);
	SetWidgetFont(		kCameraBox_ErrorMsg,	kFont_Medium);
	SetWidgetText(		kCameraBox_ErrorMsg,	"-----");
	SetWidgetTextColor(	kCameraBox_ErrorMsg,	CV_RGB(255,	0,	0));
	yLoc			+=	cRadioBtnHt * 2;
	yLoc			+=	2;


//	kCameraBox_StartExposure,
//	kCameraBox_Rank2,
//	kCameraBox_Rank3,
//
//	kCameraBox_DownloadImage,
//	kCameraBox_Btn_8Bit,
//	kCameraBox_DownloadRGBarray,
//
//	kCameraBox_SaveOutline,
//


	//=======================================================
	//*	start exposure button
	yLocExposure	=	yLoc;
	SetWidget(			kCameraBox_StartExposure,	cClm1_offset,	yLoc,	((cClmWidth * 2) -4),	cLrgBtnHeight);
	SetWidgetType(		kCameraBox_StartExposure,	kWidgetType_Button);
	SetWidgetText(		kCameraBox_StartExposure,	"Start Exposure");
	SetWidgetFont(		kCameraBox_StartExposure,	kFont_Medium);
	SetWidgetBGColor(	kCameraBox_StartExposure,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kCameraBox_StartExposure,	CV_RGB(255,	0,	0));



	SetWidget(			kCameraBox_DownloadImage,	cClm5_offset,	yLoc,	((cClmWidth * 2) -4),	cLrgBtnHeight);
	SetWidgetType(		kCameraBox_DownloadImage,	kWidgetType_Button);
	SetWidgetText(		kCameraBox_DownloadImage,	"Download Image");
	SetWidgetFont(		kCameraBox_DownloadImage,	kFont_Medium);
	SetWidgetBGColor(	kCameraBox_DownloadImage,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kCameraBox_DownloadImage,	CV_RGB(255,	0,	0));

	yLoc			+=	cLrgBtnHeight;
	yLoc			+=	4;

	SetWidget(			kCameraBox_StopExposure,	cClm1_offset,	yLoc,	((cClmWidth * 2) -4),	cLrgBtnHeight);
	SetWidgetType(		kCameraBox_StopExposure,	kWidgetType_Button);
	SetWidgetText(		kCameraBox_StopExposure,	"Stop Exposure");
	SetWidgetFont(		kCameraBox_StopExposure,	kFont_Medium);
	SetWidgetBGColor(	kCameraBox_StopExposure,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kCameraBox_StopExposure,	CV_RGB(255,	0,	0));


	SetWidget(			kCameraBox_DownloadRGBarray,	cClm5_offset,	yLoc,	((cClmWidth * 2) -4),	cLrgBtnHeight);
	SetWidgetType(		kCameraBox_DownloadRGBarray,	kWidgetType_Button);
	SetWidgetText(		kCameraBox_DownloadRGBarray,	"DL RGBarray");
	SetWidgetFont(		kCameraBox_DownloadRGBarray,	kFont_Medium);
	SetWidgetBGColor(	kCameraBox_DownloadRGBarray,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kCameraBox_DownloadRGBarray,	CV_RGB(255,	0,	0));

	yLoc			+=	cLrgBtnHeight;
	yLoc			+=	4;
	yLocSave		=	yLoc;
	//=======================================================
	yLoc			=	yLocExposure;
	SetWidget(			kCameraBox_Rank2,	cClm3_offset,	yLoc,	((cClmWidth) -4),	cRadioBtnHt);
	SetWidgetType(		kCameraBox_Rank2,	kWidgetType_RadioButton);
	SetWidgetFont(		kCameraBox_Rank2,	kFont_RadioBtn);
	SetWidgetText(		kCameraBox_Rank2,	"R2");

	SetWidget(			kCameraBox_Rank3,	cClm4_offset,	yLoc,	((cClmWidth) -4),	cRadioBtnHt);
	SetWidgetType(		kCameraBox_Rank3,	kWidgetType_RadioButton);
	SetWidgetFont(		kCameraBox_Rank3,	kFont_RadioBtn);
	SetWidgetText(		kCameraBox_Rank3,	"R3");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	4;

	SetWidget(			kCameraBox_EnableBinary,	cClm3_offset,	yLoc,	((cClmWidth * 2) -4),	cRadioBtnHt);
	SetWidgetType(		kCameraBox_EnableBinary,	kWidgetType_CheckBox);
	SetWidgetFont(		kCameraBox_EnableBinary,	kFont_RadioBtn);
	SetWidgetText(		kCameraBox_EnableBinary,	"Allow Binary");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	4;


	//=======================================================
	SetWidget(			kCameraBox_Btn_8Bit,	cClm3_offset,	yLoc,	((cClmWidth * 2) -4),	cRadioBtnHt);
	SetWidgetType(		kCameraBox_Btn_8Bit,	kWidgetType_CheckBox);
	SetWidgetText(		kCameraBox_Btn_8Bit,	"Force 8 Bit");
	SetWidgetFont(		kCameraBox_Btn_8Bit,	kFont_RadioBtn);
//	SetWidgetBGColor(	kCameraBox_Btn_8Bit,	CV_RGB(255,	255,	255));
//	SetWidgetTextColor(	kCameraBox_Btn_8Bit,	CV_RGB(255,	0,	0));

	SetWidgetOutlineBox(	kCameraBox_SaveOutline,
							kCameraBox_StartExposure,
							(kCameraBox_SaveOutline - 1));

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	4;

	yLoc			=	yLocSave;
	yLocClm4		=	yLoc;
	//=======================================================
	//*	Filter wheel (if present)
	if (cHasFilterWheel)
	{
	int	slotNum;
	int	filterWhlWidth;
	int		fnWidgetNum;	//*	filter name
	int		foWidgetNum;	//*	filter offset

//		CONSOLE_DEBUG("Setting up filterwheel!!!!!!!!!!!!!!!!!!!!!!!");

		filterWhlWidth	=	(cClmWidth * 3) - 6;
		SetWidget(		kCameraBox_FilterWheelName,	cClm1_offset,	yLoc,	filterWhlWidth,		cRadioBtnHt	);
		SetWidgetFont(	kCameraBox_FilterWheelName,	kFont_RadioBtn);
		SetWidgetText(	kCameraBox_FilterWheelName,	"Filter wheel");
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	1;

		slotNum	=	1;
//		for (iii=kCameraBox_FilterWheel1; iii<=kCameraBox_FilterWheel8; iii++)
		for (iii=0; iii<8; iii++)
		{
			fnWidgetNum	=	kCameraBox_FilterWheel1 + iii;
			foWidgetNum	=	kCameraBox_FilterOffset1 + iii;
			SetWidget(		fnWidgetNum,	2,				yLoc,	cWidth/4,		cRadioBtnHt	);
			SetWidgetFont(	fnWidgetNum,	kFont_RadioBtn);
			SetWidgetType(	fnWidgetNum,	kWidgetType_RadioButton);
			SetWidgetTextColor(fnWidgetNum, CV_RGB(255,	255,	255));
			sprintf(textBuff, "Slot #%d", slotNum);
			SetWidgetText(	fnWidgetNum,	textBuff);
			slotNum++;

			SetWidget(		foWidgetNum,	cClm3_offset,	yLoc,	cClmWidth,		cRadioBtnHt	);
			SetWidgetFont(	foWidgetNum,	kFont_RadioBtn);
			SetWidgetTextColor(foWidgetNum, CV_RGB(255,	255,	255));
			SetWidgetNumber(foWidgetNum, 0);


			yLoc			+=	cRadioBtnHt;
			yLoc			+=	1;
		}
		SetWidgetOutlineBox(	kCameraBox_FilterWheelOutline,
								kCameraBox_FilterWheelName,
								kCameraBox_FilterWheel8);
	}
	yLoc			+=	4;
	yLoc			+=	4;

	//=======================================================
	//*	Temperature
	SetWidget(		kCameraBox_Temp_Label,	cClm4_offset,	yLocClm4,		cClmWidth,		cRadioBtnHt	);
	SetWidget(		kCameraBox_Temperature,	cClm5_offset,	yLocClm4,		cClmWidth * 2,	cRadioBtnHt	);
	SetWidgetFont(	kCameraBox_Temp_Label,	kFont_Small);
	SetWidgetFont(	kCameraBox_Temperature,	kFont_Small);
	SetWidgetText(	kCameraBox_Temp_Label,	"Temp");
	yLocClm4	+=	cRadioBtnHt;
	yLocClm4	+=	2;
	//*	Cooler check box
	SetWidget(		kCameraBox_CoolerChkBox,	cClm4_offset + 2,	yLocClm4,	cClmWidth,	cRadioBtnHt	);
	SetWidgetType(	kCameraBox_CoolerChkBox,	kWidgetType_CheckBox);
	SetWidgetText(	kCameraBox_CoolerChkBox,	"Camera Cooler");
	SetWidgetFont(	kCameraBox_CoolerChkBox,	kFont_Medium);
	yLocClm4	+=	cRadioBtnHt;
	yLocClm4	+=	2;

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	if (yLocClm4 > yLoc)
	{
		yLoc	=	yLocClm4;
	}


#ifdef _TEMP_GRAPH_
	SetWidget(		kCameraBox_TempGraph,	cClm1_offset+ 3,	yLoc,		cWidth - 8,		75	);
	SetWidgetType(	kCameraBox_TempGraph,	kWidgetType_Graph);

	SetWidgetOutlineBox(kCameraBox_TempOutline, kCameraBox_Temp_Label, kCameraBox_TempGraph);

	yLoc			+=	100;
	yLoc			+=	4;
#endif // _TEMP_GRAPH_


#ifdef _SUPPORT_REMOTE_IMU_
int imuClmWidth;
int	imuHeight;

	//=======================================================
	xLoc		=	cClm4_offset + 2;
	yLoc		+=	5;
	imuClmWidth	=	cClmWidth * 2;
	imuHeight	=	25;
	iii		=	kCameraBox_IMU_Title;
	while (iii < kCameraBox_IMU_Outline)
	{
		SetWidget(					iii,	xLoc,	yLoc,	imuClmWidth,	imuHeight);
		SetWidgetType(				iii,	kWidgetType_TextBox);
		SetWidgetFont(				iii,	kFont_TextList);
		SetWidgetJustification  (	iii,	kJustification_Left);
		iii++;

		yLoc			+=	imuHeight;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(kCameraBox_IMU_Outline, kCameraBox_IMU_Title, (kCameraBox_IMU_Outline -1));
	SetWidgetFont(				kCameraBox_IMU_Title,	kFont_Medium);
	SetWidgetJustification  (	kCameraBox_IMU_Title,	kJustification_Center);
	SetWidgetText(				kCameraBox_IMU_Title,	"IMU");
	SetWidgetBGColor(			kCameraBox_IMU_Title,	CV_RGB(0xff, 0xff, 0xff));
	SetWidgetTextColor(			kCameraBox_IMU_Title,	CV_RGB(0x00, 0x00, 0x00));

	SetWidgetText(				kCameraBox_IMU_Heading,	"Heading");
	SetWidgetText(				kCameraBox_IMU_Roll,	"Roll");
	SetWidgetText(				kCameraBox_IMU_Pitch,	"Pitch");

	//*	now turn all the IMU stuff off unless we actually see an IMU
	SetRemoteIMUdisplay(false);
#endif	//	_SUPPORT_REMOTE_IMU_

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kCameraBox_IPaddr,
							kCameraBox_Readall,
							kCameraBox_AlpacaErrorMsg,
							kCameraBox_LastCmdString,
							kCameraBox_AlpacaLogo,
							-1);
}

//**************************************************************************************
void	WindowTabCamera::DisableFilterWheel(void)
{
int		iii;

	for (iii=kCameraBox_FilterWheelName; iii<=kCameraBox_FilterWheelOutline; iii++)
	{
		SetWidgetValid(iii, false);
	}
}


#define	kCameraLogoCount	8

#ifdef _USE_OPENCV_CPP_
	cv::Mat		gCameraLogoImage[kCameraLogoCount];
#endif


//**************************************************************************************
void	WindowTabCamera::SetCameraLogo(void)
{
char	logoImagePath[32];
int		camLogoIdx;
#ifdef _USE_OPENCV_CPP_
	cv::Mat		*logoImagePtr;
#else
	IplImage	*logoImagePtr;
#endif // _USE_OPENCV_CPP_

	CONSOLE_DEBUG_W_STR("cAlpacaDeviceName=", cAlpacaDeviceName);
	camLogoIdx		=	-1;
	logoImagePtr	=	NULL;

	strcpy(logoImagePath, "logos/");
	if (strcasestr(cAlpacaDeviceName, "ZWO") != NULL)
	{
		strcat(logoImagePath, "zwo-logo.png");
		camLogoIdx	=	0;
	}
	else if (strcasestr(cAlpacaDeviceName, "Atik") != NULL)
	{
		strcat(logoImagePath, "atik-logo.png");
		camLogoIdx	=	1;
	}
	else if ((strcasestr(cAlpacaDeviceName, "toup") != NULL) || (strcasestr(cAlpacaDeviceName, "GCMOS") != NULL))
	{
		strcat(logoImagePath, "touptek-logo.png");
		camLogoIdx	=	2;
	}
	else if (strcasestr(cAlpacaDeviceName, "QHY") != NULL)
	{
		strcat(logoImagePath, "qhy-logo.png");
		camLogoIdx	=	3;
	}
	else if (strcasestr(cAlpacaDeviceName, "QSI") != NULL)
	{
		strcat(logoImagePath, "qsi-logo.png");
		camLogoIdx	=	4;
	}
	else if (strcasestr(cAlpacaDeviceName, "FLIR") != NULL)
	{
		strcat(logoImagePath, "flir-logo.png");
		camLogoIdx	=	5;
	}
	else if (strcasestr(cAlpacaDeviceName, "Phase") != NULL)
	{
		strcat(logoImagePath, "PhaseOne.png");
		camLogoIdx	=	6;
	}


	CONSOLE_DEBUG_W_NUM("camLogoIdx   \t=",	camLogoIdx);
	CONSOLE_DEBUG_W_STR("logoImagePath\t=",	logoImagePath);

	if ((camLogoIdx >= 0) && (camLogoIdx < kCameraLogoCount))
	{
#ifdef _USE_OPENCV_CPP_
		gCameraLogoImage[camLogoIdx]	=	cv::imread(logoImagePath);
		logoImagePtr					=	&gCameraLogoImage[camLogoIdx];
#else
		CONSOLE_DEBUG("Not using _USE_OPENCV_CPP_");
	#if (CV_MAJOR_VERSION <= 3)
		CONSOLE_DEBUG("CV_MAJOR_VERSION <= 3)");
		logoImagePtr	=	cvLoadImage(logoImagePath,		CV_LOAD_IMAGE_COLOR);
	#endif
#endif // _USE_OPENCV_CPP_
	}

//	CONSOLE_DEBUG_W_HEX("Calling SetWidgetImage() with logoImagePtr=", logoImagePtr);
	if (logoImagePtr != NULL)
	{
		SetWidgetImage(kCameraBox_Logo, logoImagePtr);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//**************************************************************************************
void	WindowTabCamera::SetTempartueDisplayEnable(bool enabled)
{

	SetWidgetValid(kCameraBox_Temp_Label,	enabled);
	SetWidgetValid(kCameraBox_Temperature,	enabled);
	SetWidgetValid(kCameraBox_CoolerChkBox,	enabled);
#ifdef _TEMP_GRAPH_
	SetWidgetValid(kCameraBox_TempGraph,	enabled);
	SetWidgetValid(kCameraBox_TempOutline,	enabled);
#endif // _TEMP_GRAPH_
}

//*****************************************************************************
void	WindowTabCamera::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool				validData;
char				dataString[64];
int					fwPosition;
bool				weHadToWait;
ControllerCamera	*myCameraController;
//uint32_t			currentMillis;
uint32_t			startMillis;
uint32_t			deltaMilliSecs;
int					loopCntr;

//	CONSOLE_DEBUG(__FUNCTION__);


	myCameraController	=	(ControllerCamera *)cParentObjPtr;
	weHadToWait			=	false;
	loopCntr			=	0;
	deltaMilliSecs		=	0;
	startMillis			=	millis();
	while ((myCameraController->cBackgroundTaskActive) && (deltaMilliSecs < 5000))
	{
		weHadToWait	=	true;
		CONSOLE_DEBUG("Waiting for background task");
		usleep(2000);
		deltaMilliSecs	=	millis() - startMillis;
		loopCntr++;
	}
	if (weHadToWait)
	{
		CONSOLE_DEBUG("DONE Waiting!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		CONSOLE_DEBUG_W_NUM("loopCntr\t=", loopCntr);
	}

	SetWidgetText(kCameraBox_ErrorMsg, "");
	switch(buttonIdx)
	{
		case kCameraBox_Exposure_Up:
			BumpExposure(0.001);
			break;

		case kCameraBox_Exposure_Down:
			BumpExposure(-0.001);
			break;

		case kCameraBox_Gain_Up:
			BumpGain(1);
			break;

		case kCameraBox_Gain_Down:
			BumpGain(-1);
			break;

		case kCameraBox_Offset_Up:
			BumpOffset(1);
			break;

		case kCameraBox_Offset_Down:
			BumpOffset(-1);
			break;


		case kCameraBox_ReadMode0:
		case kCameraBox_ReadMode1:
		case kCameraBox_ReadMode2:
		case kCameraBox_ReadMode3:
		case kCameraBox_ReadMode4:
			sprintf(dataString, "ReadoutMode=%d", (buttonIdx - kCameraBox_ReadMode0));
			validData	=	AlpacaSendPutCmd(	"camera", "readoutmode",	dataString);
			ForceAlpacaUpdate();
			break;

		case kCameraBox_LiveMode:
			ToggleLiveMode();
			break;

		case kCameraBox_SideBar:
			ToggleSideBar();
			break;

		case kCameraBox_AutoExposure:
			ToggleAutoExposure();
			break;

		case kCameraBox_DisplayImage:
			ToggleDisplayImage();
			break;

		case kCameraBox_SaveAll:
			ToggleSaveAll();
			break;

		case kCameraBox_CoolerChkBox:
			ToggleCooler();
			break;

		case kCameraBox_Reset:
			validData	=	AlpacaSendPutCmd(	"camera", "abortexposure",	NULL);
			if (validData == false)
			{
				CONSOLE_DEBUG("abortexposure failed");
			}
			break;

		case kCameraBox_StartExposure:
			CONSOLE_DEBUG(__FUNCTION__);
			StartExposure();
			break;

		case kCameraBox_StopExposure:
//++			StopExposure();
			break;

		case kCameraBox_Btn_8Bit:
			cForce8BitRead	=	!cForce8BitRead;
			ForceAlpacaUpdate();
			break;

		case kCameraBox_EnableBinary:
			cAllowBinaryDownload	=	!cAllowBinaryDownload;
			ForceAlpacaUpdate();
			break;

		case kCameraBox_DownloadImage:
			DownloadImage(false);	//*	false -> imageArray
			break;

		case kCameraBox_DownloadRGBarray:
			DownloadImage(true);	//*	true -> Use RGBarray
			break;



		case kCameraBox_FilterWheel1:
		case kCameraBox_FilterWheel2:
		case kCameraBox_FilterWheel3:
		case kCameraBox_FilterWheel4:
		case kCameraBox_FilterWheel5:
		case kCameraBox_FilterWheel6:
		case kCameraBox_FilterWheel7:
		case kCameraBox_FilterWheel8:
			//*	alpaca is a zero based index for the filter number
			//*	number is 0 to N-1
			fwPosition	=	buttonIdx - kCameraBox_FilterWheel1;
			sprintf(dataString, "Position=%d", fwPosition);
	//		CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
			validData	=	AlpacaSendPutCmd(	"filterwheel", "position",	dataString);
			if (validData)
			{
				ForceAlpacaUpdate();
			}
			else
			{
				CONSOLE_DEBUG("Error setting filter wheel position");
			}
			break;

		case kCameraBox_FlipHorzCheckBox:
			ToggleFlipMode(true, false);
			break;

		case kCameraBox_FlipVertCheckBox:
			ToggleFlipMode(false, true);
			break;

		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
			break;
	}
	DisplayLastAlpacaCommand();
}

//*****************************************************************************
void	WindowTabCamera::ProcessDoubleClick(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);


	switch(widgetIdx)
	{
		case kCameraBox_Title:
		case kCameraBox_Connected:
			AlpacaSetConnected("camera", true);
			break;

	}
}

//*****************************************************************************
void	WindowTabCamera::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
ControllerCamera	*myCameraController;
uint32_t			currentMillis;
uint32_t			deltaMilliSecs;
int					newSliderValue_int;


//	CONSOLE_DEBUG(__FUNCTION__);
	currentMillis	=	millis();

	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	switch(widgetIdx)
	{
		case kCameraBox_Exposure_Slider:
			deltaMilliSecs	=	currentMillis - cLastExposureUpdate_Millis;
			if (deltaMilliSecs > 4)
			{
				if (myCameraController != NULL)
				{
	//				CONSOLE_DEBUG_W_DBL("newSliderValue\t=", newSliderValue);
					myCameraController->SetExposure(newSliderValue);

				}
				cLastExposureUpdate_Millis	=	millis();
			}
			break;

		case kCameraBox_Gain_Slider:
			deltaMilliSecs	=	currentMillis - cLastGainUpdate_Millis;
			if (deltaMilliSecs > 4)
			{
				if (myCameraController != NULL)
				{
					newSliderValue_int	=	newSliderValue;
					myCameraController->SetGain(newSliderValue_int);
				}
				cLastGainUpdate_Millis	=	millis();
			}
			break;

		case kCameraBox_Offset_Slider:
			deltaMilliSecs	=	currentMillis - cLastOffsetUpdate_Millis;
			if (deltaMilliSecs > 4)
			{
				if (myCameraController != NULL)
				{
					newSliderValue_int	=	newSliderValue;
					myCameraController->SetOffset(newSliderValue_int);
				}
				cLastOffsetUpdate_Millis	=	millis();
			}
			break;
	}
	ForceAlpacaUpdate();
}


//*****************************************************************************
void	WindowTabCamera::ForceAlpacaUpdate(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetChecked(kCameraBox_Btn_8Bit,		cForce8BitRead);
	SetWidgetChecked(kCameraBox_EnableBinary,	cAllowBinaryDownload);

	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->cForceAlpacaUpdate	=	true;
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::BumpExposure(const double howMuch)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->BumpExposure(howMuch);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::BumpGain(const int howMuch)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->BumpGain(howMuch);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::BumpOffset(const int howMuch)
{
ControllerCamera	*myCameraController;

	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->BumpOffset(howMuch);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::ToggleLiveMode(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleLiveMode();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::ToggleSideBar(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleSideBar();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}



//*****************************************************************************
void	WindowTabCamera::ToggleAutoExposure(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleAutoExposure();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::ToggleDisplayImage(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleDisplayImage();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}


//*****************************************************************************
void	WindowTabCamera::ToggleSaveAll(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleSaveAll();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}


//*****************************************************************************
void	WindowTabCamera::ToggleCooler(void)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleCooler();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::ToggleFlipMode(bool toggleHorz, bool toggleVert)
{
ControllerCamera	*myCameraController;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->ToggleFlipMode(toggleHorz, toggleVert);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::StartExposure(void)
{
ControllerCamera	*myCameraController;

	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		SetWidgetText(kCameraBox_State,			"-----");
		ForceAlpacaUpdate();

		myCameraController->StartExposure();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//*****************************************************************************
void	WindowTabCamera::DownloadImage(const bool useRGBarray)
{
ControllerCamera	*myCameraController;
#ifdef _USE_OPENCV_CPP_
	cv::Mat			*myDownLoadedImage;
#else
	IplImage		*myDownLoadedImage;
	int				quality[3] = {16, 200, 0};
	int				openCVerr;
#endif
char				textBuf[128];
double				download_MBytes;
double				download_MB_per_sec;
double				download_seconds;
char				filePath[256];
//char				fileName[256];

	CONSOLE_DEBUG(__FUNCTION__);


	myDownLoadedImage	=	NULL;
	myCameraController	=	(ControllerCamera *)cParentObjPtr;
	if (myCameraController != NULL)
	{
//		CONSOLE_DEBUG("Starting download");
		myDownLoadedImage	=	NULL;
		if (useRGBarray)
		{
			myDownLoadedImage	=	myCameraController->DownloadImage_rgbarray();
		}
		else
		{
			myDownLoadedImage	=	myCameraController->DownloadImage(cForce8BitRead, cAllowBinaryDownload);
		}

		if (myDownLoadedImage != NULL)
		{
			CONSOLE_DEBUG("Download complete");
#ifdef _USE_OPENCV_CPP_
			CONSOLE_DEBUG_W_NUM("myDownLoadedImage->cols\t=",	myDownLoadedImage->cols);
			CONSOLE_DEBUG_W_NUM("myDownLoadedImage->rows\t=",	myDownLoadedImage->rows);
#else
			CONSOLE_DEBUG_W_NUM("myDownLoadedImage->width\t=",	myDownLoadedImage->width);
			CONSOLE_DEBUG_W_NUM("myDownLoadedImage->height\t=",	myDownLoadedImage->height);
#endif // _USE_OPENCV_CPP_
			//========================================================
			//*	save the image
			strcpy(filePath, gDownloadFilePath);
			strcat(filePath, "/");
			strcat(filePath, cDownLoadedFileNameRoot);
			strcat(filePath, ".jpg");

			CONSOLE_DEBUG_W_STR("Saving image as", filePath);
#ifdef _USE_OPENCV_CPP_
			try
			{
				cv::imwrite(filePath, *myDownLoadedImage);
			}
			catch (cv::Exception& ex)
			{
				CONSOLE_DEBUG_W_STR("Exception writing jpg image", ex.what());
			}

			//========================================================
			//*	save it out as PNG as well
			strcpy(filePath, gDownloadFilePath);
			strcat(filePath, "/");
			strcat(filePath, cDownLoadedFileNameRoot);
			strcat(filePath, ".png");
			try
			{
				cv::imwrite(filePath, *myDownLoadedImage);
			}
			catch (cv::Exception& ex)
			{
				CONSOLE_DEBUG_W_STR("Exception writing jpg image", ex.what());
			}

#else

		#if (CV_MAJOR_VERSION <= 3)
			openCVerr	=	cvSaveImage(filePath, myDownLoadedImage, quality);
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
		#endif
#endif // _USE_OPENCV_CPP_


			download_MBytes		=	1.0 * myCameraController->cLastDownload_Bytes / (1024.0 * 1024.0);
			download_seconds	=	1.0 * myCameraController->cLastDownload_Millisecs / 1000.0;
			download_MB_per_sec	=	download_MBytes / download_seconds;

			sprintf(textBuf,	"%2.2f mbytes in %2.2f sec =%2.2f mbytes/sec",
								download_MBytes,
								download_seconds,
								download_MB_per_sec
								);
			SetWidgetType(kCameraBox_ErrorMsg, kWidgetType_TextBox);
			SetWidgetText(kCameraBox_ErrorMsg, textBuf);

#ifdef _ENABLE_CTRL_IMAGE_
ControllerCamera	*parentCameraController;
ControllerImage		*imageWindowController;
			CONSOLE_DEBUG("Creating ControllerImage()")
			//*	this will open a new window with the image displayed
			parentCameraController	=	(ControllerCamera *)cParentObjPtr;
			imageWindowController	=	new ControllerImage(	cDownLoadedFileNameRoot,
											myDownLoadedImage,
											&parentCameraController->cBinaryImageHdr);
			if (imageWindowController != NULL)
			{
				//*	set the image download data
				imageWindowController->SetDownloadInfo(download_MBytes, download_seconds);
			}
#else
			cvReleaseImage(&myDownLoadedImage);
#endif // _ENABLE_CTRL_IMAGE_
		}
		else
		{
			SetWidgetType(kCameraBox_ErrorMsg, kWidgetType_TextBox);
			SetWidgetText(kCameraBox_ErrorMsg, "Failed to download image, no image exists");
			CONSOLE_DEBUG("Failed to download image");
		}
	}
}

//*****************************************************************************
void	WindowTabCamera::SetReceivedFileName(const char *newFileName)
{
	strcpy(cDownLoadedFileNameRoot, newFileName);
}

#ifdef _SUPPORT_REMOTE_IMU_
//**************************************************************************************
void	WindowTabCamera::SetRemoteIMUdisplay(const bool enableFlag)
{
int		iii;

	for (iii=kCameraBox_IMU_Title; iii<=kCameraBox_IMU_Outline; iii++)
	{
		SetWidgetValid(iii, enableFlag);
	}
}
#endif // _SUPPORT_REMOTE_IMU_


#endif // _ENABLE_CTRL_CAMERA_
