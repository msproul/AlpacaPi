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


#ifndef	_CAMERA_DRIVER_QHY_H_
#define	_CAMERA_DRIVER_QHY_H_

#ifndef	_CAMERA_DRIVER_H_
	#include	"cameradriver.h"
#endif

#ifndef __QHYCCD_H__
//	#include	"../QHY/include/qhyccd.h"
	#include	<qhyccd.h>
#endif

#ifndef _PARSE_NMEA_H_
	#include	"ParseNMEA.h"
#endif

int		CreateCameraObjects_QHY(void);

//**************************************************************************************
//Image header data structure:
//*	this is the header as it comes from the camera
//**************************************************************************************
typedef struct	//	TYPE_QHY_RawImgHeader
{
	//Serial Number
	//The serial number is the hardware counter for the frame, which starts at 0 and
	//adds one to each frame that is generated. It works as the CMOS camera begins to work.
	//0 Sequence Number MSB
	//1 Sequence Number
	//2 Sequence Number
	//3 Sequence Number LSB
	uint8_t		SequenceNumber_MSB;
	uint8_t		SequenceNumber_1;
	uint8_t		SequenceNumber_2;
	uint8_t		SequenceNumber_LSB;


	//4
	//temporary Sequence Number
	//(Normally no use)
	uint8_t		TempSequenceNum;

	//Image Width
	//5
	//6
	//Image Width MSB
	//Image Width LSB
	uint8_t		ImageWidth_MSB;
	uint8_t		ImageWidth_LSB;

	//Image Height
	//7
	//8
	//Image Height MSB
	//Image Height LSB
	uint8_t		ImageHeight_MSB;
	uint8_t		ImageHeight_LSB;


	//Current Latitude Obtained from GPS
	//9
	//10
	//11
	//12
	//latitude MSB
	//latitude
	//latitude
	//latitude LSB
	uint8_t		Latitude_MSB;
	uint8_t		Latitude_10;
	uint8_t		Latitude_11;
	uint8_t		Latitude_LSB;

	//Current Longitude Obtained from GPS
	//13
	//14
	//15
	//16
	//longitude MSB
	//longitude
	//longitude
	//longitude LSB
	uint8_t		Longitude_MSB;
	uint8_t		Longitude_14;
	uint8_t		Longitude_15;
	uint8_t		Longitude_LSB;

	//17 Start_Flag
	uint8_t		Start_Flag;

	//Shutter Start Time (JS)
	//18 Start Second MSB
	//19 Start Second
	//20 Start Second
	//21 Start Second LSB
	uint8_t		ShutterStartTimeJS_MSB;
	uint8_t		ShutterStartTimeJS_19;
	uint8_t		ShutterStartTimeJS_20;
	uint8_t		ShutterStartTimeJS_LSB;

	//22 Start micro second MSB
	//23 Start micro second
	//24 Start micro second LSB
	uint8_t		StartMicroSecond_MSB;
	uint8_t		StartMicroSecond_23;
	uint8_t		StartMicroSecond_LSB;

	//25 End flag
	uint8_t		EndFlag;

	//Shutter End Time (JS)
	//26 End Second MSB
	//27 End Second
	//28 End Second
	//29 End Second LSB
	uint8_t		ShutterEndTimeJS_MSB;
	uint8_t		ShutterEndTimeJS_27;
	uint8_t		ShutterEndTimeJS_28;
	uint8_t		ShutterEndTimeJS_LSB;

	//30 End micro second MSB
	//31 End micro second
	//32 End micro second LSB
	uint8_t		EndMicroSecond_MSB;
	uint8_t		EndMicroSecond_31;
	uint8_t		EndMicroSecond_LSB;

	//GPS Status
	//33	now flag
	uint8_t		NowFlag;
	//Current Time
	//The current time is the vertical sync time of the CMOS sensor, not the exact time of
	//the shutter starts or closes
	//34 now second MSB
	//35 now second
	//36 now second
	//37 now second LSB

	uint8_t		NowSecond_MSB;
	uint8_t		NowSecond_35;
	uint8_t		NowSecond_36;
	uint8_t		NowSecond_LSB;

	//38 now micro second MSB
	//39 now micro second
	//40 now micro second LSB
	uint8_t		NowMicroSecond_MSB;
	uint8_t		NowMicroSecond_39;
	uint8_t		NowMicroSecond_LSB;

	//PPS Counter Value
	//The PPS counter value should be 10 million, but it may not be 10 million because of
	//the crystal temperature.The VCXO can be adjusted to bring it close to 10 million.When
	//the PPS signal is lost, it becomes 10000500.Do not set values above 10000500.
	//41 count of PPS MSB
	//42 count of PPS
	//43 count of PPS LSB

	uint8_t		CountOfPPS_MSB;
	uint8_t		CountOfPPS_42;
	uint8_t		CountOfPPS_LSB;

} TYPE_QHY_RawImgHeader;

