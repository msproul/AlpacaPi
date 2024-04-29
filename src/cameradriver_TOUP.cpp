//**************************************************************************
//*	Name:			cameradriver_TOUP.cpp
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
//*		http://www.touptek.com/product/showproduct.php?lang=en&id=137
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<PDB>	=	Paul De Barker
//*****************************************************************************
//*	Jan  8,	2020	<MLS> Received ToupTek Guidecam from Cloudy Nights
//*	Jan  9,	2020	<MLS> Created cameradriver_TOUP.cpp
//*	Jan 13,	2020	<MLS> Added HandleToupCallbackEvent()
//*	Jan 14,	2020	<MLS> Successfully saving files from ToupTek camera
//*	Jan 29,	2020	<MLS> Toupcam is working on NVIDIA Jetson board
//*	Mar  5,	2020	<MLS> Working on Toupcam image readout modes
//*	Mar  5,	2020	<MLS> SUPPORTED: ToupTek cameras
//*	Jan 15,	2021	<PDB> Found bug in GetImage_ROI_info()
//*	Jan 24,	2021	<PDB> Working on Read_ImageData()
//*	Jan 24,	2021	<PDB> Added Read_Gain()
//*	Jan 25,	2021	<MLS> Upgraded to driver version 48.18081.20201206
//*	Jan 25,	2021	<MLS> Integrating updates from <PDB>
//*	Mar  1,	2021	<PDB> Changed to RAW8 for now
//*	Mar  1,	2021	<PDB> Updated ToupTech code
//*	Mar  1,	2021	<PDB> Camera working in 16bit mode, reading image properly
//*	Aug 27,	2023	<MLS> Updated Makefile to compile toup and touppi properly
//*	Aug 27,	2023	<MLS> Changed to if (SUCCEEDED(toupResult)) as per .h file
//-----------------------------------------------------------------------------
//*	Feb  4,	2120	<TODO> Add 16 bit readout to Toupcam
//*	Feb 16,	2120	<TODO> Add gain setting to Toupcam
//*****************************************************************************


#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)

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

#include	"toupcam.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"cameradriver_TOUP.h"

//*	these are not defined in toupcam.h, they should be
#define		S_OK			0x00000000		//	Operation successful
#define		S_FALSE			0x00000001		//
#define		E_FAIL			0x80004005		//	Unspecified failure
#define		E_ACCESSDENIED	0x80070005		//	General access denied error
#define		E_INVALIDARG	0x80070057		//	One or more arguments are not valid
#define		E_NOTIMPL		0x80004001		//	Not supported or not implemented
#define		E_NOINTERFACE	0x80004002		//	Interface not supported
#define		E_POINTER		0x80004003		//	Pointer that is not valid
#define		E_UNEXPECTED	0x8000FFFF		//	Unexpected failure
#define		E_OUTOFMEMORY	0x8007000E		//	Out of memory
#define		E_WRONG_THREAD	0x8001010E		//	call function in the wrong thread
#define		E_GEN_FAILURE	0x8007001F		//	device not functioning


#define	kMaxCameraCnt	5



