//**************************************************************************
//*	Name:			cameradriver_ASI.c
//*
//*	Author:			Mark Sproul (C) 2019
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
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*
//*	Fits Info
//*		http://tigra-astronomy.com/sbfitsext-guidelines-for-fits-keywords
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 14,	2019	<MLS> Started on camera code
//*	Apr 15,	2019	<MLS> Added command table for camera
//*	Apr 17,	2019	<MLS> Added Camera_OutputHTML()
//*	Apr 17,	2019	<MLS> Read temperature working on ASI1600
//*	Apr 27,	2019	<MLS> Added error messages for ASI failures
//*	Apr 29,	2019	<MLS> Reading images from ASI camera
//*	Apr 29,	2019	<MLS> Started using openCV for the image
//*	Apr 29,	2019	<MLS> OpenCV image saving working
//*	Apr 30,	2019	<MLS> Got openCV installed on the Stellarmate
//*	May  2,	2019	<MLS> Added alpacaErrMsg to all operation functions
//*	May 10,	2019	<MLS> Added Camera_Get_Lastexposureduration()
//*	May 10,	2019	<MLS> Added Get_ASI_SensorTemp(), Get_ASI_ImageTypeString()
//*	May 13,	2019	<MLS> Cant close the camera until after the data is read
//*	May 13,	2019	<MLS> We now always keep the camera open
//*	May 14,	2019	<MLS> Recovery from swapped cameras now working
//*	May 14,	2019	<MLS> Added CheckForClosedError() & CloseASICamera()
//*	May 23,	2019	<MLS> Added Camera_Readoutmode() & Camera_Readoutmodes()
//*	May 24,	2019	<MLS> Started implementing ASCOM/Alpaca error codes
//*	Jun  5,	2019	<MLS> Compiling and running on 64 bit linux
//*	Jun  8,	2019	<MLS> Started on WriteFireCaptureTextFile()
//*	Jun 12,	2019	<MLS> Adding OpenCV support to camera interface
//*	Jun 21,	2019	<MLS> Added CameraThread()
//*	Jun 25,	2019	<MLS> ASI Camera working on nvidia Jetson board
//*	Jun 26,	2019	<MLS> Added _USE_THREADS_FOR_ASI_CAMERA_
//*	Jun 26,	2019	<MLS> CameraThread() working
//*	Jun 26,	2019	<MLS> Added IsValidCamera()
//*	Jun 27,	2019	<MLS> AVI output working from "savemovie" very slow on R-Pi
//*	Sep  3,	2019	<MLS> Started on C++ version of ASI camera driver
//*	Oct  2,	2019	<MLS> Added Read_ImageData()
//*	Oct  4,	2019	<MLS> C++ version now reading image correctly
//*	Nov  7,	2019	<MLS> Added ASI data structures to class
//*	Dec 23,	2019	<MLS> ASI camera fails to take picture on Raspberry Pi-4 (ASI120MC)
//*	Dec 26,	2019	<MLS> Yang Zhou@ZWO says USB3.0 cameras work fine on R-Pi4
//*	Dec 26,	2019	<MLS> Code compiled on R-Pi4 works on R-Pi3B+
//*	Dec 26,	2019	<MLS> Verified: ASI1600MC-Pro & ASI120MM-S work on R-Pi4 (both USB3)
//*	Dec 27,	2019	<MLS> Verified: ASI290MM & ASI120MM Mini work on R-Pi4
//*	Dec 27,	2019	<MLS> Verified: ASI120MC-S(1600MC-C) works on R-Pi4
//*	Dec 27,	2019	<MLS> NOT WORKING on R-Pi4, ASI120 and ASI034MC
//*	Jan 28,	2020	<MLS> ASI120MC does NOT work on NVIDIA Jetson board
//*	Jan 29,	2020	<MLS> ASI178MC is working on NVIDIA Jetson board
//*	Feb 16,	2020	<MLS> Added Read_Gain() & Write_Gain()
//*	Mar  8,	2020	<MLS> Camera temp control working on ASI cameras
//*	Jun 11,	2020	<MLS> Added timestamp option to video output
//*	Jun 16,	2020	<MLS> Added timestamp text (csv) file for video output
//*	Aug 11,	2020	<MLS> Added auto exposure to video output
//*	Mar 26,	2021	<MLS> Added Write_Offset() & Read_Offset()
//*	Mar 26,	2021	<MLS> Offset value read/write working in ASI cameras
//*	Oct 13,	2021	<MLS> Added Write_BinX() & Write_BinY() to ASI driver
//*****************************************************************************
//*	Length: unspecified [text/plain]
//*	Saving to: "imagearray.1"
//*
//*		[		<=>		   ] 163,891,406 1.43MB/s   in 1m 50s
//*
//*	2019-04-30 21:22:24 (1.42 MB/s) - "imagearray.1" saved [163891406]
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ASI_)

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



#ifdef _USE_OPENCV_
	#include "opencv/highgui.h"
	#include "opencv2/highgui/highgui_c.h"
#endif

#include	"ASICamera2.h"

#include	"JsonResponse.h"
#include	"eventlogging.h"
#include	"helper_functions.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"cameradriver_ASI.h"




#define	kMaxCameraCnt	5

static int				gASIcameraCount	=	0;


static void	Get_ASI_SensorName(const char *cameraName, char *retSensorName);
static void	Get_ASI_ErrorMsg(ASI_ERROR_CODE asiErrorCode, char *asiErrorMessage);
static void	Get_ASI_ImageTypeString(ASI_IMG_TYPE imageType, char *typeString);

