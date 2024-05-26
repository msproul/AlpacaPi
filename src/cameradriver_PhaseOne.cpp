//**************************************************************************
//*	Name:			cameradriver_PhaseOne.cpp
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	https://developer.phaseone.com/sdk/index.html
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep 13,	2022	<MLS> Created cameradriver_PhaseOne.cpp
//*	Sep 20,	2022	<MLS> SDK docs are at https://developer.phaseone.com/sdk/
//*	Sep 20,	2022	<MLS> SDK @ https://developer.phaseone.com/sdk/docs/appendices/download.html
//*	Sep 20,	2022	<MLS> SDK Version  : SDK V2.0.31
//*	Sep 20,	2022	<MLS> SDK BuildDate: Feb 11 2022 12:13:12
//*	Sep 20,	2022	<MLS> SDK Compiler : GNU C/C++ 7.5.0
//*	Jul 14,	2023	<MLS> Spoke to Paul Mecca, back working on driver
//*	Jul 15,	2023	<MLS> Figured out that the camera talks on port 7381
//*	Jul 15,	2023	<MLS> Find camera using >>nmap -p7381 -T4 192.168.1.1/24
//*	Jul 16,	2023	<MLS> Added OutputHTML_Part2() to PhaseOne camera driver
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_PHASEONE_)


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"eventlogging.h"
#include	"cameradriver.h"
#include	"cameradriver_PhaseOne.h"
#include	"linuxerrors.h"

#include	<P1Camera.hpp>	// CameraSDK
//#include	<P1Image.hpp>	// ImageSDK

//**************************************************************************************
enum
{
	kP1_PropertyID_Manufacturer					=	1,
	kP1_PropertyID_Model						=	2,
	kP1_PropertyID_Serial						=	3,
	kP1_PropertyID_Firmware						=	4,
	kP1_PropertyID_ModelId						=	10,
	kP1_PropertyID_SystemSerialNumber			=	12,
	kP1_PropertyID_UnitNumber					=	13,
	kP1_PropertyID_UnitName						=	14,
	kP1_PropertyID_Profile						=	18,
	kP1_PropertyID_SensorArea					=	24,
	kP1_PropertyID_SensorActiveRect				=	25,
	kP1_PropertyID_FileFormat					=	102,
	kP1_PropertyID_Compression					=	103,
	kP1_PropertyID_ImageOrientation				=	104,
	kP1_PropertyID_ISO							=	120,
	kP1_PropertyID_WhiteBalance					=	122,
	kP1_PropertyID_BlackRef						=	128,
	kP1_PropertyID_Antiflickering				=	141,
	kP1_PropertyID_CaptureCount					=	200,
	kP1_PropertyID_BackBatteryCapacity			=	201,
	kP1_PropertyID_BackBatteryState				=	204,
	kP1_PropertyID_CameraBatteryCapacity		=	205,
	kP1_PropertyID_CameraBatteryState			=	206,
	kP1_PropertyID_SystemBatteryCapacity		=	207,
	kP1_PropertyID_SystemBatteryState			=	208,
	kP1_PropertyID_StorageMode					=	230,
	kP1_PropertyID_ServiceCommand				=	500,
	kP1_PropertyID_ExposureProgram				=	1000,
	kP1_PropertyID_ShutterSpeed					=	1002,
	kP1_PropertyID_Aperture						=	1003,
	kP1_PropertyID_ExposureComp					=	1004,
	kP1_PropertyID_MeteringMode					=	1006,
	kP1_PropertyID_ShutterMode					=	1008,
	kP1_PropertyID_Drive						=	1009,
	kP1_PropertyID_AFmode						=	1010,
	kP1_PropertyID_AFdrive						=	1011,
	kP1_PropertyID_AutoExposureOutOfRange		=	1013,
	kP1_PropertyID_MirrorUp						=	1020,
	kP1_PropertyID_ExpError						=	1021,
	kP1_PropertyID_FocusAdjustDiscrete			=	1022,
	kP1_PropertyID_FocusAdjustContinuous		=	1023,
	kP1_PropertyID_LeafShutterCount				=	1025,
	kP1_PropertyID_ApertureCount				=	1026,
	kP1_PropertyID_ShutterModeActual			=	1028,
	kP1_PropertyID_RemoteFocus					=	1029,
	kP1_PropertyID_FocusMarker					=	1030,
	kP1_PropertyID_ISOmin						=	1051,
	kP1_PropertyID_ISOmax						=	1052,
	kP1_PropertyID_ShutterSpeedMax				=	1053,
	kP1_PropertyID_ShutterSpeedMin				=	1054,
	kP1_PropertyID_ApertureMin					=	1055,
	kP1_PropertyID_ApertureMax					=	1056,
	kP1_PropertyID_ISOauto						=	1057,
	kP1_PropertyID_ShutterSpeedAuto				=	1058,
	kP1_PropertyID_ApertureAuto					=	1059,
//?	kP1_PropertyID_Model						=	1151,
//?	kP1_PropertyID_Firmware						=	1152,
//?	kP1_PropertyID_Serial						=	1161,
	kP1_PropertyID_UnsavedImages				=	1200,
	kP1_PropertyID_ReadyForCapture				=	1201,
	kP1_PropertyID_LiveViewActive				=	1205,
	kP1_PropertyID_Message						=	1206,
	kP1_PropertyID_Acknowledge					=	1207,
	kP1_PropertyID_HDMIliveView					=	1220,
	kP1_PropertyID_HDMIoverlay					=	1221,
	kP1_PropertyID_LiveViewAutoGain				=	1230,
	kP1_PropertyID_Preview						=	1241,
	kP1_PropertyID_XQD_CFXAvailability			=	1243,
	kP1_PropertyID_OverlayLayout				=	1244,
	kP1_PropertyID_Transparency					=	1245,
	kP1_PropertyID_PreviewTime					=	1246,
	kP1_PropertyID_PreviewOrientation			=	1247,
	kP1_PropertyID_HDMILiveViewZoom				=	1248,
	kP1_PropertyID_StaticSetup					=	1251,
	kP1_PropertyID_IPaddress					=	1252,
	kP1_PropertyID_Netmask						=	1253,
	kP1_PropertyID_Gateway						=	1254,
	kP1_PropertyID_DHCPaddress					=	1256,
	kP1_PropertyID_Status_Action				=	1257,
	kP1_PropertyID_DHCPmode						=	1258,
	kP1_PropertyID_BonjourIsRunning				=	1260,
	kP1_PropertyID_BonjourEnable				=	1261,
	kP1_PropertyID_BonjourEnabledAtPowerOn		=	1262,
	kP1_PropertyID_HostName						=	1263,
	kP1_PropertyID_CustomWB						=	1290,
	kP1_PropertyID_Action_Restore				=	1291,
	kP1_PropertyID_NetworkSetup					=	1293,
	kP1_PropertyID_HostStorage					=	1302,
	kP1_PropertyID_XQD_CFX						=	1303,
	kP1_PropertyID_SD							=	1304,
	kP1_PropertyID_SDstorage					=	1305,
	kP1_PropertyID_ZoomPanX						=	1310,
	kP1_PropertyID_ZoomPanY						=	1311,
	kP1_PropertyID_FocusPeaking					=	1312,
	kP1_PropertyID_FocusPeakingThreshold		=	1313,
	kP1_PropertyID_DiagnosticsToStorageConfig	=	1401,
	kP1_PropertyID_CurrentTemperature			=	1420,
	kP1_PropertyID_UnderTemperature				=	1421,
	kP1_PropertyID_OverTemperature				=	1422,
	kP1_PropertyID_WarningTemperature			=	1423,
	kP1_PropertyID_CriticalTemperature			=	1424,
	kP1_PropertyID_ThrottleTemperatureT1		=	1425,
	kP1_PropertyID_ThrottleTemperatureT2		=	1426,
	kP1_PropertyID_XQD_CFXremainingFreeSpace	=	1460,
	kP1_PropertyID_XQD_CFXsize					=	1461,
	kP1_PropertyID_SDcardAvailability			=	1462,
	kP1_PropertyID_SDcardRemainingFreeSpace		=	1463,
	kP1_PropertyID_SDcardSize					=	1464,
	kP1_PropertyID_PowerShareWithCamera			=	1470,
	kP1_PropertyID_PrimaryPowerSource			=	1471

};