//**************************************************************************************
int	CreateCameraObjects_TOUP(void)
{
int				cameraCount;
int				ii;
bool			rulesFileOK;
int				toupCameraCnt;
ToupcamDeviceV2	toupCamList[TOUPCAM_MAX];
char			rulesFileName[]	=	"99-toupcam.rules";
char			driverVersionString[64];

	CONSOLE_DEBUG(__FUNCTION__);
	cameraCount	=	0;
	strcpy(driverVersionString, Toupcam_Version());
	CONSOLE_DEBUG_W_STR("Toupcam_Version", driverVersionString);
	LogEvent(	"camera",
				"Library version (TOUP)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("camera", "TOUP", driverVersionString);


	toupCameraCnt	=	Toupcam_EnumV2(toupCamList);
	CONSOLE_DEBUG_W_NUM("toupCameraCnt\t=", toupCameraCnt);

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
					"Problem with TOUP rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}


	if (toupCameraCnt > 0)
	{
		for (ii=0; ii < toupCameraCnt; ii++)
		{
			new CameraDriverTOUP(ii);
			cameraCount++;
		}
//		CONSOLE_ABORT(__FUNCTION__);
	}
	return(cameraCount);
}




//**************************************************************************************
static void __stdcall EventCallback(unsigned nEvent, void* pCallbackCtx)
{
CameraDriverTOUP	*thisPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	thisPtr	=	(CameraDriverTOUP *)pCallbackCtx;
	if (thisPtr != NULL)
	{
		thisPtr->HandleToupCallbackEvent(nEvent);
	}
}


//**************************************************************************************
CameraDriverTOUP::CameraDriverTOUP(const int deviceNum)
	:CameraDriver()
{
HRESULT			toupResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceManufAbrev,	"TOUP");


	cCameraID					=	deviceNum;
	cToupPicReady				=	false;
	cIsTriggerCam				=	false;
	cCameraProp.ExposureMin_us	=	400;				//*	0.4 ms
	cCameraProp.ExposureMax_us	=	800 * 1000 *1000;	//*	800 seconds

	ReadTOUPcameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

	if (cToupCamH != NULL)
	{
		toupResult	=	Toupcam_StartPullModeWithCallback(cToupCamH, EventCallback, this);
		if (FAILED(toupResult))
		{
			CONSOLE_DEBUG("FAILED PULLBACK");
		}
	}

}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverTOUP::~CameraDriverTOUP(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
void	CameraDriverTOUP::ReadTOUPcameraInfo(void)
{
unsigned int	ii;
int				toupCameraCnt;
ToupcamDeviceV2	toupCamList[TOUPCAM_MAX];
HRESULT			toupResult;
char			hardwareVer[16];
char			productionDate[10];
char			fpgaVersion[32];
int				bAutoExposure;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceVersion, Toupcam_Version());
	CONSOLE_DEBUG_W_STR("Toupcam_Version", cDeviceVersion);
	toupCameraCnt	=	Toupcam_EnumV2(toupCamList);

	if (cCameraID < toupCameraCnt)
	{
		cToupDeviceInfo	=	toupCamList[cCameraID];

		strcpy(cDeviceManufacturer,	"ToupTek");
		strcpy(cDeviceManufAbrev,	"TOUP");
		strcpy(cDeviceSerialNum,	cToupDeviceInfo.id);

	//	strcpy(cCommonProp.Name,			"ToupTek-");
	//	strcat(cCommonProp.Name,			cToupDeviceInfo.displayname);

		SetCommonPropertyName(NULL, cToupDeviceInfo.displayname);


		cCameraProp.CameraXsize	=	cToupDeviceInfo.model->res[0].width;
		cCameraProp.CameraYsize	=	cToupDeviceInfo.model->res[0].height;
		cCameraProp.NumX		=	cCameraProp.CameraXsize;
		cCameraProp.NumY		=	cCameraProp.CameraYsize;
		cCameraProp.PixelSizeY	=	cToupDeviceInfo.model->xpixsz;
		cCameraProp.PixelSizeX	=	cToupDeviceInfo.model->ypixsz;
		cIsColorCam				=	true;

		if ((cCameraProp.CameraXsize > 0) && (cCameraProp.CameraYsize > 0))
		{
			AllocateImageBuffer(0);
		}

		CONSOLE_DEBUG_W_STR("Display name\t=",	cToupDeviceInfo.displayname);
		CONSOLE_DEBUG_W_STR("ID\t\t\t=",		cToupDeviceInfo.id);

		CONSOLE_DEBUG_W_STR("name\t\t=",		cToupDeviceInfo.model->name);

		CONSOLE_DEBUG_W_HEX("flag\t\t=",		(int)cToupDeviceInfo.model->flag);
		CONSOLE_DEBUG_W_NUM("preview\t\t=",		cToupDeviceInfo.model->preview);
		CONSOLE_DEBUG_W_NUM("still\t\t=",		cToupDeviceInfo.model->still);
		CONSOLE_DEBUG_W_NUM("ioctrol\t\t=",		cToupDeviceInfo.model->ioctrol);
		CONSOLE_DEBUG_W_NUM("maxfanspeed\t=",	cToupDeviceInfo.model->maxfanspeed);

		for (ii=0; ii<cToupDeviceInfo.model->preview; ii++)
		{
			CONSOLE_DEBUG_W_NUM("width\t\t=",		cToupDeviceInfo.model->res[ii].width);
			CONSOLE_DEBUG_W_NUM("height\t\t=",		cToupDeviceInfo.model->res[ii].height);
		}

		//*	check these in this order to make sure we record the highest value
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RAW8)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RAW8");
			cBitDepth		=	8;
			cToupCamFormat	=	TOUPCAM_FLAG_RAW8;
			AddReadoutModeToList(kImageType_RAW8);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RAW10)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RAW10");
			cBitDepth		=	10;
			cToupCamFormat	=	TOUPCAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RAW12)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RAW12");
			cBitDepth		=	12;
			cToupCamFormat	=	TOUPCAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RAW14)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RAW14");
			cBitDepth		=	14;
			cToupCamFormat	=	TOUPCAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RAW16)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RAW16");
			cBitDepth		=	16;
			cToupCamFormat	=	TOUPCAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_ST4)
		{
			cSt4Port	=	true;
			CONSOLE_DEBUG("TOUPCAM_FLAG_ST4");
		}


		//---------------------------------------------
		cToupCamH	=	Toupcam_Open(cToupDeviceInfo.id);
		if (cToupCamH != NULL)
		{
		unsigned int	nFourCC;
		unsigned int	bitsperpixel;
		char			fourCCstring[8];
		char			serialNumberStr[64];
//		int				finalWidth;
//		int				finalHeight;
		char			firmWareVersion[32];

			CONSOLE_DEBUG("Toupcam_Open OK");
#ifdef _USE_PDB_ADDITIONS_
			toupResult  =   Toupcam_put_Option(cToupCamH, TOUPCAM_OPTION_RAW, 1);
			CONSOLE_DEBUG_W_HEX("toupResult\t\t=",		toupResult);
			if (FAILED(toupResult))
			{
				CONSOLE_DEBUG("Failed RAW");
			}
			if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TRIGGER_SOFTWARE)
			{
				cIsTriggerCam	=	true;	//*	<MLS> 1/25/2021
				CONSOLE_DEBUG("TOUPCAM_FLAG_TRIGGER_SOFTWARE");
				toupResult	=	Toupcam_put_Option(cToupCamH, TOUPCAM_OPTION_TRIGGER, 1);
			}
			toupResult	=	Toupcam_get_RawFormat(cToupCamH, &nFourCC, &bitsperpixel);
