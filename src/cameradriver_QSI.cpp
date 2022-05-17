//**************************************************************************
//*	Name:			cameradriver_QSI.cpp
//*
//*	Author:			Mark Sproul (C) 2022
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
//*			https://qsimaging.com/
//*
//*			sudo apt-get install libftdi-devel		??????
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr  7,	2022	<MLS> Created cameradriver_QSI.cpp
//*	Apr  8,	2022	<MLS> Got QSI library to compile and install
//*	Apr 16,	2022	<MLS> Added simulation option to QSI camera driver
//*	Apr 17,	2022	<JMH> Making progress on QSI camera
//*	May 14,	2022	<MLS> JMH was able to take a picture
//*	May 14,	2022	<MLS> Working on cooler support for QSI camera
//*	May 15,	2022	<MLS> Finished Read_CoolerPowerLevel()
//*	May 17,	2022	<MLS> Implemented Read_CoolerState() for QSI camera
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QSI_)


#include	<string>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"cameradriver.h"
#include	"cameradriver_QSI.h"
#include	"linuxerrors.h"

//*	qsiapi-7.6.0/lib/
#include	"qsiapi.h"
#include	"QSIError.h"


#define	_SIMULATE_CAMERA_

//**************************************************************************************
void	CreateQSI_CameraObjects(void)
{
//int				iii;
unsigned int	qsi_Result;
std::string		info	=	"";
std::string		modelNumber("");
std::string		camSerial[QSICamera::MAXCAMERAS];
std::string		camDesc[QSICamera::MAXCAMERAS];
std::string		text;
std::string		lastError("");
int				numCamerasFound;
QSICamera		cQSIcam;

	CONSOLE_DEBUG(__FUNCTION__);

	numCamerasFound	=	0;
	camSerial[0]	=	"";
	cQSIcam.put_UseStructuredExceptions(false);
	qsi_Result	=	cQSIcam.get_DriverInfo(info);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG(info.c_str());

		AddLibraryVersion("camera", "QSI", info.c_str());
		LogEvent(	"camera",
					"Library version (QSI)",
					NULL,
					kASCOM_Err_Success,
					info.c_str());
	}
	else
	{
		CONSOLE_DEBUG("Error getting get_DriverInfo()");
	}

	//*	Discover the connected cameras
	qsi_Result	=	cQSIcam.get_AvailableCameras(camSerial, camDesc, numCamerasFound);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_NUM("numCamerasFound\t=", numCamerasFound);

//		if (numCamerasFound > 0)
//		{
//			for (iii=0; iii < numCamerasFound; iii++)
//			{
//				new CameraDriverQSI(iii, camSerial[iii]);
//			}
//		}
//		else
		{
			//*	for debugging without a camera
			new CameraDriverQSI(0, camSerial[0]);
		}
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG(lastError.c_str());
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
CameraDriverQSI::CameraDriverQSI(	const int	deviceNum,
									std::string qsiSerialNumber)
					:CameraDriver()
{
unsigned int	qsi_Result;
bool			cameraInfoOK;
bool			isConnected;
//std::string		text		=	"";
std::string		lastError	=	"";
std::string		info		=	"";
std::string		camSerial[QSICamera::MAXCAMERAS];
std::string		camDesc[QSICamera::MAXCAMERAS];
int				numCamerasFound;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("Creating QSI device number ", deviceNum);

	gVerbose				=	true;
	cVerboseDebug			=	true;

	cCameraID				=	deviceNum;

	//*	set defaults
	strcpy(cDeviceManufAbrev,		"QSI");
	strcpy(cCommonProp.Name,		"QSI");
	strcpy(cCommonProp.Description,	"QSI");
	strcpy(cCommonProp.DriverInfo,	"QSI");

	cTempReadSupported	=	true;

	qsi_Result	=	cQSIcam.put_UseStructuredExceptions(false);
	qsi_Result	=	cQSIcam.get_DriverInfo(info);
	if (qsi_Result == QSI_OK)
	{
		strcpy(cCommonProp.DriverInfo,	info.c_str());
	}

	qsi_Result	=	cQSIcam.get_AvailableCameras(camSerial, camDesc, numCamerasFound);
	if (qsi_Result != QSI_OK)
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_AvailableCameras)\t=",	lastError.c_str());
	}

	cQSIserialNumber	=	qsiSerialNumber;
	CONSOLE_DEBUG_W_STR("cQSIserialNumber\t=",	cQSIserialNumber.c_str());

	isConnected		=	AlpacaConnect();
	if (isConnected)
	{

	}
	else
	{
		CONSOLE_DEBUG("Failed to Connect!!!!!!!!");
	}
