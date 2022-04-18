//**************************************************************************
//*	Name:			cameradriver_QSI.h
//*
//*	Author:			Mark Sproul (C) 2022
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
//*	Apr  7,	2022	<MLS> Created cameradriver_QSI.h
//*****************************************************************************
//#include	"cameradriver_QSI.h"


void	CreateQSI_CameraObjects(void);


#ifndef	_CAMERA_DRIVER_QSI_H_
#define	_CAMERA_DRIVER_QSI_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef qsiapi_H
//	#include	"lib/qsiapi.h"
	#include	"qsiapi.h"
#endif


//**************************************************************************************
class CameraDriverQSI: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverQSI(const int deviceNum, std::string qsiSerialNumber);
		virtual			~CameraDriverQSI(void);
		virtual	bool	AlpacaConnect(void);
//		virtual	bool	AlpacaDisConnect(void);

//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);


		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
//		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

//		virtual	int						Start_Video(void);
//		virtual	int						Stop_Video(void);
//		virtual	int						Take_Video(void);

		virtual	bool					GetImage_ROI_info(void);

//		virtual	int						Cooler_TurnOn(void);
//		virtual	int						Cooler_TurnOff(void);
//		virtual	int						Read_CoolerState(bool *coolerOnOff);
//		virtual	int						Read_CoolerPowerLevel(void);
//		virtual	int						Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);


	protected:
				bool					ReadQSIcameraInfo(void);
				std::string				cQSIserialNumber;
				QSICamera				cQSIcamera;

};
#endif // _CAMERA_DRIVER_QSI_H_