#endif


			toupResult	=	Toupcam_get_RawFormat(cToupCamH, &nFourCC, &bitsperpixel);
			CONSOLE_DEBUG_W_HEX("toupResult\t\t=",		toupResult);
			CONSOLE_DEBUG_W_NUM("bitsperpixel\t=",		bitsperpixel);
			CONSOLE_DEBUG_W_HEX("nFourCC\t\t=",			nFourCC);

			if (nFourCC == 'GRBG')
			{
				CONSOLE_DEBUG("nFourCC == 'GRBG'");
				AddReadoutModeToList(kImageType_RGB24);
			}
			fourCCstring[0]	=	(nFourCC >> 24) & 0x0ff;
			fourCCstring[1]	=	(nFourCC >> 16) & 0x0ff;
			fourCCstring[2]	=	(nFourCC >> 8) & 0x0ff;
			fourCCstring[3]	=	(nFourCC) & 0x0ff;
			fourCCstring[4]	=	0;
			CONSOLE_DEBUG_W_STR("nFourCC\t\t=",					fourCCstring);

			cBitDepth	=	Toupcam_get_MaxBitDepth(cToupCamH);
			CONSOLE_DEBUG_W_NUM("cBitDepth\t\t=",				cBitDepth);

			toupResult	=	Toupcam_get_SerialNumber(cToupCamH,	serialNumberStr);
			CONSOLE_DEBUG_W_STR("serialNumber\t=",				serialNumberStr);

			toupResult	=	Toupcam_get_FwVersion(cToupCamH,	firmWareVersion);
			CONSOLE_DEBUG_W_STR("firmWareVersion\t=",			firmWareVersion);
			if (SUCCEEDED(toupResult))
			{
				strcpy(cDeviceFirmwareVersStr, firmWareVersion);
			}

			//*	get the camera hardware version, such as: 3.12
			toupResult	=	Toupcam_get_HwVersion(cToupCamH, hardwareVer);
			CONSOLE_DEBUG_W_STR("hardware version\t=",		hardwareVer);

			//*	get the production date, such as: 20150327, YYYYMMDD, (YYYY: year, MM: month, DD: day)
			toupResult	=	Toupcam_get_ProductionDate(cToupCamH, productionDate);
			if (SUCCEEDED(toupResult))
			{
				CONSOLE_DEBUG_W_STR("production date\t=",		productionDate);
				strcpy(cCameraProp.ProductionDate,				productionDate);
			}
			//*	get the FPGA version, such as: 1.13
			toupResult	=	Toupcam_get_FpgaVersion(cToupCamH, fpgaVersion);
			if (SUCCEEDED(toupResult))
			{
				CONSOLE_DEBUG_W_STR("FPGA version\t=",		fpgaVersion);
				strcpy(cCameraProp.FPGAversion,				fpgaVersion);
			}

			toupResult	=	Toupcam_get_MonoMode(cToupCamH);
			//CONSOLE_DEBUG_W_HEX("toupResult\t\t=",		toupResult);
			if (SUCCEEDED(toupResult))
			{
				if (toupResult == S_OK)
				{
					cIsColorCam	=	false;
				}
				CONSOLE_DEBUG_W_BOOL("cIsColorCam\t=", cIsColorCam);
			}
