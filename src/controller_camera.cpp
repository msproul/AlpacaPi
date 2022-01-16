//*****************************************************************************
//*		controller_camera.cpp		(c) 2020 by Mark Sproul
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar  1,	2020	<MLS> Created controller_camera.cpp
//*	Mar  6,	2020	<MLS> Added filename display
//*	Mar  6,	2020	<MLS> ATIK cameras do not support gain, disabled if ATIK
//*	Mar  8,	2020	<MLS> Added camera temperature logging
//*	Mar  8,	2020	<MLS> Gain now updating properly if set from elsewhere
//*	Mar  9,	2020	<MLS> Added flag so we dont keep asking for ccd temp when not supported
//*	Mar  9,	2020	<MLS> Added cHas_readall flag
//*	Mar  9,	2020	<MLS> Added AlpacaGetStatus_ReadAll() & AlpacaGetStatus_OneAAT()
//*	Mar 10,	2020	<MLS> Re-read startup info when coming back online
//*	Mar 14,	2020	<MLS> On exit, turn the cooler off if it is on.
//*	Mar 17,	2020	<MLS> Error messages are now being displayed
//*	Mar 20,	2020	<MLS> Alpaca driver version now being displayed
//*	Mar 22,	2020	<MLS> Added SetFileNameOptions()
//*	Apr  4,	2020	<MLS> Stopped checking on cooler state if it is not present
//*	Apr  7,	2020	<MLS> Moving temp graph to graph tab
//*	Apr  7,	2020	<MLS> Added advanced tab
//*	Apr 21,	2020	<MLS> Added about box to camera controller
//*	Jun 24,	2020	<MLS> Made decision to switch camera to have sub classes
//*	Jun 24,	2020	<MLS> Added series of Update...() functions for sub class use
//*	Jun 24,	2020	<MLS> Added DownloadImage()
//*	Jun 25,	2020	<MLS> Added UpdateReceivedFileName()
//*	Jun 25,	2020	<MLS> Added UpdateCameraTemperature()
//*	Jun 29,	2020	<MLS> Added UpdateBackgroundColor()
//*	Jun 29,	2020	<MLS> Added UpdateFreeDiskSpace()
//*	Jan 15,	2021	<MLS> Added DownloadImage_rgbarray() & DownloadImage_imagearray()
//*	Jan 16,	2021	<MLS> Now able to download monochrome image using "imagearray"
//*	Jan 17,	2021	<MLS> Changed  UpdateReadAllStatus() to UpdateSupportedActions()
//*	Jan 25,	2021	<MLS> Converted CameraController to use properties struct
//*	Jan 29,	2021	<MLS> Added support for RANK=3 in DownloadImage_imagearray()
//*	Feb 14,	2021	<MLS> Added SetExposure() and SetGain()
//*	Feb 18,	2021	<MLS> Added Added AlpacaGetStatus_Gain() & AlpacaGetStatus_Exposure()
//*	Mar 27,	2021	<MLS> Added UpdateCameraOffset()
//*	Mar 27,	2021	<MLS> Added SetOffset() & BumpOffset()
//*	Sep  4,	2021	<MLS> Added AlpacaGetStartupData_OneAAT()
//*	Nov 16,	2021	<MLS> fitsview now handles color fits images (8 bit)
//*	Dec 12, 2021	<MLS> ImageArray now has RGB correct
//*	Dec 16,	2021	<MLS> Added support for percentcompleted
//*	Dec 16,	2021	<MLS> Added UpdatePercentCompleted()
//*	Dec 17,	2021	<MLS> Added support for canstopexposure
//*****************************************************************************
//*	Jan  1,	2121	<TODO> control key for different step size.
//*	Jan  1,	2121	<TODO> add error list window
//*	Jan  1,	2121	<TODO> save cross hair location
//*	Jan  1,	2121	<TODO> finish exposure step options
//*	Jan  1,	2121	<TODO> clear error msg
//*	Feb  6,	2121	<TODO> Move downloading of images to a separate thread
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

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kCamWindowWidth		456
#define	kCamWindowHeight	800


#include	"helper_functions.h"
#include	"windowtab_camera.h"
#include	"windowtab_camgraph.h"
#include	"windowtab_filelist.h"
#include	"windowtab_camsettings.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_camera.h"



//**************************************************************************************
ControllerCamera::ControllerCamera(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice,
									const int			xSize,
									const int			ySize)

	:Controller(argWindowName, xSize,  ySize)
{
int		iii;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	memset(&cCameraProp, 0, sizeof(TYPE_CameraProperties));
	strcpy(cAlpacaDeviceTypeStr,	"camera");


	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();

	cOnLine					=	true;		//*	assume its online, if it wasnt, we wouldnt be here
	cReadStartup			=	true;
	cHasCCDtemp				=	true;
	cHasCooler				=	true;
	cAutoExposure			=	false;
	cDisplayImage			=	false;
	cSaveAllImages			=	false;
	cHas_FilterWheel		=	false;
	cExposure				=	0.001;
	cDarkExposure			=	false;
	cCameraState_imageready	=	false;

	cLiveMode				=	false;
	cSideBar				=	false;

	cHas_autoexposure		=	false;
	cHas_displayimage		=	false;
	cHas_exposuretime		=	false;
	cHas_filelist			=	false;
	cHas_filenameoptions	=	false;
	cHas_livemode			=	false;
	cHas_rgbarray			=	false;
	cHas_sidebar			=	false;
	cHas_SaveAll			=	false;

	//*	download status stuff
	cPrevProgessValue		=	0.0;
	cProgressUpdates		=	0;
	cProgressReDraws		=	0;

	cReadData8Bit			=	false;

	//*	clear list of readout modes
	for (iii=0; iii<kMaxReadOutModes; iii++)
	{
		memset(&cCameraProp.ReadOutModes[iii], 0, sizeof(READOUTMODE));
	}

	//*	clear the filter wheel properties
	memset(&cFilterWheelProp, 0, sizeof(TYPE_FilterWheelProperties));
	//*	clear list of filterwheel names
	for (iii=0; iii<kMaxFiltersPerWheel; iii++)
	{
//-		memset(&cFilterNames[iii], 0, sizeof(FILTERWHEEL));
	}

	//*	clear list of remote files
	for (iii=0; iii<kMaxRemoteFileCnt; iii++)
	{
		memset(&cRemoteFiles[iii], 0, sizeof(TYPE_REMOTE_FILE));
	}

	//*	clear camera temperature array
	for (iii=0; iii<kMaxTemperatureValues; iii++)
	{
		cCameraTempLog[iii]	=	0.0;
	}
	cTempLogCount	=	0;

	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);


		CheckConnectedState();		//*	check connected and connect if not already connected

		GetConfiguredDevices();
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_

