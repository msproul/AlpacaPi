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

void	CreateSimulator_CameraObjects(void);


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

};
#endif // _CAMERA_DRIVER_SIM_H_
