//**************************************************************************
//*	Name:			sonyStandAlone.h
//*
//*	Author:			Mark Sproul
//*
//*
//*	The purpose of this file is to allow a stand alone version of
//*	my Sony camera drivers without having to write two complete applications
//*	The normal use of the driver code is part of my AlpacaPi project
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//	#include	"sonyStandAlone.h"


#ifndef _SONY_STANDALONE_H_
#define	_SONY_STANDALONE_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_

//**************************************************************************************
class CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriver(void);
		virtual			~CameraDriver(void);
				void	GenerateFileNameRoot(void);


				int			cCameraID;						//*	this is used to control everything of the camera in other functions.Start from 0.
				bool		cCameraIsOpen;					//*	flag to tell us the camera is open
//-				double		cPixelSizeX;					//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
//-				double		cPixelSizeY;					//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
				bool		cIsColorCam;					//*	false= monochrome, true = color
//				bool		cMechanicalShutter;
//				bool		cIsCoolerCam;
//				bool		cIsUSB3Host;
				bool		cIsUSB3Camera;
//				double		cElecPerADU;
//				int			cBitDepth;
//				bool		cIsTriggerCam;
				char		cFileNameRoot[256];
				char		cDriverversionStr[64];
				char		cLastCameraErrMsg[256];

				TYPE_CommonProperties	cCommonProp;
				TYPE_CameraProperties	cCameraProp;
};



//**************************************************************************************
class CameraDriverSONY: public CameraDriver
{
	public:

		//
		// Construction
		//
						CameraDriverSONY(const int deviceNum, ICrCameraObjectInfo *camera_info);
		virtual			~CameraDriverSONY(void);
//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);

				//===================================================================
				//*	from alpacadriver.h
				bool				cDeviceConnected;		//*	normally always true
				char				cDeviceDescription[64];
				char				cDeviceName[128];
				char				cDeviceManufacturer[64];
				char				cDeviceManufAbrev[8];

				//===================================================================
				//*	from cameradriver.h
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);


			struct timeval	cLastExposureStartTime;		//*	time exposure or video was started for frame rate calculations

				//===================================================================
				//*	from cameradriver_SONY.h
		virtual	bool	AlpacaConnect(void);
		virtual	bool	AlpacaDisConnect(void);
		virtual	void	RunStateMachine_Device(void);

				void	ReadProperties(void);
				int		GetLiveView(void);
				bool	ProcessProperty(CrDeviceProperty *cameraProperty, char *propertyDisplayStr);

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

				bool							cUpdateProperties;
				const	ICrCameraObjectInfo		*cSonyCamera_info;
				SonyCallBack					*cCallbackObjPtr;
				CrDeviceHandle					cSonyDeviceHandle;
				char							cSONYidString[32];


};


#endif // _SONY_STANDALONE_H_

