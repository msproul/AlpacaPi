//**************************************************************************
//*	Name:			cameradriver_sim.cpp
//*
//*	Author:			Mark Sproul (C) 2022
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 22,	2022	<MLS> Created cameradriver_sim.cpp
//*	Mar  4,	2023	<MLS> CONFORMU-camera/simulator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jun 18,	2023	<MLS> Added Read_CoolerPowerLevel()
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_CAMERA_SIMULATOR_)

#include	<stdlib.h>
#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"cameradriver.h"
#include	"cameradriver_sim.h"
#include	"linuxerrors.h"


//**************************************************************************************
int	CreateCameraObjects_Sim(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	//*	for debugging without a camera
	new CameraDriverSIM(0);
	return(1);
}

//**************************************************************************************
CameraDriverSIM::CameraDriverSIM(	const int	deviceNum)
					:CameraDriver()
{
bool		isConnected;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("Creating Simulation device number ", deviceNum);

//	gVerbose					=	true;
//	cVerboseDebug				=	true;

	cCameraID					=	deviceNum;
	cCameraIsSiumlated			=	true;
	cSimulatedState				=   kExposure_Idle;
	cIsColorCam					=	true;
	cIsCoolerCam				=	true;
	strcpy(cDeviceManufAbrev,		"SIM");
	strcpy(cCommonProp.Name,		"AlpacaPi Camera Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi Camera Simulator");
	strcpy(cCameraProp.SensorName,	"Fake-ASI2600");


	cTempReadSupported		=	true;
	cOffsetSupported		=	true;
	cBitDepth				=	8;
	//*	set some defaults for testing
	strcpy(cDeviceManufacturer,	"AlpacaPi");


	//*	In order to simulate a particular image size
#ifdef __arm__
	cCameraProp.CameraXsize	=	1000;
	cCameraProp.CameraYsize	=	800;
#else
	cCameraProp.CameraXsize	=	2500;
	cCameraProp.CameraYsize	=	2000;
#endif
	//*	faking an ASI2600
//	cCameraProp.CameraXsize	=	6248;
//	cCameraProp.CameraYsize	=	4176;


	cCameraProp.SensorType			=   kSensorType_RGGB;
	cCameraProp.NumX				=	cCameraProp.CameraXsize;
	cCameraProp.NumY				=	cCameraProp.CameraYsize;

	cCameraProp.GainMin				=	0;
	cCameraProp.GainMax				=	10;
	cCameraProp.ElectronsPerADU		=	65000;
	cCameraProp.PixelSizeX			=	3.76;
	cCameraProp.PixelSizeY			=	3.76;
	cCameraProp.FullWellCapacity	=	50000;

	AddReadoutModeToList(kImageType_RAW8);
	AddReadoutModeToList(kImageType_RAW16);
	AddReadoutModeToList(kImageType_RGB24);

	SetImageType(kImageType_RGB24);

	DumpCameraProperties(__FUNCTION__);

	isConnected		=	AlpacaConnect();
	if (isConnected)
	{

	}
	else
	{
		CONSOLE_DEBUG("Failed to Connect!!!!!!!!");
	}

	DumpCameraProperties(__FUNCTION__);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

}



//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverSIM::~CameraDriverSIM(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
bool	CameraDriverSIM::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cCommonProp.Connected	=	true;
	return(cCommonProp.Connected);
}


