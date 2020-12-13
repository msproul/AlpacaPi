//*****************************************************************************
//*		controller_preview.cpp		(c) 2020 by Mark Sproul
//*
//*
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
//*	Jun 23,	2020	<MLS> Created controller_preview.cpp
//*	Jun 30,	2020	<MLS> Added SetRefID()
//*	Aug 11,	2020	<MLS> Added UpdateDisplayModes() to Preview window
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"alpaca_defs.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"windowtab.h"
#include	"windowtab_preview.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_preview.h"

#define	kPreviewWindowWidth		750
#define	kPreviewWindowHeight	800



//**************************************************************************************
enum
{
	kTab_Preview	=	0,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerPreview::ControllerPreview(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice)
	:ControllerCamera(argWindowName, alpacaDevice, kPreviewWindowWidth,  kPreviewWindowHeight)
{

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	cPrevProgessValue	=	0.0;
	cProgressUpdates	=	0;
	cProgressReDraws	=	0;

	SetupWindowControls();
	if (cPreviewTabObjPtr != NULL)
	{
		cPreviewTabObjPtr->SetDeviceInfo(alpacaDevice);
		cPreviewTabObjPtr->SetWindowTabColorScheme(gCurrWindowTabColorScheme);
	}

	if (cAboutBoxTabObjPtr != NULL)
	{
		cAboutBoxTabObjPtr->SetWindowTabColorScheme(gCurrWindowTabColorScheme);
	}
//	CONSOLE_DEBUG_W_STR("exit", cWindowName);
}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerPreview::~ControllerPreview(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cPreviewTabObjPtr != NULL)
	{
		delete cPreviewTabObjPtr;
		cPreviewTabObjPtr	=	NULL;
	}
	if (cAboutBoxTabObjPtr != NULL)
	{
		delete cAboutBoxTabObjPtr;
		cAboutBoxTabObjPtr	=	NULL;
	}
}


//**************************************************************************************
void	ControllerPreview::SetupWindowControls(void)
{
char	lineBuff[64];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Preview,		"Preview");
	SetTabText(kTab_About,			"About");

	cPreviewTabObjPtr		=	new WindowTabPreview(	cWidth, cHeight, cBackGrndColor, cWindowName);
	cAboutBoxTabObjPtr		=	new WindowTabAbout(		cWidth, cHeight, cBackGrndColor, cWindowName);

	SetTabWindow(kTab_Preview,	cPreviewTabObjPtr);
	SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);

	if (cPreviewTabObjPtr != NULL)
	{
		cPreviewTabObjPtr->SetParentObjectPtr(this);
	}
	if (cAboutBoxTabObjPtr != NULL)
	{
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//*****************************************************************************
void	ControllerPreview::UpdateReadAllStatus(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetValid(kTab_Preview,	kPreviewBox_Readall,	cHasReadAll);
	SetWidgetValid(kTab_About,		kAboutBox_Readall,		cHasReadAll);
}
//*****************************************************************************
void	ControllerPreview::UpdateRemoteAlpacaVersion(void)
{
	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,	cAlpacaVersionString);
}
//*****************************************************************************
void	ControllerPreview::UpdateCameraName(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(kTab_Preview,		kPreviewBox_Title,	cCameraName);
}
//*****************************************************************************
void	ControllerPreview::UpdateReadoutModes(void)
{
int		iii;
int		widgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<kMaxReadOutModes; iii++)
	{
//		CONSOLE_DEBUG(cReadOutModes[iii].mode);
		widgetIdx	=	kPreviewBox_ReadMode0 + iii;
		if (strlen(cReadOutModes[iii].mode) > 0)
		{
			SetWidgetValid(	kTab_Preview,	widgetIdx, true);
			SetWidgetText(	kTab_Preview,	widgetIdx, cReadOutModes[iii].mode);
		}
		else
		{
			SetWidgetValid(	kTab_Preview,	widgetIdx, false);
		}
	}
}

//*****************************************************************************
void	ControllerPreview::UpdateCurrReadoutMode(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetChecked(kTab_Preview, kPreviewBox_ReadMode0, (cReadOutMode == 0));
	SetWidgetChecked(kTab_Preview, kPreviewBox_ReadMode1, (cReadOutMode == 1));
	SetWidgetChecked(kTab_Preview, kPreviewBox_ReadMode2, (cReadOutMode == 2));
	SetWidgetChecked(kTab_Preview, kPreviewBox_ReadMode3, (cReadOutMode == 3));
	SetWidgetChecked(kTab_Preview, kPreviewBox_ReadMode4, (cReadOutMode == 4));
}


