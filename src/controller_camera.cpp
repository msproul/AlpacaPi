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
//*	Re-distributions of this source code must retain this copyright notice.
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
//*	Dec 12,	2021	<MLS> ImageArray now has RGB correct
//*	Dec 16,	2021	<MLS> Added support for percentcompleted
//*	Dec 16,	2021	<MLS> Added UpdatePercentCompleted()
//*	Dec 17,	2021	<MLS> Added support for canstopexposure
//*	Dec 23,	2021	<MLS> Added UpdateFlipMode() & SetFlipMode()
//*	Dec 26,	2021	<MLS> Update timeout is now 1 second if camera is anything but idle
//*	Sep 21,	2022	<MLS> Added ProcessReadAll_IMU()
//*	Apr 30,	2023	<MLS> Added processing of "setccdtemperature" to readall
//*	May 29,	2023	<MLS> Added SetExposureRange()
//*	May 30,	2023	<MLS> Added UpdateCameraExposureStartup()
//*	May 30,	2023	<MLS> Exposure step options finally finished
//*	Jun  4,	2023	<MLS> Added ProcessReadAll_SaveAs()
//*	Jun 25,	2023	<ADD> Add binx and biny to DeviceState
//*	Jun 25,	2023	<ADD> Add gain to DeviceState
//*	Jun 25,	2023	<ADD> Add cooleron to DeviceState
//*	Jun 25,	2023	<ADD> Add offset to DeviceState
//*	Jun 25,	2023	<ADD> Add readoutmode to DeviceState
//*	Jun 25,	2023	<ADD> Add startx and starty to DeviceState
//*	Jul  1,	2023	<MLS> Added GetStatus_SubClass() to camera controller
//*****************************************************************************
//*	Jan  1,	2121	<TODO> control key for different step size.
//*	Jan  1,	2121	<TODO> add error list window
//*	Feb  6,	2121	<TODO> Move downloading of images to a separate thread
//*****************************************************************************

#ifdef _ENABLE_CTRL_CAMERA_



#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"alpaca_defs.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"windowtab_camsettings.h"

#include	"controller.h"
#include	"controller_camera.h"
#include	"camera_AlpacaCmds.h"
#include	"camera_AlpacaCmds.cpp"



