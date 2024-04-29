//**************************************************************************
//*	Name:			cameradriver_sim.h
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
//*	May  4,	2022	<MLS> Created cameradriver_sim.h
//*****************************************************************************
//#include	"cameradriver_sim.h"




#ifndef	_CAMERA_DRIVER_SIM_H_
#define	_CAMERA_DRIVER_SIM_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

int		CreateCameraObjects_Sim(void);


//**************************************************************************************
class CameraDriverSIM: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverSIM(const int deviceNum);
		virtual			~CameraDriverSIM(void);
		virtual	bool	AlpacaConnect(void);
//		virtual	bool	AlpacaDisConnect(void);

//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);


		//*****************************************************************************
		//*	Camera specific routines
//		virtual	bool					IsCameraIDvalid(const int argCameraID);
//		virtual	void					ResetCamera(void);
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame);
//		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
//		virtual	TYPE_ASCOM_STATUS		Abort_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//
//		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);
//		virtual	TYPE_ASCOM_STATUS		SetImageType(char *newImageTypeString);
//
		virtual	TYPE_ASCOM_STATUS		Write_BinX(const int newBinXvalue);
		virtual	TYPE_ASCOM_STATUS		Write_BinY(const int newBinYvalue);
//
//
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);
		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
//
		virtual	TYPE_ASCOM_STATUS		Read_Offset(int *cameraOffsetValue);
		virtual	TYPE_ASCOM_STATUS		Write_Offset(const int newOffsetValue);
//
//		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
//		virtual	TYPE_ASCOM_STATUS		Take_Video(void);
//
//		virtual	TYPE_ASCOM_STATUS		SetFlipMode(const int newFlipMode);
//
//		virtual	TYPE_ALPACA_CAMERASTATE	Read_AlapcaCameraState(void);
//
//
//
		virtual	bool					GetImage_ROI_info(void);
//
//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
//		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
//		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);

	protected:
		TYPE_EXPOSURE_STATUS			cSimulatedState;

};
#endif // _CAMERA_DRIVER_SIM_H_
