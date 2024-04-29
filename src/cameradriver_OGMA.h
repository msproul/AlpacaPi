//**************************************************************************
//*	Name:			cameradriver_OGMA.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 24,	2024	<MLS> Created cameradriver_OGMA.cpp
//*****************************************************************************
//#include	"cameradriver_OGMA.h"


#ifndef	_CAMERA_DRIVER_OGMA_H_
#define	_CAMERA_DRIVER_OGMA_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef __ogmacam_h__
	#include	"../OGMAcamSDK/inc/ogmacam.h"
#endif

int	CreateCameraObjects_OGMA(void);


//**************************************************************************************
class CameraDriverOGMA: public CameraDriver
{
	public:

		//
		// Construction
		//
							CameraDriverOGMA(const int deviceNum);
		virtual				~CameraDriverOGMA(void);


		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame=true);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	bool					GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);


				void	HandleOGMAcallbackEvent(unsigned nEvent);


	protected:
				void	ReadCameraInfo(void);


				OgmacamDeviceV2		cOGMAdeviceInfo;
				HOgmacam			cOGMAcamH;
				bool				cOGMApicReady;
				unsigned int		cOGMAautoExpTime_us;

				unsigned int		cOGMAcamFormat;

				bool				cIsTriggerCam;
};



#endif	//	_CAMERA_DRIVER_OGMA_H_