//**************************************************************************************
void	CreateASI_CameraObjects(void)
{
int		devNum;
char	driverVersionString[64];
char	rulesFileName[]	=	"asi.rules";		//	"99-asi.rules";
bool	rulesFileOK;


//	CONSOLE_DEBUG_W_LONG("sizeof(CameraDriver)\t=",		(long)sizeof(CameraDriver));
//	CONSOLE_DEBUG_W_LONG("sizeof(unsigned long)\t=",	(long)sizeof(unsigned long));
//	CONSOLE_DEBUG_W_LONG("sizeof(long)\t\t=",			(long)sizeof( long));
//	CONSOLE_DEBUG_W_LONG("sizeof(int)\t\t=",			(long)sizeof( int));
//	CONSOLE_DEBUG_W_LONG("sizeof(uint32_t)\t=",			(long)sizeof( uint32_t));
//	CONSOLE_DEBUG_W_LONG("sizeof(ASI_CAMERA_INFO)\t=",	(long)sizeof( ASI_CAMERA_INFO));

	strcpy(driverVersionString,	ASIGetSDKVersion());
	LogEvent(	"camera",
				"Library version (ZWO-ASI)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("camera", "ZWO", driverVersionString);

	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK == false)
	{
		LogEvent(	"camera",
					"Problem with ZWO-ASI rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}


	gASIcameraCount	=	ASIGetNumOfConnectedCameras();
	CONSOLE_DEBUG_W_NUM("gASIcameraCount\t=", gASIcameraCount);
	for (devNum=0; devNum < gASIcameraCount; devNum++)
	{
		new CameraDriverASI(devNum);
	}
}


//**************************************************************************************
CameraDriverASI::CameraDriverASI(const int deviceNum)
	:CameraDriver()
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cCameraID	=	deviceNum;
	strcpy(cDeviceManufAbrev,	"ZWO");
	ReadASIcameraInfo();

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
CameraDriverASI::~CameraDriverASI(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
void	CameraDriverASI::ReadASIcameraInfo(void)
{
int					numberOfCtrls;
int					ii;
ASI_ERROR_CODE		asiErrorCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceVersion,	ASIGetSDKVersion());
	strcpy(cDeviceManufacturer,	"ZWO");


	printf("----------------------------------\r\n");
	asiErrorCode	=	ASIGetCameraProperty(&cAsiCameraInfo, cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		LogEvent(	"camera",
					"ZWO Camera detected",
					NULL,
					kASCOM_Err_Success,
					cAsiCameraInfo.Name);

		printf("- %d %-20s\t%ld\t%ld\t%d\t%f\r\n",	cCameraID,
													cAsiCameraInfo.Name,
													cAsiCameraInfo.MaxWidth,
													cAsiCameraInfo.MaxHeight,
													cAsiCameraInfo.IsColorCam,
													cAsiCameraInfo.PixelSize);

		printf("SupportedBins =");
		ii	=	0;
		while ((cAsiCameraInfo.SupportedBins[ii] > 0) && (ii<16))
		{
			printf("\t%d",	cAsiCameraInfo.SupportedBins[ii]);
			ii++;
		}
		printf("\r\n");

		//*	check if we have a valid RefId
		if (strlen(gObseratorySettings.RefID) > 0)
		{
			//*	ASCOM only displays the name, having more than 1 of the same camera
			//*	it became confusing as to which camera to select from the ASCOM list.
			//*	this helps figure out when using a strictly ASCOM app on Windows
			strcpy(cCommonProp.Name, gObseratorySettings.RefID);
			strcat(cCommonProp.Name, "-");
			strcat(cCommonProp.Name, cAsiCameraInfo.Name);
			CONSOLE_DEBUG_W_STR("cCommonProp.Name", cCommonProp.Name);
		}
		else
		{
			strcpy(cCommonProp.Name, cAsiCameraInfo.Name);
		}

		cCameraID						=	cAsiCameraInfo.CameraID;
		cCameraProp.CameraXsize			=	cAsiCameraInfo.MaxWidth;
		cCameraProp.CameraYsize			=	cAsiCameraInfo.MaxHeight;
		cIsColorCam						=	cAsiCameraInfo.IsColorCam;
		cBayerPattern					=	cAsiCameraInfo.BayerPattern;
		cCameraProp.PixelSizeX			=	cAsiCameraInfo.PixelSize;
		cCameraProp.PixelSizeY			=	cAsiCameraInfo.PixelSize;
		cCameraProp.HasShutter			=	cAsiCameraInfo.MechanicalShutter;
		cSt4Port						=	cAsiCameraInfo.ST4Port;
		cIsCoolerCam					=	cAsiCameraInfo.IsCoolerCam;
		cIsUSB3Host						=	cAsiCameraInfo.IsUSB3Host;
		cIsUSB3Camera					=	cAsiCameraInfo.IsUSB3Camera;
		cCameraProp.ElectronsPerADU		=	cAsiCameraInfo.ElecPerADU;
		cBitDepth						=	cAsiCameraInfo.BitDepth;
		cIsTriggerCam					=	cAsiCameraInfo.IsTriggerCam;
		cCameraProp.ExposureResolution	=	0.000001;

		cCameraProp.NumX		=	cCameraProp.CameraXsize;
		cCameraProp.NumY		=	cCameraProp.CameraYsize;

		Get_ASI_SensorName(cCommonProp.Name, cSensorName);

	//	CONSOLE_DEBUG_W_NUM("cCameraID\t\t=", cCameraID);

		//*	figure out max bin values
		ii	=	0;
		while ((cAsiCameraInfo.SupportedBins[ii] > 0) && (ii<16))
		{
			if (cAsiCameraInfo.SupportedBins[ii] > cCameraProp.MaxbinX)
			{
				//*	Maximum binning for the camera X axis
				cCameraProp.MaxbinX	=	cAsiCameraInfo.SupportedBins[ii];
				//*	Maximum binning for the camera Y axis
				cCameraProp.MaxbinY	=	cAsiCameraInfo.SupportedBins[ii];
			}
			ii++;
		}

		//*	get the supported image formats
		ii	=	0;
		while ((ii<8) && (cAsiCameraInfo.SupportedVideoFormat[ii] != ASI_IMG_END))
		{
			switch(cAsiCameraInfo.SupportedVideoFormat[ii])
			{
				case ASI_IMG_RAW8:
					AddReadoutModeToList(kImageType_RAW8);
					break;

				case ASI_IMG_RGB24:
					AddReadoutModeToList(kImageType_RGB24);
					break;

				case ASI_IMG_RAW16:
					AddReadoutModeToList(kImageType_RAW16);
					break;

				case ASI_IMG_Y8:
					AddReadoutModeToList(kImageType_Y8);
					break;

				default:
					CONSOLE_DEBUG_W_NUM("Unknown image type", cAsiCameraInfo.SupportedVideoFormat[ii]);
					break;
			}
			ii++;
		}
	}
	else
	{
		printf("ASIGetCameraProperty failed with return code %d\r\n", asiErrorCode);
	}

	asiErrorCode	=	ASIOpenCamera(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		asiErrorCode	=	ASIInitCamera(cCameraID);
		asiErrorCode	=	ASIGetNumOfControls(cCameraID, &numberOfCtrls);
		for (ii=0; ii< numberOfCtrls; ii++)
		{
			ReadASIcontrol(ii);
		}
		//*	read the supported modes
		asiErrorCode	=	ASIGetCameraSupportMode(cCameraID, &supportedModes);
		//	for (ii=0; ii<16; ii++)
		//	{
		//		CONSOLE_DEBUG_W_NUM("supportedModes\t=",	supportedModes.SupportedCameraMode[ii]);
		//	}

		memset(&cAsiSerialNum, 0, sizeof(ASI_ID));
		asiErrorCode	=	ASIGetSerialNumber(cCameraID, &cAsiSerialNum);
		if (ASI_ERROR_GENERAL_ERROR == ASI_SUCCESS)
		{
			strncpy(cDeviceSerialNum, (char *)cAsiSerialNum.id, 8);
			cDeviceSerialNum[8]	=	0;
		}
		else
		{
		//	CONSOLE_DEBUG_W_NUM("ASIGetSerialNumber:asiErrorCode=", asiErrorCode);
		//	sprintf(cDeviceSerialNum, "none%d", cCameraID);
			sprintf(cDeviceSerialNum, "%d", cCameraID);
		}
//		asiErrorCode	=	ASIGetID(cCameraID, &cAsiSerialNum);
//		CONSOLE_DEBUG_W_NUM("ASIGetID:asiErrorCode=", asiErrorCode);
//		CONSOLE_DEBUG_W_STR("cAsiSerialNum.id=", cAsiSerialNum.id);


//		CONSOLE_DEBUG("ASICloseCamera");
		asiErrorCode	=	ASICloseCamera(cCameraID);
	}
	else
	{

	}

#ifdef _USE_THREADS_FOR_ASI_CAMERA_
	if (cThreadIsActive == false)
	{
	int	threadErr;

		cKeepRunning	=	true;
		threadErr		=	pthread_create(&threadID, NULL, &CameraThread, this);
		cThreadIsActive	=	true;
	}
#endif // _USE_THREADS_FOR_ASI_CAMERA_

}

//*****************************************************************************
void	CameraDriverASI::ReadASIcontrol(const int controlNum)
{
ASI_ERROR_CODE		asiErrorCode;
ASI_CONTROL_CAPS	controlCaps;

	asiErrorCode	=	ASIGetControlCaps(cCameraID, controlNum, &controlCaps);
	if (asiErrorCode == ASI_SUCCESS)
	{
		printf("- %-25s\t%-60s\t%d\t%ld\t%ld\n",	controlCaps.Name,
													controlCaps.Description,
													controlCaps.ControlType,
													controlCaps.MinValue,
													controlCaps.MaxValue);

		//******************************************************************************************************************
		//	- 0 ZWO ASI120MC			1280	960	1	3.750000
		//	- Gain					 	Gain															0	0	100
		//	- Exposure				 	Exposure Time(us)										   	1	64	2000000000
		//	- WB_R					 	White balance: Red component									3	1	100
		//	- WB_B					 	White balance: Blue component							   	4	1	100
		//	- Offset				   	offset													  	5	0	20
		//	- BandWidth					The total data transfer rate percentage					 	6	40	100
		//	- Flip					 	Flip: 0->None 1->Horiz 2->Vert 3->Both					  	9	0	3
		//	- AutoExpMaxGain		   	Auto exposure maximum gain value								10	0	100
		//	- AutoExpMaxExpMS		  	Auto exposure maximum exposure value(unit ms)			   	11	1	60000
		//	- AutoExpTargetBrightness  	Auto exposure target brightness value					   	12	50	160
		//	- HighSpeedMode				Is high speed mode:0->No 1->Yes							 	14	0	1
		//	- MonoBin				  	bin R G G B to one pixel for color camera, color will loss  	18	0	1
		//	- Temperature			  	Sensor temperature(degrees Celsius)						 	8	-500	1000
		//	----------------------------------
		//	- 0 ZWO ASI1600MC Pro   	4656	3520	1	3.800000
		//	- Gain					 	Gain															0	0	600
		//	- Exposure				 	Exposure Time(us)										   	1	32	2000000000
		//	- WB_R					 	White balance: Red component									3	1	99
		//	- WB_B					 	White balance: Blue component							   	4	1	99
		//	- Offset				   	offset													  	5	0	100
		//	- BandWidth					The total data transfer rate percentage					 	6	40	100
		//	- Flip					 	Flip: 0->None 1->Horiz 2->Vert 3->Both					  	9	0	3
		//	- AutoExpMaxGain		   	Auto exposure maximum gain value								10	0	600
		//	- AutoExpMaxExpMS		  	Auto exposure maximum exposure value(unit ms)			   	11	1	60000
		//	- AutoExpTargetBrightness  	Auto exposure target brightness value					   	12	50	160
		//	- HardwareBin			  	Is hardware bin2:0->No 1->Yes							   	13	0	1
		//	- HighSpeedMode				Is high speed mode:0->No 1->Yes							 	14	0	1
		//	- MonoBin				  	bin R G G B to one pixel for color camera, color will loss  	18	0	1
		//	- Temperature			  	Sensor temperature(degrees Celsius)						 	8	-500	1000
		//	- CoolPowerPerc				Cooler power percent											15	0	100
		//	- TargetTemp			   	Target temperature(cool camera only)							16	-40	30
		//	- CoolerOn				 	turn on/off cooler(cool camera only)							17	0	1
		//******************************************************************************************************************

		//*	depending on which value it is, we may want to save some of the info
		switch(controlCaps.ControlType)
		{
			case ASI_GAIN:
				cCameraProp.GainMin	=	controlCaps.MinValue;
				cCameraProp.GainMax	=	controlCaps.MaxValue;
				cGain_default		=	controlCaps.DefaultValue;

				break;

			case ASI_EXPOSURE:
				cCameraProp.ExposureMin_us	=	controlCaps.MinValue;
				cCameraProp.ExposureMax_us	=	controlCaps.MaxValue;
				cExposureDefault_us			=	controlCaps.DefaultValue;
				break;

			case ASI_GAMMA:
			case ASI_WB_R:
			case ASI_WB_B:
				break;

			case ASI_OFFSET:
				cCameraProp.Offset		=	controlCaps.DefaultValue;
				cCameraProp.OffsetMin	=	controlCaps.MinValue;
				cCameraProp.OffsetMax	=	controlCaps.MaxValue;
				break;

			case ASI_BANDWIDTHOVERLOAD:
			case ASI_OVERCLOCK:
				break;

			case ASI_TEMPERATURE:				// return 10*temperature
				cTempReadSupported	=	true;
				break;

			case ASI_FLIP:
				cCanFlipImage		=	true;
				break;

			case ASI_AUTO_MAX_GAIN:
			case ASI_AUTO_MAX_EXP:				//micro second
			case ASI_AUTO_TARGET_BRIGHTNESS:	//target brightness
			case ASI_HARDWARE_BIN:
				break;

			case ASI_HIGH_SPEED_MODE:
			//	cCameraProp.CanFastReadout		=	true;
				break;

			case ASI_COOLER_POWER_PERC:
				cCameraProp.CanGetCoolerPower	=	true;
				break;

			case ASI_TARGET_TEMP:				// not need *10
				if (controlCaps.IsWritable)
				{
					cCameraProp.Cansetccdtemperature		=	true;
				}
				break;

			case ASI_COOLER_ON:
			case ASI_MONO_BIN:				//leads to less grid at software bin mode for color camera
			case ASI_FAN_ON:
			case ASI_PATTERN_ADJUST:
			case ASI_ANTI_DEW_HEATER:
				break;

		}
	}
}

//*****************************************************************************
//*	opens the ZWO ASI camera if it is not already open
//*	calls camera init
//*	updates the table
//*	returns ASI error code
//*****************************************************************************
ASI_ERROR_CODE	CameraDriverASI::OpenASIcameraIfNeeded(int foo)
{
ASI_ERROR_CODE		asiErrorCode;
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cCameraID\t=", cCameraID);

	if (cCameraIsOpen == true)
	{
//		CONSOLE_DEBUG("Camera already open");
		asiErrorCode	=	ASI_SUCCESS;
	}
	else
	{
//		CONSOLE_DEBUG("Calling ASIOpenCamera()---------------------------");
		asiErrorCode		=	ASIOpenCamera(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			cCameraIsOpen	=	true;
			asiErrorCode	=	ASIInitCamera(cCameraID);
			if (asiErrorCode == ASI_SUCCESS)
			{
				//*	set the image type
				alpacaErrCode	=	SetImageTypeCameraOpen(cDesiredImageType);
			//	CONSOLE_DEBUG_W_NUM("cDesiredImageType\t=",		cDesiredImageType);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ASIInitCamera() failed with error ", asiErrorCode);
			}
		}
		else if (asiErrorCode == ASI_ERROR_CAMERA_REMOVED)
		{
			CONSOLE_DEBUG("Camera removed error - resetting");
//++			Init_ASICamera();
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ASIOpenCamera() failed with error ", asiErrorCode);
		}
	}
	return(asiErrorCode);
}



//*****************************************************************************
void	CameraDriverASI::CloseASICamera(const int cameraTblIdx)
{
ASI_ERROR_CODE	asiErrorCode;

	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	ASICloseCamera(cameraTblIdx);
	if (asiErrorCode != 0)
	{
		Get_ASI_ErrorMsg(asiErrorCode, cLastCameraErrMsg);
	}
	cCameraIsOpen	=	false;
}


//*****************************************************************************
void	CameraDriverASI::CheckForClosedError(ASI_ERROR_CODE theAsiErrorCode)
{

	CONSOLE_DEBUG(__FUNCTION__);

	if (theAsiErrorCode == ASI_ERROR_CAMERA_CLOSED)
	{
		CONSOLE_DEBUG("Camera was closed!!!!!!!!!!!!!!!!!!!!!!!!!");
		cCameraIsOpen	=	false;
	}
	else if (theAsiErrorCode == ASI_ERROR_CAMERA_REMOVED)
	{
		CONSOLE_DEBUG("Camera removed error - resetting");
//++		Init_ASICamera();
	}
}


//*****************************************************************************
bool	CameraDriverASI::IsCameraIDvalid(const int argCameraID)
{
bool	isValid;

	if ((argCameraID >= 0) && (argCameraID < gASIcameraCount))
	{
		isValid	=	true;
	}
	else
	{
		isValid	=	false;
	}
	return(isValid);

}

//*****************************************************************************
void	CameraDriverASI::ResetCamera(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CloseASICamera(cCameraID);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Start_CameraExposure(int32_t exposureMicrosecs)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
ASI_EXPOSURE_STATUS	exposureStatatus;
int					busyCnt;
bool				cameraIsBusy;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_INT32("currentTime\t=",		millis());

	if (exposureMicrosecs < 0)
	{
		CONSOLE_DEBUG_W_INT32("INVALID EXPOSURE TIME\t=", exposureMicrosecs);

	}

//-?	cNumFramesSaved	=	0;
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
//		CONSOLE_DEBUG_W_INT32("exposureMicrosecs\t=",	exposureMicrosecs);

		//*	lets check to see if the camera is busy
		cameraIsBusy	=	true;	//*	assume that it is busy
		busyCnt			=	0;
		while (cameraIsBusy && (busyCnt < 30))
		{
			asiErrorCode	=	ASIGetExpStatus(cCameraID, &exposureStatatus);
			if (asiErrorCode == ASI_SUCCESS)
			{
//				CONSOLE_DEBUG_W_NUM("ASISetControlValue->exposureStatatus\t=",	exposureStatatus);
				switch(exposureStatatus)
				{
					case ASI_EXP_WORKING:
						cameraIsBusy	=	true;
						usleep(1000);
						break;

					default:
						cameraIsBusy	=	false;
						break;
				}
			}
			else
			{
				usleep(1000);
			}
			busyCnt++;
		}
//		CONSOLE_DEBUG_W_NUM("busyCnt\t=",	busyCnt);

		if (cameraIsBusy)
		{
			CONSOLE_DEBUG("Calling ASIStopExposure()");
			asiErrorCode	=	ASIStopExposure(cCameraID);
			CONSOLE_DEBUG_W_NUM("ASIStopExposure->asiErrorCode\t=",	asiErrorCode);
		}

		cCurrentExposure_us	=	exposureMicrosecs;
		if (gVerbose)
		{
//			CONSOLE_DEBUG_W_DBL("Current Exposure (secs)\t=",	(cCurrentExposure_us / 1000000.0));
		}

//		asiErrorCode	=	ASIInitCamera(cCameraID);
//		if (asiErrorCode == ASI_SUCCESS)
		{
//			CONSOLE_DEBUG_W_INT32("currentTime\t=",		millis());
		//	asiErrorCode	=	ASISetCameraMode(cCameraID, ASI_MODE_NORMAL);
		//	CONSOLE_DEBUG_W_NUM("ASISetCameraMode->asiErrorCode\t=",	asiErrorCode);

//			CONSOLE_DEBUG("Calling ASISetControlValue()");
			bAuto			=	ASI_FALSE;
			asiErrorCode	=	ASISetControlValue(	cCameraID,
													ASI_EXPOSURE,
													exposureMicrosecs,
													bAuto);
			if (asiErrorCode == ASI_SUCCESS)
			{
				//*	make sure the camera is in idle state
				exposureStatatus	=	(ASI_EXPOSURE_STATUS)-1;
				asiErrorCode		=	ASIGetExpStatus(cCameraID, &exposureStatatus);
				if (asiErrorCode == ASI_SUCCESS)
				{
//					CONSOLE_DEBUG_W_NUM("cCameraID\t=",		cCameraID);
					if (exposureStatatus == ASI_EXP_WORKING)
					{
						CONSOLE_DEBUG("Camera is busy!!!!!!!!!!!!!!");
						CONSOLE_DEBUG_W_NUM("ASISetControlValue->exposureStatatus\t=",	exposureStatatus);
						alpacaErrCode	=	kASCOM_Err_CameraBusy;
						strcpy(cLastCameraErrMsg, "Camera is busy");
					}
					else
					{
//						CONSOLE_DEBUG_W_NUM("exposureStatatus\t=",	exposureStatatus);
						SetLastExposureInfo();
//-						cCameraProp.Lastexposure_duration_us	=	exposureMicrosecs;
//-						gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);
						asiErrorCode	=	ASIStartExposure(cCameraID, ASI_FALSE);
						if (asiErrorCode == ASI_SUCCESS)
						{
							cInternalCameraState	=	kCameraState_TakingPicture;
							alpacaErrCode			=	kASCOM_Err_Success;

						}
						else
						{
							CONSOLE_DEBUG_W_NUM("Camera is not idle, status\t=",	asiErrorCode);
							alpacaErrCode	=	kASCOM_Err_FailedUnknown;
							strcpy(cLastCameraErrMsg, "Failed to start exposure");
							CONSOLE_DEBUG_W_STR("Error Msg\t=",	cLastCameraErrMsg);
						}
					}
				}
				else
				{
					CheckForClosedError(asiErrorCode);
					CONSOLE_DEBUG_W_NUM("ASIGetExpStatus->asiErrorCode\t=",	asiErrorCode);
					alpacaErrCode	=	kASCOM_Err_FailedUnknown;
					strcpy(cLastCameraErrMsg, "Failed to get status");
				}
			}
			else
			{
				CheckForClosedError(asiErrorCode);
				CONSOLE_DEBUG_W_NUM("ASISetControlValue->asiErrorCode\t=",	asiErrorCode);
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
				strcpy(cLastCameraErrMsg, "Failed to set exposure");
			}
		}
//		else
//		{
//			CheckForClosedError(asiErrorCode);
//			CONSOLE_DEBUG_W_NUM("ASIInitCamera->asiErrorCode\t=",	asiErrorCode);
//		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "Failed to open cameraa");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
	}


//	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=",	alpacaErrCode);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	returns 0 if success, else alpaca error code
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
ASI_ERROR_CODE		asiErrorCode;

	CONSOLE_DEBUG(__FUNCTION__);
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		asiErrorCode	=	ASIStopExposure(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			sprintf(cLastCameraErrMsg, "Failed to stop exposure. asiErrorCode=%d", asiErrorCode);
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		strcpy(cLastCameraErrMsg, "Failed to open cameraa");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverASI::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	exposureState;
ASI_ERROR_CODE			asiErrorCode;
ASI_EXPOSURE_STATUS		exposureStatatus;

//	CONSOLE_DEBUG(__FUNCTION__);
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		asiErrorCode	=	ASIGetExpStatus(cCameraID, &exposureStatatus);
		if (asiErrorCode == ASI_SUCCESS)
		{
			switch (exposureStatatus )
			{
				case ASI_EXP_WORKING:		//*	exposing
					exposureState	=	kExposure_Working;
					break;

				case ASI_EXP_IDLE:			//*	idle states, you can start exposure now
					exposureState	=	kExposure_Idle;
					break;

				case ASI_EXP_SUCCESS:		//*	exposure finished and waiting for download
					gettimeofday(&cCameraProp.Lastexposure_EndTime, NULL);
					exposureState	=	kExposure_Success;
					break;

				case ASI_EXP_FAILED:		//*	exposure failed, you need to start exposure again
					exposureState	=	kExposure_Failed;
					CONSOLE_DEBUG_W_NUM("Exposure failed:ASI exposureStatatus\t=",		exposureStatatus);
					break;

				default:
					exposureState	=	kExposure_Unknown;
					break;

			}
			if (verboseFlag)
			{
				if (asiErrorCode != 0)
				{
					CONSOLE_DEBUG_W_NUM("ASIGetExpStatus:asiErrorCode\t=",			asiErrorCode);
				}
			//	CONSOLE_DEBUG_W_NUM("ASIGetExpStatus:exposureStatatus\t=",		exposureStatatus);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ASIGetExpStatus:asiErrorCode\t=",		asiErrorCode);
			exposureState	=	kExposure_Failed;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Camera failed to open:asiErrorCode\t=",		asiErrorCode);
		exposureState	=	kExposure_Failed;
	}
	return(exposureState);
}



//#ifdef _TEMP_DISABLE_
#if 1



#pragma mark -
#pragma mark Video commands

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Start_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_ERROR_CODE		asiErrorCode;
ASI_BOOL			bAuto		=	ASI_FALSE;

	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		asiErrorCode	=	ASISetControlValue(	cCameraID,
												ASI_EXPOSURE,
												cCurrentExposure_us,
												bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			cCameraProp.Lastexposure_duration_us	=	cCurrentExposure_us;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to set exposure time");
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}

		asiErrorCode	=	ASIStartVideoCapture(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;

			gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);


		#ifdef _USE_OPENCV_
			CreateOpenCVImage(NULL);
		#endif
			cInternalCameraState	=	kCameraState_TakingVideo;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			strcpy(cLastCameraErrMsg, "Failed to start video capture");
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to open ASI camera");
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Stop_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

	CONSOLE_DEBUG(__FUNCTION__);
	if ((cCameraID >= 0) && (cCameraID < kMaxCameraCnt))
	{

		switch(cInternalCameraState)
		{
			case kCameraState_Idle:
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
				strcpy(cLastCameraErrMsg, "Camera not taking video");
				break;

			case kCameraState_TakingPicture:
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
				strcpy(cLastCameraErrMsg, "Single frame exposure in progress");
				break;

			case kCameraState_StartVideo:
			case kCameraState_TakingVideo:
				asiErrorCode		=	ASIStopVideoCapture(cCameraID);
				if (asiErrorCode != 0)
				{
					strcpy(cLastCameraErrMsg, "ASIStopVideoCapture returned error: ");
					Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
					strcat(cLastCameraErrMsg, asiErrorMsgString);
				}
				cInternalCameraState	=	kCameraState_Idle;
				break;

			default:
				break;
		}
	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Take_Video(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				timeToStop;

//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _USE_OPENCV_
ASI_ERROR_CODE	asiErrorCode;
int				deltaSecs;

	deltaSecs	=	0;
	if (cOpenCV_Image != NULL)
	{
	int		videoWriteRC;

		memset(cOpenCV_Image->imageData, 0, cOpenCV_Image->imageSize);

		asiErrorCode	=	ASIGetVideoData(cCameraID,
											(unsigned char*)cOpenCV_Image->imageData,
											cOpenCV_Image->imageSize,
											-1);
		if (asiErrorCode == ASI_SUCCESS)
		{
			cNumVideoFramesSaved++;
//#define _DEBUG_VIDEO_
		#ifdef _DEBUG_VIDEO_
			char	imageFilePath[64];
			int		openCVerr;
			int		quality[3] = {16, 200, 0};

			sprintf(imageFilePath, "%s/DEBUG_IMG%03d.jpg", kImageDataDir, cNumVideoFramesSaved);
			CONSOLE_DEBUG(imageFilePath);
			openCVerr	=	cvSaveImage(imageFilePath, cOpenCV_Image, quality);

			CONSOLE_DEBUG_W_NUM("openCVerr\t=", openCVerr);
		#endif // _DEBUG_VIDEO_

			//*	calculate frames per sec
			gettimeofday(&cCameraProp.Lastexposure_EndTime, NULL);
			deltaSecs	=	cCameraProp.Lastexposure_EndTime.tv_sec - cCameraProp.Lastexposure_StartTime.tv_sec;
			if (deltaSecs > 0)
			{
				cFrameRate	=	(cNumVideoFramesSaved * 1.0) / deltaSecs;
			}
			if (cOpenCV_videoWriter != NULL)
			{
				//*	if we want a time stamp
				//*	change to preference/setting latter
				//*	Jun 29,	2120	<TODO> Change video timestamp to an option
				if (1)
				{
				CvPoint		point1;
				CvRect		myCVrect;
				char		timeStampString[256];
				char		testDataString[256];

					#define	kTextBoxHeight	35
					//*	first erase the text area
					myCVrect.x		=	0;
					myCVrect.y		=	cOpenCV_Image->height - kTextBoxHeight;
					myCVrect.width	=	cOpenCV_Image->width;
					myCVrect.height	=	kTextBoxHeight;
					cvRectangleR(	cOpenCV_Image,
									myCVrect,
									cSideBarBlk,				//	CvScalar color,
									CV_FILLED,					//	int thickness CV_DEFAULT(1),
									8,							//	int line_type CV_DEFAULT(8),
									0);							//	int shift CV_DEFAULT(0));

					FormatTimeStringISO8601(&cCameraProp.Lastexposure_EndTime, timeStampString);
					point1.x	=	5;
					point1.y	=	cOpenCV_Image->height - 10;
					cvPutText(	cOpenCV_Image,	timeStampString,	point1,	&cOverlayTextFont,	cVideoOverlayColor);

					point1.x	=	cOpenCV_Image->width / 2;
					sprintf(testDataString, "S-%s,%s", cObjectName, cAuxTextTag);
					cvPutText(	cOpenCV_Image,	testDataString,	point1,	&cOverlayTextFont,	cVideoOverlayColor);

					if (cVideoTimeStampFilePtr != NULL)
					{
					double	lastExposureTimeSecs;

						lastExposureTimeSecs	=	cCameraProp.Lastexposure_EndTime.tv_sec;
						lastExposureTimeSecs	+=	cCameraProp.Lastexposure_EndTime.tv_usec / 1000000.0;

						fprintf(cVideoTimeStampFilePtr, "%d,%s,%1.3f\r\n",	cNumVideoFramesSaved,
																			timeStampString,
																			lastExposureTimeSecs);
					}
				}
				videoWriteRC	=	cvWriteFrame(cOpenCV_videoWriter, cOpenCV_Image);
				if (videoWriteRC != 1)
				{
					CONSOLE_DEBUG_W_NUM("videoWriteRC\t=", videoWriteRC);
				}
				//============================================================
				//*	Aug 11,	2020	<MLS> Added auto exposure to video output
				//*	check to see if we are in auto exposure adjustment
				if (cAutoAdjustExposure)
				{
					if ((cNumVideoFramesSaved % 10) == 0)
					{
						AutoAdjustExposure();
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("cOpenCV_videoWriter is NULL");
//				CONSOLE_ABORT(__FUNCTION__);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ASIGetVideoData() returned asiErrorCode\t=", asiErrorCode);
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image == NULL");
		cInternalCameraState	=	kCameraState_Idle;
		CONSOLE_ABORT(__FUNCTION__);
	}
	if ((cNumVideoFramesSaved % 100) == 0)
	{
		CONSOLE_DEBUG_W_NUM("cNumVideoFramesSaved\t=", cNumVideoFramesSaved);
	}

	timeToStop	=	false;

	//*	do we have a limit on the number of frames
	if (cNumFramesToSave > 0)
	{
		//*	check to see if its time to stop
		if (cNumVideoFramesSaved >= cNumFramesToSave)
		{
			timeToStop	=	true;
		}
	}
	if (deltaSecs >= cVideoDuration_secs)
	{
		timeToStop	=	true;
	}
	if (timeToStop)
	{
		CONSOLE_DEBUG("time to stop taking video");
		asiErrorCode	=	ASIStopVideoCapture(cCameraID);
		CONSOLE_DEBUG_W_NUM("ASI Video capture stopped, asiErrorCode\t=", asiErrorCode);

		gettimeofday(&cCameraProp.Lastexposure_EndTime, NULL);


		//*	time to stop taking video
		cvReleaseVideoWriter(&cOpenCV_videoWriter);
//			CONSOLE_DEBUG_W_HEX("cOpenCV_videoWriter\t=", (unsigned long)cOpenCV_videoWriter);

		cOpenCV_videoWriter	=	NULL;
		CONSOLE_DEBUG("cOpenCV_videoWriter released");
	#ifdef _ENABLE_FITS_
		SaveImageAsFITS(SAVE_AVI);
	#endif // _ENABLE_FITS_

		if (cVideoTimeStampFilePtr != NULL)
		{
			fclose(cVideoTimeStampFilePtr);
			cVideoTimeStampFilePtr	=	NULL;
		}
		cInternalCameraState	=	kCameraState_Idle;

		WriteFireCaptureTextFile();
	}
#endif	//	_USE_OPENCV_
	return(alpacaErrCode);
}

#endif //	_TEMP_DISABLE_

#pragma mark -



#pragma mark -
//*****************************************************************************
void	CameraDriverASI::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
int					ii;
ASI_ERROR_CODE		asiErrorCode;
int					numberOfCtrls;
ASI_CONTROL_CAPS	controlCaps;
char				lineBuffer[256];
char				asiImageTypeString[16];
int					currentROIwidth;
int					currentROIheight;
int					currentROIbin;
ASI_IMG_TYPE		currentASI_ROIimageType;
int					mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");


//	CONSOLE_DEBUG_W_NUM("Calling ASIGetCameraProperty for cCameraID", cCameraID);
//	asiErrorCode	=	ASIGetCameraProperty(&cAsiCameraInfo, cCameraID);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD></TD><TD>BayerPattern =%d =>",	cAsiCameraInfo.BayerPattern);
	switch(cAsiCameraInfo.BayerPattern)
	{
		case ASI_BAYER_RG:	strcat(lineBuffer, "RG");	break;
		case ASI_BAYER_BG:	strcat(lineBuffer, "BG");	break;
		case ASI_BAYER_GR:	strcat(lineBuffer, "GR");	break;
		case ASI_BAYER_GB:	strcat(lineBuffer, "GB");	break;
	}
	strcat(lineBuffer,	"</TD></TR>\r\n");

	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	//*	supported image formats
	lineBuffer[0]	=	0;
	strcat(lineBuffer, "<TR><TD></TD><TD>Image formats = ");
	ii	=	0;
	while ((ii<8) && (cAsiCameraInfo.SupportedVideoFormat[ii] != ASI_IMG_END))
	{
		Get_ASI_ImageTypeString(cAsiCameraInfo.SupportedVideoFormat[ii], asiImageTypeString);
		strcat(lineBuffer, asiImageTypeString);
		strcat(lineBuffer, ", ");

		ii++;
	}
	strcat(lineBuffer,	"</TD></TR>\r\n");

	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Supported Bins</TD><TD>");
	ii	=	0;
	while ((cAsiCameraInfo.SupportedBins[ii] > 0) && (ii<16))
	{
	char	numBuff[16];

		sprintf(numBuff, "%d,",	cAsiCameraInfo.SupportedBins[ii]);
		strcat(lineBuffer, numBuff);
		ii++;
	}
	strcat(lineBuffer, "</TD></TR>\r\n");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		asiErrorCode	=	ASIGetROIFormat(cCameraID,
											&currentROIwidth,
											&currentROIheight,
											&currentROIbin,
											&currentASI_ROIimageType);
//		CONSOLE_DEBUG_W_NUM("currentROIwidth\t=",		currentROIwidth);
//		CONSOLE_DEBUG_W_NUM("currentROIheight\t=",		currentROIheight);
//		CONSOLE_DEBUG_W_NUM("currentROIbin\t=",			currentROIbin);
//		CONSOLE_DEBUG_W_NUM("currentASI_ROIimageType\t=",	currentASI_ROIimageType);

		sprintf(lineBuffer,	"\t<TR><TD></TD><TD>Current Image type =%d =>", currentASI_ROIimageType);
		switch(currentASI_ROIimageType)
		{
			case ASI_IMG_RAW8:
				strcat(lineBuffer, "RAW8");
				break;

			case ASI_IMG_RGB24:
				strcat(lineBuffer, "RGB24");
				break;

			case ASI_IMG_RAW16:
				strcat(lineBuffer, "RAW16");
				break;

			case ASI_IMG_Y8:
				strcat(lineBuffer, "Y8");
				break;

			default:
				strcat(lineBuffer, "???");
				break;
		}
		strcat(lineBuffer,	"</TD></TR>\r\n");

		SocketWriteData(mySocketFD,	lineBuffer);



		//*-----------------------------------------------------------
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Name</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Description</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Min</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Max</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Default</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>AutoSupported</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Writable</TD>\r\n");
			SocketWriteData(mySocketFD,	"\t<TD><CENTER>Type</TD>\r\n");
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		numberOfCtrls	=	0;
		asiErrorCode	=	ASIGetNumOfControls(cCameraID, &numberOfCtrls);
		if (asiErrorCode == ASI_SUCCESS)
		{
			for (ii=0; ii< numberOfCtrls; ii++)
			{
				memset(&controlCaps, 0, sizeof(ASI_CONTROL_CAPS));

				asiErrorCode	=	ASIGetControlCaps(cCameraID, ii, &controlCaps);
				if (asiErrorCode == ASI_SUCCESS)
				{
					SocketWriteData(mySocketFD,	"<TR>\r\n");
						SocketWriteData(mySocketFD,	"\t<TD>");
						SocketWriteData(mySocketFD,	controlCaps.Name);
						SocketWriteData(mySocketFD,	"</TD>\r\n");

						SocketWriteData(mySocketFD,	"\t<TD>");
						SocketWriteData(mySocketFD,	controlCaps.Description);
						SocketWriteData(mySocketFD,	"</TD>\r\n");

						sprintf(lineBuffer,	"\t<TD><CENTER>%ld</TD>\r\n",	controlCaps.MinValue);
						SocketWriteData(mySocketFD,	lineBuffer);

						sprintf(lineBuffer,	"\t<TD><CENTER>%ld</TD>\r\n",	controlCaps.MaxValue);
						SocketWriteData(mySocketFD,	lineBuffer);

						sprintf(lineBuffer,	"\t<TD><CENTER>%ld</TD>\r\n",	controlCaps.DefaultValue);
						SocketWriteData(mySocketFD,	lineBuffer);

						sprintf(lineBuffer,	"\t<TD><CENTER>%s</TD>\r\n",	(controlCaps.IsAutoSupported == 1) ? "Yes" : "No");
						SocketWriteData(mySocketFD,	lineBuffer);

						sprintf(lineBuffer,	"\t<TD><CENTER>%s</TD>\r\n",	(controlCaps.IsWritable == 1) ? "Yes" : "No");
						SocketWriteData(mySocketFD,	lineBuffer);

						sprintf(lineBuffer,	"\t<TD><CENTER>%d</TD>\r\n",	controlCaps.ControlType);
						SocketWriteData(mySocketFD,	lineBuffer);

					SocketWriteData(mySocketFD,	"</TR>\r\n");
				}
				else
				{
					SocketWriteData(mySocketFD,	"<TR>\r\n");
						sprintf(lineBuffer,	"\t<TD>Failed to read control caps, asiErr=%d</TD>\r\n",	asiErrorCode);
						SocketWriteData(mySocketFD,	lineBuffer);
					SocketWriteData(mySocketFD,	"</TR>\r\n");

				}
			}
			//*-----------------------------------------------------------
			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD COLSPAN=2></TD>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD COLSPAN=5>Total controls</TD>\r\n");

				sprintf(lineBuffer,	"\t<TD><CENTER>%d</TD>\r\n",	numberOfCtrls);
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");
		}
		else
		{
			//*-----------------------------------------------------------
			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD COLSPAN=6>Error from ASIGetNumOfControls(), ");

				Get_ASI_ErrorMsg(asiErrorCode, lineBuffer);
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"\t</TD>\r\n");

			SocketWriteData(mySocketFD,	"</TR>\r\n");
			CheckForClosedError(asiErrorCode);
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
	}
	else
	{
		//*	failed to open camera
		CONSOLE_DEBUG("Failed to open camera");

		SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=8><CENTER>\r\n");
		SocketWriteData(mySocketFD,	"Failed to open camera\r\n");
		SocketWriteData(mySocketFD,	"</TD></TR>\r\n");
	}

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");

	//*	now generate links to all of the commands
	GenerateHTMLcmdLinkTable(mySocketFD, "camera", cCameraID, gCameraCmdTable);
}


#pragma mark -

#ifdef _USE_THREADS_FOR_ASI_CAMERA_


//*****************************************************************************
//*	this is where we set up all of the OpenCV stuff to save the video file
static void	CameraThread_StartVideo(TYPE_CameraDef *theCamera)
{
ASI_ERROR_CODE	asiErrorCode;
ASI_BOOL		bAuto		=	ASI_FALSE;
int				currentROIwidth;
int				currentROIheight;
int				currentROIbin;
ASI_IMG_TYPE	currentASI_ROIimageType;

	CONSOLE_DEBUG(__FUNCTION__);
	if (IsValidCamera(theCamera))
	{
		theCamera->numFramesToTake	=	100;
		theCamera->numFamesTaken	=	0;

		//*	we have to get the image size and mode
		asiErrorCode	=	ASIGetROIFormat(theCamera->cCameraID,
											&currentROIwidth,
											&currentROIheight,
											&currentROIbin,
											&currentASI_ROIimageType);


		currentASI_ROIimageType	=	ASI_IMG_RGB24;
		asiErrorCode	=	ASISetROIFormat(theCamera->cCameraID,
											currentROIwidth,
											currentROIheight,
											currentROIbin,
											currentASI_ROIimageType);


		asiErrorCode	=	ASIGetROIFormat(theCamera->cCameraID,
											&currentROIwidth,
											&currentROIheight,
											&currentROIbin,
											&currentASI_ROIimageType);

		CONSOLE_DEBUG_W_NUM("ASIGetROIFormat() asiErrorCode\t=", asiErrorCode);
		CONSOLE_DEBUG_W_NUM("currentROIwidth \t=",		currentROIwidth);
		CONSOLE_DEBUG_W_NUM("currentROIheight\t=",		currentROIheight);
		CONSOLE_DEBUG_W_NUM("currentROIbin   \t=",			currentROIbin);
		CONSOLE_DEBUG_W_NUM("currentASI_ROIimageType\t=",	currentASI_ROIimageType);

#ifdef _USE_OPENCV_
		switch(currentASI_ROIimageType)
		{
			case ASI_IMG_RAW8:
				theCamera->openCV_Image	=	cvCreateImage(cvSize(currentROIwidth, currentROIwidth), IPL_DEPTH_8U, 1);
				break;

			case ASI_IMG_RGB24:
				theCamera->openCV_Image	=	cvCreateImage(cvSize(currentROIwidth, currentROIwidth), IPL_DEPTH_8U, 3);
				break;

			case ASI_IMG_RAW16:
				theCamera->openCV_Image	=	cvCreateImage(cvSize(currentROIwidth, currentROIwidth), IPL_DEPTH_16U, 1);
				break;

			case ASI_IMG_Y8:
				break;

		}
		if (theCamera->openCV_Image != NULL)
		{
			CONSOLE_DEBUG("OpenCV image created succesfully");
			CONSOLE_DEBUG_W_NUM("currentROIwidth \t=",		currentROIwidth);
			CONSOLE_DEBUG_W_NUM("currentROIheight\t=",		currentROIheight);
			//*	create the opencv video writer
			theCamera->openCV_videoWriter	=	cvCreateVideoWriter(	"testing.avi",
																	//	CV_FOURCC_DEFAULT,
																		CV_FOURCC('M', 'J', 'L', 'S'),
																		30.0,
																		cvSize(currentROIwidth, currentROIwidth),
																		1);
			if (theCamera->openCV_videoWriter != NULL)
			{
				CONSOLE_DEBUG("OpenCV video writer created succesfully");
			}
			else
			{
				CONSOLE_DEBUG("OpenCV video writer FAILED!!!!!!!!!!!");
				theCamera->cInternalCameraState	=	kCameraState_Idle;
			}
		}
		else
		{
			CONSOLE_DEBUG("theCamera->openCV_Image == NULL");
			theCamera->cInternalCameraState	=	kCameraState_Idle;
		}
#endif	//	_USE_OPENCV_
	}
}

//*****************************************************************************
static void	CameraThread_GetVideo(TYPE_CameraDef *theCamera)
{
ASI_ERROR_CODE	asiErrorCode;
ASI_BOOL		bAuto		=	ASI_FALSE;
long			autoGain	=	0;
long			autoExp		=	0;
long			ltemp		=	0;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (IsValidCamera(theCamera))
	{
#ifdef _USE_OPENCV_
	IplImage		*myOpenCVimgPtr;

		myOpenCVimgPtr	=	theCamera->openCV_Image;
		if (myOpenCVimgPtr != NULL)
		{
		int		videoWriteRC;

			memset(myOpenCVimgPtr->imageData, 0, myOpenCVimgPtr->imageSize);

			asiErrorCode	=	ASIGetVideoData(theCamera->cCameraID,
												(unsigned char*)myOpenCVimgPtr->imageData,
												myOpenCVimgPtr->imageSize,
												-1);
		//	cvShowImage(theCamera->cameraName, myOpenCVimgPtr);
			cvShowImage("camera", myOpenCVimgPtr);
			cvWaitKey(1);
			if (asiErrorCode == ASI_SUCCESS)
			{
				// Read current camera parameters
			//	ASIGetControlValue(theCamera->cCameraID,	ASI_EXPOSURE,		&autoExp,	&bAuto);
			//	ASIGetControlValue(theCamera->cCameraID,	ASI_GAIN,			&autoGain,	&bAuto);
			//	ASIGetControlValue(theCamera->cCameraID,	ASI_TEMPERATURE,	&ltemp,		&bAuto);

				if (theCamera->openCV_videoWriter != NULL)
				{
					videoWriteRC	=	cvWriteFrame(theCamera->openCV_videoWriter, myOpenCVimgPtr);
				//	CONSOLE_DEBUG_W_NUM("videoWriteRC\t=", videoWriteRC);
				}
				else
				{
					CONSOLE_DEBUG("theCamera->openCV_videoWriter is NULL");
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ASIGetVideoData() returned asiErrorCode\t=", asiErrorCode);
			}
		}
		else
		{
			CONSOLE_DEBUG("myOpenCVimgPtr == NULL");
			theCamera->cInternalCameraState	=	kCameraState_Idle;
		}

		theCamera->numFamesTaken++;
		CONSOLE_DEBUG_W_NUM("numFamesTaken\t=", theCamera->numFamesTaken);

		//*	do we have a limit on the number of frames
		if (theCamera->numFramesToTake > 0)
		{
			//*	check to see if its time to stop
			if (theCamera->numFamesTaken >= theCamera->numFramesToTake)
			{
				CONSOLE_DEBUG("time to stop taking video");
				asiErrorCode	=	ASIStopVideoCapture(theCamera->cCameraID);
				CONSOLE_DEBUG_W_NUM("ASI Video capture stopped, asiErrorCode\t=", asiErrorCode);

				//*	time to stop taking video
				cvReleaseVideoWriter(&theCamera->openCV_videoWriter);
				CONSOLE_DEBUG_W_HEX("openCV_videoWriter\t=", theCamera->openCV_videoWriter);

				theCamera->openCV_videoWriter	=	NULL;
				CONSOLE_DEBUG("openCV_videoWriter released");

				cvReleaseImage(&theCamera->openCV_Image);
				theCamera->openCV_Image	=	NULL;
				CONSOLE_DEBUG("openCV_Image released");

				theCamera->cInternalCameraState	=	kCameraState_Idle;
			}
		}
#endif	//	_USE_OPENCV_
	}
}

//*****************************************************************************
static void	*CameraThread(void *arg)
{
TYPE_CameraDef	*theCamera;

	CONSOLE_DEBUG(__FUNCTION__);

	theCamera	=	(TYPE_CameraDef *)arg;

	if (IsValidCamera(theCamera))
	{
		while (theCamera->keepRunning)
		{
			switch(theCamera->cInternalCameraState)
			{
				case kCameraState_Idle:
					sleep(1);
					break;

				case kCameraState_TakingPicture:
					break;


				case kCameraState_StartVideo:
					CameraThread_StartVideo(theCamera);
					theCamera->cInternalCameraState++;	//*	bump to the next state
					break;

				case kCameraState_TakingVideo:
					CameraThread_GetVideo(theCamera);
					break;

				default:
					//*	we should never get here
					break;
			}
		}
	}
	CONSOLE_DEBUG("CameraThread -- exit --");
	return(NULL);
}
#endif // _USE_THREADS_FOR_ASI_CAMERA_




#pragma mark -
#pragma mark Virtual functions
//*****************************************************************************
//*	the camera must already be open when this is called
//*	returns true if success
//*****************************************************************************
bool	CameraDriverASI::GetImage_ROI_info(void)
{
ASI_ERROR_CODE	asiErrorCode;
bool			returnFlag;

	returnFlag	=	false;

	memset(&cROIinfo, 0, sizeof(TYPE_IMAGE_ROI_Info));
	//*	get current stats of the camera
	asiErrorCode	=	ASIGetROIFormat(cCameraID,
										&cROIinfo.currentROIwidth,
										&cROIinfo.currentROIheight,
										&cROIinfo.currentROIbin,
										&cCurrentASIimageType);
	if (asiErrorCode == ASI_SUCCESS)
	{
//		CONSOLE_DEBUG_W_NUM("cROIinfo.currentROIwidth\t=",	cROIinfo.currentROIwidth);
//		CONSOLE_DEBUG_W_NUM("cROIinfo.currentROIheight\t=",	cROIinfo.currentROIheight);
//		CONSOLE_DEBUG_W_NUM("cROIinfo.currentROIbin\t=",	cROIinfo.currentROIbin);
		returnFlag	=	true;
		switch(cCurrentASIimageType)
		{
			case ASI_IMG_RAW8:	cROIinfo.currentROIimageType	=	kImageType_RAW8;	break;
			case ASI_IMG_RAW16:	cROIinfo.currentROIimageType	=	kImageType_RAW16;	break;
			case ASI_IMG_RGB24:	cROIinfo.currentROIimageType	=	kImageType_RGB24;	break;
			case ASI_IMG_Y8:	cROIinfo.currentROIimageType	=	kImageType_Y8;		break;
			default:			cROIinfo.currentROIimageType	=	kImageType_RAW8;	break;
		}

	}
	else
	{
		returnFlag	=	false;
		CONSOLE_DEBUG_W_NUM("ASIGetROIFormat->asiErrorCode\t=",	asiErrorCode);
	}

	return(returnFlag);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_FailedUnknown;
ASI_ERROR_CODE		asiErrorCode;
int					currentROIwidth;
int					currentROIheight;
int					currentROIbin;
ASI_IMG_TYPE		currentASIimageType;

	CONSOLE_DEBUG("---------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	ASIGetROIFormat(cCameraID,
										&currentROIwidth,
										&currentROIheight,
										&currentROIbin,
										&currentASIimageType);
	if (asiErrorCode == ASI_SUCCESS)
	{
		switch(newImageType)
		{
			case kImageType_RAW8:	currentASIimageType	=	ASI_IMG_RAW8;	break;
			case kImageType_RAW16:	currentASIimageType	=	ASI_IMG_RAW16;	break;
			case kImageType_RGB24:	currentASIimageType	=	ASI_IMG_RGB24;	break;
			case kImageType_Y8:		currentASIimageType	=	ASI_IMG_Y8;		break;
			default:				currentASIimageType	=	ASI_IMG_RAW8;	break;
		}

		asiErrorCode	=	ASISetROIFormat(cCameraID,
											currentROIwidth,
											currentROIheight,
											currentROIbin,
											currentASIimageType);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ASISetROIFormat returned", asiErrorCode);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("ASIGetROIFormat returned", asiErrorCode);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;

	CONSOLE_DEBUG("==========================================");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("newImageType\t=",			newImageType);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		alpacaErrCode					=	SetImageTypeCameraOpen(newImageType);
		cROIinfo.currentROIimageType	=	newImageType;
		cDesiredImageType				=	newImageType;
		CONSOLE_DEBUG_W_NUM("SUCESSS! - cDesiredImageType\t=",		cDesiredImageType);
	}
	else
	{
		CONSOLE_DEBUG("Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
long				myGainValue;
ASI_BOOL			bAuto;

//	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		myGainValue		=	newGainValue;
		asiErrorCode	=	ASISetControlValue(	cCameraID,
												ASI_GAIN,
												myGainValue,
												bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to set gain");
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
TYPE_ASCOM_STATUS	CameraDriverASI::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
long				myGainValue;
ASI_BOOL			bAuto;

//	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		asiErrorCode	=	ASIGetControlValue(	cCameraID,
												ASI_GAIN,
												&myGainValue,
												&bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode		=	kASCOM_Err_Success;
			*cameraGainValue	=	myGainValue;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to read gain");
			CONSOLE_DEBUG(cLastCameraErrMsg);
			alpacaErrCode	=	kASCOM_Err_DataFailure;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Write_Offset(const int newOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
long				myOffsetValue;
ASI_BOOL			bAuto;

//	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		myOffsetValue	=	newOffsetValue;
		asiErrorCode	=	ASISetControlValue(	cCameraID,
												ASI_OFFSET,
												myOffsetValue,
												bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to set offset");
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
TYPE_ASCOM_STATUS	CameraDriverASI::Read_Offset(int *cameraOffsetValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
long				myOffsetValue;
ASI_BOOL			bAuto;

//	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		asiErrorCode	=	ASIGetControlValue(	cCameraID,
												ASI_OFFSET,
												&myOffsetValue,
												&bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode		=	kASCOM_Err_Success;
			*cameraOffsetValue	=	myOffsetValue;
		}
		else
		{
			strcpy(cLastCameraErrMsg, "Failed to read offset");
			CONSOLE_DEBUG(cLastCameraErrMsg);
			alpacaErrCode	=	kASCOM_Err_DataFailure;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto			=	ASI_FALSE;
		cCoolerState	=	1;
		asiErrorCode	=	ASISetControlValue(cCameraID, ASI_COOLER_ON, cCoolerState, bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{

		}
		else if (asiErrorCode == ASI_ERROR_INVALID_CONTROL_TYPE)
		{
			strcpy(cLastCameraErrMsg, "Coolor not supported on this camera");
		}
		else
		{
			CheckForClosedError(asiErrorCode);
			CONSOLE_DEBUG_W_NUM("ASISetControlValue->asiErrorCode\t=",	asiErrorCode);
			Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
			strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue:, asiErr=");
			strcat(cLastCameraErrMsg, asiErrorMsgString);
		}
		CONSOLE_DEBUG_W_LONG("cCoolerState\t=",		cCoolerState);

	}
	else
	{
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

	cCoolerState	=	0;
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto				=	ASI_FALSE;
		asiErrorCode		=	ASISetControlValue(cCameraID, ASI_COOLER_ON, cCoolerState, bAuto);
		switch (asiErrorCode)
		{
			case ASI_SUCCESS:
				alpacaErrCode	=	kASCOM_Err_Success;
				break;

			case ASI_ERROR_INVALID_CONTROL_TYPE:
				strcpy(cLastCameraErrMsg, "Coolor not supported on this camera:");
				Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
				strcat(cLastCameraErrMsg, asiErrorMsgString);
				break;

			default:
				CheckForClosedError(asiErrorCode);
				CONSOLE_DEBUG_W_NUM("ASISetControlValue->asiErrorCode\t=",	asiErrorCode);
				Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
				strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue:, asiErr=");
				strcat(cLastCameraErrMsg, asiErrorMsgString);
				alpacaErrCode	=	kASCOM_Err_Unknown;
				break;
		}
		CONSOLE_DEBUG_W_LONG("cCoolerState\t=",		cCoolerState);

	}
	else
	{
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Read_CoolerState(bool *coolerOnOff)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	cCoolerState	=	0;
	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto				=	ASI_FALSE;
		asiErrorCode		=	ASIGetControlValue(cCameraID, ASI_COOLER_ON, &cCoolerState, &bAuto);
		switch(asiErrorCode)
		{
			case ASI_SUCCESS:
				alpacaErrCode	=	kASCOM_Err_Success;
				*coolerOnOff	=	cCoolerState;
//				CONSOLE_DEBUG_W_LONG("cCoolerState\t=",		cCoolerState);
				break;

			case  ASI_ERROR_INVALID_CONTROL_TYPE:
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				strcpy(cLastCameraErrMsg, "Coolor not supported on this camera");
				break;

			default:
				CheckForClosedError(asiErrorCode);
				CONSOLE_DEBUG_W_NUM("ASIGetControlValue->asiErrorCode\t=",	asiErrorCode);
				Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
				strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue:, asiErr=");
				strcat(cLastCameraErrMsg, asiErrorMsgString);
				alpacaErrCode	=	kASCOM_Err_Unknown;
				break;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

	cCoolerPowerLevel	=	0;
	asiErrorCode		=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		bAuto				=	ASI_FALSE;
		asiErrorCode		=	ASIGetControlValue(cCameraID, ASI_COOLER_POWER_PERC, &cCoolerPowerLevel, &bAuto);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
//			CONSOLE_DEBUG_W_LONG("cCoolerPowerLevel\t=",		cCoolerPowerLevel);
		}
		else if (asiErrorCode == ASI_ERROR_INVALID_CONTROL_TYPE)
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			strcpy(cLastCameraErrMsg, "Coolor not supported on this camera");
		}
		else
		{
			CheckForClosedError(asiErrorCode);
			CONSOLE_DEBUG_W_NUM("ASIGetControlValue->asiErrorCode\t=",	asiErrorCode);
			Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
			strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue:, asiErr=");
			strcat(cLastCameraErrMsg, asiErrorMsgString);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Read_Fastreadout(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
ASI_CONTROL_CAPS	controlCaps;
ASI_ERROR_CODE		asiErrorCode;
ASI_BOOL			bAuto;
char				asiErrorMsgString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	memset(&controlCaps, 0, sizeof(ASI_CONTROL_CAPS));
	bAuto			=	ASI_FALSE;
	cHighSpeedMode	=	0;
	asiErrorCode	=	ASIGetControlValue(cCameraID, ASI_HIGH_SPEED_MODE, &cHighSpeedMode, &bAuto);
	if (asiErrorCode == ASI_SUCCESS)
	{
//		CONSOLE_DEBUG_W_LONG("cHighSpeedMode\t=",		cHighSpeedMode);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else if (asiErrorCode == ASI_ERROR_INVALID_CONTROL_TYPE)
	{
		strcpy(cLastCameraErrMsg, "Fast read out not supported on this camera");
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	else
	{
		CheckForClosedError(asiErrorCode);
		CONSOLE_DEBUG_W_NUM("ASIGetControlValue->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	return(alpacaErrCode);
}

//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
long				cameraTemperature;
ASI_BOOL			bAuto;
ASI_ERROR_CODE		asiErrorCode;
char				asiErrorMsgString[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	cLastCameraErrMsg[0]	=	0;
	if (cTempReadSupported)
	{
		asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			cameraTemperature	=	-600;
			bAuto				=	ASI_FALSE;
			asiErrorCode		=	ASIGetControlValue(cCameraID, ASI_TEMPERATURE, &cameraTemperature, &bAuto);
//			CONSOLE_DEBUG_W_NUM("ASIGetControlValue->asiErrorCode\t=",	asiErrorCode);
			switch (asiErrorCode)
			{
				case ASI_SUCCESS:
					cCameraProp.CCDtemperature		=	cameraTemperature / 10.0;
//					CONSOLE_DEBUG_W_LONG("cameraTemperature\t=",	cameraTemperature);
//					CONSOLE_DEBUG_W_DBL("cameraTempDbl\t=",			cCameraProp.CCDtemperature);
					alpacaErrCode	=	kASCOM_Err_Success;
					break;

				case  ASI_ERROR_INVALID_CONTROL_TYPE:
					alpacaErrCode	=	kASCOM_Err_NotImplemented;
					strcpy(cLastCameraErrMsg, "Coolor not supported on this camera");
					CONSOLE_DEBUG(cLastCameraErrMsg);
					break;

				default:
					CONSOLE_DEBUG_W_NUM("ASIGetControlValue->asiErrorCode\t=",	asiErrorCode);
					CheckForClosedError(asiErrorCode);
					Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
					strcpy(cLastCameraErrMsg, "Failed on ASIGetControlValue:, asiErr=");
					strcat(cLastCameraErrMsg, asiErrorMsgString);
					CONSOLE_DEBUG(cLastCameraErrMsg);
					alpacaErrCode	=	kASCOM_Err_Unknown;
					break;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
			Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
			strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
			strcat(cLastCameraErrMsg, asiErrorMsgString);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		strcpy(cLastCameraErrMsg, "Temperature not supported on this camera");
//		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}



//*****************************************************************************
//*	this routine gets called by BOTH Write_BinX() & Write_BinY()
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Write_BinXY(const int newBinXvalue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
ASI_ERROR_CODE		asiErrorCode;
int					currentROIwidth;
int					currentROIheight;
int					currentROIbin;
ASI_IMG_TYPE		currentASI_ROIimageType;
char				asiErrorMsgString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
	if (asiErrorCode == ASI_SUCCESS)
	{
		asiErrorCode	=	ASIGetROIFormat(cCameraID,
											&currentROIwidth,
											&currentROIheight,
											&currentROIbin,
											&currentASI_ROIimageType);
		if (asiErrorCode == ASI_SUCCESS)
		{
			CONSOLE_DEBUG_W_NUM("currentROIbin\t=", currentROIbin);
			CONSOLE_DEBUG_W_NUM("currentROIwidth\t=", currentROIwidth);
			CONSOLE_DEBUG_W_NUM("currentROIheight\t=", currentROIheight);
		}
		else
		{
			CONSOLE_DEBUG("ASIGetROIFormat failed-------------------------");
			CONSOLE_DEBUG_W_NUM("asiErrorCode\t=", asiErrorCode);
		}

		currentROIbin		=	newBinXvalue;
		currentROIwidth		=   cCameraProp.CameraXsize / newBinXvalue;
		currentROIheight	=   cCameraProp.CameraYsize / newBinXvalue;
		asiErrorCode		=	ASISetROIFormat(cCameraID,
											currentROIwidth,
											currentROIheight,
											currentROIbin,
											currentASI_ROIimageType);
		if (asiErrorCode == ASI_SUCCESS)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_DEBUG("ASISetROIFormat failed-------------------------");
			CONSOLE_DEBUG_W_NUM("asiErrorCode\t=", asiErrorCode);
			Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);

			strcpy(cLastCameraErrMsg, "AlpacaPi: ASISetROIFormat failed-");
			strcat(cLastCameraErrMsg, asiErrorMsgString);
			strcat(cLastCameraErrMsg, ":");
			strcat(cLastCameraErrMsg, __FUNCTION__);
		}


		asiErrorCode	=	ASIGetROIFormat(cCameraID,
											&currentROIwidth,
											&currentROIheight,
											&currentROIbin,
											&currentASI_ROIimageType);
		if (asiErrorCode == ASI_SUCCESS)
		{
			CONSOLE_DEBUG_W_NUM("currentROIbin\t=", currentROIbin);
			CONSOLE_DEBUG_W_NUM("currentROIwidth\t=", currentROIwidth);
			CONSOLE_DEBUG_W_NUM("currentROIheight\t=", currentROIheight);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("asiErrorCode\t=", asiErrorCode);
		}
	}
	else
	{
		CheckForClosedError(asiErrorCode);
		CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
		Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
		strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
		strcat(cLastCameraErrMsg, asiErrorMsgString);
		alpacaErrCode	=	kASCOM_Err_FailedUnknown;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Write_BinX(const int newBinXvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	Write_BinXY(newBinXvalue);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Write_BinY(const int newBinYvalue)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	Write_BinXY(newBinYvalue);
	return(alpacaErrCode);
}


#pragma mark -
#pragma mark Image data commands
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
ASI_EXPOSURE_STATUS	exposureStatatus;
long				pixelCount;
long				bufferSize;
char				asiErrorMsgString[64];

	SETUP_TIMING();

//	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_FailedUnknown;
	if ((cCameraID >= 0) && (cCameraID < kMaxCameraCnt))
	{
		asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			exposureStatatus	=	(ASI_EXPOSURE_STATUS)-1;
			asiErrorCode		=	ASIGetExpStatus(cCameraID, &exposureStatatus);
//			CONSOLE_DEBUG_W_NUM("exposureStatatus\t=",	exposureStatatus);
			if ((asiErrorCode == ASI_SUCCESS) && (exposureStatatus == ASI_EXP_SUCCESS))
			{
//				CONSOLE_DEBUG("We have data!!!!!!!!!!!!!!");
				//*	get the ROI information which has the current image type
				GetImage_ROI_info();


				//*	we have to allocate a buffer big enough to hold the image
				pixelCount	=	cCameraProp.CameraXsize *
								cCameraProp.CameraYsize;
				bufferSize	=	(pixelCount * 3) + 100;

				AllcateImageBuffer(-1);		//*	let it figure out how much

				if (cCameraDataBuffer != NULL)
				{
					memset(cCameraDataBuffer, 0, bufferSize);

					START_TIMING();
					asiErrorCode	=	ASIGetDataAfterExp(cCameraID, cCameraDataBuffer, bufferSize);
					if (asiErrorCode == ASI_SUCCESS)
					{
						DEBUG_TIMING("Time to read image (milliseconds)\t=");
						alpacaErrCode	=	kASCOM_Err_Success;
					}
					else
					{
						Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
						strcpy(cLastCameraErrMsg, "Failed to read image, asiErr=");
						strcat(cLastCameraErrMsg, asiErrorMsgString);

						CONSOLE_DEBUG_W_NUM("asiErrorCode\t=",		asiErrorCode);
						CONSOLE_DEBUG(cLastCameraErrMsg);
					}
				}
				else
				{
					strcpy(cLastCameraErrMsg, "Failed to allocate buffer");
					CONSOLE_DEBUG(cLastCameraErrMsg);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("asiErrorCode\t=",		asiErrorCode);
				CheckForClosedError(asiErrorCode);
				strcpy(cLastCameraErrMsg, "No image to get.");
				CONSOLE_DEBUG(cLastCameraErrMsg);

				if (asiErrorCode != ASI_SUCCESS)
				{
					Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
					strcat(cLastCameraErrMsg, " asiErr=");
					strcat(cLastCameraErrMsg, asiErrorMsgString);
				}
			}
		}
		else
		{
			CheckForClosedError(asiErrorCode);
			CONSOLE_DEBUG_W_NUM("ASIOpenCamera->asiErrorCode\t=",	asiErrorCode);
			Get_ASI_ErrorMsg(asiErrorCode, asiErrorMsgString);
			strcpy(cLastCameraErrMsg, "Failed to open ASI camera:, asiErr=");
			strcat(cLastCameraErrMsg, asiErrorMsgString);
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		CONSOLE_DEBUG("Invalid device number");
	}
//	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=",	alpacaErrCode);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverASI::SetFlipMode(int newFlipMode)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ASI_ERROR_CODE		asiErrorCode;
long				myFlipValue;
ASI_BOOL			bAuto;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCanFlipImage)
	{
		asiErrorCode	=	OpenASIcameraIfNeeded(cCameraID);
		if (asiErrorCode == ASI_SUCCESS)
		{
			bAuto			=	ASI_FALSE;
			myFlipValue		=	newFlipMode;
			asiErrorCode	=	ASISetControlValue(	cCameraID,
													ASI_FLIP,
													myFlipValue,
													bAuto);
			if (asiErrorCode == ASI_SUCCESS)
			{
				cFlipMode		=	newFlipMode;
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				strcpy(cLastCameraErrMsg, "Failed to set flip mode");
				CONSOLE_DEBUG(cLastCameraErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			strcpy(cLastCameraErrMsg, "Failed to open connection to camera");
			CONSOLE_DEBUG(cLastCameraErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		strcpy(cLastCameraErrMsg, "Camera does not support image flip");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	return(alpacaErrCode);
}


#pragma mark -

//*****************************************************************************
static TYPE_SensorName	gSensorNameList[]	=
{

	{	"ASI034MC",		"ONS ASX340CS"		},
	{	"ASI071MC",		"Sony IMX071"		},
	{	"ASI094MC",		"Sony IMX094"		},
	{	"ASI120MC",		"ONS AR0130CS"		},
	{	"ASI120MM",		"ONS MT9M034"		},
	{	"ASI120MC-S",	"ONS AR0130CS"		},
	{	"ASI120MM-S",	"ONS MT9M034"		},
	{	"ASI120MM",		"ONS AR0130CS"		},
	{	"ASI128MC",		"Sony IMX128"		},
	{	"ASI174MC",		"Sony IMX174"		},
	{	"ASI174MM",		"Sony IMX174"		},
	{	"ASI178MC",		"Sony IMX178"		},
	{	"ASI178MM",		"Sony IMX178"		},
	{	"ASI183MC",		"Sony IMX183"		},
	{	"ASI183MM",		"Sony IMX183"		},
	{	"ASI185MC",		"Sony IMX185"		},
	{	"ASI224MC",		"Sony IMX224"		},
	{	"ASI290MC",		"Sony IMX290"		},
	{	"ASI290MM",		"Sony IMX290"		},
	{	"ASI294MC",		"Sony IMX294"		},
	{	"ASI385MC",		"Sony IMX385"		},
	{	"ASI1600MC",	"Panasonic MN34230"	},
	{	"ASI1600MM",	"Panasonic MN34230"	},


	{	"",		""	},
};


//*****************************************************************************
static void	Get_ASI_SensorName(const char *argCameraName, char *retSensorName)
{
char	*asiNamePtr;
int		ii;
char	myCameraName[32];

	retSensorName[0]	=	0;
	CONSOLE_DEBUG_W_STR("argCameraName\t=", argCameraName);
	asiNamePtr	=	strstr((char *)argCameraName, "ASI");
	if (asiNamePtr != NULL)
	{
		ii	=	0;
		while (	(asiNamePtr[ii] > 0x20) &&
				(asiNamePtr[ii] != '(') &&
				(ii<(kMaxSensorNameLen - 2)))
		{
			myCameraName[ii]	=	asiNamePtr[ii];
			ii++;
		}
		myCameraName[ii]	=	0;

		ii	=	0;
		while ((gSensorNameList[ii].cameraModel[0] != 0) && (ii < 100))
		{
			if (strcmp(myCameraName, gSensorNameList[ii].cameraModel) == 0)
			{
				strcpy(retSensorName, gSensorNameList[ii].sensorName);
				break;
			}
			ii++;
		}
	}
}


//*****************************************************************************
static void	Get_ASI_ErrorMsg(ASI_ERROR_CODE asiErrorCode, char *asiErrorMessage)
{
	asiErrorMessage[0]	=	0;
	switch(asiErrorCode)
	{
		case ASI_SUCCESS:
			break;

		case ASI_ERROR_INVALID_INDEX:		//no camera connected or index value out of boundary
			strcpy(asiErrorMessage, "ERROR_INVALID_INDEX");
			break;

		case ASI_ERROR_INVALID_ID:				//invalid ID
			strcpy(asiErrorMessage, "ERROR_INVALID_ID");
			break;

		case ASI_ERROR_INVALID_CONTROL_TYPE:	//invalid control type
			strcpy(asiErrorMessage, "ERROR_INVALID_CONTROL_TYPE");
			break;

		case ASI_ERROR_CAMERA_CLOSED:			//camera didn't open
			strcpy(asiErrorMessage, "ERROR_CAMERA_CLOSED");
			break;

		case ASI_ERROR_CAMERA_REMOVED:			//failed to find the camera, maybe the camera has been removed
			strcpy(asiErrorMessage, "ERROR_CAMERA_REMOVED");
			break;

		case ASI_ERROR_INVALID_PATH:			//cannot find the path of the file
			strcpy(asiErrorMessage, "ERROR_INVALID_PATH");
			break;

		case ASI_ERROR_INVALID_FILEFORMAT:
			strcpy(asiErrorMessage, "ERROR_INVALID_FILEFORMAT");
			break;

		case ASI_ERROR_INVALID_SIZE:			//wrong video format size
			strcpy(asiErrorMessage, "ERROR_INVALID_SIZE");
			break;

		case ASI_ERROR_INVALID_IMGTYPE:			//unsupported image formate
			strcpy(asiErrorMessage, "ERROR_INVALID_IMGTYPE");
			break;

		case ASI_ERROR_OUTOF_BOUNDARY:			//the startpos is out of boundary
			strcpy(asiErrorMessage, "ERROR_OUTOF_BOUNDARY");
			break;

		case ASI_ERROR_TIMEOUT:					//timeout
			strcpy(asiErrorMessage, "ERROR_TIMEOUT");
			break;

		case ASI_ERROR_INVALID_SEQUENCE:		//stop capture first
			strcpy(asiErrorMessage, "ERROR_INVALID_SEQUENCE");
			break;

		case ASI_ERROR_BUFFER_TOO_SMALL:		//buffer size is not big enough
			strcpy(asiErrorMessage, "ERROR_BUFFER_TOO_SMALL");
			break;

		case ASI_ERROR_VIDEO_MODE_ACTIVE:
			strcpy(asiErrorMessage, "ERROR_VIDEO_MODE_ACTIVE");
			break;

		case ASI_ERROR_EXPOSURE_IN_PROGRESS:
			strcpy(asiErrorMessage, "ERROR_EXPOSURE_IN_PROGRESS");
			break;

		case ASI_ERROR_GENERAL_ERROR:			//general error, eg: value is out of valid range
			strcpy(asiErrorMessage, "ERROR_GENERAL_ERROR");
			break;

		case ASI_ERROR_INVALID_MODE:			//the current mode is wrong
			strcpy(asiErrorMessage, "ERROR_INVALID_MODE");
			break;

		case ASI_ERROR_END:
			strcpy(asiErrorMessage, "ERROR_END");
			break;

	}
}

//*****************************************************************************
static void	Get_ASI_ImageTypeString(ASI_IMG_TYPE imageType, char *typeString)
{

	switch(imageType)
	{
		case ASI_IMG_RAW8:
			strcpy(typeString, "RAW8");
			break;

		case ASI_IMG_RGB24:
			strcpy(typeString, "RGB24");
			break;

		case ASI_IMG_RAW16:
			strcpy(typeString, "RAW16");
			break;

		case ASI_IMG_Y8:
			strcpy(typeString, "Y8");
			break;

		default:
			strcpy(typeString, "???");
			break;
	}
}

#endif	//	(_ENABLE_CAMERA_) && defined(_ENABLE_ASI_)