//	CONSOLE_DEBUG_W_STR("exit", cWindowName);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerCamera::~ControllerCamera(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cCameraProp.CoolerOn)
	{
		//*	the ATIK cameras need to have cooling turned off gracefully
		//*	so, just as a last minute thing, turn it off.
		CONSOLE_DEBUG_W_STR("turning coolor off", cWindowName);
		ToggleCooler();
	}
}

//**************************************************************************************
void	ControllerCamera::SetupWindowControls(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	CONSOLE_DEBUG("This routine should be overridden");
}

//**************************************************************************************
void	ControllerCamera::DrawWidgetCustom(TYPE_WIDGET *theWidget)
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
						theWidget->borderColor,		//	CvScalar color,
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


//**************************************************************************************
void	ControllerCamera::RunBackgroundTasks(bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;
bool		readStartUpOK;

	if (cButtonClickInProgress)
	{
		return;
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("cReadStartup == true", cWindowName);
		//*	so the window shows up
		HandleWindowUpdate();
		cvWaitKey(60);


		GetConfiguredDevices();

		readStartUpOK	=	AlpacaGetStartupData();
		if (readStartUpOK)
		{
			cReadStartup	=	false;
			if (cHas_FilterWheel)
			{
				AlpacaSetConnected("filterwheel", true);
				AlpacaGetFilterWheelStartup();
			}
		}

	}

	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead)
	{
		cFirstDataRead	=	false;
		needToUpdate	=	true;
	}

	if (deltaSeconds >= 5)
	{
		needToUpdate	=	true;
	}
	if (cForceAlpacaUpdate)	//*	force update is set when a switch is clicked
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}

	if (needToUpdate)
	{
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();

			if (validData == false)
			{
			//	CONSOLE_DEBUG("Failed to get data")
			}
			UpdateConnectedStatusIndicator();
		}
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
void	ControllerCamera::UpdateSupportedActions(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateRemoteAlpacaVersion(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraName(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateReadoutModes(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCurrReadoutMode(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraGain(const TYPE_ASCOM_STATUS lastAlpacaErr)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraOffset(const TYPE_ASCOM_STATUS lastAlpacaErr)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraExposure(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraSize(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraState(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraTemperature(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdatePercentCompleted(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateCoolerState(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateFilterWheelInfo(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateFilterWheelPosition(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateFileNameOptions(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateReceivedFileName(const char *newFileName)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}
//*****************************************************************************
void	ControllerCamera::UpdateRemoteFileList(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
}



//*****************************************************************************
void	ControllerCamera::UpdateDisplayModes(void)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
}


//*****************************************************************************
//*	this lets us see if there is a focuser present
//*****************************************************************************
void	ControllerCamera::GetConfiguredDevices(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
int				jjj;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	SJP_Init(&jsonParser);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										"/management/v1/configureddevices",
										NULL,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "DEVICETYPE") == 0)
			{
				if (strcasecmp(jsonParser.dataList[jjj].valueString, "Filterwheel") == 0)
				{
					CONSOLE_DEBUG("FilterWheel - !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					cHas_FilterWheel	=	true;
				}
			}
		}
	}
	else
	{
		cReadFailureCnt++;
		cOnLine	=	false;
		SetWindowIPaddrInfo(NULL, cOnLine);
	}
}

#pragma mark -


//*****************************************************************************
bool	ControllerCamera::AlpacaGetStartupData_OneAAT(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				readOutModeIdx;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	validData	=	false;

	//*	get the camera description
	if (cOnLine)
	{
		SJP_Init(&jsonParser);

		CONSOLE_DEBUG_W_STR(__FUNCTION__, "description");
		sprintf(alpacaString,	"/api/v1/camera/%d/description", cAlpacaDevNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			CONSOLE_DEBUG_W_STR(__FUNCTION__, "validData");
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{
	//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
	//											jsonParser.dataList[jjj].valueString);

				//*	this is an extra in AlpacaPi only
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "DEVICE") == 0)
				{
					strcpy(cCommonProp.Name,	jsonParser.dataList[jjj].valueString);
				}

				if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
				{
					if (strlen(jsonParser.dataList[jjj].valueString) >= kCommonPropMaxStrLen)
					{
						jsonParser.dataList[jjj].valueString[kCommonPropMaxStrLen - 1]	=	0;
					}
					strcpy(cCommonProp.Description,	jsonParser.dataList[jjj].valueString);
				}
			}
			UpdateCameraName();

		}
		else
		{
			cReadFailureCnt++;
			cOnLine	=	false;
		}
		CONSOLE_DEBUG_W_STR("cCommonProp.Name\t=", cCommonProp.Name);
	}

	//===============================================================
	//*	get the readout modes
	if (cOnLine)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "readoutmodes");
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/camera/%d/readoutmodes", cAlpacaDevNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			jjj	=	0;
			while (jjj<jsonParser.tokenCount_Data)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
				{
					readOutModeIdx	=	0;
					jjj++;
					while ((jjj<jsonParser.tokenCount_Data) &&
							(jsonParser.dataList[jjj].keyword[0] != ']'))
					{
						if (readOutModeIdx < kMaxReadOutModes)
						{
							strcpy(cCameraProp.ReadOutModes[readOutModeIdx].modeStr,
										jsonParser.dataList[jjj].valueString);

	//						CONSOLE_DEBUG(cCameraProp.ReadOutModes[readOutModeIdx].modeStr);
							readOutModeIdx++;
						}
						jjj++;
					}
					UpdateReadoutModes();
				}
				jjj++;
			}
		}
		else
		{
			CONSOLE_DEBUG("Read failure - readoutmodes");
			cReadFailureCnt++;
			cOnLine	=	false;
		}
	//	DEBUG_TIMING("readoutmodes");
	}

	if (cOnLine)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "cameraxsize");
		validData	=	AlpacaGetIntegerValue("camera", "cameraxsize",	NULL,	&cCameraProp.CameraXsize);
		validData	=	AlpacaGetIntegerValue("camera", "cameraysize",	NULL,	&cCameraProp.CameraYsize);
		UpdateCameraSize();

		//-----------------------------------------------------------------
		//*	deal with the GAIN information
		validData	=	AlpacaGetIntegerValue("camera", "gain",			NULL,	&cCameraProp.Gain);
		validData	=	AlpacaGetIntegerValue("camera", "gainmin",		NULL,	&cCameraProp.GainMin);
		validData	=	AlpacaGetIntegerValue("camera", "gainmax",		NULL,	&cCameraProp.GainMax);
	//	CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t", cLastAlpacaErrNum);
	//	CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t", cLastAlpacaErrStr);
		UpdateCameraGain(cLastAlpacaErrNum);

		//-----------------------------------------------------------------
		//*	deal with the OFFSET information
		validData	=	AlpacaGetIntegerValue("camera", "offset",			NULL,	&cCameraProp.Offset);
		validData	=	AlpacaGetIntegerValue("camera", "offsetmin",		NULL,	&cCameraProp.OffsetMin);
		validData	=	AlpacaGetIntegerValue("camera", "offsetmax",		NULL,	&cCameraProp.OffsetMax);
		UpdateCameraOffset(cLastAlpacaErrNum);

		validData	=	AlpacaGetDoubleValue("camera", "exposuremin",	NULL,	&cCameraProp.ExposureMin_seconds);
		validData	=	AlpacaGetDoubleValue("camera", "exposuremax",	NULL,	&cCameraProp.ExposureMax_seconds);
		UpdateCameraExposure();

		validData	=	AlpacaGetBooleanValue(	"camera", "canstopexposure",	NULL,	&cCameraProp.CanStopExposure);
		UpdateCameraState();
	}

	return(validData);

}

//*****************************************************************************
//*	this routine gets called one time to get the info on the camera that does not change
//*****************************************************************************
bool	ControllerCamera::AlpacaGetStartupData(void)
{
bool			validData;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

//	CONSOLE_DEBUG("Timing Start----------------------");
//	SETUP_TIMING();

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("camera", cAlpacaDevNum);
	if (validData)
	{
		//*	AlpacaGetSupportedActions() sets the cHas_readall appropriately
		UpdateSupportedActions();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
		cOnLine	=	false;
	}
//	DEBUG_TIMING("Timing for supportedactions=");

	AlpacaGetCommonProperties_OneAAT("camera");

	//===============================================================
	//*	Start by getting info about the camera
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("camera", cAlpacaDevNum);
	}
//	else
	{
		validData	=	AlpacaGetStartupData_OneAAT();
	}


	cLastUpdate_milliSecs	=	millis();

//	DEBUG_TIMING("camera specs");

	SetWindowIPaddrInfo(NULL, cOnLine);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
	return(validData);
}

