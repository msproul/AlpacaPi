//**************************************************************************
//*	Name:			cameradriver_PlayerOne.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 19,	2024	<MLS> Created cameradriver_PlayerOne.h
//*****************************************************************************
//#include	"cameradriver_PlayerOne.h"


#ifndef	_CAMERA_DRIVER_PLAYERONE_H_
#define	_CAMERA_DRIVER_PLAYERONE_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef PLAYERONECAMERA_H
	#include	"PlayerOneCamera.h"
#endif

int	CreateCameraObjects_PlayerOne(void);

#define	kMaxAtrributeCnt	32

//**************************************************************************************
class CameraDriverPlayerOne: public CameraDriver
{
	public:

		//
		// Construction
		//
							CameraDriverPlayerOne(const int deviceNum);
		virtual				~CameraDriverPlayerOne(void);
		virtual	void		OutputHTML_Part2(TYPE_GetPutRequestData *reqData);


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
		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);

		virtual	TYPE_ASCOM_STATUS		Read_Offset(int *cameraOffsetValue);
		virtual	TYPE_ASCOM_STATUS		Write_Offset(const int newOffsetValue);

		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);

		virtual	TYPE_ASCOM_STATUS		SetFlipMode(const int newFlipMode);

		virtual	void					RunThread_CheckPictureStatus(void);

	protected:
				void		ReadCameraInfo(void);
				void		ReadAttributes(void);

//				int					cPlayerOneCamIdx;
				POACameraProperties	cPlayerOneProp;
				POAConfigAttributes	cPlayerOneAtrribute[kMaxAtrributeCnt];

				bool				cImageCompletedOK;
};



#endif	//	_CAMERA_DRIVER_PLAYERONE_H_