#ifdef _USE_PDB_ADDITIONS_
			// Disable AutoExposure
			toupResult	=	Toupcam_put_AutoExpoEnable(cToupCamH, false);
#endif // _USE_PDB_ADDITIONS_

			toupResult	=	Toupcam_get_AutoExpoEnable(cToupCamH, &bAutoExposure);
			if (SUCCEEDED(toupResult))
			{
				CONSOLE_DEBUG_W_NUM("Auto Exposure enabled\t=",	bAutoExposure);
				if (bAutoExposure != 0)
				{
					cCameraAutoExposure	=	true;
				}
				else
				{
					cCameraAutoExposure	=	false;
				}
			}

		//	CONSOLE_DEBUG("Calling Toupcam_get_FinalSize");
		//	toupResult	=	Toupcam_get_FinalSize(cToupCamH, &finalWidth, &finalHeight);
		//	CONSOLE_DEBUG_W_NUM("finalWidth\t=",	finalWidth);
		//	CONSOLE_DEBUG_W_NUM("finalHeight\t=",	finalHeight);
		//	int		testPattern;
		//	toupResult	=	Toupcam_get_Option(cToupCamH, TOUPCAM_OPTION_TESTPATTERN, &testPattern);
		//	if (SUCCEEDED(toupResult))
		//	{
		//		CONSOLE_DEBUG_W_NUM("testPattern\t=",	testPattern);
		//		toupResult	=	Toupcam_put_Option(cToupCamH, TOUPCAM_OPTION_TESTPATTERN, 9);
		//	}
		//	else
		//	{
		//		CONSOLE_DEBUG_W_HEX("Toupcam_get_Option() failed, toupResult = ", toupResult);
		//	}


		}
		else
		{
			CONSOLE_DEBUG("Toupcam_Open FAILED!!!!");
		}
		//---------------------------------------------

		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_CMOS)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_CMOS");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_CCD_PROGRESSIVE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_CCD_PROGRESSIVE");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_CCD_INTERLACED)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_CCD_INTERLACED");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_ROI_HARDWARE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_ROI_HARDWARE");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_MONO)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_MONO");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_BINSKIP_SUPPORTED)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_BINSKIP_SUPPORTED");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_USB30)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_USB30");
			cIsUSB3Camera	=	true;
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TEC)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_TEC");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_USB30_OVER_USB20)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_USB30_OVER_USB20");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_GETTEMPERATURE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_GETTEMPERATURE");
		}
#ifdef TOUPCAM_FLAG_PUTTEMPERATURE
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_PUTTEMPERATURE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_PUTTEMPERATURE");
		}