//*****************************************************************************
void	ControllerCamera::AlpacaProcessSupportedActions(const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString)
{
//	CONSOLE_DEBUG_W_STR("valueString\t=", valueString);

	//*	put these in alphabetical order for easy of reading

	if (strcasecmp(valueString,			"autoexposure") == 0)
	{
		cHas_autoexposure	=	true;
	}
	else if (strcasecmp(valueString,	"displayimage") == 0)
	{
		cHas_displayimage	=	true;
	}
	else if (strcasecmp(valueString,	"exposuretime") == 0)
	{
		cHas_exposuretime	=	true;
	}
	else if (strcasecmp(valueString,	"filenameoptions") == 0)
	{
		cHas_filenameoptions	=	true;
	}
	else if (strcasecmp(valueString,	"livemode") == 0)
	{
		cHas_livemode	=	true;
	}
	else if (strcasecmp(valueString,	"readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString,	"rgbarray") == 0)
	{
		cHas_rgbarray	=	true;
	}
	else if (strcasecmp(valueString,	"saveallimages") == 0)
	{
		cHas_SaveAll	=	true;
	}
	else if (strcasecmp(valueString,	"sidebar") == 0)
	{
		cHas_sidebar	=	true;
	}
}



//*****************************************************************************
void	ControllerCamera::UpdateSettings_Object(const char *filePrefix)
{
	//*	this should be overloaded if needed

//+	if (cCamSettingsTabObjPtr != NULL)
//+	{
//+		cCamSettingsTabObjPtr->UpdateSettings_Object(filePrefix);
//+		cUpdateWindow	=	true;
//+	}
}

//*****************************************************************************
int	hextoi(const char *hexString)
{
int	intValue;
int	iii;

	intValue	=	0;
	iii			=	0;
	while (hexString[iii] >= 0x30)
	{
		intValue	=	intValue << 4;
		if (isdigit(hexString[iii]))
		{
			intValue	+=	hexString[iii] & 0x0f;
		}
		else if (hexString[iii] >= 'A')
		{
			intValue	+=	9;
			intValue	+=	hexString[iii] & 0x0f;

		}
		iii++;
	}
	return(intValue);
}

//*****************************************************************************
void	ControllerCamera::AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{

//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);

	//*	these are supposed to be in alphabetic order for ease of reading the code

	if (strcasecmp(keywordString, "autoexposure") == 0)
	{
		//=================================================================================
		//*	auto exposure
		cAutoExposure	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "backgroundcolor") == 0)
	{
	uint16_t	myRed;
	uint16_t	myGrn;
	uint16_t	myBlu;
	char		colorStr[8];

//		CONSOLE_DEBUG_W_STR("valueString\t=",	valueString);
		if (valueString[0] == '#')
		{
			colorStr[0]		=	valueString[1];
			colorStr[1]		=	valueString[2];
			colorStr[2]		=	0;
			myRed			=	hextoi(colorStr);

			colorStr[0]		=	valueString[3];
			colorStr[1]		=	valueString[4];
			myGrn			=	hextoi(colorStr);

			colorStr[0]		=	valueString[5];
			colorStr[1]		=	valueString[6];
			myBlu			=	hextoi(colorStr);

//-			SetWidgetBGColor(kTab_Camera, kCameraBox_Title, CV_RGB(myRed,	myGrn,	myBlu));
			if ((myRed + myGrn + myBlu) < (128 * 3))
			{
//-				SetWidgetTextColor(kTab_Camera, kCameraBox_Title, CV_RGB(255,	255,	255));
			}
			else
			{
//-				SetWidgetTextColor(kTab_Camera, kCameraBox_Title, CV_RGB(0,	0,	0));
			}

			UpdateBackgroundColor(myRed,	myGrn,	myBlu);

			cUpdateWindow	=	true;
		}
	}
	else if (strcasecmp(keywordString, "camerastate") == 0)
	{
		//=================================================================================
		//*	camerastate
		cCameraProp.CameraState	=	(TYPE_ALPACA_CAMERASTATE)atoi(valueString);
		UpdateCameraState();
	}
	else if (strcasecmp(keywordString, "cameraxsize") == 0)
	{
		cCameraProp.CameraXsize	=	atoi(valueString);
	}
	else if (strcasecmp(keywordString, "cameraysize") == 0)
	{
		cCameraProp.CameraYsize	=	atoi(valueString);
		UpdateCameraSize();
	}
	else if (strcasecmp(keywordString, "canstopexposure") == 0)
	{
		cCameraProp.CanStopExposure	=	IsTrueFalse(valueString);
		UpdateCommonProperties();
	}
	else if (strcasecmp(keywordString, "ccdtemperature") == 0)
	{
//		CONSOLE_DEBUG("ccdtemperature");
		//=================================================================================
		//*	ccdtemperature
		cCameraProp.CCDtemperature	=	atof(valueString);
		UpdateCameraTemperature();
		LogCameraTemp(cCameraProp.CCDtemperature);
	}
	else if (strcasecmp(keywordString, "cooleron") == 0)
	{
		//=================================================================================
		//*	cooler state
		cCameraProp.CoolerOn	=	IsTrueFalse(valueString);
		UpdateCoolerState();
	}
	else if (strcasecmp(keywordString, "displayImage") == 0)
	{
		cDisplayImage	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "exposuretime") == 0)
	{
		//=================================================================================
		//*	exposure
		cExposure	=	atof(valueString);

		UpdateCameraExposure();
	}
	else if (strcasecmp(keywordString, "fileNamePrefix") == 0)
	{
		UpdateSettings_Object(valueString);
	}
	else if (strcasecmp(keywordString, "filenameroot") == 0)
	{
		//=================================================================================
		//*	filenameroot
		if (strlen(valueString) > 0)
		{
			UpdateReceivedFileName(valueString);
		}
	}
	else if (strcasecmp(keywordString, "filenameincludefilter") == 0)
	{
		cFN_includeFilter	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "filenameincludecamera") == 0)
	{
		cFN_includeManuf	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "filenameincludeserialnum") == 0)
	{
		cFN_includeSerialNum	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "filenameincluderefid") == 0)
	{
		cFN_includeRefID	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "freeDisk_Gigabytes") == 0)
	{
	double	gigabytesFree;

		gigabytesFree	=	atof(valueString);
		UpdateFreeDiskSpace(gigabytesFree);
	}
	else if (strcasecmp(keywordString, "gain") == 0)
	{
		//=================================================================================
		//*	gain
		cCameraProp.Gain	=	atoi(valueString);
		UpdateCameraGain();
	}
	else if (strcasecmp(keywordString, "imageready") == 0)
	{
		//=================================================================================
		//*	imageready
		cCameraState_imageready	=	IsTrueFalse(valueString);
		UpdateCameraState();
	}
	else if (strcasecmp(keywordString, "livemode") == 0)
	{
		//=================================================================================
		//*	livemode
		cLiveMode	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "offset") == 0)
	{
		//=================================================================================
		//*	offset
		cCameraProp.Offset	=	atoi(valueString);
		UpdateCameraOffset();
	}
	else if (strcasecmp(keywordString, "percentcompleted") == 0)
	{
		//=================================================================================
		//*	percentcompleted
	}
	else if (strcasecmp(keywordString, "readoutmode") == 0)
	{
//		CONSOLE_DEBUG("readoutmode");
		//=================================================================================
		//*	readoutmode
		cCameraProp.ReadOutMode	=	atoi(valueString);
		UpdateCurrReadoutMode();
	}
	else if (strcasecmp(keywordString, "saveallimages") == 0)
	{
		//=================================================================================
		//*	save all images
		cSaveAllImages	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "sidebar") == 0)
	{
		//=================================================================================
		//*	sidebar
		cSideBar	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
		UpdateRemoteAlpacaVersion();

	}


}

