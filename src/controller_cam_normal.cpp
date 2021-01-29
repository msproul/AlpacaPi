//*****************************************************************************
//*		controller_cam_normal.cpp		(c) 2020 by Mark Sproul
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
//*	Jun 24,	2020	<MLS> Made decision to switch camera to have sub classes
//*	Jun 24,	2020	<MLS> Created controller_cam_normal.cpp
//*	Jun 25,	2020	<MLS> Cam_normal subclass now back to same functionality as before
//*	Dec 27,	2020	<MLS> Added UpdateDownloadProgress()
//*****************************************************************************
//*	todo
//*		control key for different step size.
//*		work on fits view to handle color fits images
//*		add error list window
//*		save cross hair location
//*		finish exposure step options
//*		clear error msg
//*****************************************************************************

#ifdef _ENABLE_CTRL_CAMERA_



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


#define	kCamWindowWidth		456
#define	kCamWindowHeight	800


#include	"windowtab.h"
#include	"windowtab_camera.h"
#include	"windowtab_camgraph.h"
#include	"windowtab_filelist.h"
#include	"windowtab_camsettings.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_camera.h"
#include	"controller_cam_normal.h"


//**************************************************************************************
enum
{
	kTab_Camera	=	1,
	kTab_Settings,
//	kTab_Advanced,
//	kTab_Graphs,
	kTab_FileList,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerCamNormal::ControllerCamNormal(	const char			*argWindowName,
											TYPE_REMOTE_DEV		*alpacaDevice)
	:ControllerCamera(argWindowName, alpacaDevice, kCamWindowWidth,  kCamWindowHeight)
{

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);


	SetupWindowControls();

}




//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerCamNormal::~ControllerCamNormal(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cCameraProp.CoolerOn)
	{
		//*	the ATIK cameras need to have cooling turned off gracefully
		//*	so, just as a last minute thing, turn it off.
		CONSOLE_DEBUG_W_STR("turning coolor off", cWindowName);
		ToggleCooler();
	}
	//*	delete the windowtab objects
	if (cCameraTabObjPtr != NULL)
	{
		delete cCameraTabObjPtr;
		cCameraTabObjPtr	=	NULL;
	}
	if (cCamSettingsTabObjPtr != NULL)
	{
		delete cCamSettingsTabObjPtr;
		cCamSettingsTabObjPtr	=	NULL;
	}
	if (cFileListTabObjPtr != NULL)
	{
		delete cFileListTabObjPtr;
		cFileListTabObjPtr	=	NULL;
	}
	if (cAboutBoxTabObjPtr != NULL)
	{
		delete cAboutBoxTabObjPtr;
		cAboutBoxTabObjPtr	=	NULL;
	}
}


//**************************************************************************************
void	ControllerCamNormal::SetupWindowControls(void)
{
char	lineBuff[64];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Camera,			"Camera");
	SetTabText(kTab_Settings,		"Settings");