#endif
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_FAN)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_FAN");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TEC_ONOFF)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_TEC_ONOFF");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_ISP)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_ISP");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TRIGGER_SOFTWARE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_TRIGGER_SOFTWARE");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TRIGGER_EXTERNAL)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_TRIGGER_EXTERNAL");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_TRIGGER_SINGLE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_TRIGGER_SINGLE");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_BLACKLEVEL)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_BLACKLEVEL");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_AUTO_FOCUS)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_AUTO_FOCUS");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_BUFFER)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_BUFFER");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_DDR)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_DDR");
		}

		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_CG)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_CG");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_YUV411)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_YUV411");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_VUYY)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_VUYY");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_YUV444)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_YUV444");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_RGB888)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_RGB888");
			AddReadoutModeToList(kImageType_RGB24);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_GMCY8)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_GMCY8");
			AddReadoutModeToList(kImageType_Y8);
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_GMCY12)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_GMCY12");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_UYVY)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_UYVY");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_CGHDR)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_CGHDR");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_GLOBALSHUTTER)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_GLOBALSHUTTER");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_FOCUSMOTOR)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_FOCUSMOTOR");
		}
		if (cToupDeviceInfo.model->flag & TOUPCAM_FLAG_PRECISE_FRAMERATE)
		{
			CONSOLE_DEBUG("TOUPCAM_FLAG_PRECISE_FRAMERATE");
		}
	}
}

//*****************************************************************************
//*	the camera must already be open when this is called
//*****************************************************************************
bool	CameraDriverTOUP::GetImage_ROI_info(void)
{

	memset(&cROIinfo, 0, sizeof(TYPE_IMAGE_ROI_Info));

	cROIinfo.currentROIimageType	=	kImageType_RAW8; // PDB kImageType_RGB24;
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;

	return(true);
}


//*****************************************************************************
//*	http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure Content-Type: -dDuration=0.001&Light=true
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;
HRESULT					toupResult;

	//*	this camera is a bit different, it is ALWAYS exposing,
	//*	istrigger

	cInternalCameraState	=	kCameraState_TakingPicture;
	alpacaErrCode			=	kASCOM_Err_Success;

	gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);

	// Still to add some error checking --PDB--
	Toupcam_put_ExpoTime(cToupCamH, exposureMicrosecs);

	// Trigger an exposure if triggercam

	// Still to add some error checking --PDB--
	if (cIsTriggerCam == true)
	{
		toupResult	=	Toupcam_Trigger(cToupCamH, 1);
		if (FAILED(toupResult))
		{
			CONSOLE_DEBUG_W_NUM("Toupcam_Trigger() returned error code:", toupResult);
		}
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS				alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);

	CONSOLE_DEBUG(cLastCameraErrMsg);

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverTOUP::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	exposureState;