//*****************************************************************************
bool	ControllerCamera::AlpacaGetStatus_Gain(void)
{
bool			validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	//=================================================================================
	//*	gain
	validData	=	AlpacaGetIntegerValue(	"camera", "gain",	NULL,	&cCameraProp.Gain);
	if (validData)
	{
		UpdateCameraGain();
	}
	else
	{
		cReadFailureCnt++;
	}
	return(validData);
}

//*****************************************************************************
bool	ControllerCamera::AlpacaGetStatus_Exposure(void)
{
bool			validData;
double			myExposureTime;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cHas_exposuretime)
	{
		validData	=	AlpacaGetDoubleValue(	"camera", "exposuretime",			NULL,	&myExposureTime);
		if (validData)
		{
			cExposure	=	myExposureTime;
			UpdateCameraExposure();
		}
		else
		{
			cReadFailureCnt++;
			CONSOLE_DEBUG("AlpacaGetDoubleValue failed");
		}
	}
	else
	{
		//*	dont want to cause a failure
		validData	=	true;
	}
	return(validData);
}



//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	ControllerCamera::AlpacaGetStatus_OneAAT(void)
{
bool			validData;
int				failedCnt;
double			myExposureTime;
int				argInt;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	failedCnt			=	0;

	//=================================================================================
	//*	gain
//	validData	=	AlpacaGetIntegerValue(	"camera", "gain",			NULL,	&cCameraProp.Gain);
	validData	=	AlpacaGetStatus_Gain();
	if (validData)
	{
		cOnLine	=	true;
	}
	else
	{
		failedCnt++;
		cOnLine	=	false;
	}


	//=================================================================================
	//*	camerastate
	if (cOnLine)
	{
		validData	=	AlpacaGetIntegerValue(	"camera", "camerastate", NULL,	&argInt);
		if (validData)
		{
			cCameraProp.CameraState	=	(TYPE_ALPACA_CAMERASTATE)argInt;
			UpdateCameraState();
		}
		else
		{
			failedCnt++;
			cOnLine	=	false;
		}
	}

	//=================================================================================
	//*	imageready
	if (cOnLine)
	{
		validData	=	AlpacaGetBooleanValue(	"camera", "imageready",	NULL,	&cCameraState_imageready);
		if (validData)
		{
			UpdateCameraState();
		}
		else
		{
			failedCnt++;
			cOnLine	=	false;
		}
	}

	//=================================================================================
	//*	ccdtemperature
	if (cOnLine)
	{
		if (cHasCCDtemp)
		{
		bool	tempDataValid;

			tempDataValid	=	true;
			validData	=	AlpacaGetDoubleValue(	"camera",
													"ccdtemperature",
													NULL,
													&cCameraProp.CCDtemperature,
													&tempDataValid);
//			if (cLastAlpacaErrNum != 0)
//			{
//				CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t", cLastAlpacaErrNum);
//			}

			if (validData)
			{
				if (tempDataValid)
				{
					UpdateCameraTemperature();
					LogCameraTemp(cCameraProp.CCDtemperature);
				}
				else
				{
//					CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t", cLastAlpacaErrNum);
		//			CONSOLE_DEBUG("Failed to read ccdtemperature");
					if (cLastAlpacaErrNum == kASCOM_Err_NotImplemented)
					{
						CONSOLE_DEBUG("Disabling ccd temp read, not implemented");
						cHasCCDtemp	=	false;
						UpdateCameraTemperature();
					}
				}
			}
			else
			{
				failedCnt++;
				cOnLine	=	false;
			}
		}
	}

	//=================================================================================
	//*	cooler state
	if (cOnLine)
	{
		if (cHasCooler)
		{
			validData	=	AlpacaGetBooleanValue(	"camera", "cooleron",	NULL,	&cCameraProp.CoolerOn);
			if (validData)
			{
//				if (cLastAlpacaErrNum != 0)
//				{
//					CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t", cLastAlpacaErrNum);
//				}
				if (cLastAlpacaErrNum == kASCOM_Err_NotImplemented)
				{
					CONSOLE_DEBUG("Disabling cooler checking, not implemented");
					cHasCooler	=	false;
				}
				UpdateCoolerState();
			}
			else
			{
				failedCnt++;
				cOnLine	=	false;
			}
		}
	}

	//=================================================================================
	//*	readoutmode
	if (cOnLine)
	{
		validData	=	AlpacaGetIntegerValue(	"camera", "readoutmode",	NULL,	&cCameraProp.ReadOutMode);
		if (validData)
		{
			UpdateCurrReadoutMode();
		}
		else
		{
			failedCnt++;
			cOnLine	=	false;
		}
	}

	//=================================================================================
	//*	percentcompleted
	if (cOnLine)
	{
		validData	=	AlpacaGetIntegerValue(	"camera", "percentcompleted",	NULL,	&cCameraProp.PercentCompleted);
		if (validData)
		{
			UpdatePercentCompleted();
		}
		else
		{
			failedCnt++;
			cOnLine	=	false;
		}
	}

	//=================================================================================
	//*	exposure
	if (cOnLine)
	{
		if (cHas_exposuretime)
		{
			validData	=	AlpacaGetDoubleValue(	"camera", "exposuretime",			NULL,	&myExposureTime);
			if (validData)
			{
				cExposure	=	myExposureTime;
				UpdateCameraExposure();
			}
			else
			{
				failedCnt++;
				cOnLine	=	false;
			}
		}
	}


	//=================================================================================
	//*	livemode
	if (cOnLine)
	{
		if (cHas_livemode)
		{
			validData	=	AlpacaGetBooleanValue(	"camera", "livemode",	NULL,	&cLiveMode);
			if (validData)
			{
			}
			else
			{
				failedCnt++;
				cOnLine	=	false;
			}
		}
	}

	//=================================================================================
	//*	auto exposure
	if (cOnLine)
	{
		if (cHas_autoexposure)
		{
			validData	=	AlpacaGetBooleanValue(	"camera", "autoexposure",	NULL,	&cAutoExposure);
			if (validData)
			{
			}
			else
			{
				failedCnt++;
				cOnLine	=	false;
			}
		}
	}


	if (failedCnt > 0)
	{
		CONSOLE_DEBUG("Data failure, probably off line");
//-		SetWidgetBGColor(kTab_Camera,	kCameraBox_IPaddr,	CV_RGB(255,	0,	0));
//-		SetWidgetBGColor(kTab_FileList,	kFileList_IPaddr,	CV_RGB(255,	0,	0));
	}
	else
	{
//-		SetWidgetBGColor(kTab_Camera,	kCameraBox_IPaddr,	CV_RGB(0,	0,	0));
//-		SetWidgetBGColor(kTab_FileList,	kFileList_IPaddr,	CV_RGB(0,	0,	0));
	}

	return(validData);
}


