//**************************************************************************
//*	Name:			cameradriver_ATIK.cpp
//*
//*	Author:			Mark Sproul (C) 2019, 2020
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
//*	Usage notes:
//*		Have to install 99-atik.rules into /lib/udev/rules.d/ and reboot
//*     Have to copy the appropriate library into /usr/lib
//*
//*	References:
//*		https://www.atik-cameras.com/wp-content/uploads/2019/11/AtikCamerasSDK-129.zip
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  5,	2019	<MLS> Created cameradriver_ATIK.cpp
//*	Oct  4,	2019	<MLS> Received Atik 460ex from Cloudy Nights
//*	Oct 20,	2019	<MLS> Copied ATIK library to /usr/lib
//*	Oct 20,	2019	<MLS> ATIK camera working partly on RaspberryPi
//*	Oct 20,	2019	<MLS> Have to install 99-atik.rules into /lib/udev/rules.d/ and reboot
//*	Nov  5,	2019	<MLS> Most of the ATIK cooler functions implemented, not yet tested
//*	Nov 16,	2019	<MLS> Live view working on ATIK camera
//*	Nov 25,	2019	<MLS> Added SetImageType() to ATIK driver
//*	Dec 15,	2019	<MLS> Added CheckATIKrulesFile()
//*	Dec 23,	2019	<MLS> Added ProcessATIKproperties()
//*	Dec 23,	2019	<MLS> Added ProcessATIKcoolingInfo()
//*	Dec 25,	2019	<MLS> Atik Titan camera working on Raspberry-Pi 4
//*	Feb 16,	2020	<MLS> Added Read_Gain() & Write_Gain()
//*	Mar 13,	2020	<MLS> Finished Read_CoolerState() for ATIK
//*	Apr 25,	2021	<MLS> Added check for camera devices (ArtemisDeviceIsCamera())
//*	Apr 15,	2022	<MLS> Installed ATIK 460ex on WO71 telescope, updating ATIK drivers
//*	Apr 16,	2022	<MLS> Fixed bug in GetImage_ROI_info()
//*	May 17,	2022	<MLS> Changed the way power level is reported
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ATIK_)

#include	<math.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"AtikCameras.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"cameradriver_ATIK.h"

#define	kMaxCameraCnt	5

//**************************************************************************************
void	CreateATIK_CameraObjects(void)
{
int		devNum;
int		atikDeviceCount;
bool	rulesOK;
char	driverVersionString[64];
int		atikVersionNum;
char	rulesFileName[]	=	"99-atik.rules";
bool	deviceIsCamera;

	atikVersionNum	=	ArtemisAPIVersion();
	sprintf(driverVersionString, "%d", atikVersionNum);
	LogEvent(	"camera",
				"Library version (ATIK)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);

	AddLibraryVersion("camera", "ATIK", driverVersionString);

	//*	check to make sure the rules file is present
	//	/lib/udev/rules.d/99-atik.rules
	rulesOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesOK != true)
	{
		CONSOLE_DEBUG("Problem with ATIK rules");
		LogEvent(	"camera",
					"Problem with ATIK rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	atikDeviceCount	=	ArtemisDeviceCount();
	CONSOLE_DEBUG_W_NUM("atikDeviceCount\t=", atikDeviceCount);
	for (devNum=0; devNum < atikDeviceCount; devNum++)
	{
		deviceIsCamera	=	ArtemisDeviceIsCamera(devNum);
		if (deviceIsCamera)
		{
			new CameraDriverATIK(devNum);
		}
	}
}


//**************************************************************************************
CameraDriverATIK::CameraDriverATIK(const int deviceNum)
	:CameraDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);
	hAtikCameraHandle	=	NULL;
	cCameraID			=	deviceNum;
	cATIKimageTypeIdx	=	0;
	strcpy(cDeviceManufAbrev,	"ATIK");

	ReadATIKcameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_
}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverATIK::~CameraDriverATIK(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}


