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
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)


#include	"../QHY/include/qhyccd.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"cameradriver_QHY.h"


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
	if (rulesFileOK == false)
	{
		LogEvent(	"camera",
					"Problem with QHY rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

//	EnableQHYCCDMessage(false);
	SetQHYCCDLogLevel(0);

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
			CONSOLE_DEBUG_W_INT32("qhyRetCode\t=",	qhyRetCode);
			CONSOLE_DEBUG_W_INT32("year\t\t=",		year);
			CONSOLE_DEBUG_W_INT32("month\t\t=",		month);
			CONSOLE_DEBUG_W_INT32("day\t\t=",		day);
			CONSOLE_DEBUG_W_INT32("subday\t\t=",	subday);

			AddLibraryVersion("camera", "QHY", driverVersionString);

		}
		else
		{
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
				CONSOLE_DEBUG_W_STR("qhyIDstring\t=",	qhyIDstring);
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
	cCameraID		=	deviceNum;
	cQHYcamHandle	=	NULL;

	strcpy(cDeviceManufAbrev,	"QHY");
	strcpy(cQHYidString, qhyIDstring);

	ReadQHYcameraInfo();

	strcpy(cDeviceDescription, cDeviceManufacturer);
	strcat(cDeviceDescription, " - Model:");
	strcat(cDeviceDescription, cDeviceName);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cDeviceName, cCameraID);
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
double			chipw,chiph,pixelw,pixelh;
char			qhyModelString[64];
int				controlID;
bool			controlValid;
uint8_t			firmwareBuff[16];
double			humidity;
uint32_t		iii;
uint32_t		numModes;
char			modeName[32];

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceManufacturer,	"QHY");

	qhyRetCode	=	GetQHYCCDModel(cQHYidString, qhyModelString);
	if (qhyRetCode == QHYCCD_SUCCESS)
	{
		CONSOLE_DEBUG_W_STR("qhyModelString\t=", qhyModelString);
		strcpy(cDeviceModel,	qhyModelString);
		strcpy(cDeviceName,		qhyModelString);
	}

	cQHYcamHandle	=	OpenQHYCCD(cQHYidString);
	if (cQHYcamHandle != NULL)
	{
		printf("Open QHYCCD success!\n");
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
			printf("GetQHYCCDChipInfo success!\n");
			printf("CCD/CMOS chip information:\n");
			printf("Chip width %3f mm,Chip height %3f mm\n",chipw,chiph);
			printf("Chip pixel width %3f um,Chip pixel height %3f um\n",pixelw,pixelh);
			printf("Chip Max Resolution is %d x %d,depth is %d\n",width, height, bpp);

			cBitDepth		=	bpp;
			cCameraXsize	=	width;
			cCameraYsize	=	height;
			cPixelSizeX		=	pixelw;
			cPixelSizeY		=	pixelh;

			cNumX			=	cCameraXsize;
			cNumY			=	cCameraYsize;
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
			else
			{
				controlValid	=	true;
				CONSOLE_DEBUG_W_INT32("IsQHYCCDControlAvailable() controlID\t=",	controlID);
				CONSOLE_DEBUG_W_INT32("IsQHYCCDControlAvailable() qhyRetCode\t=",	qhyRetCode);
			}
			switch(controlID)
			{
				case CONTROL_BRIGHTNESS:		//!< image brightness
					break;

				case CONTROL_CONTRAST:			//!< image contrast
					break;

				case CONTROL_WBR:				//!< red of white balance
					break;

				case CONTROL_WBB:				//!< blue of white balance
					break;

				case CONTROL_WBG:				//!< the green of white balance
					break;

				case CONTROL_GAMMA:				//!< screen gamma
					break;

				case CONTROL_GAIN:				//!< camera gain
					break;

				case CONTROL_OFFSET:			//!< camera offset
					break;

				case CONTROL_EXPOSURE:			//!< expose time (us)
					break;

				case CONTROL_SPEED:				//!< transfer speed
					break;

				case CONTROL_TRANSFERBIT:		//!< image depth bits
					break;

				case CONTROL_CHANNELS:			//!< image channels
					break;

				case CONTROL_USBTRAFFIC:		//!< hblank
					break;

				case CONTROL_ROWNOISERE:		//!< row denoise
					break;

				case CONTROL_CURTEMP:			//!< current cmos or ccd temprature
					CONSOLE_DEBUG_W_STR("CONTROL_CURTEMP\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CURPWM:			//!< current cool pwm
					CONSOLE_DEBUG_W_STR("CONTROL_CURPWM\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_MANULPWM:			//!< set the cool pwm
					break;

				case CONTROL_CFWPORT:			//!< control camera color filter wheel port
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
					break;

				case CAM_BIN1X1MODE:				//!< check if camera has bin1x1 mode
					if (controlValid)
					{
						cMaxbinX	=	1;
						cMaxbinY	=	1;
					}
					break;

				case CAM_BIN2X2MODE:				//!< check if camera has bin2x2 mode
					if (controlValid)
					{
						cMaxbinX	=	2;
						cMaxbinY	=	2;
					}
					break;

				case CAM_BIN3X3MODE:				//!< check if camera has bin3x3 mode
					if (controlValid)
					{
						cMaxbinX	=	3;
						cMaxbinY	=	3;
					}
					break;

				case CAM_BIN4X4MODE:				//!< check if camera has bin4x4 mode
					if (controlValid)
					{
						cMaxbinX	=	4;
						cMaxbinY	=	4;
					}
					break;

				case CAM_MECHANICALSHUTTER:					//!< mechanical shutter
					cHasShutter	=	controlValid;
					break;

				case CAM_TRIGER_INTERFACE:					//!< triger
					cIsTriggerCam	=	controlValid;
					break;

				case CAM_TECOVERPROTECT_INTERFACE:			//!< tec overprotect
					break;

				case CAM_SINGNALCLAMP_INTERFACE:			//!< singnal clamp
					break;

				case CAM_FINETONE_INTERFACE:				//!< fine tone
					break;

				case CAM_SHUTTERMOTORHEATING_INTERFACE:		//!< shutter motor heating
					break;

				case CAM_CALIBRATEFPN_INTERFACE:			//!< calibrated frame
					break;

				case CAM_CHIPTEMPERATURESENSOR_INTERFACE:	//!< chip temperaure sensor
					cTempReadSupported	=	controlValid;
					CONSOLE_DEBUG_W_STR("CAM_CHIPTEMPERATURESENSOR_INTERFACE\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_USBREADOUTSLOWEST_INTERFACE:		//!< usb readout slowest
					break;

				case CAM_8BITS:								//!< 8bit depth
					CONSOLE_DEBUG_W_STR("CAM_8BITS\t\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_16BITS:							//!< 16bit depth
					CONSOLE_DEBUG_W_STR("CAM_16BITS\t\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CAM_GPS:								//!< check if camera has gps
					break;

				case CAM_IGNOREOVERSCAN_INTERFACE:			//!< ignore overscan area
					break;

				case QHYCCD_3A_AUTOBALANCE:
					break;

				case QHYCCD_3A_AUTOEXPOSURE:
					break;

				case QHYCCD_3A_AUTOFOCUS:
					break;

				case CONTROL_AMPV:							//!< ccd or cmos ampv
					CONSOLE_DEBUG_W_STR("CONTROL_AMPV\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_VCAM:							//!< Virtual Camera on off
					break;

				case CAM_VIEW_MODE:
					CONSOLE_DEBUG_W_STR("CAM_VIEW_MODE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case CONTROL_CFWSLOTSNUM:					//!< check CFW slots number
					break;

				case IS_EXPOSING_DONE:
					CONSOLE_DEBUG_W_STR("IS_EXPOSING_DONE\t\t=",	(controlValid ? "YES" : "NO"));
					break;

				case ScreenStretchB:
					break;

				case ScreenStretchW:
					break;

				case CONTROL_DDR:
					break;

				case CAM_LIGHT_PERFORMANCE_MODE:
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
		}
		CONSOLE_DEBUG_W_NUM("cMaxbinX\t\t\t=",	cMaxbinX);

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
double	controlMin;
double	controlMax;
double	controlStep;
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
			cGainMin	=	controlMin;
			cGainMax	=	controlMax;
		}
		//===============================================================
		//*	Exposure min/max/step
		//*	CONTROL_EXPOSURE,       //!< expose time (us)
		qhyRetCode	=	GetQHYCCDParamMinMaxStep(cQHYcamHandle,
												CONTROL_EXPOSURE,
												&controlMin,
												&controlMax,
												&controlStep);
		if (qhyRetCode == QHYCCD_SUCCESS)
		{
			CONSOLE_DEBUG_W_DBL("exposure min\t\t=", controlMin);
			CONSOLE_DEBUG_W_DBL("exposure max\t\t=", controlMax);
			cExposureMin_us	=	controlMin;
			cExposureMax_us	=	controlMax;
		}


//		if (cTempReadSupported)
		{
		double	controlValue;

			controlValue	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURTEMP);
			CONSOLE_DEBUG_W_DBL("CONTROL_CURTEMP\t\t=", controlValue);
		}
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_Readoutmodes(char *readOutModeString, bool includeQuotes)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	strcpy(readOutModeString, "RGB24");


	return(alpacaErrCode);
}

//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQHY::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				cameraTemp_DegC;

	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		if (cQHYcamHandle != NULL)
		{
			cameraTemp_DegC	=	GetQHYCCDParam(cQHYcamHandle, CONTROL_CURTEMP);
			CONSOLE_DEBUG_W_DBL("cameraTemp_DegC\t\t=", cameraTemp_DegC);

			cCameraTemp_Dbl	=	cameraTemp_DegC;
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
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cQHYcamHandle != NULL)
	{

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
		CONSOLE_DEBUG_W_DBL("rawGainValue\t\t=", rawGainValue);

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


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
