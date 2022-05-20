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




#ifndef	_CAMERA_DRIVER_QSI_H_
#define	_CAMERA_DRIVER_QSI_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef qsiapi_H
//	#include	"lib/qsiapi.h"
	#include	"qsiapi.h"
#endif

void	CreateQSI_CameraObjects(void);


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
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame=true);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_ASCOM_STATUS		Abort_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_BinX(const int newBinXvalue);
		virtual	TYPE_ASCOM_STATUS		Write_BinY(const int newBinYvalue);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

//		virtual	int						Start_Video(void);
//		virtual	int						Stop_Video(void);
//		virtual	int						Take_Video(void);

		virtual	bool					GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Write_SensorTemp(const double newCCDtemp);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);

//		virtual	int						Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);


	protected:
				bool					ReadQSIcameraInfo(void);
				std::string				cQSIserialNumber;
				QSICamera				cQSIcam;

};
#endif // _CAMERA_DRIVER_QSI_H_