///src/cameradriver_PhaseOne.cpp         : 634 [AlpacaConnect       ] AlpacaConnect
//:3219 [DumpCommonProperties] *************************************************************
//:3220 [DumpCommonProperties] ******************** Alpaca device properties ***************
//:3223 [DumpCommonProperties] ************* Called from: CameraDriverPhaseOne *************
//:3224 [DumpCommonProperties] *************************************************************
//:3227 [DumpCommonProperties] cDeviceType                    	= 0
//:3228 [DumpCommonProperties] cAlpacaDeviceNum                 = 0
//:3229 [DumpCommonProperties] cCommonProp.Connected          	= TRUE
//:3230 [DumpCommonProperties] cCommonProp.Description        	= PhaseOne--
//:3231 [DumpCommonProperties] cCommonProp.DriverInfo         	=
//:3232 [DumpCommonProperties] cCommonProp.DriverVersion      	= V0.5.1-beta Build 154
//:3233 [DumpCommonProperties] cCommonProp.InterfaceVersion   	= 3
//:3234 [DumpCommonProperties] cCommonProp.Name               	= IQ4 150MP
//:8131 [DumpCameraProperties] ------------------------------------
//:8132 [DumpCameraProperties] cCameraProp.BayerOffsetX       	= 0
//:8133 [DumpCameraProperties] cCameraProp.BayerOffsetY       	= 0
//:8134 [DumpCameraProperties] cCameraProp.BinX               	= 1
//:8135 [DumpCameraProperties] cCameraProp.BinY               	= 1
//:8136 [DumpCameraProperties] cCameraProp.CameraXsize        	= 14308
//:8137 [DumpCameraProperties] cCameraProp.CameraYsize        	= 10760
//:8138 [DumpCameraProperties] cCameraProp.CanAbortExposure   	= FALSE
//:8139 [DumpCameraProperties] cCameraProp.CanAsymmetricBin   	= FALSE
//:8140 [DumpCameraProperties] cCameraProp.CanFastReadout     	= FALSE
//:8141 [DumpCameraProperties] cCameraProp.CanGetCoolerPower  	= FALSE
//:8142 [DumpCameraProperties] cCameraProp.CanPulseGuide      	= FALSE
//:8143 [DumpCameraProperties] cCameraProp.CanSetCCDtemperature	= FALSE
//:8144 [DumpCameraProperties] cCameraProp.CanStopExposure    	= FALSE
//:8145 [DumpCameraProperties] cCameraProp.CCDtemperature     	= 0.000000000000000
//:8146 [DumpCameraProperties] cCameraProp.CoolerOn           	= FALSE
//:8147 [DumpCameraProperties] cCameraProp.CoolerPower        	= 0.000000000000000
//:8148 [DumpCameraProperties] cCameraProp.ElectronsPerADU    	= 65000.000000000000000
//:8149 [DumpCameraProperties] cCameraProp.ExposureMax_seconds	= 3600.000000000000000
//:8150 [DumpCameraProperties] cCameraProp.ExposureMin_seconds	= 0.000250000011874
//:8151 [DumpCameraProperties] cCameraProp.ExposureResolution 	= 1.000000000000000
//:8152 [DumpCameraProperties] cCameraProp.FastReadout        	= FALSE
//:8153 [DumpCameraProperties] cCameraProp.FullWellCapacity   	= 0.000000000000000
//:8154 [DumpCameraProperties] cCameraProp.Gain               	= 0
//:8155 [DumpCameraProperties] cCameraProp.GainMax            	= 10
//:8156 [DumpCameraProperties] cCameraProp.GainMin            	= 0
//:8157 [DumpCameraProperties] cCameraProp.HasShutter         	= FALSE
//:8158 [DumpCameraProperties] cCameraProp.HeatSinkTemperature	= 0.000000000000000
//:8159 [DumpCameraProperties] cCameraProp.ImageReady         	= FALSE
//:8160 [DumpCameraProperties] cCameraProp.IsPulseGuiding     	= FALSE
//:8165 [DumpCameraProperties] cCameraProp.MaxADU             	= 0
//:8166 [DumpCameraProperties] cCameraProp.MaxbinX            	= 1
//:8167 [DumpCameraProperties] cCameraProp.MaxbinY            	= 1
//:8168 [DumpCameraProperties] cCameraProp.NumX               	= 14308
//:8169 [DumpCameraProperties] cCameraProp.NumY               	= 10760
//:8170 [DumpCameraProperties] cCameraProp.Offset             	= 0
//:8171 [DumpCameraProperties] cCameraProp.OffsetMax          	= 0
//:8172 [DumpCameraProperties] cCameraProp.OffsetMin          	= 0
//:8173 [DumpCameraProperties] cCameraProp.PercentCompleted   	= -99
//:8174 [DumpCameraProperties] cCameraProp.PixelSizeX         	= 2.500000000000000
//:8175 [DumpCameraProperties] cCameraProp.PixelSizeY         	= 2.500000000000000
//:8176 [DumpCameraProperties] cCameraProp.ReadOutMode        	= 0
//:8177 [DumpCameraProperties] cCameraProp.SensorName         	= Fake
//:8178 [DumpCameraProperties] cCameraProp.SensorType         	= 0
//:8179 [DumpCameraProperties] cCameraProp.StartX             	= 0
//:8180 [DumpCameraProperties] cCameraProp.StartY             	= 0
//:8181 [DumpCameraProperties] ---------------Non alpaca stuff
//:8182 [DumpCameraProperties] cCameraProp.FlipMode           	= 0
//:8183 [DumpCameraProperties] cIsCoolerCam                   	= FALSE
//:8184 [DumpCameraProperties] cTempReadSupported             	= TRUE