//*****************************************************************************
void	ControllerPreview::UpdateCameraGain(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetSliderLimits(	kTab_Preview, kPreviewBox_Gain_Slider, cGainMin, cGainMax);
	SetWidgetSliderValue(	kTab_Preview, kPreviewBox_Gain_Slider,	cGain);
	SetWidgetNumber(		kTab_Preview, kPreviewBox_Gain, cGain);
}
//*****************************************************************************
void	ControllerPreview::UpdateCameraExposure(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetSliderLimits(	kTab_Preview, kPreviewBox_Exposure_Slider,	cExposureMin, cExposureMax);
	SetWidgetSliderValue(	kTab_Preview, kPreviewBox_Exposure_Slider,	cExposure);
	SetWidgetNumber(		kTab_Preview, kPreviewBox_Exposure,			cExposure);
}
//*****************************************************************************
void	ControllerPreview::UpdateCameraSize(void)
{
char	textBuff[64];

	sprintf(textBuff, "%d x %d", cCameraSizeX, cCameraSizeY);
	SetWidgetText(kTab_Preview, kPreviewBox_Size, textBuff);
}
//*****************************************************************************
void	ControllerPreview::UpdateCameraState(void)
{
char			linebuff[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(cAlpacaCameraState)
	{
		case	kALPACA_CameraState_Idle:		strcpy(linebuff,	"Idle");		break;
		case	kALPACA_CameraState_Waiting:	strcpy(linebuff,	"Waiting");		break;
		case	kALPACA_CameraState_Exposing:	strcpy(linebuff,	"Exposing");	break;
		case	kALPACA_CameraState_Reading:	strcpy(linebuff,	"Reading");		break;
		case	kALPACA_CameraState_Download:	strcpy(linebuff,	"Download");	break;
		case	kALPACA_CameraState_Error:		strcpy(linebuff,	"Error");		break;

		default:								strcpy(linebuff,	"unknown");		break;
	}
	SetWidgetText(		kTab_Preview, kPreviewBox_State,	linebuff);
	if (cPreviewTabObjPtr != NULL)
	{
		cPreviewTabObjPtr->UpdateCameraState(cAlpacaCameraState);
	}
}

//*****************************************************************************
void	ControllerPreview::UpdateReceivedFileName(const char *newFileName)
{
	SetWidgetText(		kTab_Preview, kPreviewBox_FileName,	newFileName);
	if (cPreviewTabObjPtr != NULL)
	{
		cPreviewTabObjPtr->SetReceivedFileName(newFileName);
	}
}

//*****************************************************************************
void	ControllerPreview::UpdateDisplayModes(void)
{
//	SetWidgetChecked(kTab_Preview, kCameraBox_LiveMode,			cLiveMode);
//	SetWidgetChecked(kTab_Preview, kCameraBox_SideBar,			cSideBar);
	SetWidgetChecked(kTab_Preview, kPreviewBox_AutoExposure,	cAutoExposure);
//	SetWidgetChecked(kTab_Preview, kCameraBox_DisplayImage,		cDisplayImage);
}

//*****************************************************************************
void	ControllerPreview::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
{
double	newProgressValue;

	newProgressValue	=	1.0 * unitsRead / totalUnits;

	if (newProgressValue < cPrevProgessValue)
	{
		CONSOLE_DEBUG("Reset progress bar");
		cPrevProgessValue	=	0.0;
		cProgressUpdates	=	0;
		cProgressReDraws	=	0;
	}
	cProgressUpdates++;
	if (((newProgressValue - cPrevProgessValue) > 0.0015) || (newProgressValue > 0.9985))
	{
		SetWidgetProgress(kTab_Preview, kPreviewBox_ProgressBar, unitsRead, totalUnits);
		cUpdateWindow	=	true;
		cvWaitKey(1);

		cProgressReDraws++;

		cPrevProgessValue	=	newProgressValue;
	}

	if (newProgressValue > 0.9999)
	{
		CONSOLE_DEBUG_W_DBL("newProgressValue\t=", newProgressValue);
		CONSOLE_DEBUG_W_NUM("cProgressUpdates\t=", cProgressUpdates);
		CONSOLE_DEBUG_W_NUM("cProgressReDraws\t=", cProgressReDraws);
	}
}

//*****************************************************************************
void	ControllerPreview::UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue)
{
	SetWidgetBGColor(kTab_Preview, kPreviewBox_Title, CV_RGB(redValue,	grnValue,	bluValue));
	if ((redValue + grnValue + bluValue) < (128 * 3))
	{
		SetWidgetTextColor(kTab_Preview, kPreviewBox_Title, CV_RGB(255,	255,	255));
	}
	else
	{
		SetWidgetTextColor(kTab_Preview, kPreviewBox_Title, CV_RGB(0,	0,	0));
	}
}

//*****************************************************************************
void	ControllerPreview::SetRefID(const char *newRefID)
{
	SetWidgetText(kTab_Preview, kPreviewBox_RefID, newRefID);
}
