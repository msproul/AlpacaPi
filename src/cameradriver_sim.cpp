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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 22,	2022	<MLS> Created cameradriver_sim.cpp
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_SIMULATE_CAMERA_)

#include	<string>


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
void	CreateSimulator_CameraObjects(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	//*	for debugging without a camera
	new CameraDriverSIM(0);
}

//**************************************************************************************
CameraDriverSIM::CameraDriverSIM(	const int	deviceNum)
					:CameraDriver()
{
bool		isConnected;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("Creating Simulation device number ", deviceNum);

	cCameraID				=	deviceNum;

	//*	set defaults
	strcpy(cDeviceManufAbrev,		"AlpacaPi Simulator");
	strcpy(cCommonProp.Name,		"AlpacaPi Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi Simulator");
	strcpy(cCommonProp.Name,		"AlpacaPi Simulator");


	cTempReadSupported	=	true;
	//*	set some defaults for testing
	strcpy(cDeviceManufacturer,	"AlpacaPi");

//	cCameraProp.CameraXsize	=	9576;
//	cCameraProp.CameraYsize	=	6388;

	//*	In order to simulate a particular image size
	cCameraProp.CameraXsize	=	1500;
	cCameraProp.CameraYsize	=	1000;
	while ((cCameraProp.CameraXsize * cCameraProp.CameraYsize) < (32 * 1000000))
	{
		cCameraProp.CameraXsize	+=	100;
		cCameraProp.CameraYsize	=	cCameraProp.CameraXsize / 1.5;
	}


	cCameraProp.NumX		=	cCameraProp.CameraXsize;
	cCameraProp.NumY		=	cCameraProp.CameraYsize;

	cCameraProp.GainMin		=	0;
	cCameraProp.GainMax		=	10;

	AddReadoutModeToList(kImageType_RAW8);
	AddReadoutModeToList(kImageType_RAW16);
	AddReadoutModeToList(kImageType_RGB24);

	DumpCameraProperties(__FUNCTION__);

	isConnected		=	AlpacaConnect();
	if (isConnected)
	{

	}
	else
	{
		CONSOLE_DEBUG("Failed to Connect!!!!!!!!");
	}


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
double				cameraTemp_DegC;

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		//	Only valid if CanSetCCDTemperature is true.
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotSupported;
		strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;

	CONSOLE_DEBUG_W_NUM(__FUNCTION__, newGainValue);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSIM::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
//double				rawGainValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverSIM::Start_CameraExposure(int32_t exposureMicrosecs)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
double				durationSeconds;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCommonProp.Connected)
	{
		cCameraProp.ImageReady		=	false;

		durationSeconds	=	(exposureMicrosecs * 1.0) / 1000000.0;

		CONSOLE_DEBUG("Simulating camera");
		alpacaErrCode			=	kASCOM_Err_Success;
		cInternalCameraState	=	kCameraState_TakingPicture;
		SetLastExposureInfo();
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
bool					imageReady;

	//--------------------------------------------
	//*	simulate image
	myExposureStatus	=	kExposure_Success;

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
int					xSize;
int					ySize;
int					elementSize;
long				imgBufferSize;
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

		AllcateImageBuffer(-1);		//*	let it figure out how much
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
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


#endif // defined(_ENABLE_CAMERA_) && defined(_SIMULATE_CAMERA_)