#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		strcpy(cDeviceManufacturer,	"QSI");
		cCameraProp.CameraXsize			=	4000;
		cCameraProp.CameraYsize			=	3000;
		cCameraProp.NumX				=	4000;
		cCameraProp.NumY				=	3000;

		cCameraProp.GainMin				=	0;
		cCameraProp.GainMax				=	10;
		cCameraProp.ElectronsPerADU		=	2.091615;
		cCameraProp.PixelSizeX			=	3.7;
		cCameraProp.PixelSizeY			=	3.7;
		cCameraProp.FullWellCapacity	=	8567.253906;

		cCameraProp.Cansetccdtemperature	=	true;
		cCameraProp.CanGetCoolerPower		=	true;
		cIsCoolerCam						=	true;

		AddReadoutModeToList(kImageType_RAW16);
	}
	else
#endif
	{
		cameraInfoOK	=	ReadQSIcameraInfo();
		if (cameraInfoOK)
		{
			strcpy(cCommonProp.Description, cDeviceManufacturer);
			strcat(cCommonProp.Description, " - Model:");
			strcat(cCommonProp.Description, cCommonProp.Name);
		}
		else
		{
			CONSOLE_DEBUG("Failed to read QSI camera info!!!!!!!!");
		}
	}

	DumpCameraProperties(__FUNCTION__);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

}



//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverQSI::~CameraDriverQSI(void)
{
bool			isConnected;
unsigned int	qsi_Result;
std::string		lastError("");

	CONSOLE_DEBUG(__FUNCTION__);
	//---------------------------------------------------------
	//*	check to see if the camera is already connected
	qsi_Result	=	cQSIcam.get_Connected(&isConnected);
	if ((qsi_Result == QSI_OK) && (isConnected == true))
	{
		// Disconnect from the camera to the selected camera and retrieve camera parameters
		qsi_Result	=	cQSIcam.put_Connected(false);
		if (qsi_Result == QSI_OK)
		{
			CONSOLE_DEBUG("cQSIcam.put_Connected worked, now disconnected");
		}
		else
		{
			cQSIcam.get_LastError(lastError);
			CONSOLE_DEBUG_W_STR("QSI Result (put_Connected)\t=",	lastError.c_str());
		}
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Resul (get_Connected)\t=",	lastError.c_str());
	}
}


//*****************************************************************************
bool	CameraDriverQSI::AlpacaConnect(void)
{
bool			isMain;
bool			isConnected;
unsigned int	qsi_Result;
std::string		text;
std::string		lastError("");

	CONSOLE_DEBUG(__FUNCTION__);
	isConnected	=	false;
	qsi_Result	=	cQSIcam.put_SelectCamera(cQSIserialNumber);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_STR("cQSIcam.put_SelectCamera worked\t=",	cQSIserialNumber.c_str());
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result, put_SelectCamera\t=",	lastError.c_str());
	}

	// Get the current selected camera role
	// Either main or guider
	qsi_Result	=	cQSIcam.get_IsMainCamera(&isMain);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_NUM("cQSIcam.get_IsMainCamera\t=",	isMain);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result\t=",	lastError.c_str());
	}

	// Set the camera role to main camera (not guider)
	qsi_Result	=	cQSIcam.put_IsMainCamera(true);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG("cQSIcam.put_IsMainCamera worked");
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (put_IsMainCamera)\t=",	lastError.c_str());
	}

	//---------------------------------------------------------
	//*	check to see if the camera is already connected
	qsi_Result	=	cQSIcam.get_Connected(&isConnected);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_NUM("get_Connected returned QSI_OK: isConnected\t=", isConnected);
		if (isConnected == false)
		{
			// Connect to the selected camera and retrieve camera parameters
			CONSOLE_DEBUG("Trying to connect (calling put_Connected(true))");
			qsi_Result	=	cQSIcam.put_Connected(true);
			if (qsi_Result == QSI_OK)
			{
				CONSOLE_DEBUG("cQSIcam.put_Connected worked");
				isConnected	=	true;
			}
			else
			{
				cQSIcam.get_LastError(lastError);
				CONSOLE_DEBUG_W_STR("QSI Result (put_Connected)\t=",	lastError.c_str());
			}
		}
		else if ((qsi_Result == QSI_OK) && (isConnected == true))
		{
			CONSOLE_DEBUG("Already connected");
			cCommonProp.Connected	=	true;
		}
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Resul (get_Connected)\t=",	lastError.c_str());
	}
