//**************************************************************************
//*	Name:			cameradriver_TOUP.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan  9,	2020	<MLS> Created cameradriver_TOUP.h
//*	Nov 29,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"cameradriver_TOUP.h"


#ifndef	_CAMERA_DRIVER_TOUP_H_
#define	_CAMERA_DRIVER_TOUP_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef __toupcam_h__
	#include	"toupcam.h"
#endif // __toupcam_h__

void	CreateTOUP_CameraObjects(void);


//**************************************************************************************
class CameraDriverTOUP: public CameraDriver
{
	public:

		//
		// Construction
		//
							CameraDriverTOUP(const int deviceNum);
		virtual				~CameraDriverTOUP(void);


		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	int		GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
//-		virtual	TYPE_ASCOM_STATUS		Read_Readoutmodes(char *readOutModeString, bool includeQuotes=false);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);



				void	HandleToupCallbackEvent(unsigned nEvent);


	protected:
				void	ReadTOUPcameraInfo(void);


				ToupcamDeviceV2		cToupDeviceInfo;
				HToupcam			cToupCamH;
				bool				cToupPicReady;
				unsigned int		cToupAutoExpTime_us;

};



#endif	//	_CAMERA_DRIVER_TOUP_H_