//*****************************************************************************
bool	ControllerCamera::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("camera", cAlpacaDevNum);
	}
	else
	{
//		CONSOLE_DEBUG("Calling AlpacaGetCommonConnectedState()");
		validData	=	AlpacaGetCommonConnectedState("camera");

//		CONSOLE_DEBUG("Calling AlpacaGetStatus_OneAAT()");
		validData	=	AlpacaGetStatus_OneAAT();	//*	One At A Time
	}
//	CONSOLE_DEBUG_W_STR("ValidData", (validData ? "True" : "FALSE"));

	if (validData)
	{
		UpdateDisplayModes();
		UpdateFileNameOptions();


		//*	check to see if we were online before
		if (cOnLine == false)
		{
			//*	if we go from offline back to online, re-do the startup info
			cReadStartup	=	true;
		}
		cOnLine	=	true;
#if 0
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != 0)
		{
		}
#endif // 0
	}
	else
	{
		cReadFailureCnt++;
		CONSOLE_DEBUG_W_STR("Offline-", cWindowName);
		cOnLine	=	false;
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	//=================================================================================
	//*	get the filter wheel position
	if (cHas_FilterWheel)
	{
		//*	this routine is in controller_fw_common.cpp
		AlpacaGetFilterWheelStatus();
	}

	cLastUpdate_milliSecs	=	millis();
	return(validData);
}