//**************************************************************************************
typedef struct	//	TYPE_QHY_ImgHeader
{
	//Serial Number
	//The serial number is the hardware counter for the frame, which starts at 0 and
	//adds one to each frame that is generated. It works as the CMOS camera begins to work.
	int			SequenceNumber;

	//temporary Sequence Number
	//(Normally no use)
//	uint8_t		TempSequenceNum;

	short		ImageWidth;
	short		ImageHeight;

	uint32_t	Latitude_Raw;
	int			Latitude_Deg;
	double		Latitude_Min;

	uint32_t	Longitude_Raw;
	int			Longitude_Deg;
	double		Longitude_Min;

//	double		Latitude;
//	double		Longitude;

	uint8_t		Start_Flag;

	uint32_t		ShutterStartTimeJS;
	uint32_t		StartMicroSecond;
	struct timeval	ShutterStartTime;

	uint8_t			EndFlag;

	uint32_t		ShutterEndTimeJS;
	uint32_t		EndMicroSecond;
	struct timeval	ShutterEndTime;

	//GPS Status
	//33	now flag
	uint8_t		NowFlag;
	//Current Time
	//The current time is the vertical sync time of the CMOS sensor, not the exact time of
	//the shutter starts or closes

	uint32_t		NowSecondJS;
	uint32_t		NowMicroSecond;
	struct timeval	NowTime;

	//PPS Counter Value
	//The PPS counter value should be 10 million, but it may not be 10 million because of
	//the crystal temperature.The VCXO can be adjusted to bring it close to 10 million.When
	//the PPS signal is lost, it becomes 10000500.Do not set values above 10000500.
	uint32_t	CountOfPPS;

} TYPE_QHY_ImgHeader;

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
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame=true);
//		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);
//		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);

		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);

		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
		virtual	TYPE_ASCOM_STATUS		Take_Video(void);

		virtual	bool					GetImage_ROI_info(void);


		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTargetTemp(void);
		virtual	TYPE_ASCOM_STATUS		Write_SensorTargetTemp(const double newCCDtargetTemp);
//		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);

		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);

	private:
		void			ConfigureGPS(void);
		void			CheckColorCamOptions(void);

	protected:
		void			ReadQHYcameraInfo(void);
		void			ProcessImageHeader(unsigned char *imageDataPtr);
		void			ParseQHYimageHeader(TYPE_QHY_RawImgHeader *rawImgHdr, TYPE_QHY_ImgHeader *imgHeader);

		qhyccd_handle	*cQHYcamHandle;
		char			cQHYidString[64];

		//*	QHY camera properties
		bool			cQHY_CAM_8BITS;
		bool			cQHY_CAM_16BITS;
		bool			cQHY_CAM_COLOR;
		bool			cQHY_CAM_GPS;
		bool			cQHY_NumberOfReadModes;

		bool			cQHYimageHasHeadrInfo;

		//*	GPS data
		TYPE_NMEAInfoStruct	cGPSnmeaInfo;

};
#endif // _CAMERA_DRIVER_QHY_H_
