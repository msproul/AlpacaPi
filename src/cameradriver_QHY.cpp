//**************************************************************************
//*	Name:			cameradriver_QHY.cpp
//*
//*	Author:			Mark Sproul (C) 2020
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar 27,	2020	<MLS> Created cameradriver_QHY.cpp
//*	Mar 27,	2020	<MLS> Received QHY camera from HighpointScientific
//*	Feb 20,	2021	<MLS> Bought used QHY PoleMaster, working on QHY again
//*	Feb 20,	2021	<MLS> Getting image from QHY-PoleMaster camera
//*	Feb 20,	2021	<MLS> Getting image from QHy5L-IIC camera
//*	Feb 20,	2021	<MLS> SUPPORTED: QHY cameras
//*	Feb 20,	2021	<MLS> QHY Color image working
//*	Feb 21,	2021	<MLS> Set ReadoutMode working for QHY
//*	Sep 18,	2021	<MLS> Trying to use QHY camera for guide scope
//*	Sep 18,	2021	<MLS> Fixed memory leak in CameraDriverQHY::Read_ImageData()
//*	Apr 15,	2022	<MLS> Installed QHY5III462C on WO102 ONAG guider
//*	Aug 29,	2023	<MLS> Received QHY174M camera with GPS from Mike (mike@bptrobotics.com)
//*	Aug 29,	2023	<MLS> GPS data is in optional image header
//*	Aug 30,	2023	<MLS> Added ProcessImageHeader()
//*	Sep  5,	2023	<MLS> More work on qhy image header
//*	Sep  6,	2023	<MLS> Added Cooler_TurnOn() & Cooler_TurnOff() to QHY
//*	Sep  6,	2023	<MLS> Added Write_SensorTargetTemp to QHY
//*	Sep  7,	2023	<MLS> 8 Bit mode now working properly
//*	Sep  8,	2023	<MLS> Added ConfigureGPS()
//*	Sep  8,	2023	<MLS> Added CheckColorCamOptions()
//*	Sep 23,	2023	<MLS> Fixed bug interpreting camera temperature availability
//*	Sep 26,	2023	<MLS> Added closing of QHY camera to class destructor
//*----------------------------------------------------------------------------
//*	Oct 10,	2122	<TODO> Add support for percentcompleted to QHY camera driver
//*****************************************************************************
//	https://www.qhyccd.com/html/prepub/log_en.html#!log_en.md
//	https://www.qhyccd.com/download/
//
//	https://www.qhyccd.com/file/repository/publish/SDK/QHYCCD_SDK_EN_2018.pdf
//	https://github.com/qhyccd-lzr/QHYCCD_Linux/tree/master


//	https://www.qhyccd.com/user-manual-of-filter-wheel-apis/
//*****************************************************************************

//: 323 [ReadQHYcameraInfo   ] CONTROL_BRIGHTNESS		= YES
//: 327 [ReadQHYcameraInfo   ] CONTROL_CONTRAST		= YES
//: 331 [ReadQHYcameraInfo   ] CONTROL_WBR		= NO
//: 335 [ReadQHYcameraInfo   ] CONTROL_WBB		= NO
//: 339 [ReadQHYcameraInfo   ] CONTROL_WBG		= NO
//: 343 [ReadQHYcameraInfo   ] CONTROL_GAMMA		= YES
//: 347 [ReadQHYcameraInfo   ] CONTROL_GAIN		= YES
//: 351 [ReadQHYcameraInfo   ] CONTROL_OFFSET		= YES
//: 355 [ReadQHYcameraInfo   ] CONTROL_EXPOSURE		= YES
//: 359 [ReadQHYcameraInfo   ] CONTROL_SPEED		= NO
//: 363 [ReadQHYcameraInfo   ] CONTROL_TRANSFERBIT	= YES
//: 367 [ReadQHYcameraInfo   ] CONTROL_CHANNELS		= NO
//: 371 [ReadQHYcameraInfo   ] CONTROL_USBTRAFFIC		= YES
//: 375 [ReadQHYcameraInfo   ] CONTROL_ROWNOISERE		= NO
//: 379 [ReadQHYcameraInfo   ] CONTROL_CURTEMP		= YES
//: 383 [ReadQHYcameraInfo   ] CONTROL_CURPWM		= YES
//: 387 [ReadQHYcameraInfo   ] CONTROL_MANULPWM		= YES
//: 391 [ReadQHYcameraInfo   ] CONTROL_CFWPORT		= YES
//: 395 [ReadQHYcameraInfo   ] CONTROL_COOLER		= YES
//: 400 [ReadQHYcameraInfo   ] CONTROL_ST4PORT		= YES
//: 405 [ReadQHYcameraInfo   ] CAM_COLOR			= NO
//: 411 [ReadQHYcameraInfo   ] CAM_BIN1X1MODE		= YES
//: 420 [ReadQHYcameraInfo   ] CAM_BIN2X2MODE		= YES
//: 429 [ReadQHYcameraInfo   ] CAM_BIN3X3MODE		= NO
//: 438 [ReadQHYcameraInfo   ] CAM_BIN4X4MODE		= NO
//: 455 [ReadQHYcameraInfo   ] CAM_TECOVERPROTECT_INTERFACE	= NO
//: 459 [ReadQHYcameraInfo   ] CAM_SINGNALCLAMP_INTERFACE	= NO
//: 463 [ReadQHYcameraInfo   ] CAM_FINETONE_INTERFACE	= NO
//: 467 [ReadQHYcameraInfo   ] CAM_SHUTTERMOTORHEATING_INTERFACE		= NO
//: 471 [ReadQHYcameraInfo   ] CAM_CALIBRATEFPN_INTERFACE	= NO
//: 476 [ReadQHYcameraInfo   ] CAM_CHIPTEMPERATURESENSOR_INTERFACE	= NO
//: 480 [ReadQHYcameraInfo   ] CAM_USBREADOUTSLOWEST_INTERFACE		= NO
//: 484 [ReadQHYcameraInfo   ] CAM_8BITS					= YES
//: 489 [ReadQHYcameraInfo   ] CAM_16BITS					= YES
//: 494 [ReadQHYcameraInfo   ] CAM_GPS						= YES
//: 499 [ReadQHYcameraInfo   ] CAM_IGNOREOVERSCAN_INTERFACE		= NO
//: 503 [ReadQHYcameraInfo   ] QHYCCD_3A_AUTOBALANCE		= NO
//: 507 [ReadQHYcameraInfo   ] QHYCCD_3A_AUTOEXPOSURE		= NO
//: 511 [ReadQHYcameraInfo   ] QHYCCD_3A_AUTOFOCUS			= NO
//: 515 [ReadQHYcameraInfo   ] CONTROL_AMPV				= YES
//: 519 [ReadQHYcameraInfo   ] CONTROL_VCAM				= YES
//: 523 [ReadQHYcameraInfo   ] CAM_VIEW_MODE			= NO
//: 527 [ReadQHYcameraInfo   ] CONTROL_CFWSLOTSNUM		= YES
//: 531 [ReadQHYcameraInfo   ] IS_EXPOSING_DONE			= NO
//: 535 [ReadQHYcameraInfo   ] ScreenStretchB			= NO
//: 542 [ReadQHYcameraInfo   ] CONTROL_DDR				= NO
//: 546 [ReadQHYcameraInfo   ] CAM_LIGHT_PERFORMANCE_MODE	= NO
//: 550 [ReadQHYcameraInfo   ] CAM_QHY5II_GUIDE_MODE	= NO
//: 554 [ReadQHYcameraInfo   ] DDR_BUFFER_CAPACITY	= NO
//: 561 [ReadQHYcameraInfo   ] DefaultGain		= NO
//: 565 [ReadQHYcameraInfo   ] DefaultOffset		= NO
//: 569 [ReadQHYcameraInfo   ] OutputDataActualBits	= NO
//: 573 [ReadQHYcameraInfo   ] OutputDataAlignment	= NO
//: 577 [ReadQHYcameraInfo   ] CAM_SINGLEFRAMEMODE	= YES
//: 581 [ReadQHYcameraInfo   ] CAM_LIVEVIDEOMODE		= YES
//: 594 [ReadQHYcameraInfo   ] cCameraProp.MaxbinX	= 2
//: 602 [ReadQHYcameraInfo   ] GetQHYCCDType returned	= 4004
//: 608 [ReadQHYcameraInfo   ] numModes			= 1
//: 613 [ReadQHYcameraInfo   ] modeName			= STANDARD MODE
//: 633 [ReadQHYcameraInfo   ] Gain_min			= 0.0000000000000000000000000
//: 634 [ReadQHYcameraInfo   ] Gain_max			= 480.0000000000000000000000000
//: 648 [ReadQHYcameraInfo   ] exposure min		= 1.0000000000000000000000000
//: 649 [ReadQHYcameraInfo   ] exposure max		= 3600000000.0000000000000000000000000
//: 660 [ReadQHYcameraInfo   ] CONTROL_CURTEMP		= 30.3000000000000007105427358

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)

#include	<string>
#include	<stdio.h>
#include	<time.h>

//#include	"../QHY/include/qhyccd.h"
#include	<qhyccd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"eventlogging.h"
#include	"cameradriver.h"
#include	"cameradriver_QHY.h"
#include	"linuxerrors.h"

#include	"ParseNMEA.h"
#include	"NMEA_helper.h"