//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		//	Only valid if CanSetCCDTemperature is true.
		cCameraProp.CCDtemperature		=	5.56;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
	}
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCameraProp.CoolerPower		=	45.0;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Write_BinX(const int newBinXvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCameraProp.BinX	=	newBinXvalue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Write_BinY(const int newBinYvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCameraProp.BinY	=	newBinYvalue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	*cameraGainValue	=	cCameraProp.Gain;
	alpacaErrCode		=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	cCameraProp.Gain	=	newGainValue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_Offset(int *cameraOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	*cameraOffsetValue	=	cCameraProp.Offset;
	alpacaErrCode		=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Write_Offset(const int newOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	cCameraProp.Offset	=	newOffsetValue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverSIM::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
//double				durationSeconds;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCommonProp.Connected)
	{
		cCameraProp.ImageReady		=	false;

//		durationSeconds	=	(exposureMicrosecs * 1.0) / 1000000.0;
//		durationSeconds	=	2;

		CONSOLE_DEBUG("Simulating camera");
		cInternalCameraState	=	kCameraState_TakingPicture;
		cCameraProp.CameraState	=   kALPACA_CameraState_Exposing;
		SetLastExposureInfo();
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
bool	CameraDriverSIM::GetImage_ROI_info(void)
{
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;
	return(true);
}

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverSIM::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	myExposureStatus;
struct timeval			currentTIme;
time_t					deltaTime_secs;

	//--------------------------------------------
	//*	simulate image
	switch(cInternalCameraState)
	{
		case kCameraState_TakingPicture:
			myExposureStatus		=	kExposure_Working;
			gettimeofday(&currentTIme, NULL);	//*	get the current time
			deltaTime_secs	=	currentTIme.tv_sec - cCameraProp.Lastexposure_StartTime.tv_sec;

//			CONSOLE_DEBUG_W_LONG("deltaTime_secs\t=",			deltaTime_secs);
			if (deltaTime_secs > 2)
			{
				CONSOLE_DEBUG("Not kCameraState_TakingPicture -->> kCameraState_Idle");
				myExposureStatus		=	kExposure_Success;
			}
			break;

		default:
	//		myExposureStatus		=	kExposure_Idle;
			myExposureStatus		=	kExposure_Success;
			break;

	}
	if (gVerbose)
	{
		CONSOLE_DEBUG_W_NUM("myExposureStatus\t=",			myExposureStatus);
	}

	return(myExposureStatus);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("newImageType\t=",			newImageType);

	if (cCommonProp.Connected)
	{
		cROIinfo.currentROIwidth	=	cCameraProp.CameraXsize;
		cROIinfo.currentROIheight	=	cCameraProp.CameraYsize;
		cROIinfo.currentROIbin		=   1;

		switch(newImageType)
		{
			case kImageType_RAW8:
				cROIinfo.currentROIimageType	=	kImageType_RAW8;
				break;

			case kImageType_RAW16:
				cROIinfo.currentROIimageType	=	kImageType_RAW16;
				break;

			case kImageType_RGB24:
				cROIinfo.currentROIimageType	=	kImageType_RGB24;
				break;

			case kImageType_Y8:
				cROIinfo.currentROIimageType	=	kImageType_Y8;
				break;

			case kImageType_MONO8:
				cROIinfo.currentROIimageType	=	kImageType_MONO8;
				break;

			case kImageType_Invalid:
			case kImageType_last:
				break;

		}
	}
	else
	{
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					bytesPerPixel;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cCommonProp.Connected)
	{
		CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",			cROIinfo.currentROIimageType);
		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:	bytesPerPixel	=	1;	break;
			case kImageType_RAW16:	bytesPerPixel	=	2;	break;
			case kImageType_RGB24:	bytesPerPixel	=	3;	break;
			default:				bytesPerPixel	=	3;	break;
		}
		CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",			bytesPerPixel);

		AllocateImageBuffer(-1);		//*	let it figure out how much
		if (cCameraDataBuffer != NULL)
		{
			//--------------------------------------------
			CreateFakeImageData(cCameraDataBuffer, cCameraProp.CameraXsize, cCameraProp.CameraYsize, bytesPerPixel);
			cCameraProp.ImageReady	=	true;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_ABORT("Failed to allocate image buffer");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG("Not connected");
		CONSOLE_ABORT("Not connected");
	}
	return(alpacaErrCode);
}


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_CAMERA_SIMULATOR_)