//**************************************************************************************
int	CreateCameraObjects_PhaseOne(void)
{
int								returnCode;		//*	0 means OK
P1::CameraSdk::Camera			p1Camera;
P1::CameraSdk::Version			p1SDKversion;
char							p1SDK_VersionStr[64];
char							p1SDK_Compiler[64];
char							p1SDK_BuildDate[64];

	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=	0;

	//----------------------------------------------
	p1SDKversion	=	P1::CameraSdk::GetSdkVersion();

	sprintf(p1SDK_VersionStr, "SDK V%d.%d.%d",	p1SDKversion.major,
												p1SDKversion.minor,
												p1SDKversion.build);
	AddLibraryVersion("camera", "PhaseOne", p1SDK_VersionStr);

	strcpy(p1SDK_BuildDate,	P1::CameraSdk::GetSdkBuildDate());
	strcpy(p1SDK_Compiler,	P1::CameraSdk::GetSdkBuildCompiler());
	CONSOLE_DEBUG_W_STR("SDK Version  :\t",	p1SDK_VersionStr);
	CONSOLE_DEBUG_W_STR("SDK BuildDate:\t",	p1SDK_BuildDate);
	CONSOLE_DEBUG_W_STR("SDK Compiler :\t",	p1SDK_Compiler);

	// All calls to the CameraSDK and ImageSDK must be wrapped in a try-catch,
	// because exceptions are used to report errors, etc.
//	try
//	{
//		p1Camera	=	P1::CameraSdk::Camera::OpenUsbCamera();
		//*	Mac Address = 60:D7:E3:72:06:8E
//		p1Camera	=	P1::CameraSdk::Camera::OpenIpCamera("192.168.1.7", "", 2);
//
//		// prepare to set cameras "exposure program" to manual
//		// this will allow us to set ISO, aperture and shutter time manually.
//		exposureProgram.mType	=	P1::CameraSdk::kPropertyTypeEnum;
//		exposureProgram.mInt	=	0;
//		exposureProgramId		=	1000;

//		// Only change exposure program - if not already 0
//		if (p1Camera.GetProperty(1000).mInt != 0)
//		{
//			p1Camera.SetProperty(exposureProgramId, exposureProgram);
//		}
//
//		// Setup ISO, Aperture and Shutter-time
//		P1::CameraSdk::PropertyValue isoValue;
//		isoValue.mType		=	P1::CameraSdk::kPropertyTypeFloat64;
//		isoValue.mDouble	=	200;
//
//		P1::CameraSdk::PropertyValue apertureValue;
//		apertureValue.mType		=	P1::CameraSdk::kPropertyTypeFloat64;
//		apertureValue.mDouble	=	5.0;
//
//		P1::CameraSdk::PropertyValue shutterTimeValue;
//		shutterTimeValue.mType		=	P1::CameraSdk::kPropertyTypeFloat64;
//		shutterTimeValue.mDouble	=	0.125;
//
//		// PropertyIDs:
//		uint32_t isoId			=	120;
//		uint32_t apertureId		=	1003;
//		uint32_t shutterTimeId	=	1002;
//
//		camera.SetProperty(isoId, isoValue);
//		camera.SetProperty(apertureId, apertureValue);
//		camera.SetProperty(shutterTimeId, shutterTimeValue);
//
//		// Set-up so images will be sent to this computer and trig the capture
//		std::cout << "Start capture" << std::endl;
//
//		camera.EnableImageReceiving(true);
//		// Necessary to sleep, otherwise TriggerCapture will start before EnableImageReceiving is done
//		std::this_thread::sleep_for(std::chrono::milliseconds(200));
//		camera.TriggerCapture();
//
//		// Wait for image
//		P1::CameraSdk::IIQImageFile imageFile	=	camera.WaitForImage(10000);
//		std::cout << "Image received" << std::endl;
//
//		//------
//		// We are done with the CameraSDK, and now use our ImageSDK for the rest
//		//------
//		//
//		// Parse IIQ file
//		P1::ImageSdk::RawImage image(imageFile.data, imageFile.size);
//
//		// Decode image data with default values
//		P1::ImageSdk::DecodeConfig decodeConfig	=	P1::ImageSdk::DecodeConfig::Defaults;
//		P1::ImageSdk::SensorBayerOutput decodedImage	=	image.Decode(decodeConfig);
//		std::cout << "Decoded image size: " << decodedImage.ByteSize()
//			<< " Height=" << decodedImage.FullHeight()
//			<< " Width=" << decodedImage.FullWidth() << std::endl;
//
//		// Write decoded bitmap to file
//		std::fstream rawFile("bitmapRAW.bin", std::ios::binary | std::ios::trunc |
//			std::ios::out);
//		rawFile.write((char*)decodedImage.Data().get(), decodedImage.ByteSize());
//		rawFile.close();
//
//		// Convert image data to RGB
//		P1::ImageSdk::ConvertConfig config;
//		config.SetOutputWidth(1024);
//		P1::ImageSdk::BitmapImage bitmap	=	config.ApplyTo(image);
//
//		std::cout << "Output bitmap.bin " << " Height=" << bitmap.Width() << " Width= "
//			<< bitmap.Height() << std::endl;
//
//		// Write RGB bitmap to file
//		std::fstream rgbFile("bitmapRGB.bin", std::ios::binary | std::ios::trunc |
//			std::ios::out);
//		rgbFile.write((char*)bitmap.Data().get(), bitmap.ByteSize());
//		rgbFile.close();
//
//		std::cout << "Program successfully completed" << std::endl;
//		return 0;
//	}
//	catch (P1::ImageSdk::SdkException exception)
//	{
//		// Exception from ImageSDK
//		std::cout << "ImageSDK Exception: " << exception.what() << " Code:" << exception.mCode <<
//			std::endl;
//		return -1;
//	}
//	catch (P1::CameraSdk::SdkException exception)
//	{
//		CONSOLE_DEBUG("P1::CameraSdk::SdkException exception");
//		// Exception from CameraSDK
//		std::cout << "CameraSDK Exception: " << exception.what() << " Code:"
//			<< exception.mErrorCode <<
//			std::endl;
//		CONSOLE_DEBUG_W_STR("exception.what()    :", exception.what());
//		CONSOLE_DEBUG_W_NUM("exception.mErrorCode:", exception.mErrorCode);
//		returnCode	=	-1;
//	}
//	catch (...)
//	{
//		CONSOLE_DEBUG("... exception");
//		// Any other exception - just in case
//		CONSOLE_DEBUG("Argh - we got an exception");
//		returnCode	=	-1;
//	}

	if (returnCode == 0)
	{
		CONSOLE_DEBUG("Creating PhaseOne camera");
		//*	for debugging without a camera
//		CONSOLE_DEBUG_W_HEX("cP1Camera\t=", (void *)&p1Camera);
//		new CameraDriverPhaseOne(0, &p1Camera);
		new CameraDriverPhaseOne("192.168.1.13", 0);

	}
	else
	{
		CONSOLE_DEBUG("No PhaseOne cameras found!!!");
	}
	return(returnCode);
}


