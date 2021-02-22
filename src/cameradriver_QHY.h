//**************************************************************************
//*	Name:			cameradriver_QHY.h
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
//*	Mar 27,	2020	<MLS> Started on cameradriver_QHY.h
//*	Nov 29,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"cameradriver_QHY.h"


void	CreateQHY_CameraObjects(void);


#ifndef	_CAMERA_DRIVER_QHY_H_
#define	_CAMERA_DRIVER_QHY_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef __QHYCCD_H__
	#include	"../QHY/include/qhyccd.h"
#endif // __QHYCCD_H__


//**************************************************************************************
class CameraDriverQHY: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverQHY(const int deviceNum, const char *qhyIDstring);
		virtual			~CameraDriverQHY(void);
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
		void			ReadQHYcameraInfo(void);

		qhyccd_handle	*cQHYcamHandle;
		char			cQHYidString[64];

		//*	QHY camera properties
		bool			cQHY_CAM_8BITS;
		bool			cQHY_CAM_16BITS;
		bool			cQHY_CAM_COLOR;
		bool			cQHY_CAM_GPS;
		bool			cQHY_NumberOfReadModes;

};
#endif // _CAMERA_DRIVER_QHY_H_