//	SetTabText(kTab_Advanced,		"Adv");
//	SetTabText(kTab_Graphs,			"Graphs");
	SetTabText(kTab_FileList,		"File List");
	SetTabText(kTab_About,			"About");

	cCameraTabObjPtr		=	new WindowTabCamera(cWidth,
													cHeight,
													cBackGrndColor,
													cWindowName,
													cAlpacaDeviceNameStr,
													cHasFilterWheel);

	cCamSettingsTabObjPtr	=	new WindowTabCamSettings(cWidth,
													cHeight,
													cBackGrndColor,
													cWindowName,
													cAlpacaDeviceNameStr,
													cHasFilterWheel);

	cFileListTabObjPtr		=	new WindowTabFileList(	cWidth, cHeight, cBackGrndColor, cWindowName);

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);

	SetTabWindow(kTab_Camera,	cCameraTabObjPtr);
	SetTabWindow(kTab_Settings,	cCamSettingsTabObjPtr);
	SetTabWindow(kTab_FileList,	cFileListTabObjPtr);
	SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);


	if (cCameraTabObjPtr != NULL)
	{
		cCameraTabObjPtr->SetParentObjectPtr(this);
	}

	if (cFileListTabObjPtr != NULL)
	{
		cFileListTabObjPtr->SetParentObjectPtr(this);
	}

	if (cCamSettingsTabObjPtr != NULL)
	{
		cCamSettingsTabObjPtr->SetParentObjectPtr(this);
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

#if 0
//**************************************************************************************
void	ControllerCamNormal::DrawWidgetCustom(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvPoint		textLoc2;
CvSize		textSize;
int			baseLine;
int			lineSpacing;
int			curFontNum;
int			iii;
char		fileTypeStr[8];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (theWidget != NULL)
	{
		myCVrect.x		=	theWidget->left;
		myCVrect.y		=	theWidget->top;
		myCVrect.width	=	theWidget->width;
		myCVrect.height	=	theWidget->height;


		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->bgColor,			//	CvScalar color,
						CV_FILLED,					//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->boarderColor,	//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

		//*	draw the file list
//		curFontNum	=	kFont_Small,
		curFontNum	=	kFont_Medium,
		cvGetTextSize(	"foo",
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);
		lineSpacing	=	textSize.height + baseLine + 3;
		textLoc.x	=	theWidget->left + 10;
		textLoc.y	=	theWidget->top + lineSpacing;
		iii			=	0;
		while ((iii<kMaxRemoteFileCnt) && (textLoc.y < cHeight))
		{
			if (cRemoteFiles[iii].validData)
			{
				cvPutText(	cOpenCV_Image,
							cRemoteFiles[iii].filename,
							textLoc,
							&gTextFont[curFontNum],
							theWidget->textColor
							);

				strcpy(fileTypeStr, "       ");
				if (cRemoteFiles[iii].hasCSV)
				{
					fileTypeStr[0]	=	'C';
				}
				if (cRemoteFiles[iii].hasFTS)
				{
					fileTypeStr[1]	=	'F';
				}
				if (cRemoteFiles[iii].hasJPG)
				{
					fileTypeStr[2]	=	'J';
				}
				if (cRemoteFiles[iii].hasPNG)
				{
					fileTypeStr[3]	=	'P';
				}
				fileTypeStr[4]	=	0;
				textLoc2		=	textLoc;
				textLoc2.x		=	cWidth - 80;
				cvPutText(	cOpenCV_Image,
							fileTypeStr,
							textLoc2,
							&gTextFont[curFontNum],
							theWidget->textColor
							);

				textLoc.y	+=	lineSpacing;
			}
			iii++;
		}
	}
}

#endif // 0

#pragma mark -


//*****************************************************************************
void	ControllerCamNormal::UpdateSettings_Object(const char *filePrefix)
{

	if (cCamSettingsTabObjPtr != NULL)
	{
		cCamSettingsTabObjPtr->UpdateSettings_Object(filePrefix);
		cUpdateWindow	=	true;
	}
}

//*****************************************************************************
void	ControllerCamNormal::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_Camera, kCameraBox_ErrorMsg, errorMsgString);
}