//*****************************************************************************
void	CameraDriverATIK::ReadATIKcameraInfo(void)
{
int					atikRetCode;
int					tempSensorCnt;
ARTEMISPROPERTIES	atikProperties;
bool				isCamera;
bool				atikOK;
ARTEMISCOLOURTYPE	colourType;
int					normalOffsetX;
int					normalOffsetY;
int					previewOffsetX;
int					previewOffsetY;
char				tempDeviceName[64];

	printf("----------------------------------\r\n");
//	ArtemisAllowDebugToConsole(true);
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cCameraID\t=",	cCameraID);
	atikOK				=	ArtemisDeviceName(cCameraID, tempDeviceName);
	if (atikOK)
	{
		strcpy(cCommonProp.Name, tempDeviceName);
	}
	CONSOLE_DEBUG_W_STR("cCommonProp.Name=", cCommonProp.Name);
	LogEvent(	"camera",
				"ATIK Camera detected",
				NULL,
				kASCOM_Err_Success,
				cCommonProp.Name);

	CONSOLE_DEBUG_W_NUM("ATIK camera #\t=",	cCameraID);

	isCamera	=	ArtemisDeviceIsCamera(cCameraID);
	if (isCamera == false)
	{
		CONSOLE_DEBUG("Houston, we have a problem");
	}
	cSt4Port			=	ArtemisDeviceHasGuidePort(cCameraID);
	cBitDepth			=	16;
	cCameraProp.GainMin	=	0;
	cCameraProp.GainMax	=	512;	//*	information found on the web

	AddReadoutModeToList(kImageType_RAW16);

	atikOK	=	ArtemisDeviceSerial(cCameraID, cDeviceSerialNum);
	if (atikOK == false)
	{
		CONSOLE_DEBUG_W_STR("Failed to get serial number\t=",	cDeviceSerialNum);
	}

	cCameraProp.ExposureMax_us		=	999999999;
	cCameraProp.ExposureMax_seconds	=	999999999.9999;

	if (strcmp(cCommonProp.Name, "Atik Titan") == 0)
	{
		strcpy(cCameraProp.SensorName, "Sony ICX424");
		//*	https://www.atik-cameras.com/product/atik-titan/
		cCameraProp.ElectronsPerADU	=	0.34;
	}
	else if (strcmp(cCommonProp.Name, "Atik 460ex") == 0)
	{
		strcpy(cCameraProp.SensorName, "Sony ICX694");
		//*	https://www.atik-cameras.com/product/atik-460ex/
		cCameraProp.ElectronsPerADU	=	0.27;
		cCameraProp.ExposureMin_us	=	1000;		//*	1/1000 sec = 1000 micro seconds
	}
	else if (strcmp(cCommonProp.Name, "Atik Horizon") == 0)
	{
		strcpy(cCameraProp.SensorName, "Panasonic MN34230");
		//*	https://www.atik-cameras.com/product/atik-horizon/
		cBitDepth					=	12;
		cCameraProp.ElectronsPerADU	=	1;
		cCameraProp.ExposureMin_us	=	18;			//*	micro seconds
	}

	CONSOLE_DEBUG_W_NUM("ATIK camera #\t=",	cCameraID);
	hAtikCameraHandle	=	ArtemisConnect(cCameraID);
	if (hAtikCameraHandle != NULL)
	{
		memset(&atikProperties, 0x7f, sizeof(ARTEMISPROPERTIES));
		atikRetCode	=	ArtemisProperties(hAtikCameraHandle,	&atikProperties);


		//*	interpret the properties in the camera flag
		ProcessATIKproperties(&atikProperties);


		// get the maximum x,y binning factors
		cCameraProp.MaxbinX	=	1;
		cCameraProp.MaxbinY	=	1;
		atikRetCode	=	ArtemisGetMaxBin( hAtikCameraHandle, &cCameraProp.MaxbinX, &cCameraProp.MaxbinY);
		CONSOLE_DEBUG_W_NUM("cCameraProp.MaxbinX\t=", cCameraProp.MaxbinX);
		CONSOLE_DEBUG_W_NUM("cCameraProp.MaxbinY\t=", cCameraProp.MaxbinY);

		CONSOLE_DEBUG("ArtemisTemperatureSensorInfo");
		tempSensorCnt	=	0;
		atikRetCode		=	ArtemisTemperatureSensorInfo(hAtikCameraHandle, 0, &tempSensorCnt);
		CONSOLE_DEBUG_W_NUM("atikRetCode\t=", atikRetCode);
		CONSOLE_DEBUG_W_NUM("tempSensorCnt\t=", tempSensorCnt);


		atikRetCode	=	ProcessATIKcoolingInfo();


		CONSOLE_DEBUG("ArtemisColourProperties-----------------------");
		atikRetCode		=	ArtemisColourProperties(hAtikCameraHandle,
													&colourType,
													&normalOffsetX,
													&normalOffsetY,
													&previewOffsetX,
													&previewOffsetY);
		CONSOLE_DEBUG_W_NUM("atikRetCode\t=",		atikRetCode);
		CONSOLE_DEBUG_W_NUM("colourType\t\t=",		colourType);
		CONSOLE_DEBUG_W_NUM("normalOffsetX\t=",		normalOffsetX);
		CONSOLE_DEBUG_W_NUM("normalOffsetY\t=",		normalOffsetY);
		CONSOLE_DEBUG_W_NUM("previewOffsetX\t=",	previewOffsetX);
		CONSOLE_DEBUG_W_NUM("previewOffsetY\t=",	previewOffsetY);
		if (colourType == ARTEMIS_COLOUR_RGGB)
		{
			AddReadoutModeToList(kImageType_RGB24);
			cIsColorCam	=	true;
		}
		else
		{
			cIsColorCam	=	false;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Failed to open ATIK camera #\t=",	cCameraID);
	}
}

//*****************************************************************************
void	CameraDriverATIK::ProcessATIKproperties(ARTEMISPROPERTIES	*atikProperties)
{
	CONSOLE_DEBUG_W_NUM("Protocol\t\t=", atikProperties->Protocol);


	cCameraProp.CameraXsize		=	atikProperties->nPixelsX;
	cCameraProp.CameraYsize		=	atikProperties->nPixelsY;
	cCameraProp.PixelSizeX		=	atikProperties->PixelMicronsX;
	cCameraProp.PixelSizeY		=	atikProperties->PixelMicronsY;
	strcpy(cCommonProp.Description,	atikProperties->Description);
	strcpy(cDeviceManufacturer,	atikProperties->Manufacturer);

	cCameraProp.NumX	=	cCameraProp.CameraXsize;
	cCameraProp.NumY	=	cCameraProp.CameraYsize;

	CONSOLE_DEBUG_W_HEX("cameraflags\t=", atikProperties->cameraflags);

	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO)
	{
		// Camera has readout FIFO fitted
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER)
	{
		// Camera has external trigger capabilities
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW)
	{
		// Camera can return preview data
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE)
	{
		// Camera can return subsampled data
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_SHUTTER)
	{
		// Camera has a mechanical shutter
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_SHUTTER");
		cCameraProp.HasShutter	=	true;
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GUIDE_PORT)
	{
		// Camera has a guide port
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GUIDE_PORT");
		cSt4Port	=	true;
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GPIO)
	{
		// Camera has GPIO capability
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GPIO");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_WINDOW_HEATER)
	{
	// Camera has a window heater
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_WINDOW_HEATER");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_EIGHT_BIT_MODE)
	{
		// Camera can download 8-bit images
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_EIGHT_BIT_MODE");
		cCanRead8Bit	=	true;
		AddReadoutModeToList(kImageType_RAW8);
	}
	else
	{
		cCanRead8Bit	=	false;
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_OVERLAP_MODE)
	{
		// Camera can overlap
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_OVERLAP_MODE");
	}
	if (atikProperties->cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_FILTERWHEEL)
	{
		// Camera has internal filterwheel
		CONSOLE_DEBUG("ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_FILTERWHEEL");
	}
}

//*****************************************************************************
int	CameraDriverATIK::ProcessATIKcoolingInfo(void)
{
int		atikRetCode;
int		coolingFlags;
int		level;
int		minlvl;
int		maxlvl;
int		setpoint;

//	CONSOLE_DEBUG("ArtemisSetCooling--------------------------");
//	atikRetCode		=	ArtemisSetCooling(hAtikCameraHandle, 0);
//	CONSOLE_DEBUG_W_NUM("atikRetCode\t=", atikRetCode);

	CONSOLE_DEBUG("ArtemisCoolingInfo-------------------------");
	atikRetCode		=	ArtemisCoolingInfo(	hAtikCameraHandle,
											&coolingFlags,
											&level,
											&minlvl,
											&maxlvl,
											&setpoint);

	CONSOLE_DEBUG_W_NUM("atikRetCode\t=",	atikRetCode);
	CONSOLE_DEBUG_W_NUM("coolingFlags\t=",	coolingFlags);
	CONSOLE_DEBUG_W_HEX("coolingFlags\t=",	coolingFlags);
	CONSOLE_DEBUG_W_NUM("level\t\t=",		level);
	CONSOLE_DEBUG_W_NUM("minlvl\t\t=",		minlvl);
	CONSOLE_DEBUG_W_NUM("maxlvl\t\t=",		maxlvl);
	CONSOLE_DEBUG_W_NUM("setpoint\t\t=",	setpoint);

	//	Info flags
	//	b5-7 report what�s actually happening
	if (coolingFlags & ARTEMIS_COOLING_INFO_HASCOOLING)
	{
		//	b0	0 = no cooling 1=coolingArtemisCoolingInfo
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_HASCOOLING");
		cIsCoolerCam	=	true;
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_CONTROLLABLE)
	{
		//	b1	0 = always on 1= controllable
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_CONTROLLABLE");
		cCameraProp.CanGetCoolerPower		=	true;
		cCameraProp.Cansetccdtemperature	=	true;
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_ONOFFCOOLINGCONTROL)
	{
		//	b2	0 = on/off control not available  1= on off cooling control
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_ONOFFCOOLINGCONTROL");
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_POWERLEVELCONTROL)
	{
		//	b3	0 = no selectable power levels 1= selectable power levels
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_POWERLEVELCONTROL");
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_SETPOINTCONTROL)
	{
		//	b4	0 = no temperature set point cooling 1= set point cooling
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_SETPOINTCONTROL");
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_WARMINGUP)
	{
		//	b5	0 =normal control 1=warming up
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_WARMINGUP");
	}
	if (coolingFlags & ARTEMIS_COOLING_INFO_COOLINGON)
	{
		//	b6	0 =cooling off   1=cooling on
		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_COOLINGON");
	}
//	if (coolingFlags & ARTEMIS_COOLING_INFO_SETPOINTCONTROL)
//	{
//		//	b7	0 = no set point control 1=set point control*/
//		CONSOLE_DEBUG("ARTEMIS_COOLING_INFO_SETPOINTCONTROL");
//	}

	return(atikRetCode);
}

//*****************************************************************************
//*	the camera must already be open when this is called
//*****************************************************************************
bool	CameraDriverATIK::GetImage_ROI_info(void)
{
	memset(&cROIinfo, 0, sizeof(TYPE_IMAGE_ROI_Info));

	cROIinfo.currentROIimageType	=	kImageType_RAW16;
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;

	return(true);
}


//*****************************************************************************
//*	http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure Content-Type: -dDuration=0.001&Light=true
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					exposureTime_ms;
int					atikRetCode;
//int		atikCameraState;

	CONSOLE_DEBUG(__FUNCTION__);

	if (hAtikCameraHandle != NULL)
	{
		cCurrentExposure_us	=	exposureMicrosecs;
		exposureTime_ms		=	exposureMicrosecs / 1000;
//		CONSOLE_DEBUG_W_NUM("exposureTime_ms\t=",		exposureTime_ms);


		gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);
		cCameraProp.Lastexposure_duration_us	=	exposureMicrosecs;
		CONSOLE_DEBUG("Calling ArtemisStartExposureMS");
		atikRetCode					=	ArtemisStartExposureMS(hAtikCameraHandle, exposureTime_ms);
		if (atikRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG("Exposure started");
			alpacaErrCode			=	kASCOM_Err_Success;
			cInternalCameraState	=	kCameraState_TakingPicture;
		//	atikCameraState			=	ArtemisCameraState(hAtikCameraHandle);

			SetLastExposureInfo();
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			sprintf(cLastCameraErrMsg, "ATIK failed to start exposure, err=%d", atikRetCode);
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		strcpy(cLastCameraErrMsg, "ATIK camera not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	CONSOLE_DEBUG("exit");

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);

	return(alpacaErrCode);
}


static int		gPrevDownloadPercent	=	-1;

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverATIK::Check_Exposure(bool verboseFlag)
{
//int						atikRetCode;
TYPE_EXPOSURE_STATUS	exposureState;
int						atikCameraState;
bool					imageIsReady;
int						downloadPercent;

//	CONSOLE_DEBUG(__FUNCTION__);
	exposureState	=	kExposure_Idle;
	if (hAtikCameraHandle != NULL)
	{
		atikCameraState	=	ArtemisCameraState(hAtikCameraHandle);
//		CONSOLE_DEBUG_W_NUM("atikCameraState\t=",		atikCameraState);
		switch(atikCameraState)
		{
			case CAMERA_ERROR:
				exposureState	=	kExposure_Failed;
				break;

			case CAMERA_IDLE:
				imageIsReady	=	ArtemisImageReady(hAtikCameraHandle);
//				CONSOLE_DEBUG_W_NUM("imageIsReady\t=",			imageIsReady);
				if (imageIsReady)
				{
					exposureState	=	kExposure_Success;
				}
				else
				{
					exposureState	=	kExposure_Idle;
				}
				break;

			case CAMERA_WAITING:
			case CAMERA_EXPOSING:
			case CAMERA_READING:
			case CAMERA_FLUSHING:
				exposureState	=	kExposure_Working;
				//*	this should probably go some place else.
				gettimeofday(&cCameraProp.Lastexposure_EndTime, NULL);
				break;

			case CAMERA_DOWNLOADING:
				downloadPercent	=	ArtemisDownloadPercent(hAtikCameraHandle);
				imageIsReady	=	ArtemisImageReady(hAtikCameraHandle);
				if (downloadPercent != gPrevDownloadPercent)
				{
					CONSOLE_DEBUG_W_NUM("downloadPercent (from camera to driver)\t=",		downloadPercent);
//					CONSOLE_DEBUG_W_NUM("imageIsReady\t=",			imageIsReady);
					gPrevDownloadPercent	=	downloadPercent;
				}
				if (imageIsReady)
				{
					CONSOLE_DEBUG("Image is ready");
					exposureState	=	kExposure_Success;
				}
				else
				{
					exposureState	=	kExposure_Working;
				}
				break;
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
	}
	return(exposureState);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;
bool				eightbit;

	CONSOLE_DEBUG(__FUNCTION__);

	if (hAtikCameraHandle != NULL)
	{
		switch(newImageType)
		{
			case kImageType_RAW8:
				// set the 8-bit imaging mode
				if (cCanRead8Bit)
				{
					CONSOLE_DEBUG("Setting to 8 bit mode");
					eightbit	=	true;
					atikRetCode	=	ArtemisEightBitMode(hAtikCameraHandle, eightbit);
					CONSOLE_DEBUG_W_NUM("atikRetCode\t=",		atikRetCode);
				}
				else
				{
					alpacaErrCode	=	kASCOM_Err_NotSupported;
					strcpy(cLastCameraErrMsg, "This camera does not support 8 bit readout");
					CONSOLE_DEBUG(cLastCameraErrMsg);
				}
				break;

			case kImageType_RAW16:
				if (cCanRead8Bit)
				{
					eightbit	=	false;
					atikRetCode	=	ArtemisEightBitMode(hAtikCameraHandle, eightbit);
					if (atikRetCode == ARTEMIS_OK)
					{
						alpacaErrCode	=	kASCOM_Err_Success;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("atikRetCode\t=",		atikRetCode);
						alpacaErrCode	=	kASCOM_Err_CameraDriverErr;
						strcpy(cLastCameraErrMsg, "ArtemisEightBitMode returned error");
					}
				}
				else
				{
					//*	8 bit not supported, so no need to set back to 16
					alpacaErrCode	=	kASCOM_Err_Success;
				}
				break;

			case kImageType_RGB24:
				if (cIsColorCam)
				{
				}
				else
				{
					alpacaErrCode	=	kASCOM_Err_NotSupported;
					strcpy(cLastCameraErrMsg, "This camera does not support color");
					CONSOLE_DEBUG(cLastCameraErrMsg);
				}
				break;

			case kImageType_Y8:
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				break;

			default:
				alpacaErrCode	=	kASCOM_Err_InternalError;
				break;
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					myGainValue;
int					myOffsetValue;
int					atikRetCode;
bool				isPreview;

	if (hAtikCameraHandle != NULL)
	{
		//*	first get the values so we can keep the offset
		isPreview	=	false;
		atikRetCode	=	ArtemisGetGain(hAtikCameraHandle, isPreview, &myGainValue, &myOffsetValue);

		if (atikRetCode == ARTEMIS_OK)
		{
			myGainValue	=	newGainValue;
			atikRetCode	=	ArtemisSetGain(hAtikCameraHandle, isPreview, myGainValue, myOffsetValue);
			if (atikRetCode == ARTEMIS_OK)
			{
				alpacaErrCode		=	kASCOM_Err_Success;
			}
			else
			{
				strcpy(cLastCameraErrMsg, "Failed to set gain");
				CONSOLE_DEBUG(cLastCameraErrMsg);
				alpacaErrCode	=	kASCOM_Err_DataFailure;
			}
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to get gain (prior to setting)");
			CONSOLE_DEBUG(cLastCameraErrMsg);
			CONSOLE_DEBUG_W_NUM("atikRetCode=", atikRetCode);
			alpacaErrCode	=	kASCOM_Err_DataFailure;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					myGainValue;
int					myOffsetValue;
int					atikRetCode;
bool				isPreview;

	if (hAtikCameraHandle != NULL)
	{
		isPreview	=	false;
		atikRetCode	=	ArtemisGetGain(hAtikCameraHandle, isPreview, &myGainValue, &myOffsetValue);

		if (atikRetCode == ARTEMIS_OK)
		{
			alpacaErrCode		=	kASCOM_Err_Success;
			*cameraGainValue	=	myGainValue;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to read gain");
//			CONSOLE_DEBUG(cLastCameraErrMsg);
			alpacaErrCode	=	kASCOM_Err_DataFailure;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


#ifdef TEMP_DISABLE
#pragma mark -
#pragma mark Movie commands

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Start_Movie(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;


	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Stop_Movie(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;


	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	return(alpacaErrCode);

}

#endif	//	TEMP_DISABLE


#pragma mark -
#pragma mark Virtual functions
//*****************************************************************************
//	Set Cooling:
//	int ArtemisSetCooling(ArtemisHandle hCam, int setpoint);
//	This function is used to set the temperature of the camera. The setpoint is in 1/100 of a degree
//	(Celcius). So, to set the cooling to -10C, you need to call the function with setpoint = -1000.
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cIsCoolerCam)
	{
		//*	we have a cooler, see what its doing
		CONSOLE_DEBUG("we have a cooler, see what its doing");
		CONSOLE_DEBUG("ArtemisSetCooling--------------------------");
		if (hAtikCameraHandle != NULL)
		{
			atikRetCode		=	ArtemisSetCooling(hAtikCameraHandle, 0);
			if (atikRetCode == ARTEMIS_OK)
			{
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("atikRetCode\t=", atikRetCode);
				sprintf(cLastCameraErrMsg, "ArtemisSetCooling failed with err %d", atikRetCode);
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "This ATIK Camera does not support cooling");
		alpacaErrCode	=	kASCOM_Err_NotSupported;
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	if (hAtikCameraHandle != NULL)
	{
		atikRetCode		=	ArtemisSetCooling(hAtikCameraHandle, 100);
		atikRetCode		=	ArtemisSetCoolingPower(hAtikCameraHandle, 0);
		atikRetCode		=	ArtemisCoolerWarmUp(hAtikCameraHandle);
		if (atikRetCode == ARTEMIS_OK)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_DataFailure;
			strcpy(cLastCameraErrMsg, "ArtemisCoolerWarmUp returned error");
			CONSOLE_DEBUG_W_NUM("ArtemisCoolerWarmUp returned atikRetCode\t=", atikRetCode);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
	}
	return(alpacaErrCode);
}


//**************************************************************************
//*	returns error code,
//*	sets class varible to current temp
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					tempSensorCnt;
int					atikRetCode;
int					temperatureVal;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (hAtikCameraHandle != NULL)
	{
		tempSensorCnt	=	0;
		atikRetCode		=	ArtemisTemperatureSensorInfo(hAtikCameraHandle, 0, &tempSensorCnt);
		if (atikRetCode == ARTEMIS_OK)
		{
			if (tempSensorCnt > 0)
			{
	//			CONSOLE_DEBUG_W_NUM("tempSensorCnt\t=", tempSensorCnt);
				temperatureVal	=	0;
				atikRetCode		=	ArtemisTemperatureSensorInfo(hAtikCameraHandle, 1, &temperatureVal);
				if (atikRetCode == ARTEMIS_OK)
				{
					cCameraProp.CCDtemperature	=	(temperatureVal * 1.0) / 100.0;
//					CONSOLE_DEBUG_W_DBL("cCameraProp.CCDtemperature\t=", cCameraProp.CCDtemperature);
					alpacaErrCode	=	kASCOM_Err_Success;
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("atikRetCode\t=", atikRetCode);
					CONSOLE_DEBUG_W_NUM("temperatureVal\t=", temperatureVal);
				}
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_NotSupported;
				strcpy(cLastCameraErrMsg, "Camera does not have a temperature sensor");
				CONSOLE_DEBUG(cLastCameraErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_DataFailure;
			CONSOLE_DEBUG_W_NUM("atikRetCode\t=", atikRetCode);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_CoolerState(bool *coolerOnOff)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;
int					coolingFlags;
int					level;
int					minlvl;
int					maxlvl;
int					setpoint;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cIsCoolerCam)
	{
		if (hAtikCameraHandle != NULL)
		{
			//*	we have a cooler, see what its doing
//			CONSOLE_DEBUG("ArtemisCoolingInfo-------------------------");
			atikRetCode		=	ArtemisCoolingInfo(	hAtikCameraHandle,
													&coolingFlags,
													&level,
													&minlvl,
													&maxlvl,
													&setpoint);
			if (atikRetCode == ARTEMIS_OK)
			{
				//	b6	0 =cooling off   1=cooling on
				if (coolingFlags & ARTEMIS_COOLING_INFO_COOLINGON)
				{
//					CONSOLE_DEBUG("Cooler is ON");
					*coolerOnOff	=	true;
				}
				else
				{
//					CONSOLE_DEBUG("Cooler is OFF");
					*coolerOnOff	=	false;
				}
				alpacaErrCode		=	kASCOM_Err_Success;
			}
			else if (atikRetCode == ARTEMIS_NOT_IMPLEMENTED)
			{
				//*	ignore this
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ArtemisCoolingInfo returned, atikRetCode=", atikRetCode);
			}
		}
	}
	else
	{
		*coolerOnOff	=	false;
		strcpy(cLastCameraErrMsg, "This ATIK Camera does not support cooling");
		alpacaErrCode	=	kASCOM_Err_NotSupported;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;
int					coolingFlags;
int					myCoolerPowerLevel;
int					minlvl;
int					maxlvl;
int					setpoint;

	myCoolerPowerLevel	=	0;
	if (cCameraProp.CanGetCoolerPower)
	{
		if (hAtikCameraHandle != NULL)
		{
			atikRetCode		=	ArtemisCoolingInfo(	hAtikCameraHandle,
													&coolingFlags,
													&myCoolerPowerLevel,
													&minlvl,
													&maxlvl,
													&setpoint);
			if (atikRetCode == ARTEMIS_OK)
			{
				cCameraProp.CoolerPower	=	myCoolerPowerLevel;
				alpacaErrCode			=	kASCOM_Err_Success;
			}
			else
			{
				strcpy(cLastCameraErrMsg, "ATIK Camera err=");

			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "This ATIK camera cannot get cooler power level");
		alpacaErrCode	=	kASCOM_Err_NotSupported;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_Fastreadout(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverATIK::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					atikRetCode;
bool				imageIsReady;
int					atikCameraState;
unsigned char		*atikImageBuffer;
int					imageStartX;
int					imageStartY;
int					imageWith;
int					imageHeight;
int					binx;
int					biny;
int					atikImageSize;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (hAtikCameraHandle != NULL)
	{
		atikCameraState	=	ArtemisCameraState(hAtikCameraHandle);
		if (atikCameraState != CAMERA_IDLE)
		{
			CONSOLE_DEBUG_W_NUM("atikCameraState\t=",		atikCameraState);
		}

		imageIsReady	=	ArtemisImageReady(hAtikCameraHandle);
//		CONSOLE_DEBUG_W_NUM("imageIsReady\t=",		imageIsReady);
		if (imageIsReady)
		{
			gettimeofday(&cDownloadStartTime, NULL);

			atikRetCode		=	ArtemisGetImageData(hAtikCameraHandle,
													&imageStartX,
													&imageStartY,
													&imageWith,
													&imageHeight,
													&binx,
													&biny);
			if (atikRetCode == ARTEMIS_OK)
			{
				atikImageBuffer	=	(unsigned char *)ArtemisImageBuffer(hAtikCameraHandle);
				gettimeofday(&cDownloadEndTime, NULL);
				if (atikImageBuffer != NULL)
				{
	//				CONSOLE_DEBUG_W_NUM("exposure start\t=",	cCameraProp.Lastexposure_StartTime.tv_sec);
	//				CONSOLE_DEBUG_W_NUM("download start\t=",	downloadStartTime.tv_sec);
	//				CONSOLE_DEBUG_W_NUM("download end\t=",		downloadEndTime.tv_sec);


	//				CONSOLE_DEBUG_W_NUM("imageStartX\t=",		imageStartX);
	//				CONSOLE_DEBUG_W_NUM("imageStartY\t=",		imageStartY);
	//				CONSOLE_DEBUG_W_NUM("imageWith\t\t=",		imageWith);
	//				CONSOLE_DEBUG_W_NUM("imageHeight\t=",		imageHeight);
	//				CONSOLE_DEBUG_W_NUM("binx\t\t=",			binx);
	//				CONSOLE_DEBUG_W_NUM("biny\t\t=",			biny);
					atikImageSize	= imageWith * imageHeight;
	//				CONSOLE_DEBUG_W_NUM("atikImageSize\t=",		atikImageSize);

				//	CONSOLE_DEBUG(__FUNCTION__);
					AllcateImageBuffer(-1);		//*	let it figure out how much
					if (cCameraDataBuffer != NULL)
					{
					//	CONSOLE_DEBUG_W_INT32("cCameraDataBuffLen\t=", cCameraDataBuffLen);
					//	memcpy(cCameraDataBuffer, atikImageBuffer, cCameraDataBuffLen);
						memcpy(cCameraDataBuffer, atikImageBuffer, (atikImageSize * 2));
					//	CONSOLE_DEBUG(__FUNCTION__);

						//--------------------------------------------
						if (gSimulateCameraImage)
						{
							//*	simulate image
							CreateFakeImageData(cCameraDataBuffer, imageWith, imageHeight, 2);
						}
					}
				}
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
				CONSOLE_DEBUG_W_NUM("ArtemisGetImageData returned\t=",		atikRetCode);
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "hAtikCameraHandle == NULL");
	}
	return(alpacaErrCode);
}





#endif	//	defined(_ENABLE_CAMERA_) && defined(_ENABLE_ATIK_)