//**************************************************************************************
void  CameraSdkLoggingHandler(const char *errorMsg, P1::CameraSdk::LogMessageLevel logMsgLvl)
{
	switch(logMsgLvl)
	{
		case P1::CameraSdk::LogMessageLevel::ERROR:
			CONSOLE_DEBUG_W_STR("ERROR:\t",	errorMsg);
			break;

		case P1::CameraSdk::LogMessageLevel::WARNING:
			CONSOLE_DEBUG_W_STR("WARN :\t",	errorMsg);
			break;

		case P1::CameraSdk::LogMessageLevel::INFO:
			CONSOLE_DEBUG_W_STR("INFO :\t",	errorMsg);
			break;

		case P1::CameraSdk::LogMessageLevel::VERBOSE:
			break;
	}
}

//**************************************************************************************
static  int P1PropertyQSortProc(const void *e1, const void *e2)
{
TYPE_PhaseOneProp	*obj1, *obj2;
int					returnValue;

	obj1		=	(TYPE_PhaseOneProp *)e1;
	obj2		=	(TYPE_PhaseOneProp *)e2;

	returnValue	=	obj1->P1propID - obj2->P1propID;
	return(returnValue);
}

//**************************************************************************************
CameraDriverPhaseOne::CameraDriverPhaseOne(const char *ipAddress, const int deviceNum)
					:CameraDriver()
{
bool									isConnected;
int										returnCode;		//*	0 means OK
P1::CameraSdk::PropertySpecification	property;
P1::CameraSdk::PropertyValue			exposureProgram;
char									*argCharPtr;
char									myPropertyName[256];
char									myPropertyString[256];
int										notHandledCnt;
int										handledCnt;
P1::CameraSdk::Version					p1SDKversion;
char									p1SDK_VersionStr[64];
bool									propWasHandled;
	//----------------------------------------------
	p1SDKversion	=	P1::CameraSdk::GetSdkVersion();

	sprintf(p1SDK_VersionStr, "SDK V%d.%d.%d",	p1SDKversion.major,
												p1SDKversion.minor,
												p1SDKversion.build);

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("Creating PhaseOne device number ", deviceNum);

	P1::CameraSdk::SetSdkLoggingHandler(CameraSdkLoggingHandler);

	cP1Camera	=	P1::CameraSdk::Camera::OpenIpCamera(ipAddress, "", 2);

	gVerbose				=	true;
	cVerboseDebug			=	false;

	cCameraID				=	deviceNum;
	cSimulatedState			=   kExposure_Idle;
	//*	set defaults
	strcpy(cDeviceManufAbrev,		"P1");
	strcpy(cCommonProp.Name,		"PhaseOne--");
	strcpy(cCommonProp.Description,	"PhaseOne--");
	strcpy(cCommonProp.Name,		"PhaseOne--");
	strcpy(cDeviceManufacturer,		"PhaseOne--");

	//*	set some defaults for testing
	cTempReadSupported		=	true;
	cOffsetSupported		=	true;
	cIsColorCam				=	true;

	//*	In order to simulate a particular image size
	cCameraProp.CameraXsize		=	1500;
	cCameraProp.CameraYsize		=	1000;

	cCameraProp.GainMin			=	0;
	cCameraProp.GainMax			=	10;
	cCameraProp.ElectronsPerADU	=	65000;

	//*	from the internet search on "phase one iq4 150mp pixel size"
	cCameraProp.PixelSizeX		=	2.5;
	cCameraProp.PixelSizeY		=	2.5;

	strcpy(cCameraProp.SensorName,	"BSI");

	returnCode	=	0;

	//--------------------------------------------------------------
	try
	{
		// prepare to set cameras "exposure program" to manual
		// this will allow us to set ISO, aperture and shutter time manually.
		exposureProgram.mType	=	P1::CameraSdk::kPropertyTypeEnum;
		exposureProgram.mInt	=	0;

		// Only change exposure program - if not already 0
		if (cP1Camera.GetProperty(kP1_PropertyID_ExposureProgram).mInt != 0)
		{
			CONSOLE_DEBUG("Calling cP1Camera.SetProperty()");
			cP1Camera.SetProperty(kP1_PropertyID_ExposureProgram, exposureProgram);
		}
		else
		{
			CONSOLE_DEBUG("mInt is already set to 0");
		}

		//-----------------------------------------------------------------------
		CONSOLE_DEBUG("Get a list of available properties");
		notHandledCnt	=	0;
		handledCnt		=	0;

		std::vector<uint32_t> propertyIdList = cP1Camera.GetAllPropertyIds();

		cP1PropCnt	=	0;
		for (auto propertyId : propertyIdList)
		{

			handledCnt++;
			// Get the property specification for the current propertyId
			property	=	cP1Camera.GetPropertySpec(propertyId);
			// Print a brief one-line description to console
//			std::cout << "[" << propertyId << "] " << property.mName << " : " << property.mValue.ToString() << " , " << std::endl;
//			printf("[%4d] %-30s\t= %-30s\t- %-20s\t",	propertyId,
//														property.mName.c_str(),
//														property.mValue.ToString().c_str(),
//														property.mValue.mPresentationString.c_str());



			strcpy(myPropertyName,		property.mName.c_str());
			strcpy(myPropertyString,	property.mValue.mString.c_str());
			if (strlen(myPropertyString) == 0)
			{
				strcpy(myPropertyString, property.mValue.mPresentationString.c_str());
			}
			StripCRLF(myPropertyName);
			StripCRLF(myPropertyString);
//			CONSOLE_DEBUG(myPropertyString);
			propWasHandled	=	true;
			CONSOLE_DEBUG_W_2STR("Property:", myPropertyName, myPropertyString);
			switch(propertyId)
			{
				//	[1] Manufacturer : (String) : Phase One ,
				case kP1_PropertyID_Manufacturer:
					strcpy(cDeviceManufacturer,	myPropertyString);
					break;

				//	[2] Model : (String) : IQ4 150MP ,
				case kP1_PropertyID_Model:
					strcpy(cCommonProp.Name,	"PhaseOne-");
					strcat(cCommonProp.Name,	myPropertyString);
					break;

				//	[3] Serial : (String) : JD023085 ,
				case kP1_PropertyID_Serial:
					strcpy(cDeviceSerialNum,	myPropertyString);
					break;

				//	[4] Firmware : (String) : 6.03.18 ,
				case kP1_PropertyID_Firmware:
					strcpy(cDeviceFirmwareVersStr,	myPropertyString);
					break;

				//	[24] Sensor Area : (Area) : { W: 14308, H: 10760 } ,
				case kP1_PropertyID_SensorArea:
					argCharPtr	=	strcasestr(myPropertyString, "w=");
					if (argCharPtr != NULL)
					{
						argCharPtr				+=	2;
						cCameraProp.CameraXsize	=	atoi(argCharPtr);
					}
					argCharPtr	=	strcasestr(myPropertyString, "h=");
					if (argCharPtr != NULL)
					{
						argCharPtr				+=	2;
						cCameraProp.CameraYsize	=	atoi(argCharPtr);
					}
					break;

				//	[1053] Shutter Speed Max : (Float) : 3600 ,
				case kP1_PropertyID_ShutterSpeedMax:
					cCameraProp.ExposureMax_seconds	=	property.mValue.mDouble;
					cCameraProp.ExposureMax_us		=	property.mValue.mDouble * 1000000.0;

					break;

				//	[1054] Shutter Speed Min : (Float) : 0.00025 ,
				case kP1_PropertyID_ShutterSpeedMin:
					cCameraProp.ExposureMin_seconds	=	property.mValue.mDouble;
					cCameraProp.ExposureMin_us		=	property.mValue.mDouble * 1000000.0;
					break;

				default:
					propWasHandled	=	false;
					notHandledCnt++;
//					CONSOLE_DEBUG_W_2STR("Not handled:", myPropertyName, myPropertyString);
//					printf("Not Handled!!!");
					break;
			}
//			printf("\r\n");

			//*	keep a copy of the info for web display
			if (cP1PropCnt < kMaxP1PropCnt)
			{
				cP1Prop[cP1PropCnt].P1propID						=	propertyId;
				cP1Prop[cP1PropCnt].P1propHandled					=	propWasHandled;
				strcpy(cP1Prop[cP1PropCnt].P1propName,					property.mName.c_str());
				strcpy(cP1Prop[cP1PropCnt].P1propToString,				property.mValue.ToString().c_str());
				strcpy(cP1Prop[cP1PropCnt].P1propPresentationString,	property.mValue.mPresentationString.c_str());
				cP1PropCnt++;
			}
			else
			{
				CONSOLE_DEBUG("Ran out of room in property table, increase kMaxP1PropCnt");
			}
		}
		qsort(cP1Prop, cP1PropCnt, sizeof(TYPE_PhaseOneProp), P1PropertyQSortProc);

		strcpy(cCommonProp.Description,	"PhaseOne-");
		strcat(cCommonProp.Description,	p1SDK_VersionStr);
		strcat(cCommonProp.Description,	"-FW:");
		strcat(cCommonProp.Description,	cDeviceFirmwareVersStr);

		CONSOLE_DEBUG_W_NUM("total count  \t=", handledCnt);
		CONSOLE_DEBUG_W_NUM("notHandledCnt\t=", notHandledCnt);
//[1206] Message : (String) :  ,
//[1207] Acknowledge : (Bool) : False ,
//[1000] Exposure Program : (Enum) : 0 ,
//[120] ISO : (Float) : 320 ,
//[1003] Aperture : (Float) : 9 ,
//[1002] Shutter Speed : (Float) : 0.1 ,
//[122] White Balance : (Enum) : 0 ,
//[230] Storage Mode : (Enum) : 1 ,
//[1303] XQD/CFX : (Enum) : 2 ,
//[1304] SD : (Enum) : 0 ,
//[1305] SD Storage : (Enum) : 0 ,
//[1302] Host Storage : (Enum) : 2 ,
//[104] Image Orientation : (Enum) : 4294967294 ,
//[102] File Format : (Enum) : 2 ,
//[103] Compression : (Enum) : 1 ,
//[1008] Shutter Mode : (Enum) : 1 ,
//[1028] Shutter Mode Actual : (Enum) : 1 ,
//[1010] AF Mode : (Enum) : 0 ,
//[1011] AF Drive : (Enum) : 0 ,
//[1009] Drive : (Enum) : 0 ,
//[1020] Mirror Up : (Bool) : False ,
//[1006] Metering mode : (Enum) : 0 ,
//[1004] Exposure Comp. : (Float) : 0 ,
//[1021] Exp Error : (Float) : 0 ,
//[1013] Auto Exposure Out Of Range : (Bool) : False ,
//[1151] Model : (String) :  ,
//[1161] Serial : (String) :  ,
//[1152] Firmware : (String) :  ,
//[1025] Leaf Shutter Count : (UInt) : 0 ,
//[1026] Aperture Count : (UInt) : 0 ,
//[1243] XQD/CFX Availability : (Bool) : False ,
//[1460] XQD/CFX remaining free space : (UInt) : 0 ,
//[1461] XQD/CFX size : (UInt) : 0 ,
//[1462] SD Card Availability : (Bool) : False ,
//[1463] SD Card Remaining Free Space : (UInt) : 0 ,
//[1464] SD Card Size : (UInt) : 0 ,
//[1420] Current Temperature : (Int) : -273 ,
//[1425] Throttle Temperature T1 : (Int) : -273 ,
//[1426] Throttle Temperature T2 : (Int) : -273 ,
//[1421] Under Temperature : (Int) : -273 ,
//[1422] Over Temperature : (Int) : -273 ,
//[1423] Warning Temperature : (Int) : -273 ,
//[1424] Critical Temperature : (Int) : -273 ,
//[1022] Focus Adjust (Discrete) : (Enum) : 0 ,
//[1023] Focus Adjust (Continuous) : (Int) : 0 ,
//[1030] Focus Marker : (Enum) : 0 ,
//[1029] RemoteFocus : (Bool) : False ,
//[128] Black Ref. : (Enum) : 1 ,
//[141] Antiflickering : (Enum) : 0 ,
//[1201] Ready For Capture : (Bool) : False ,
//[1200] Unsaved Images : (UInt) : 0 ,
//[1205] LiveView Active : (Bool) : False ,
//[1057] ISO Auto : (Bool) : False ,
//[1051] ISO Min : (Float) : 50 ,
//[1052] ISO Max : (Float) : 12800 ,
//[1058] Shutter Speed Auto : (Bool) : False ,
//[1053] Shutter Speed Max : (Float) : 3600 ,
//[1054] Shutter Speed Min : (Float) : 0.00025 ,
//[1059] Aperture Auto : (Bool) : False ,
//[1055] Aperture Min : (Float) : 1.4 ,
//[1056] Aperture Max : (Float) : 45 ,
//[1230] LiveView AutoGain : (Enum) : 1 ,
//[1470] Power Share With Camera : (Bool) : True ,
//[1220] HDMI LiveView : (Bool) : False ,
//[1221] HDMI Overlay : (Bool) : False ,
//[1244] Overlay Layout : (Enum) : 0 ,
//[1245] Transparency : (UInt) : 170 ,
//[1246] Preview Time : (Enum) : 3 ,
//[1247] Preview Orientation : (Enum) : 1 ,
//[1241] Preview : (Enum) : 0 ,
//[1248] HDMI LiveView Zoom : (Enum) : 0 ,
//[1310] Zoom Pan X : (UInt) : 50 ,
//[1311] Zoom Pan Y : (UInt) : 50 ,
//[1312] Focus Peaking : (Bool) : True ,
//[1313] Focus Peaking Threshold : (UInt) : 200 ,
//[1251] Static Setup : (Bool) : False ,
//[1252] IP Address : (String) :  ,
//[1253] Netmask : (String) :  ,
//[1254] Gateway : (String) :  ,
//[1258] DHCP Mode : (Enum) : 1 ,
//[1256] DHCP Address : (String) : 192.168.1.175 ,
//[1257] Status/Action : (Enum) : 3 ,
//[1260] Bonjour Is Running : (Bool) : True ,
//[1261] Bonjour Enable : (Bool) : True ,
//[1262] Bonjour Enabled At Power On : (Bool) : True ,
//[1263] Host Name : (String) : phaaeone  ,
//[1290] Custom WB : (Bool) : False ,
//[1293] Network Setup : (Bool) : False ,
//[1291] Action: Restore : (Bool) : False ,
//[500] Service Command : (String) :  ,
//[14] Unit Name : (String) :  ,
//[13] Unit Number : (UInt) : 0 ,
//[12] System Serial Number : (String) :  ,
//[2] Model : (String) : IQ4 150MP ,
//[1] Manufacturer : (String) : Phase One ,
//[18] Profile : (String) : Standard ,
//[3] Serial : (String) : JD023085 ,
//[4] Firmware : (String) : 6.03.18 ,
//[10] Model Id : (UInt) : 293 ,
//[200] Capture Count : (UInt) : 49296 ,
//[24] Sensor Area : (Area) : { W: 14308, H: 10760 } ,
//[25] Sensor Active Rect. : (Rect) : { X: 102, Y: 106, W: 14204, H: 10652 } ,
//[1471] Primary Power Source : (Enum) : 0 ,
//[204] Back Battery State : (Enum) : 0 ,
//[206] Camera Battery State : (Enum) : 0 ,
//[208] System Battery State : (Enum) : 0 ,
//[201] Back Battery Capacity : (UInt) : 100 ,
//[205] Camera Battery Capacity : (UInt) : 50 ,
//[207] System Battery Capacity : (UInt) : 100 ,
//[1401] Diagnostics To Storage Config : (UInt) : 0 ,
//.

//		//-----------------------------------------------------------------------
//		// Define the PropertyId for the 'Model' property
////		uint32_t modelId	=	IQ4150MP::Property::Model;		// ID is defined by the Camera Property Model
//		uint32_t modelId	=	P1::CameraSdk::Property::Model;		// ID is defined by the Camera Property Model
//
//		CONSOLE_DEBUG_W_INT32("modelId\t=",	modelId);
//		// Get the PropertyValue object from the camera
//		PropertyValue cameraModel = camera.GetProperty(modelId);
//
//		// Print its value to the console
//		std::cout << "Camera Model: " << cameraModel << std::endl;
	}
	catch (P1::CameraSdk::SdkException exception)
	{
		CONSOLE_DEBUG("P1::CameraSdk::SdkException exception");
		// Exception from CameraSDK
		std::cout << "CameraSDK Exception: " << exception.what() << " Code:"
			<< exception.mErrorCode <<
			std::endl;
		CONSOLE_DEBUG_W_STR("exception.what()    :", exception.what());
		CONSOLE_DEBUG_W_NUM("exception.mErrorCode:", exception.mErrorCode);
		returnCode	=	-1;
	}
	catch (...)
	{
		CONSOLE_DEBUG("... exception");
		// Any other exception - just in case
		CONSOLE_DEBUG("Argh - we got an exception");
		returnCode	=	-1;
	}

	cCameraProp.NumX			=	cCameraProp.CameraXsize;
	cCameraProp.NumY			=	cCameraProp.CameraYsize;

	AddReadoutModeToList(kImageType_RAW8);
	AddReadoutModeToList(kImageType_RAW16);
	AddReadoutModeToList(kImageType_RGB24);


	isConnected		=	AlpacaConnect();
	if (isConnected)
	{

	}
	else
	{
		CONSOLE_DEBUG("Failed to Connect!!!!!!!!");
	}
	SetImageType(kImageType_RGB24);

	DumpCameraProperties(__FUNCTION__);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

}