//**************************************************************************************
static uint32_t	GetQHYlibraryVersionString(char *driverVersionString)
{
uint32_t		qhyRetCode;
uint32_t		year;
uint32_t		month;
uint32_t		day;
uint32_t		subday;

	qhyRetCode	=	GetQHYCCDSDKVersion(&year, &month, &day, &subday);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		sprintf(driverVersionString,	"V20%02d%02d%02d_%d",
										year,
										month,
										day,
										subday);
	//	CONSOLE_DEBUG_W_INT32("year\t\t=",		year);
	//	CONSOLE_DEBUG_W_INT32("month\t\t=",		month);
	//	CONSOLE_DEBUG_W_INT32("day\t\t=",		day);
	//	CONSOLE_DEBUG_W_INT32("subday\t\t=",	subday);

		CONSOLE_DEBUG_W_STR("QHY Driver version\t=", driverVersionString);
	}
	else
	{
		CONSOLE_DEBUG_W_INT32("qhyRetCode\t=",	qhyRetCode);
		strcpy(driverVersionString, "Failed to read version");
	}
	return(qhyRetCode);
}


//**************************************************************************************
int	CreateCameraObjects_QHY(void)
{
int				cameraCount;
uint32_t		qhyRetCode;
int				qhyCameraCnt;
int				iii;
char			qhyIDstring[64];
char			rulesFileName[]	=	"85-qhyccd.rules";
char			driverVersionString[64];
bool			rulesFileOK;

	CONSOLE_DEBUG(__FUNCTION__);

	CONSOLE_DEBUG_W_LONG("TYPE_QHY_RawImgHeader\t=", sizeof(TYPE_QHY_RawImgHeader));
	CONSOLE_DEBUG_W_LONG("TYPE_QHY_ImgHeader   \t=", sizeof(TYPE_QHY_ImgHeader));

//	CONSOLE_ABORT(__FUNCTION__);

	cameraCount	=	0;
	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK)
	{
		LogEvent(	"camera",
					"QHY rules OK",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}
	else
	{
		LogEvent(	"camera",
					"Problem with QHY rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

//	EnableQHYCCDMessage(false);
	SetQHYCCDLogLevel(1);

	qhyRetCode	=	InitQHYCCDResource();
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		qhyRetCode	=	GetQHYlibraryVersionString(driverVersionString);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
 			AddLibraryVersion("camera", "QHY", driverVersionString);
		}
		LogEvent(	"camera",
					"Library version (QHY)",
					NULL,
					kASCOM_Err_Success,
					driverVersionString);

		//*	see how many QHY cameras are attached
		qhyCameraCnt	=	ScanQHYCCD();
		CONSOLE_DEBUG_W_INT32("qhyCameraCnt\t\t=",	qhyCameraCnt);
		for (iii=0; iii < qhyCameraCnt; iii++)
		{
			qhyRetCode = GetQHYCCDId(iii, qhyIDstring);
			if (qhyRetCode == QHYCCD_SUCCESS)
			{
				CONSOLE_DEBUG_W_STR("Creating driver object for\t=",	qhyIDstring);
				new CameraDriverQHY(0, qhyIDstring);
				cameraCount++;
			}
			else
			{
				CONSOLE_DEBUG_W_INT32("GetQHYCCDId() failed qhyRetCode\t=",	qhyRetCode);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG_W_INT32("InitQHYCCDResource() failed qhyRetCode\t\t=",		qhyRetCode);
	}
	return(cameraCount);
}



//**************************************************************************************
CameraDriverQHY::CameraDriverQHY(const int deviceNum, const char *qhyIDstring)
	:CameraDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);
	cCameraID				=	deviceNum;
	cQHYcamHandle			=	NULL;
	cQHY_CAM_8BITS			=	false;
	cQHY_CAM_16BITS			=	false;
	cQHY_CAM_COLOR			=	false;
	cQHY_CAM_GPS			=	false;
	cQHYimageHasHeadrInfo	=	false;
	cQHY_NumberOfReadModes	=	0;

	strcpy(cDeviceManufAbrev,	"QHY");
	strcpy(cQHYidString, qhyIDstring);

	//*	set some defaults
	cROIinfo.currentROIimageType	=	kImageType_RAW8;

	ReadQHYcameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

	ParseNMEA_init(&cGPSnmeaInfo);

#ifdef _USE_CAMERA_READ_THREAD_
	StartDriverThread();
#endif
}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverQHY::~CameraDriverQHY(void)
{
uint32_t		qhyRetCode;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cQHYcamHandle != NULL)
	{
		qhyRetCode	=	CloseQHYCCD(cQHYcamHandle);
		if (qhyRetCode != QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_NUM("CloseQHYCCD returned ERROR:", qhyRetCode);
		}
	}
}

//*****************************************************************************
void	CameraDriverQHY::ReadQHYcameraInfo(void)
{
uint32_t		qhyRetCode;
unsigned int	width;
unsigned int	height;
unsigned int	bpp;
//unsigned int	channels;
double			chipw;
double			chiph;
double			pixelw;
double			pixelh;
char			qhyModelString[64];
char			qhyFPGAstring[64];
int				controlID;
bool			controlValid;
uint8_t			firmwareBuff[16];
double			humidity;
uint32_t		iii;
uint32_t		numModes;
char			modeName[32];
double			controlMin;
double			controlMax;
double			controlStep;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceManufacturer,	"QHY");

	qhyRetCode	=	GetQHYlibraryVersionString(cDeviceVersion);

	qhyRetCode	=	GetQHYCCDModel(cQHYidString, qhyModelString);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		CONSOLE_DEBUG_W_STR("qhyModelString\t=", qhyModelString);
		strcpy(cDeviceModel,		qhyModelString);
		strcpy(cGPS.CameraName,		qhyModelString);

		SetCommonPropertyName("QHY-", qhyModelString);
	}

	cQHYcamHandle	=	OpenQHYCCD(cQHYidString);
	if (cQHYcamHandle != NULL)
	{
		CONSOLE_DEBUG("Open QHYCCD success!");
		qhyRetCode	=	GetQHYCCDChipInfo(	cQHYcamHandle,
											&chipw,
											&chiph,
											&width,
											&height,
											&pixelw,
											&pixelh,
											&bpp);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG("GetQHYCCDChipInfo");
			printf("GetQHYCCDChipInfo success!\n");
			printf("CCD/CMOS chip information:\n");
			printf("Chip width %3f mm,Chip height %3f mm\n",chipw,chiph);
			printf("Chip pixel width %3f um,Chip pixel height %3f um\n",pixelw,pixelh);
			printf("Chip Max Resolution is %d x %d,depth is %d\n",width, height, bpp);

			cBitDepth				=	bpp;
			cCameraProp.CameraXsize	=	width;
			cCameraProp.CameraYsize	=	height;
			cCameraProp.PixelSizeX	=	pixelw;
			cCameraProp.PixelSizeY	=	pixelh;

			cCameraProp.NumX		=	cCameraProp.CameraXsize;
			cCameraProp.NumY		=	cCameraProp.CameraYsize;
		}
		qhyRetCode	=	InitQHYCCD(cQHYcamHandle);