//*****************************************************************************
bool	ControllerCamera::AlpacaGetFileList(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				iii;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	//*	get the File list
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/camera/%d/filelist", cAlpacaDevNum);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		//*	clear list of remote files
		for (iii=0; iii<kMaxRemoteFileCnt; iii++)
		{
			memset(&cRemoteFiles[iii], 0, sizeof(TYPE_REMOTE_FILE));
		}

		CONSOLE_DEBUG_W_NUM("jsonParser.tokenCount_Data\t=", jsonParser.tokenCount_Data);
//		SJP_DumpJsonData(&jsonParser);
		jjj	=	0;
		while (jjj<jsonParser.tokenCount_Data)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
			{
				jjj++;
				while ((jjj<jsonParser.tokenCount_Data) &&
						(jsonParser.dataList[jjj].keyword[0] != ']'))
				{
					AddFileToRemoteList(jsonParser.dataList[jjj].valueString);
					jjj++;
				}
			}
			jjj++;
		}
		UpdateRemoteFileList();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - filelist");
		cReadFailureCnt++;
	}


	return(validData);
}

//*****************************************************************************
void	ControllerCamera::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
}


//*****************************************************************************
void	ControllerCamera::AddFileToRemoteList(const char *fileName)
{
char	myFileName[64];
char	fileExension[8];
int		iii;
int		sLen;
bool	fileNeedsAdded;
int		fileIndex;

	strcpy(myFileName, fileName);
	sLen	=	strlen(myFileName);
	iii		=	sLen;
	while ((myFileName[iii] != '.') && (iii > 0))
	{
		iii--;
	}
	myFileName[iii]	=	0;
	iii++;
	strncpy(fileExension, &myFileName[iii], 4);

	sLen	=	strlen(myFileName);
	if (sLen > 0)
	{
		//*	now look to see if the file is in the list
		fileIndex		=	-1;
		fileNeedsAdded	=	true;
		iii				=	0;
		while (	fileNeedsAdded
				&& (strlen(cRemoteFiles[iii].filename) > 0)
				&& (iii<kMaxRemoteFileCnt))
		{
			if (strcmp(myFileName, cRemoteFiles[iii].filename) == 0)
			{
				fileNeedsAdded	=	false;
				fileIndex		=	iii;
			}

			iii++;
		}
		if (fileNeedsAdded)
		{
			strcpy(cRemoteFiles[iii].filename, myFileName);
			cRemoteFiles[iii].validData	=	true;
			fileIndex		=	iii;
			CONSOLE_DEBUG_W_STR("new file\t=",	cRemoteFiles[iii].filename);

		}
		if (fileIndex >= 0)
		{
			if (strcasecmp(fileExension, "jpg") == 0)
			{
				cRemoteFiles[fileIndex].hasJPG	=	true;
			}
			else if (strcasecmp(fileExension, "fits") == 0)
			{
				cRemoteFiles[fileIndex].hasFTS	=	true;
			}
			else if (strcasecmp(fileExension, "fit") == 0)
			{
				cRemoteFiles[fileIndex].hasFTS	=	true;
			}
			else if (strcasecmp(fileExension, "csv") == 0)
			{
				cRemoteFiles[fileIndex].hasCSV	=	true;
			}
			else if (strcasecmp(fileExension, "png") == 0)
			{
				cRemoteFiles[fileIndex].hasPNG	=	true;
			}
		}
		//CONSOLE_DEBUG_W_STR("fileExension\t=",	fileExension);
	}
}

//*****************************************************************************
void	ControllerCamera::SetGain(const int newGainValue)
{
char	dataString[48];
bool	validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	sprintf(dataString, "Gain=%d", newGainValue);
	validData		=	AlpacaSendPutCmd(	"camera",
											"gain",
											dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//*****************************************************************************
void	ControllerCamera::BumpGain(const int howMuch)
{
int		newGainValue;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);


	newGainValue	=	cCameraProp.Gain + howMuch;
	SetGain(newGainValue);
}

//*****************************************************************************
void	ControllerCamera::SetOffset(const int newOffsetValue)
{
char	dataString[48];
bool	validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	sprintf(dataString, "offset=%d", newOffsetValue);
	validData		=	AlpacaSendPutCmd(	"camera",
											"offset",
											dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//*****************************************************************************
void	ControllerCamera::BumpOffset(const int howMuch)
{
int		newOffsetValue;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	newOffsetValue	=	cCameraProp.Offset + howMuch;
	SetOffset(newOffsetValue);
}


//*****************************************************************************
void	ControllerCamera::SetExposure(const double newExposure)
{
char	dataString[48];
bool	validData;

//	CONSOLE_DEBUG_W_DBL("newExposure\t", newExposure);

	cExposure	=	newExposure;
	if (cExposure < cCameraProp.ExposureMin_seconds)
	{
		cExposure	=	cCameraProp.ExposureMin_seconds;
	}
	if (cExposure > cCameraProp.ExposureMax_seconds)
	{
		cExposure	=	cCameraProp.ExposureMax_seconds;
	}
	if (cHas_exposuretime)
	{
		sprintf(dataString, "Duration=%f", cExposure);
		validData		=	AlpacaSendPutCmd(	"camera",
												"exposuretime",
												dataString);
		if (validData == false)
		{
			CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
		}
		AlpacaGetStatus_Exposure();
	}
	else
	{
		UpdateCameraExposure();
	}
}


//*****************************************************************************
void	ControllerCamera::BumpExposure(const double howMuch)
{
	SetExposure(cExposure + howMuch);

}

//*****************************************************************************
void	ControllerCamera::ToggleLiveMode(void)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "Livemode=%s", (cLiveMode ? "false" : "true"));
//	CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
	validData	=	AlpacaSendPutCmd(	"camera", "livemode",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//*****************************************************************************
void	ControllerCamera::ToggleSideBar(void)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "Sidebar=%s", (cSideBar ? "false" : "true"));
	validData	=	AlpacaSendPutCmd(	"camera", "sidebar",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//*****************************************************************************
void	ControllerCamera::ToggleAutoExposure(void)
{
char	dataString[48];
bool	validData;

	CONSOLE_DEBUG(__FUNCTION__);

	sprintf(dataString, "autoexposure=%s", (cAutoExposure ? "false" : "true"));
	CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
	validData	=	AlpacaSendPutCmd(	"camera", "autoexposure",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
	if (cLastAlpacaErrNum != 0)
	{
		CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=", cLastAlpacaErrNum)
		CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr)
	}
}

//*****************************************************************************
void	ControllerCamera::ToggleDisplayImage(void)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "displayimage=%s", (cDisplayImage ? "false" : "true"));
	validData	=	AlpacaSendPutCmd(	"camera", "displayimage",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//*****************************************************************************
void	ControllerCamera::ToggleSaveAll(void)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "saveallimages=%s", (cSaveAllImages ? "false" : "true"));
	validData	=	AlpacaSendPutCmd(	"camera", "saveallimages",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}



//*****************************************************************************
void	ControllerCamera::ToggleCooler(void)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "CoolerOn=%s", (cCameraProp.CoolerOn ? "false" : "true"));
//	CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
	validData	=	AlpacaSendPutCmd(	"camera", "cooleron",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}


//*****************************************************************************
void	ControllerCamera::StartExposure(void)
{
bool			validData;
SJP_Parser_t	jsonParser;
int				jjj;
char			parameterString[128];

	CONSOLE_DEBUG(__FUNCTION__);

	UpdateReceivedFileName("unknown");
	AlpacaDisplayErrorMessage("---");

	//*	my drivers have extra commands, readall and live mode.
	//*	if we are in live mode, just tell the driver to save the next frame
	if (cHas_readall && cLiveMode)
	{
//		CONSOLE_DEBUG("savenextimage");
		validData	=	AlpacaSendPutCmdwResponse(	"camera", "savenextimage",	NULL, &jsonParser);
	}
	else if (cHas_exposuretime)
	{
//		CONSOLE_DEBUG("Calling AlpacaSendPutCmdwResponse");
		validData	=	AlpacaSendPutCmdwResponse(	"camera",
													"startexposure",
													NULL,
													&jsonParser);
	}
	else
	{
		//********************************************************************
		//*	OK, this is a normal alpaca driver without any of my extras.
		//*	we need "Duration=0.123&Light=true&ClientID=22&ClientTransactionID=33"
		//*	the "ClientID=22&ClientTransactionID=33" will be added by the next routine
		//********************************************************************

		sprintf(parameterString, "Duration=%f&Light=%s",	cExposure,
															(cDarkExposure ? "false" : "true"));
		CONSOLE_DEBUG("NORMAL CAMERA, NOT AlpacaPi");
		CONSOLE_DEBUG_W_2STR("sending", "startexposure", parameterString);
		validData	=	AlpacaSendPutCmdwResponse(	"camera",
													"startexposure",
													parameterString,
													&jsonParser);
	}

	if (validData)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "filenameroot") == 0)
			{
		//		CONSOLE_DEBUG_W_STR("device type\t=", jsonParser.dataList[jjj].valueString);
				UpdateReceivedFileName(jsonParser.dataList[jjj].valueString);
			}
		}
	}
	else
	{
		cReadFailureCnt++;
		CONSOLE_DEBUG("validData is false");
	}
}

