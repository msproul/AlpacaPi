//**************************************************************************
//*	Name:			cameradriver_PlayerOne.cpp
//*
//*	Author:			Mark Sproul (C) 2024
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	References:
//*		https://player-one-astronomy.com/
//*		https://player-one-astronomy.com/service/software/
//*****************************************************************************
//*	Edit History
//*		<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 18,	2024	<MLS> Discussed Alpaca migration with Vanessa Zhang
//*	Apr 19,	2024	<MLS> Created cameradriver_PlayerOne.cpp
//*	Apr 21,	2024	<MLS> Purchased Player One Neptune-C II Camera (4 mega-pixels)
//*	Apr 22,	2024	<MLS> PlayerOne camera reading images properly
//*	Apr 22,	2024	<MLS> Finished Read/Write Gain
//*	Apr 22,	2024	<MLS> Image flip working
//*	Apr 22,	2024	<MLS> Finished Read/Write Offset
//*	May 26,	2024	<MLS> Fixed common property "Name"  aka DeviceName
//*	May 26,	2024	<MLS> Found new version of PlayerOne SDK V3.6.2
//*	May 26,	2024	<MLS> Updated install_playerone.sh to handle new version
//*****************************************************************************
//https://player-one-astronomy.com/download/softwares/PlayerOne_Camera_SDK_Linux_V3.6.2.tar.gz

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_CAMERA_PLAYERONE_)

#include	<math.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define _USE_PDB_ADDITIONS_

#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"PlayerOneCamera.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"cameradriver_PlayerOne.h"


#define	kMaxCameraCnt	5