//*****************************************************************************
void	ControllerCamNormal::UpdateSupportedActions(void)
{
	SetWidgetValid(kTab_Camera,		kCameraBox_Readall,		cHas_readall);
	SetWidgetValid(kTab_Settings,	kCamSet_Readall,		cHas_readall);
//	SetWidgetValid(kTab_Graphs,		kHistogram_Readall,		cHas_readall);
	SetWidgetValid(kTab_FileList,	kFileList_Readall,		cHas_readall);
	SetWidgetValid(kTab_About,		kAboutBox_Readall,		cHas_readall);

	if (cHas_readall == false)
	{
		SetWidgetCrossedout(kTab_Settings, kCamSet_ObjOutline,		true);
		SetWidgetCrossedout(kTab_Settings, kCamSet_FilenameOutline, true);

		SetWidgetText(kTab_Camera,		kCameraBox_AlpacaDrvrVersion,	"readall not supported");
		SetWidgetText(kTab_Settings,	kCamSet_AlpacaDrvrVersion,		"readall not supported");
	}
	SetWidgetValid(kTab_Camera,	kCameraBox_AutoExposure,		cHas_autoexposure);
	SetWidgetValid(kTab_Camera,	kCameraBox_DisplayImage,		cHas_displayimage);
	SetWidgetValid(kTab_Camera,	kCameraBox_LiveMode,			cHas_livemode);
	SetWidgetValid(kTab_Camera,	kCameraBox_SideBar,				cHas_sidebar);
	SetWidgetValid(kTab_Camera,	kCameraBox_DownloadRGBarray,	cHas_rgbarray);

}

//*****************************************************************************
void	ControllerCamNormal::UpdateRemoteAlpacaVersion(void)
{
	SetWidgetText(kTab_Camera,		kCameraBox_AlpacaDrvrVersion,	cAlpacaVersionString);
	SetWidgetText(kTab_Settings,	kCamSet_AlpacaDrvrVersion,		cAlpacaVersionString);
	SetWidgetText(kTab_FileList,	kFileList_AlpacaDrvrVersion,	cAlpacaVersionString);
	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,	cAlpacaVersionString);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateCameraName(void)
{
int		jjj;

	SetWidgetText(kTab_Camera,		kCameraBox_Title,	cCameraName);
	SetWidgetText(kTab_Settings,	kCamSet_Title,		cCameraName);
	SetWidgetText(kTab_FileList,	kFileList_Title,	cCameraName);
	//*	check for ATIK cameras, they do not support GAIN
	if (strcasestr(cCameraName, "ATIK") != NULL)
	{
		for (jjj=kCameraBox_Gain_Label; jjj<=kCameraBox_Gain_Down; jjj++)
		{
			SetWidgetValid(kTab_Camera, jjj, false);
		}
		SetWidgetType(	kTab_Camera, kCameraBox_Gain_Slider, kWidgetType_Text);
		SetWidgetText(	kTab_Camera, kCameraBox_Gain_Slider, "Gain not supported");
		SetWidgetValid(	kTab_Camera, kCameraBox_Gain_Slider, true);
	}
}

//*****************************************************************************
void	ControllerCamNormal::UpdateReadoutModes(void)
{
int		jjj;

	for (jjj=0; jjj<kMaxReadOutModes; jjj++)
	{
		if (strlen(cCameraProp.ReadOutModes[jjj].mode) > 0)
		{
			SetWidgetValid(	kTab_Camera, (kCameraBox_ReadMode0 + jjj), true);
			SetWidgetText(	kTab_Camera,
						(kCameraBox_ReadMode0 + jjj),
						cCameraProp.ReadOutModes[jjj].mode);
		}
	}
}

//*****************************************************************************
void	ControllerCamNormal::UpdateCurrReadoutMode(void)
{
	SetWidgetChecked(kTab_Camera, kCameraBox_ReadMode0, (cCameraProp.ReadOutMode == 0));
	SetWidgetChecked(kTab_Camera, kCameraBox_ReadMode1, (cCameraProp.ReadOutMode == 1));
	SetWidgetChecked(kTab_Camera, kCameraBox_ReadMode2, (cCameraProp.ReadOutMode == 2));
	SetWidgetChecked(kTab_Camera, kCameraBox_ReadMode3, (cCameraProp.ReadOutMode == 3));
	SetWidgetChecked(kTab_Camera, kCameraBox_ReadMode4, (cCameraProp.ReadOutMode == 4));
}

