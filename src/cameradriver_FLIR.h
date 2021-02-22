//**************************************************************************
//*	Name:			cameradriver_FLIR.h
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar 30,	2020	<MLS> Created cameradriver_FLIR.h
//*****************************************************************************
//#include	"cameradriver_FLIR.h"


#ifndef	_CAMERA_DRIVER_FLIR_H_
#define	_CAMERA_DRIVER_FLIR_H_


#ifndef _STDIO_H
	#include	<stdio.h>
#endif // _STDIO_H
#ifndef	_STRING_H
	#include	<string.h>
#endif // _STRING_H
#ifndef FLIR_SPINNAKER_C_H
	#include	<spinnaker/spinc/SpinnakerC.h>
#endif // FLIR_SPINNAKER_C_H
#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif


void	CreateFLIR_CameraObjects(void);




void	CreateFLIR_CameraObjects(void);


//**************************************************************************************
class CameraDriverFLIR: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverFLIR(spinCamera hCamera);

		virtual			~CameraDriverFLIR(void);
//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);

		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
//		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//		virtual	int						SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
//		virtual	int						SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

//		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Take_Video(void);

//		virtual	bool					GetImage_ROI_info(void);

//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
//		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);


	protected:
		int				ReadFLIRcameraInfo(void);
		spinError		SetFlirAqcuistionMode(int mode);
		int				ConvertToMono(void);
		int				ExtractColorImage(void);

		spinCamera			cSpinCameraHandle;
		spinImage			cSpinImageHandle;
		spinNodeMapHandle	cSpinNodeMapHandle;
		spinNodeMapHandle	cSpinNodeMapTLDeviceH;


};

extern char	gSpinakerVerString[];


#endif // _CAMERA_DRIVER_FLIR_H_
