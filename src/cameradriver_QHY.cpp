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
//*	Redistributions of this source code must retain this copyright notice.
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
//*	Feb 20,	2021	<MLS> QHY Color image working
//*	Feb 21,	2021	<MLS> Set ReadoutMode working for QHY
//*	Sep 18,	2021	<MLS> Trying to use QHY camera for guide scope
//*	Sep 18,	2021	<MLS> Fixed memory leak in CameraDriverQHY::Read_ImageData()
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)


#include	"../QHY/include/qhyccd.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"cameradriver.h"
#include	"cameradriver_QHY.h"
#include	"linuxerrors.h"


//**************************************************************************************
void	CreateQHY_CameraObjects(void)
{
uint32_t		qhyRetCode;
uint32_t		year;
uint32_t		month;
uint32_t		day;
uint32_t		subday;
int				qhyCameraCnt;
int				iii;
char			qhyIDstring[64];
char			rulesFileName[]	=	"85-qhyccd.rules";
char			driverVersionString[64];
bool			rulesFileOK;

	CONSOLE_DEBUG(__FUNCTION__);


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

			AddLibraryVersion("camera", "QHY", driverVersionString);
		}
		else
		{
			CONSOLE_DEBUG_W_INT32("qhyRetCode\t=",	qhyRetCode);
			strcpy(driverVersionString, "Failed to read version");
		}
		LogEvent(	"camera",
					"Library version (QHY)",
					NULL,
					kASCOM_Err_Success,
					driverVersionString);

		qhyCameraCnt	=	ScanQHYCCD();
		CONSOLE_DEBUG_W_INT32("qhyCameraCnt\t=",	qhyCameraCnt);
		for(iii=0; iii < qhyCameraCnt; iii++)
		{

			qhyRetCode = GetQHYCCDId(iii, qhyIDstring);
			if (qhyRetCode == QHYCCD_SUCCESS)
			{
				CONSOLE_DEBUG_W_STR("Creating driver object for\t=",	qhyIDstring);
				new CameraDriverQHY(0, qhyIDstring);
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
	cQHY_NumberOfReadModes	=	0;

	strcpy(cDeviceManufAbrev,	"QHY");
	strcpy(cQHYidString, qhyIDstring);

	ReadQHYcameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverQHY::~CameraDriverQHY(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
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

	qhyRetCode	=	GetQHYCCDModel(cQHYidString, qhyModelString);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		CONSOLE_DEBUG_W_STR("qhyModelString\t=", qhyModelString);
		strcpy(cDeviceModel,		qhyModelString);
		strcpy(cCommonProp.Name,	"QHY-");
		strcat(cCommonProp.Name,	qhyModelString);
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

		//*	get the control information from the camera
		for (controlID=0; controlID<CONTROL_MAX_ID; controlID++)
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
					break;

				case CONTROL_CURPWM:			//!< current cool pwm
					CONSOLE_DEBUG_W_STR("CONTROL_CURPWM\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_MANULPWM:			//!< set the cool pwm
					CONSOLE_DEBUG_W_STR("CONTROL_MANULPWM\t\t=",	(controlValid ? "YES" : "NO"));
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
					cSt4Port		=	controlValid;
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
					cTempReadSupported	=	controlValid;
					CONSOLE_DEBUG_W_STR("CAM_CHIPTEMPERATURESENSOR_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
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
			qhyRetCode	=	IsQHYCCDControlAvailable(cQHYcamHandle, CAM_COLOR);
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
		}
		if (cQHY_CAM_16BITS)
		{
		}
	}
}

//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				cameraTemp_DegC;

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		if (cQHYcamHandle != NULL)
		{
			cameraTemp_DegC	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURTEMP);
//			CONSOLE_DEBUG_W_DBL("cameraTemp_DegC\t\t=", cameraTemp_DegC);

			cCameraProp.CCDtemperature	=	cameraTemp_DegC;
		}
		else
		{
			CONSOLE_DEBUG("cQHYcamHandle is NULL");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			strcpy(cLastCameraErrMsg, "Camera is not open");
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
TYPE_ASCOM_STATUS	CameraDriverQHY::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InvalidOperation;
uint32_t			qhyRetCode;

	CONSOLE_DEBUG_W_NUM(__FUNCTION__, newGainValue);

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
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "Failed to open connection to camera");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				rawGainValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cQHYcamHandle != NULL)
	{
		rawGainValue	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_GAIN);
//		CONSOLE_DEBUG_W_DBL("rawGainValue\t\t=", rawGainValue);

		*cameraGainValue	=	rawGainValue;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "Failed to open connection to camera");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverQHY::Start_CameraExposure(int32_t exposureMicrosecs)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			qhyRetCode;
double				exposureDBL;
//double				offset;
//double				gain;
//double				speed;
//double				traffic;
//double				bit;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);

	if (cQHYcamHandle != NULL)
	{
		exposureDBL	=	exposureMicrosecs;
		qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle,	CONTROL_EXPOSURE,	exposureDBL);

//		CONSOLE_DEBUG_W_DBL("exposureDBL\t=",	exposureDBL);
//		CONSOLE_DEBUG_W_NUM("qhyRetCode\t=",	qhyRetCode);

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

		qhyRetCode	=	ExpQHYCCDSingleFrame(cQHYcamHandle);