#ifdef _SIMULATE_CAMERA_
	cCommonProp.Connected	=	true;
	isConnected				=	true;
#endif
	return(isConnected);
}

//*****************************************************************************
//*	returns false if any errors occured
//*****************************************************************************
bool	CameraDriverQSI::ReadQSIcameraInfo(void)
{
unsigned int	qsi_Result;
bool			cameraInfoOK;
bool			canSetTemp;
bool			hasFilters;
bool			hasShutter;
bool			myBoolValue;
long			xsize;
long			ysize;
double			electronsPerADU;
double			fullWellCapacity;
double			exposureTime;
short			maxBin;
long			maxADU;
double			pixelSize;
std::string		serial("");
std::string		desc("");
std::string		text("");
std::string		info	=	"";
std::string		modelNumber("");
std::string		lastError("");


	CONSOLE_DEBUG(__FUNCTION__);
	cameraInfoOK			=	true;

	//*	set some defaults for testing
	strcpy(cDeviceManufacturer,	"QSI");
	cCameraProp.CameraXsize	=	4000;
	cCameraProp.CameraYsize	=	3000;
	cCameraProp.NumX		=	4000;
	cCameraProp.NumY		=	3000;

	cCameraProp.GainMin		=	0;
	cCameraProp.GainMax		=	1;		//*	qsi has high/low reversed, we will have to handle that

	AddReadoutModeToList(kImageType_RAW16);

	//--------------------------------------------------------------
	//*	Get Model Number
	qsi_Result	=	cQSIcam.get_ModelNumber(modelNumber);
	if (qsi_Result == QSI_OK)
	{
		strcpy(cCommonProp.Name,		modelNumber.c_str());
		CONSOLE_DEBUG_W_STR("QSI-modelNumber\t=",	cCommonProp.Name);
	}
	else if (qsi_Result == QSI_NOTCONNECTED)
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_ModelNumber)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	//*	Get Description
	qsi_Result	=	cQSIcam.get_Description(desc);
	if (qsi_Result == QSI_OK)
	{
		strcpy(cCommonProp.Description,		desc.c_str());
		CONSOLE_DEBUG_W_STR("QSI-Description\t\t\t=",	cCommonProp.Description);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	//*	Get the dimensions of the CCD
	qsi_Result	=	cQSIcam.get_CameraXSize(&xsize);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.CameraXsize	=	xsize;
		cCameraProp.NumX		=	xsize;
		CONSOLE_DEBUG_W_LONG("xsize\t=",	xsize);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_CameraXSize)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}
	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_CameraYSize(&ysize);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.CameraYsize	=	ysize;
		cCameraProp.NumY		=	ysize;
		CONSOLE_DEBUG_W_LONG("ysize\t=",	ysize);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_CameraYSize)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_ElectronsPerADU(&electronsPerADU);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.ElectronsPerADU	=				electronsPerADU;
		CONSOLE_DEBUG_W_DBL("electronsPerADU\t=",	electronsPerADU);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_ElectronsPerADU)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_FullWellCapacity(&fullWellCapacity);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.FullWellCapacity	=			fullWellCapacity;
		CONSOLE_DEBUG_W_DBL("fullWellCapacity\t=",	fullWellCapacity);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_FullWellCapacity)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_MinExposureTime(&exposureTime);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.ExposureMin_seconds	=			exposureTime;
		cCameraProp.ExposureMin_us		=			exposureTime * 1000000;
		CONSOLE_DEBUG_W_DBL("ExposureMin_seconds\t=",	exposureTime);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_MinExposureTime)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}
	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_MaxExposureTime(&exposureTime);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.ExposureMax_seconds	=			exposureTime;
		cCameraProp.ExposureMax_us		=			exposureTime * 1000000;
		CONSOLE_DEBUG_W_DBL("ExposureMin_seconds\t=",	exposureTime);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_MaxExposureTime)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_HasShutter(&hasShutter);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.HasShutter	=			hasShutter;
		CONSOLE_DEBUG_W_DBL("HasShutter\t=",	hasShutter);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_HasShutter)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