//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverPhaseOne::~CameraDriverPhaseOne(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
bool	CameraDriverPhaseOne::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cCommonProp.Connected	=	true;
	return(cCommonProp.Connected);
}


//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		//	Only valid if CanSetCCDTemperature is true.
		cCameraProp.CCDtemperature		=	5.56;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotSupported;
		strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Write_BinX(const int newBinXvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

	CONSOLE_DEBUG(__FUNCTION__);

	cCameraProp.BinX	=	newBinXvalue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Write_BinY(const int newBinYvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCameraProp.BinY	=	newBinYvalue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	*cameraGainValue	=	cCameraProp.Gain;
	alpacaErrCode		=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	cCameraProp.Gain	=	newGainValue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Read_Offset(int *cameraOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	*cameraOffsetValue	=	cCameraProp.Offset;
	alpacaErrCode		=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Write_Offset(const int newOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	cCameraProp.Offset	=	newOffsetValue;
	alpacaErrCode		=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverPhaseOne::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS				alpacaErrCode	=	kASCOM_Err_NotImplemented;
double							durationSeconds;
int								returnCode;		//*	0 means OK
P1::CameraSdk::PropertyValue	isoValue;
P1::CameraSdk::PropertyValue	apertureValue;
P1::CameraSdk::PropertyValue	shutterTimeValue;

	CONSOLE_DEBUG(__FUNCTION__);
	returnCode	=	0;
	if (cCommonProp.Connected)
	{
		cInternalCameraState		=	kCameraState_TakingPicture;
		cCameraProp.CameraState		=   kALPACA_CameraState_Exposing;
		cCameraProp.ImageReady		=	false;

		durationSeconds	=	(exposureMicrosecs * 1.0) / 1000000.0;

		durationSeconds	=	2;

		// Setup ISO, Aperture and Shutter-time
		isoValue.mType				=	P1::CameraSdk::kPropertyTypeFloat64;
		isoValue.mDouble			=	200;

		apertureValue.mType			=	P1::CameraSdk::kPropertyTypeFloat64;
		apertureValue.mDouble		=	5.0;

		shutterTimeValue.mType		=	P1::CameraSdk::kPropertyTypeFloat64;
		shutterTimeValue.mDouble	=	0.125;
		try
		{
			cP1Camera.SetProperty(kP1_PropertyID_ISO,			isoValue);
			cP1Camera.SetProperty(kP1_PropertyID_Aperture,		apertureValue);
			cP1Camera.SetProperty(kP1_PropertyID_ShutterSpeed,	shutterTimeValue);
			CONSOLE_DEBUG("EnableImageReceiving");
			cP1Camera.EnableImageReceiving(true);
			CONSOLE_DEBUG("Sleeping");
			// Necessary to sleep, otherwise TriggerCapture will start before EnableImageReceiving is done
	//		std::this_thread::sleep_for(std::chrono::milliseconds(200));
			usleep(200 * 1000);
			CONSOLE_DEBUG("TriggerCapture");
			cP1Camera.TriggerCapture();

			P1::CameraSdk::IIQImageFile imageFile	=	cP1Camera.WaitForImage(80000);
			CONSOLE_DEBUG("Image received");
		}
		catch (P1::CameraSdk::SdkException exception)
		{
			CONSOLE_DEBUG("P1::CameraSdk::SdkException exception");
			// Exception from CameraSDK
//			std::cout << "CameraSDK Exception: " << exception.what() << " Code:"
//				<< exception.mErrorCode <<
//				std::endl;
			CONSOLE_DEBUG_W_NUM("exception.mErrorCode:", exception.mErrorCode);
			CONSOLE_DEBUG_W_STR("exception.what()    :", exception.what());
			returnCode	=	-1;
		}
		catch (...)
		{
			CONSOLE_DEBUG("... exception");
			// Any other exception - just in case
			CONSOLE_DEBUG("Argh - we got an exception");
			returnCode	=	-1;
		}

		cInternalCameraState		=	kCameraState_Idle;
		cCameraProp.CameraState		=   kALPACA_CameraState_Idle;

		SetLastExposureInfo();
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
bool	CameraDriverPhaseOne::GetImage_ROI_info(void)
{
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;
	return(true);
}

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverPhaseOne::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	myExposureStatus;
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

//			CONSOLE_DEBUG_W_LONG("deltaTime_secs\t=",			deltaTime_secs);
			if ((deltaTime_secs > 2) && (cInternalCameraState == kCameraState_Idle))
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
//	CONSOLE_DEBUG_W_NUM("myExposureStatus\t=",			myExposureStatus);

	return(myExposureStatus);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("newImageType\t=",			newImageType);

	if (cCommonProp.Connected)
	{
		cROIinfo.currentROIwidth	=	cCameraProp.CameraXsize;
		cROIinfo.currentROIheight	=	cCameraProp.CameraYsize;
		cROIinfo.currentROIbin		=   1;

		switch(newImageType)
		{
			case kImageType_RAW8:
				cROIinfo.currentROIimageType	=	kImageType_RAW8;
				break;

			case kImageType_RAW16:
				cROIinfo.currentROIimageType	=	kImageType_RAW16;
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
TYPE_ASCOM_STATUS	CameraDriverPhaseOne::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					bytesPerPixel;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cCommonProp.Connected)
	{
		CONSOLE_DEBUG_W_NUM("currentROIimageType\t=",			cROIinfo.currentROIimageType);
		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:	bytesPerPixel	=	1;	break;
			case kImageType_RAW16:	bytesPerPixel	=	2;	break;
			case kImageType_RGB24:	bytesPerPixel	=	3;	break;
			default:				bytesPerPixel	=	3;	break;
		}
		CONSOLE_DEBUG_W_NUM("bytesPerPixel\t=",			bytesPerPixel);

		AllocateImageBuffer(-1);		//*	let it figure out how much
		if (cCameraDataBuffer != NULL)
		{
			//--------------------------------------------
			//*	simulate an image
//			CreateFakeImageData(cCameraDataBuffer, cCameraProp.CameraXsize, cCameraProp.CameraYsize, bytesPerPixel);
			cCameraProp.ImageReady	=	true;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_ABORT("Failed to allocate image buffer");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG("Not connected");
		CONSOLE_ABORT("Not connected");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
void	CameraDriverPhaseOne::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
int					iii;
char				lineBuffer[256];
int					mySocketFD;
char				clmPrefixStr[64];

	mySocketFD	=	reqData->socket;
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>PhaseOne camera properties</H2><P>\r\n");

	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	//*-----------------------------------------------------------
	SocketWriteData(mySocketFD,	"<TR>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD><CENTER>ID</TD>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD><CENTER>Name</TD>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD><CENTER>ToString</TD>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD><CENTER>PresString</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	for (iii=0; iii< cP1PropCnt; iii++)
	{
		if (cP1Prop[iii].P1propHandled)
		{
			strcpy(clmPrefixStr,	"\t<TD><FONT COLOR=GREEN>");
		}
		else
		{
			strcpy(clmPrefixStr,	"\t<TD>");
		}
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			sprintf(lineBuffer,	"\t<TD><CENTER>%ld</TD>\r\n",	cP1Prop[iii].P1propID);
			SocketWriteData(mySocketFD,	lineBuffer);

			SocketWriteData(mySocketFD,	clmPrefixStr);
			SocketWriteData(mySocketFD,	cP1Prop[iii].P1propName);
			SocketWriteData(mySocketFD,	"</TD>\r\n");

			SocketWriteData(mySocketFD,	clmPrefixStr);
			SocketWriteData(mySocketFD,	cP1Prop[iii].P1propToString);
			SocketWriteData(mySocketFD,	"</TD>\r\n");

			SocketWriteData(mySocketFD,	clmPrefixStr);
			SocketWriteData(mySocketFD,	cP1Prop[iii].P1propPresentationString);
			SocketWriteData(mySocketFD,	"</TD>\r\n");

		SocketWriteData(mySocketFD,	"</TR>\r\n");
	}
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	sprintf(lineBuffer,	"<BR>Total properties %d\r\n",	cP1PropCnt);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
}


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_PHASEONE_)