//**************************************************************************************
int	CreateCameraObjects_PlayerOne(void)
{
int		cameraCount;
int		iii;
bool	rulesFileOK;
int		playerOneCameraCnt;
char	rulesFileName[]	=	"99-player_one_astronomy.rules";
char	driverVersionString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(driverVersionString, POAGetSDKVersion());
	CONSOLE_DEBUG_W_STR("PlayerOne Ver#", driverVersionString);
	LogEvent(	"camera",
				"Library version (PlayerOne)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("camera", "PlayerOne", driverVersionString);

	//*	check to make sure the rules file is present
	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK)
	{
		CONSOLE_DEBUG_W_STR("rules is present:", rulesFileName);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("rules is MISSING:", rulesFileName);
		LogEvent(	"camera",
					"Problem with PlayerOne rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	playerOneCameraCnt	=	POAGetCameraCount();
	CONSOLE_DEBUG_W_NUM("playerOneCameraCnt\t=", playerOneCameraCnt);

	cameraCount	=	0;
	for (iii=0; iii < playerOneCameraCnt; iii++)
	{
		new CameraDriverPlayerOne(iii);
		cameraCount++;
	}
	return(cameraCount);
}

//**************************************************************************************
CameraDriverPlayerOne::CameraDriverPlayerOne(const int deviceNum)
	:CameraDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	memset(cPlayerOneAtrribute,	0,	(kMaxAtrributeCnt * sizeof(POAConfigAttributes)));
	memset(&cROIinfo,			0,	sizeof(TYPE_IMAGE_ROI_Info));
	strcpy(cDeviceManufacturer,	"PlayerOne");
	strcpy(cCommonProp.Name,	"PlayerOne");
	strcpy(cDeviceManufAbrev,	"PO");


	cCameraID					=	deviceNum;
	cIsTriggerCam				=	false;
	cCameraProp.ExposureMin_us	=	400;				//*	0.4 ms
	cCameraProp.ExposureMax_us	=	800 * 1000 *1000;	//*	800 seconds
	cCameraProp.CanStopExposure	=	true;
	cImageCompletedOK			=	false;

	ReadCameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

#ifdef _USE_CAMERA_READ_THREAD_
	StartDriverThread();
#endif

}

//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverPlayerOne::~CameraDriverPlayerOne(void)
{
POAErrors		poError;

	CONSOLE_DEBUG(__FUNCTION__);
	poError	=	POACloseCamera(cPlayerOneProp.cameraID);
	if (poError == POA_OK)
	{

	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
	}
}


//*****************************************************************************
void	CameraDriverPlayerOne::ReadCameraInfo(void)
{
unsigned int	iii;
//char			hardwareVer[16];
POAErrors		poError;

	CONSOLE_DEBUG(__FUNCTION__);
	poError	=	POAGetCameraProperties(cCameraID, &cPlayerOneProp);	//get camaera properties
	if (poError == POA_OK)
	{
		strcpy(cDeviceModel,			cPlayerOneProp.cameraModelName);
		strcpy(cCommonProp.Name,		"PlayerOne-");
		strcat(cCommonProp.Name,		cPlayerOneProp.cameraModelName);
		strcpy(cDeviceSerialNum,		cPlayerOneProp.SN);
		strcpy(cCameraProp.SensorName,	cPlayerOneProp.sensorModelName);
		CONSOLE_DEBUG_W_NUM("camera ID:             \t",	cPlayerOneProp.cameraID);
		CONSOLE_DEBUG_W_STR("camera name:           \t",	cPlayerOneProp.cameraModelName);
		CONSOLE_DEBUG_W_STR("camera SN:             \t",	cPlayerOneProp.SN);
		CONSOLE_DEBUG_W_STR("sensorModelName:       \t",	cPlayerOneProp.sensorModelName);
		CONSOLE_DEBUG_W_STR("cCommonProp.Name:      \t",	cCommonProp.Name);
		CONSOLE_DEBUG_W_STR("cCameraProp.SensorName:\t",	cCameraProp.SensorName);

//	char userCustomID[16];			///< user custom name, it will be will be added after the camera name, max len 16 bytes,like:Mars-C [Juno], default is empty
//	int cameraID;					///< it's unique,camera can be controlled and set by the cameraID
//	POABayerPattern bayerPattern;	///< the bayer filter pattern of camera
//	char localPath[256];			///< the path of the camera in the computer host
//	int bins[8];					///< bins supported by the camera, 1 == bin1, 2 == bin2,..., end with 0, eg:[1,2,3,4,0,0,0,0]
//	POAImgFormat imgFormats[8];		///< image data format supported by the camera, end with POA_END, eg:[POA_RAW8, POA_RAW16, POA_END,...]
//	POABool isSupportHardBin;		///< does the camera sensor support hardware bin (since V3.3.0)
//	int pID;							///< camera's Product ID, note: the vID of PlayerOne is 0xA0A0 (since V3.3.0)
		strcpy(cCameraProp.SensorName,	cPlayerOneProp.sensorModelName);
		cCameraProp.CameraXsize		=	cPlayerOneProp.maxWidth;
		cCameraProp.CameraYsize		=	cPlayerOneProp.maxHeight;
		cCameraProp.PixelSizeX		=	cPlayerOneProp.pixelSize;
		cCameraProp.PixelSizeY		=	cPlayerOneProp.pixelSize;

		cCameraProp.NumX			=	cCameraProp.CameraXsize;
		cCameraProp.NumY			=	cCameraProp.CameraYsize;

//		cBayerPattern				=	cPlayerOneProp.bayerPattern;
		cIsColorCam					=	cPlayerOneProp.isColorCamera;			//*	false= monochrome, true = color
		cIsCoolerCam				=	cPlayerOneProp.isHasCooler;
//		cIsUSB3Host					=	cPlayerOneProp.
		cIsUSB3Camera				=	cPlayerOneProp.isUSB3Speed;
		cBitDepth					=	cPlayerOneProp.bitDepth;
//		cIsTriggerCam				=	cPlayerOneProp.
		cSt4Port					=	cPlayerOneProp.isHasST4Port;
		cCameraProp.MaxbinX			=	1;
		cCameraProp.MaxbinY			=	1;

		if (cIsColorCam)
		{
			cCameraProp.SensorType	=	kSensorType_Color;
		}
		//----------------------------------------------------
		//*	process the supported bin values
		for (iii=0; iii<8; iii++)
		{
//			CONSOLE_DEBUG_W_NUM("bins[iii]\t=",	cPlayerOneProp.bins[iii]);
			if (cPlayerOneProp.bins[iii] > cCameraProp.MaxbinX)
			{
				cCameraProp.MaxbinX	=	cPlayerOneProp.bins[iii];
				cCameraProp.MaxbinY	=	cPlayerOneProp.bins[iii];
			}
		}
		//----------------------------------------------------
		//*	process the supported image formats
		iii	=	0;
		while ((cPlayerOneProp.imgFormats[iii] >= 0) && (iii < 8))
		{
			CONSOLE_DEBUG_W_NUM("imgFormats[iii]\t=",	cPlayerOneProp.imgFormats[iii]);
			switch(cPlayerOneProp.imgFormats[iii])
			{
				///< 8bit raw data, 1 pixel 1 byte, value range[0, 255]
				case POA_RAW8:
					AddReadoutModeToList(kImageType_RAW8);
					break;

				///< RGB888 color data, 1 pixel 3 bytes, value range[0, 255] (only color camera)
				case POA_RGB24:
					AddReadoutModeToList(kImageType_RGB24);
					break;

				///< 16bit raw data, 1 pixel 2 bytes, value range[0, 65535]
				case POA_RAW16:
					AddReadoutModeToList(kImageType_RAW16);
					break;

				///< 8bit monochrome data, convert the Bayer Filter Array to monochrome data. 1 pixel 1 byte, value range[0, 255] (only color camera)
				case POA_MONO8:
					AddReadoutModeToList(kImageType_MONO8);
					break;

				default:
					CONSOLE_DEBUG_W_NUM("Unknown image type", cPlayerOneProp.imgFormats[iii]);
					break;
			}
			iii++;
		}
		CONSOLE_DEBUG_W_STR("localPath\t=",	cPlayerOneProp.localPath);
		DumpCameraProperties(__FUNCTION__);

		poError	=	POAOpenCamera(cPlayerOneProp.cameraID);
		if (poError != POA_OK)
		{
			CONSOLE_DEBUG_W_NUM("POAOpenCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POAOpenCamera() returned error:\t",	POAGetErrorString(poError));
		}

		////////////////////////////////////////////////init camera////////////////////////////////////////////////
		poError	=	POAInitCamera(cPlayerOneProp.cameraID);
		if (poError != POA_OK) //This is just an example, regarding error handling, you can use your own method.
		{
			CONSOLE_DEBUG_W_NUM("POAInitCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POAInitCamera() returned error:\t",	POAGetErrorString(poError));
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
		}
		//--------------------------------------------------------------------------------
		//*	Get config Attribute
		ReadAttributes();

	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGetCameraProperties() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAGetCameraProperties() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
//	CONSOLE_ABORT(__FUNCTION__);
}

//
//config name: Exposure, config description: exposure duration(microsecond(us))
//is writable: 1
//is readable: 1
//min: 10, max: 2000000000, default: 10000
//
//config name: Gain, config description: gain
//is writable: 1
//is readable: 1
//min: 0, max: 400, default: 0
//
//config name: WB_R, config description: white balance: red pixels
//is writable: 1
//is readable: 1
//min: -1200, max: 1200, default: 0
//
//config name: WB_G, config description: white balance: green pixels
//is writable: 1
//is readable: 1
//min: -1200, max: 1200, default: 0
//
//config name: WB_B, config description: white balance: blue pixels
//is writable: 1
//is readable: 1
//min: -1200, max: 1200, default: 0
//
//
//config name: AutoExpMaxGain, config description: maximum gain value when gain is automatic
//is writable: 1
//is readable: 1
//min: 0, max: 400, default: 0
//
//config name: AutoExpMaxExpMS, config description: maximum exposure value(ms) when exposure is automatic
//is writable: 1
//is readable: 1
//min: 1, max: 1000, default: 100
//
//config name: AutoExpTargetBrightness, config description: target brightness value when exposure is automatic
//is writable: 1
//is readable: 1
//min: 50, max: 200, default: 100
//
//config name: Temperature, config description: sensor temperature(degrees celsius)
//is writable: 0
//is readable: 1
//min: -50.000000, max: 100.000000, default: 0.000000
//
//config name: CoolPowerPerc, config description: percentage of cooler power
//is writable: 0
//is readable: 1
//min: 0, max: 100, default: 0
//
//config name: TargetTemp, config description: target temperature
//is writable: 1
//is readable: 1
//min: -50, max: 30, default: 0
//
//config name: CoolerOn, config description: turn on/off cooler
//is writable: 1
//is readable: 1
//default is on: 0
//
//config name: FanPowerPerc, config description: percentage of radiator fan power
//is writable: 1
//is readable: 1
//min: 0, max: 100, default: 70
//
//config name: LensHeater, config description: turn on/off lens heater
//is writable: 0
//is readable: 1
//default is on: 0
//
//config name: LensHeaterPowerPerc, config description: percentage of lens heater power
//is writable: 1
//is readable: 1
//min: 0, max: 100, default: 10
//
//config name: e/ADU, config description: e/ADU
//is writable: 0
//is readable: 1
//min: 0.000000, max: 10.000000, default: 1.010000
//
//config name: Flip None, config description: the image is not flipped
//is writable: 1
//is readable: 1
//default is on: 1
//
//config name: Flip Hori, config description: flip the image horizontally
//is writable: 1
//is readable: 1
//default is on: 0
//
//config name: Flip Vert, config description: flip the image vertically
//is writable: 1
//is readable: 1
//default is on: 0
//
//config name: Flip Both, config description: flip the image horizontally and vertically
//is writable: 1
//is readable: 1
//default is on: 0
//
//config name: FrameLimit, config description: frame rate limit
//is writable: 1
//is readable: 1
//min: 0, max: 2000, default: 0
//
//config name: USBBandWidthLimit, config description: USB bandwidth limit
//is writable: 1
//is readable: 1
//min: 35, max: 100, default: 90
//
//config name: Pixels Binning Sum, config description: take the sum of pixels after binning
//is writable: 1
//is readable: 1
//default is on: 0

//*****************************************************************************
void	CameraDriverPlayerOne::ReadAttributes(void)
{
POAErrors			poError;
int					iii;
int					config_count;
int					myAttributIdx;
POAConfigAttributes	poaAtrribute;

	CONSOLE_DEBUG_W_BOOL("cCanFlipImage\t=",	cCanFlipImage);

	config_count	=	0;
	poError			=	POAGetConfigsCount(cPlayerOneProp.cameraID, &config_count);
	if (poError == POA_OK)
	{
		CONSOLE_DEBUG_W_NUM("config_count\t=",	config_count);
		for (iii = 0; iii < config_count; iii++)
		{
			poError	=	POAGetConfigAttributes(cPlayerOneProp.cameraID, iii, &poaAtrribute);

			if (poError == POA_OK)
			{
				//*	keep a copy for web display
				myAttributIdx	=	poaAtrribute.configID;
				if ((myAttributIdx >= 0) && (myAttributIdx < kMaxAtrributeCnt))
				{
					cPlayerOneAtrribute[myAttributIdx]	=	poaAtrribute;
				}

				switch(poaAtrribute.configID)
				{
					///< exposure time(unit: us), read-write, valueType == VAL_INT
					case POA_EXPOSURE:
						cCameraProp.ExposureMin_us		=	poaAtrribute.minValue.intValue;
						cCameraProp.ExposureMax_us		=	poaAtrribute.maxValue.intValue;
						cCameraProp.ExposureMin_seconds	=	cCameraProp.ExposureMin_us / (1000000.0);
						cCameraProp.ExposureMax_seconds	=	cCameraProp.ExposureMax_us / (1000000.0);
						break;

					///< gain, read-write, valueType == VAL_INT
					case POA_GAIN:
						if (poaAtrribute.isReadable)
						{
							cCameraProp.Gain	=	poaAtrribute.defaultValue.intValue;
							cCameraProp.GainMin	=	poaAtrribute.minValue.intValue;
							cCameraProp.GainMax	=	poaAtrribute.maxValue.intValue;
						}
						break;

					///< hardware bin, read-write, valueType == VAL_BOOL
					case POA_HARDWARE_BIN:
						break;

					///< camera temperature(uint: C), read-only, valueType == VAL_FLOAT
					case POA_TEMPERATURE:
						if (poaAtrribute.isReadable)
						{
							cTempReadSupported	=	true;
						}
						break;

					///< red pixels coefficient of white balance, read-write, valueType == VAL_INT
					case POA_WB_R:
						break;

					///< green pixels coefficient of white balance, read-write, valueType == VAL_INT
					case POA_WB_G:
						break;

					///< blue pixels coefficient of white balance, read-write, valueType == VAL_INT
					case POA_WB_B:
						break;

					///< camera offset, read-write, valueType == VAL_INT
					case POA_OFFSET:
						if (poaAtrribute.isReadable)
						{
							cCameraProp.Offset		=	poaAtrribute.defaultValue.intValue;
							cCameraProp.OffsetMin	=	poaAtrribute.minValue.intValue;
							cCameraProp.OffsetMax	=	poaAtrribute.maxValue.intValue;
							CONSOLE_DEBUG_W_NUM("cCameraProp.Offset   \t=",	cCameraProp.Offset);
							CONSOLE_DEBUG_W_NUM("cCameraProp.OffsetMin\t=",	cCameraProp.OffsetMin);
							CONSOLE_DEBUG_W_NUM("cCameraProp.OffsetMax\t=",	cCameraProp.OffsetMax);
						}
						if (poaAtrribute.isWritable)
						{
							cOffsetSupported	=	true;
						}
						break;

					///< maximum gain when auto-adjust, read-write, valueType == VAL_INT
					case POA_AUTOEXPO_MAX_GAIN:
						break;

					///< maximum exposure when auto-adjust(uint: ms), read-write, valueType == VAL_INT
					case POA_AUTOEXPO_MAX_EXPOSURE:
						break;

					///< target brightness when auto-adjust, read-write, valueType == VAL_INT
					case POA_AUTOEXPO_BRIGHTNESS:
						break;

					///< ST4 guide north, generally,it's DEC+ on the mount, read-write, valueType == VAL_BOOL
					case POA_GUIDE_NORTH:
						break;

					///< ST4 guide south, generally,it's DEC- on the mount, read-write, valueType == VAL_BOOL
					case POA_GUIDE_SOUTH:
						break;

					///< ST4 guide east, generally,it's RA+ on the mount, read-write, valueType == VAL_BOOL
					case POA_GUIDE_EAST:
						break;

					///< ST4 guide west, generally,it's RA- on the mount, read-write, valueType == VAL_BOOL
					case POA_GUIDE_WEST:
						break;

					///< e/ADU, This value will change with gain, read-only, valueType == VAL_FLOAT
					case POA_EGAIN:
						if (poaAtrribute.isReadable)
						{
							cCameraProp.ElectronsPerADU	=	poaAtrribute.defaultValue.floatValue;
						}
						break;

					///< cooler power percentage[0-100%](only cool camera), read-only, valueType == VAL_INT
					case POA_COOLER_POWER:
						if (poaAtrribute.isReadable)
						{
							cCameraProp.CanGetCoolerPower	=	true;
						}
						break;

					///< camera target temperature(uint: C), read-write, valueType == VAL_INT
					case POA_TARGET_TEMP:
						if (poaAtrribute.isWritable)
						{
							cCameraProp.CanSetCCDtemperature	=	true;
						}
						break;

					///< turn cooler(and fan) on or off, read-write, valueType == VAL_BOOL
					case POA_COOLER:
						break;

					///< (deprecated)get state of lens heater(on or off), read-only, valueType == VAL_BOOL
					case POA_HEATER:
						break;

					///< lens heater power percentage[0-100%], read-write, valueType == VAL_INT
					case POA_HEATER_POWER:
						break;

					///< radiator fan power percentage[0-100%], read-write, valueType == VAL_INT
					case POA_FAN_POWER:
						break;

					///< no flip, Note: set this config(POASetConfig), the 'confValue' will be ignored, read-write, valueType == VAL_BOOL
					case POA_FLIP_NONE:
						if (poaAtrribute.isWritable)
						{
							cCanFlipImage	=	true;
						}
						break;

					///< flip the image horizontally, Note: set this config(POASetConfig), the 'confValue' will be ignored, read-write, valueType == VAL_BOOL
					case POA_FLIP_HORI:
						if (poaAtrribute.isWritable)
						{
							cCanFlipImage	=	true;
						}
						break;

					///< flip the image vertically, Note: set this config(POASetConfig), the 'confValue' will be ignored, read-write, valueType == VAL_BOOL
					case POA_FLIP_VERT:
						if (poaAtrribute.isWritable)
						{
							cCanFlipImage	=	true;
						}
						break;

					///< flip the image horizontally and vertically, Note: set this config(POASetConfig), the 'confValue' will be ignored, read-write, valueType == VAL_BOOL
					case POA_FLIP_BOTH:
						if (poaAtrribute.isWritable)
						{
							cCanFlipImage	=	true;
						}
						break;

					///< Frame rate limit, the range:[0, 2000], 0 means no limit, read-write, valueType == VAL_INT
					case POA_FRAME_LIMIT:
						break;

					///< High quality image, for those without DDR camera(guide camera), if set POA_TRUE, this will reduce the waviness and stripe of the image,
					///< but frame rate may go down, note: this config has no effect on those cameras that with DDR. read-write, valueType == VAL_BOOL
					case POA_HQI:
						break;

					///< USB bandwidth limit, read-write, valueType == VAL_INT
					case POA_USB_BANDWIDTH_LIMIT:
						break;

					///< take the sum of pixels after binning, POA_TRUE is sum and POA_FLASE is average, default is POA_FLASE, read-write, valueType == VAL_BOOL
					case POA_PIXEL_BIN_SUM:
						break;

					default:
						CONSOLE_DEBUG_W_NUM("Unkown config ID\t=",	poaAtrribute.configID);
						break;

				}
				printf("\n");
				printf("config name: %s, config description: %s \n", poaAtrribute.szConfName, poaAtrribute.szDescription);

				printf("is writable: %d \n", (int)poaAtrribute.isWritable);

				printf("is readable: %d \n", (int)poaAtrribute.isReadable);

				if (poaAtrribute.valueType == VAL_INT)
				{
					printf("min: %ld, max: %ld, default: %ld \n", poaAtrribute.minValue.intValue, poaAtrribute.maxValue.intValue, poaAtrribute.defaultValue.intValue);
				}
				else if (poaAtrribute.valueType == VAL_FLOAT)
				{
					printf("min: %lf, max: %lf, default: %lf \n", poaAtrribute.minValue.floatValue, poaAtrribute.maxValue.floatValue, poaAtrribute.defaultValue.floatValue);
				}
				else if (poaAtrribute.valueType == VAL_BOOL) // The maxValue and minValue values of this VAL_BOOL type are meaningless
				{
					printf("default is on: %d \n", (int)poaAtrribute.defaultValue.boolValue);
				}
			}
			else
			{
				printf("get config attributes failed, index: %d, error code: %s \n", iii, POAGetErrorString(poError));
			}
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGetConfigsCount() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAGetConfigsCount() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	CONSOLE_DEBUG_W_BOOL("cCanFlipImage\t=",	cCanFlipImage);
}

//*****************************************************************************
//*	the camera must already be open when this is called
//*****************************************************************************
bool	CameraDriverPlayerOne::GetImage_ROI_info(void)
{

//	memset(&cROIinfo, 0, sizeof(TYPE_IMAGE_ROI_Info));

//	cROIinfo.currentROIimageType	=	kImageType_RAW8; // PDB kImageType_RGB24;
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;

	return(true);
}


//*****************************************************************************
//*	http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure Content-Type: -dDuration=0.001&Light=true
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAErrors			poError;
POAConfigValue		configValue;

	CONSOLE_DEBUG(__FUNCTION__);


	//*	fist set the exposure
	configValue.intValue	=	exposureMicrosecs;
	poError					=	POASetConfig(	cPlayerOneProp.cameraID,
												POA_EXPOSURE,
												configValue,
												POA_FALSE);	//*	isAuto = false
	if (poError == POA_OK)
	{
		CONSOLE_DEBUG("POASetConfig(POA_EXPOSURE) OK!!!!!");

	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		CONSOLE_DEBUG_W_NUM("POAOpenCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAOpenCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}

	cImageCompletedOK	=	false;
	poError				=	POAStartExposure(cPlayerOneProp.cameraID, POA_TRUE);	//*	POA_TRUE means single frame mode
	if (poError == POA_OK)
	{
		CONSOLE_DEBUG("POAStartExposure() OK!!!!!");
		gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);
		cInternalCameraState	=	kCameraState_TakingPicture;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAOpenCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAOpenCamera() returned error:\t",	POAGetErrorString(poError));
		switch (poError)
		{
			case POA_ERROR_INVALID_ID:
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_INVALID_ID");
				break;

			case POA_ERROR_NOT_OPENED:
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not Connected");
				break;

			case POA_ERROR_INVALID_ARGU:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_INVALID_ARGU");
				break;

			case POA_ERROR_OPERATION_FAILED:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_OPERATION_FAILED");
				break;

			case POA_ERROR_EXPOSING:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_EXPOSING");
				break;

			default:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "unknown");
				break;
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAErrors			poError;

//	CONSOLE_DEBUG(__FUNCTION__);
// * @return  POA_OK: operation successful
// *          POA_ERROR_INVALID_ID: no camera with this ID was found or the ID is out of boundary
// *          POA_ERROR_NOT_OPENED: camera not opened
// *          POA_ERROR_OPERATION_FAILED: operation failed
//
	poError	=	POAStopExposure(cPlayerOneProp.cameraID);
	if (poError == POA_OK)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		CONSOLE_DEBUG_W_NUM("POAOpenCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAOpenCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverPlayerOne::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	exposureState;

//	CONSOLE_DEBUG(__FUNCTION__);
	switch(cInternalCameraState)
	{
		case kCameraState_Idle:
			exposureState	=	kExposure_Idle;
			break;

		case kCameraState_TakingPicture:
			if (cImageCompletedOK)
			{
				exposureState		=	kExposure_Success;
				cImageCompletedOK	=	false;
			}
			else
			{
				exposureState	=	kExposure_Working;
			}
			break;

		case kCameraState_StartVideo:
			exposureState	=	kExposure_Idle;
			break;

		case kCameraState_TakingVideo:
			exposureState	=	kExposure_Idle;
			break;

		default:
			exposureState	=	kExposure_Idle;
			break;
	}
	return(exposureState);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAErrors			poError;
POAImgFormat		poImageFormat;

	switch(newImageType)
	{
		case kImageType_RAW8:	poImageFormat	=	POA_RAW8;	break;
		case kImageType_RAW16:	poImageFormat	=	POA_RAW16;	break;
		case kImageType_RGB24:	poImageFormat	=	POA_RGB24;	break;
		case kImageType_Y8:		poImageFormat	=	POA_RAW8;	break;
		case kImageType_MONO8:	poImageFormat	=	POA_MONO8;	break;
		default:				poImageFormat	=	POA_RAW8;	break;
	}

	poError	=	POASetImageFormat(cPlayerOneProp.cameraID, poImageFormat);
	if (poError == POA_OK)
	{
		cROIinfo.currentROIimageType	=	newImageType;
		cDesiredImageType				=	newImageType;
		alpacaErrCode					=	kASCOM_Err_Success;
		CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAOpenCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POAOpenCamera() returned error:\t",	POAGetErrorString(poError));

		// * @return  POA_OK: operation successful
		// *          POA_ERROR_INVALID_ID: no camera with this ID was found or the ID is out of boundary
		// *          POA_ERROR_NOT_OPENED: camera not opened
		// *          POA_ERROR_INVALID_ARGU: the imgFormat is not a POAImgFormat
		// *          POA_ERROR_OPERATION_FAILED: operation failed
		switch (poError)
		{
			case POA_ERROR_INVALID_ID:
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_INVALID_ID");
				break;

			case POA_ERROR_NOT_OPENED:
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not Connected");
				break;

			case POA_ERROR_INVALID_ARGU:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_INVALID_ARGU");
				break;

			case POA_ERROR_OPERATION_FAILED:
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "POA_ERROR_OPERATION_FAILED");
				break;

			default:
				break;
		}
		CONSOLE_ABORT(__FUNCTION__);
	}

	return(alpacaErrCode);
}

#pragma mark -
#pragma mark Virtual functions

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
POAConfigValue		configValue;
POABool				isAuto;
POAErrors			poError;

	poError	=	POAGetConfig(cPlayerOneProp.cameraID, POA_GAIN, &configValue, &isAuto);
	if (poError == POA_OK)
	{
		*cameraGainValue	=	configValue.intValue;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAConfigValue		configValue;
POAErrors			poError;

//	CONSOLE_DEBUG(__FUNCTION__);

	configValue.intValue	=	newGainValue;
	poError					=	POASetConfig(	cPlayerOneProp.cameraID,
												POA_GAIN,
												configValue,
												POA_FALSE);	//*	isAuto = false
	if (poError == POA_OK)
	{
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_Offset(int *cameraOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
POAConfigValue		configValue;
POABool				isAuto;
POAErrors			poError;

	poError	=	POAGetConfig(cPlayerOneProp.cameraID, POA_OFFSET, &configValue, &isAuto);
	if (poError == POA_OK)
	{
		*cameraOffsetValue	=	configValue.intValue;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Write_Offset(const int newOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAConfigValue		configValue;
POAErrors			poError;

//	CONSOLE_DEBUG(__FUNCTION__);

	configValue.intValue	=	newOffsetValue;
	poError					=	POASetConfig(	cPlayerOneProp.cameraID,
												POA_OFFSET,
												configValue,
												POA_FALSE);	//*	isAuto = false
	if (poError == POA_OK)
	{
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//	Set Cooling:
//	int ArtemisSetCooling(ArtemisHandle hCam, int setpoint);
//	This function is used to set the temperature of the camera. The setpoint is in 1/100 of a degree
//	(Celcius). So, to set the cooling to -10C, you need to call the function with setpoint = -1000.
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAConfigValue		configValue;
POAErrors			poError;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCameraProp.CanSetCCDtemperature)
	{
		configValue.boolValue	=	POA_TRUE;
		poError					=	POASetConfig(	cPlayerOneProp.cameraID,
													POA_COOLER,
													configValue,
													POA_FALSE);	//*	isAuto = false
		if (poError == POA_OK)
		{
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not supported on this camera");
	}

	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAConfigValue		configValue;
POAErrors			poError;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCameraProp.CanSetCCDtemperature)
	{
		configValue.boolValue	=	POA_FALSE;
		poError					=	POASetConfig(	cPlayerOneProp.cameraID,
													POA_COOLER,
													configValue,
													POA_FALSE);	//*	isAuto = false
		if (poError == POA_OK)
		{
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not supported on this camera");
	}

	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//**************************************************************************
//*	returns error code,
//*	sets class varible to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAErrors			poError;
POAConfigValue		configValue;
POABool				isAuto;

	if (cTempReadSupported)
	{
		poError	=	POAGetConfig(cPlayerOneProp.cameraID, POA_TEMPERATURE, &configValue, &isAuto);
		if (poError == POA_OK)
		{
			cCameraProp.CCDtemperature	=	configValue.floatValue;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_CoolerState(bool *coolerOnOff)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
//	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
//	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_Fastreadout(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

////	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
//	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
POAErrors			poError;
int					nTimeoutms;
int					pixelSize_Bytes;
long				lBufSize;
long				pixelCount;
POAImgFormat		poImgFormat;

	CONSOLE_DEBUG(__FUNCTION__);
	//*	we have to allocate a buffer big enough to hold the image
	pixelCount	=	cCameraProp.CameraXsize *
					cCameraProp.CameraYsize;

	poError	=	POAGetImageFormat(cPlayerOneProp.cameraID, &poImgFormat);
	CONSOLE_DEBUG_W_NUM("poImgFormat\t=",	poImgFormat);

	switch(poImgFormat)
	{
		case POA_RGB24:
			pixelSize_Bytes	=	3;
			break;

		case POA_RAW16:
			pixelSize_Bytes	=	2;
			break;

		case POA_RAW8:
		case POA_MONO8:
		default:
			pixelSize_Bytes	=	1;
			break;

	}
	lBufSize	=	(pixelCount * pixelSize_Bytes) + 100;
	CONSOLE_DEBUG_W_NUM("cCameraProp.CameraXsize\t=",	cCameraProp.CameraXsize);
	CONSOLE_DEBUG_W_NUM("cCameraProp.CameraYsize\t=",	cCameraProp.CameraYsize);
	CONSOLE_DEBUG_W_NUM("pixelCount             \t=",	pixelCount);
	CONSOLE_DEBUG_W_NUM("pixelSize_Bytes        \t=",	pixelSize_Bytes);
	CONSOLE_DEBUG_W_NUM("lBufSize               \t=",	lBufSize);

	AllocateImageBuffer(-1);		//*	let it figure out how much

	if (cCameraDataBuffer != NULL)
	{

		nTimeoutms	=	-1;
		poError		=	POAGetImageData(cPlayerOneProp.cameraID,
										cCameraDataBuffer,
										lBufSize,
										nTimeoutms);
		if (poError == POA_OK)
		{
			CONSOLE_DEBUG("POAGetImageData() OK!!!!!");
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
			CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "failed to allocate image buffer");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPlayerOne::SetFlipMode(const int newFlipMode)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
POAConfigValue		configValue;
POAErrors			poError;
POAConfig			flipConfigID;

	CONSOLE_DEBUG(__FUNCTION__);

	switch(newFlipMode)
	{
		case kFlip_Horiz:
			flipConfigID	=	POA_FLIP_HORI;
			break;

		case kFlip_Vert:
			flipConfigID	=	POA_FLIP_VERT;
			break;

		case kFlip_Both:
			flipConfigID	=	POA_FLIP_BOTH;
			break;

		case kFlip_None:
		default:
			flipConfigID	=	POA_FLIP_NONE;
			break;
	}
	CONSOLE_DEBUG_W_NUM("flipConfigID\t=",	flipConfigID);

	configValue.boolValue	=	POA_TRUE;	//*	this gets ignored
	poError					=	POASetConfig(	cPlayerOneProp.cameraID,
												flipConfigID,
												configValue,
												POA_FALSE);	//*	isAuto = false
	if (poError == POA_OK)
	{
		cFlipMode		=	newFlipMode;
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
	return(alpacaErrCode);
}

//**************************************************************************
void	CameraDriverPlayerOne::RunThread_CheckPictureStatus(void)
{
POAErrors		poError;
POABool			isImageReady;

	poError	=	POAImageReady(cPlayerOneProp.cameraID, &isImageReady);
	if (poError == POA_OK)
	{
		if (isImageReady)
		{
			cImageCompletedOK	=	true;
//			Read_ImageData();
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POACloseCamera() returned error:\t",	poError);
		CONSOLE_DEBUG_W_STR("POACloseCamera() returned error:\t",	POAGetErrorString(poError));
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, POAGetErrorString(poError));
	}
}

//*****************************************************************************
static char	gPOattributeHeader[]	=
{
	"<TR><TH COLSPAN=8>Player One Camera attributes</TH></TR>\r\n"
	"<TR>"
	"<TH>#</TH>"
	"<TH>Name</TH>"
	"<TH>Description</TH>"
	"<TH>Type</TH>"
	"<TH>Min</TH>"
	"<TH>Max</TH>"
	"<TH>Default</TH>"
	"<TH>R/W</TH>"
	"</TR>\r\n"
};

//*****************************************************************************
void	CameraDriverPlayerOne::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
int					iii;
int					numberOfCtrls;
char				lineBuffer[256];
char				typeString[32];
char				minValueString[34];
char				maxValueString[34];
char				defValueString[34];
int					currentROIwidth;
int					currentROIheight;
int					currentROIbin;
int					mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(mySocketFD,	gPOattributeHeader);


	//*-----------------------------------------------------------
//typedef struct _POAConfigAttributes     ///< Camera Config Attributes Definition(every POAConfig has a POAConfigAttributes)
//{
//    POABool isSupportAuto;              ///< is support auto?
//    POABool isWritable;                 ///< is writable?
//    POABool isReadable;                 ///< is readable?
//    POAConfig configID;                 ///< config ID, eg: POA_EXPOSURE
//    POAValueType valueType;             ///< value type, eg: VAL_INT
//    POAConfigValue maxValue;            ///< maximum value
//    POAConfigValue minValue;            ///< minimum value
//    POAConfigValue defaultValue;        ///< default value
//    char szConfName[64];                ///< POAConfig name, eg: POA_EXPOSURE: "Exposure", POA_TARGET_TEMP: "TargetTemp"
//    char szDescription[128];            ///< a brief introduction about this one POAConfig
//
//    char reserved[64];                  ///< reserved
//} POAConfigAttributes;

	for (iii=0; iii< POA_PIXEL_BIN_SUM; iii++)
	{
		if (strlen(cPlayerOneAtrribute[iii].szConfName) > 0)
		{
			SocketWriteData(mySocketFD,	"<TR>\r\n");
			//-------------------------------------------------------
//			sprintf(lineBuffer,	"\t<TD>%d</TD>\r\n",	iii);
//			SocketWriteData(mySocketFD,	lineBuffer);

			//-------------------------------------------------------
			sprintf(lineBuffer,	"\t<TD>%d</TD>\r\n",	cPlayerOneAtrribute[iii].configID);
			SocketWriteData(mySocketFD,	lineBuffer);
			//-------------------------------------------------------
			SocketWriteData(mySocketFD,	"\t<TD>");
			SocketWriteData(mySocketFD,	cPlayerOneAtrribute[iii].szConfName);
			SocketWriteData(mySocketFD,	"</TD>\r\n");
			//-------------------------------------------------------
			SocketWriteData(mySocketFD,	"\t<TD>");
			SocketWriteData(mySocketFD,	cPlayerOneAtrribute[iii].szDescription);
			SocketWriteData(mySocketFD,	"</TD>\r\n");

			//-------------------------------------------------------
			switch((int)cPlayerOneAtrribute[iii].valueType)
			{
				case VAL_INT:
					strcpy(typeString,		"\t<TD>INT</TD>\r\n");
					sprintf(minValueString,	"\t<TD><CENTER>%ld</TD>\r\n",	cPlayerOneAtrribute[iii].minValue.intValue);
					sprintf(maxValueString,	"\t<TD><CENTER>%ld</TD>\r\n",	cPlayerOneAtrribute[iii].maxValue.intValue);
					sprintf(defValueString,	"\t<TD><CENTER>%ld</TD>\r\n",	cPlayerOneAtrribute[iii].defaultValue.intValue);
					break;

				case VAL_FLOAT:
					strcpy(typeString,		"\t<TD>DBL</TD>\r\n");
					sprintf(minValueString,	"\t<TD><CENTER>%3.5f</TD>\r\n",	cPlayerOneAtrribute[iii].minValue.floatValue);
					sprintf(maxValueString,	"\t<TD><CENTER>%3.5f</TD>\r\n",	cPlayerOneAtrribute[iii].maxValue.floatValue);
					sprintf(defValueString,	"\t<TD><CENTER>%3.5f</TD>\r\n",	cPlayerOneAtrribute[iii].defaultValue.floatValue);
					break;

				case VAL_BOOL:
					strcpy(typeString,		"\t<TD>BOOL</TD>\r\n");
					sprintf(minValueString,	"\t<TD><CENTER>%s</TD>\r\n",	(cPlayerOneAtrribute[iii].minValue.boolValue		? "True" : "False"));
					sprintf(maxValueString,	"\t<TD><CENTER>%s</TD>\r\n",	(cPlayerOneAtrribute[iii].maxValue.boolValue		? "True" : "False"));
					sprintf(defValueString,	"\t<TD><CENTER>%s</TD>\r\n",	(cPlayerOneAtrribute[iii].defaultValue.boolValue	? "True" : "False"));
					break;

			}
			SocketWriteData(mySocketFD,	typeString);
			SocketWriteData(mySocketFD,	minValueString);
			SocketWriteData(mySocketFD,	maxValueString);
			SocketWriteData(mySocketFD,	defValueString);

			//-------------------------------------------------------
			if (cPlayerOneAtrribute[iii].isWritable && cPlayerOneAtrribute[iii].isReadable)
			{
				strcpy(lineBuffer,	"<TD><CENTER>R/W</TD>\r\n");
			}
			else if (cPlayerOneAtrribute[iii].isReadable)
			{
				strcpy(lineBuffer,	"<TD><CENTER>R-only</TD>\r\n");
			}
			else if (cPlayerOneAtrribute[iii].isReadable)
			{
				strcpy(lineBuffer,	"<TD><CENTER>W-only</TD>\r\n");
			}
			SocketWriteData(mySocketFD,	lineBuffer);


			SocketWriteData(mySocketFD,	"</TR>\r\n");
		}
	}
	//*-----------------------------------------------------------
	//*	display the most recent jpeg image
	if (strlen(cLastJpegImageName) > 0)
	{
		SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=8><CENTER>\r\n");
		sprintf(lineBuffer,	"\t<img src=../%s width=75%%>\r\n",	cLastJpegImageName);
		SocketWriteData(mySocketFD,	lineBuffer);
	//	SocketWriteData(mySocketFD,	"<img src=../image.jpg width=75\%>\r\n");
		SocketWriteData(mySocketFD,	"</TD></TR>\r\n");
	}

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
}


#endif	//	defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)