//*****************************************************************************
void	ControllerCamNormal::UpdateCameraGain(void)
{
	SetWidgetSliderLimits(	kTab_Camera, kCameraBox_Gain_Slider, cCameraProp.GainMin, cCameraProp.GainMax);
	SetWidgetSliderValue(	kTab_Camera, kCameraBox_Gain_Slider, cCameraProp.Gain);
	SetWidgetNumber(		kTab_Camera, kCameraBox_Gain, cCameraProp.Gain);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateCameraExposure(void)
{
	SetWidgetSliderLimits(	kTab_Camera, kCameraBox_Exposure_Slider,	cCameraProp.ExposureMin_seconds,
																		cCameraProp.ExposureMax_seconds);
	SetWidgetSliderValue(	kTab_Camera, kCameraBox_Exposure_Slider,	cExposure);
	SetWidgetNumber(		kTab_Camera, kCameraBox_Exposure,			cExposure);
}
//*****************************************************************************
void	ControllerCamNormal::UpdateCameraSize(void)
{
char	linebuff[64];

	sprintf(linebuff, "%d x %d", cCameraProp.CameraXsize, cCameraProp.CameraYsize);
	SetWidgetText(kTab_Camera, kCameraBox_Size, linebuff);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateCameraState(void)
{
char			linebuff[64];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

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
	SetWidgetText(		kTab_Camera, kCameraBox_State,			linebuff);
	if (cCameraState_imageready)
	{
		//*	image is ready, highlight the buttons
		SetWidgetBGColor(kTab_Camera, kCameraBox_DownloadImage, CV_RGB(255, 255, 255));
		SetWidgetBGColor(kTab_Camera, kCameraBox_DownloadRGBarray, CV_RGB(255, 255, 255));
	}
	else
	{
		//*	not ready, grey the buttons
		SetWidgetBGColor(kTab_Camera, kCameraBox_DownloadImage, CV_RGB(100, 100, 100));
		SetWidgetBGColor(kTab_Camera, kCameraBox_DownloadRGBarray, CV_RGB(100, 100, 100));
	}
}


//*****************************************************************************
void	ControllerCamNormal::UpdateCameraTemperature(void)
{
char			linebuff[128];

	sprintf(linebuff, "%1.1f C / %1.1f F", cCameraProp.CCDtemperature,
											(cCameraProp.CCDtemperature * 9.0/5.0) +32.0);
	SetWidgetText(kTab_Camera, kCameraBox_Temperature, linebuff);

	if (cHasCCDtemp	== false)
	{
		if (cCameraTabObjPtr != NULL)
		{
			//*	disable the display of the temperature items
			cCameraTabObjPtr->SetTempartueDisplayEnable(false);
		}
	}
}
//*****************************************************************************
void	ControllerCamNormal::UpdateCoolerState(void)
{
	SetWidgetChecked(kTab_Camera, kCameraBox_CoolerChkBox, cCameraProp.CoolerOn);

}
//*****************************************************************************
void	ControllerCamNormal::UpdateFilterWheelInfo(void)
{
int		iii;
int		fwTabNumber;

	SetWidgetText(	kTab_Camera,	kCameraBox_FilterWheelName,	cFilterWheelName);
	for (iii=0; iii<kMaxFilters; iii++)
	{
		if (strlen(cFilterNames[iii].filterName) > 0)
		{
			//*	set the text in the window widget
			fwTabNumber	=	kCameraBox_FilterWheel1 + iii;
			SetWidgetValid(	kTab_Camera, fwTabNumber, true);
			SetWidgetText(	kTab_Camera,
							fwTabNumber,
							cFilterNames[iii].filterName);

			if (strncasecmp(cFilterNames[iii].filterName, "RED", 3) == 0)
			{
				SetWidgetTextColor(kTab_Camera, fwTabNumber, CV_RGB(255,	0,	0));
			}
			else if (strncasecmp(cFilterNames[iii].filterName, "GREEN", 5) == 0)
			{
				SetWidgetTextColor(kTab_Camera, fwTabNumber, CV_RGB(0,	255,	0));
			}
			else if (strncasecmp(cFilterNames[iii].filterName, "BLUE", 4) == 0)
			{
				SetWidgetTextColor(kTab_Camera, fwTabNumber, CV_RGB(0,	0,	255));
			}
		}
	}
}

//*****************************************************************************
void	ControllerCamNormal::UpdateFilterWheelPosition(void)
{
int		jjj;

	for (jjj=kCameraBox_FilterWheel1; jjj<=kCameraBox_FilterWheel8; jjj++)
	{
		SetWidgetChecked(kTab_Camera, jjj, false);
	}
	SetWidgetChecked(kTab_Camera, (kCameraBox_FilterWheel1 + cFilterWheelPosition), true);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateFileNameOptions(void)
{
	SetWidgetChecked(kTab_Settings,	kCamSet_FN_IncFilter,		cFN_includeFilter);
	SetWidgetChecked(kTab_Settings,	kCamSet_FN_IncCamera,		cFN_includeManuf);
	SetWidgetChecked(kTab_Settings,	kCamSet_FN_IncSerialNum,	cFN_includeSerialNum);
	SetWidgetChecked(kTab_Settings,	kCamSet_FN_IncRefID,		cFN_includeRefID);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateReceivedFileName(const char *newFileName)
{
	SetWidgetText(kTab_Camera, kCameraBox_Filename, newFileName);
	if (cCameraTabObjPtr != NULL)
	{
		cCameraTabObjPtr->SetReceivedFileName(newFileName);
	}
}

//*****************************************************************************
void	ControllerCamNormal::UpdateRemoteFileList(void)
{
int		iii;
int		jjj;

	//*	now update the displayed list in the file list window
	iii	=	0;
	jjj	=	kFileList_FistEntry;
	while ((jjj <= kFileList_LastEntry) && (iii < kMaxRemoteFileCnt))
	{
		CONSOLE_DEBUG_W_NUM("Widget Index\t\t=", jjj);
		CONSOLE_DEBUG_W_STR("cRemoteFiles[iii].filename\t=", cRemoteFiles[iii].filename);

		if (cRemoteFiles[iii].validData)
		{
			SetWidgetText(kTab_FileList, jjj, cRemoteFiles[iii].filename);
		}
		else
		{
			CONSOLE_DEBUG("INVALID");
		}
		iii++;
		jjj++;
	}
}

//*****************************************************************************
void	ControllerCamNormal::UpdateDisplayModes(void)
{
	SetWidgetChecked(kTab_Camera, kCameraBox_LiveMode,		cLiveMode);
	SetWidgetChecked(kTab_Camera, kCameraBox_SideBar,		cSideBar);
	SetWidgetChecked(kTab_Camera, kCameraBox_AutoExposure,	cAutoExposure);
	SetWidgetChecked(kTab_Camera, kCameraBox_DisplayImage,	cDisplayImage);
}

//*****************************************************************************
void	ControllerCamNormal::UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue)
{
	SetWidgetBGColor(kTab_Camera, kCameraBox_Title, CV_RGB(redValue,	grnValue,	bluValue));
	if ((redValue + grnValue + bluValue) < (128 * 3))
	{
		SetWidgetTextColor(kTab_Camera, kCameraBox_Title, CV_RGB(255,	255,	255));
	}
	else
	{
		SetWidgetTextColor(kTab_Camera, kCameraBox_Title, CV_RGB(0,	0,	0));
	}
}


//*****************************************************************************
void	ControllerCamNormal::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
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
	if (((newProgressValue - cPrevProgessValue) > 0.002) || (newProgressValue > 0.9985))
	{
		SetWidgetProgress(kTab_Camera, kCameraBox_ErrorMsg, unitsRead, totalUnits);
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


#endif // _ENABLE_CTRL_CAMERA_