//*****************************************************************************
void	ControllerCamera::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
{
	//*	this is to be over loaded if needed
}

//*****************************************************************************
void	ControllerCamera::UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue)
{
	//*	this is to be over loaded if needed
}
//*****************************************************************************
void	ControllerCamera::UpdateFreeDiskSpace(const double gigabytesFree)
{
	//*	this is to be over loaded if needed
}

//*****************************************************************************
IplImage	*ControllerCamera::DownloadImage_rgbarray(void)
{
IplImage	*myOpenCVimage	=	NULL;
bool		validData;
int			pixelsCount;
int			*imageData;
int			valuesRead;
int			iii;
int			pixIdx;

	CONSOLE_DEBUG(__FUNCTION__);

	pixelsCount	=	cCameraProp.CameraXsize * cCameraProp.CameraYsize;
	if (pixelsCount > 0)
	{
		imageData	=	(int *)malloc(pixelsCount * sizeof(int));
		if (imageData!= NULL)
		{
			valuesRead	=	0;
			validData	=	AlpacaGetIntegerArrayShortLines(	"camera",
													cAlpacaDevNum,
													"rgbarray",
													"",
													imageData,
													pixelsCount,
													&valuesRead);

			CONSOLE_DEBUG_W_NUM("valuesRead\t\t=", valuesRead);
			if (validData && (valuesRead > 10))
			{
				myOpenCVimage	=	cvCreateImage(	cvSize(cCameraProp.CameraXsize, cCameraProp.CameraYsize),
													IPL_DEPTH_8U,
													3);
				if (myOpenCVimage != NULL)
				{
					if (myOpenCVimage->imageData != NULL)
					{
						memset(myOpenCVimage->imageData, 128, (myOpenCVimage->height * myOpenCVimage->widthStep));
					}

					//*	move the image data into the openCV image structure
					pixIdx	=	0;
					for (iii=0; iii<pixelsCount; iii++)
					{
						myOpenCVimage->imageData[pixIdx++]	=	(imageData[iii] >> 16) & 0x00ff;
						myOpenCVimage->imageData[pixIdx++]	=	(imageData[iii] >> 8) & 0x00ff;
						myOpenCVimage->imageData[pixIdx++]	=	(imageData[iii]) & 0x00ff;
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("Failed to download integer array");
			}

			free(imageData);
		}
		else
		{
			CONSOLE_DEBUG("Failed to allocate image buffer");
		}
	}
	else
	{
		CONSOLE_DEBUG("Image size is not known");
	}
	return(myOpenCVimage);
}

//*****************************************************************************
IplImage	*ControllerCamera::DownloadImage_imagearray(const bool force8BitRead, const bool allowBinary)
{
IplImage		*myOpenCVimage	=	NULL;
int				pixelCount;
int				valuesRead;
int				iii;
int				xxx;
int				yyy;
int				pixIdx;
int				pixIdxRowStart;
int				imgRank;
TYPE_ImageArray	*imageArray;
int				buffSize;


	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cCameraProp.CameraXsize\t=",	cCameraProp.CameraXsize);
	CONSOLE_DEBUG_W_NUM("cCameraProp.CameraYsize\t=",	cCameraProp.CameraYsize);
	CONSOLE_DEBUG_W_NUM("force8BitRead\t\t=",	force8BitRead);
	CONSOLE_DEBUG_W_NUM("allowBinary\t\t=",		allowBinary);

	imageArray		=	NULL;
	pixelCount		=	cCameraProp.CameraXsize * cCameraProp.CameraYsize;
	if (pixelCount > 0)
	{
		CONSOLE_DEBUG_W_NUM("pixelCount\t=", pixelCount);
		buffSize	=	(pixelCount + 100) * sizeof(TYPE_ImageArray);
		CONSOLE_DEBUG_W_NUM("buffSize\t=", buffSize);
		imageArray	=	(TYPE_ImageArray *)malloc(buffSize);
		if (imageArray!= NULL)
		{
			memset(imageArray, 0, buffSize);
			valuesRead	=	0;
			CONSOLE_DEBUG("Calling AlpacaGetImageArray()");
			SETUP_TIMING();
			imgRank	=	AlpacaGetImageArray(	"camera",
												cAlpacaDevNum,
												"imagearray",
												"",
												allowBinary,
												imageArray,
												pixelCount,
												&valuesRead);

			DEBUG_TIMING("Image downloading (ms)");
			CONSOLE_DEBUG_W_NUM("imgRank\t\t=",		imgRank);
			CONSOLE_DEBUG_W_NUM("valuesRead\t\t=",	valuesRead);
			if ((imgRank > 0) && (valuesRead > 10))
			{
				myOpenCVimage	=	cvCreateImage(cvSize(	cCameraProp.CameraXsize, cCameraProp.CameraYsize),
															IPL_DEPTH_8U,
															3);
				if (myOpenCVimage != NULL)
				{
					if (myOpenCVimage->imageData != NULL)
					{
						memset(myOpenCVimage->imageData, 128, (myOpenCVimage->height * myOpenCVimage->widthStep));
					}
					CONSOLE_DEBUG_W_NUM("width \t\t=",	myOpenCVimage->width);
					CONSOLE_DEBUG_W_NUM("height\t\t=",	myOpenCVimage->height);
					CONSOLE_DEBUG_W_NUM("widthStep\t=",	myOpenCVimage->widthStep);

					//*	move the image data into the openCV image structure
					START_TIMING();
					iii	=	0;
					//*	stepping ACROSS the field
					for (xxx=0; xxx < myOpenCVimage->width; xxx++)
					{
						pixIdxRowStart	=	0;
						//*	stepping DOWN the column
						for (yyy=0; yyy < myOpenCVimage->height; yyy++)
						{
							pixIdx	=	pixIdxRowStart + (xxx * 3);

							//*	openCV uses BGR instead of RGB
							//*	https://docs.opencv.org/master/df/d24/tutorial_js_image_display.html

							if (force8BitRead)
							{
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].BluValue) & 0x00ff;
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].GrnValue) & 0x00ff;
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].RedValue) & 0x00ff;
							}
							else
							{
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].BluValue >> 8) & 0x00ff;
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].GrnValue >> 8) & 0x00ff;
								myOpenCVimage->imageData[pixIdx++]	=	(imageArray[iii].RedValue >> 8) & 0x00ff;
							}

							iii++;
							//*	advance to the next row
							pixIdxRowStart	+=	myOpenCVimage->widthStep;
						}
					}
					DEBUG_TIMING("Image stuffing (ms)");
					CONSOLE_DEBUG_W_NUM("iii\t\t=",		iii);
				}
			}
			else
			{
				CONSOLE_DEBUG("imgRank is invalid");
			}
			free(imageArray);
		}
		else
		{
			CONSOLE_DEBUG("Failed to allocate image buffer");
		}
	}
	else
	{
		CONSOLE_DEBUG("Image size is not known");
	}
	return(myOpenCVimage);
}