//		CONSOLE_DEBUG_W_INT32("InitQHYCCD() returned qhyRetCode\t=",	qhyRetCode);

		memset(firmwareBuff, 0, sizeof(firmwareBuff));

		//----------------------------------------------------------------
		qhyRetCode	=	GetQHYCCDFWVersion(cQHYcamHandle, firmwareBuff);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{

		//	CONSOLE_DEBUG_W_HEX("firmwareBuff\t=",	firmwareBuff[0]);
		//	CONSOLE_DEBUG_W_HEX("firmwareBuff\t=",	firmwareBuff[1]);
			if ((firmwareBuff[0] >> 4) <= 9)
			{
				sprintf(cDeviceFirmwareVersStr,	"20%d_%d_%d",
												((firmwareBuff[0] >> 4) + 0x10),
												(firmwareBuff[0]&~0xf0),
												firmwareBuff[1]);
			}
			else
			{
				sprintf(cDeviceFirmwareVersStr,	"20%d_%d_%d",
												(firmwareBuff[0] >> 4),
												(firmwareBuff[0]&~0xf0),
												firmwareBuff[1]);
			}
			CONSOLE_DEBUG_W_STR("cDeviceFirmwareVersStr\t=",	cDeviceFirmwareVersStr);
		}


		//----------------------------------------------------------------
		//EXPORTC uint32_t STDCALL GetQHYCCDFPGAVersion(qhyccd_handle *h, uint8_t fpga_index, uint8_t *buf);
		uint8_t fpgaVer[32]	=	{0};
		//Gets the first FPGA version number
		cGPS.FPGAversion[0]	=	0;
		qhyRetCode	=	GetQHYCCDFPGAVersion(cQHYcamHandle, 0, fpgaVer);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			sprintf(qhyFPGAstring, "%d-%d-%d-%d", fpgaVer[0],fpgaVer[1],fpgaVer[2],fpgaVer[3]);
			strcpy(cGPS.FPGAversion,		qhyFPGAstring);
			strcpy(cCameraProp.FPGAversion,	qhyFPGAstring);
		}
		else
		{
			CONSOLE_DEBUG_W_INT32("GetQHYCCDFPGAVersion(0) qhyRetCode\t=",	qhyRetCode);
		}

		//Get two FPGA version numbers
		qhyRetCode	=	GetQHYCCDFPGAVersion(cQHYcamHandle, 1, fpgaVer);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			sprintf(qhyFPGAstring, "%d-%d-%d-%d", fpgaVer[0],fpgaVer[1],fpgaVer[2],fpgaVer[3]);
			strcat(cGPS.FPGAversion, " / ");
			strcat(cGPS.FPGAversion, qhyFPGAstring);
		}
		else
		{
			CONSOLE_DEBUG_W_INT32("GetQHYCCDFPGAVersion(1) qhyRetCode\t=",	qhyRetCode);
		}
		//----------------------------------------------------------------
		//*	get the control information from the camera
		for (controlID = 0; controlID < CONTROL_MAX_ID; controlID++)
		{
			qhyRetCode	=	IsQHYCCDControlAvailable(cQHYcamHandle,	(CONTROL_ID)controlID);
			if (qhyRetCode == QHYCCD_SUCCESS)
			{
			//	CONSOLE_DEBUG_W_INT32("Supported: controlID\t=",	controlID);
				controlValid	=	true;
			}
			else if (qhyRetCode == QHYCCD_ERROR)
			{
				controlValid	=	false;
			}
			else if (controlID != CAM_COLOR)
			{
				controlValid	=	true;
				CONSOLE_DEBUG_W_INT32("IsQHYCCDControlAvailable() controlID\t=",	controlID);
				CONSOLE_DEBUG_W_INT32("IsQHYCCDControlAvailable() qhyRetCode\t=",	qhyRetCode);
			}
			switch(controlID)
			{
				case CONTROL_BRIGHTNESS:		//!< image brightness
					CONSOLE_DEBUG_W_STR("CONTROL_BRIGHTNESS\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CONTRAST:			//!< image contrast
					CONSOLE_DEBUG_W_STR("CONTROL_CONTRAST\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_WBR:				//!< red of white balance
					CONSOLE_DEBUG_W_STR("CONTROL_WBR\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_WBB:				//!< blue of white balance
					CONSOLE_DEBUG_W_STR("CONTROL_WBB\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_WBG:				//!< the green of white balance
					CONSOLE_DEBUG_W_STR("CONTROL_WBG\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_GAMMA:				//!< screen gamma
					CONSOLE_DEBUG_W_STR("CONTROL_GAMMA\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_GAIN:				//!< camera gain
					CONSOLE_DEBUG_W_STR("CONTROL_GAIN\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_OFFSET:			//!< camera offset
					CONSOLE_DEBUG_W_STR("CONTROL_OFFSET\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_EXPOSURE:			//!< expose time (us)
					CONSOLE_DEBUG_W_STR("CONTROL_EXPOSURE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_SPEED:				//!< transfer speed
					CONSOLE_DEBUG_W_STR("CONTROL_SPEED\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_TRANSFERBIT:		//!< image depth bits
					CONSOLE_DEBUG_W_STR("CONTROL_TRANSFERBIT\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CHANNELS:			//!< image channels
					CONSOLE_DEBUG_W_STR("CONTROL_CHANNELS\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_USBTRAFFIC:		//!< hblank
					CONSOLE_DEBUG_W_STR("CONTROL_USBTRAFFIC\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_ROWNOISERE:		//!< row denoise
					CONSOLE_DEBUG_W_STR("CONTROL_ROWNOISERE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CURTEMP:			//!< current cmos or ccd temprature
					CONSOLE_DEBUG_W_STR("CONTROL_CURTEMP\t\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cTempReadSupported	=	true;
					}
					break;

				case CONTROL_CURPWM:			//!< current cool pwm
					CONSOLE_DEBUG_W_STR("CONTROL_CURPWM\t\t=",	(controlValid ? "YES" : "NO"));
					cCameraProp.CanSetCCDtemperature	=	controlValid;
					break;

				case CONTROL_MANULPWM:			//!< set the cool pwm
					CONSOLE_DEBUG_W_STR("CONTROL_MANULPWM\t\t=",	(controlValid ? "YES" : "NO"));
					cCameraProp.CanGetCoolerPower	=	controlValid;
					break;

				case CONTROL_CFWPORT:			//!< control camera color filter wheel port
					CONSOLE_DEBUG_W_STR("CONTROL_CFWPORT\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_COOLER:			//!< check if camera has cooler
					CONSOLE_DEBUG_W_STR("CONTROL_COOLER\t\t=",	(controlValid ? "YES" : "NO"));
					cIsCoolerCam	=	controlValid;
					break;

				case CONTROL_ST4PORT:			//!< check if camera has st4port
					CONSOLE_DEBUG_W_STR("CONTROL_ST4PORT\t\t=",	(controlValid ? "YES" : "NO"));
					cSt4Port					=	controlValid;
					cCameraProp.CanPulseGuide	=	controlValid;
					break;

				case CAM_COLOR:
					CONSOLE_DEBUG_W_STR("CAM_COLOR\t\t\t=",	(controlValid ? "YES" : "NO"));
					cIsColorCam		=	controlValid;
					cQHY_CAM_COLOR	=	controlValid;
					break;

				case CAM_BIN1X1MODE:				//!< check if camera has bin1x1 mode
					CONSOLE_DEBUG_W_STR("CAM_BIN1X1MODE\t\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cCameraProp.MaxbinX	=	1;
						cCameraProp.MaxbinY	=	1;
					}
					break;

				case CAM_BIN2X2MODE:				//!< check if camera has bin2x2 mode
					CONSOLE_DEBUG_W_STR("CAM_BIN2X2MODE\t\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cCameraProp.MaxbinX	=	2;
						cCameraProp.MaxbinY	=	2;
					}
					break;

				case CAM_BIN3X3MODE:				//!< check if camera has bin3x3 mode
					CONSOLE_DEBUG_W_STR("CAM_BIN3X3MODE\t\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cCameraProp.MaxbinX	=	3;
						cCameraProp.MaxbinY	=	3;
					}
					break;

				case CAM_BIN4X4MODE:				//!< check if camera has bin4x4 mode
					CONSOLE_DEBUG_W_STR("CAM_BIN4X4MODE\t\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cCameraProp.MaxbinX	=	4;
						cCameraProp.MaxbinY	=	4;
					}
					break;

				case CAM_MECHANICALSHUTTER:					//!< mechanical shutter
					cCameraProp.HasShutter	=	controlValid;
					break;

				case CAM_TRIGER_INTERFACE:					//!< triger
					cIsTriggerCam	=	controlValid;
					break;

				case CAM_TECOVERPROTECT_INTERFACE:			//!< tec overprotect
					CONSOLE_DEBUG_W_STR("CAM_TECOVERPROTECT_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_SINGNALCLAMP_INTERFACE:			//!< singnal clamp
					CONSOLE_DEBUG_W_STR("CAM_SINGNALCLAMP_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_FINETONE_INTERFACE:				//!< fine tone
					CONSOLE_DEBUG_W_STR("CAM_FINETONE_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_SHUTTERMOTORHEATING_INTERFACE:		//!< shutter motor heating
					CONSOLE_DEBUG_W_STR("CAM_SHUTTERMOTORHEATING_INTERFACE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_CALIBRATEFPN_INTERFACE:			//!< calibrated frame
					CONSOLE_DEBUG_W_STR("CAM_CALIBRATEFPN_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_CHIPTEMPERATURESENSOR_INTERFACE:	//!< chip temperaure sensor
					CONSOLE_DEBUG_W_STR("CAM_CHIPTEMPERATURESENSOR_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					if (controlValid)
					{
						cTempReadSupported	=	true;
					}
					break;

				case CAM_USBREADOUTSLOWEST_INTERFACE:		//!< usb readout slowest
					CONSOLE_DEBUG_W_STR("CAM_USBREADOUTSLOWEST_INTERFACE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_8BITS:								//!< 8bit depth
					CONSOLE_DEBUG_W_STR("CAM_8BITS\t\t\t=",	(controlValid ? "YES" : "NO"));
					cQHY_CAM_8BITS	=	controlValid;
					break;

				case CAM_16BITS:							//!< 16bit depth
					CONSOLE_DEBUG_W_STR("CAM_16BITS\t\t\t=",	(controlValid ? "YES" : "NO"));
					cQHY_CAM_16BITS	=	controlValid;
					break;

				case CAM_GPS:								//!< check if camera has gps
					CONSOLE_DEBUG_W_STR("CAM_GPS\t\t\t=",	(controlValid ? "YES" : "NO"));
					cQHY_CAM_GPS	=	controlValid;
					if (cQHY_CAM_GPS)
					{
						cGPS.Present	=	true;
						cOverlayMode	=	1;		//*	force this for now, it will be an option later
						strcat(cGPS.CameraName,		"-GPS");
						CONSOLE_DEBUG_W_STR("cGPScameraName\t=", cGPS.CameraName);
					}
					break;

				case CAM_IGNOREOVERSCAN_INTERFACE:			//!< ignore overscan area
					CONSOLE_DEBUG_W_STR("CAM_IGNOREOVERSCAN_INTERFACE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case QHYCCD_3A_AUTOBALANCE:
					CONSOLE_DEBUG_W_STR("QHYCCD_3A_AUTOBALANCE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case QHYCCD_3A_AUTOEXPOSURE:
					CONSOLE_DEBUG_W_STR("QHYCCD_3A_AUTOEXPOSURE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case QHYCCD_3A_AUTOFOCUS:
					CONSOLE_DEBUG_W_STR("QHYCCD_3A_AUTOFOCUS\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_AMPV:							//!< ccd or cmos ampv
					CONSOLE_DEBUG_W_STR("CONTROL_AMPV\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_VCAM:							//!< Virtual Camera on off
					CONSOLE_DEBUG_W_STR("CONTROL_VCAM\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_VIEW_MODE:
					CONSOLE_DEBUG_W_STR("CAM_VIEW_MODE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CFWSLOTSNUM:					//!< check CFW slots number
					CONSOLE_DEBUG_W_STR("CONTROL_CFWSLOTSNUM\t=",	(controlValid ? "YES" : "NO"));
					break;

				case IS_EXPOSING_DONE:
					CONSOLE_DEBUG_W_STR("IS_EXPOSING_DONE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case ScreenStretchB:
					CONSOLE_DEBUG_W_STR("ScreenStretchB\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case ScreenStretchW:
					break;

				case CONTROL_DDR:
					CONSOLE_DEBUG_W_STR("CONTROL_DDR\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_LIGHT_PERFORMANCE_MODE:
					CONSOLE_DEBUG_W_STR("CAM_LIGHT_PERFORMANCE_MODE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_QHY5II_GUIDE_MODE:
					CONSOLE_DEBUG_W_STR("CAM_QHY5II_GUIDE_MODE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case DDR_BUFFER_CAPACITY:
					CONSOLE_DEBUG_W_STR("DDR_BUFFER_CAPACITY\t=",	(controlValid ? "YES" : "NO"));
					break;

				case DDR_BUFFER_READ_THRESHOLD:
					break;

				case DefaultGain:
					CONSOLE_DEBUG_W_STR("DefaultGain\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case DefaultOffset:
					CONSOLE_DEBUG_W_STR("DefaultOffset\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case OutputDataActualBits:
					CONSOLE_DEBUG_W_STR("OutputDataActualBits\t=",	(controlValid ? "YES" : "NO"));
					break;

				case OutputDataAlignment:
					CONSOLE_DEBUG_W_STR("OutputDataAlignment\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_SINGLEFRAMEMODE:
					CONSOLE_DEBUG_W_STR("CAM_SINGLEFRAMEMODE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_LIVEVIDEOMODE:
					CONSOLE_DEBUG_W_STR("CAM_LIVEVIDEOMODE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_MAX_ID:
					break;
			}
	//		if (controlValid)
	//		{
	//		unsigned long		paramUlong	=	0;
	//			paramUlong		=	GetQHYCCDParam(cQHYcamHandle, (CONTROL_ID)controlID);
	//			CONSOLE_DEBUG_W_HEX("Param value\t\t=", paramUlong);
	//		}
		}
		CONSOLE_DEBUG_W_NUM("cCameraProp.MaxbinX\t=",	cCameraProp.MaxbinX);

		qhyRetCode	=	GetQHYCCDHumidity(cQHYcamHandle, &humidity);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_DBL("humidity\t\t\t=", humidity);
		}
		qhyRetCode	=	GetQHYCCDType(cQHYcamHandle);
		CONSOLE_DEBUG_W_INT32("GetQHYCCDType returned\t=", qhyRetCode);

		qhyRetCode	=	GetQHYCCDNumberOfReadModes(cQHYcamHandle, &numModes);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			cQHY_NumberOfReadModes	=	numModes;
			CONSOLE_DEBUG_W_INT32("numModes\t\t\t=", numModes);
			for (iii=0; iii<numModes; iii++)
			{
				// Get the name of a read mode
				qhyRetCode	=	GetQHYCCDReadModeName(cQHYcamHandle,  iii, modeName);
				CONSOLE_DEBUG_W_STR("modeName\t\t\t=", modeName);
			}
		}

		// Get the maximum resolution for a read mode
//		qhyRetCode	=	GetQHYCCDReadModeResolution(cQHYcamHandle,uint32_t modeNumber, uint32_t* width, uint32_t* height);
//		// Get the read mode
//		qhyRetCode	=	GetQHYCCDReadMode(cQHYcamHandle,uint32_t* modeNumber);

		//*	get limits of parameters

		//===============================================================
		//*	Gain min/max/step
		qhyRetCode	=	GetQHYCCDParamMinMaxStep(cQHYcamHandle,
												CONTROL_GAIN,
												&controlMin,
												&controlMax,
												&controlStep);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_DBL("Gain_min\t\t\t=", controlMin);
			CONSOLE_DEBUG_W_DBL("Gain_max\t\t\t=", controlMax);
			cCameraProp.GainMin	=	controlMin;
			cCameraProp.GainMax	=	controlMax;
		}
		//===============================================================
		//*	Exposure min/max/step
		//*	CONTROL_EXPOSURE,	   //!< expose time (us)
		qhyRetCode	=	GetQHYCCDParamMinMaxStep(cQHYcamHandle,
												CONTROL_EXPOSURE,
												&controlMin,
												&controlMax,
												&controlStep);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_DBL("exposure min\t\t=", controlMin);
			CONSOLE_DEBUG_W_DBL("exposure max\t\t=", controlMax);
			cCameraProp.ExposureMin_us	=	controlMin;
			cCameraProp.ExposureMax_us	=	controlMax;
		}


//		if (cTempReadSupported)
		{
		double	controlValue;

			controlValue	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURTEMP);
			CONSOLE_DEBUG_W_DBL("CONTROL_CURTEMP\t\t=", controlValue);
		}


		//================================================
		//*	Lets build the ReadOutModesList;
		if (cQHY_CAM_8BITS)
		{
			AddReadoutModeToList(kImageType_RAW8);
			//*	set this to the default mode
			SetImageType(kImageType_RAW8);
		}
		if (cQHY_CAM_16BITS)
		{
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cQHY_CAM_8BITS && cQHY_CAM_COLOR)
		{
			AddReadoutModeToList(kImageType_RGB24);
		}

		//================================================
		if (cQHY_CAM_COLOR)
		{
			CheckColorCamOptions();
		}
		if (cQHY_CAM_16BITS)
		{
		}

		//================================================
		//*	if we have a GPS, see what we can do
		if (cQHY_CAM_GPS)
		{
			ConfigureGPS();
		}
	}
}

//*****************************************************************************
void	CameraDriverQHY::ConfigureGPS(void)
{
uint32_t	qhyRetCode;
int			ledCalMode	=	1;
double		addGPStoHdr	=	1;
double		gpsDoubleVal;

	CONSOLE_DEBUG("Checking GPS.");
	qhyRetCode	=	SetQHYCCDGPSLedCalMode(cQHYcamHandle, ledCalMode);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		CONSOLE_DEBUG("Set QHYCCD led cal mode success.");
	}
	else
	{
		CONSOLE_DEBUG("Set QHYCCD led cal mode fail.");
	}
	//ADD:Data Structure of the Image Head
	//The camera records the GPS information and insert into each frame's head. This function
	//can be enabled/disable by the API:
	qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CAM_GPS, addGPStoHdr);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		CONSOLE_DEBUG("Set QHYCCD CCDParam success.");
		if (addGPStoHdr != 0)
		{
			cQHYimageHasHeadrInfo	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG("Set QHYCCD CCDParam failed.");
	}
	gpsDoubleVal	=	GetQHYCCDParam(cQHYcamHandle, CAM_GPS);
	CONSOLE_DEBUG_W_DBL("gpsDoubleVal\t=", gpsDoubleVal);
}

//*****************************************************************************
void	CameraDriverQHY::CheckColorCamOptions(void)
{
uint32_t	qhyRetCode;

	//================================================
	//*	check if color
	qhyRetCode	=	IsQHYCCDControlAvailable(cQHYcamHandle, CAM_COLOR);
	if (qhyRetCode != QHYCCD_ERROR)
	{
		if (qhyRetCode == BAYER_GB || qhyRetCode == BAYER_GR || qhyRetCode == BAYER_BG || qhyRetCode == BAYER_RG)
		{
			CONSOLE_DEBUG("This is a color camera.");
		//	printf("even this is a color camera, in Single Frame mode THE SDK ONLY SUPPORT RAW OUTPUT.So please do not set SetQHYCCDDebayerOnOff() to true;");
			qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, true);
			CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
			qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBR, 20);
			CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
			qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBG, 20);
			CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
			qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBB, 20);
			CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
		}
		switch(qhyRetCode)
		{
			case BAYER_GB:	CONSOLE_DEBUG("the bayer format is BAYER_GB");	break;
			case BAYER_GR:	CONSOLE_DEBUG("the bayer format is BAYER_GR");	break;
			case BAYER_BG:	CONSOLE_DEBUG("the bayer format is BAYER_BG");	break;
			case BAYER_RG:	CONSOLE_DEBUG("the bayer format is BAYER_RG");	break;
			default:		CONSOLE_DEBUG("Unknown color bayer option");		break;
		}
	}
	else
	{
		CONSOLE_DEBUG("This is a mono camera");
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "AlpacaPi: Not implemented-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			qhyRetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	//Closed refrigeration
	if (cQHYcamHandle != NULL)
	{
		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_MANULPWM, 0.0);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not implemented");
		}
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_SensorTargetTemp(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not implemented");
//	CONSOLE_DEBUG(cLastCameraErrMsg);

	//*	temporary to clean up other stuff
	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Write_SensorTargetTemp(const double newCCDtargetTemp)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			qhyRetCode;

	//sleep(1000); //You don't have to set it too often, just once every second
	if (cQHYcamHandle != NULL)
	{
		//Set the target cooling temperature, automatic mode
		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_COOLER, newCCDtargetTemp);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG("Set target temperature successfully.");
			alpacaErrCode	=	kASCOM_Err_Success;
		}
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}


//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				cameraTemp_DegC;

//	if (gVerbose)
	{
		CONSOLE_DEBUG(__FUNCTION__);
	}

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		if (cQHYcamHandle != NULL)
		{
			cameraTemp_DegC	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURTEMP);
			CONSOLE_DEBUG_W_DBL("cameraTemp_DegC\t\t=", cameraTemp_DegC);

			cCameraProp.CCDtemperature	=	cameraTemp_DegC;
		}
		else
		{
			CONSOLE_DEBUG("cQHYcamHandle is NULL");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotSupported;
		strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				currentPWMlevel;

	if (cQHYcamHandle != NULL)
	{
		currentPWMlevel			=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURPWM);
		cCameraProp.CoolerPower	=	currentPWMlevel / 255.0 * 100.0;
//		CONSOLE_DEBUG_W_DBL("currentPWMlevel        \t=", currentPWMlevel);
//		CONSOLE_DEBUG_W_DBL("cCameraProp.CoolerPower\t=", cCameraProp.CoolerPower);
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InvalidOperation;
uint32_t			qhyRetCode;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, newGainValue);

	if (cQHYcamHandle != NULL)
	{
		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_GAIN, newGainValue);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			sprintf(cLastCameraErrMsg, "SetQHYCCDParam failed Err#%d", qhyRetCode);
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				rawGainValue;

//	if (gVerbose)
//	{
//		CONSOLE_DEBUG(__FUNCTION__);
//	}

	if (cQHYcamHandle != NULL)
	{
		rawGainValue	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_GAIN);
//		CONSOLE_DEBUG_W_DBL("rawGainValue\t\t=", rawGainValue);

		*cameraGainValue	=	rawGainValue;
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverQHY::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			qhyRetCode;
double				exposureDBL;
//double				offset;
//double				gain;
//double				speed;
//double				traffic;
//double				bit;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);

	if (cQHYcamHandle != NULL)
	{
		exposureDBL	=	exposureMicrosecs;
		CONSOLE_DEBUG("Calling SetQHYCCDParam(CONTROL_EXPOSURE)");
		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_EXPOSURE,	exposureDBL);
		if (qhyRetCode != QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_NUM("SetQHYCCDParam(CONTROL_EXPOSURE) returned\t=",	qhyRetCode);
		}
//		CONSOLE_DEBUG_W_DBL("exposureDBL\t=",	exposureDBL);
		CONSOLE_DEBUG_W_NUM("qhyRetCode\t=",	qhyRetCode);

//		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_GAIN,		gain);
//		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_OFFSET,		offset);
//		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_SPEED,		speed);
//		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_USBTRAFFIC,	traffic);
//		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_TRANSFERBIT,bit);
//		qhyRetCode	=	SetQHYCCDBinMode(cQHYcamHandle,	2,2);
//		qhyRetCode	=	SetQHYCCDResolution(cQHYcamHandle,	0,0,imagew/2,imageh/2);
//		qhyRetCode	=	SetQHYCCDResolution(cQHYcamHandle,	0,0,500,500);
//		qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle,	true);
//		qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle,	false);

		CONSOLE_DEBUG("Calling ExpQHYCCDSingleFrame()");
		qhyRetCode	=	ExpQHYCCDSingleFrame(cQHYcamHandle);
		CONSOLE_DEBUG_W_NUM("ExpQHYCCDSingleFrame() returned\t=",	qhyRetCode);
		CONSOLE_DEBUG_W_HEX("ExpQHYCCDSingleFrame() returned\t=",	qhyRetCode);

		switch(qhyRetCode)
		{
			case QHYCCD_READ_DIRECTLY:
				CONSOLE_DEBUG("QHYCCD_READ_DIRECTLY");
				alpacaErrCode			=	kASCOM_Err_Success;
				cInternalCameraState	=	kCameraState_TakingPicture;
				break;

			case QHYCCD_ERROR:
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				strcpy(cLastCameraErrMsg, "ExpQHYCCDSingleFrame() failed");
				CONSOLE_DEBUG(cLastCameraErrMsg);
				break;

			default:
				CONSOLE_DEBUG("No error");
				alpacaErrCode			=	kASCOM_Err_Success;
				cInternalCameraState	=	kCameraState_TakingPicture;
				break;
		}

		cLastExposure_ROIinfo	=	cROIinfo;
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
//	CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);
	return(alpacaErrCode);
}


//*****************************************************************************
bool	CameraDriverQHY::GetImage_ROI_info(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;
	return(true);
}

static int	gQHYcheckExposrureCtr	=	0;
//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverQHY::Check_Exposure(bool verboseFlag)
{
uint32_t				precentRemaining;
TYPE_EXPOSURE_STATUS	myExposureStatus;

	if (gVerbose)
	{
		CONSOLE_DEBUG_W_NUM(__FUNCTION__, gQHYcheckExposrureCtr++);
	}

	myExposureStatus	=	kExposure_Unknown;
	if (cQHYcamHandle != NULL)
	{
		precentRemaining	=	GetQHYCCDExposureRemaining(cQHYcamHandle);
//		CONSOLE_DEBUG_W_NUM("cInternalCameraState\t=",	cInternalCameraState);
//		CONSOLE_DEBUG_W_NUM("precentRemaining    \t=",	precentRemaining);

		if (precentRemaining > 0)
		{
			myExposureStatus	=	kExposure_Working;
			if (gVerbose)
			{
				CONSOLE_DEBUG_W_NUM("precentRemaining    \t=",	precentRemaining);
			}
		}
		else if (cInternalCameraState == kCameraState_TakingPicture)
		{
			myExposureStatus	=	kExposure_Success;
		}
		else
		{
			myExposureStatus	=	kExposure_Idle;
		}
	}
	else
	{
		myExposureStatus	=	kExposure_Unknown;
		strcpy(cLastCameraErrMsg, "cQHYcamHandle is NULL");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(myExposureStatus);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
uint32_t			qhyRetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("newImageType\t=",			newImageType);

	if (cQHYcamHandle != NULL)
	{
		cROIinfo.currentROIwidth	=	cCameraProp.CameraXsize;
		cROIinfo.currentROIheight	=	cCameraProp.CameraYsize;
		cROIinfo.currentROIbin		=   1;

		switch(newImageType)
		{
			case kImageType_RAW8:
			case kImageType_Y8:
				qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, false);
				CONSOLE_DEBUG_W_NUM("SetQHYCCDDebayerOnOff(false) returned\t=", qhyRetCode);

				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_TRANSFERBIT, 8);
				if (qhyRetCode == QHYCCD_SUCCESS)
				{
					CONSOLE_DEBUG("Set Image to 8 Bits Successfully");
					cROIinfo.currentROIimageType	=	kImageType_RAW8;
				}
				else
				{
					CONSOLE_DEBUG("Failed to set 8 Bits");
				}
				break;

			case kImageType_RAW16:
				qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, false);
				CONSOLE_DEBUG_W_NUM("SetQHYCCDDebayerOnOff(false) returned\t=", qhyRetCode);

				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_TRANSFERBIT, 16);
				CONSOLE_DEBUG_W_NUM("SetQHYCCDParam(16 bit) returned \t=", qhyRetCode);

				if (qhyRetCode == QHYCCD_SUCCESS)
				{
					CONSOLE_DEBUG("Set Image to 16 Bits Successfully");
					cROIinfo.currentROIimageType	=	kImageType_RAW16;
				}
				else
				{
					CONSOLE_DEBUG("Failed to set 16 Bits");
				}
				break;

			case kImageType_RGB24:
				qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, true);
				CONSOLE_DEBUG_W_NUM("SetQHYCCDDebayerOnOff/qhyRetCode\t=", qhyRetCode);

				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBR, 20);
//				CONSOLE_DEBUG_W_NUM("SetQHYCCDParam/qhyRetCode\t=", qhyRetCode);

				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBG, 20);
//				CONSOLE_DEBUG_W_NUM("SetQHYCCDParam/qhyRetCode\t=", qhyRetCode);

				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBB, 20);
//				CONSOLE_DEBUG_W_NUM("SetQHYCCDParam/qhyRetCode\t=", qhyRetCode);

				cROIinfo.currentROIimageType	=	kImageType_RGB24;
				break;


			case kImageType_Invalid:
			case kImageType_last:
				break;
		}
	}
	else
	{
		CONSOLE_DEBUG("cQHYcamHandle is NULL");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Camera is not open");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Start_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not finished");
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Stop_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not finished");
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Take_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(cLastCameraErrMsg, "Not finished");
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


////*****************************************************************************
//static void	DumpHex(uint8_t *dataBuffer, int length)
//{
//int		iii;
//int		jjj;
//char	theChar;
//
//	iii	=	0;
//	while (iii<length)
//	{
//		printf("%04X\t", iii);
//		for (jjj=0; jjj<16; jjj++)
//		{
//			printf("%02X ", (dataBuffer[iii+jjj] & 0x00ff));
//		}
//		printf("\t[");
//		for (jjj=0; jjj<16; jjj++)
//		{
//			theChar	=	(dataBuffer[iii+jjj] & 0x00ff);
//			if ((theChar < 0x20) || (theChar >= 0x7f))
//			{
//				theChar	=	'.';
//			}
//			printf("%c", theChar);
//		}
//		printf("]\r\n");
//		iii	+=	16;
//	}
//}


#define	PACKNUMBER(byte1, byte2, byte3, byte4)	\
					(((byte1 & 0x0ff) << 24) +	\
					 ((byte2 & 0x0ff) << 16) +	\
					 ((byte3 & 0x0ff) << 8) +	\
					 ((byte4 & 0x0ff)))

////**************************************************************************
//static void	QHY_CalcLatiude(int temp)
//{
//int		deg, minute, south;
//double	fractMin, latitude;
//	//	From the QHY SDK manual, Section 46, page 78
//	//latitude
//
//	south		=	temp > 1000000000;
//	deg			=	(temp % 1000000000) / 10000000;
//	minute		=	(temp % 10000000) / 100000;
//	fractMin	=	(temp % 100000) / 100000.0;
//	latitude	=	(deg + (minute + fractMin) / 60.0) * (south==0 ? 1 : -1);
//
//	CONSOLE_DEBUG_W_NUM(	"temp    \t=",	temp);
//	CONSOLE_DEBUG_W_NUM(	"deg     \t=",	deg);
//	CONSOLE_DEBUG_W_NUM(	"minute  \t=",	minute);
//	CONSOLE_DEBUG_W_NUM(	"south   \t=",	south);
//	CONSOLE_DEBUG_W_DBL(	"fractMin\t=",	fractMin);
//	CONSOLE_DEBUG_W_DBL(	"latitude\t=",	latitude);
//}


//**************************************************************************
//	Time format conversion:
//	Start_sec, end_sec, and now_sec are the total number of seconds from October 10, 1995 to the present,
//	which can be converted to the format of year, month, day, hour, minute, and second.
//**************************************************************************
static void	ConvertQHYtime(	time_t			qhyTimeSecs,
							long			microSeconds,
							struct timeval	*timeValStruct,
							char			*timeString)
{
struct tm		qhyStartTime;
time_t			qhyStartSecs;
time_t			newTimeSecs;

	memset(&qhyStartTime, 0, sizeof(qhyStartTime));
	//*	compute the number of seconds for Oct 10, 1995
	qhyStartTime.tm_year	=	95;
	qhyStartTime.tm_mon		=	10 - 1;
	qhyStartTime.tm_mday	=	10;

	qhyStartSecs	=	timegm(&qhyStartTime);
	newTimeSecs		=	qhyStartSecs + qhyTimeSecs;

	timeValStruct->tv_sec	=	newTimeSecs;
	timeValStruct->tv_usec	=	microSeconds;
	FormatTimeStringISO8601(timeValStruct, timeString);

//	gmtime_r(&newTimeSecs, linuxTime);
//	FormatTimeString_time_t(&newTimeSecs, timeString);
//	CONSOLE_DEBUG_W_STR("QHY Time:", timeString);
}

//**************************************************************************
//*	this is enabled if there is a GPS (QHY174)
//**************************************************************************
void	CameraDriverQHY::ParseQHYimageHeader(TYPE_QHY_RawImgHeader *rawImgHdr, TYPE_QHY_ImgHeader *imgHeader)
{
bool		isSouth;
bool		isWest;

//	CONSOLE_DEBUG("------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);

	memset(imgHeader, 0, sizeof(TYPE_QHY_ImgHeader));

	imgHeader->SequenceNumber		=	PACKNUMBER(	rawImgHdr->SequenceNumber_MSB,
													rawImgHdr->SequenceNumber_1,
													rawImgHdr->SequenceNumber_2,
													rawImgHdr->SequenceNumber_LSB);

	imgHeader->ImageWidth			=	PACKNUMBER(	0, 0,
													rawImgHdr->ImageWidth_MSB,
													rawImgHdr->ImageWidth_LSB);

	imgHeader->ImageHeight			=	PACKNUMBER(	0, 0,
													rawImgHdr->ImageHeight_MSB,
													rawImgHdr->ImageHeight_LSB);

	imgHeader->Latitude_Raw			=	PACKNUMBER(	rawImgHdr->Latitude_MSB,
													rawImgHdr->Latitude_10,
													rawImgHdr->Latitude_11,
													rawImgHdr->Latitude_LSB);
//	QHY_CalcLatiude(imgHeader->Latitude_Raw);

	imgHeader->Longitude_Raw		=	PACKNUMBER(	rawImgHdr->Longitude_MSB,
													rawImgHdr->Longitude_14,
													rawImgHdr->Longitude_15,
													rawImgHdr->Longitude_LSB);

	imgHeader->Start_Flag			=	rawImgHdr->Start_Flag;

	imgHeader->ShutterStartTimeJS	=	PACKNUMBER(	rawImgHdr->ShutterStartTimeJS_MSB,
													rawImgHdr->ShutterStartTimeJS_19,
													rawImgHdr->ShutterStartTimeJS_20,
													rawImgHdr->ShutterStartTimeJS_LSB);

	imgHeader->StartMicroSecond		=	PACKNUMBER(	0,
													rawImgHdr->StartMicroSecond_MSB,
													rawImgHdr->StartMicroSecond_23,
													rawImgHdr->StartMicroSecond_LSB);

	imgHeader->EndFlag				=	rawImgHdr->EndFlag;
	imgHeader->ShutterEndTimeJS		=	PACKNUMBER(	rawImgHdr->ShutterEndTimeJS_MSB,
													rawImgHdr->ShutterEndTimeJS_27,
													rawImgHdr->ShutterEndTimeJS_28,
													rawImgHdr->ShutterEndTimeJS_LSB);

	imgHeader->EndMicroSecond		=	PACKNUMBER(	0,
													rawImgHdr->EndMicroSecond_MSB,
													rawImgHdr->EndMicroSecond_31,
													rawImgHdr->EndMicroSecond_LSB);

	imgHeader->NowFlag				=	rawImgHdr->NowFlag;

	imgHeader->NowSecondJS			=	PACKNUMBER(	rawImgHdr->NowSecond_MSB,
													rawImgHdr->NowSecond_35,
													rawImgHdr->NowSecond_36,
													rawImgHdr->NowSecond_LSB);

	imgHeader->NowMicroSecond		=	PACKNUMBER(	0,
													rawImgHdr->NowMicroSecond_MSB,
													rawImgHdr->NowMicroSecond_39,
													rawImgHdr->NowMicroSecond_LSB);

	imgHeader->CountOfPPS			=	PACKNUMBER(	0,
													rawImgHdr->CountOfPPS_MSB,
													rawImgHdr->CountOfPPS_42,
													rawImgHdr->CountOfPPS_LSB);

	cGPS.SequenceNumber	=	imgHeader->SequenceNumber;
	cGPS.PPSC			=	imgHeader->CountOfPPS;
	cGPS.SU				=	imgHeader->StartMicroSecond;
	cGPS.EU				=	imgHeader->EndMicroSecond;
	cGPS.NU				=	imgHeader->NowMicroSecond;

	//=======================================================================
	//	From the QHY SDK manual, Section 46, page 78
	//latitude
	//int temp, deg, min, south;
	//double fractMin, latitude;
	//temp = 256*256*256*imageHead[9]+256*256*imageHead[10]+256*imageHead[11]+imageHead[12];
	//south = temp > 1000000000;
	//deg = (temp % 1000000000) / 10000000;
	//min = (temp % 10000000) / 100000;
	//fractMin = (temp % 100000) / 100000.0;
	//latitude = (deg + (min + fractMin) / 60.0) * (south==0 ? 1 : -1);
//	degrees		=	(imgHeader->Latitude_Raw % 1000000000) / 10000000;
//	minutes		=	(imgHeader->Latitude_Raw % 10000000) / 100000;
//	fractMin	=	(imgHeader->Latitude_Raw % 100000) / 100000.0;
//	latitude	=	(degrees + (minutes + fractMin) / 60.0);
	cGPS.Status	=	false;
	if (imgHeader->Latitude_Raw != 0)
	{
		isSouth					=	(imgHeader->Latitude_Raw > 1000000000);
		imgHeader->Latitude_Deg	=	((imgHeader->Latitude_Raw % 1000000000) / 10000000);	//*	7 zeros
		imgHeader->Latitude_Min	=	(imgHeader->Latitude_Raw % 10000000) / 100000.0;
		cGPS.Lat				=	imgHeader->Latitude_Deg + (imgHeader->Latitude_Min / 60.0);
		if (isSouth)
		{
			cGPS.Lat				=	-cGPS.Lat;
			imgHeader->Latitude_Deg	=	-imgHeader->Latitude_Deg;
		}
	}
//	CONSOLE_DEBUG_W_DBL("cGPS.Lat    \t=",	cGPS.Lat);
//	CONSOLE_DEBUG_W_NUM("Latitude_Deg\t=",	imgHeader->Latitude_Deg);
//	CONSOLE_DEBUG_W_DBL("Latitude_Min\t=",	imgHeader->Latitude_Min);

	//=======================================================================
	//longitude
	//int temp, deg, min, west;
	//double fractMin, longitude;
	//temp = 256*256*256*imageHead[13]+256*256*imageHead[14]+256*imageHead[15]+imageHead[16];
	//west = temp > 1000000000;
	//deg = (temp % 1000000000) / 1000000;
	//min = (temp % 1000000) / 10000;
	//fractMin = (temp % 10000) / 10000.0;
	//longitude = (deg + (min + fractMin) / 60.0) * (west==0 ? 1 : -1);

	if (imgHeader->Longitude_Raw > 0)
	{
		isWest						=	(imgHeader->Longitude_Raw / 1000000000);
		imgHeader->Longitude_Deg	=	(imgHeader->Longitude_Raw % 1000000000) / 1000000;
		imgHeader->Longitude_Min	=	(imgHeader->Longitude_Raw % 1000000) / 10000.0;	//*	4 zeros
		cGPS.Long					=	imgHeader->Longitude_Deg + (imgHeader->Longitude_Min / 60.0);
		if (isWest)
		{
			cGPS.Long					=	-cGPS.Long;
			imgHeader->Longitude_Deg	=	-imgHeader->Longitude_Deg;
		}
//		CONSOLE_DEBUG_W_DBL("cGPS.Long   \t=",	cGPS.Long);
//		CONSOLE_DEBUG_W_NUM("LongitudeDeg\t=",	imgHeader->Longitude_Deg);
//		CONSOLE_DEBUG_W_DBL("LongitudeMin\t=",	imgHeader->Longitude_Min);
	}
	//*	check for  valid GPS data
	if ((imgHeader->Latitude_Raw != 0) && (imgHeader->Longitude_Raw != 0))
	{
		cGPS.Status	=	true;
	}

//	CONSOLE_DEBUG_W_NUM("SequenceNumber    \t=",	imgHeader->SequenceNumber);
//	CONSOLE_DEBUG_W_NUM("ImageWidth        \t=",	imgHeader->ImageWidth);
//	CONSOLE_DEBUG_W_NUM("ImageHeight       \t=",	imgHeader->ImageHeight);
//	CONSOLE_DEBUG_W_NUM("Latitude_Raw      \t=",	imgHeader->Latitude_Raw);
//	CONSOLE_DEBUG_W_NUM("Latitude_Deg      \t=",	imgHeader->Latitude_Deg);
//	CONSOLE_DEBUG_W_DBL("Latitude_Min      \t=",	imgHeader->Latitude_Min);
//	CONSOLE_DEBUG_W_DBL("Latitude          \t=",	imgHeader->Latitude);
//	CONSOLE_DEBUG_W_NUM("Longitude_Raw     \t=",	imgHeader->Longitude_Raw);
//	CONSOLE_DEBUG_W_NUM("Longitude_Deg     \t=",	imgHeader->Longitude_Deg);
//	CONSOLE_DEBUG_W_DBL("Longitude_Min     \t=",	imgHeader->Longitude_Min);
//	CONSOLE_DEBUG_W_DBL("Longitude         \t=",	imgHeader->Longitude);
//	CONSOLE_DEBUG_W_HEX("Start_Flag        \t=",	imgHeader->Start_Flag);
//	CONSOLE_DEBUG_W_NUM("ShutterStartTimeJS\t=",	imgHeader->ShutterStartTimeJS);
//	CONSOLE_DEBUG_W_NUM("StartMicroSecond  \t=",	imgHeader->StartMicroSecond);
//	CONSOLE_DEBUG_W_HEX("EndFlag           \t=",	imgHeader->EndFlag);
//	CONSOLE_DEBUG_W_NUM("ShutterEndTimeJS  \t=",	imgHeader->ShutterEndTimeJS);
//	CONSOLE_DEBUG_W_NUM("EndMicroSecond    \t=",	imgHeader->EndMicroSecond);
//	CONSOLE_DEBUG_W_HEX("NowFlag           \t=",	imgHeader->NowFlag);
//	CONSOLE_DEBUG_W_NUM("NowSecondJS       \t=",	imgHeader->NowSecondJS);
//	CONSOLE_DEBUG_W_NUM("NowMicroSecond    \t=",	imgHeader->NowMicroSecond);
//	CONSOLE_DEBUG_W_NUM("CountOfPPS        \t=",	imgHeader->CountOfPPS);

	ConvertQHYtime(	imgHeader->ShutterStartTimeJS,
					imgHeader->StartMicroSecond,
					&imgHeader->ShutterStartTime,
					cGPS.ShutterStartTimeStr);

	ConvertQHYtime(	imgHeader->ShutterEndTimeJS,
					imgHeader->EndMicroSecond,
					&imgHeader->ShutterEndTime,
					cGPS.ShutterEndTimeStr);
	//-------------------------------------------------------
	//*	calculate the exposure time
	cGPS.Exposure_us	=	((imgHeader->ShutterEndTime.tv_sec - imgHeader->ShutterStartTime.tv_sec) / 1000000.0)+
							(imgHeader->ShutterEndTime.tv_usec - imgHeader->ShutterStartTime.tv_usec);

	//-------------------------------------------------------
	//*	calculate system clock offset
	ConvertQHYtime(	imgHeader->NowSecondJS,
					imgHeader->NowMicroSecond,
					&imgHeader->NowTime,
					cGPS.NowTimeStr);

//	CONSOLE_DEBUG_W_LONG("SysTime(secs)]=",	cGPS.SystemTime.tv_sec);
//	CONSOLE_DEBUG_W_LONG("GPSTime(secs)]=",	imgHeader->NowTime.tv_sec);

	cGPS.ClockDeltaSecs	=	((cGPS.SystemTime.tv_sec - imgHeader->NowTime.tv_sec) +
							((cGPS.SystemTime.tv_usec - imgHeader->NowTime.tv_usec) / 1000000.0));


//	CONSOLE_DEBUG_W_STR("Shutter Start Time\t=",	cGPS.ShutterStartTimeStr);
//	CONSOLE_DEBUG_W_STR("Shutter End Time  \t=",	cGPS.ShutterEndTimeStr);

//	ConvertQHYtime(imgHeader->ShutterEndTimeJS, &qhyTime);
//	ConvertQHYtime(imgHeader->NowSecond, &qhyTime);

//	CONSOLE_DEBUG("------------------------------------");
}

#define	kMaxNMEAdataLen	1024

//**************************************************************************
void	CameraDriverQHY::ProcessImageHeader(unsigned char *imageDataPtr)
{
TYPE_QHY_ImgHeader	imgHeader;
int					iii;
int					ccc;
char				theChar;
char				theChar2;
char				nmeaBuff[100];
short				calcChecksum;
short				rcvdChecksum;
int					nmeaStartIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	ParseQHYimageHeader((TYPE_QHY_RawImgHeader *)cCameraDataBuffer, &imgHeader);

	//========================================================
	//*	now deal with the GPS data, NMEA sentences
	GPS_ResetNMEAbuffer();

	//-------------------------------------------
	//*	look for the start of the GPS data
	//*	the binary header is 44 bytes long, start after that
	//*	we will also assume that as start location in case we dont find this sequence
	//*	this code was supplied by QinXiaoXu <qxx@qhyccd.com> at QHY
	nmeaStartIdx	=	44;
	for (iii=nmeaStartIdx; iii < 1024; iii++)
	{
		if (cCameraDataBuffer[iii] == 0x11)
		{
			if ((cCameraDataBuffer[iii + 1] == 0x22) &&
				(cCameraDataBuffer[iii + 2] == 0x33) &&
				(cCameraDataBuffer[iii + 3] == 0x66))
			{
				nmeaStartIdx	=	iii + 4;
				break;
			}
		}
	}
//	CONSOLE_DEBUG_W_NUM("nmeaStartIdx\t=", nmeaStartIdx);
	cGPS.NMEAerrCnt	=	0;
	iii				=	nmeaStartIdx;
	ccc				=	0;
	while (iii < kMaxNMEAdataLen)
	{
		theChar		=	cCameraDataBuffer[iii];
		theChar2	=	cCameraDataBuffer[iii + 1];
		if ((theChar == '$') && (theChar2 == 'G'))
		{
			//* we have the start of a proper NMEA sentence
			ccc			=	0;
			nmeaBuff[0]	=	0;
			while ((ccc < 99) && (iii < kMaxNMEAdataLen))
			{
				theChar			=	cCameraDataBuffer[iii];
				iii++;
				if (theChar >= 0x20)
				{
					if (ccc < 99)
					{
						nmeaBuff[ccc++]	=	theChar;
						nmeaBuff[ccc]	=	0;
					}
					else
					{
						CONSOLE_DEBUG_W_STR("Buffer overflow:", nmeaBuff);
					}
				}
				else if ((theChar == 0x0d) || (theChar == 0x0a))
				{
					if (strlen(nmeaBuff) > 10)
					{
						rcvdChecksum	=	ExtractChecksumFromNMEAline(nmeaBuff);
						calcChecksum	=	CalculateNMEACheckSum(nmeaBuff);
						if (calcChecksum == rcvdChecksum)
						{
//							CONSOLE_DEBUG_W_STR("OK!:", nmeaBuff);
							GPS_AddNMEAstring(nmeaBuff);
							ParseNMEAstring(&cGPSnmeaInfo, nmeaBuff);
						}
						else
						{
							cGPS.NMEAerrCnt++;
							CONSOLE_DEBUG_W_STR("BAD:", nmeaBuff);
						}
					}
					ccc			=	0;
					nmeaBuff[0]	=	0;
					break;
				}
				else
				{
//					CONSOLE_DEBUG_W_STR("invalid NMEA data\t=", nmeaBuff);
				}
			}
		}
		else
		{
			iii++;
		}
	}
	cGPS.SatsInView	=	atoi(cGPSnmeaInfo.theNN.NumSats);
	cGPS.SatMode1	=	cGPSnmeaInfo.currSatMode1;
	cGPS.SatMode2	=	cGPSnmeaInfo.currSatMode2;
	cGPS.Altitude	=	cGPSnmeaInfo.altitudeMeters;


	cGPS.DateValid	=	cGPSnmeaInfo.validDate;
	cGPS.TimeValid	=	cGPSnmeaInfo.validTime;
	cGPS.LaLoValid	=	cGPSnmeaInfo.validLatLon;
	cGPS.AltValid	=	cGPSnmeaInfo.validAlt;

	//*	check the status from the QHY header and if Lat/Lon not filled in and
	//*	we have at least a 2D fix, then copy over OUR findings
	if (cGPS.Status == false)
	{
		//*	OK, QHY did not find anything.
		if (cGPSnmeaInfo.validLatLon)
		{
			cGPS.Lat	=	GetLatLonDouble(&cGPSnmeaInfo.latitude);
			cGPS.Long	=	GetLatLonDouble(&cGPSnmeaInfo.longitude);
			cGPS.Status	=	true;

//			CONSOLE_DEBUG_W_DBL("cGPS.Lat \t=", cGPS.Lat);
//			CONSOLE_DEBUG_W_DBL("cGPS.Long\t=", cGPS.Long);
		}
	}
	//*	check again for Lat/Lon status
	if ((imgHeader.Latitude_Raw != 0) && (imgHeader.Longitude_Raw != 0))
	{
		cGPS.Status	=	true;
	}
//	CONSOLE_DEBUG_W_NUM("BAD NMEA packets\t=", cGPS.NMEAerrCnt);
//	DumpGPSdata(&cGPSnmeaInfo);
//	DumpHex(cCameraDataBuffer, 0x6000);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			qhyImgDataLen;
uint32_t			qhyRetCode;
unsigned int		iii;
unsigned int		imgWidth;
unsigned int		imgHeight;
unsigned int		imgSizeBytes;
unsigned int		bpp;	//*	bits per pixel
unsigned int		channels = 0;
bool				imageDataBuffOK;

//	if (gVerbose)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
	}

	if (cQHYcamHandle != NULL)
	{
		qhyImgDataLen	= GetQHYCCDMemLength(cQHYcamHandle);
		if (qhyImgDataLen > 0)
		{
			//	allocate
			imageDataBuffOK	=	AllocateImageBuffer(qhyImgDataLen * 2);
			if ((imageDataBuffOK) && (cCameraDataBuffer != NULL))
			{
//				memset(cCameraDataBuffer, 0xff, qhyImgDataLen);	//*	used for debugging
				memset(cCameraDataBuffer, 0, qhyImgDataLen);

				//*	this is for use with the QHY174-GPS camera
				gettimeofday(&cGPS.SystemTime, NULL);
				qhyRetCode	=	GetQHYCCDSingleFrame(	cQHYcamHandle,
														&imgWidth,
														&imgHeight,
														&bpp,		//*	bits per pixel
														&channels,
														cCameraDataBuffer);
				if (qhyRetCode == QHYCCD_SUCCESS)
				{

					imgSizeBytes	=	imgHeight * imgWidth * (bpp / 8) * channels;

//					CONSOLE_DEBUG_W_NUM("qhyImgDataLen\t=", qhyImgDataLen);
//					CONSOLE_DEBUG_W_NUM("imgWidth     \t=", imgWidth);
//					CONSOLE_DEBUG_W_NUM("imgHeight    \t=", imgHeight);
//					CONSOLE_DEBUG_W_NUM("bpp          \t=", bpp);
//					CONSOLE_DEBUG_W_NUM("channels     \t=", channels);
//					CONSOLE_DEBUG_W_NUM("imgSizeBytes \t=",	imgSizeBytes);


					//*	was the image header enabled
					if (cQHYimageHasHeadrInfo)
					{
						ProcessImageHeader(cCameraDataBuffer);

						//*	if we doing an overlay, it gets put at the top to overwrite the garbage
						if (cOverlayMode == 0)
						{
						unsigned int	imgOffset;
						unsigned char	*imgPtr;
							//*	move the image up in memory to remove the garbage header

							imgOffset	=	(imgWidth * 6 * (bpp / 8));
//							CONSOLE_DEBUG_W_NUM("imgOffset \t=",	imgOffset);
//							CONSOLE_DEBUG_W_HEX("imgOffset \t=",	imgOffset);
							imgPtr		=	cCameraDataBuffer + imgOffset;
							for (iii=0; iii<imgSizeBytes; iii++)
							{
								cCameraDataBuffer[iii]	=	imgPtr[iii] & 0x0ff;
							}
						}
						else
						{
							//*	set the data to black
							for (iii=0; iii<0x5330; iii++)
							{
								cCameraDataBuffer[iii]	=	0;
							}
						}
					}

					//--------------------------------------------
					//*	simulate image
					if (gSimulateCameraImage)
					{
						if (channels == 3)
						{
							CreateFakeImageData(cCameraDataBuffer, cCameraProp.CameraXsize, cCameraProp.CameraYsize, 3);
						}
					}
					cCameraProp.ImageReady	=	true;
					alpacaErrCode			=	kASCOM_Err_Success;
				}
				else
				{
					sprintf(cLastCameraErrMsg, "GetQHYCCDSingleFrame() failed with qhy return code:%d", qhyRetCode);
					CONSOLE_DEBUG(cLastCameraErrMsg);
					alpacaErrCode	=	kASCOM_Err_FailedToTakePicture;
				}
			}
			else
			{
			char	errorString[256];

				CONSOLE_DEBUG("Failed to allocate image data buffer");
				GetLinuxErrorString(errno, errorString);
				CONSOLE_DEBUG(errorString);
			}
		}
		else
		{
			CONSOLE_DEBUG("Failed to get image length");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "Failed to open connection to camera");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

#ifdef _USE_QHY_TIME_CODE_
//******************************************************************************
//******************************************************************************
//******************************************************************************
//*	QHY time conversion routines
typedef struct
{
	uint16_t year;
	uint16_t month;
	uint16_t date;
	uint16_t hour;
	uint16_t min;
	uint16_t sec;
	uint16_t week;
} drive_time;;

//drive_time UTC;
//
//////******************************************************************************
////Initialization time
//drive_time struct_time =
//{
//	.year	=	1995,
//	.month	=	10,
//	.date	=	10,
//	.hour	=	0,
//	.min	=	0,
//	.sec	=	0,
//};

////******************************************************************************
//bool isLeapYear( int year )
//{
//	if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
//	{
//		return true;
//	}
//	return false;
//}

////******************************************************************************
////To get utc time, add 8 hours to convert to Beijing time
//int get_UTC(unsigned long second, drive_time *UTC)
//{
//const char Leap_Year_day[2][12]	=	{ {31,28,31,30,31,30,31,31,30,31,30,31},{31,29,31,30,31,30,31,31,30,31,30,31} };
//int Leap_Year	=	0;
//int month_day	=	0;
//
//	Leap_Year	=	isLeapYear(struct_time.year);
//	month_day	=	Leap_Year_day[Leap_Year][struct_time.month-1];
//	UTC->year	=	struct_time.year;
//	UTC->month	=	struct_time.month;
//	UTC->date	=	struct_time.date;
//	UTC->hour	=	struct_time.hour +(second / 3600 % 24);
//
//	UTC->min	=	struct_time.min+ (second / 60 % 60);
//	UTC->sec	=	struct_time.sec +(second % 60);
//	uint16_t count_days	=	second / 86400;
//	if (UTC->sec >=60)
//	{
//		UTC->sec	=	UTC->sec%60;
//		(UTC->min) ++;
//	}
//	if (UTC->min >=60)
//	{
//		UTC->min	=	UTC->min%60;
//		(UTC->hour) ++;
//	}
//	if (UTC->hour >=24)
//	{
//		UTC->hour	=	UTC->hour%24;
//		(count_days) ++;
//	}
//	for (int i = 0 ; i < count_days; i++ )
//	{
//		Leap_Year	=	isLeapYear(UTC->year);
//		month_day	=	Leap_Year_day[Leap_Year][(UTC->month)-1];
//	}
//	return(0);
//}


//(UTC->date) ++;
//if ((UTC->date) > month_day)
//{
//	(UTC->date) = 1;
//	(UTC->month) ++;
//	if ((UTC->month) > 12)
//	{
//		(UTC->month) = 1;
//		(UTC->year) ++;
//		if ( ( (UTC->year) - (struct_time.year) ) >100)
//			return -1;
//	}
//}
//return
//0;
//get_UTC(start_sec, &UTC);
//printf("%d %d %d %d %d %d %d\n", UTC.year, UTC.month, UTC.date, UTC.hour, UTC.min, UTC.sec);
#endif // _USE_QHY_TIME_CODE_


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
