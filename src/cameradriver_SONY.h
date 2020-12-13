//**************************************************************************
//*	Name:			cameradriver_SONY.h
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
//*	Jul 16,	2020	<MLS> Started on cameradriver_SONY.h
//*****************************************************************************
//#include	"cameradriver_SONY.h"




#ifndef	_CAMERA_DRIVER_SONY_H_
#define	_CAMERA_DRIVER_SONY_H_


#ifndef CAMERAREMOTE_SDK_H
	#include	"CameraRemote_SDK.h"
#endif // CAMERAREMOTE_SDK_H

#ifndef ICRCAMERAOBJECTINFO_H
	#include "ICrCameraObjectInfo.h"
#endif // ICRCAMERAOBJECTINFO_H

#ifndef IDEVICECALLBACK_H
	#include	"IDeviceCallback.h"
#endif // IDEVICECALLBACK_H

void	CreateSONY_CameraObjects(void);


#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

//using namespace SCRSDK;



#ifndef _SONY_CALLBACK_H_
	#include	"sonyCallback.h"
#endif // _SONY_CALLBACK_H_


//**************************************************************************************
class CameraDriverSONY: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverSONY(const int deviceNum, SCRSDK::ICrCameraObjectInfo *camera_info);
		virtual			~CameraDriverSONY(void);
		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);

		virtual	void	AlpacaConnect(void);
		virtual	void	AlpacaDisConnect(void);
		virtual	void	RunStateMachine_Device(void);

		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//		virtual	int						SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
//		virtual	int						SetImageType(TYPE_IMAGE_TYPE newImageType);

//		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
//		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

//		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Take_Video(void);

//		virtual	int		GetImage_ROI_info(void);

//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
//		virtual	TYPE_ASCOM_STATUS		Read_Readoutmodes(char *readOutModeString, bool includeQuotes=false);
//		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
//		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
//		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);


				void	ReadProperties(void);
				int		GetLiveView(void);
				bool	ProcessProperty(SCRSDK::CrDeviceProperty *cameraProperty, char *propertyDisplayStr);

				//*	these are the call back functions, they get called indirectly
				void OnConnected(SCRSDK::DeviceConnectionVersioin version);
				void OnDisconnected(CrInt32u error);
				void OnPropertyChanged(void);
				void OnLvPropertyChanged(void);
				void OnCompleteDownload(CrChar* filename);
				void OnWarning(CrInt32u warning);
				void OnError(CrInt32u error);


	protected:
				void	ReadSONYcameraInfo(void);
				void	SaveData(unsigned char *pdata, int imgSize, const char *fileName);

				bool	cUpdateProperties;
				const	SCRSDK::ICrCameraObjectInfo	*cSonyCamera_info;
				SonyCallBack						*cCallbackObjPtr;
				SCRSDK::CrDeviceHandle				cSonyDeviceHandle;
				char								cSONYidString[32];


};
#endif // _CAMERA_DRIVER_SONY_H_