//*****************************************************************************
IplImage	*ControllerCamera::DownloadImage(const bool force8BitRead,  const bool allowBinary)
{
IplImage	*myOpenCVimage	=	NULL;

	myOpenCVimage	=	DownloadImage_imagearray(force8BitRead, allowBinary);

	return(myOpenCVimage);
}


//**************************************************************************************
void	ControllerCamera::SetObjectText(const char *newObjectText, const char *newPrefixText)
{
char	dataString[48];
bool	validData;

	//	-d "Prefix=$OBJECTNAME&Object=OBJECTNAME&Sufix=$SUFFIX"

	sprintf(dataString, "Object=%s&Prefix=%s", newObjectText, newPrefixText);
	CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
	validData	=	AlpacaSendPutCmd(	"camera", "settelescopeinfo",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}

//**************************************************************************************
void	ControllerCamera::SetFileNameOptions(const int fnOptionBtn)
{
char	dataString[48];
bool	validData;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(fnOptionBtn)
	{
		case kCamSet_FN_IncFilter:
			sprintf(dataString, "includefilter=%s",		(cFN_includeFilter ? "false" : "true"));
			break;

		case kCamSet_FN_IncCamera:
			sprintf(dataString, "includecamera=%s",		(cFN_includeManuf ? "false" : "true"));
			break;

		case kCamSet_FN_IncSerialNum:
			sprintf(dataString, "includeserialnum=%s",	(cFN_includeSerialNum ? "false" : "true"));
			break;

		case kCamSet_FN_IncRefID:
			sprintf(dataString, "includerefid=%s",	(cFN_includeRefID ? "false" : "true"));
			break;

	}
	validData	=	AlpacaSendPutCmd(	"camera", "filenameoptions",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}


//**************************************************************************************
void	ControllerCamera::LogCameraTemp(const double cameraTemp)
{
long	currMillsecs;
int		secondsSinceStart;
int		centiSeconds;
int		iii;

	if (cameraTemp < 50.0)
	{
		//*	we want to log every 10 seconds
		currMillsecs		=	millis();
		secondsSinceStart	=	currMillsecs / 1000;
		centiSeconds		=	secondsSinceStart / 6;
		if (centiSeconds < kMaxTemperatureValues)
		{
			cCameraTempLog[centiSeconds]	=	cameraTemp;
		}
		else
		{
			//*	move everything back one
			for (iii=0; iii<(kMaxTemperatureValues-1); iii++)
			{
				cCameraTempLog[iii]	=	cCameraTempLog[iii + 1];
			}
			cCameraTempLog[kMaxTemperatureValues-1]	=	cameraTemp;
		}
	}
}

//**************************************************************************************
void	ControllerCamera::UpdateConnectedStatusIndicator(void)
{
	//*	this doesnt do anything, it should be overridden
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
#define	PARENT_CLASS	ControllerCamera

#include "controller_fw_common.cpp"



#endif // _ENABLE_CTRL_CAMERA_