//		CONSOLE_DEBUG_W_NUM("ExpQHYCCDSingleFrame() returned\t=",	qhyRetCode);
//		CONSOLE_DEBUG_W_HEX("ExpQHYCCDSingleFrame() returned\t=",	qhyRetCode);


		if (qhyRetCode == QHYCCD_READ_DIRECTLY)
		{
//			CONSOLE_DEBUG("QHYCCD_READ_DIRECTLY");
			alpacaErrCode			=	kASCOM_Err_Success;
			cInternalCameraState	=	kCameraState_TakingPicture;
		}
		else if (qhyRetCode != QHYCCD_ERROR)
		{
//			CONSOLE_DEBUG("No error");
			alpacaErrCode			=	kASCOM_Err_Success;
			cInternalCameraState	=	kCameraState_TakingPicture;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			strcpy(cLastCameraErrMsg, "ExpQHYCCDSingleFrame() failed");
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}

		cLastExposure_ROIinfo	=	cROIinfo;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		strcpy(cLastCameraErrMsg, "Failed to open connection to camera");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
//	CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",	cROIinfo.currentROIimageType);
	return(alpacaErrCode);
}


//*****************************************************************************
bool	CameraDriverQHY::GetImage_ROI_info(void)
{

	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;
	return(true);
}

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverQHY::Check_Exposure(bool verboseFlag)
{
uint32_t				precentRemaining;
TYPE_EXPOSURE_STATUS	myExposureStatus;

//	CONSOLE_DEBUG(__FUNCTION__);

	myExposureStatus	=	kExposure_Unknown;
	if (cQHYcamHandle != NULL)
	{
		precentRemaining	=	GetQHYCCDExposureRemaining(cQHYcamHandle);
//		CONSOLE_DEBUG_W_NUM("cInternalCameraState\t=",	cInternalCameraState);
//		CONSOLE_DEBUG_W_NUM("precentRemaining    \t=",	precentRemaining);

		if (precentRemaining > 0)
		{
			CONSOLE_DEBUG_W_NUM("precentRemaining    \t=",	precentRemaining);
			myExposureStatus	=	kExposure_Working;
		}
		else if (cInternalCameraState == kCameraState_TakingPicture)
		{
			myExposureStatus		=	kExposure_Success;
		}
		else
		{
			myExposureStatus	=	kExposure_Idle;
		}
	}
	else
	{
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
			case kImageType_RAW16:
				qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, false);
				CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
				cROIinfo.currentROIimageType	=	kImageType_RAW8;
//+				cROIinfo.currentROIimageType	=	kImageType_RAW16;
				break;


			case kImageType_RGB24:
				qhyRetCode	=	SetQHYCCDDebayerOnOff(cQHYcamHandle, true);
				CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBR, 20);
				CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBG, 20);
				CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);
				qhyRetCode	=	SetQHYCCDParam(cQHYcamHandle, CONTROL_WBB, 20);
				CONSOLE_DEBUG_W_NUM("qhyRetCode\t=", qhyRetCode);

				cROIinfo.currentROIimageType	=	kImageType_RGB24;
				break;

			case kImageType_Y8:
				cROIinfo.currentROIimageType	=	kImageType_Y8;
				break;

			case kImageType_Invalid:
			case kImageType_last:
				break;

		}


	}
	else
	{
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}


//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
uint32_t			imgDataLength;
uint32_t			qhyRetCode;
unsigned int		imgWidth;
unsigned int		imgHeight;
unsigned int		bpp;
unsigned int		channels = 0;
//unsigned char 		*imgDataPtr;
bool				imageDataBuffOK;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cQHYcamHandle != NULL)
	{
		imgDataLength	= GetQHYCCDMemLength(cQHYcamHandle);
		if(imgDataLength > 0)
		{
//			CONSOLE_DEBUG_W_NUM("imgDataLength\t=", imgDataLength);

			imageDataBuffOK	=	AllcateImageBuffer(imgDataLength * 2);
			if ((imageDataBuffOK) && (cCameraDataBuffer != NULL))
			{
				memset(cCameraDataBuffer, 0, imgDataLength);

				qhyRetCode	=	GetQHYCCDSingleFrame(	cQHYcamHandle,
														&imgWidth,
														&imgHeight,
														&bpp,
														&channels,
														cCameraDataBuffer);
				if (qhyRetCode == QHYCCD_SUCCESS)
				{
	//				CONSOLE_DEBUG_W_NUM("imgWidth\t=", imgWidth);
	//				CONSOLE_DEBUG_W_NUM("imgHeight\t=", imgHeight);
	//				CONSOLE_DEBUG_W_NUM("bpp\t=", bpp);
	//				CONSOLE_DEBUG_W_NUM("channels\t=", channels);

					cCameraDataBuffer		=	cCameraDataBuffer;

					cCameraProp.ImageReady	=	true;
					alpacaErrCode			=	kASCOM_Err_Success;
				}
				else
				{
					sprintf(cLastCameraErrMsg, "GetQHYCCDSingleFrame() failed with qhy return code:", qhyRetCode);
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


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
