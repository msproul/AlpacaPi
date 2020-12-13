//**************************************************************************
//*	Name:			cameradriver_ATIK.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  5,	2019	<MLS> Created cameradriver_ATIK.h
//*	Nov 29,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"cameradriver_ATIK.h"


#ifndef	_CAMERA_DRIVER_ATIK_H_
#define	_CAMERA_DRIVER_ATIK_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#include	"AtikCameras.h"

void	CreateATIK_CameraObjects(void);


//**************************************************************************************
class CameraDriverATIK: public CameraDriver
{
	public:

		//
		// Construction
		//
							CameraDriverATIK(const int deviceNum);
		virtual				~CameraDriverATIK(void);


		//*****************************************************************************
		//*	Camera specific routines
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

		virtual	int		GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);





	protected:
		void			ReadATIKcameraInfo(void);
		void			ProcessATIKproperties(ARTEMISPROPERTIES	*atikProperties);
		int				ProcessATIKcoolingInfo(void);

		ArtemisHandle	hAtikCameraHandle;

		int				cATIKimageTypeIdx;	//*	used for initializing image type list

};

//*****************************************************************************
//*this is commented out in the ATIK SDK header file, it is needed
enum ARTEMISCOOLINGINFO
{
//	/*Info flags
//	b0-4 capabilities
//	b0	0 = no cooling 1=cooling
//	b1	0 = always on 1= controllable
//	b2	0 = on/off control not available	1= on off cooling control
//	b3	0 = no selectable power levels 1= selectable power levels
//	b4	0 = no temperature set point cooling 1= set point cooling
//	b5-7 report what�s actually happening
//	b5	0 = normal control	1=warming up
//	b6	0 = cooling off		1=cooling on
//	b7	0 = no set point control 1=set point control*/
	ARTEMIS_COOLING_INFO_HASCOOLING				=	1,
	ARTEMIS_COOLING_INFO_CONTROLLABLE			=	2,
	ARTEMIS_COOLING_INFO_ONOFFCOOLINGCONTROL	=	4,
	ARTEMIS_COOLING_INFO_POWERLEVELCONTROL		=	8,
	ARTEMIS_COOLING_INFO_SETPOINTCONTROL		=	16,
	ARTEMIS_COOLING_INFO_WARMINGUP				=	32,
	ARTEMIS_COOLING_INFO_COOLINGON				=	64,
	ARTEMIS_COOLING_INFO_SETPOINTCONTROLactive	=	128
};


#endif	//	_CAMERA_DRIVER_ATIK_H_