//	int get_HasFilterWheel(bool* pVal);
//	int get_HeatSinkTemperature(double* pVal);
//	int get_ImageArraySize(int& xSize, int& ySize, int& elementSize);
//	int get_ImageArray(unsigned short* pVal);
//	int get_ImageArray(double * pVal);
//	int get_ImageReady(bool* pVal);
//	int get_IsMainCamera(bool* pVal);
//	int put_IsMainCamera(bool newVal);
//	int get_IsPulseGuiding(bool* pVal);
//	int get_LastError(std::string& pVal);
//	int get_LastExposureDuration(double* pVal);
//	int get_LastExposureStartTime(std::string& pVal);

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_MaxADU(&maxADU);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.MaxADU	=			maxADU;
		CONSOLE_DEBUG_W_LONG("MaxADU\t=",	maxADU);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_MaxADU)\t\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_MaxBinX(&maxBin);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.MaxbinX	=				maxBin;
		CONSOLE_DEBUG_W_NUM("MaxbinX\t=",	maxBin);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_MaxBinX)\t\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}
	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_MaxBinY(&maxBin);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.MaxbinY	=				maxBin;
		CONSOLE_DEBUG_W_NUM("MaxbinY\t=",	maxBin);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_MaxBinY)\t\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_PixelSizeX(&pixelSize);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.PixelSizeX	=				pixelSize;
		CONSOLE_DEBUG_W_DBL("PixelSizeX\t=",	pixelSize);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (get_PixelSizeX)\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}

	//--------------------------------------------------------------
	qsi_Result	=	cQSIcam.get_PixelSizeY(&pixelSize);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.PixelSizeY	=				pixelSize;
		CONSOLE_DEBUG_W_DBL("PixelSizeY\t=",	pixelSize);
	}
	else
	{
		cQSIcam.get_LastError(lastError);
		CONSOLE_DEBUG_W_STR("QSI Result (PixelSizeY)\t\t=",	lastError.c_str());
		cameraInfoOK	=	false;
	}


	// Enable the beeper
	qsi_Result	=	cQSIcam.put_SoundEnabled(true);
	// Enable the indicator LED
	qsi_Result	=	cQSIcam.put_LEDEnabled(true);
	// Set the fan mode
	qsi_Result	=	cQSIcam.put_FanMode(QSICamera::fanQuiet);
	// Query the current flush mode setting
	qsi_Result	=	cQSIcam.put_PreExposureFlush(QSICamera::FlushNormal);

	//-----------------------------------------------------------
	// Query if the camera can control the CCD temp
	qsi_Result	=	cQSIcam.get_CanSetCCDTemperature(&canSetTemp);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_NUM("canSetTemp\t=",	canSetTemp);
		cCameraProp.Cansetccdtemperature	=	canSetTemp;
		if (canSetTemp)
		{
			cIsCoolerCam	=	true;

			// Set the CCD temp setpoint to 10.0C
			qsi_Result	=	cQSIcam.put_SetCCDTemperature(10.0);
			// Enable the cooler
			qsi_Result	=	cQSIcam.put_CoolerOn(true);
		}
	}

	//-----------------------------------------------------------
	// Query if the camera can get cooler power level
	qsi_Result	=	cQSIcam.get_CanGetCoolerPower(&myBoolValue);
	if (qsi_Result == QSI_OK)
	{
		cCameraProp.CanGetCoolerPower	=	myBoolValue;
	}

	if (modelNumber.substr(0,1) == "6")
	{
		qsi_Result	=	cQSIcam.put_ReadoutSpeed(QSICamera::FastReadout);
	}

	// Does the camera have a filer wheel?
	qsi_Result	=	cQSIcam.get_HasFilterWheel(&hasFilters);
	if (qsi_Result == QSI_OK)
	{
		CONSOLE_DEBUG_W_BOOL("hasFilters\t=",	hasFilters);

		if ( hasFilters)
		{
			// Set the filter wheel to position 1 (0 based position)
			qsi_Result	=	cQSIcam.put_Position(0);
		}
	}

	if (modelNumber.substr(0,3) == "520" || modelNumber.substr(0,3) == "540")
	{
		qsi_Result	=	cQSIcam.put_CameraGain(QSICamera::CameraGainHigh);
		qsi_Result	=	cQSIcam.put_PreExposureFlush(QSICamera::FlushNormal);
	}
	//
	//////////////////////////////////////////////////////////////
	// Set image size
	//
	qsi_Result	=	cQSIcam.put_BinX(1);
	qsi_Result	=	cQSIcam.put_BinY(1);
	// Set the exposure to a full frame
	qsi_Result	=	cQSIcam.put_StartX(0);
	qsi_Result	=	cQSIcam.put_StartY(0);
	qsi_Result	=	cQSIcam.put_NumX(xsize);
	qsi_Result	=	cQSIcam.put_NumY(ysize);

	//*	now lets check the cooler and get info on it
	if (cCameraProp.CanGetCoolerPower)
	{
	double				coolerPowerLevel;
		qsi_Result	=	cQSIcam.get_CoolerPower(&coolerPowerLevel);
		if (qsi_Result == QSI_OK)
		{
			cCameraProp.CoolerPower	=	coolerPowerLevel;
		}
	}
	if (cCameraProp.Cansetccdtemperature)
	{
	double		cameraTemp_DegC;

		//	Returns the current CCD temperature in degrees Celsius in parameter 1.
		//	Only valid if CanSetCCDTemperature is true.
		qsi_Result	=	cQSIcam.get_CCDTemperature(&cameraTemp_DegC);
		if (qsi_Result == QSI_OK)
		{
			cCameraProp.CCDtemperature		=	cameraTemp_DegC;
		}
	}

	return(cameraInfoOK);
}