//	CONSOLE_DEBUG_W_STR("cInternalCameraState\t=", gCameraStateStrings[cInternalCameraState]);
	switch(cInternalCameraState)
	{
		case kCameraState_Idle:
			exposureState	=	kExposure_Idle;
			break;

		case kCameraState_TakingPicture:
			if (cToupPicReady)
			{
				exposureState	=	kExposure_Success;
				cToupPicReady	=	false;
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
TYPE_ASCOM_STATUS	CameraDriverTOUP::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					toupTek_bitDepth;
int					rgbMode;
HRESULT				toupResult;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cToupCamH != NULL)
	{
		toupTek_bitDepth	=	-1;
		toupResult	=	Toupcam_get_Option(cToupCamH, TOUPCAM_OPTION_BITDEPTH, &toupTek_bitDepth);
		if (SUCCEEDED(toupResult))
		{
			CONSOLE_DEBUG_W_NUM("toupTek_bitDepth (0 = 8 bits mode, 1 = 16 bits mode)\t=", toupTek_bitDepth);
			switch(newImageType)
			{
				case kImageType_RAW8:
				case kImageType_RGB24:
				case kImageType_Y8:
					toupTek_bitDepth	=	0;
					break;

				case kImageType_RAW16:
					toupTek_bitDepth	=	1;
					break;

				default:
					toupTek_bitDepth	=	0;
					break;
			}
			toupResult	=	Toupcam_put_Option(cToupCamH, TOUPCAM_OPTION_BITDEPTH, toupTek_bitDepth);
			if (SUCCEEDED(toupResult))
			{
				cROIinfo.currentROIimageType	=	newImageType;
				cDesiredImageType				=	newImageType;
				alpacaErrCode					=   kASCOM_Err_Success;
			}
			else
			{
				CONSOLE_DEBUG_W_HEX("Toupcam_put_Option failed, toupResult\t=", toupResult);
			}
			toupResult	=	Toupcam_get_Option(cToupCamH, TOUPCAM_OPTION_BITDEPTH, &toupTek_bitDepth);
			CONSOLE_DEBUG_W_NUM("bitDepth (0 = 8 bits mode, 1 = 16 bits mode)\t=", toupTek_bitDepth);

			toupResult	=	Toupcam_get_Option(cToupCamH, TOUPCAM_OPTION_RGB, &rgbMode);
			CONSOLE_DEBUG("0 => RGB24;");
			CONSOLE_DEBUG("1 => enable RGB48 format when bitdepth > 8;");
			CONSOLE_DEBUG("2 => RGB32;");
			CONSOLE_DEBUG("3 => 8 Bits Gray (only for mono camera); 4 => 16 Bits Gray (only for mono camera when bitdepth > 8)");
			CONSOLE_DEBUG("4 => 16 Bits Gray (only for mono camera when bitdepth > 8)");
			CONSOLE_DEBUG_W_NUM("rgbMode\t=", rgbMode);

		}
		else if ((unsigned int)toupResult == E_NOTIMPL)
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			strcpy(cLastCameraErrMsg, "Camera does not support specified pixel depth");
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
	return(alpacaErrCode);
}

#pragma mark -
#pragma mark Virtual functions

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
unsigned short		gain;
HRESULT				toupResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cToupCamH != NULL)
	{
//		CONSOLE_DEBUG("reading gain");
		toupResult	=	Toupcam_get_ExpoAGain(cToupCamH, &gain);
		if (SUCCEEDED(toupResult))
		{
//			CONSOLE_DEBUG("found gain");
			*cameraGainValue	=	gain;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			if ((unsigned int)toupResult == E_NOTIMPL)
			{
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				strcpy(cLastCameraErrMsg, "Camera does not support gain");
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
//	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
//	Set Cooling:
//	int ArtemisSetCooling(ArtemisHandle hCam, int setpoint);
//	This function is used to set the temperature of the camera. The setpoint is in 1/100 of a degree
//	(Celcius). So, to set the cooling to -10C, you need to call the function with setpoint = -1000.
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);

	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


//**************************************************************************
//*	returns error code,
//*	sets class varible to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
short				temperature;
HRESULT				toupResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cToupCamH != NULL)
	{
		toupResult	=	Toupcam_get_Temperature(cToupCamH, &temperature);
		if (SUCCEEDED(toupResult))
		{
			cCameraProp.CCDtemperature	=	(temperature * 1.0) / 10.0;
		}
		else if ((unsigned int)toupResult == E_NOTIMPL)
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			strcpy(cLastCameraErrMsg, "Camera does not support tempeature");
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_CoolerState(bool *coolerOnOff)
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
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_Fastreadout(void)
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

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverTOUP::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
ToupcamFrameInfoV2	toupFrameInfo;

	CONSOLE_DEBUG(__FUNCTION__);

	memset((void*)&toupFrameInfo, 0, sizeof(ToupcamFrameInfoV2));

	GetImage_ROI_info();

	if ((cToupCamH != NULL) && (cCameraDataBuffer != NULL))
	{
		cNewImageReadyToDisplay	=	true;
		cToupPicReady			=	false;
	}
	else
	{
		CONSOLE_DEBUG("Invalid device number or camera buffer failed to allocate");
	}
	return(alpacaErrCode);
}

//**************************************************************************
void	CameraDriverTOUP::HandleToupCallbackEvent(unsigned nEvent)
{
ToupcamFrameInfoV2	toupFrameInfo;
HRESULT				toupResult;

	memset((void*)&toupFrameInfo, 0, sizeof(ToupcamFrameInfoV2));

	switch(nEvent)
	{
		case TOUPCAM_EVENT_EXPOSURE:		//*	exposure time changed
			CONSOLE_DEBUG("TOUPCAM_EVENT_EXPOSURE");
			if (cToupCamH != NULL)
			{
				toupResult	=	Toupcam_get_ExpoTime(cToupCamH, &cToupAutoExpTime_us);	//*	in microseconds
				if (SUCCEEDED(toupResult))
				{
					CONSOLE_DEBUG_W_NUM("expTime_us\t=", cToupAutoExpTime_us);
					cCurrentExposure_us						=	cToupAutoExpTime_us;
					cCameraProp.Lastexposure_duration_us	=	cToupAutoExpTime_us;
				}
				else
				{
					CONSOLE_DEBUG_W_HEX("Toupcam_get_ExpoTime() failed, toupResult = ", toupResult);
				}
			}
			break;

		case TOUPCAM_EVENT_TEMPTINT:		//*	white balance changed, Temp/Tint mode
			CONSOLE_DEBUG_W_NUM(__FUNCTION__, nEvent);
			break;

		case TOUPCAM_EVENT_IMAGE:			//*	live image arrived, use Toupcam_PullImage to get this image
//			CONSOLE_DEBUG("TOUPCAM_EVENT_IMAGE");
// Think it is better move this code out of the call back function
// Probable better place is read_imagedata function, but then some
// indicator should be set here that read_imageda should handle the pull_image
//
// PDB

			if ((cToupCamH != NULL) && (cCameraDataBuffer != NULL))
			{
				//*	we do not want to read the image if an image save is in progress

				// Hardcoded 16bit for now. Must be changed to the proper value based on camerasetting
				// To be done -PDB-
				toupResult	=	Toupcam_PullImageV2(cToupCamH, cCameraDataBuffer, 16, &toupFrameInfo);
				if (SUCCEEDED(toupResult))
				{
				//	CONSOLE_DEBUG_W_HEX("toupFrameInfo.flag\t\t=",		toupFrameInfo.flag);
				//	CONSOLE_DEBUG_W_NUM("toupFrameInfo.timestamp\t=",	toupFrameInfo.timestamp);
					if (cCameraAutoExposure)
					{
						cCurrentExposure_us						=	cToupAutoExpTime_us;
						cCameraProp.Lastexposure_duration_us	=	cToupAutoExpTime_us;
					}

					cNewImageReadyToDisplay	=	true;
					if (cInternalCameraState == kCameraState_TakingPicture)
					{
						cToupPicReady	=	true;
					}
					//*	get the frame rate: framerate (fps) = Frame * 1000.0 / nTime
					unsigned	nFrame;
					unsigned	nTime;
					unsigned	nTotalFrame;
				//	double		myFrameRate;
					toupResult	=	Toupcam_get_FrameRate(cToupCamH, &nFrame, &nTime, &nTotalFrame);
					if (SUCCEEDED(toupResult))
					{
				//		myFrameRate	=	(nFrame * 1000.0) / nTime;
					//	CONSOLE_DEBUG_W_NUM("nFrame\t=",		nFrame);
					//	CONSOLE_DEBUG_W_NUM("nTime\t\t=",		nTime);
					//	CONSOLE_DEBUG_W_NUM("nTotalFrame\t=",	nTotalFrame);
//						CONSOLE_DEBUG_W_DBL("myFrameRate\t=",	myFrameRate);
					}
				}
				else
				{
					CONSOLE_DEBUG_W_HEX("failed to pull image, toupResult = ", toupResult);
				}
			}
			else
			{
				CONSOLE_DEBUG("Internal error");
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;

		case TOUPCAM_EVENT_STILLIMAGE:		//*	snap (still) frame arrived, use Toupcam_PullStillImage to get this frame
		case TOUPCAM_EVENT_WBGAIN:			//*	white balance changed, RGB Gain mode
		case TOUPCAM_EVENT_TRIGGERFAIL:		//*	trigger failed
		case TOUPCAM_EVENT_BLACK :			//*	black balance changed
		case TOUPCAM_EVENT_FFC:				//*	flat field correction status changed
		case TOUPCAM_EVENT_DFC:				//*	dark field correction status changed
		case TOUPCAM_EVENT_ROI:				//*	roi changed
		case TOUPCAM_EVENT_ERROR:			//*	generic error
		case TOUPCAM_EVENT_DISCONNECTED:	//*	camera disconnected
		case TOUPCAM_EVENT_NOFRAMETIMEOUT:	//*	no frame timeout error
		case TOUPCAM_EVENT_AFFEEDBACK:		//*	auto focus feedback information
		case TOUPCAM_EVENT_AFPOSITION:		//*	auto focus sensor board position
		case TOUPCAM_EVENT_NOPACKETTIMEOUT:	//*	no packet timeout
		case TOUPCAM_EVENT_FACTORY:			//*	restore factory settings

		default:
			CONSOLE_DEBUG_W_NUM(__FUNCTION__, nEvent);
			break;
	}
}




#endif	//	defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)