//**************************************************************************************
ControllerCamera::ControllerCamera(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice,
									const int			xSize,
									const int			ySize)

	:Controller(argWindowName, xSize,  ySize, true, alpacaDevice)
{
int		iii;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	memset(&cCameraProp, 0, sizeof(TYPE_CameraProperties));
	strcpy(cAlpacaDeviceTypeStr,	"camera");

	SetCommandLookupTable(gCameraCmdTable);
	SetAlternateLookupTable(gCameraExtrasTable);

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
	cHas_autoexposure		=	false;
	cHas_displayimage		=	false;
	cHas_exposuretime		=	false;
	cHas_filelist			=	false;
	cHas_filenameoptions	=	false;
	cHas_livemode			=	false;
	cHas_rgbarray			=	false;
	cHas_SaveAll			=	false;
	cHas_Flip				=	false;

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

//*****************************************************************************
void	ControllerCamera::GetStatus_SubClass(void)
{
	//=================================================================================
	//*	get the filter wheel position
	if (cHas_FilterWheel)
	{
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//*	this routine is in controller_fw_common.cpp
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		AlpacaGetFilterWheelStatus();

		if (cHas_FilterWheel == false)
		{
			CONSOLE_DEBUG("FilterWheel is now disabled");
		}
	}
}

//*****************************************************************************
void	ControllerCamera::UpdateSupportedActions(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//**************************************************************************************
void	ControllerCamera::UpdateStatusData(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraName(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateReadoutModes(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCurrReadoutMode(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraGain(const TYPE_ASCOM_STATUS lastAlpacaErr)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraOffset(const TYPE_ASCOM_STATUS lastAlpacaErr)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerCamera::UpdateCameraExposureStartup(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraExposure(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraSize(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraState(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCameraTemperature(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdatePercentCompleted(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateCoolerState(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateFilterWheelInfo(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateFilterWheelPosition(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateFileNameOptions(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateLiveMode(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateReceivedFileName(const char *newFileName)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateRemoteFileList(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::SetExposureRange(char *name, double exposureMin, double exposureMax, double exposureStep)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateDisplayModes(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}
//*****************************************************************************
void	ControllerCamera::UpdateFlipMode(void)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
}


//*****************************************************************************
void	ControllerCamera::ProcessConfiguredDevices(const char *keyword, const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("kw:val", keyword, valueString);

	if (strcasecmp(keyword, "DEVICETYPE") == 0)
	{
		if (strcasecmp(valueString, "Filterwheel") == 0)
		{
//			CONSOLE_DEBUG("FilterWheel - !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			cHas_FilterWheel	=	true;
		}
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

	//*	common properties are already taken care of.

	UpdateCameraName();
//	CONSOLE_DEBUG_W_BOOL("cOnLine  \t=",	cOnLine);
//	CONSOLE_DEBUG_W_BOOL("validData\t=",	validData);
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
		validData	=	AlpacaGetIntegerValue("camera", "gainmin",		NULL,	&cCameraProp.GainMin);
		validData	=	AlpacaGetIntegerValue("camera", "gainmax",		NULL,	&cCameraProp.GainMax);
		validData	=	AlpacaGetIntegerValue("camera", "gain",			NULL,	&cCameraProp.Gain);
		UpdateCameraGain(cLastAlpacaErrNum);

		//-----------------------------------------------------------------
		//*	deal with the OFFSET information
		validData	=	AlpacaGetIntegerValue("camera", "offset",			NULL,	&cCameraProp.Offset);
		validData	=	AlpacaGetIntegerValue("camera", "offsetmin",		NULL,	&cCameraProp.OffsetMin);
		validData	=	AlpacaGetIntegerValue("camera", "offsetmax",		NULL,	&cCameraProp.OffsetMax);
		UpdateCameraOffset(cLastAlpacaErrNum);

		validData	=	AlpacaGetDoubleValue("camera", "exposuremin",	NULL,	&cCameraProp.ExposureMin_seconds);
		validData	=	AlpacaGetDoubleValue("camera", "exposuremax",	NULL,	&cCameraProp.ExposureMax_seconds);
		UpdateCameraExposureStartup();

		UpdateCameraState();
	}
	return(validData);
}

//*****************************************************************************
void	ControllerCamera::AlpacaGetCapabilities(void)
{
	ReadOneDriverCapability("camera",	"canabortexposure",		"CanAbortExposure",		&cCameraProp.CanAbortExposure);
	ReadOneDriverCapability("camera",	"canasymmetricbin",		"CanAsymmetricBin",		&cCameraProp.CanAsymmetricBin);
	ReadOneDriverCapability("camera",	"canfastreadout",		"CanFastreadout",		&cCameraProp.CanFastReadout);
	ReadOneDriverCapability("camera",	"cangetcoolerpower",	"CanGetCoolerPower",	&cCameraProp.CanGetCoolerPower);
	ReadOneDriverCapability("camera",	"canpulseguide",		"CanPulseguide",		&cCameraProp.CanPulseGuide);
	ReadOneDriverCapability("camera",	"cansetccdtemperature",	"CanSetCCDtemperature",	&cCameraProp.CanSetCCDtemperature);
	ReadOneDriverCapability("camera",	"canstopexposure",		"CanStopExposure",		&cCameraProp.CanStopExposure);
}

//*****************************************************************************
void	ControllerCamera::GetStartUpData_SubClass(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				readOutModeIdx;

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
	}
	if (cHas_FilterWheel)
	{
		AlpacaSetConnected("filterwheel", true);
		AlpacaGetFilterWheelStartup();
	}
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
	else if (strcasecmp(valueString, "devicestate") == 0)
	{
		cHas_DeviceState	=	true;
	}
	else if (strcasecmp(valueString,	"rgbarray") == 0)
	{
		cHas_rgbarray	=	true;
	}
	else if (strcasecmp(valueString,	"saveallimages") == 0)
	{
		cHas_SaveAll	=	true;
	}
	else if (strcasecmp(valueString,	"flip") == 0)
	{
		cHas_Flip	=	true;
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
//*	this routine is going to be present even if IMU support is not enabled
//*	this one is a place holder, the real processing will be in the subclass (controller_cam_normal)
//*****************************************************************************
void	ControllerCamera::ProcessReadAll_IMU(	const char	*deviceTypeStr,
											const int	deviceNum,
											const char	*keywordString,
											const char	*valueString)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this should be over ridden if it is needed
}

//*****************************************************************************
void	ControllerCamera::ProcessReadAll_SaveAs(const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this should be over ridden if it is needed
}

//*****************************************************************************
bool	ControllerCamera::AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString)
{
bool	dataWasHandled	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
	switch(keywordEnum)
	{
		//=================================================================================
		case kCmd_Camera_bayeroffsetX:
			cCameraProp.BayerOffsetX	=	atoi(valueString);
			break;

		case kCmd_Camera_bayeroffsetY:
			cCameraProp.BayerOffsetY	=	atoi(valueString);
			break;

		case kCmd_Camera_binX:
			cCameraProp.BinX	=	atoi(valueString);
			break;

		case kCmd_Camera_binY:
			cCameraProp.BinY	=	atoi(valueString);
			break;

		case kCmd_Camera_camerastate:
			//=================================================================================
			cCameraProp.CameraState	=	(TYPE_ALPACA_CAMERASTATE)atoi(valueString);
			UpdateCameraState();
			break;

		case kCmd_Camera_cameraxsize:
			cCameraProp.CameraXsize	=	atoi(valueString);
			break;

		case kCmd_Camera_cameraysize:
			cCameraProp.CameraYsize	=	atoi(valueString);
			UpdateCameraSize();
			break;

		//*	Indicates whether the camera can abort exposures.
		case kCmd_Camera_canabortexposure:
			cCameraProp.CanAbortExposure	=	IsTrueFalse(valueString);
			break;

		//*	Indicates whether the camera supports asymmetric binning
		case kCmd_Camera_canasymmetricbin:
			cCameraProp.CanAsymmetricBin	=	IsTrueFalse(valueString);
			break;

		//*	Indicates whether the camera has a fast readout mode.
		case kCmd_Camera_canfastreadout:
			cCameraProp.CanFastReadout	=	IsTrueFalse(valueString);
			break;

		//*	Indicates whether the camera's cooler power setting can be read.
		case kCmd_Camera_cangetcoolerpower:
			cCameraProp.CanGetCoolerPower	=	IsTrueFalse(valueString);
			break;

		//*	Indicates whether this camera supports pulse guiding
		case kCmd_Camera_canpulseguide:
			cCameraProp.CanPulseGuide	=	IsTrueFalse(valueString);
			break;

		//*	Indicates whether this camera supports setting the CCD temperature
		case kCmd_Camera_cansetccdtemperature:
			cCameraProp.CanSetCCDtemperature	=	IsTrueFalse(valueString);
			break;

		//*	Returns a flag indicating whether this camera can stop an exposure that is in progress
		case kCmd_Camera_canstopexposure:
			cCameraProp.CanStopExposure	=	IsTrueFalse(valueString);
			UpdateCommonProperties();
			break;

		case kCmd_Camera_ccdtemperature:
			//=================================================================================
			//*	ccdtemperature
			cCameraProp.CCDtemperature	=	AsciiToDouble(valueString);
			UpdateCameraTemperature();
			LogCameraTemp(cCameraProp.CCDtemperature);
			break;

		case kCmd_Camera_cooleron:
			//=================================================================================
			//*	cooler state
			cCameraProp.CoolerOn	=	IsTrueFalse(valueString);
			UpdateCoolerState();
			break;

		case kCmd_Camera_CoolerPower:
			cCameraProp.CoolerPower	=	atof(valueString);
			break;

		case kCmd_Camera_electronsperadu:
			cCameraProp.ElectronsPerADU	=	atof(valueString);
			break;

		case kCmd_Camera_exposuremax:
			cCameraProp.ExposureMax_seconds	=	atof(valueString);
			break;

		case kCmd_Camera_exposuremin:
			cCameraProp.ExposureMin_seconds	=	atof(valueString);
			break;

		case kCmd_Camera_exposureresolution:
			cCameraProp.ExposureResolution	=	atof(valueString);
			break;

		case kCmd_Camera_fastreadout:
			cCameraProp.FastReadout	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_FullWellCapacity:
			cCameraProp.FullWellCapacity	=	atof(valueString);
			break;

		case kCmd_Camera_gain:
			cCameraProp.Gain	=	atoi(valueString);
//			CONSOLE_DEBUG_W_NUM("cCameraProp.Gain\t=", cCameraProp.Gain);
			break;

		case kCmd_Camera_gainmax:
			cCameraProp.GainMax	=	atoi(valueString);
//			CONSOLE_DEBUG_W_NUM("cCameraProp.GainMax\t=", cCameraProp.GainMax);
			break;

		case kCmd_Camera_gainmin:
			cCameraProp.GainMin	=	atoi(valueString);
//			CONSOLE_DEBUG_W_NUM("cCameraProp.GainMin\t=", cCameraProp.GainMin);
			break;

		case kCmd_Camera_gains:
			break;

		case kCmd_Camera_hasshutter:
			cCameraProp.HasShutter	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_heatsinktemperature:
			cCameraProp.HeatSinkTemperature	=	atof(valueString);
			break;

		case kCmd_Camera_imagearray:
			break;

		case kCmd_Camera_imagearrayvariant:
			break;

		//*	stored in microseconds, ASCOM wants seconds, convert on the fly
		case kCmd_Camera_lastexposureduration:
			cCameraProp.Lastexposure_duration_us	=	1000000 * atof(valueString);
			break;

		case kCmd_Camera_lastexposurestarttime:
			break;

		case kCmd_Camera_imageready:
			cCameraState_imageready	=	IsTrueFalse(valueString);
			UpdateCameraState();
			break;

		case kCmd_Camera_IsPulseGuiding:
			cCameraProp.IsPulseGuiding	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_maxadu:
			cCameraProp.MaxADU	=	atoi(valueString);
			break;

		case kCmd_Camera_maxbinX:
			cCameraProp.MaxbinX	=	atoi(valueString);
			break;

		case kCmd_Camera_maxbinY:
			cCameraProp.MaxbinY	=	atoi(valueString);
			break;


		case kCmd_Camera_numX:	//*	The current subframe width
			cCameraProp.NumX	=	atoi(valueString);
			break;

		case kCmd_Camera_numY:	//*	The current subframe height
			cCameraProp.NumY	=	atoi(valueString);
			break;

		case kCmd_Camera_offset:
			//*	offset
			cCameraProp.Offset	=	atoi(valueString);
			UpdateCameraOffset();
			break;

		case kCmd_Camera_offsetmax:
			cCameraProp.OffsetMax	=	atoi(valueString);
			break;

		case kCmd_Camera_offsetmin:
			cCameraProp.OffsetMin	=	atoi(valueString);
			break;

		case kCmd_Camera_offsets:
			break;

		case kCmd_Camera_percentcompleted:
			cCameraProp.PercentCompleted	=	atoi(valueString);
			UpdatePercentCompleted();
			break;

		//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
		case kCmd_Camera_PixelSizeX:
			cCameraProp.PixelSizeX	=	atof(valueString);
			break;

		//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
		case kCmd_Camera_PixelSizeY:
			cCameraProp.PixelSizeY	=	atof(valueString);
			break;

		case kCmd_Camera_readoutmode:
			cCameraProp.ReadOutMode	=	atoi(valueString);
			UpdateCurrReadoutMode();
			break;

		case kCmd_Camera_readoutmodes:
			break;

		case kCmd_Camera_sensorname:
			strcpy(cCameraProp.SensorName, valueString);
			break;

		//*	Type of information returned by the the camera sensor (monochrome or colour)
		case kCmd_Camera_sensortype:
			cCameraProp.SensorType	=	(TYPE_SensorType)atoi(valueString);
			break;

		case kCmd_Camera_setccdtemperature:
			//=================================================================================
			//*	setccdtemperature
			cCameraProp.SetCCDTemperature	=	atof(valueString);
			UpdateCameraTemperature();
			break;

		case kCmd_Camera_startX:
			cCameraProp.StartX	=	atoi(valueString);
			break;

		case kCmd_Camera_startY:
			cCameraProp.StartY	=	atoi(valueString);
			break;

		case kCmd_Camera_subexposureduration:
			cCameraProp.SubExposureDuration	=	atof(valueString);
			break;

		//=================================================================================
		//*	non standard commands
		//=================================================================================
		case kCmd_Camera_autoexposure:
			cAutoExposure	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_BackGroundColor:
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

					UpdateBackgroundColor(myRed,	myGrn,	myBlu);

					cUpdateWindow	=	true;
				}
			}
			break;

		case kCmd_Camera_displayimage:
			cDisplayImage	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_ExposureTime:		//*	exposure time in ?
			cExposure	=	AsciiToDouble(valueString);

			UpdateCameraExposure();
			break;

		case kCmd_Camera_filelist:
			break;

		case kCmd_Camera_FileNamePrefix:
			UpdateSettings_Object(valueString);
			break;

		case kCmd_Camera_FileNameSuffix:
			break;

		case kCmd_Camera_FilenameRoot:
			if (strlen(valueString) > 0)
			{
				UpdateReceivedFileName(valueString);
			}
			break;

		case kCmd_Camera_FilenameIncludeFilter:
			cFN.IncludeFilter	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_FilenameIncludeCamera:
			cFN.IncludeManuf	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_FilenameIncludeSerialnum:
			cFN.IncludeSerialNum	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_FilenameIncludeRefid:
			cFN.IncludeRefID	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_fitsheader:
			break;

		case kCmd_Camera_framerate:
			break;

		case kCmd_Camera_FreeDisk_Gigabytes:
			{
			double	gigabytesFree;

				gigabytesFree	=	AsciiToDouble(valueString);
				UpdateFreeDiskSpace(gigabytesFree);
			}
			break;

		case kCmd_Camera_flip:
			//=================================================================================
			//*	flip mode
			cCameraProp.FlipMode	=	atoi(valueString);
			UpdateFlipMode();
			break;


		case kCmd_Camera_livemode:
			{
			bool	previousLiveMode;

				//=================================================================================
				//*	livemode
				previousLiveMode	=	cLiveMode;
				cLiveMode			=	IsTrueFalse(valueString);
				if (cLiveMode != previousLiveMode)
				{
					UpdateLiveMode();
				}
			}
			break;

		case kCmd_Camera_REFID:
			break;

		case kCmd_Camera_rgbarray:
			break;

		case kCmd_Camera_saveasFITS:
			cSaveAsFITS	=	IsTrueFalse(valueString);;
			break;

		case kCmd_Camera_saveasJPEG:
			cSaveAsJPEG	=	IsTrueFalse(valueString);;
			break;

		case kCmd_Camera_saveasPNG:
			cSaveAsPNG	=	IsTrueFalse(valueString);;
			break;

		case kCmd_Camera_saveasRAW:
			cSaveAsRaw	=	IsTrueFalse(valueString);;
			break;

		case kCmd_Camera_saveallimages:
			//=================================================================================
			//*	save all images
			cSaveAllImages	=	IsTrueFalse(valueString);
			break;

		case kCmd_Camera_savedimages:
			//=================================================================================
			//*	saved image count
			cCameraProp.SavedImageCnt	=	atoi(valueString);
			break;

		case kCmd_Camera_savenextimage:
			break;

		default:
			dataWasHandled	=	false;
			break;
	}
	return(dataWasHandled);
}


//*****************************************************************************
bool	ControllerCamera::AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
int		enumValue;
bool	dataWasHandled	=	false;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (strlen(keywordString) > 0)
	{
		enumValue		=	LookupStringInCmdTable(keywordString, gCameraCmdTable);
		if (enumValue >= 0)
		{
			dataWasHandled	=	AlpacaProcessReadAllIdx(deviceTypeStr, deviceNum, enumValue, valueString);
		}
		if (dataWasHandled == false)
		{
			if (strncasecmp(keywordString, "IMU", 3) == 0)
			{
				ProcessReadAll_IMU(deviceTypeStr, deviceNum, keywordString, valueString);
			}
			else if (strncasecmp(keywordString, "saveas", 6) == 0)
			{
				ProcessReadAll_SaveAs(deviceTypeStr, deviceNum, keywordString, valueString);
			}
			else if (strncasecmp(keywordString, "comment", 7) == 0)
			{
				//*	ignore comments
			}
			else
			{
//				CONSOLE_DEBUG_W_2STR("Not Found:",	keywordString, valueString);
				dataWasHandled	=	false;
			}
		}
	}
	else
	{
		dataWasHandled	=	false;
	}
	return(dataWasHandled);
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
TYPE_ASCOM_STATUS	ControllerCamera::ToggleCooler(void)
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
	if (cLastAlpacaErrNum != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG(cLastAlpacaErrStr);
	}
	return(cLastAlpacaErrNum);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ControllerCamera::SetCCDtargetTemperature(double newCCDtargetTemp)
{
char	dataString[48];
bool	validData;

	sprintf(dataString, "SetCCDTemperature=%3.3f", newCCDtargetTemp);
	CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
	validData	=	AlpacaSendPutCmd(	"camera", "SetCCDTemperature",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to set data, Req=", dataString)
	}
	if (cLastAlpacaErrNum != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG(cLastAlpacaErrStr);
	}
	return(cLastAlpacaErrNum);
}



//*****************************************************************************
void	ControllerCamera::StartExposure(void)
{
bool			validData;
SJP_Parser_t	jsonParser;
int				jjj;
char			parameterString[128];
int				returnedAlpacaErrNum;
char			returnedAlpacaErrStr[256];

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
		returnedAlpacaErrNum	=	0;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString)

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "filenameroot") == 0)
			{
				CONSOLE_DEBUG_W_STR("filenameroot\t=", jsonParser.dataList[jjj].valueString);
				UpdateReceivedFileName(jsonParser.dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "ERRORNUMBER") == 0)
			{
				returnedAlpacaErrNum	=	atoi(jsonParser.dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "ERRORMESSAGE") == 0)
			{
				strcpy(returnedAlpacaErrStr, jsonParser.dataList[jjj].valueString);
			}
		}
		if (returnedAlpacaErrNum != 0)
		{
			CONSOLE_DEBUG_W_STR("Error\t=", returnedAlpacaErrStr);
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
	CONSOLE_ABORT(__FUNCTION__);
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

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
cv::Mat	*ControllerCamera::DownloadImage_rgbarray(void)
{
	CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	return(NULL);
}
#else
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
		imageData	=	(int *)calloc(pixelsCount, sizeof(int));
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
#endif // _USE_OPENCV_CPP_

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
cv::Mat	*ControllerCamera::DownloadImage_imagearray(const bool force8BitRead, const bool allowBinary)
{
cv::Mat			*myOpenCVimage	=	NULL;
int				pixelCount;
int				valuesRead;
int				iii;
int				xxx;
int				yyy;
int				pixIdx;
int				pixIdxRowStart;
int				imgRank;
int				bytesPerPixel;
TYPE_ImageArray	*imageArray;
int				buffSize;
int				imageDataLen;
int				imageWidthStep;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM(	"cCameraProp.CameraXsize\t=",	cCameraProp.CameraXsize);
	CONSOLE_DEBUG_W_NUM(	"cCameraProp.CameraYsize\t=",	cCameraProp.CameraYsize);
	CONSOLE_DEBUG_W_BOOL(	"force8BitRead          \t=",	force8BitRead);
	CONSOLE_DEBUG_W_BOOL(	"allowBinary            \t=",	allowBinary);

	//*	set up default values for the binary image header in case we use JSON
	memset((void *)&cBinaryImageHdr, 0, sizeof(TYPE_BinaryImageHdr));
	cBinaryImageHdr.MetadataVersion			=	-1;
//	cBinaryImageHdr.ImageElementType		=	kAlpacaImageData_Byte;
//	cBinaryImageHdr.TransmissionElementType	=	kAlpacaImageData_Byte;
//	cBinaryImageHdr.Rank					=	2;
//	cBinaryImageHdr.Dimension1				=	cCameraProp.CameraXsize;
//	cBinaryImageHdr.Dimension2				=	cCameraProp.CameraYsize;
//	cBinaryImageHdr.Dimension3				=	0;

	imageArray		=	NULL;
	pixelCount		=	cCameraProp.CameraXsize * cCameraProp.CameraYsize;
	if (pixelCount > 0)
	{
		CONSOLE_DEBUG_W_NUM("pixelCount\t=", pixelCount);
		buffSize	=	(pixelCount + 100) * sizeof(TYPE_ImageArray);
		CONSOLE_DEBUG_W_NUM("buffSize\t=", buffSize);
		imageArray	=	(TYPE_ImageArray *)calloc(1, buffSize);
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
				if ((cBinaryImageHdr.ImageElementType == kAlpacaImageData_Int16) ||
					(cBinaryImageHdr.ImageElementType == kAlpacaImageData_UInt16))
				{
					bytesPerPixel	=	2;
					myOpenCVimage	=	new cv::Mat(	cBinaryImageHdr.Dimension2,	//*	Note, Height is FIRST
														cBinaryImageHdr.Dimension1,
														CV_16UC1);
				}
				else
				{
					bytesPerPixel	=	3;
					myOpenCVimage	=	new cv::Mat(	cCameraProp.CameraYsize,	//*	Note, Height is FIRST
														cCameraProp.CameraXsize,
														CV_8UC3);
				}

				if (myOpenCVimage != NULL)
				{
					imageWidthStep	=	myOpenCVimage->step[0];
					imageDataLen	=	myOpenCVimage->rows * imageWidthStep;
					if (myOpenCVimage->data != NULL)
					{
						memset(myOpenCVimage->data, 128, imageDataLen);
					}
					CONSOLE_DEBUG_W_NUM("width \t\t=",		myOpenCVimage->cols);
					CONSOLE_DEBUG_W_NUM("height\t\t=",		myOpenCVimage->rows);
					CONSOLE_DEBUG_W_LONG("widthStep\t=",	myOpenCVimage->step[0]);

					//*	move the image data into the openCV image structure
					START_TIMING();
					iii	=	0;
					//*	stepping ACROSS the field


					CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",		bytesPerPixel);

					for (xxx=0; xxx < myOpenCVimage->cols; xxx++)
					{
					//	CONSOLE_DEBUG_W_NUM("xxx \t\t=",		xxx);
						pixIdxRowStart	=	0;
						//*	stepping DOWN the column
						for (yyy=0; yyy < myOpenCVimage->rows; yyy++)
						{
							switch(bytesPerPixel)
							{
								case 2:
									pixIdx					=	pixIdxRowStart + (xxx * 2);
								#if (__ORDER_LITTLE_ENDIAN__ == 1234)
									myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].RedValue) & 0x00ff;
									myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].RedValue >> 8) & 0x00ff;
								#else
									#error "Not finished"
								#endif
									break;

								case 3:
									pixIdx	=	pixIdxRowStart + (xxx * 3);

									//*	openCV uses BGR instead of RGB
									//*	https://docs.opencv.org/master/df/d24/tutorial_js_image_display.html

									if (force8BitRead)
									{
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].BluValue) & 0x00ff;
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].GrnValue) & 0x00ff;
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].RedValue) & 0x00ff;
									}
									else
									{
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].BluValue >> 8) & 0x00ff;
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].GrnValue >> 8) & 0x00ff;
										myOpenCVimage->data[pixIdx++]	=	(imageArray[iii].RedValue >> 8) & 0x00ff;
									}
									break;
							}

							iii++;
							//*	advance to the next row
							pixIdxRowStart	+=	imageWidthStep;
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
#else
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
//		imageArray	=	(TYPE_ImageArray *)malloc(buffSize);
		imageArray	=	(TYPE_ImageArray *)calloc(1, buffSize);
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
#endif // _USE_OPENCV_CPP_

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
cv::Mat	*ControllerCamera::DownloadImage(const bool force8BitRead,  const bool allowBinary)
{
cv::Mat	*myOpenCVimage	=	NULL;

	myOpenCVimage	=	DownloadImage_imagearray(force8BitRead, allowBinary);

	return(myOpenCVimage);
}
#else
//*****************************************************************************
IplImage	*ControllerCamera::DownloadImage(const bool force8BitRead,  const bool allowBinary)
{
IplImage	*myOpenCVimage	=	NULL;

	myOpenCVimage	=	DownloadImage_imagearray(force8BitRead, allowBinary);

	return(myOpenCVimage);
}
#endif // _USE_OPENCV_CPP_


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
			//		NOTE:  The state we send is OPPOSITE of the current setting
			sprintf(dataString, "includefilter=%s",		(cFN.IncludeFilter ? "false" : "true"));
			break;

		case kCamSet_FN_IncCamera:
			sprintf(dataString, "includecamera=%s",		(cFN.IncludeManuf ? "false" : "true"));
			break;

		case kCamSet_FN_IncSerialNum:
			sprintf(dataString, "includeserialnum=%s",	(cFN.IncludeSerialNum ? "false" : "true"));
			break;

		case kCamSet_FN_IncRefID:
			sprintf(dataString, "includerefid=%s",	(cFN.IncludeRefID ? "false" : "true"));
			break;

	}
	validData	=	AlpacaSendPutCmd(	"camera", "filenameoptions",	dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get data, Req=", dataString)
	}
}


//*****************************************************************************
void	ControllerCamera::SetFlipMode(const int newFlipMode)
{
char	dataString[48];
bool	validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	sprintf(dataString, "flip=%d", newFlipMode);
	validData		=	AlpacaSendPutCmd(	"camera",
											"flip",
											dataString);
	if (validData == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to send data, Req=", dataString);
	}
	ForceAlpacaUpdate();
}

//*****************************************************************************
void	ControllerCamera::ToggleFlipMode(bool toggleHorz, bool toggleVert)
{
int		newFlipMode;

	newFlipMode	=	cCameraProp.FlipMode;
	if (toggleHorz)
	{
		newFlipMode	=	newFlipMode ^ 0x01;
	}
	if (toggleVert)
	{
		newFlipMode	=	newFlipMode ^ 0x02;
	}

	SetFlipMode(newFlipMode);
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
#define	_PARENT_IS_CAMERA_

#include "controller_fw_common.cpp"



#endif // _ENABLE_CTRL_CAMERA_