//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				cameraTemp_DegC;
unsigned int		qsi_Result;
std::string			lastError("");

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		cCameraProp.CCDtemperature	=	2.23;
		alpacaErrCode				=	kASCOM_Err_Success;
	}
	else
#endif
	{
		if (cCameraProp.Cansetccdtemperature)
		{
			//	Returns the current CCD temperature in degrees Celsius in parameter 1.
			//	Only valid if CanSetCCDTemperature is true.
			qsi_Result	=	cQSIcam.get_CCDTemperature(&cameraTemp_DegC);
			if (qsi_Result == QSI_OK)
			{
				cCameraProp.CCDtemperature		=	cameraTemp_DegC;
				CONSOLE_DEBUG_W_DBL("CCDtemperature\t=",	cameraTemp_DegC);
			}
			else if (qsi_Result == QSI_NOTCONNECTED)
			{
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				strcpy(cLastCameraErrMsg, "QSI Result: not connected");
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				cQSIcam.get_LastError(lastError);
				strcpy(cLastCameraErrMsg, "QSI Result:");
				strcat(cLastCameraErrMsg, lastError.c_str());
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotSupported;
			strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Write_BinX(const int newBinXvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		cCameraProp.BinX	=	newBinXvalue;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
#endif
	{
		alpacaErrCode	=	kASCOM_Err_Success;

	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Write_BinY(const int newBinYvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		cCameraProp.BinY	=	newBinYvalue;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
#endif
	{
		alpacaErrCode	=	kASCOM_Err_Success;

	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;

	CONSOLE_DEBUG_W_NUM(__FUNCTION__, newGainValue);
#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		cCameraProp.Gain	=	newGainValue;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
#endif
	{
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
unsigned int			qsi_Result;
std::string				lastError("");
QSICamera::CameraGain	qsiGainValue;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		*cameraGainValue	=	cCameraProp.Gain;
		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
#endif
	{
		qsi_Result	=	cQSIcam.get_CameraGain(&qsiGainValue);
		if (qsi_Result == QSI_OK)
		{
			*cameraGainValue	=	(int)qsiGainValue;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else if (qsi_Result == QSI_NOTCONNECTED)
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
		else
		{
			cQSIcam.get_LastError(lastError);
			CONSOLE_DEBUG_W_STR("QSI Result\t=",	lastError.c_str());
			alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverQSI::Start_CameraExposure(int32_t exposureMicrosecs)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
double				durationSeconds;
unsigned int		qsi_Result;
std::string			lastError("");

	CONSOLE_DEBUG(__FUNCTION__);
	cCameraProp.ImageReady		=	false;

	durationSeconds	=	(exposureMicrosecs * 1.0) / 1000000.0;

	CONSOLE_DEBUG_W_NUM("exposureMicrosecs\t=",	exposureMicrosecs);
	CONSOLE_DEBUG_W_DBL("durationSeconds\t=",	durationSeconds);

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		CONSOLE_DEBUG("Simulating camera");
		cInternalCameraState	=	kCameraState_TakingPicture;
		SetLastExposureInfo();
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
#endif
	{
		//-----------------------------------------------------------------
		//Parameters
		//double Duration - Duration of exposure in seconds
		//bool Light - true for light frame, false for dark frame (ignored if no shutter)
		qsi_Result			=	cQSIcam.StartExposure(durationSeconds, true);
		if (qsi_Result == QSI_OK)
		{
			alpacaErrCode			=	kASCOM_Err_Success;
			cInternalCameraState	=	kCameraState_TakingPicture;
			SetLastExposureInfo();
		}
		else
		{
			cQSIcam.get_LastError(lastError);

			strcpy(cLastCameraErrMsg,	"QSI Err:");
			strcat(cLastCameraErrMsg,	lastError.c_str());
			CONSOLE_DEBUG(cLastCameraErrMsg);
			switch(qsi_Result)
			{
				case QSI_NOTCONNECTED:	alpacaErrCode	=	kASCOM_Err_NotConnected;		break;
				default:				alpacaErrCode	=	kASCOM_Err_InvalidOperation;	break;
			}
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverQSI::Abort_Exposure(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
std::string			lastError("");

	qsi_Result			=	cQSIcam.AbortExposure();
	if (qsi_Result == QSI_OK)
	{
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		cQSIcam.get_LastError(lastError);

		strcpy(cLastCameraErrMsg,	"QSI Err:");
		strcat(cLastCameraErrMsg,	lastError.c_str());
		CONSOLE_DEBUG(cLastCameraErrMsg);
		switch(qsi_Result)
		{
			case QSI_NOTCONNECTED:	alpacaErrCode	=	kASCOM_Err_NotConnected;		break;
			default:				alpacaErrCode	=	kASCOM_Err_InvalidOperation;	break;
		}
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverQSI::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	this should be over ridden
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}



//*****************************************************************************
bool	CameraDriverQSI::GetImage_ROI_info(void)
{
	cROIinfo.currentROIimageType	=	kImageType_RAW16;
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;
	return(true);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
std::string			lastError("");

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_BOOL(	"cCameraProp.Cansetccdtemperature\t=",	cCameraProp.Cansetccdtemperature);
	CONSOLE_DEBUG_W_BOOL(	"cCameraProp.CanGetCoolerPower  \t=",	cCameraProp.CanGetCoolerPower);

	if (cCameraProp.Cansetccdtemperature)
	{
		// Enable the cooler
		qsi_Result	=	cQSIcam.put_CoolerOn(true);
		if (qsi_Result == QSI_OK)
		{
			CONSOLE_DEBUG("cQSIcam.put_CoolerOn(true); return OK");
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			cQSIcam.get_LastError(lastError);
			strcpy(cLastCameraErrMsg, lastError.c_str());
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		strcpy(cLastCameraErrMsg, "Camera does not support cooling");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
std::string			lastError("");

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_BOOL(	"cCameraProp.Cansetccdtemperature\t=",	cCameraProp.Cansetccdtemperature);
	CONSOLE_DEBUG_W_BOOL(	"cCameraProp.CanGetCoolerPower  \t=",	cCameraProp.CanGetCoolerPower);

	if (cCameraProp.Cansetccdtemperature)
	{
		// disable the cooler
		qsi_Result	=	cQSIcam.put_CoolerOn(false);
		if (qsi_Result == QSI_OK)
		{
			CONSOLE_DEBUG("cQSIcam.put_CoolerOn(false); return OK");
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			cQSIcam.get_LastError(lastError);
			strcpy(cLastCameraErrMsg, lastError.c_str());
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		strcpy(cLastCameraErrMsg, "Camera does not support cooling");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
std::string			lastError("");
double				coolerPowerLevel;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCameraProp.CanGetCoolerPower)
	{
	#ifdef _SIMULATE_CAMERA_
		if (gSimulateCameraImage)
		{
			cCameraProp.CoolerPower	=	52.34;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
	#endif
		{
			qsi_Result	=	cQSIcam.get_CoolerPower(&coolerPowerLevel);
			if (qsi_Result == QSI_OK)
			{
				cCameraProp.CoolerPower	=	coolerPowerLevel;
				alpacaErrCode			=	kASCOM_Err_Success;
				CONSOLE_DEBUG_W_DBL("cCameraProp.CoolerPower\t=", cCameraProp.CoolerPower);
			}
			else
			{
				cQSIcam.get_LastError(lastError);
				strcpy(cLastCameraErrMsg, lastError.c_str());
				alpacaErrCode	=	kASCOM_Err_NotConnected;
				CONSOLE_DEBUG_W_STR("cQSIcam.get_CoolerPower returned", cLastCameraErrMsg);
			}
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "AlpacaPi: Not implemented-");
		strcat(cLastCameraErrMsg, __FILE__);
		strcat(cLastCameraErrMsg, ":");
		strcat(cLastCameraErrMsg, __FUNCTION__);
		return(alpacaErrCode);
	}
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::Read_CoolerState(bool *coolerOnOff)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
std::string			lastError("");
bool				myCoolerState;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		*coolerOnOff	=	true;
	}
	else
#endif
	{
		qsi_Result	=	cQSIcam.get_CoolerOn(&myCoolerState);
		if (qsi_Result == QSI_OK)
		{
			*coolerOnOff	=	myCoolerState;
			alpacaErrCode	=	kASCOM_Err_Success;
			CONSOLE_DEBUG_W_BOOL("coolerOnOff\t=", coolerOnOff);
		}
		else
		{
			cQSIcam.get_LastError(lastError);
			strcpy(cLastCameraErrMsg, lastError.c_str());
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			CONSOLE_DEBUG_W_STR("cQSIcam.get_CoolerPower returned", cLastCameraErrMsg);
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverQSI::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	myExposureStatus;
unsigned int			qsi_Result;
QSICamera::CameraState	qsiCameraState;
std::string				lastError("");
bool					imageReady;

//	CONSOLE_DEBUG(__FUNCTION__);

	myExposureStatus	=	kExposure_Failed;
#ifdef _SIMULATE_CAMERA_
	//--------------------------------------------
	//*	simulate image
	if (gSimulateCameraImage)
	{
	struct timeval			currentTIme;
	time_t					deltaTime_secs;
		//--------------------------------------------
		//*	simulate image
		switch(cInternalCameraState)
		{
			case kCameraState_TakingPicture:
				myExposureStatus		=	kExposure_Working;
				gettimeofday(&currentTIme, NULL);	//*	get the current time
				deltaTime_secs	=	currentTIme.tv_sec - cCameraProp.Lastexposure_StartTime.tv_sec;

				CONSOLE_DEBUG_W_LONG("deltaTime_secs\t=",			deltaTime_secs);
				if (deltaTime_secs > 2)
				{
					CONSOLE_DEBUG("Not kCameraState_TakingPicture -->> kCameraState_Idle");
					myExposureStatus		=	kExposure_Success;
				}
				break;

			default:
		//		myExposureStatus		=	kExposure_Idle;
				myExposureStatus		=	kExposure_Success;
				break;

		}
		CONSOLE_DEBUG_W_NUM("myExposureStatus\t=",			myExposureStatus);

	return(myExposureStatus);
	}
#endif

	qsi_Result	=	cQSIcam.get_CameraState(&qsiCameraState);
	if (qsi_Result == QSI_OK)
	{
		switch(qsiCameraState)
		{
			case QSICamera::CameraIdle:		//At idle state, available to start exposure
				myExposureStatus	=	kExposure_Idle;
				imageReady			=	false;
				qsi_Result			=	cQSIcam.get_ImageReady(&imageReady);
				if (imageReady)
				{
					myExposureStatus	=	kExposure_Success;
				}
				break;

			case QSICamera::CameraWaiting:	//Exposure started but waiting (for shutter, trigger, filter wheel, etc.)
			case QSICamera::CameraExposing:	//Exposure currently in progress
			case QSICamera::CameraReading:	//CCD array is being read out (digitized)
			case QSICamera::CameraDownload:	//Downloading data to PC
				myExposureStatus	=	kExposure_Working;
				break;

			case QSICamera::CameraError:	//Camera error condition serious enough to prevent further operations (link fail, etc.).
				myExposureStatus	=	kExposure_Failed;
				break;
		}
	}
	else
	{
		myExposureStatus	=	kExposure_Failed;

		cQSIcam.get_LastError(lastError);

		strcpy(cLastCameraErrMsg,	"QSI Err:");
		strcat(cLastCameraErrMsg,	lastError.c_str());
//		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
#ifdef _SIMULATE_CAMERA_
	//--------------------------------------------
	//*	simulate image
	if (gSimulateCameraImage)
	{
		myExposureStatus	=	kExposure_Success;
	}
#endif

	return(myExposureStatus);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverQSI::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("newImageType\t=",			newImageType);

	if (1)
	{
		cROIinfo.currentROIwidth	=	cCameraProp.CameraXsize;
		cROIinfo.currentROIheight	=	cCameraProp.CameraYsize;
		cROIinfo.currentROIbin		=   1;

		switch(newImageType)
		{
			case kImageType_RAW8:
			case kImageType_RAW16:
				cROIinfo.currentROIimageType	=	kImageType_RAW8;
//+				cROIinfo.currentROIimageType	=	kImageType_RAW16;
				break;


			case kImageType_RGB24:
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
TYPE_ASCOM_STATUS	CameraDriverQSI::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
unsigned int		qsi_Result;
int					xSize;
int					ySize;
int					elementSize;
long				imgBufferSize;
std::string			lastError("");

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _SIMULATE_CAMERA_
	if (gSimulateCameraImage)
	{
		AllcateImageBuffer(-1);		//*	let it figure out how much
		if (cCameraDataBuffer != NULL)
		{
			//--------------------------------------------
			CreateFakeImageData(cCameraDataBuffer, cCameraProp.CameraXsize, cCameraProp.CameraYsize, 2);
			cCameraProp.ImageReady	=	true;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
	}
	else
#endif
	{
		//*	get the image size
		qsi_Result	=	cQSIcam.get_ImageArraySize(xSize, ySize, elementSize);
		if (qsi_Result == QSI_OK)
		{
			//*	make sure the buffer is allocated
			imgBufferSize	=	xSize * ySize * elementSize;
			AllcateImageBuffer(imgBufferSize);
			if (cCameraDataBuffer != NULL)
			{
				//*	get the data
				qsi_Result	=	cQSIcam.get_ImageArray((unsigned short *)cCameraDataBuffer);
				if (qsi_Result == QSI_OK)
				{
					alpacaErrCode		=	kASCOM_Err_Success;
				}
				else
				{
					alpacaErrCode			=	kASCOM_Err_InvalidOperation;
					cQSIcam.get_LastError(lastError);

					strcpy(cLastCameraErrMsg,	"QSI Err:");
					strcat(cLastCameraErrMsg,	lastError.c_str());
				}
			}
		}
	}
	return(alpacaErrCode);
}


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_QSI_)
