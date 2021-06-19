//**************************************************************************
//*	Name:			cameradriver_ASI.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  3,	2019	<MLS> Created cameradriver_ASI.h
//*	Nov 29,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"cameradriver_ASI.h"


#ifndef	_CAMERA_DRIVER_ASI_H_
#define	_CAMERA_DRIVER_ASI_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif
#ifndef ASICAMERA2_H
	#include	"ASICamera2.h"
#endif

void	CreateASI_CameraObjects(void);


//**************************************************************************************
class CameraDriverASI: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverASI(const int deviceNum);
		virtual			~CameraDriverASI(void);
		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);

		virtual	bool	IsCameraIDvalid(const int argCameraID);
		virtual	void	ResetCamera(void);

		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

		virtual	TYPE_ASCOM_STATUS		Write_Offset(const int newOffsetValue);
		virtual	TYPE_ASCOM_STATUS		Read_Offset(int *cameraOffsetValue);

		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
		virtual	TYPE_ASCOM_STATUS		Take_Video(void);

		virtual	bool					GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
//		virtual	TYPE_ASCOM_STATUS		Read_Readoutmodes(char *readOutModeString, bool includeQuotes=false);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);
		virtual	TYPE_ASCOM_STATUS		SetFlipMode(int newFlipMode);

//		virtual	TYPE_ALPACA_CAMERASTATE		Read_AlapcaCameraState(void);


	protected:
		void			ReadASIcameraInfo(void);
		void			ReadASIcontrol(const int controlNum);
		ASI_ERROR_CODE	OpenASIcameraIfNeeded(int foo);
		void			CloseASICamera(const int cameraTblIdx);
		void			CheckForClosedError(ASI_ERROR_CODE theAsiErrorCode);

		//*****************************************************************************
		//*	data for this specific camera type
		int					asiDeviceNum;
		ASI_IMG_TYPE		cCurrentASIimageType;
		ASI_SUPPORTED_MODE	supportedModes;
		ASI_CAMERA_INFO		cAsiCameraInfo;
		ASI_ID				cAsiSerialNum;



};


#endif	//	_CAMERA_DRIVER_ASI_H_
